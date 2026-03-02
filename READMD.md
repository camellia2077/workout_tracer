# fitness_calculator
一个用于处理健身数据的程序

## 目录结构
```
/
├── build.sh
├── CMakeLists.txt
├── main.cpp
├── mapping.json
├── common/
│   ├── JsonReader.cpp
│   ├── JsonReader.hpp
│   └── parsed_data.hpp # 共享的log结构体数据类型
├── db_inserter/
│   ├── SqliteManager.cpp
│   └── SqliteManager.hpp
└── reprocessor # 数据预处理
    ├── date_processor # 补全日期
    │   ├── DateProcessor.cpp
    │   └── DateProcessor.hpp
    ├── log_formatter # 把解析的内容格式化输出
    │   ├── LogFormatter.cpp
    │   └── LogFormatter.hpp
    ├── log_parser # 把log内容解析成结构体 
    │   ├── LogParser.cpp
    │   └── LogParser.hpp
    ├── name_mapper # 映射项目名称
    │   ├── ProjectNameMapper.cpp
    │   └── ProjectNameMapper.hpp
    └── volume_calculator/ 计算容量
    │    ├── VolumeCalculator.cpp
    │    └── VolumeCalculator.hpp
    ├── Reprocessor.cpp # 封装
    └── Reprocessor.hpp
 
```
## 命令行功能
1.  **模式互斥**: `-r`、`-o`、`-p` 现在是互斥的。如果用户同时使用多个，程序会报错。
2.  **默认模式**: 如果用户不指定任何模式（但提供了日志文件），程序将**默认执行 `-r` 的行为**，即同时生成文件并存入数据库。这通常是最符合用户期望的默认行为。
3.  **最高优先级**: `--validate` 选项的优先级最高。即使用户指定了 `-r`，只要同时使用了 `--validate`，程序就只会执行验证。
4.  **清晰的帮助信息**: `-h` 或 `--help` 会打印出清晰、准确的用法说明。



  * **只输出文件**: `workout_tracker_cli my_log.txt -o`
  * **只存入数据库**: `workout_tracker_cli my_log.txt -p`
  * **两者都做 (显式)**: `workout_tracker_cli my_log.txt -r`
  * **两者都做 (默认)**: `workout_tracker_cli my_log.txt`
  * **指定年份并存入数据库**: `workout_tracker_cli my_log.txt -p --year 2023`
  * **仅验证文件**: `workout_tracker_cli my_log.txt --validate`

## 验证规则

### 日志文件格式验证规则

`Validator` 模块用于确保原始日志文件的格式严格遵守规范，以便后续模块能够正确解析。验证过程基于一个状态机，逐行检查文件内容，其核心规则如下：

#### **一、 基本原则**

1.  **文件结构**: 日志文件由一个或多个“日期块”组成。
2.  **日期块结构**: 每个“日期块”以一个四位数的日期开始，内部包含至少一个“项目块”。
3.  **项目块结构**: 每个“项目块”由一个“标题行”和一个紧随其后的“内容行”组成。
4.  **空行**: 验证器会自动忽略所有空行或仅包含空白字符的行。

#### **二、 行级格式详解**

1.  **日期行 (Date Line)**
    * **格式**: 必须是**四位连续的数字**，例如 `0704` 或 `1120`。
    * **位置**:
        * 必须是文件的第一个非空行。
        * 可以在一个完整的内容行之后出现，标志着前一个日期块的结束和新日期块的开始。

2.  **标题行 (Title Line)**
    * **格式**: 必须完全由**小写英文字母**组成，例如 `bp` 或 `squat`。
    * **位置**: 必须紧跟在一个“日期行”或另一个“内容行”之后。

3.  **内容行 (Content Line)**
    * **格式**: 必须严格遵循 `+重量<空格>数量` 的格式。
        * **重量**: 由 `+` 号和至少一个数字组成（例如 `+60`）。`+` 和数字之间不能有空格。
        * **数量**: 由一个或多个数字组成，数字之间必须用 `+` 分隔（例如 `10+10+9` 或 `5`）。
        * **分隔**: 重量和数量之间必须有**一个或多个空格**。
    * **位置**: 必须紧跟在一个“标题行”之后。

#### **三、 常见错误场景**

验证器会捕捉并报告以下具体错误：

* **文件开头非日期**: 如果文件的第一个有效行不是四位数字的日期格式，验证将失败。
* **日期后非标题**: 在一个日期行之后，如果紧接着的不是一个标题行（小写字母），验证将失败。
* **标题后非内容**: 在一个标题行之后，如果紧接着的不是一个符合 `+重量 数量` 格式的内容行，验证将失败。
* **内容行后格式错误**: 在一个内容行之后，如果紧接着的既不是一个新的标题行，也不是一个新的日期行，验证将失败。
* **文件意外结束**: 如果文件的最后一行是一个标题行，但其后缺少对应的内容行，验证将失败。