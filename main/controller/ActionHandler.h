// ActionHandler.h

#ifndef ACTION_HANDLER_H
#define ACTION_HANDLER_H

#include "reprocessor/Reprocessor.h"
#include "db_inserter/DataManager.h"
#include <string>
#include <optional>

// 新增：定义输出模式的枚举
enum class OutputMode {
    ALL,        // 输出文件并保存到数据库（默认）
    FILE_ONLY,  // 只输出到文件
    DB_ONLY     // 只保存到数据库
};

struct AppConfig {
    std::string log_filepath;
    std::string db_path;
    std::string mapping_path;
    std::optional<int> specified_year;
    bool validate_only = false;
    OutputMode output_mode = OutputMode::ALL; // 新增：添加输出模式字段，默认为ALL
};

class ActionHandler {
public:
    bool run(const AppConfig& config);

private:
    bool writeStringToFile(const std::string& filepath, const std::string& content);

    Reprocessor reprocessor_;
    DataManager dataManager_;
};

#endif // ACTION_HANDLER_H