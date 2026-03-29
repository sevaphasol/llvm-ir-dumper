#!/usr/bin/env python3

import argparse
import shutil
import subprocess
import sys
from dataclasses import dataclass, field
from pathlib import Path
from typing import Iterable


def default_tool_path(*relative_candidates: str) -> Path:
    script_path = Path(__file__).resolve()

    for root in script_path.parents:
        for candidate in relative_candidates:
            path = root / candidate
            if path.exists():
                return path

    if len(script_path.parents) >= 3:
        fallback_root = script_path.parents[2]
    else:
        fallback_root = script_path.parent

    return fallback_root / relative_candidates[0]


DEFAULT_PLUGIN_PATH = default_tool_path(
    "install/lib/libgraphcc-llvm-pass.so", "lib/libgraphcc-llvm-pass.so"
)
DEFAULT_CONVERTER_PATH = default_tool_path(
    "install/bin/ir_graph_to_dot", "bin/ir_graph_to_dot"
)
DEFAULT_PROFILE_TOOL_PATH = default_tool_path(
    "install/bin/ir_graph_merge_with_profile",
    "bin/ir_graph_merge_with_profile",
    "install/bin/ir_graph_profile_merge",
    "bin/ir_graph_profile_merge",
)

STAGE_CHOICES = ("before", "after", "both")
DEFAULT_EMIT_DIR_SENTINEL = "__graphcc_default_emit_dir__"


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
    emit_profile: bool
    json_output_root: Path | None
    dot_output_root: Path | None
    ll_output_root: Path | None
    profile_output_root: Path | None
    static_svg_output_root: Path | None
    dynamic_svg_output_root: Path | None
    render_svg: bool
    dynamic_svg_out: Path | None
    program_args: tuple[str, ...]
    extra_clang_args: tuple[str, ...]
    keep_tmp: bool
    delete_bins: bool
    debug: bool
    verbose: bool


@dataclass(frozen=True)
class DriverBehavior:
    source_path: Path
    workdir_base: Path
    plugin_path: Path
    converter_path: Path
    profile_tool_path: Path
    compiler: str
    opt_levels: tuple[str, ...]
    static_stages: tuple[str, ...]
    dynamic_stages: tuple[str, ...]
    emit_static_graph: bool
    emit_dynamic_graph: bool
    emit_json: bool
    emit_dot: bool
    emit_ll: bool
    emit_profile: bool
    json_output_dir: Path | None
    dot_output_dir: Path | None
    ll_output_dir: Path | None
    profile_output_dir: Path | None
    static_svg_output_dir: Path | None
    dynamic_svg_output_dir: Path | None
    render_svg: bool
    dynamic_svg_out: Path | None
    program_args: tuple[str, ...]
    extra_clang_args: tuple[str, ...]
    keep_tmp: bool
    delete_bins: bool
    debug: bool
    verbose: bool


