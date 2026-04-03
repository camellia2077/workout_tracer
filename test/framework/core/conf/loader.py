# test/conf/loader.py
import re
import sys
from pathlib import Path
from typing import Any, Dict

from .definitions import Cleanup, CLINames, CommandSpec, GlobalConfig, Paths, RunControl
from .schema_validator import validate_suite_schema

# Python 3.11+ 内置 tomllib
if sys.version_info >= (3, 11):
    import tomllib
else:
    try:
        import tomli as tomllib
    except ImportError as error:
        raise ImportError(
            "For Python versions < 3.11, please install 'tomli' using 'pip install tomli'"
        ) from error

def _merge_toml(base, override, key_path=""):
    if isinstance(base, dict) and isinstance(override, dict):
        merged = dict(base)
        for key, value in override.items():
            new_path = f"{key_path}.{key}" if key_path else key
            if key in merged:
                merged[key] = _merge_toml(merged[key], value, new_path)
            else:
                merged[key] = value
        return merged

    if isinstance(base, list) and isinstance(override, list):
        if key_path in {"commands", "command_groups"}:
            return base + override
        return override

    return override

_VAR_PATTERN = re.compile(r"\$\{([a-zA-Z_][a-zA-Z0-9_]*)\}")
_PATH_KEYS = {
    "project_apps_root",
    "project_build_root",
    "source_executables_dir",
    "test_data_path",
    "target_executables_dir",
    "db_dir",
    "export_output_dir",
    "py_output_dir",
    "output_dir_name",
    "processed_json_dir",
}


def _detect_repo_root(config_path: Path) -> Path:
    resolved = config_path.resolve()
    for parent in [resolved.parent, *resolved.parents]:
        if (
            (parent / "scripts" / "run.py").exists()
            and (parent / "test").exists()
        ):
            return parent
        if (parent / "apps").exists() and (parent / "test").exists():
            return parent
    return resolved.parent


def _substitute_variables(value: str, variables: Dict[str, str]) -> str:
    def replace(match):
        key = match.group(1)
        return str(variables.get(key, match.group(0)))
    return _VAR_PATTERN.sub(replace, value)


def _expand_variables(data: Any, variables: Dict[str, str]) -> Any:
    if isinstance(data, dict):
        return {k: _expand_variables(v, variables) for k, v in data.items()}
    if isinstance(data, list):
        return [_expand_variables(item, variables) for item in data]
    if isinstance(data, str):
        return _substitute_variables(data, variables)
    return data


def _resolve_paths_section_for_file(data: dict, base_dir: Path) -> dict:
    paths_data = data.get("paths")
    if not isinstance(paths_data, dict):
        return data

    normalized_paths = dict(paths_data)
    for key, value in paths_data.items():
        if key not in _PATH_KEYS or not isinstance(value, str):
            continue
        value_str = value.strip()
        if not value_str:
            continue
        path_obj = Path(value_str)
        if not path_obj.is_absolute():
            path_obj = (base_dir / path_obj).resolve()
        normalized_paths[key] = str(path_obj)

    updated = dict(data)
    updated["paths"] = normalized_paths
    return updated


def _load_toml_with_includes(config_path: Path, visited=None,
                             variables: Dict[str, str] | None = None) -> dict:
    if visited is None:
        visited = set()
    resolved = config_path.resolve()
    if resolved in visited:
        raise RuntimeError(f"Circular include detected at: {resolved}")
    visited.add(resolved)

    scoped_variables: Dict[str, str] = dict(variables or {})
    scoped_variables["config_dir"] = str(resolved.parent)

    with open(resolved, "rb") as f:
        data = tomllib.load(f)
    data = _expand_variables(data, scoped_variables)

    merged = {}
    includes = data.get("includes", [])
    if isinstance(includes, list):
        for inc in includes:
            inc_path = Path(inc)
            if not inc_path.is_absolute():
                inc_path = resolved.parent / inc_path
            merged = _merge_toml(
                merged,
                _load_toml_with_includes(
                    inc_path,
                    visited,
                    variables=scoped_variables,
                ),
            )

    data_no_includes = {k: v for k, v in data.items() if k != "includes"}
    data_no_includes = _resolve_paths_section_for_file(
        data_no_includes,
        resolved.parent,
    )
    merged = _merge_toml(merged, data_no_includes)
    return merged

