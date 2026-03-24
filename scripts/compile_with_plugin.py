#!/usr/bin/env python3

import argparse
import shutil
import subprocess
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[1]
DEFAULT_PLUGIN_PATH = REPO_ROOT / "install" / "lib" / "libLLVMIRDumper.so"
DEFAULT_CONVERTER_PATH = REPO_ROOT / "install" / "bin" / "ir_graph_to_dot"
DEFAULT_BINARY_OUT = REPO_ROOT / "build" / "a.out"


def parse_args():
    parser = argparse.ArgumentParser(
        description=(
            "Compile a source file with the LLVM IR dumper plugin, dump the IR graph to JSON, "
            "and convert the JSON graph into dot/SVG."
        )
    )

    parser.add_argument(
        "--workdir",
        default=REPO_ROOT,
        help="Parent directory to source, llvm_ir, json, dot and svg. Defaults to %(default)s",
    )

    parser.add_argument(
        "--source",
        help="Path to the source file to compile.",
        required=True,
    )

    parser.add_argument(
        "--plugin-path",
        default=DEFAULT_PLUGIN_PATH,
        help="Path to libLLVMIRDumper.so. Defaults to %(default)s.",
    )
    parser.add_argument(
        "--converter-path",
        default=DEFAULT_CONVERTER_PATH,
        help="Path to ir_graph_to_dot executable. Defaults to %(default)s.",
    )

    parser.add_argument(
        "--before-ll",
        default="llvm_ir/before_opt.ll",
        help="Output path for the before-optimization llvm ir. Defaults to %(default)s.",
    )
    parser.add_argument(
        "--after-ll",
        default="llvm_ir/after_opt.ll",
        help="Output path for the after-optimization llvm ir. Defaults to %(default)s.",
    )

    parser.add_argument(
        "--before-json",
        default="json/before_opt.json",
        help="Output path for serialized graph before optimization. Defaults to %(default)s.",
    )
    parser.add_argument(
        "--after-json",
        default="json/after_opt.json",
        help="Output path for serialized graph after optimization. Defaults to %(default)s.",
    )

    parser.add_argument(
        "--before-dot",
        default="dot/before_opt.dot",
        help="Output path for the before-optimization dot dump. Defaults to %(default)s.",
    )
    parser.add_argument(
        "--after-dot",
        default="dot/after_opt.dot",
        help="Output path for the after-optimization dot dump. Defaults to %(default)s.",
    )

    parser.add_argument(
        "--no-svg",
        action="store_true",
        help="Do not render the before/after dot dumps to SVG via Graphviz dot.",
    )
    parser.add_argument(
        "--before-svg",
        default="svg/before_opt.svg",
        help="Output path for the before-optimization SVG. Defaults to %(default)s.",
    )
    parser.add_argument(
        "--after-svg",
        default="svg/after_opt.svg",
        help="Output path for the after-optimization SVG. Defaults to %(default)s.",
    )

    parser.add_argument(
        "--binary-out",
        default=DEFAULT_BINARY_OUT,
        help="Output path for the binary out. Defaults to %(default)s.",
    )
    parser.add_argument(
        "--opt-level",
        default="O2",
        help="Optimization level passed to clang. Defaults to %(default)s.",
    )
    parser.add_argument(
        "--extra-clang-arg",
        action="append",
        default=[],
        metavar="ARG",
        help="Extra argument to forward to clang. Can be provided multiple times.",
    )

    return parser.parse_args()


def ensure_parent_dir(path):
    path.parent.mkdir(parents=True, exist_ok=True)


def require_existing_file(path, description):
    if not path.is_file():
        raise FileNotFoundError(f"{description} not found: {path}")


def exec_command(command):
    print(" ".join(command), file=sys.stderr)
    subprocess.run(command, check=True)


def validate_args(args):
    converter_path = Path(args.converter_path).expanduser()
    require_existing_file(converter_path, "Graph converter executable")

    if not args.no_svg and shutil.which("dot") is None:
        raise FileNotFoundError("Graphviz 'dot' executable was not found in PATH.")


def exec_clang(args):
    workdir = Path(args.workdir).expanduser()
    plugin_path = Path(args.plugin_path).expanduser()
    binary_out = workdir / args.binary_out
    source_path = workdir / args.source
    before_json = workdir / args.before_json
    after_json = workdir / args.after_json
    before_ll = workdir / args.before_ll
    after_ll = workdir / args.after_ll

    require_existing_file(source_path, "Source file")
    require_existing_file(plugin_path, "Plugin library")

    ensure_parent_dir(binary_out)
    ensure_parent_dir(before_ll)
    ensure_parent_dir(after_ll)
    ensure_parent_dir(before_json)
    ensure_parent_dir(after_json)

    command = [
        "clang",
        "-Xclang",
        "-load",
        "-Xclang",
        str(plugin_path),
        f"-fpass-plugin={plugin_path}",
        "-mllvm",
        f"-dumper-pass-json-out-before-opt={before_json}",
        "-mllvm",
        f"-dumper-pass-json-out-after-opt={after_json}",
        "-mllvm",
        f"-dumper-pass-ir-out-before-opt={before_ll}",
        "-mllvm",
        f"-dumper-pass-ir-out-after-opt={after_ll}",
    ]

    command.extend(args.extra_clang_arg)
    command.extend(
        [
            str(source_path),
            f"-{args.opt_level}",
            "-o",
            str(binary_out),
        ]
    )

    exec_command(command)


def generate_dot(workdir, converter_path, json_path, dot_path):
    workdir = Path(workdir).expanduser()
    converter_path = Path(converter_path).expanduser()
    path_json = workdir / json_path
    path_dot = workdir / dot_path

    require_existing_file(path_json, "Serialized graph dump")
    require_existing_file(converter_path, "Graph converter executable")
    ensure_parent_dir(path_dot)

    exec_command(
        [
            str(converter_path),
            "--input-json",
            str(path_json),
            "--output-dot",
            str(path_dot),
        ]
    )


def generate_svg(workdir, dot_path, svg_path):
    workdir = Path(workdir).expanduser()
    path_dot = workdir / dot_path
    path_svg = workdir / svg_path

    require_existing_file(path_dot, "Dot dump")
    ensure_parent_dir(path_svg)

    exec_command(["dot", "-Tsvg", str(path_dot), "-o", str(path_svg)])


def main():
    args = parse_args()
    validate_args(args)
    exec_clang(args)
    generate_dot(args.workdir, args.converter_path, args.before_json, args.before_dot)
    generate_dot(args.workdir, args.converter_path, args.after_json, args.after_dot)

    if not args.no_svg:
        generate_svg(args.workdir, args.before_dot, args.before_svg)
        generate_svg(args.workdir, args.after_dot, args.after_svg)

    return 0


if __name__ == "__main__":
    main()
