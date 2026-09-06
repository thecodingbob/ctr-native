#!/usr/bin/env python3
"""Deterministic byte-matching pipeline for CTR NTSC-U 926."""

from __future__ import annotations

import argparse
import difflib
import hashlib
import json
import re
import struct
import subprocess
import sys
from dataclasses import dataclass
from pathlib import Path
from typing import Any, Iterable


ROOT = Path(__file__).resolve().parents[2]
PIPELINE = Path(__file__).resolve()
METADATA = ROOT / "metadata" / "retail" / "ntsc-u-926"
DEFAULT_MANIFEST = METADATA / "matching.json"
BUILD = ROOT / "build" / "matching" / "ntsc-u-926"
CHUNK_SIZE = 1024 * 1024


class MatchError(RuntimeError):
    """A concise, user-facing pipeline error."""


@dataclass(frozen=True)
class Toolchain:
    compiler_version: str
    compiler_banner: str
    compiler_directory: Path
    compiler: Path
    compiler_hashes: dict[str, str]
    maspsx: Path
    maspsx_commit: str
    maspsx_hashes: dict[str, str]
    default_aspsx_version: str
    assembler_banner: str
    binutils: dict[str, Path]
    binutils_hashes: dict[str, str]
    config_hash: str


def parse_int(value: int | str) -> int:
    if isinstance(value, int):
        return value
    return int(value, 0)


def load_json(path: Path) -> dict[str, Any]:
    try:
        value = json.loads(path.read_text())
    except FileNotFoundError as exc:
        raise MatchError(f"missing file: {path}") from exc
    except json.JSONDecodeError as exc:
        raise MatchError(f"invalid JSON in {path}: {exc}") from exc
    if not isinstance(value, dict):
        raise MatchError(f"expected an object in {path}")
    return value


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        while chunk := stream.read(CHUNK_SIZE):
            digest.update(chunk)
    return digest.hexdigest()


def sha256_bytes(value: bytes) -> str:
    return hashlib.sha256(value).hexdigest()


def sha256_json(value: Any) -> str:
    encoded = json.dumps(value, sort_keys=True, separators=(",", ":")).encode()
    return sha256_bytes(encoded)


def artifact_by_id(manifest: dict[str, Any], artifact_id: str) -> dict[str, Any]:
    for artifact in manifest["artifacts"]:
        if artifact["id"] == artifact_id:
            return artifact
    raise MatchError(f"manifest has no artifact named {artifact_id!r}")


def probe_by_symbol(manifest: dict[str, Any], symbol: str) -> dict[str, Any]:
    for probe in manifest["compiler_probes"]:
        if probe["symbol"] == symbol:
            return probe
    names = ", ".join(probe["symbol"] for probe in manifest["compiler_probes"])
    raise MatchError(f"unknown compiler probe {symbol!r}; available: {names}")


def artifact_build_by_id(
    manifest: dict[str, Any], artifact_id: str
) -> dict[str, Any]:
    for build in manifest.get("artifact_builds", []):
        if build["artifact"] == artifact_id:
            return build
    names = ", ".join(
        build["artifact"] for build in manifest.get("artifact_builds", [])
    )
    raise MatchError(f"artifact {artifact_id!r} is not buildable; available: {names}")


def reference_root(manifest: dict[str, Any], override: str | None) -> Path:
    if override:
        return Path(override).expanduser().resolve()
    return ROOT / manifest["reference_root"]


def check_psx_exe_header(path: Path, artifact: dict[str, Any]) -> list[str]:
    header = path.read_bytes()[:0x800]
    errors: list[str] = []
    if len(header) != 0x800:
        return [f"{artifact['id']}: PS-X EXE header is shorter than 0x800 bytes"]
    if header[:8] != b"PS-X EXE":
        errors.append(f"{artifact['id']}: missing PS-X EXE magic")

    fields = {
        "entrypoint": struct.unpack_from("<I", header, 0x10)[0],
        "global_pointer": struct.unpack_from("<I", header, 0x14)[0],
        "load_address": struct.unpack_from("<I", header, 0x18)[0],
        "mapped_size": struct.unpack_from("<I", header, 0x1C)[0],
        "stack_pointer": struct.unpack_from("<I", header, 0x30)[0],
    }
    expected = {
        "entrypoint": parse_int(artifact["header"]["entrypoint"]),
        "global_pointer": parse_int(artifact["header"]["global_pointer"]),
        "load_address": parse_int(artifact["load_address"]),
        "mapped_size": parse_int(artifact["mapped_size"]),
        "stack_pointer": parse_int(artifact["header"]["stack_pointer"]),
    }
    for name, actual in fields.items():
        if actual != expected[name]:
            errors.append(
                f"{artifact['id']}: {name} is 0x{actual:08x}, "
                f"expected 0x{expected[name]:08x}"
            )
    return errors


def verify_references(
    manifest: dict[str, Any], root: Path, selected: set[str] | None = None
) -> list[dict[str, Any]]:
    results: list[dict[str, Any]] = []
    for artifact in manifest["artifacts"]:
        if selected is not None and artifact["id"] not in selected:
            continue
        path = root / artifact["path"]
        result = {
            "id": artifact["id"],
            "path": str(path),
            "expected_size": artifact["size"],
            "expected_sha256": artifact["sha256"],
            "errors": [],
        }
        if not path.is_file():
            result["errors"].append("missing")
            results.append(result)
            continue

        actual_size = path.stat().st_size
        actual_hash = sha256_file(path)
        result["actual_size"] = actual_size
        result["actual_sha256"] = actual_hash
        if actual_size != artifact["size"]:
            result["errors"].append(
                f"size is {actual_size}, expected {artifact['size']}"
            )
        if actual_hash != artifact["sha256"]:
            result["errors"].append(
                f"sha256 is {actual_hash}, expected {artifact['sha256']}"
            )
        if artifact["kind"] == "ps-x-exe":
            result["errors"].extend(check_psx_exe_header(path, artifact))
        results.append(result)
    return results


