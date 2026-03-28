#!/usr/bin/env python3

import argparse
import shutil
import subprocess
import sys
from dataclasses import dataclass
from pathlib import Path
from typing import Iterable


def script_root() -> Path:
    return Path(__file__).resolve().parents[1]


TOOL_ROOT = script_root()


def default_tool_path(*relative_candidates: str) -> Path:
    for candidate in relative_candidates:
        path = TOOL_ROOT / candidate
        if path.exists():
            return path
    return TOOL_ROOT / relative_candidates[0]


DEFAULT_PLUGIN_PATH = default_tool_path(
    "install/lib/libLLVMIRDumper.so", "lib/libLLVMIRDumper.so"
)
DEFAULT_CONVERTER_PATH = default_tool_path(
    "install/bin/ir_graph_to_dot", "bin/ir_graph_to_dot"
)
DEFAULT_PROFILE_TOOL_PATH = default_tool_path(
    "install/bin/ir_graph_profile_merge", "bin/ir_graph_profile_merge"
)

STAGE_CHOICES = ("before", "after", "both")


@dataclass(frozen=True)
class DriverConfig:
    source_path: Path
    workdir_base: Path
    output_root: Path
    tmp_root: Path
    binary_out: Path
    plugin_path: Path
    converter_path: Path
    profile_tool_path: Path
    compiler: str
    opt_level: str
    static_stages: tuple[str, ...]
    dynamic_stages: tuple[str, ...]
    emit_static_graph: bool
    emit_dynamic_graph: bool
    emit_json: bool
    emit_dot: bool
    emit_ll: bool
    render_svg: bool
    dynamic_svg_out: Path | None
    program_args: tuple[str, ...]
    extra_clang_args: tuple[str, ...]
    keep_tmp: bool


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        prog="graphcc",
        description=(
            "Compile a C/C++ source file with the LLVM IR dumper plugin and emit static or "
            "dynamic CFG/DFG graph artifacts."
        ),
    )

    parser.add_argument("source", nargs="?", help="Path to the input source file.")
    parser.add_argument(
        "--source", dest="source_flag", help="Compatibility alias for source path."
    )
    parser.add_argument(
        "--workdir",
        default=".",
        help="Base directory for resolving the source and creating the <workdir>/<opt-level> outputs.",
    )
    parser.add_argument(
        "--opt-level",
        default="O2",
        help="Optimization level passed to clang/clang++. Defaults to %(default)s.",
    )
    parser.add_argument(
        "--plugin-path",
        default=str(DEFAULT_PLUGIN_PATH),
        help="Path to libLLVMIRDumper.so. Defaults to %(default)s.",
    )
    parser.add_argument(
        "--converter-path",
        default=str(DEFAULT_CONVERTER_PATH),
        help="Path to ir_graph_to_dot. Defaults to %(default)s.",
    )
    parser.add_argument(
        "--profile-tool-path",
        default=str(DEFAULT_PROFILE_TOOL_PATH),
        help="Path to ir_graph_profile_merge. Defaults to %(default)s.",
    )
    parser.add_argument(
        "--compiler",
        help="Compiler executable to use explicitly. Defaults to clang for .c and clang++ otherwise.",
    )
    parser.add_argument(
        "--emit-static-graph",
        action="store_true",
        help="Emit static SVG graph(s) for the selected static stage(s).",
    )
    parser.add_argument(
        "--emit-dynamic-graph",
        action="store_true",
        help="Emit dynamic SVG graph(s) for the selected dynamic stage(s).",
    )
    parser.add_argument(
        "--emit-json",
        action="store_true",
        help="Preserve emitted JSON graph artifacts under <workdir>/<opt-level>/json.",
    )
    parser.add_argument(
        "--emit-dot",
        action="store_true",
        help="Preserve emitted DOT graph artifacts under <workdir>/<opt-level>/dot.",
    )
    parser.add_argument(
        "--emit-ll",
        action="store_true",
        help="Preserve emitted LLVM IR snapshots under <workdir>/<opt-level>/llvm_ir.",
    )
    parser.add_argument(
        "--static-stage",
        choices=STAGE_CHOICES,
        default="after",
        help="Which static stage(s) to emit: before, after or both. Defaults to %(default)s.",
    )
    parser.add_argument(
        "--dynamic-stage",
        choices=STAGE_CHOICES,
        default="after",
        help="Which dynamic stage(s) to emit: before, after or both. Defaults to %(default)s.",
    )
    parser.add_argument(
        "--program-arg",
        action="append",
        default=[],
        metavar="ARG",
        help="Argument passed to the instrumented program. Can be specified multiple times.",
    )
    parser.add_argument(
        "--program-args",
        nargs=argparse.REMAINDER,
        default=None,
        metavar="ARG",
        help=(
            "Arguments passed to the instrumented program. Place this option last, or use "
            "repeated --program-arg for fully general argv."
        ),
    )
    parser.add_argument(
        "--extra-clang-arg",
        action="append",
        default=[],
        metavar="ARG",
        help="Extra argument forwarded to clang/clang++. Can be specified multiple times.",
    )
    parser.add_argument(
        "--no-svg",
        action="store_true",
        help="Do not render SVG graphs. Useful together with --emit-json/--emit-dot/--emit-ll.",
    )
    parser.add_argument(
        "--keep-tmp",
        action="store_true",
        help="Do not delete <workdir>/<opt-level>/tmp after a successful run.",
    )
    parser.add_argument(
        "-o",
        dest="output",
        help=(
            "Output SVG path for the final after-opt dynamic graph. Equivalent to requesting "
            "--emit-dynamic-graph --dynamic-stage after."
        ),
    )

    return parser.parse_args()


