#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include "sqlite3.h"
// 这个类负责通用的数据库连接和事务管理
class Database {
public:
    Database();
    ~Database();

    // 禁止拷贝和赋值
    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;

    /**
     * @brief 连接到 SQLite 数据库。
     * @param dbPath 数据库文件路径。
     * @return 成功返回 true，否则返回 false。
     */
    bool connect(const std::string& dbPath);

    /**
     * @brief 断开数据库连接。
     */
    void disconnect();

    /**
     * @brief 执行一个简单的、不返回结果的 SQL 命令。
     * @param sql 要执行的 SQL 语句。
     * @return 成功返回 true，否则返回 false。
     */
    bool execute(const std::string& sql);

    /**
     * @brief 开始一个事务。
     * @return 成功返回 true，否则返回 false。
     */
    bool beginTransaction();

    /**
     * @brief 提交事务。
     * @return 成功返回 true，否则返回 false。
     */
    bool commit();

    /**
     * @brief 回滚事务。
     * @return 成功返回 true，否则返回 false。
     */
    bool rollback();

    /**
     * @brief 获取底层的 sqlite3 句柄，以便进行更复杂的操作（如预处理语句）。
     * @return sqlite3 数据库连接句柄。
     */
    sqlite3* getHandle();

private:
    sqlite3* db{nullptr};
};

#endif // DATABASE_H