def first_file_difference(expected: Path, actual: Path) -> int | None:
    offset = 0
    with expected.open("rb") as lhs, actual.open("rb") as rhs:
        while True:
            left = lhs.read(CHUNK_SIZE)
            right = rhs.read(CHUNK_SIZE)
            common = min(len(left), len(right))
            if left[:common] != right[:common]:
                for index, (left_byte, right_byte) in enumerate(
                    zip(left[:common], right[:common])
                ):
                    if left_byte != right_byte:
                        return offset + index
            if len(left) != len(right):
                return offset + common
            if not left:
                return None
            offset += len(left)


def mismatch_location(artifact: dict[str, Any], file_offset: int) -> str:
    mapped_offset = parse_int(artifact.get("mapped_file_offset", 0))
    if "load_address" in artifact and file_offset >= mapped_offset:
        address = parse_int(artifact["load_address"]) + file_offset - mapped_offset
        return f"file+0x{file_offset:x} / vram 0x{address:08x}"
    return f"file+0x{file_offset:x}"


def compare_artifacts(
    manifest: dict[str, Any],
    expected_root: Path,
    candidate_root: Path,
    selected: set[str] | None = None,
) -> list[dict[str, Any]]:
    results: list[dict[str, Any]] = []
    for artifact in manifest["artifacts"]:
        if selected is not None and artifact["id"] not in selected:
            continue
        expected = expected_root / artifact["path"]
        actual = candidate_root / artifact["path"]
        result: dict[str, Any] = {
            "id": artifact["id"],
            "expected": str(expected),
            "candidate": str(actual),
            "exact": False,
        }
        if not expected.is_file():
            result["error"] = "reference missing"
        elif not actual.is_file():
            result["error"] = "candidate missing"
        else:
            result["expected_size"] = expected.stat().st_size
            result["candidate_size"] = actual.stat().st_size
            result["expected_sha256"] = sha256_file(expected)
            result["candidate_sha256"] = sha256_file(actual)
            if (
                result["expected_size"] != artifact["size"]
                or result["expected_sha256"] != artifact["sha256"]
            ):
                result["error"] = "reference does not match the committed oracle"
                results.append(result)
                continue
            result["exact"] = (
                result["expected_size"] == result["candidate_size"]
                and result["expected_sha256"] == result["candidate_sha256"]
            )
            if not result["exact"]:
                difference = first_file_difference(expected, actual)
                if difference is not None:
                    result["first_difference"] = difference
                    result["first_difference_location"] = mismatch_location(
                        artifact, difference
                    )
        results.append(result)
    return results


def _scan_balanced_body(text: str, opening_brace: int) -> int:
    depth = 0
    state = "code"
    index = opening_brace
    while index < len(text):
        char = text[index]
        following = text[index + 1] if index + 1 < len(text) else ""
        if state == "code":
            if char == "/" and following == "/":
                state = "line-comment"
                index += 2
                continue
            if char == "/" and following == "*":
                state = "block-comment"
                index += 2
                continue
            if char == '"':
                state = "string"
            elif char == "'":
                state = "character"
            elif char == "{":
                depth += 1
            elif char == "}":
                depth -= 1
                if depth == 0:
                    return index + 1
        elif state == "line-comment":
            if char == "\n":
                state = "code"
        elif state == "block-comment":
            if char == "*" and following == "/":
                state = "code"
                index += 2
                continue
        elif state in {"string", "character"}:
            if char == "\\":
                index += 2
                continue
            if (state == "string" and char == '"') or (
                state == "character" and char == "'"
            ):
                state = "code"
        index += 1
    raise MatchError("unterminated function body")


def extract_function(text: str, symbol: str) -> str:
    pattern = re.compile(
        rf"(?m)^[ \t]*[^\n;{{}}]*\b{re.escape(symbol)}[ \t]*\([^;{{}}]*\)"
        rf"[ \t]*(?:\n[ \t]*)?\{{"
    )
    matches = list(pattern.finditer(text))
    if len(matches) != 1:
        raise MatchError(
            f"expected one definition of {symbol}, found {len(matches)}"
        )
    match = matches[0]
    opening_brace = text.find("{", match.start(), match.end())
    end = _scan_balanced_body(text, opening_brace)
    return text[match.start() : end].strip() + "\n"


def command_output(command: list[str]) -> str:
    result = subprocess.run(
        command, check=True, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT
    )
    return result.stdout.strip()


def repository_path(value: str) -> Path:
    path = (ROOT / value).resolve()
    try:
        path.relative_to(ROOT)
    except ValueError as exc:
        raise MatchError(f"path escapes the repository: {value}") from exc
    return path


def require_tool(path: Path, description: str, expected_hash: str) -> Path:
    if not path.is_file():
        raise MatchError(f"vendored {description} is missing: {path}")
    actual_hash = sha256_file(path)
    if actual_hash != expected_hash:
        raise MatchError(
            f"vendored {description} has sha256 {actual_hash}, "
            f"expected {expected_hash}"
        )
    return path