def require_existing_file(path: Path, description: str) -> None:
    if not path.is_file():
        raise FileNotFoundError(f"{description} not found: {path}")


def ensure_parent_dir(path: Path) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)


def ensure_dir(path: Path) -> None:
    path.mkdir(parents=True, exist_ok=True)


def exec_command(command: list[str], stdout=None) -> None:
    print(" ".join(command), file=sys.stderr)
    completed = subprocess.run(command, stdout=stdout)
    if completed.returncode != 0:
        raise RuntimeError(
            f"command failed with exit code {completed.returncode}: {' '.join(command)}"
        )


def expand_stage(stage: str) -> tuple[str, ...]:
    if stage == "both":
        return ("before", "after")
    return (stage,)


def detect_compiler(source_path: Path, explicit_compiler: str | None) -> str:
    if explicit_compiler:
        return explicit_compiler
    if source_path.suffix == ".c":
        return "clang"
    return "clang++"


def resolve_source_path(
    source_arg: str | None, source_flag: str | None, workdir_base: Path
) -> Path:
    source_text = source_arg or source_flag
    if not source_text:
        raise ValueError("source path was not provided")

    source_path = Path(source_text).expanduser()
    if not source_path.is_absolute():
        source_path = workdir_base / source_path
    return source_path.resolve()


def runtime_program_args(args: argparse.Namespace) -> tuple[str, ...]:
    values = list(args.program_arg)
    if args.program_args:
        remainder = (
            args.program_args[1:] if args.program_args[0] == "--" else args.program_args
        )
        values.extend(remainder)
    return tuple(values)


