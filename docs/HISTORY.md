## [v0.2.0] - 2026-02-03

### Added
*   新增 **PR 追踪功能**: 通过 `pr` 命令查询各练习的历史最高重量（PR）。
*   新增 **1RM 预估功能**: 在 PR 查询结果中，利用 **Epley** 与 **Brzycki** 公式自动计算并展示每组的预估 1RM 值。
*   新增 **训练周期查询**: 通过 `cycles` 命令列出数据库中的所有训练周期及其基本信息，并支持展示每个周期内包含的所有练习类型（如 `[pull, push, squat]`）。
*   新增 **练习列表查询**: 通过 `list` 命令列出所有练习，支持 `--type` 参数进行分类筛选。
*   新增 **容量与运动学分析**: 通过 `volume` 命令查询指定周期内特定练习类型的深度统计，包括总容量、日均容量、**平均强度**、**训练频率**、**训练密度**及基于次数区间的**强度分布**。
*   新增 **强化报告系统**:
    *   自动生成 `Summary.md` (名人堂)，汇总所有动作的 PR 记录及预估 1RM。
    *   在周期报告顶部增加 **运动学仪表盘 (Kinematics Dashboard)**。
    *   在训练日志中增加 **组级容量计算** 与 **实时预估 1RM**。
*   支持 **多样化备注**:
    *   支持日级别备注（`r` + 空格）。
    *   支持活动（Project）与组（Set）级别备注（使用 `//`, `#`, `;` 符号）。
    *   备注信息贯穿 **Validator**、**Converter**、**Database**、**Serializer** 及 **Markdown** 导出完整全链路。

### Changed/Refactor
*   优化 **CLI 帮助界面**: 将命令按功能归类为 `Analysis & Query`、`Project Tools`、`Storage & Output`，提升易用性。
*   重构 **Parameter Handling**: 在 `db_manager.cpp` 和 `markdown_formatter.cpp` 中引入参数结构体，解决 `bugprone-easily-swappable-parameters` 警告。
*   代码风格同步: 将部分命名从 `snake_case` 统一为 **PascalCase**，并全面启用 **C++23 Designated Initializers**。

### Fixed
*   修复了多个核心模块中的 `clang-tidy` 警告，包括幻数（Magic Numbers）、隐式布尔转换及标识符命名规范。


## [v0.1.1] - 2026-01-31

### Changed/Refactor
*   重构 **JSON 引擎**: 使用 `cJSON` 替代原有的 JSON 处理库，提升性能。


## [v0.1.0] - 2026-01-20

### Added
*   初始版本发布: 支持按照训练周期 (Cycle) 存储和导出训练数据。