def resolve_toolchain(manifest: dict[str, Any]) -> Toolchain:
    config = manifest["toolchain"]
    compiler = config["compiler"]
    gcc_dir = repository_path(compiler["directory"])
    compiler_tools = {}
    for name, expected_hash in compiler["sha256"].items():
        compiler_tools[name] = require_tool(
            gcc_dir / name,
            f"GCC {compiler['version']} {name}",
            expected_hash,
        )
    gcc = compiler_tools[compiler["executable"]]

    maspsx_config = config["maspsx"]
    maspsx_dir = repository_path(maspsx_config["directory"])
    maspsx_files = {}
    for name, expected_hash in maspsx_config["sha256"].items():
        maspsx_files[name] = require_tool(
            maspsx_dir / name,
            f"maspsx {name}",
            expected_hash,
        )
    maspsx = maspsx_files[maspsx_config["entrypoint"]]

    binutils = config["binutils"]
    prefix = repository_path(binutils["prefix"])
    tools = {}
    for name, expected_hash in binutils["sha256"].items():
        tools[name] = require_tool(
            Path(f"{prefix}{name}"),
            f"GNU binutils {name}",
            expected_hash,
        )

    compiler_banner = command_output([str(gcc), "--version"]).splitlines()[0]
    if compiler_banner != compiler["version"]:
        raise MatchError(
            f"vendored GCC reports {compiler_banner}, "
            f"expected {compiler['version']}"
        )

    assembler_banner = command_output([str(tools["as"]), "--version"]).splitlines()[0]
    if binutils["version"] not in assembler_banner:
        raise MatchError(
            f"vendored assembler reports {assembler_banner}, "
            f"expected binutils {binutils['version']}"
        )

    return Toolchain(
        compiler_version=compiler["version"],
        compiler_banner=compiler_banner,
        compiler_directory=gcc_dir,
        compiler=gcc,
        compiler_hashes=compiler["sha256"],
        maspsx=maspsx,
        maspsx_commit=maspsx_config["commit"],
        maspsx_hashes=maspsx_config["sha256"],
        default_aspsx_version=maspsx_config["aspsx_version"],
        assembler_banner=assembler_banner,
        binutils=tools,
        binutils_hashes=binutils["sha256"],
        config_hash=sha256_json(config),
    )


def reference_bytes(
    manifest: dict[str, Any], root: Path, probe: dict[str, Any]
) -> bytes:
    artifact = artifact_by_id(manifest, probe["region"])
    address = parse_int(probe["address"])
    load_address = parse_int(artifact["load_address"])
    file_offset = parse_int(artifact.get("mapped_file_offset", 0))
    offset = file_offset + address - load_address
    if offset < 0:
        raise MatchError(f"{probe['symbol']} precedes {artifact['id']} load address")
    path = root / artifact["path"]
    if not path.is_file():
        raise MatchError(f"reference artifact is missing: {path}")
    if (
        path.stat().st_size != artifact["size"]
        or sha256_file(path) != artifact["sha256"]
    ):
        raise MatchError(
            f"reference artifact does not match the committed oracle: {path}"
        )
    with path.open("rb") as stream:
        stream.seek(offset)
        value = stream.read(probe["size"])
    if len(value) != probe["size"]:
        raise MatchError(
            f"reference slice for {probe['symbol']} is {len(value)} bytes, "
            f"expected {probe['size']}"
        )
    return value


def run_checked(command: list[str], stdin_path: Path | None = None) -> None:
    if stdin_path is None:
        result = subprocess.run(
            command, stdout=subprocess.PIPE, stderr=subprocess.PIPE
        )
    else:
        with stdin_path.open("rb") as stream:
            result = subprocess.run(
                command, stdin=stream, stdout=subprocess.PIPE, stderr=subprocess.PIPE
            )
    if result.returncode != 0:
        rendered = " ".join(command)
        details = result.stderr.decode(errors="replace").strip()
        suffix = f"\n{details}" if details else ""
        raise MatchError(f"command failed ({result.returncode}): {rendered}{suffix}")


def compile_c(
    toolchain: Toolchain,
    source: Path,
    assembly: Path,
    flags: Iterable[str],
    include_directories: Iterable[Path] = (),
    forced_includes: Iterable[Path] = (),
    dependency_file: Path | None = None,
) -> None:
    command = [
        str(toolchain.compiler),
        f"-B{toolchain.compiler_directory}/",
        "-x",
        "c",
        "-S",
        "-o",
        str(assembly),
        *flags,
    ]
    if dependency_file is not None:
        command.append(f"-Wp,-MD,{dependency_file}")
    command.extend(f"-I{path}" for path in include_directories)
    for path in forced_includes:
        command.extend(["-include", str(path)])
    command.append(str(source))
    run_checked(command)


def assemble_psx(
    toolchain: Toolchain,
    assembly: Path,
    object_file: Path,
    aspsx_version: str,
    small_data_limit: int,
) -> None:
    run_checked(
        [
            sys.executable,
            str(toolchain.maspsx),
            f"--aspsx-version={aspsx_version}",
            "--run-assembler",
            f"--gnu-as-path={toolchain.binutils['as']}",
            "--dont-force-G0",
            f"-G{small_data_limit}",
            "-o",
            str(object_file),
        ],
        stdin_path=assembly,
    )


def extract_binary_section(
    toolchain: Toolchain,
    source: Path,
    section: str,
    output: Path,
) -> None:
    run_checked(
        [
            str(toolchain.binutils["objcopy"]),
            "-O",
            "binary",
            "-j",
            section,
            str(source),
            str(output),
        ]
    )