def build_config(args: argparse.Namespace) -> DriverConfig:
    workdir_base = Path(args.workdir).expanduser().resolve()
    source_path = resolve_source_path(args.source, args.source_flag, workdir_base)
    plugin_path = Path(args.plugin_path).expanduser().resolve()
    converter_path = Path(args.converter_path).expanduser().resolve()
    profile_tool_path = Path(args.profile_tool_path).expanduser().resolve()
    dynamic_svg_out = Path(args.output).expanduser().resolve() if args.output else None

    emit_dynamic_graph = args.emit_dynamic_graph or dynamic_svg_out is not None
    emit_static_graph = args.emit_static_graph
    if (
        not emit_dynamic_graph
        and not emit_static_graph
        and (args.emit_json or args.emit_dot or args.emit_ll)
    ):
        emit_static_graph = True

    static_stages = expand_stage(args.static_stage) if emit_static_graph else tuple()
    dynamic_stages = expand_stage(args.dynamic_stage) if emit_dynamic_graph else tuple()

    if dynamic_svg_out is not None and args.dynamic_stage != "after":
        raise ValueError("-o can only be used with '--dynamic-stage after'")

    render_svg = not args.no_svg
    if dynamic_svg_out is not None and not render_svg:
        raise ValueError("-o cannot be combined with --no-svg")

    if (
        not emit_static_graph
        and not emit_dynamic_graph
        and not args.emit_json
        and not args.emit_dot
        and not args.emit_ll
    ):
        raise ValueError(
            "nothing to emit: add -o, --emit-static-graph, --emit-dynamic-graph or --emit-*"
        )

    if (
        emit_dynamic_graph
        and not render_svg
        and not args.emit_json
        and not args.emit_dot
        and not args.emit_ll
    ):
        raise ValueError(
            "dynamic graph requested, but --no-svg disables the only output format"
        )

    if (
        emit_static_graph
        and not render_svg
        and not args.emit_json
        and not args.emit_dot
        and not args.emit_ll
    ):
        raise ValueError(
            "static graph requested, but --no-svg disables the only output format"
        )

    output_root = (
        workdir_base
        if workdir_base.name == args.opt_level
        else workdir_base / args.opt_level
    )
    tmp_root = output_root / "tmp"

    return DriverConfig(
        source_path=source_path,
        workdir_base=workdir_base,
        output_root=output_root,
        tmp_root=tmp_root,
        binary_out=output_root / "out",
        plugin_path=plugin_path,
        converter_path=converter_path,
        profile_tool_path=profile_tool_path,
        compiler=detect_compiler(source_path, args.compiler),
        opt_level=args.opt_level,
        static_stages=static_stages,
        dynamic_stages=dynamic_stages,
        emit_static_graph=emit_static_graph,
        emit_dynamic_graph=emit_dynamic_graph,
        emit_json=args.emit_json,
        emit_dot=args.emit_dot,
        emit_ll=args.emit_ll,
        render_svg=render_svg,
        dynamic_svg_out=dynamic_svg_out,
        program_args=runtime_program_args(args),
        extra_clang_args=tuple(args.extra_clang_arg),
        keep_tmp=args.keep_tmp,
    )


def validate_config(config: DriverConfig) -> None:
    require_existing_file(config.source_path, "Source file")
    require_existing_file(config.plugin_path, "Plugin library")
    require_existing_file(config.converter_path, "DOT converter executable")
    if config.emit_dynamic_graph:
        require_existing_file(config.profile_tool_path, "Profile merge executable")
    needs_graphviz = (
        config.emit_static_graph or config.emit_dynamic_graph
    ) and config.render_svg
    if needs_graphviz and shutil.which("dot") is None:
        raise FileNotFoundError("Graphviz 'dot' executable was not found in PATH.")


def reset_tmp_dir(tmp_root: Path) -> None:
    if tmp_root.exists():
        shutil.rmtree(tmp_root)
    ensure_dir(tmp_root)


def compile_root(config: DriverConfig, variant: str = "base") -> Path:
    if variant == "base":
        return config.tmp_root
    return config.tmp_root / f"compile_{variant}"


def compile_binary_path(config: DriverConfig, variant: str = "base") -> Path:
    if variant == "base":
        return config.binary_out
    return compile_root(config, variant) / "out"


def tmp_ll_path(config: DriverConfig, stage: str, variant: str = "base") -> Path:
    suffix = "before_opt.ll" if stage == "before" else "after_opt.ll"
    return compile_root(config, variant) / suffix


def tmp_static_json_path(
    config: DriverConfig, stage: str, variant: str = "base"
) -> Path:
    suffix = "before_static.json" if stage == "before" else "after_static.json"
    return compile_root(config, variant) / suffix


def tmp_dynamic_json_path(config: DriverConfig, stage: str) -> Path:
    suffix = "before_dynamic.json" if stage == "before" else "after_dynamic.json"
    return config.tmp_root / suffix


