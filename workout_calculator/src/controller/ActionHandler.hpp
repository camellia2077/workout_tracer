// src/controller/ActionHandler.hpp

#ifndef ACTION_HANDLER_H
#define ACTION_HANDLER_H

#include <string>

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
};

class ActionHandler {
public:
    // Add the "static" keyword here
    static bool run(const AppConfig& config);
};

#endif // ACTION_HANDLER_H