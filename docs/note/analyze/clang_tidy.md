## 当前 Python 命令

当前程序使用 `clang-tidy` 的主 Python 入口命令是：

```bash
python tools/run.py tidy --app workout_calculator
```

可选常用参数：

```bash
python tools/run.py tidy --app workout_calculator --jobs 16 --parse-workers 8
python tools/run.py tidy --app workout_calculator --keep-going
python tools/run.py tidy --app workout_calculator --no-keep-going
```

此外，当前工具链里还提供了围绕 `clang-tidy` 的扩展 Python 命令：

```bash
python tools/run.py tidy-split --app workout_calculator
python tools/run.py tidy-fix --app workout_calculator
python tools/run.py tidy-flow --app workout_calculator
python tools/run.py tidy-loop --app workout_calculator
```

## `tidy` 主流程如何工作

`python tools/run.py tidy --app workout_calculator` 会进入 `TidyCommand.execute(...)`。

执行逻辑如下：

1. 使用 `build/build_tidy` 作为默认 clang-tidy 构建目录
2. 如果 `build/build_tidy/CMakeCache.txt` 不存在，会先自动执行：

```bash
python tools/run.py configure --app workout_calculator --tidy
```

3. `configure --tidy` 内部会把以下 CMake 选项加进去：

```text
-D ENABLE_CLANG_TIDY=ON
-D ENABLE_PCH=OFF
```

4. 然后执行：

```bash
cmake --build build/build_tidy --target tidy -j
```

5. 如果启用了 keep-going，还会在 Ninja 后面追加：

```text
-- -k 0
```

6. 构建日志会落到：

```text
build/build_tidy/build.log
```

7. 随后 Python 逻辑会继续解析 `build.log`，并把诊断拆分到：

```text
build/build_tidy/tasks/
```

## CMake 侧真正的 clang-tidy 调用

`tidy` target 定义在 `apps/cmake/TargetSetup.cmake`：

```cmake
add_custom_target(
    tidy
    COMMAND ${CLANG_TIDY} -p ${CMAKE_BINARY_DIR} ${SOURCE_LIST}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Running clang-tidy on all source files..."
)
```

也就是说，Python 命令最终不是直接调 `clang-tidy`，而是：

1. 通过 `tools/run.py` 进入 Python 工具链
2. 通过 `cmake --build build/build_tidy --target tidy` 调用 CMake
3. 再由 CMake target 执行：

```bash
clang-tidy -p <build_dir> <source_list>
```

其中 `-p ${CMAKE_BINARY_DIR}` 依赖构建目录里的 `compile_commands.json`。

## `tidy-fix` 的当前入口

当前还有一个修复型入口：

```bash
python tools/run.py tidy-fix --app workout_calculator
```

它默认使用同一个 `build/build_tidy`，并执行：

```bash
cmake --build build/build_tidy --target tidy-fix -j
```

如果传 `--limit N`，则会改为：

```text
tidy_fix_step_N
```

## 配置来源

`tools/config.toml` 中与当前 clang-tidy 行为直接相关的配置包括：

* `build.build_root = "build"`
* `tidy.jobs`
* `tidy.parse_workers`
* `tidy.keep_going`
* `tidy.max_lines`
* `tidy.max_diags`
* `tidy.batch_size`
* `tidy.run_fix_before_tidy`
* `tidy.fix_limit`

## 当前结论

当前仓库里 `clang-tidy` 的标准 Python 用法是：

```bash
python tools/run.py tidy --app workout_calculator
```

如果要做自动修复链路，则使用：

```bash
python tools/run.py tidy-fix --app workout_calculator
```

如果要走“生成任务 + 修复流”的完整链路，则使用：

```bash
python tools/run.py tidy-flow --app workout_calculator
```
