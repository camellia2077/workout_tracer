// src/controller/ActionHandler.hpp

#ifndef ACTION_HANDLER_H
#define ACTION_HANDLER_H

#include "reprocessor/Reprocessor.hpp"
#include <string>
#include <optional>

// [MODIFIED] 添加了 Export 动作
enum class ActionType {
    Validate,
    Convert,
    Insert,
    Export
};

struct AppConfig {
    ActionType action;
    std::string log_filepath;      
    std::string mapping_path;
    std::string base_path;
    // [REMOVED] specified_year 已被移除
};

class ActionHandler {
public:
    bool run(const AppConfig& config);

private:
    bool writeStringToFile(const std::string& filepath, const std::string& content);
    Reprocessor reprocessor_;
};

#endif // ACTION_HANDLER_H