#ifndef LOG_REPOSITORY_H
#define LOG_REPOSITORY_H

#include <string>
#include <vector>
#include "common/parsed_data.hpp" // 引用共享的数据结构
#include "Database.hpp"           // 依赖于新的 Database 类
// 数据仓库，专门负责 DailyData 的持久化逻辑
class LogRepository {
public:
    /**
     * @brief 构造函数，需要一个 Database 实例来进行操作。
     * @param db 对 Database 对象的引用。
     */
    explicit LogRepository(Database& db);

    // 禁止拷贝和赋值
    LogRepository(const LogRepository&) = delete;
    LogRepository& operator=(const LogRepository&) = delete;

    /**
     * @brief 初始化存储日志数据所需的数据库表。
     * @return 成功返回 true，否则返回 false。
     */
    bool initializeSchema();

    /**
     * @brief 将处理过的数据保存到数据库中。
     * @param processedData 包含多天日志数据的向量。
     * @return 成功返回 true，否则返回 false。
     */
    bool save(const std::vector<DailyData>& processedData);

private:
    Database& database; // 持有对 Database 对象的引用
};

#endif // LOG_REPOSITORY_H