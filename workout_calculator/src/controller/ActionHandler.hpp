// ActionHandler.hpp

#ifndef ACTION_HANDLER_H
#define ACTION_HANDLER_H

#include "reprocessor/Reprocessor.hpp"
#include "db_inserter/DataManager.hpp"
#include <string>
#include <optional>
#include <vector> // 新增

// 定义输出模式的枚举
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
    OutputMode output_mode = OutputMode::ALL;
};

class ActionHandler {
public:
    bool run(const AppConfig& config);

private:
    // **新增**: 封装了处理单个文件的核心逻辑
    bool processFile(const std::string& logFilePath, const AppConfig& config);
    
    bool writeStringToFile(const std::string& filepath, const std::string& content);

    Reprocessor reprocessor_;
    DataManager dataManager_;
};

#endif // ACTION_HANDLER_H