def mismatch_offsets(expected: bytes, actual: bytes, limit: int = 16) -> list[int]:
    offsets = [
        index
        for index, (left, right) in enumerate(zip(expected, actual))
        if left != right
    ]
    if len(actual) != len(expected):
        offsets.extend(
            range(min(len(actual), len(expected)), max(len(actual), len(expected)))
        )
    return offsets[:limit]


def range_comparison(
    expected: bytes,
    actual: bytes,
    ranges: list[dict[str, Any]],
    symbols: dict[str, dict[str, int]] | None = None,
) -> list[dict[str, Any]]:
    results: list[dict[str, Any]] = []
    symbols = symbols or {}
    for item in ranges:
        offset = parse_int(item["offset"])
        size = parse_int(item["size"])
        candidate_offset = offset
        candidate_size = size
        if symbol_name := item.get("candidate_symbol"):
            if symbol_name not in symbols:
                raise MatchError(
                    f"linked artifact has no symbol named {symbol_name!r}"
                )
            candidate_offset = symbols[symbol_name]["offset"]
            symbol_size = symbols[symbol_name]["size"]
            if symbol_size:
                candidate_size = min(size, symbol_size)
        expected_slice = expected[offset : offset + size]
        actual_slice = actual[candidate_offset : candidate_offset + candidate_size]
        matching_bytes = sum(
            left == right for left, right in zip(expected_slice, actual_slice)
        )
        result = {
            **item,
            "offset": offset,
            "size": size,
            "candidate_offset": candidate_offset,
            "candidate_size": len(actual_slice),
            "placement_delta": candidate_offset - offset,
            "matching_bytes": matching_bytes,
            "expected_sha256": sha256_bytes(expected_slice),
            "candidate_sha256": sha256_bytes(actual_slice),
            "content_exact": actual_slice == expected_slice,
            "placement_exact": candidate_offset == offset,
            "exact": candidate_offset == offset and actual_slice == expected_slice,
        }
        if item["kind"] == "code":
            matching_words = sum(
                actual_slice[index : index + 4] == expected_slice[index : index + 4]
                for index in range(0, len(expected_slice), 4)
                if len(actual_slice[index : index + 4]) == 4
            )
            result["matching_words"] = matching_words
            result["total_words"] = len(expected_slice) // 4
        results.append(result)
    return results


def linked_symbols(
    objdump: Path, linked_object: Path, load_address: int
) -> dict[str, dict[str, int]]:
    output = command_output([str(objdump), "-t", str(linked_object)])
    symbols: dict[str, dict[str, int]] = {}
    for line in output.splitlines():
        parts = line.split()
        if (
            len(parts) >= 3
            and re.fullmatch(r"[0-9a-fA-F]{8}", parts[0])
            and re.fullmatch(r"[0-9a-fA-F]{8}", parts[-2])
        ):
            address = int(parts[0], 16)
            symbols[parts[-1]] = {
                "address": address,
                "offset": address - load_address,
                "size": int(parts[-2], 16),
            }
    return symbols


def normalized_objdump(command: list[str]) -> str:
    output = command_output(command)
    instruction = re.compile(r"^[0-9a-f]+:\s")
    lines = [line.rstrip() for line in output.splitlines() if instruction.match(line)]
    return "\n".join(lines) + "\n"


def dependency_inputs(path: Path) -> list[str]:
    text = path.read_text().replace("\\\n", " ")
    try:
        _, values = text.split(":", 1)
    except ValueError as exc:
        raise MatchError(f"invalid dependency file: {path}") from exc
    inputs: list[str] = []
    for value in values.split():
        dependency = repository_path(value)
        inputs.append(str(dependency.relative_to(ROOT)))
    return inputs


def artifact_build_input_hashes(
    build: dict[str, Any], dependency_file: Path | None = None
) -> dict[str, str]:
    inputs = [
        build["source"],
        build["linker_script"],
        *build.get("forced_includes", []),
    ]
    if dependency_file is not None:
        inputs.extend(dependency_inputs(dependency_file))
    inputs = list(dict.fromkeys(inputs))
    return {value: sha256_file(repository_path(value)) for value in inputs}


def artifact_inputs_are_current(
    build: dict[str, Any], recorded: Any
) -> bool:
    if not isinstance(recorded, dict):
        return False
    declared = set(artifact_build_input_hashes(build))
    if not declared.issubset(recorded):
        return False
    try:
        return all(
            sha256_file(repository_path(path)) == expected
            for path, expected in recorded.items()
        )
    except (FileNotFoundError, MatchError):
        return False


