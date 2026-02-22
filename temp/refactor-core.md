# main/src 核心跨端复用重构计划（Android 优先）

## 目标（按你的目的重述）
- 让 `core` 在 Windows CLI 与 Android 复用。
- Android 端只实现 `表现层 + IO 层`，不复制业务规则。
- 核心依赖方向固定为：`presentation -> io adapter -> core(application + domain)`。
- 传输策略固定为：`表现层边界传 JSON`，`非表现层边界使用 C ABI`。

## 相对当前文档，必须调整的点
1. `core` 边界要更硬：禁止任何平台/IO 依赖进入核心。
- 禁止进入 `core`：`windows.h`、`std::filesystem`、`sqlite3`、`cJSON`、`std::cout/std::cerr`。
- 这些都应该留在 `infrastructure` 或 `presentation`。

2. 去掉 `IConsole` 端口。
- 现在文档里把 `IConsole` 放在 application ports，这会把表现层概念带进核心。
- 改为：use case 返回结构化 `Result/DTO`，由 CLI/Android 各自渲染。

3. 数据传输协议分层固定。
- `presentation <-> adapter`：JSON（便于 Android/CLI 一致接入）。
- `adapter <-> core`：C ABI（`extern \"C\"` 导出函数 + C 风格数据结构）。
- 文件读取、目录扫描、写文件都在平台 IO 层完成。

4. `core` 产物应是可被 Android NDK 链接的库。
- CMake 目标新增 `workout_core`（static/shared）。
- Windows CLI 与 Android JNI 都链接同一个 `workout_core`。

5. 报表与查询分层要再细化。
- 统计公式（1RM、频率、密度、分布）进 `domain/application`。
- Markdown 格式化、SQLite SQL、文件导出路径策略留在平台 IO 层。

## 目标分层（建议）
- `main/src/core/domain`：实体、值对象、纯计算服务。
- `main/src/core/application`：use case、ports、request/response DTO、error model。
- `main/src/platform/windows_cli`：命令解析、console 展示、Windows 特定初始化。
- `main/src/platform/windows_infra`：文件、SQLite、JSON、Markdown 适配器。
- `main/src/platform/android_bridge`：JNI/Kotlin 调用桥。
- `main/src/platform/android_infra`：Android 端 IO 适配（如 Room/文件访问实现）。

## 数据传输约定（新增）
- 表现层输入：JSON Request（命令参数或 UI 输入最终映射为 JSON）。
- 进入核心前：由 adapter 将 JSON 反序列化为 C ABI 请求结构。
- 核心对外：仅暴露 C ABI 接口（函数、错误码、句柄、POD 结构）。
- 核心内部：可以使用 C++ DTO/实体实现，但不直接暴露给平台层。
- 核心输出：C ABI 响应结构，由 adapter 序列化为 JSON Response 提供给表现层。
- 禁止：在 core 直接依赖 JSON 库类型（如 `cJSON*`）。

---

## 进度状态
- Phase 0：`[TODO]`
- Phase 1：`[DONE]`（1.1 / 1.2 / 1.3 已落地）
- Phase 2：`[DONE]`（2.1 / 2.2 已完成，路径耦合已剥离到 JSON->ABI adapter）
- Phase 3：`[DONE]`（3.1 / 3.2 已完成）
- Phase 4：`[IN_PROGRESS]`（目录收口已完成：`apps/workout_core/src`、`apps/workout_windows_cli/src`）
- Phase 5：`[TODO]`

---

## Phase 0 - 先锁定 Core 边界
### Step 0.1 定义 In/Out Scope
- In scope（核心）：解析规则、校验规则、容量/1RM/统计公式、用例编排。
- Out scope（平台）：文件系统、数据库、JSON 库、控制台打印、Markdown 输出。

### Step 0.2 加边界检查
- 在 CI 或脚本里检查 `core` include，发现 `sqlite3/cJSON/filesystem/windows.h/iostream` 直接失败。

**完成标准**
- `core` 边界规则可自动验证。

---

## Phase 1 - 搭建可复用的 `workout_core`（已完成）
### Step 1.1 新建 core target（已完成）
- CMake 增加 `workout_core`，只编译 `core/domain + core/application`。

### Step 1.2 定义 ports（仅业务需要）（已完成）
- 保留/新增：`IMappingProvider`、`IWorkoutRepository`、`IReportExporter`、`IClock`（如需要）。
- 移除：`IConsole`。

### Step 1.3 统一返回模型（已完成）
- 定义 `UseCaseResult<T>`（核心内部）与对应 C ABI 结果结构（对外）。
- `AppExitCode` 只在 CLI 层映射，不进入核心。
- JSON 序列化仅发生在 platform adapter。

**完成标准**
- `workout_core` 不链接 `sqlite3/cJSON`，可独立编译。

