// controller/DatabaseHandler.hpp

#ifndef DATABASE_HANDLER_H
#define DATABASE_HANDLER_H

#include "ActionHandler.hpp"

// 这个类专门处理与数据库相关的所有操作。
class DatabaseHandler {
public:
    bool handle(const AppConfig& config);
};

#endif // DATABASE_HANDLER_H