from __future__ import annotations

import importlib.util
import io
import struct
import sys
import tempfile
import unittest
from contextlib import redirect_stdout
from pathlib import Path
from types import SimpleNamespace
from unittest import mock


MODULE_PATH = Path(__file__).with_name("match.py")
SPEC = importlib.util.spec_from_file_location("ctr_match", MODULE_PATH)
assert SPEC is not None and SPEC.loader is not None
ctr_match = importlib.util.module_from_spec(SPEC)
sys.modules[SPEC.name] = ctr_match
SPEC.loader.exec_module(ctr_match)


class ExtractFunctionTests(unittest.TestCase):
    def test_extracts_only_named_function(self) -> None:
        source = """
void before(void) {}
// Keep this comment outside the generated source.
void target(void)
{
    const char *brace = "}";
    /* { ignored } */
}
void after(void) {}
"""
        self.assertEqual(
            ctr_match.extract_function(source, "target"),
            'void target(void)\n{\n    const char *brace = "}";\n    /* { ignored } */\n}\n',
        )

    def test_rejects_ambiguous_definition(self) -> None:
        with self.assertRaises(ctr_match.MatchError):
            ctr_match.extract_function("void nope(void) {}\n", "target")

    def test_manifest_probes_extract_from_production_source(self) -> None:
        manifest = ctr_match.load_json(ctr_match.DEFAULT_MANIFEST)
        for probe in manifest["compiler_probes"]:
            source = ctr_match.ROOT / probe["source"]
            extracted = ctr_match.extract_function(source.read_text(), probe["symbol"])
            self.assertIn(probe["symbol"], extracted)