def build_artifact(
    manifest: dict[str, Any],
    toolchain: Toolchain,
    build: dict[str, Any],
    references: Path,
) -> dict[str, Any]:
    artifact = artifact_by_id(manifest, build["artifact"])
    verification = verify_references(manifest, references, {artifact["id"]})[0]
    if verification["errors"]:
        raise MatchError(
            f"reference artifact {artifact['id']} failed verification: "
            f"{'; '.join(verification['errors'])}"
        )

    source = repository_path(build["source"])
    linker_script = repository_path(build["linker_script"])
    include_directories = [
        repository_path(directory) for directory in build["include_directories"]
    ]
    forced_includes = [
        repository_path(path) for path in build.get("forced_includes", [])
    ]
    output = BUILD / "artifacts" / artifact["id"]
    output.mkdir(parents=True, exist_ok=True)
    assembly = output / f"{artifact['id']}.s"
    object_file = output / f"{artifact['id']}.o"
    linked_object = output / f"{artifact['id']}.elf"
    candidate_binary = output / f"{artifact['id']}.bin"
    dependency_file = output / f"{artifact['id']}.d"

    aspsx_version = build.get("aspsx_version", toolchain.default_aspsx_version)
    compile_c(
        toolchain,
        source,
        assembly,
        build["compiler_flags"],
        include_directories,
        forced_includes,
        dependency_file,
    )
    assemble_psx(
        toolchain,
        assembly,
        object_file,
        aspsx_version,
        build["small_data_limit"],
    )

    link_command = [
        str(toolchain.binutils["ld"]),
        "-T",
        str(linker_script),
        "-o",
        str(linked_object),
        f"--defsym=__overlay_load_address={artifact['load_address']}",
        f"--defsym=__overlay_id={build['overlay_id']}",
    ]
    for symbol, address in build["symbols"].items():
        link_command.append(f"--defsym={symbol}={address}")
    link_command.append(str(object_file))
    run_checked(link_command)
    extract_binary_section(
        toolchain,
        linked_object,
        build["output_section"],
        candidate_binary,
    )

    expected_path = references / artifact["path"]
    expected = expected_path.read_bytes()
    actual = candidate_binary.read_bytes()
    reconstructed = BUILD / "reconstructed" / artifact["path"]
    reconstructed.parent.mkdir(parents=True, exist_ok=True)
    reconstructed.write_bytes(actual)

    load_address = parse_int(artifact["load_address"])
    symbols = linked_symbols(
        toolchain.binutils["objdump"], linked_object, load_address
    )
    objdump_base = [
        str(toolchain.binutils["objdump"]),
        "-D",
        "-b",
        "binary",
        "-m",
        "mips:3000",
        "-EL",
        f"--adjust-vma=0x{load_address:08x}",
    ]
    expected_disassembly = normalized_objdump([*objdump_base, str(expected_path)])
    candidate_disassembly = normalized_objdump([*objdump_base, str(candidate_binary)])
    expected_objdump = output / "retail.objdump"
    candidate_objdump = output / "candidate.objdump"
    diff_path = output / "objdump.diff"
    expected_objdump.write_text(expected_disassembly)
    candidate_objdump.write_text(candidate_disassembly)
    diff = difflib.unified_diff(
        expected_disassembly.splitlines(keepends=True),
        candidate_disassembly.splitlines(keepends=True),
        fromfile=f"retail/{artifact['id']}",
        tofile=f"candidate/{artifact['id']}",
    )
    diff_path.write_text("".join(diff))

    matching_bytes = sum(left == right for left, right in zip(expected, actual))
    first_mismatch = mismatch_offsets(expected, actual, limit=1)
    result = {
        "schema_version": 1,
        "target": manifest["target"],
        "artifact": artifact["id"],
        "artifact_build_config_sha256": sha256_json(build),
        "build_input_sha256": artifact_build_input_hashes(build, dependency_file),
        "pipeline_sha256": sha256_file(PIPELINE),
        "expected_size": len(expected),
        "candidate_size": len(actual),
        "expected_sha256": sha256_bytes(expected),
        "candidate_sha256": sha256_bytes(actual),
        "matching_bytes": matching_bytes,
        "matching_byte_percent": matching_bytes * 100 / len(expected),
        "first_mismatch_offset": first_mismatch[0] if first_mismatch else None,
        "first_mismatch_location": (
            mismatch_location(artifact, first_mismatch[0])
            if first_mismatch
            else None
        ),
        "ranges": range_comparison(expected, actual, build["ranges"], symbols),
        "compiler_version": toolchain.compiler_banner,
        "compiler_sha256": toolchain.compiler_hashes,
        "compiler_flags": build["compiler_flags"],
        "maspsx_commit": toolchain.maspsx_commit,
        "maspsx_sha256": toolchain.maspsx_hashes,
        "aspsx_version": aspsx_version,
        "binutils_version": toolchain.assembler_banner,
        "binutils_sha256": toolchain.binutils_hashes,
        "toolchain_config_sha256": toolchain.config_hash,
        "candidate": str(candidate_binary.relative_to(ROOT)),
        "objdump_diff": str(diff_path.relative_to(ROOT)),
        "exact": actual == expected,
    }
    (output / "result.json").write_text(json.dumps(result, indent=2) + "\n")
    return result


def compiler_flags(probe: dict[str, Any], optimization: str | None) -> list[str]:
    flags = list(probe["compiler_flags"])
    if optimization is None:
        return flags
    flags = [flag for flag in flags if not re.fullmatch(r"-O(?:[0-3s]|fast)?", flag)]
    flags.insert(0, f"-O{optimization}")
    return flags


