# fitness_calculator
一个用于处理健身数据的程序

# 目录结构
```
.
├── build.sh
├── CMakeLists.txt
├── main.cpp
├── mapping.json
├── common/
│   ├── JsonReader.cpp
│   ├── JsonReader.h
│   └── parsed_data.h
├── db_inserter/
│   ├── SqliteManager.cpp
│   └── SqliteManager.h
└── reprocessor
    ├── data_processor/
    │   ├── DataProcessor.cpp
    │   └── DataProcessor.h
    ├── log_formatter/
    │   ├── LogFormatter.cpp
    │   └── LogFormatter.h
    ├── log_parser/
    │   ├── LogParser.cpp
    │   └── LogParser.h
    ├── name_mapper/
    │   ├── ProjectNameMapper.cpp
    │   └── ProjectNameMapper.h
    ├── Reprocessor.cpp
    └── Reprocessor.h
```