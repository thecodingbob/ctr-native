from __future__ import annotations

import io
import struct
import sys
import tempfile
import unittest
from contextlib import redirect_stdout
from pathlib import Path
from types import SimpleNamespace
from unittest import mock


sys.path.insert(0, str(Path(__file__).resolve().parent))
import pipeline as ctr_match
import resident as ctr_resident
import match as cli


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
        for probe in ctr_match.configured_targets(manifest, "probe"):
            source = ctr_match.ROOT / probe["source"]
            extracted = ctr_match.extract_function(source.read_text(), probe["symbol"])
            self.assertIn(probe["symbol"], extracted)


class SharedSectionsTests(unittest.TestCase):
    def test_generated_tables_must_agree_with_the_emitted_artifact(self) -> None:
        toolchain = SimpleNamespace(binutils={"objdump": "objdump"})
        with tempfile.TemporaryDirectory() as directory:
            linked = Path(directory) / "overlay.elf"
            for address, table, accepted in ((0x80001004, b"KEY!", True),
                                             (0x80001004, b"BAD!", False),
                                             (0x80001004, b"KEY", False),
                                             (0x80000FFC, b"KEY!", False),
                                             (0x80001008, b"KEY!", False)):
                headers = (
                    "  0 .overlay 00000008 80001000 80001000 00001000 2**2\n"
                    "                  CONTENTS, ALLOC, LOAD, CODE\n"
                    f"  1 .table 00000004 {address:08x} {address:08x} 00002000 2**2\n"
                    "                  CONTENTS, ALLOC, LOAD, READONLY, DATA\n"
                    "  2 .bss 00000004 80001008 80001008 00003000 2**2\n"
                    "                  ALLOC\n"
                    "  3 .comment 00000004 00000000 00000000 00004000 2**0\n"
                    "                  CONTENTS, READONLY\n"
                )
                with self.subTest(address=address, table=table), \
                     mock.patch.object(ctr_match, "command_output", return_value=headers), \
                     mock.patch.object(ctr_match, "extract_binary_section", side_effect=lambda *args: args[-1].write_bytes(table)):
                    if accepted:
                        self.assertEqual(ctr_match.verify_shared_sections(toolchain, linked, ".overlay", 0x80001000, b"HEADKEY!"),
                                         [{"section": ".table", "offset": 4, "size": 4}])
                    else:
                        with self.assertRaises(ctr_match.MatchError):
                            ctr_match.verify_shared_sections(toolchain, linked, ".overlay", 0x80001000, b"HEADKEY!")


class ComparisonTests(unittest.TestCase):
    def test_rejects_unknown_artifact_selection(self) -> None:
        manifest = {"artifacts": [{"id": "exe"}]}
        with self.assertRaises(ctr_match.MatchError):
            cli.parse_selected(manifest, ["typo"])

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

    def test_assembler_tracks_included_sources(self) -> None:
        toolchain = SimpleNamespace(binutils={"as": Path("mips-as")})

        with mock.patch.object(ctr_match, "run_checked") as run_checked:
            ctr_match.assemble_mips_source(
                toolchain,
                Path("renderer.s"),
                Path("renderer.o"),
                0,
                [Path("game/RenderLevel/psx")],
                Path("renderer.d"),
            )

        run_checked.assert_called_once_with(
            [
                "mips-as",
                "-G0",
                "-Igame/RenderLevel/psx",
                "--MD",
                "renderer.d",
                "-o",
                "renderer.o",
                "renderer.s",
            ]
        )