def build_probe(
    manifest: dict[str, Any],
    toolchain: Toolchain,
    probe: dict[str, Any],
    aspsx_version: str,
    references: Path,
    optimization: str | None = None,
) -> dict[str, Any]:
    compiler = toolchain.compiler_version
    linker = toolchain.binutils["ld"]

    output = (
        BUILD
        / "probes"
        / probe["symbol"]
        / f"gcc-{compiler}-aspsx-{aspsx_version}"
    )
    if optimization is not None:
        output = (
            BUILD
            / "probes"
            / probe["symbol"]
            / f"gcc-{compiler}-O{optimization}-aspsx-{aspsx_version}"
        )
    output.mkdir(parents=True, exist_ok=True)
    source_path = ROOT / probe["source"]
    source_text = probe.get("compile_prelude", "")
    if source_text and not source_text.endswith("\n"):
        source_text += "\n"
    source_text += extract_function(source_path.read_text(), probe["symbol"])
    extracted_source = output / f"{probe['symbol']}.c"
    assembly = output / f"{probe['symbol']}.s"
    object_file = output / f"{probe['symbol']}.o"
    linked_object = output / f"{probe['symbol']}.elf"
    candidate_binary = output / f"{probe['symbol']}.bin"
    extracted_source.write_text(source_text)

    flags = compiler_flags(probe, optimization)
    compile_c(toolchain, extracted_source, assembly, flags)
    assemble_psx(
        toolchain,
        assembly,
        object_file,
        aspsx_version,
        probe["small_data_limit"],
    )
    binary_input = object_file
    if link := probe.get("link"):
        link_command = [
            linker,
            f"-Ttext={link['text_address']}",
            "-e",
            probe["symbol"],
            "-o",
            str(linked_object),
        ]
        for symbol, address in link.get("symbols", {}).items():
            link_command.append(f"--defsym={symbol}={address}")
        link_command.append(str(object_file))
        run_checked(link_command)
        binary_input = linked_object
    extract_binary_section(toolchain, binary_input, ".text", candidate_binary)

    expected = reference_bytes(manifest, references, probe)
    actual = candidate_binary.read_bytes()
    result = {
        "schema_version": 1,
        "target": manifest["target"],
        "symbol": probe["symbol"],
        "probe_config_sha256": sha256_json(probe),
        "pipeline_sha256": sha256_file(PIPELINE),
        "source": probe["source"],
        "source_sha256": sha256_file(source_path),
        "extracted_source_sha256": sha256_bytes(source_text.encode()),
        "region": probe["region"],
        "address": probe["address"],
        "expected_size": len(expected),
        "candidate_size": len(actual),
        "expected_sha256": sha256_bytes(expected),
        "candidate_sha256": sha256_bytes(actual),
        "compiler": compiler,
        "compiler_version": toolchain.compiler_banner,
        "compiler_sha256": toolchain.compiler_hashes,
        "compiler_flags": flags,
        "optimization": optimization,
        "link": probe.get("link"),
        "maspsx_commit": toolchain.maspsx_commit,
        "maspsx_sha256": toolchain.maspsx_hashes,
        "aspsx_version": aspsx_version,
        "binutils_version": toolchain.assembler_banner,
        "binutils_sha256": toolchain.binutils_hashes,
        "toolchain_config_sha256": toolchain.config_hash,
        "exact": actual == expected,
        "first_mismatch_offsets": mismatch_offsets(expected, actual),
    }
    (output / "result.json").write_text(json.dumps(result, indent=2) + "\n")
    return result


def print_verification(results: Iterable[dict[str, Any]]) -> bool:
    exact = True
    for result in results:
        if result["errors"]:
            exact = False
            print(f"FAIL    {result['id']}: {'; '.join(result['errors'])}")
        else:
            print(
                f"OK      {result['id']}: {result['actual_size']} bytes "
                f"{result['actual_sha256'][:12]}"
            )
    return exact


def print_comparison(results: Iterable[dict[str, Any]]) -> bool:
    exact = True
    for result in results:
        if result["exact"]:
            print(
                f"MATCH   {result['id']}: {result['candidate_size']} bytes "
                f"{result['candidate_sha256'][:12]}"
            )
        else:
            exact = False
            detail = result.get("error")
            if detail is None:
                detail = (
                    "first difference at "
                    f"{result.get('first_difference_location', 'unknown')}; "
                    f"candidate size {result['candidate_size']}, "
                    f"expected {result['expected_size']}"
                )
            print(f"DIFF    {result['id']}: {detail}")
    return exact


def parse_selected(
    manifest: dict[str, Any], values: list[str] | None
) -> set[str] | None:
    if not values:
        return None
    selected = set(values)
    known = {artifact["id"] for artifact in manifest["artifacts"]}
    if unknown := selected - known:
        raise MatchError(f"unknown artifact(s): {', '.join(sorted(unknown))}")
    return selected


def print_toolchain(toolchain: Toolchain) -> None:
    print(
        f"OK      GCC {toolchain.compiler_banner}: "
        f"{toolchain.compiler_hashes['gcc'][:12]}"
    )
    print(
        f"OK      maspsx {toolchain.maspsx_commit[:12]}: "
        f"{toolchain.maspsx_hashes['maspsx.py'][:12]}"
    )
    print(
        f"OK      {toolchain.assembler_banner}: "
        f"{toolchain.binutils_hashes['as'][:12]}"
    )


def cmd_toolchain(args: argparse.Namespace) -> int:
    manifest = load_json(Path(args.manifest))
    print_toolchain(resolve_toolchain(manifest))
    return 0


def cmd_verify(args: argparse.Namespace) -> int:
    manifest = load_json(Path(args.manifest))
    results = verify_references(
        manifest,
        reference_root(manifest, args.reference_root),
        parse_selected(manifest, args.artifact),
    )
    return 0 if print_verification(results) else 1


def cmd_compare(args: argparse.Namespace) -> int:
    manifest = load_json(Path(args.manifest))
    results = compare_artifacts(
        manifest,
        reference_root(manifest, args.reference_root),
        Path(args.candidate_root).expanduser().resolve(),
        parse_selected(manifest, args.artifact),
    )
    if args.report:
        report = Path(args.report)
        report.parent.mkdir(parents=True, exist_ok=True)
        report.write_text(json.dumps(results, indent=2) + "\n")
    return 0 if print_comparison(results) else 1