def _load_paths(toml_data) -> Paths:
    paths_data = toml_data.get("paths", {})
    paths_inst = Paths()
    default_build_dir = paths_data.get("default_build_dir")
    if isinstance(default_build_dir, str):
        default_build_dir = default_build_dir.strip()
    paths_inst.DEFAULT_BUILD_DIR = default_build_dir or None
    
    # 项目应用程序根目录
    project_apps_root = paths_data.get("project_apps_root")
    paths_inst.PROJECT_APPS_ROOT = Path(project_apps_root) if project_apps_root else None
    project_build_root = paths_data.get("project_build_root")
    paths_inst.PROJECT_BUILD_ROOT = (
        Path(project_build_root) if project_build_root else None
    )

    # 源执行文件目录
    source_exe_dir = paths_data.get("source_executables_dir")
    paths_inst.SOURCE_EXECUTABLES_DIR = Path(source_exe_dir) if source_exe_dir else None
    
    # [新增] 如果提供了 bin_dir（来自 CLI），则最高优先级覆盖
    bin_dir = toml_data.get("_bin_dir")
    build_dir_name = toml_data.get("_build_dir_name")
    if not build_dir_name:
        build_dir_name = paths_inst.DEFAULT_BUILD_DIR
    if bin_dir:
        paths_inst.SOURCE_EXECUTABLES_DIR = Path(bin_dir)
        print(f"  - Binary directory override active: {paths_inst.SOURCE_EXECUTABLES_DIR}")
    else:
        # [新增] 如果提供了 build_dir_name 且有项目根目录，则动态重写
        build_dir_name = toml_data.get("_build_dir_name")
        build_root = paths_inst.PROJECT_BUILD_ROOT or paths_inst.PROJECT_APPS_ROOT
        if build_dir_name and build_root:
            paths_inst.SOURCE_EXECUTABLES_DIR = build_root / build_dir_name / "bin"
            print(f"  - Build Folder override active: Using {build_dir_name}")
    if build_dir_name and not (paths_inst.PROJECT_BUILD_ROOT or paths_inst.PROJECT_APPS_ROOT):
        raise ValueError(
            "Config error: [paths].project_build_root or [paths].project_apps_root is required when using --build-dir."
        )
    if not bin_dir and not build_dir_name:
        raise ValueError(
            "Config error: executable directory is not configured. "
            "Pass --build-dir/--bin-dir, or set [paths].default_build_dir."
        )
    
    # 源数据路径（强制要求）
    test_data_path_str = paths_data.get("test_data_path")
    if not test_data_path_str:
        raise ValueError("Config error: 'test_data_path' is missing in [paths] section.")
    paths_inst.SOURCE_DATA_PATH = Path(test_data_path_str)
    
    # 自动推导逻辑
    paths_inst.TEST_DATA_ROOT = paths_inst.SOURCE_DATA_PATH.parent
    paths_inst.SOURCE_DATA_FOLDER_NAME = paths_inst.SOURCE_DATA_PATH.name
    
    # [修改] 不再强制要求 target_executables_dir 存在，默认为 None
    # 等待 EnvironmentManager 在运行时动态注入临时路径
    target_exe_dir = paths_data.get("target_executables_dir")
    paths_inst.TARGET_EXECUTABLES_DIR = Path(target_exe_dir) if target_exe_dir else None
    
    # 其它路径配置
    db_dir = paths_data.get("db_dir")
    paths_inst.DB_DIR = Path(db_dir) if db_dir else None
    
    export_dir = paths_data.get("export_output_dir")
    paths_inst.EXPORT_OUTPUT_DIR = Path(export_dir) if export_dir else None
    
    py_output_val = paths_data.get("py_output_dir")
    paths_inst.PY_OUTPUT_DIR = Path(py_output_val) if py_output_val else Path.cwd() / "py_output"
    
    output_dir_name = paths_data.get("output_dir_name")
    paths_inst.OUTPUT_DIR_NAME = Path(output_dir_name) if output_dir_name else None

    # 派生目录名称与路径
    paths_inst.PROCESSED_DATA_DIR_NAME = f"Processed_{paths_inst.SOURCE_DATA_FOLDER_NAME}"
    processed_json_dir = paths_data.get("processed_json_dir")
    paths_inst.PROCESSED_JSON_DIR = Path(processed_json_dir) if processed_json_dir else None

    if paths_inst.PROCESSED_JSON_DIR:
        paths_inst.PROCESSED_JSON_PATH = paths_inst.PROCESSED_JSON_DIR
    elif paths_inst.OUTPUT_DIR_NAME:
        paths_inst.PROCESSED_JSON_PATH = paths_inst.OUTPUT_DIR_NAME / paths_inst.PROCESSED_DATA_DIR_NAME
    
    return paths_inst
    
