# workout_calculator Agent Rules

## 执行目录
- 所有命令默认在仓库根目录执行：`workout_calculator/`

## 标准流程（代码改动必须执行）
1. 编译（先配置再构建）
```bash
python scripts/run.py configure --app workout_calculator --build-dir build_agent
python scripts/run.py build --app workout_calculator --build-dir build_agent
```

2. 测试（进入 test 流程执行套件）
```bash
python test/run.py --suite workout_calculator --build-dir build_agent --concise
```

3. 查看测试结果（至少检查以下位置）
- 汇总结果：`test/output/workout_calculator/result.json`
- 运行日志：`test/output/workout_calculator/logs/output.log`
- 产物目录：`test/output/workout_calculator/artifacts/`

## 例外规则
- 如果修改只涉及文档（如 `docs/**`、`*.md`、`.agent/**`），不需要编译和测试。
- 如果是文档+代码混合改动，仍按“标准流程（代码改动必须执行）”处理。
