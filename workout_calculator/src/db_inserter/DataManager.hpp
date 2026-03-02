#ifndef DATA_MANAGER_H
#define DATA_MANAGER_H

#include "db_inserter/_internal/Database.hpp"
#include "db_inserter/_internal/LogRepository.hpp"
#include "common/parsed_data.hpp" // 数据结构头文件路径
#include <string>
#include <vector>
#include <memory>

/**
 * @brief 数据管理外观类。
 * 封装了数据库连接、初始化和数据保存的整个流程，
 * 为客户端提供一个简单的接口。
 */
class DataManager {
public:
    DataManager();
    ~DataManager();

    // 禁止拷贝和赋值
    DataManager(const DataManager&) = delete;
    DataManager& operator=(const DataManager&) = delete;

    /**
     * @brief 连接到数据库并初始化数据表。
     * @param dbPath 数据库文件路径。
     * @return 成功返回 true，否则返回 false。
     */
    bool connectAndInitialize(const std::string& dbPath);

    /**
     * @brief 保存日志数据。
     * @param processedData 要保存的数据向量。
     * @return 成功返回 true，否则返回 false。
     */
    bool saveData(const std::vector<DailyData>& processedData);

private:
    std::unique_ptr<Database> database_;
    std::unique_ptr<LogRepository> logRepository_;
};

#endif // DATA_MANAGER_H