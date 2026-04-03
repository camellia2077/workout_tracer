## 当前 Python 命令

当前程序使用的 Python 入口命令是：

```bash
python tools/run.py format --app workout_calculator
```

测试流程在成功后也会自动调用同一条链路：

```bash
python tools/run.py format --app workout_calculator
```

对应实现位置：

* `test/framework/runner/formatting.py`
* `tools/run.py`
* `tools/agent/commands/format.py`

## 实际执行过程

`tools/run.py format --app workout_calculator` 会进入 `FormatCommand.execute(...)`。

执行逻辑如下：

1. 默认使用构建目录 `build/build_agent`
2. 如果 `build/build_agent/CMakeCache.txt` 不存在，会先自动执行一次：

```bash
python tools/run.py configure --app workout_calculator
```

3. 然后执行 CMake target：

```bash
cmake --build build/build_agent --target format
```

## CMake 侧真正的 clang-format 调用

`format` target 定义在 `apps/cmake/TargetSetup.cmake`：

```cmake
add_custom_target(
    format
    COMMAND ${CLANG_FORMAT} -i ${SOURCE_LIST}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Formatting all source files..."
)
```

也就是说，Python 命令最终不是直接调 `clang-format`，而是：

1. 通过 `tools/run.py` 进入 Python 工具链
2. 通过 `cmake --build ... --target format` 调用 CMake
3. 再由 CMake target 执行 `clang-format -i ${SOURCE_LIST}`

## 当前结论

当前仓库里 `clang-format` 的标准 Python 用法是：

```bash
python tools/run.py format --app workout_calculator
```

如果是在测试成功后触发格式化，则仍然是同一条命令链，只是由测试 runner 自动调用。