@dataclass
class RunArtifacts:
    preserved_paths: set[Path] = field(default_factory=set)
    binary_paths: set[Path] = field(default_factory=set)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        prog="graphcc",
        description=(
            "Compile a C/C++ source file with the LLVM IR dumper plugin and emit static or "
            "dynamic CFG/DFG graph artifacts."
        ),
    )

    parser.add_argument("source", help="Path to the input source file.")
    parser.add_argument(
        "--workdir",
        default=".",
        help="Base directory for resolving the source and creating the output tree.",
    )
    parser.add_argument(
        "-O",
        "--opt-level",
        action="append",
        default=[],
        metavar="LEVEL",
        help=(
            "Optimization level passed to clang/clang++. Accepts 0/1/2/3/g/s/z/fast or "
            "O0/O1/O2/O3/Og/Os/Oz/Ofast. Can be repeated."
        ),
    )
    parser.add_argument(
        "--plugin-path",
        default=str(DEFAULT_PLUGIN_PATH),
        help="Path to libgraphcc-llvm-pass.so. Defaults to %(default)s.",
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
        nargs="?",
        const=DEFAULT_EMIT_DIR_SENTINEL,
        default=None,
        metavar="DIR",
        help=(
            "Emit static SVG graph(s) for the selected static stage(s). "
            "Optionally specify the destination directory."
        ),
    )
    parser.add_argument(
        "--emit-dynamic-graph",
        nargs="?",
        const=DEFAULT_EMIT_DIR_SENTINEL,
        default=None,
        metavar="DIR",
        help=(
            "Emit dynamic SVG graph(s) for the selected dynamic stage(s). "
            "Optionally specify the destination directory."
        ),
    )
    parser.add_argument(
        "--emit-json",
        nargs="?",
        const=DEFAULT_EMIT_DIR_SENTINEL,
        default=None,
        metavar="DIR",
        help=(
            "Preserve emitted JSON graph artifacts. Optionally specify the "
            "destination directory."
        ),
    )
    parser.add_argument(
        "--emit-dot",
        nargs="?",
        const=DEFAULT_EMIT_DIR_SENTINEL,
        default=None,
        metavar="DIR",
        help=(
            "Preserve emitted DOT graph artifacts. Optionally specify the "
            "destination directory."
        ),
    )
    parser.add_argument(
        "--emit-ll",
        nargs="?",
        const=DEFAULT_EMIT_DIR_SENTINEL,
        default=None,
        metavar="DIR",
        help=(
            "Preserve emitted LLVM IR snapshots. Optionally specify the "
            "destination directory."
        ),
    )
    parser.add_argument(
        "--emit-profile",
        nargs="?",
        const=DEFAULT_EMIT_DIR_SENTINEL,
        default=None,
        metavar="DIR",
        help=(
            "Preserve captured runtime profile logs. Optionally specify the "
            "destination directory."
        ),
    )
    parser.add_argument(
        "--full",
        action="store_true",
        help=(
            "Emit the full artifact set: JSON, DOT, LLVM IR, runtime profile logs, "
            "static graphs and dynamic graphs for both before and after stages."
        ),
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
        help="Do not delete <workdir>/tmp after a successful run.",
    )
    parser.add_argument(
        "--delete-bins",
        action="store_true",
        help="Delete final binaries from <workdir>/bin after a successful run.",
    )
    parser.add_argument(
        "--debug",
        action="store_true",
        help="Keep intermediate and partially emitted artifacts if the driver fails.",
    )
    parser.add_argument(
        "--verbose",
        action="store_true",
        help="Print executed commands and stream tool output to the terminal.",
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


def format_command(command: list[str]) -> str:
    return " ".join(command)


def format_process_failure(
    command: list[str], returncode: int, stdout_text: str | None, stderr_text: str | None
) -> str:
    details = [
        f"command failed with exit code {returncode}: {format_command(command)}"
    ]

    if stdout_text:
        details.append("stdout:")
        details.append(stdout_text.rstrip())

    if stderr_text:
        details.append("stderr:")
        details.append(stderr_text.rstrip())

    return "\n".join(details)


def exec_command(command: list[str], verbose: bool, stdout=None) -> None:
    if verbose:
        print(format_command(command), file=sys.stderr)
        completed = subprocess.run(command, stdout=stdout)
        if completed.returncode != 0:
            raise RuntimeError(
                f"command failed with exit code {completed.returncode}: {format_command(command)}"
            )
        return

    completed = subprocess.run(
        command,
        stdout=stdout if stdout is not None else subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
    )
    if completed.returncode != 0:
        raise RuntimeError(
            format_process_failure(
                command, completed.returncode, completed.stdout, completed.stderr
            )
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


def normalize_opt_level(opt_level: str) -> str:
    normalized = opt_level.strip()
    if not normalized:
        raise ValueError("optimization level cannot be empty")

    if normalized.lower() == "ofast":
        normalized = "fast"
    elif normalized[0] in ("O", "o"):
        normalized = normalized[1:]

    normalized = normalized.lower()

    if normalized not in {"0", "1", "2", "3", "g", "s", "z", "fast"}:
        raise ValueError(
            "unsupported optimization level: "
            f"{opt_level}. Use 0/1/2/3/g/s/z/fast or O0/O1/O2/O3/Og/Os/Oz/Ofast."
        )

    return f"O{normalized}"


def normalize_opt_levels(opt_levels: list[str]) -> tuple[str, ...]:
    if not opt_levels:
        return ("O2",)

    normalized_levels: list[str] = []
    for opt_level in opt_levels:
        normalized = normalize_opt_level(opt_level)
        if normalized not in normalized_levels:
            normalized_levels.append(normalized)

    return tuple(normalized_levels)


def resolve_emit_directory(raw_value: str | None) -> Path | None:
    if raw_value is None or raw_value == DEFAULT_EMIT_DIR_SENTINEL:
        return None
    return Path(raw_value).expanduser().resolve()


def resolve_source_path(source_arg: str) -> Path:
    source_path = Path(source_arg).expanduser()
    return source_path.resolve()


def runtime_program_args(args: argparse.Namespace) -> tuple[str, ...]:
    values = list(args.program_arg)
    if args.program_args:
        remainder = (
            args.program_args[1:] if args.program_args[0] == "--" else args.program_args
        )
        values.extend(remainder)
    return tuple(values)


def build_behavior(args: argparse.Namespace) -> DriverBehavior:
    workdir_base = Path(args.workdir).expanduser().resolve()
    source_path = resolve_source_path(args.source)
    plugin_path = Path(args.plugin_path).expanduser().resolve()
    converter_path = Path(args.converter_path).expanduser().resolve()
    profile_tool_path = Path(args.profile_tool_path).expanduser().resolve()
    dynamic_svg_out = Path(args.output).expanduser().resolve() if args.output else None
    opt_levels = normalize_opt_levels(args.opt_level)
    static_svg_output_dir = resolve_emit_directory(args.emit_static_graph)
    dynamic_svg_output_dir = resolve_emit_directory(args.emit_dynamic_graph)
    json_output_dir = resolve_emit_directory(args.emit_json)
    dot_output_dir = resolve_emit_directory(args.emit_dot)
    ll_output_dir = resolve_emit_directory(args.emit_ll)
    profile_output_dir = resolve_emit_directory(args.emit_profile)

    emit_profile = args.emit_profile or args.full
    emit_dynamic_graph = args.emit_dynamic_graph or dynamic_svg_out is not None or args.full
    emit_static_graph = args.emit_static_graph or args.full
    emit_json = args.emit_json or args.full
    emit_dot = args.emit_dot or args.full
    emit_ll = args.emit_ll or args.full
    needs_dynamic_pipeline = emit_dynamic_graph or emit_profile
    if (
        not emit_dynamic_graph
        and not emit_static_graph
        and not emit_profile
        and (emit_json or emit_dot or emit_ll)
    ):
        emit_static_graph = True

    static_stages = ("before", "after") if args.full else (
        expand_stage(args.static_stage) if emit_static_graph else tuple()
    )
    dynamic_stages = ("before", "after") if args.full else (
        expand_stage(args.dynamic_stage) if needs_dynamic_pipeline else tuple()
    )

    if dynamic_svg_out is not None and len(opt_levels) > 1:
        raise ValueError("-o cannot be used with multiple optimization levels")

    if dynamic_svg_out is not None and "after" not in dynamic_stages:
        raise ValueError("-o requires after-stage dynamic output")

    render_svg = not args.no_svg
    if dynamic_svg_out is not None and not render_svg:
        raise ValueError("-o cannot be combined with --no-svg")

    if (
        not emit_static_graph
        and not emit_dynamic_graph
        and not emit_json
        and not emit_dot
        and not emit_ll
        and not emit_profile
    ):
        raise ValueError(
            "nothing to emit: add -o, --emit-static-graph, --emit-dynamic-graph or --emit-*"
        )

    if (
        emit_dynamic_graph
        and not render_svg
        and not emit_json
        and not emit_dot
        and not emit_ll
        and not emit_profile
    ):
        raise ValueError(
            "dynamic graph requested, but --no-svg disables the only output format"
        )

    if (
        emit_static_graph
        and not render_svg
        and not emit_json
        and not emit_dot
        and not emit_ll
        and not emit_profile
    ):
        raise ValueError(
            "static graph requested, but --no-svg disables the only output format"
        )

    return DriverBehavior(
        source_path=source_path,
        workdir_base=workdir_base,
        plugin_path=plugin_path,
        converter_path=converter_path,
        profile_tool_path=profile_tool_path,
        compiler=detect_compiler(source_path, args.compiler),
        opt_levels=opt_levels,
        static_stages=static_stages,
        dynamic_stages=dynamic_stages,
        emit_static_graph=emit_static_graph,
        emit_dynamic_graph=emit_dynamic_graph,
        emit_json=emit_json,
        emit_dot=emit_dot,
        emit_ll=emit_ll,
        emit_profile=emit_profile,
        json_output_dir=json_output_dir,
        dot_output_dir=dot_output_dir,
        ll_output_dir=ll_output_dir,
        profile_output_dir=profile_output_dir,
        static_svg_output_dir=static_svg_output_dir,
        dynamic_svg_output_dir=dynamic_svg_output_dir,
        render_svg=render_svg,
        dynamic_svg_out=dynamic_svg_out,
        program_args=runtime_program_args(args),
        extra_clang_args=tuple(args.extra_clang_arg),
        keep_tmp=args.keep_tmp,
        delete_bins=args.delete_bins,
        debug=args.debug,
        verbose=args.verbose,
    )


def build_configs(behavior: DriverBehavior) -> list[DriverConfig]:
    multiple_opt_levels = len(behavior.opt_levels) > 1
    configs: list[DriverConfig] = []

    def per_config_output_root(custom_root: Path | None, default_subdir: str) -> Path | None:
        if custom_root is None:
            return None
        if multiple_opt_levels:
            return custom_root / opt_level
        return custom_root

    def default_or_custom_root(enabled: bool, custom_root: Path | None, default_subdir: str) -> Path | None:
        if not enabled:
            return None
        if custom_root is None:
            return output_root / default_subdir
        return per_config_output_root(custom_root, default_subdir)

    for opt_level in behavior.opt_levels:
        output_root = (
            behavior.workdir_base / opt_level if multiple_opt_levels else behavior.workdir_base
        )

        configs.append(
            DriverConfig(
                source_path=behavior.source_path,
                workdir_base=behavior.workdir_base,
                output_root=output_root,
                tmp_root=output_root / "tmp",
                binary_out=output_root / "bin" / "out",
                plugin_path=behavior.plugin_path,
                converter_path=behavior.converter_path,
                profile_tool_path=behavior.profile_tool_path,
                compiler=behavior.compiler,
                opt_level=opt_level,
                static_stages=behavior.static_stages,
                dynamic_stages=behavior.dynamic_stages,
                emit_static_graph=behavior.emit_static_graph,
                emit_dynamic_graph=behavior.emit_dynamic_graph,
                emit_json=behavior.emit_json,
                emit_dot=behavior.emit_dot,
                emit_ll=behavior.emit_ll,
                emit_profile=behavior.emit_profile,
                json_output_root=default_or_custom_root(
                    behavior.emit_json, behavior.json_output_dir, "json"
                ),
                dot_output_root=default_or_custom_root(
                    behavior.emit_dot, behavior.dot_output_dir, "dot"
                ),
                ll_output_root=default_or_custom_root(
                    behavior.emit_ll, behavior.ll_output_dir, "llvm_ir"
                ),
                profile_output_root=default_or_custom_root(
                    behavior.emit_profile, behavior.profile_output_dir, "profile"
                ),
                static_svg_output_root=default_or_custom_root(
                    behavior.emit_static_graph, behavior.static_svg_output_dir, "svg"
                ),
                dynamic_svg_output_root=default_or_custom_root(
                    behavior.emit_dynamic_graph, behavior.dynamic_svg_output_dir, "svg"
                ),
                render_svg=behavior.render_svg,
                dynamic_svg_out=behavior.dynamic_svg_out,
                program_args=behavior.program_args,
                extra_clang_args=behavior.extra_clang_args,
                keep_tmp=behavior.keep_tmp,
                delete_bins=behavior.delete_bins,
                debug=behavior.debug,
                verbose=behavior.verbose,
            )
        )

    return configs


def validate_config(config: DriverConfig) -> None:
    require_existing_file(config.source_path, "Source file")
    require_existing_file(config.plugin_path, "Plugin library")
    require_existing_file(config.converter_path, "DOT converter executable")
    if config.emit_dynamic_graph or config.emit_profile:
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
        return config.tmp_root / "bin" / "out"
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
    assert config.ll_output_root is not None
    return config.ll_output_root / suffix


def final_binary_path(config: DriverConfig) -> Path:
    return config.binary_out


def final_json_path(kind: str, config: DriverConfig, stage: str) -> Path:
    assert config.json_output_root is not None
    return config.json_output_root / f"{stage}_{kind}.json"


def final_profile_path(config: DriverConfig, stage: str) -> Path:
    suffix = "before.runtime.log" if stage == "before" else "after.runtime.log"
    assert config.profile_output_root is not None
    return config.profile_output_root / suffix


def final_dot_path(kind: str, config: DriverConfig, stage: str) -> Path:
    assert config.dot_output_root is not None
    return config.dot_output_root / f"{stage}_{kind}.dot"


def final_svg_path(kind: str, config: DriverConfig, stage: str) -> Path:
    output_root = (
        config.static_svg_output_root if kind == "static" else config.dynamic_svg_output_root
    )
    assert output_root is not None
    return output_root / f"{stage}_{kind}.svg"


def copy_artifact(source: Path, destination: Path, artifacts: RunArtifacts | None = None) -> None:
    ensure_parent_dir(destination)
    shutil.copy2(source, destination)
    if artifacts is not None:
        artifacts.preserved_paths.add(destination)


def emit_artifact_to_many(
    source: Path, destinations: Iterable[Path], artifacts: RunArtifacts | None = None
) -> None:
    for destination in destinations:
        copy_artifact(source, destination, artifacts)


def exec_clang(
    config: DriverConfig,
    artifacts: RunArtifacts,
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

    exec_command(command, verbose=config.verbose)
    if variant == "base":
        artifacts.binary_paths.add(binary_out)


def run_instrumented_program(config: DriverConfig, stage: str) -> None:
    runtime_log = tmp_runtime_log_path(config, stage)
    ensure_parent_dir(runtime_log)

    command = [str(compile_binary_path(config, stage)), *config.program_args]
    if config.verbose:
        print(format_command(command), file=sys.stderr)
        with runtime_log.open("w", encoding="utf-8") as output:
            completed = subprocess.run(command, stdout=output)

        if completed.returncode != 0:
            raise RuntimeError(
                f"instrumented program exited with code {completed.returncode}: {format_command(command)}"
            )
        return

    with runtime_log.open("w", encoding="utf-8") as output:
        completed = subprocess.run(command, stdout=output, stderr=subprocess.PIPE, text=True)

    if completed.returncode != 0:
        raise RuntimeError(
            format_process_failure(command, completed.returncode, None, completed.stderr)
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
    exec_command(command, verbose=config.verbose)


def generate_dot(config: DriverConfig, input_json: Path, output_dot: Path) -> None:
    ensure_parent_dir(output_dot)
    command = [
        str(config.converter_path),
        "--input-json",
        str(input_json),
        "--output-dot",
        str(output_dot),
    ]
    exec_command(command, verbose=config.verbose)


def generate_svg(input_dot: Path, output_svg: Path, verbose: bool) -> None:
    ensure_parent_dir(output_svg)
    exec_command(
        ["dot", "-Tsvg", str(input_dot), "-o", str(output_svg)],
        verbose=verbose,
    )


def preserve_ll_artifacts(config: DriverConfig, artifacts: RunArtifacts) -> None:
    if not config.emit_ll:
        return

    stages = sorted(set((*config.static_stages, *config.dynamic_stages)))
    for stage in stages:
        copy_artifact(tmp_ll_path(config, stage), final_ll_path(config, stage), artifacts)


def preserve_binary_artifact(config: DriverConfig, artifacts: RunArtifacts) -> None:
    if config.delete_bins:
        return
    copy_artifact(compile_binary_path(config), final_binary_path(config), artifacts)


def maybe_delete_final_binary(config: DriverConfig) -> None:
    if not config.delete_bins:
        return

    final_binary = final_binary_path(config)
    if final_binary.exists():
        final_binary.unlink()

    final_bin_dir = final_binary.parent
    if final_bin_dir.exists() and not any(final_bin_dir.iterdir()):
        final_bin_dir.rmdir()


def preserve_json_artifacts(config: DriverConfig, artifacts: RunArtifacts) -> None:
    if not config.emit_json:
        return

    for stage in config.static_stages:
        copy_artifact(
            tmp_static_json_path(config, stage),
            final_json_path("static", config, stage),
            artifacts,
        )
    for stage in config.dynamic_stages:
        copy_artifact(
            tmp_dynamic_json_path(config, stage),
            final_json_path("dynamic", config, stage),
            artifacts,
        )


def preserve_profile_artifacts(config: DriverConfig, artifacts: RunArtifacts) -> None:
    if not config.emit_profile:
        return

    for stage in config.dynamic_stages:
        copy_artifact(
            tmp_runtime_log_path(config, stage),
            final_profile_path(config, stage),
            artifacts,
        )


def emit_static_outputs(config: DriverConfig, artifacts: RunArtifacts) -> None:
    for stage in config.static_stages:
        needs_dot = config.emit_dot or (config.emit_static_graph and config.render_svg)
        if not needs_dot:
            continue

        tmp_json = tmp_static_json_path(config, stage)
        tmp_dot = tmp_dot_path("static", config, stage)
        generate_dot(config, tmp_json, tmp_dot)

        if config.emit_dot:
            copy_artifact(tmp_dot, final_dot_path("static", config, stage), artifacts)

        if config.emit_static_graph and config.render_svg:
            tmp_svg = tmp_svg_path("static", config, stage)
            generate_svg(tmp_dot, tmp_svg, config.verbose)
            copy_artifact(tmp_svg, final_svg_path("static", config, stage), artifacts)


def emit_dynamic_outputs(config: DriverConfig, artifacts: RunArtifacts) -> None:
    for stage in config.dynamic_stages:
        needs_dot = config.emit_dot or (config.emit_dynamic_graph and config.render_svg)
        if not needs_dot:
            continue

        tmp_json = tmp_dynamic_json_path(config, stage)
        tmp_dot = tmp_dot_path("dynamic", config, stage)
        generate_dot(config, tmp_json, tmp_dot)

        if config.emit_dot:
            copy_artifact(tmp_dot, final_dot_path("dynamic", config, stage), artifacts)

        if config.emit_dynamic_graph and config.render_svg:
            tmp_svg = tmp_svg_path("dynamic", config, stage)
            generate_svg(tmp_dot, tmp_svg, config.verbose)

            destinations: list[Path] = []
            if config.emit_dynamic_graph and config.dynamic_svg_out is None:
                destinations.append(final_svg_path("dynamic", config, stage))
            if config.dynamic_svg_out is not None and stage == "after":
                destinations.append(config.dynamic_svg_out)

            emit_artifact_to_many(tmp_svg, destinations, artifacts)


def maybe_cleanup_tmp(config: DriverConfig) -> None:
    if config.keep_tmp:
        return
    if config.tmp_root.exists():
        shutil.rmtree(config.tmp_root)


def cleanup_failed_run(config: DriverConfig, artifacts: RunArtifacts) -> None:
    if config.debug:
        return

    if config.tmp_root.exists():
        shutil.rmtree(config.tmp_root)

    for path in sorted(
            artifacts.preserved_paths | artifacts.binary_paths,
        key=lambda value: len(value.parents),
        reverse=True,
    ):
        if path.exists():
            path.unlink()


def main() -> int:
    current_config: DriverConfig | None = None
    current_artifacts = RunArtifacts()
    try:
        args = parse_args()
        behavior = build_behavior(args)

        for config in build_configs(behavior):
            current_config = config
            current_artifacts = RunArtifacts()
            validate_config(config)

            reset_tmp_dir(config.tmp_root)
            exec_clang(config, current_artifacts, variant="base")

            if config.emit_dynamic_graph or config.emit_profile:
                for stage in config.dynamic_stages:
                    exec_clang(
                        config,
                        current_artifacts,
                        variant=stage,
                        inject_before=(stage == "before"),
                        inject_after=(stage == "after"),
                    )
                    run_instrumented_program(config, stage)
                    merge_dynamic_profile(config, stage)

            preserve_binary_artifact(config, current_artifacts)
            preserve_ll_artifacts(config, current_artifacts)
            preserve_json_artifacts(config, current_artifacts)
            preserve_profile_artifacts(config, current_artifacts)
            emit_static_outputs(config, current_artifacts)
            emit_dynamic_outputs(config, current_artifacts)
            maybe_delete_final_binary(config)
            maybe_cleanup_tmp(config)
            current_config = None
            current_artifacts = RunArtifacts()
        return 0
    except Exception as exc:
        if current_config is not None:
            cleanup_failed_run(current_config, current_artifacts)
        print(f"[graphcc] {exc}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
