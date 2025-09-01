// src/controller/ActionHandler.hpp

#ifndef ACTION_HANDLER_H
#define ACTION_HANDLER_H

#include "reprocessor/Reprocessor.hpp"
#include <string>
#include <optional>

// [MODIFIED] 使用枚举来明确表示操作类型
enum class ActionType {
    Validate,
    Convert
};

struct AppConfig {
    ActionType action;             // 要执行的操作
    std::string log_filepath;      // 日志文件或目录的路径
    std::string mapping_path;      // mapping.json 的路径
    std::string base_path;         // 程序运行的基础路径
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