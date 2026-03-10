#!/usr/bin/env python3

import argparse
import shutil
import subprocess
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[1]
DEFAULT_PLUGIN_PATH = REPO_ROOT / "install" / "lib" / "libLLVMIRDumper.so"
DEFAULT_BINARY_OUTPUT = REPO_ROOT / "build" / "a.out"


def parse_args():
    parser = argparse.ArgumentParser(
        description=(
            "Compile a source file with the LLVM IR dumper plugin and optionally "
            "render the produced dot dumps as SVG."
        )
    )

    parser.add_argument(
        "--source",
        help="Path to the source file to compile.",
        required=True,
    )

    parser.add_argument(
        "--plugin-path",
        default=str(DEFAULT_PLUGIN_PATH),
        help="Path to libLLVMIRDumper.so. Defaults to %(default)s.",
    )

    parser.add_argument(
        "--before-dot",
        help="Path for the before-optimization dot dump.",
        required=True,
    )
    parser.add_argument(
        "--after-dot",
        help="Path for the after-optimization dot dump.",
        required=True,
    )

    parser.add_argument(
        "--generate-svg",
        action="store_true",
        help="Render the before/after dot dumps to SVG via Graphviz dot.",
    )
    parser.add_argument(
        "--before-svg",
        help="Output path for the before-optimization SVG. Defaults to before-dot with .svg.",
    )
    parser.add_argument(
        "--after-svg",
        help="Output path for the after-optimization SVG. Defaults to after-dot with .svg.",
    )

    parser.add_argument(
        "--binary-output",
        default=str(DEFAULT_BINARY_OUTPUT),
        help="Output path for the compiled binary. Defaults to %(default)s.",
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
    if not args.generate_svg and (args.before_svg or args.after_svg):
        raise ValueError("--before-svg and --after-svg require --generate-svg.")

    if args.generate_svg and shutil.which("dot") is None:
        raise FileNotFoundError("Graphviz 'dot' executable was not found in PATH.")


def exec_clang(args):
    plugin_path = Path(args.plugin_path).expanduser()
    source_path = Path(args.source).expanduser()
    binary_output = Path(args.binary_output).expanduser()
    before_dot = Path(args.before_dot).expanduser()
    after_dot = Path(args.after_dot).expanduser()

    require_existing_file(source_path, "Source file")
    require_existing_file(plugin_path, "Plugin library")

    ensure_parent_dir(binary_output)
    ensure_parent_dir(before_dot)
    ensure_parent_dir(after_dot)

    command = [
        "clang",
        "-Xclang",
        "-load",
        "-Xclang",
        str(plugin_path),
        f"-fpass-plugin={plugin_path}",
        "-mllvm",
        f"-dumper-pass-dot-out-before-opt={before_dot}",
        "-mllvm",
        f"-dumper-pass-dot-out-after-opt={after_dot}",
    ]

    command.extend(args.extra_clang_arg)
    command.extend(
        [
            str(source_path),
            f"-{args.opt_level}",
            "-o",
            str(binary_output),
        ]
    )

    exec_command(command)


def generate_svg(dot, svg):
    path_dot = Path(dot).expanduser()
    path_svg = Path(svg).expanduser() if svg else path_dot.with_suffix(".svg")

    require_existing_file(path_dot, "Before dot dump")
    ensure_parent_dir(path_svg)

    exec_command(["dot", "-Tsvg", str(path_dot), "-o", str(path_svg)])


def main():
    args = parse_args()
    validate_args(args)
    exec_clang(args)

    if args.generate_svg:
        generate_svg(args.before_dot, args.before_svg)
        generate_svg(args.after_dot, args.after_svg)

    return 0


if __name__ == "__main__":
    main()