def _load_cli_names(toml_data) -> CLINames:
    cli_names_data = toml_data.get("cli_names", {})
    cli_inst = CLINames()
    cli_inst.EXECUTABLE_CLI_NAME = cli_names_data.get("executable_cli_name")
    cli_inst.EXECUTABLE_APP_NAME = cli_names_data.get("executable_app_name")
    cli_inst.GENERATED_DB_FILE_NAME = cli_names_data.get("generated_db_file_name")
    return cli_inst

def _load_cleanup_params(toml_data) -> Cleanup:
    cleanup_data = toml_data.get("cleanup", {})
    cleanup_inst = Cleanup()
    cleanup_inst.FILES_TO_COPY = cleanup_data.get("files_to_copy", [])
    # [新增] 读取配置，给予默认值以兼容旧代码
    cleanup_inst.FOLDERS_TO_COPY = cleanup_data.get("folders_to_copy", ["config", "plugins"])
    cleanup_inst.DIRECTORIES_TO_CLEAN = cleanup_data.get("directories_to_clean", [])
    return cleanup_inst

def _load_run_control(toml_data) -> RunControl:
    run_control_data = toml_data.get("run_control", {})
    run_inst = RunControl()
    run_inst.ENABLE_ENVIRONMENT_CLEAN = bool(run_control_data.get("enable_environment_clean", True))
    run_inst.ENABLE_ENVIRONMENT_PREPARE = bool(run_control_data.get("enable_environment_prepare", True))
    run_inst.ENABLE_TEST_EXECUTION = bool(run_control_data.get("enable_test_execution", True))
    run_inst.STOP_ON_FAILURE = bool(run_control_data.get("stop_on_failure", True))
    return run_inst

def _normalize_list(value):
    if value is None:
        return []
    if isinstance(value, list):
        return value
    return [value]

def _safe_format(template: str, values: dict) -> str:
    class SafeDict(dict):
        def __missing__(self, key):
            return "{" + key + "}"
    return template.format_map(SafeDict(values))

def _parse_command_item(item: dict) -> CommandSpec:
    name = item.get("name")
    args = _normalize_list(item.get("args"))
    if not name or not args:
        raise ValueError("Each command must define non-empty 'name' and 'args'.")

    return CommandSpec(
        name=name,
        args=[str(a) for a in args],
        stage=item.get("stage", "commands"),
        expect_exit=int(item.get("expect_exit", 0)),
        add_output_dir=bool(item.get("add_output_dir", False)),
        stdin_input=item.get("stdin_input"),
        expect_files=[str(p) for p in _normalize_list(item.get("expect_files"))],
        expect_stdout_contains=[str(s) for s in _normalize_list(item.get("expect_stdout_contains"))],
        expect_stderr_contains=[str(s) for s in _normalize_list(item.get("expect_stderr_contains"))],
        expect_stdout_not_contains=[str(s) for s in _normalize_list(item.get("expect_stdout_not_contains"))],
        expect_stderr_not_contains=[str(s) for s in _normalize_list(item.get("expect_stderr_not_contains"))],
    )