**完成记录（代码）**
- Core target 与链接：`main/CMakeLists.txt`、`main/cmake/SourceFileCollection.cmake`
- Ports：`main/src/application/interfaces/i_workout_repository.hpp`、`main/src/application/interfaces/i_report_exporter.hpp`、`main/src/application/interfaces/i_clock.hpp`、`main/src/application/interfaces/workout_query_models.hpp`
- Result/C ABI：`main/src/core/application/use_case_result.hpp`、`main/src/core/application/core_error_code.hpp`、`main/src/core/abi/workout_core_abi.h`、`main/src/core/abi/workout_core_abi.cpp`
- CLI 映射骨架：`main/src/cli/framework/core_status_mapper.hpp`、`main/src/cli/framework/core_status_mapper.cpp`

---

## Phase 2 - 用例 API 改为平台无关（已完成）
### Step 2.1 Request/Response DTO 改造（已完成）
- `Validate/Convert/Ingest/Query...` 全部先定义 C ABI request/response 契约，避免路径耦合。
- 表现层先提交 JSON，再由 adapter 转为 C ABI request（例如 `validate_request_t`）。

### Step 2.2 Dispatcher 与命令映射解耦（已完成）
- CLI 命令只做参数解析和 JSON/ABI 组装。
- 核心通过 C ABI 被调用，不感知命令行。

**完成标准**
- 核心 API 可直接被 JNI 调用（不需要伪造文件路径），且 `adapter <-> core` 边界统一为 C ABI。

**阶段记录（已落地代码）**
- C ABI request/execute/delegate：`main/src/core/abi/workout_core_abi.h`、`main/src/core/abi/workout_core_abi.cpp`
- CLI->C ABI 请求映射：`main/src/cli/framework/core_request_mapper.hpp`、`main/src/cli/framework/core_request_mapper.cpp`
- ActionHandler C ABI 桥接：`main/src/application/action_handler_abi_bridge.hpp`、`main/src/application/action_handler_abi_bridge.cpp`
- CLI 主流程改为 C ABI 执行：`main/src/main_cli.cpp`
- 路径字段从 `workout_core_request_t` 移除，改为 `request_json_utf8` 通道；路径/筛选参数改由 JSON adapter 编解码。

---

## Phase 3 - 迁移业务逻辑回 Core（已完成）
### Step 3.1 统一公式实现（已完成）
- 把 `QueryFacade/MarkdownFormatter` 里的重复计算迁回 domain service。

### Step 3.2 解析与校验纯函数化（已完成）
- 解析器/校验器基于字符串或流输入，不依赖具体文件实现。

**完成标准**
- 相同输入在 Windows/Android 得到一致业务结果。

**阶段记录（已落地代码）**
- 新增统一指标域服务：`main/src/domain/services/training_metrics_service.hpp`、`main/src/domain/services/training_metrics_service.cpp`
- 统一公式调用（PR/e1RM/频率/密度/分布）：
  - `main/src/infrastructure/persistence/facade/query_facade.cpp`
  - `main/src/infrastructure/reporting/database/database_manager.cpp`
  - `main/src/infrastructure/reporting/formatter/markdown_formatter.cpp`
  - `main/src/application/database_handler.cpp`
- 解析入口纯函数化（支持文本解析）：
  - `main/src/application/interfaces/i_log_parser.hpp`（新增 `ParseText`）
  - `main/src/infrastructure/converter/log_parser.hpp`
  - `main/src/infrastructure/converter/log_parser.cpp`（`ParseFile` 与 `ParseText` 复用 `ParseInput(std::istream&)`）

---

## Phase 4 - 平台适配层落地
### Step 4.1 Windows CLI 作为一个适配器
- `main_cli.cpp` 保留 `SetConsoleOutputCP/SetConsoleCP`。
- CLI 仅负责：参数 -> JSON -> C ABI request -> 调用 core -> C ABI response -> JSON -> 输出渲染。

### Step 4.2 Android 适配
- 新增 `android_bridge`（JNI）调用 `workout_core`。
- Android 端实现自己的 IO（文件/数据库）与展示（Compose/XML），与 core 交互边界使用 JSON。

**完成标准**
- 同一条业务路径可在 CLI 与 Android 复用同一核心代码。

**阶段记录（已落地代码）**
- 模块目录已收口：
  - `apps/workout_core/src`（application/common/core/domain/infrastructure）
  - `apps/workout_windows_cli/src`（main_cli/cli/pch）
- 构建路径已同步：
  - `apps/CMakeLists.txt`
  - `apps/cmake/SourceFileCollection.cmake`
  - `apps/cmake/TargetSetup.cmake`
- 脚本与测试路径已切换到 `apps`：
  - `scripts/config.toml`
  - `test/suites/workout_calculator/env.toml`

---

## Phase 5 - 回归与交付
### Step 5.1 双端一致性测试
- 以同一批输入数据对比 core 输出（JSON snapshot 或结构化断言）。

### Step 5.2 性能与稳定性
- 验证 JNI 往返开销可接受。
- 验证异常与错误码在 Kotlin/CLI 两侧都可正确处理。

**完成标准**
- 发布物至少包含：
  - `workout_core`（可被 Android NDK 链接）
  - Windows CLI 可执行程序
  - 双端一致性测试报告

---

## 推荐执行顺序（按风险）
1. 先完成 Phase 0/1（边界和 core target）。
2. 再做 Phase 2/3（API 与业务迁移）。
3. 最后做 Phase 4/5（Android bridge 与双端验证）。
