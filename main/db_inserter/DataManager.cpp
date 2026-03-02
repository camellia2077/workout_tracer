#include "DataManager.h"
#include <iostream>

DataManager::DataManager() = default;
DataManager::~DataManager() = default;

bool DataManager::connectAndInitialize(const std::string& dbPath) {
    // 1. 创建并连接 Database 对象
    database_ = std::make_unique<Database>();
    if (!database_->connect(dbPath)) {
        std::cerr << "Error: [DataManager] Database connection failed." << std::endl;
        database_.reset(); // 操作失败，清理资源
        return false;
    }

    // 2. 创建 LogRepository 并用 Database 对象进行初始化
    logRepository_ = std::make_unique<LogRepository>(*database_);
    if (!logRepository_->initializeSchema()) {
        std::cerr << "Error: [DataManager] Schema initialization failed." << std::endl;
        // 操作失败，清理所有已分配的资源
        logRepository_.reset();
        database_.reset();
        return false;
    }
    
    std::cout << "[DataManager] Connection and initialization successful." << std::endl;
    return true;
}

bool DataManager::saveData(const std::vector<DailyData>& processedData) {
    // 检查是否已成功连接和初始化
    if (!logRepository_) {
        std::cerr << "Error: [DataManager] Not connected. Call connectAndInitialize() first." << std::endl;
        return false;
    }
    // 将调用委托给 LogRepository
    return logRepository_->save(processedData);
}