def print_artifact_result(result: dict[str, Any]) -> bool:
    status = "MATCH" if result["exact"] else "DIFF"
    print(
        f"{status:<7} {result['artifact']}: "
        f"{result['candidate_size']}/{result['expected_size']} bytes; "
        f"{result['matching_bytes']} bytes equal at the same offsets "
        f"({result['matching_byte_percent']:.2f}%)"
    )
    if not result["exact"]:
        print(f"        first difference: {result['first_mismatch_location']}")
    for item in result["ranges"]:
        detail = f"{item['matching_bytes']}/{item['size']} bytes"
        if item["kind"] == "code":
            detail += f", {item['matching_words']}/{item['total_words']} words"
        if item["placement_delta"]:
            detail += f", placement {item['placement_delta']:+#x}"
        print(f"        {item['name']}: {detail}")
    print(f"        objdump diff: {result['objdump_diff']}")
    return bool(result["exact"])


def cmd_check(args: argparse.Namespace) -> int:
    manifest = load_json(Path(args.manifest))
    toolchain = resolve_toolchain(manifest)
    print_toolchain(toolchain)
    references = reference_root(manifest, args.reference_root)
    verification = verify_references(manifest, references)
    if not print_verification(verification):
        return 1

    compiler = toolchain.compiler_version
    aspsx_version = args.aspsx_version or toolchain.default_aspsx_version
    exact = 0
    print("source probes:")
    for probe in manifest["compiler_probes"]:
        result = build_probe(
            manifest,
            toolchain,
            probe,
            aspsx_version,
            references,
        )
        exact += int(result["exact"])
        status = "MATCH" if result["exact"] else "DIFF"
        print(
            f"{status:<7} {result['symbol']} {result['address']} "
            f"{result['candidate_size']}/{result['expected_size']}"
        )
    print(
        f"{exact}/{len(manifest['compiler_probes'])} probes matched with "
        f"GCC {compiler}, ASPSX {aspsx_version}"
    )

    artifact_builds = manifest.get("artifact_builds", [])
    exact_artifacts = 0
    if artifact_builds:
        print("linked artifacts:")
    for build in artifact_builds:
        result = build_artifact(manifest, toolchain, build, references)
        exact_artifacts += int(print_artifact_result(result))
    if artifact_builds:
        print(f"{exact_artifacts}/{len(artifact_builds)} linked artifacts matched")

    all_probes_exact = exact == len(manifest["compiler_probes"])
    all_artifacts_exact = exact_artifacts == len(artifact_builds)
    return 0 if all_probes_exact and all_artifacts_exact else 1


def cmd_probe(args: argparse.Namespace) -> int:
    manifest = load_json(Path(args.manifest))
    toolchain = resolve_toolchain(manifest)
    probe = probe_by_symbol(manifest, args.symbol)
    aspsx_version = args.aspsx_version or toolchain.default_aspsx_version
    result = build_probe(
        manifest,
        toolchain,
        probe,
        aspsx_version,
        reference_root(manifest, args.reference_root),
        args.optimization,
    )
    status = "MATCH" if result["exact"] else "DIFF"
    print(
        f"{status:<7} {result['symbol']} {result['address']}: "
        f"{result['candidate_size']}/{result['expected_size']} bytes; "
        f"GCC {result['compiler']}, ASPSX {result['aspsx_version']}"
    )
    if not result["exact"] and result["first_mismatch_offsets"]:
        offsets = ", ".join(
            f"0x{value:x}" for value in result["first_mismatch_offsets"]
        )
        print(f"        first mismatch offsets: {offsets}")
    return 0 if result["exact"] else 1


def cmd_artifact(args: argparse.Namespace) -> int:
    manifest = load_json(Path(args.manifest))
    toolchain = resolve_toolchain(manifest)
    result = build_artifact(
        manifest,
        toolchain,
        artifact_build_by_id(manifest, args.artifact),
        reference_root(manifest, args.reference_root),
    )
    return 0 if print_artifact_result(result) else 1


def cmd_matrix(args: argparse.Namespace) -> int:
    manifest = load_json(Path(args.manifest))
    toolchain = resolve_toolchain(manifest)
    probe = probe_by_symbol(manifest, args.symbol)
    compiler = toolchain.compiler_version
    references = reference_root(manifest, args.reference_root)

    matrix_results: list[dict[str, Any]] = []
    matches = 0
    total = 0
    optimizations: list[str | None]
    if args.all_optimizations:
        optimizations = ["0", "1", "2", "3"]
    elif args.optimization:
        optimizations = args.optimization
    else:
        optimizations = [None]

    for aspsx_version in probe["aspsx_versions"]:
        for optimization in optimizations:
            total += 1
            try:
                result = build_probe(
                    manifest,
                    toolchain,
                    probe,
                    aspsx_version,
                    references,
                    optimization,
                )
            except MatchError as exc:
                result = {
                    "compiler": compiler,
                    "aspsx_version": aspsx_version,
                    "optimization": optimization,
                    "exact": False,
                    "error": str(exc),
                }
                matrix_results.append(result)
                opt_label = (
                    f"O{optimization}" if optimization is not None else "default"
                )
                print(
                    f"ERROR   GCC {compiler:<7} {opt_label:<7} "
                    f"ASPSX {aspsx_version}: compile failed"
                )
                continue
            matrix_results.append(result)
            matches += int(result["exact"])
            status = "MATCH" if result["exact"] else "DIFF"
            opt_label = (
                f"O{optimization}" if optimization is not None else "default"
            )
            print(
                f"{status:<7} GCC {compiler:<7} {opt_label:<7} "
                f"ASPSX {aspsx_version}"
            )
    matrix_report = BUILD / "matrices" / f"{probe['symbol']}.json"
    matrix_report.parent.mkdir(parents=True, exist_ok=True)
    matrix_report.write_text(json.dumps(matrix_results, indent=2) + "\n")
    print(f"{matches}/{total} matrix entries matched")
    return 0


