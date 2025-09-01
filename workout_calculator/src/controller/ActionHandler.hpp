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
    // 对于 Export 操作，此路径将不被使用
    std::string log_filepath;      
    std::string mapping_path;
    std::string base_path;
    std::optional<int> specified_year;
};

class ActionHandler {
public:
    bool run(const AppConfig& config);

private:
    bool writeStringToFile(const std::string& filepath, const std::string& content);
    Reprocessor reprocessor_;
};

#endif // ACTION_HANDLER_H