class ResidentNamespaceTests(unittest.TestCase):
    def test_symbol_ranges_cover_the_interval_without_gaps(self) -> None:
        config = {
            "name": "sample",
            "start_symbol": "Veh_First",
            "end_symbol": "AfterVehicle",
            "symbol_prefix": "Veh_",
        }
        symbols = [
            ctr_resident.Symbol("BeforeVehicle", 0x0FF0),
            ctr_resident.Symbol("Veh_First", 0x1000),
            ctr_resident.Symbol("Veh_Second", 0x1020),
            ctr_resident.Symbol("AfterVehicle", 0x1050),
        ]

        ranges = ctr_resident.namespace_symbol_ranges(config, symbols)

        self.assertEqual(
            [(symbol.name, size) for symbol, size in ranges],
            [("Veh_First", 0x20), ("Veh_Second", 0x30)],
        )

    def test_resident_assembly_closes_small_data_before_a_function(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            assembly = Path(directory) / "source.s"
            assembly.write_text(
                "\t.sdata\nvalue:\n\t.word\t1\n"
                "\t.extern\tsdata_static+832, 4\n"
                '\t.section .Veh_Test,"ax",@progbits\n'
            )

            ctr_match.normalize_compiler_directives(assembly)

            self.assertEqual(
                assembly.read_text(),
                "\t.sdata\nvalue:\n\t.word\t1\n\t.text\n"
                '\t.section .Veh_Test,"ax",@progbits\n',
            )

    def test_function_linker_script_keeps_reachable_helpers_nearby(self) -> None:
        function = ctr_resident.FunctionRange(
            name="Veh_Test",
            address=0x80050000,
            size=0x40,
            source="game/Vehicle/Test.c",
        )

        linker = ctr_resident.function_linker_script(
            function,
            [".Veh_Test", ".Veh_Test_Helper", ".Unrelated"],
        )

        self.assertIn(".Veh_Test 0x80050000", linker)
        self.assertEqual(linker.count("*(.Veh_Test)"), 1)
        self.assertIn("*(.Veh_Test_Helper)", linker)
        self.assertIn("*(.Unrelated)", linker)
        self.assertLess(
            linker.index(".Veh_Test 0x80050000"),
            linker.index(".__function_closure"),
        )

    def test_vehicle_inventory_maps_every_retail_symbol_to_production(self) -> None:
        config = ctr_match.target_by_name(
            ctr_match.load_json(ctr_match.DEFAULT_MANIFEST), "vehicle"
        )
        symbols = ctr_resident.parse_symbols(
            ctr_match.repository_path(config["symbol_file"])
        )

        functions = ctr_resident.discover_function_ranges(config, symbols)

        self.assertEqual(len(functions), 129)
        self.assertEqual(functions[0].name, "VehAfterColl_GetSurface")
        self.assertEqual(functions[0].address, 0x80057C44)
        self.assertEqual(functions[-1].name, "VehTurbo_ThTick")
        self.assertEqual(
            functions[-1].address + functions[-1].size,
            0x80069BB0,
        )
        self.assertEqual(len({function.name for function in functions}), 129)
        self.assertEqual(
            len({function.source for function in functions}),
            len(ctr_resident.namespace_sources(config)),
        )


class CheckTests(unittest.TestCase):
    def test_check_includes_every_kind_and_fails_on_any_mismatch(self) -> None:
        targets = [
            {"name": "221", "kind": "artifact"},
            {"name": "vehicle", "kind": "resident"},
            {"name": "probe", "kind": "probe"},
        ]
        manifest = {"targets": targets}
        args = cli.build_parser().parse_args(["check"])
        for failed in (None, "221", "vehicle", "probe"):
            with (
                self.subTest(failed=failed),
                mock.patch.object(cli, "load_json", return_value=manifest),
                mock.patch.object(cli, "resolve_toolchain"),
                mock.patch.object(cli, "print_toolchain"),
                mock.patch.object(cli, "reference_root", return_value=Path("/retail")),
                mock.patch.object(cli, "verify_references", return_value=[]),
                mock.patch.object(
                    cli,
                    "build_target",
                    side_effect=lambda manifest, tools, config, root: {
                        "exact": config["name"] != failed
                    },
                ) as build,
                redirect_stdout(io.StringIO()),
            ):
                self.assertEqual(cli.cmd_check(args), int(failed is not None))
                self.assertEqual(
                    [call.args[2]["name"] for call in build.call_args_list],
                    ["221", "vehicle", "probe"],
                )

    def test_target_inventory_rejects_duplicates_and_unknown_names(self) -> None:
        config = {"name": "vehicle", "kind": "resident"}
        with self.assertRaises(ctr_match.MatchError):
            ctr_match.configured_targets({"targets": [config, config]})
        with self.assertRaises(ctr_match.MatchError):
            ctr_match.target_by_name({"targets": [config]}, "typo")

    def test_existing_build_commands_use_the_same_entry_point(self) -> None:
        for command in ("build", "artifact", "namespace"):
            args = cli.build_parser().parse_args([command, "vehicle"])
            self.assertIs(args.func, cli.cmd_build)
            self.assertEqual(args.target, "vehicle")


class EvidenceTests(unittest.TestCase):
    def test_evidence_expires_when_an_input_or_candidate_changes(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            source = root / "source.c"
            header = root / "header.h"
            candidate = root / "candidate.bin"
            source.write_text("source")
            header.write_text("header")
            candidate.write_bytes(b"code")
            config = {
                "name": "test",
                "kind": "probe",
                "source": "source.c",
                "region": "exe",
                "compiler_flags": [],
            }
            manifest = {
                "target": "test",
                "artifacts": [{"id": "exe", "sha256": "retail"}],
                "toolchain": {
                    "compiler": {"directory": "gcc", "sha256": {}},
                    "maspsx": {
                        "directory": "maspsx",
                        "sha256": {},
                        "aspsx_version": "2.77",
                    },
                    "binutils": {"prefix": "mips-", "sha256": {}},
                },
            }
            tools = SimpleNamespace(
                compiler_banner="2.8.1",
                compiler_hashes={},
                default_aspsx_version="2.77",
                maspsx_commit="pinned",
                maspsx_hashes={},
                assembler_banner="2.40",
                binutils_hashes={},
                config_hash=ctr_match.sha256_json(manifest["toolchain"]),
            )
            with mock.patch.object(ctr_match, "ROOT", root):
                result = {
                    **ctr_match.build_evidence(manifest, tools, config, ["header.h"]),
                    "candidate": "candidate.bin",
                    "candidate_sha256": ctr_match.sha256_file(candidate),
                    "exact": True,
                }
                self.assertTrue(ctr_match.result_is_current(manifest, config, result))
                for path, content in (
                    (source, "source"),
                    (header, "header"),
                    (candidate, "code"),
                ):
                    path.write_text("changed")
                    self.assertFalse(
                        ctr_match.result_is_current(manifest, config, result)
                    )
                    path.write_text(content)
                del result["build_input_sha256"]["source.c"]
                self.assertFalse(ctr_match.result_is_current(manifest, config, result))

    def test_partial_resident_result_is_reported_as_partial(self) -> None:
        config = {"name": "vehicle", "kind": "resident"}
        result = {
            "complete": False,
            "exact": False,
            "selected_exact": True,
            "exact_function_count": 1,
            "function_count": 129,
        }
        args = cli.build_parser().parse_args(["status"])
        output = io.StringIO()
        with (
            mock.patch.object(
                cli,
                "load_json",
                side_effect=[{"target": "test", "targets": [config]}, result],
            ),
            mock.patch.object(cli, "result_path", return_value=Path("result.json")),
            mock.patch.object(cli, "result_is_current", return_value=True),
            redirect_stdout(output),
        ):
            self.assertEqual(cli.cmd_status(args), 0)
        self.assertIn("PARTIAL", output.getvalue())
        self.assertIn("1/129", output.getvalue())
        self.assertNotIn("MATCH", output.getvalue())


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