def _expand_command_groups(toml_data) -> list[CommandSpec]:
    groups = toml_data.get("command_groups", [])
    if not isinstance(groups, list):
        return []

    expanded: list[CommandSpec] = []
    for group in groups:
        if not isinstance(group, dict):
            continue

        name = group.get("name", "group")
        stage = group.get("stage", "commands")
        template = _normalize_list(group.get("template") or group.get("args"))
        if not template:
            continue

        matrix = group.get("matrix", {})
        if not isinstance(matrix, dict):
            matrix = {}

        keys = list(matrix.keys())
        value_lists = [_normalize_list(matrix[k]) for k in keys]

        if not keys:
            combos = [()]
        else:
            combos = [[]]
            for values in value_lists:
                combos = [c + [v] for c in combos for v in values]

        name_template = group.get("name_template")
        for combo in combos:
            variables = dict(zip(keys, combo, strict=False))
            args = [_safe_format(str(t), variables) for t in template]

            if name_template:
                cmd_name = _safe_format(str(name_template), variables)
            else:
                suffix = ", ".join(f"{k}={variables[k]}" for k in keys)
                cmd_name = f"{name} ({suffix})" if suffix else name

            expanded.append(CommandSpec(
                name=cmd_name,
                args=args,
                stage=stage,
                expect_exit=int(group.get("expect_exit", 0)),
                add_output_dir=bool(group.get("add_output_dir", False)),
                stdin_input=group.get("stdin_input"),
                expect_files=[_safe_format(str(p), variables) for p in _normalize_list(group.get("expect_files"))],
                expect_stdout_contains=[_safe_format(str(s), variables) for s in _normalize_list(group.get("expect_stdout_contains"))],
                expect_stderr_contains=[_safe_format(str(s), variables) for s in _normalize_list(group.get("expect_stderr_contains"))],
                expect_stdout_not_contains=[_safe_format(str(s), variables) for s in _normalize_list(group.get("expect_stdout_not_contains"))],
                expect_stderr_not_contains=[_safe_format(str(s), variables) for s in _normalize_list(group.get("expect_stderr_not_contains"))],
            ))

    return expanded

def _load_commands(toml_data) -> list[CommandSpec]:
    commands: list[CommandSpec] = []
    for item in toml_data.get("commands", []):
        if isinstance(item, dict):
            commands.append(_parse_command_item(item))

    commands.extend(_expand_command_groups(toml_data))

    return commands

# [修改] 增加 config_path / build_dir_name / bin_dir 参数
def load_config(config_path: Path = None, build_dir_name: str = None,
                bin_dir: str = None) -> GlobalConfig:
    """加载 config.toml 并返回统一的 GlobalConfig 对象。"""
    try:
        # 1. 确定配置文件路径
        if config_path:
            target_path = config_path
        else:
            # 如果没传路径，尝试在当前目录找 (回退兼容)
            target_path = Path("config.toml")

        print(f"Loading config from: {target_path.absolute()}") # 调试用
        repo_root = _detect_repo_root(target_path)
        toml_data = _load_toml_with_includes(
            target_path,
            variables={
                "repo_root": str(repo_root),
                "test_root": str(repo_root / "test"),
            },
        )
        validate_suite_schema(toml_data, target_path.resolve())

        # 将 CLI 覆盖参数注入 toml_data 方便 _load_paths 读取
        if build_dir_name:
            toml_data["_build_dir_name"] = build_dir_name
        if bin_dir:
            toml_data["_bin_dir"] = bin_dir

        paths_cfg = _load_paths(toml_data)

        return GlobalConfig(
            paths=paths_cfg,
            cli_names=_load_cli_names(toml_data),
            cleanup=_load_cleanup_params(toml_data),
            run_control=_load_run_control(toml_data),
            commands=_load_commands(toml_data)
        )
        
    except FileNotFoundError:
        # [修改] 提示信息带上尝试寻找的路径，方便排错
        raise FileNotFoundError(
            f"config.toml not found at: {target_path.absolute()}"
        ) from None
    except Exception as e:
        raise RuntimeError(f"Error loading config.toml: {e}") from e