def tmp_runtime_log_path(config: DriverConfig, stage: str) -> Path:
    suffix = "before.runtime.log" if stage == "before" else "after.runtime.log"
    return config.tmp_root / suffix


def tmp_dot_path(kind: str, config: DriverConfig, stage: str) -> Path:
    return config.tmp_root / f"{stage}_{kind}.dot"


def tmp_svg_path(kind: str, config: DriverConfig, stage: str) -> Path:
    return config.tmp_root / f"{stage}_{kind}.svg"


def final_ll_path(config: DriverConfig, stage: str) -> Path:
    suffix = "before_opt.ll" if stage == "before" else "after_opt.ll"
    return config.output_root / "llvm_ir" / suffix


def final_json_path(kind: str, config: DriverConfig, stage: str) -> Path:
    return config.output_root / "json" / f"{stage}_{kind}.json"


def final_dot_path(kind: str, config: DriverConfig, stage: str) -> Path:
    return config.output_root / "dot" / f"{stage}_{kind}.dot"


def final_svg_path(kind: str, config: DriverConfig, stage: str) -> Path:
    return config.output_root / "svg" / f"{stage}_{kind}.svg"


def copy_artifact(source: Path, destination: Path) -> None:
    ensure_parent_dir(destination)
    shutil.copy2(source, destination)


def emit_artifact_to_many(source: Path, destinations: Iterable[Path]) -> None:
    for destination in destinations:
        copy_artifact(source, destination)


def exec_clang(
    config: DriverConfig,
    variant: str = "base",
    inject_before: bool = False,
    inject_after: bool = False,
) -> None:
    variant_root = compile_root(config, variant)
    binary_out = compile_binary_path(config, variant)

    ensure_dir(config.output_root)
    ensure_dir(variant_root)
    ensure_parent_dir(binary_out)
    ensure_parent_dir(tmp_ll_path(config, "before", variant))
    ensure_parent_dir(tmp_static_json_path(config, "before", variant))

    command = [
        config.compiler,
        "-Xclang",
        "-load",
        "-Xclang",
        str(config.plugin_path),
        f"-fpass-plugin={config.plugin_path}",
        "-mllvm",
        f"-dumper-pass-json-out-before-opt={tmp_static_json_path(config, 'before', variant)}",
        "-mllvm",
        f"-dumper-pass-json-out-after-opt={tmp_static_json_path(config, 'after', variant)}",
        "-mllvm",
        f"-dumper-pass-ir-out-before-opt={tmp_ll_path(config, 'before', variant)}",
        "-mllvm",
        f"-dumper-pass-ir-out-after-opt={tmp_ll_path(config, 'after', variant)}",
    ]

    if inject_before:
        command.extend(["-mllvm", "-dumper-pass-enable-before-logging-injection"])
    if inject_after:
        command.extend(["-mllvm", "-dumper-pass-enable-after-logging-injection"])

    command.extend(config.extra_clang_args)
    command.extend(
        [
            str(config.source_path),
            f"-{config.opt_level}",
            "-o",
            str(binary_out),
        ]
    )

    exec_command(command)


def run_instrumented_program(config: DriverConfig, stage: str) -> None:
    runtime_log = tmp_runtime_log_path(config, stage)
    ensure_parent_dir(runtime_log)

    command = [str(compile_binary_path(config, stage)), *config.program_args]
    print(" ".join(command), file=sys.stderr)
    with runtime_log.open("w", encoding="utf-8") as output:
        completed = subprocess.run(command, stdout=output)

    if completed.returncode != 0:
        raise RuntimeError(
            f"instrumented program exited with code {completed.returncode}: {' '.join(command)}"
        )


def merge_dynamic_profile(config: DriverConfig, stage: str) -> None:
    command = [
        str(config.profile_tool_path),
        "--input-json",
        str(tmp_static_json_path(config, stage)),
        "--runtime-log",
        str(tmp_runtime_log_path(config, stage)),
        "--output-json",
        str(tmp_dynamic_json_path(config, stage)),
    ]
    exec_command(command)


