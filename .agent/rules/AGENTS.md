# workout_calculator Agent Rules

## 执行目录
- 所有命令默认在仓库根目录执行：`workout_calculator/`

## 标准流程（代码改动必须执行）
1. 编译（先配置再构建）
```bash
python tools/run.py configure --app workout_calculator --build-dir build_agent
python tools/run.py build --app workout_calculator --build-dir build_agent
```

2. 测试（进入 test 流程执行套件）
```bash
python test/run.py --suite workout_calculator --build-dir build_agent --concise
```

3. 查看测试结果（至少检查以下位置）
- 汇总结果：`output/tests/workout_calculator/result.json`
- 运行日志：`output/tests/workout_calculator/logs/output.log`
- 产物目录：`output/tests/workout_calculator/artifacts/`

## 例外规则
- 如果修改只涉及文档（如 `docs/**`、`*.md`、`.agent/**`），不需要编译和测试。
- 如果是文档+代码混合改动，仍按“标准流程（代码改动必须执行）”处理。

## CLI / Python 命令约定
- 对于 Python 工具命令，优先使用分层 `--help` 逐层查看说明，不要一次性打印根级全部帮助。
- 推荐顺序：先看主命令 `--help`，再看子命令 `--help`，最后再执行具体命令。
- Android 构建在仓库根目录使用 Python 入口：
```bash
python tools/run.py android assemble-debug
python tools/run.py android assemble-release
python tools/run.py android native-debug
python tools/run.py android native-release
python tools/run.py android test-debug
```
