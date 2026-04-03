# 重量单位标准化与显示业务逻辑

## 1. 文档目标

本文只描述当前已经完成重构的这一部分业务逻辑：

* 文本输入中的重量单位如何解释
* 系统内部如何做重量标准化
* 数据库应该保存什么语义
* 查询、报表、CLI 应该如何显示重量

这份文档刻意按 `输入层 -> 业务语义层 -> 存储层 -> 展示层` 来写，避免把 IO 细节、计算逻辑和表现逻辑混在一起。

## 2. 适用范围

本次规则适用于：

* `apps/workout_core`
* `apps/workout_io`
* `apps/workout_windows_cli`

本次不改 `Android` 端，但核心侧已经保证 `display_unit` 缺省时按 `original` 处理，所以现有 Android 请求不会因为这次重构而失效。

## 3. 核心设计原则

### 3.1 计算与显示分离

系统统一遵循以下原则：

* 所有计算一律基于 `kg`
* 所有显示在最终输出时再决定使用 `original`、`kg` 或 `lb`

这意味着：

* 训练容量、平均强度、PR 排序、e1RM 估算，都不再直接依赖用户输入时写的是 `kg` 还是 `lb`
* 用户原始输入单位不会丢失，因为展示层仍然可以按原始单位回显

### 3.2 原始语义保留，但不保留原始字面后缀

系统会保留：

* `original_unit`
* `original_weight_value`

系统不会保留：

* 原样单位后缀文本，例如 `l`、`lb`、`lbs` 的字面差异

也就是说：

* `l` / `lb` / `lbs` 进入系统后都规范化为 `lb`

## 4. 输入层业务规则

### 4.1 文本层允许的单位

训练组文本中的重量部分允许：

* 无单位后缀：默认视为 `kg`
* `kg`
* `l`
* `lb`
* `lbs`

单位大小写不敏感。

### 4.2 文本层示例

以下输入都合法：

```txt
+90 4+4+10
+90kg 4+4
+90l 4+4+10
+90lb 4+4
-100lbs 10+10
```

其含义分别为：

* `+90` -> 原始单位 `kg`
* `+90kg` -> 原始单位 `kg`
* `+90l` / `+90lb` / `+90lbs` -> 原始单位 `lb`
* `-100lbs` -> 原始单位 `lb`，原始重量值为 `-100`

### 4.3 负数重量的业务含义

负数重量继续保留“辅助 / 减重”的业务语义，例如：

```txt
pu
-100lbs 10+10
```

这表示该组是辅助型负重，不再拆成单独的 `elastic_band` 语义。

系统只关心：

* 原始输入是一个带符号数值
* 标准化后得到一个带符号的 `weight_kg`

## 5. 业务语义层规则

### 5.1 标准化后的单组模型

每一组训练数据的核心重量语义为：

* `weight_kg`
* `original_unit`
* `original_weight_value`

其中：

* `weight_kg`：标准化后的重量，统一用于计算
* `original_unit`：只允许 `kg` 或 `lb`
* `original_weight_value`：保留原始输入的带符号数值

### 5.2 标准化公式

标准化公式固定为：

* `factor(kg) = 1.0`
* `factor(lb) = 0.45359237`
* `weight_kg = original_weight_value * factor`

例如：

* `+90` -> `weight_kg = 90`
* `+90lb` -> `weight_kg = 40.8233133`
* `-100lbs` -> `weight_kg = -45.359237`

### 5.3 容量与 e1RM 规则

容量与估算逻辑统一基于 `weight_kg`：

* 当 `weight_kg > 0` 时，`volume = weight_kg * reps`
* 当 `weight_kg <= 0` 时，`volume = 0`

因此：

* 辅助型负重不计入容量
* 零负重也不计入容量
* 正数的 `kg` / `lb` 记录都按标准化后的 `kg` 参与容量与 e1RM 计算

## 6. 存储层业务规则

### 6.1 中间 JSON 契约

`convert` / `insert` / `ingest` 所经过的 set JSON 结构固定为：

* `weight_kg`
* `original_unit`
* `original_weight_value`
* `reps`
* `volume`
* `note`

旧字段不再属于当前业务模型：

