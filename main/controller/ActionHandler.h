// ActionHandler.h

#ifndef ACTION_HANDLER_H
#define ACTION_HANDLER_H

#include "reprocessor/Reprocessor.h"
#include "db_inserter/DataManager.h"
#include <string>
#include <optional>

struct AppConfig {
    std::string log_filepath;
    std::string db_path;
    std::string mapping_path;
    std::optional<int> specified_year;
    bool validate_only = false; // 新增：用于表示是否只执行验证
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