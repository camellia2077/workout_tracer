// src/controller/ActionHandler.hpp

#ifndef ACTION_HANDLER_H
#define ACTION_HANDLER_H

#include "reprocessor/Reprocessor.hpp"
#include <string>
#include <optional>

enum class ActionType {
    Validate,
    Convert,
    Insert
};

struct AppConfig {
    ActionType action;
    std::string log_filepath;      // 日志文件/目录 或 reprocessed_json 目录的路径
    std::string mapping_path;
    std::string base_path;
    std::optional<int> specified_year;
    // [REMOVED] db_path 字段已被移除
};

class ActionHandler {
public:
    bool run(const AppConfig& config);

private:
    bool writeStringToFile(const std::string& filepath, const std::string& content);
    Reprocessor reprocessor_;
};

#endif // ACTION_HANDLER_H