def generate_dot(config: DriverConfig, input_json: Path, output_dot: Path) -> None:
    ensure_parent_dir(output_dot)
    command = [
        str(config.converter_path),
        "--input-json",
        str(input_json),
        "--output-dot",
        str(output_dot),
    ]
    exec_command(command)


def generate_svg(input_dot: Path, output_svg: Path) -> None:
    ensure_parent_dir(output_svg)
    exec_command(["dot", "-Tsvg", str(input_dot), "-o", str(output_svg)])


def preserve_ll_artifacts(config: DriverConfig) -> None:
    if not config.emit_ll:
        return

    stages = sorted(set((*config.static_stages, *config.dynamic_stages)))
    for stage in stages:
        copy_artifact(tmp_ll_path(config, stage), final_ll_path(config, stage))


def preserve_json_artifacts(config: DriverConfig) -> None:
    if not config.emit_json:
        return

    for stage in config.static_stages:
        copy_artifact(
            tmp_static_json_path(config, stage),
            final_json_path("static", config, stage),
        )
    for stage in config.dynamic_stages:
        copy_artifact(
            tmp_dynamic_json_path(config, stage),
            final_json_path("dynamic", config, stage),
        )


def emit_static_outputs(config: DriverConfig) -> None:
    for stage in config.static_stages:
        needs_dot = config.emit_dot or (config.emit_static_graph and config.render_svg)
        if not needs_dot:
            continue

        tmp_json = tmp_static_json_path(config, stage)
        tmp_dot = tmp_dot_path("static", config, stage)
        generate_dot(config, tmp_json, tmp_dot)

        if config.emit_dot:
            copy_artifact(tmp_dot, final_dot_path("static", config, stage))

        if config.emit_static_graph and config.render_svg:
            tmp_svg = tmp_svg_path("static", config, stage)
            generate_svg(tmp_dot, tmp_svg)
            copy_artifact(tmp_svg, final_svg_path("static", config, stage))


def emit_dynamic_outputs(config: DriverConfig) -> None:
    for stage in config.dynamic_stages:
        needs_dot = config.emit_dot or (config.emit_dynamic_graph and config.render_svg)
        if not needs_dot:
            continue

        tmp_json = tmp_dynamic_json_path(config, stage)
        tmp_dot = tmp_dot_path("dynamic", config, stage)
        generate_dot(config, tmp_json, tmp_dot)

        if config.emit_dot:
            copy_artifact(tmp_dot, final_dot_path("dynamic", config, stage))

        if config.emit_dynamic_graph and config.render_svg:
            tmp_svg = tmp_svg_path("dynamic", config, stage)
            generate_svg(tmp_dot, tmp_svg)

            destinations: list[Path] = []
            if config.emit_dynamic_graph and config.dynamic_svg_out is None:
                destinations.append(final_svg_path("dynamic", config, stage))
            if config.dynamic_svg_out is not None:
                destinations.append(config.dynamic_svg_out)

            emit_artifact_to_many(tmp_svg, destinations)


def maybe_cleanup_tmp(config: DriverConfig) -> None:
    if config.keep_tmp:
        return
    if config.tmp_root.exists():
        shutil.rmtree(config.tmp_root)


def main() -> int:
    try:
        args = parse_args()
        config = build_config(args)
        validate_config(config)

        reset_tmp_dir(config.tmp_root)
        exec_clang(config, variant="base")

        if config.emit_dynamic_graph:
            for stage in config.dynamic_stages:
                exec_clang(
                    config,
                    variant=stage,
                    inject_before=(stage == "before"),
                    inject_after=(stage == "after"),
                )
                run_instrumented_program(config, stage)
                merge_dynamic_profile(config, stage)

        preserve_ll_artifacts(config)
        preserve_json_artifacts(config)
        emit_static_outputs(config)
        emit_dynamic_outputs(config)
        maybe_cleanup_tmp(config)
        return 0
    except Exception as exc:
        print(f"[graphcc] {exc}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
