// src/controller/ActionHandler.hpp

#ifndef ACTION_HANDLER_H
#define ACTION_HANDLER_H

#include "reprocessor/Reprocessor.hpp"
#include <string>
#include <optional>
#include <vector>

// 配置结构体，移除了数据库路径和输出模式
struct AppConfig {
    std::string log_filepath;      // 日志文件或目录的路径
    std::string mapping_path;      // mapping.json 的路径
    std::string base_path;         // 程序运行的基础路径，用于定位输出文件夹
    std::optional<int> specified_year;
    bool validate_only = false;    // 是否只进行验证
};

class ActionHandler {
public:
    bool run(const AppConfig& config);

private:
    bool processFile(const std::string& logFilePath, const AppConfig& config);
    bool writeStringToFile(const std::string& filepath, const std::string& content);

    Reprocessor reprocessor_;
    // 已移除: DataManager dataManager_;
};

#endif // ACTION_HANDLER_H