def cmd_status(args: argparse.Namespace) -> int:
    manifest = load_json(Path(args.manifest))
    compiler = manifest["toolchain"]["compiler"]["version"]
    aspsx_version = manifest["toolchain"]["maspsx"]["aspsx_version"]
    expected_toolchain_hash = sha256_json(manifest["toolchain"])
    probes = {
        probe["symbol"]: probe for probe in manifest["compiler_probes"]
    }
    results = [
        BUILD
        / "probes"
        / symbol
        / f"gcc-{compiler}-aspsx-{aspsx_version}"
        / "result.json"
        for symbol in probes
    ]
    exact_results = 0
    stale_results = 0
    for path in results:
        if not path.is_file():
            stale_results += 1
            continue
        try:
            result = load_json(path)
        except MatchError:
            stale_results += 1
            continue
        source = ROOT / result.get("source", "")
        probe = probes.get(result.get("symbol"))
        expected_probe_hash = sha256_json(probe) if probe is not None else None
        if (
            not source.is_file()
            or result.get("source_sha256") != sha256_file(source)
            or result.get("probe_config_sha256") != expected_probe_hash
            or result.get("pipeline_sha256") != sha256_file(PIPELINE)
            or result.get("toolchain_config_sha256") != expected_toolchain_hash
        ):
            stale_results += 1
            continue
        exact_results += int(result.get("exact", False))
    print(f"target:             {manifest['target']}")
    print(f"goal artifacts:     {len(manifest['artifacts'])}")
    print(f"compiler probes:    {len(manifest['compiler_probes'])}")
    print(f"vendored old GCC:   {compiler}")
    current_results = len(results) - stale_results
    print(
        f"baseline results:   {exact_results}/{current_results} exact, "
        f"{stale_results} missing/stale"
    )
    artifact_builds = manifest.get("artifact_builds", [])
    current_artifacts = 0
    exact_artifacts = 0
    for build in artifact_builds:
        path = BUILD / "artifacts" / build["artifact"] / "result.json"
        if not path.is_file():
            continue
        try:
            result = load_json(path)
            current = (
                result.get("artifact_build_config_sha256") == sha256_json(build)
                and artifact_inputs_are_current(
                    build, result.get("build_input_sha256")
                )
                and result.get("pipeline_sha256") == sha256_file(PIPELINE)
                and result.get("toolchain_config_sha256")
                == expected_toolchain_hash
            )
        except (MatchError, FileNotFoundError):
            current = False
        if current:
            current_artifacts += 1
            exact_artifacts += int(result.get("exact", False))
    stale_artifacts = len(artifact_builds) - current_artifacts
    print(
        f"linked artifacts:   {exact_artifacts}/{current_artifacts} exact, "
        f"{stale_artifacts} missing/stale"
    )
    print("disc reconstruction: not implied by possession of the retail oracle")
    return 0


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        description="CTR byte-matching pipeline; exact bytes are the acceptance oracle."
    )
    parser.add_argument("--manifest", default=str(DEFAULT_MANIFEST))
    subparsers = parser.add_subparsers(dest="command", required=True)

    toolchain = subparsers.add_parser(
        "toolchain", help="verify the vendored matching tools"
    )
    toolchain.set_defaults(func=cmd_toolchain)

    verify = subparsers.add_parser("verify", help="verify the retail oracle")
    verify.add_argument("--reference-root")
    verify.add_argument("--artifact", action="append")
    verify.set_defaults(func=cmd_verify)

    compare = subparsers.add_parser(
        "compare", help="compare reconstructed artifacts with retail"
    )
    compare.add_argument("candidate_root")
    compare.add_argument("--reference-root")
    compare.add_argument("--artifact", action="append")
    compare.add_argument("--report")
    compare.set_defaults(func=cmd_compare)

    check = subparsers.add_parser(
        "check", help="verify the toolchain, retail inputs, and source corpus"
    )
    check.add_argument("--aspsx-version")
    check.add_argument("--reference-root")
    check.set_defaults(func=cmd_check)

    probe = subparsers.add_parser(
        "probe", help="compile one source probe and compare its retail bytes"
    )
    probe.add_argument("symbol")
    probe.add_argument("--aspsx-version")
    probe.add_argument("--reference-root")
    probe.add_argument("--optimization", choices=("0", "1", "2", "3"))
    probe.set_defaults(func=cmd_probe)

    artifact = subparsers.add_parser(
        "artifact", help="build and compare one complete linked artifact"
    )
    artifact.add_argument("artifact")
    artifact.add_argument("--reference-root")
    artifact.set_defaults(func=cmd_artifact)

    matrix = subparsers.add_parser(
        "matrix", help="run an ASPSX/optimization matrix with vendored GCC"
    )
    matrix.add_argument("symbol")
    matrix.add_argument("--reference-root")
    matrix.add_argument(
        "--optimization", action="append", choices=("0", "1", "2", "3")
    )
    matrix.add_argument("--all-optimizations", action="store_true")
    matrix.set_defaults(func=cmd_matrix)

    status = subparsers.add_parser(
        "status", help="show evidence without upgrading unproven progress"
    )
    status.set_defaults(func=cmd_status)

    return parser


def main() -> int:
    try:
        args = build_parser().parse_args()
        return args.func(args)
    except MatchError as exc:
        print(f"ERROR: {exc}", file=sys.stderr)
        return 2
    except subprocess.CalledProcessError as exc:
        print(f"ERROR: command failed with exit code {exc.returncode}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
