// db/facade/DbFacade.cpp

#include "DbFacade.hpp"
#include "db/inserter/DataInserter.hpp"
#include <iostream>
#include <stdexcept>

bool DbFacade::insertTrainingData(sqlite3* db, const cJSON* jsonData) {
    char* zErrMsg = nullptr;

    // 1. 开始事务
    if (sqlite3_exec(db, "BEGIN TRANSACTION;", NULL, NULL, &zErrMsg) != SQLITE_OK) {
        std::cerr << "SQL error starting transaction: " << zErrMsg << std::endl;
        sqlite3_free(zErrMsg);
        return false;
    }

    try {
        // 2. 将具体工作委托给 Inserter
        DataInserter inserter(db);
        inserter.insert(jsonData); // 这里的 DataInserter.insert 现在接受 const cJSON*

    } catch (const std::exception& e) {
        std::cerr << "An error occurred during insertion: " << e.what() << std::endl;
        // 3a. 如果出错，回滚事务
        sqlite3_exec(db, "ROLLBACK;", NULL, NULL, NULL);
        return false;
    }

    // 3b. 如果成功，提交事务
    if (sqlite3_exec(db, "COMMIT;", NULL, NULL, &zErrMsg) != SQLITE_OK) {
        std::cerr << "SQL error committing transaction: " << zErrMsg << std::endl;
        sqlite3_free(zErrMsg);
        // 提交失败也需要回滚
        sqlite3_exec(db, "ROLLBACK;", NULL, NULL, NULL);
        return false;
    }

    return true;
}