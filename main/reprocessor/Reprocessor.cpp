// Reprocessor.cpp

#include "reprocessor/Reprocessor.h"
#include "common/JsonReader.h" 

#include <iostream>
#include <chrono>
#include <ctime>   
#include <iomanip>
#include <vector>
#include <string>

bool Reprocessor::configure(const std::string& mappingFilePath) {
    auto jsonDataOpt = JsonReader::readFile(mappingFilePath);
    if (!jsonDataOpt) {
        std::cerr << "Error: [Reprocessor] Failed to read or parse mapping file." << std::endl;
        return false;
    }
    return mapper.loadMappings(jsonDataOpt.value());
}

/**
 * @brief (新) 纯验证接口的实现
 */
bool Reprocessor::validateFile(const std::string& logFilePath, const std::string& mappingFilePath) const {
    // 直接将任务委托给 Validator
    return Validator::validate(logFilePath, mappingFilePath);
} // <-- ADDED: Missing curly brace

/**
 * @brief (新) 纯解析接口的实现
 */
std::optional<std::vector<DailyData>> Reprocessor::parseFile(const std::string& logFilePath) {
    // 注意：这里不再进行验证
    if (!parser.parseFile(logFilePath)) {
        std::cerr << "Error: [Reprocessor] Parsing log file failed." << std::endl;
        return std::nullopt;
    }

    return parser.getParsedData();
}

/**
 * @brief 数据处理接口实现 (保持不变)
 */
void Reprocessor::processData(std::vector<DailyData>& data, std::optional<int> specifiedYear) {
    // 步骤 1: 确定要使用的年份
    int yearToUse;
    if (specifiedYear.has_value()) {
        yearToUse = specifiedYear.value();
    } else {
        const auto now = std::chrono::system_clock::now();
        const auto in_time_t = std::chrono::system_clock::to_time_t(now);
        std::tm buf;
        #ifdef _WIN32
            localtime_s(&buf, &in_time_t);
        #else
            localtime_r(&in_time_t, &buf);
        #endif
        yearToUse = 1900 + buf.tm_year;
    }

    // 步骤 3: 补全年份并格式化日期
    DateProcessor::completeDates(data, yearToUse);

    // 步骤 4: 计算训练容量
    VolumeCalculator::calculateVolume(data); 

    // 步骤 5: 应用名称映射
    for (auto& dailyData : data) {
        for (auto& project : dailyData.projects) {
            project.projectName = mapper.getFullName(project.projectName);
        }
    }
}

std::string Reprocessor::formatDataToString(const std::vector<DailyData>& processedData) {
    return LogFormatter::format(processedData);
}