class ComparisonTests(unittest.TestCase):
    def test_rejects_unknown_artifact_selection(self) -> None:
        manifest = {"artifacts": [{"id": "exe"}]}
        with self.assertRaises(ctr_match.MatchError):
            ctr_match.parse_selected(manifest, ["typo"])

    def test_first_difference(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            expected = root / "expected.bin"
            actual = root / "actual.bin"
            expected.write_bytes(b"abcdef")
            actual.write_bytes(b"abcxef")
            self.assertEqual(ctr_match.first_file_difference(expected, actual), 3)

    def test_length_difference(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            expected = root / "expected.bin"
            actual = root / "actual.bin"
            expected.write_bytes(b"abcdef")
            actual.write_bytes(b"abc")
            self.assertEqual(ctr_match.first_file_difference(expected, actual), 3)

    def test_exact_files_have_no_difference(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            expected = root / "expected.bin"
            actual = root / "actual.bin"
            expected.write_bytes(b"same")
            actual.write_bytes(b"same")
            self.assertIsNone(ctr_match.first_file_difference(expected, actual))

    def test_range_comparison_counts_same_offset_words(self) -> None:
        expected = bytes.fromhex("01020304 05060708 090a0b0c")
        actual = bytes.fromhex("01020304 ffffffff")
        result = ctr_match.range_comparison(
            expected,
            actual,
            [{"name": "code", "kind": "code", "offset": "0", "size": "0xc"}],
        )[0]
        self.assertEqual(result["matching_bytes"], 4)
        self.assertEqual(result["matching_words"], 1)
        self.assertEqual(result["total_words"], 3)
        self.assertFalse(result["exact"])

    def test_range_comparison_can_align_a_candidate_symbol(self) -> None:
        expected = bytes.fromhex("00000000 01020304")
        actual = bytes.fromhex("01020304")
        result = ctr_match.range_comparison(
            expected,
            actual,
            [
                {
                    "name": "data",
                    "kind": "data",
                    "candidate_symbol": "data",
                    "offset": "0x4",
                    "size": "0x4",
                }
            ],
            {"data": {"offset": 0, "size": 4}},
        )[0]
        self.assertTrue(result["content_exact"])
        self.assertFalse(result["placement_exact"])
        self.assertEqual(result["placement_delta"], -4)


class ToolchainTests(unittest.TestCase):
    def test_manifest_pins_vendored_gcc(self) -> None:
        manifest = ctr_match.load_json(ctr_match.DEFAULT_MANIFEST)
        compiler = manifest["toolchain"]["compiler"]
        self.assertEqual(compiler["version"], "2.8.1")
        self.assertTrue(compiler["directory"].startswith("externals/"))

    def test_require_tool_rejects_wrong_hash(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "tool"
            path.write_bytes(b"not the pinned tool")
            with self.assertRaises(ctr_match.MatchError):
                ctr_match.require_tool(path, "test tool", "0" * 64)

    def test_repository_path_rejects_escape(self) -> None:
        with self.assertRaises(ctr_match.MatchError):
            ctr_match.repository_path("../outside")

    def test_manifest_has_production_overlay_221_build(self) -> None:
        manifest = ctr_match.load_json(ctr_match.DEFAULT_MANIFEST)
        build = ctr_match.artifact_build_by_id(manifest, "221")
        self.assertEqual(build["source"], "game/221.c")
        self.assertEqual(build["aspsx_version"], "2.77")
        self.assertEqual(build["overlay_id"], 5)
        self.assertTrue((ctr_match.ROOT / build["linker_script"]).is_file())
        self.assertEqual(build["linker_script"], "tools/matching/overlay.ld")
        self.assertEqual(
            build["forced_includes"],
            [
                "tools/matching/overlays/221/abi.h",
            ],
        )
        self.assertNotIn(
            "tools/matching/overlays/221/include",
            build["include_directories"],
        )
        self.assertIn("sdata_static", build["symbols"])

        linker = (ctr_match.ROOT / build["linker_script"]).read_text()
        self.assertIn("__overlay_load_address", linker)
        self.assertIn("__overlay_id", linker)
        self.assertNotIn("0x8009f6fc", linker)
        self.assertNotIn("CC_EndEvent_DrawMenu", linker)

        abi = ctr_match.ROOT / build["forced_includes"][0]
        abi_text = abi.read_text()
        self.assertIn("#include <common.h>", abi_text)
        self.assertIn("extern struct GameTracker *cc_gameTracker", abi_text)
        self.assertIn("#define CC_READ_GAME_TRACKER()", abi_text)
        self.assertNotIn(
            "extern struct GameTracker *cc_gameTracker",
            (ctr_match.ROOT / build["source"]).read_text(),
        )
        self.assertEqual(
            set(ctr_match.artifact_build_input_hashes(build)),
            {
                build["source"],
                build["linker_script"],
                *build["forced_includes"],
            },
        )


class CheckTests(unittest.TestCase):
    def test_check_rebuilds_linked_artifacts(self) -> None:
        build = {"artifact": "221"}
        manifest = {
            "compiler_probes": [{"symbol": "probe"}],
            "artifact_builds": [build],
        }
        toolchain = SimpleNamespace(
            compiler_version="2.8.1",
            default_aspsx_version="2.77",
        )
        args = SimpleNamespace(
            manifest="matching.json",
            reference_root=None,
            aspsx_version=None,
        )
        references = Path("/retail")
        probe_result = {
            "exact": True,
            "symbol": "probe",
            "address": "0x80000000",
            "candidate_size": 4,
            "expected_size": 4,
        }
        artifact_result = {"exact": True}

        with (
            mock.patch.object(ctr_match, "load_json", return_value=manifest),
            mock.patch.object(
                ctr_match, "resolve_toolchain", return_value=toolchain
            ),
            mock.patch.object(ctr_match, "print_toolchain"),
            mock.patch.object(
                ctr_match, "reference_root", return_value=references
            ),
            mock.patch.object(
                ctr_match, "verify_references", return_value=[]
            ),
            mock.patch.object(
                ctr_match, "print_verification", return_value=True
            ),
            mock.patch.object(
                ctr_match, "build_probe", return_value=probe_result
            ),
            mock.patch.object(
                ctr_match, "build_artifact", return_value=artifact_result
            ) as build_artifact,
            mock.patch.object(
                ctr_match, "print_artifact_result", return_value=True
            ),
        ):
            with redirect_stdout(io.StringIO()):
                self.assertEqual(ctr_match.cmd_check(args), 0)

        build_artifact.assert_called_once_with(
            manifest, toolchain, build, references
        )


class PsxHeaderTests(unittest.TestCase):
    def test_header_contract(self) -> None:
        artifact = {
            "id": "exe",
            "load_address": "0x80010000",
            "mapped_size": "0x7d800",
            "header": {
                "entrypoint": "0x8007793c",
                "global_pointer": "0",
                "stack_pointer": "0x801ffff0",
            },
        }
        header = bytearray(0x800)
        header[:8] = b"PS-X EXE"
        struct.pack_into("<I", header, 0x10, 0x8007793C)
        struct.pack_into("<I", header, 0x14, 0)
        struct.pack_into("<I", header, 0x18, 0x80010000)
        struct.pack_into("<I", header, 0x1C, 0x7D800)
        struct.pack_into("<I", header, 0x30, 0x801FFFF0)
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "SCUS_944.26"
            path.write_bytes(header)
            self.assertEqual(ctr_match.check_psx_exe_header(path, artifact), [])


if __name__ == "__main__":
    unittest.main()