* `weight`
* `unit`
* `elastic_band`

### 6.2 数据库契约

`training_sets` 现在只保留一个标准重量列，并附加原始显示语义：

* `weight_kg REAL NOT NULL`
* `original_unit TEXT NOT NULL DEFAULT 'kg'`
* `original_weight_value REAL NOT NULL`
* `reps`
* `volume`
* `set_note`

这意味着数据库层的业务原则是：

* 存一份标准计算值
* 存一份原始显示语义
* 不再把辅助重量拆到单独列里

### 6.3 兼容性边界

本次重构明确不处理旧库兼容：

* 不做迁移
* 不做兼容层
* 不做自动重建

如果数据库仍然是旧 schema，运行时报错是允许的行为。

## 7. 查询与展示层业务规则

### 7.1 `display_unit` 的三个合法值

展示层允许的单位策略只有三种：

* `original`
* `kg`
* `lb`

默认值为：

* `original`

### 7.2 单条记录 / 明细记录显示规则

对于单条记录、PR、报表中的具体 set 明细：

* `--unit original` -> 按 `original_unit + original_weight_value` 显示
* `--unit kg` -> 按 `weight_kg` 显示
* `--unit lb` -> 按 `weight_kg` 反算成 `lb` 显示

例如某条记录原始输入是：

```txt
-100lbs 10+10
```

则：

* `original` 显示为 `-100lb`
* `kg` 显示为 `-45.359kg`
* `lb` 显示为 `-100lb`

### 7.3 聚合指标显示规则

对于 `query volume`、报表 dashboard 这类聚合指标：

* `--unit original`：
  * 如果参与聚合的数据原始单位完全一致，则按该单位显示
  * 如果参与聚合的数据混合了 `kg` 和 `lb`，则回退为 `kg`
* `--unit kg` -> 强制统一显示为 `kg`
* `--unit lb` -> 强制统一显示为 `lb`

为支持这件事，聚合查询会额外维护：

* `common_original_unit`

它的含义是：

* 聚合样本原始单位全相同，则为 `kg` 或 `lb`
* 只要混合，就为空

### 7.4 PR 选取规则

PR 查询不再只看一个模糊的 `MAX(weight)`，而是固定为稳定排序规则：

1. 先按 `weight_kg` 最大
2. 相同重量时按 `reps` 最大
3. 仍相同时取最新 `date`

这样可以避免历史 SQL 聚合在同重量场景下选中不稳定记录。

## 8. CLI 暴露范围

本次只有以下命令支持 `--unit`：

* `workout query pr [--unit <original|kg|lb>]`
* `workout query volume --type <type> --cycle <cycle> [--unit <original|kg|lb>]`
* `workout report export [--unit <original|kg|lb>]`

以下命令暂时不涉及单位显示：

* `workout query list`
* `workout query cycles`

## 9. 分层边界总结

### 9.1 输入层职责

输入层只负责：

* 识别合法单位
* 解析出原始数值与 reps
* 把 `l/lb/lbs` 规范化为 `lb`

输入层不负责：

* 决定最终显示单位
* 做聚合统计解释

### 9.2 业务语义层职责

业务语义层只负责：

* 把重量统一标准化为 `weight_kg`
* 定义容量与 e1RM 的计算规则
* 定义负重与辅助型记录的业务含义

### 9.3 存储层职责

存储层只负责：

* 保存标准化后的计算值
* 保存必要的原始显示语义

存储层不负责：

* 决定最终给用户显示成 `kg` 还是 `lb`

### 9.4 展示层职责

展示层只负责：

* 根据 `display_unit` 决定如何回显
* 在聚合场景下决定 `original` 是否需要回退到 `kg`

展示层不应修改底层存储语义。

## 10. 当前结论

这次重构后的业务结论可以概括为一句话：

* 输入允许 `kg/lb` 两套用户习惯，系统内部统一按 `kg` 计算，展示时再按 `original/kg/lb` 回显。

这套规则的好处是：

* 计算口径统一
* 用户原始记录习惯不丢
* CLI / 报表 / 后续 GUI 可以共享同一套展示策略
* 业务逻辑、IO 与表现层边界更清晰

