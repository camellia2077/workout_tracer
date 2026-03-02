// Reprocessor.cpp

#include "reprocessor/Reprocessor.h"
#include "reprocessor/data_processor/DataProcessor.h"
#include "common/JsonReader.h" 

#include <iostream>
#include <chrono>
#include <ctime>   
#include <iomanip>

bool Reprocessor::configure(const std::string& mappingFilePath) {
    auto jsonDataOpt = JsonReader::readFile(mappingFilePath);
    if (!jsonDataOpt) {
        std::cerr << "Error: [Reprocessor] Failed to read or parse mapping file." << std::endl;
        return false;
    }
    return mapper.loadMappings(jsonDataOpt.value());
}

std::vector<DailyData> Reprocessor::processLogFile(
    const std::string& logFilePath,
    std::optional<int> specifiedYear
) {
    // 1. 确定要使用的年份
    int yearToUse;
    if (specifiedYear.has_value()) {
        yearToUse = specifiedYear.value();
    } else {
        // 如果未指定年份，自动获取当前系统年份
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

    // 2. 解析原始日志文件
    if (!parser.parseFile(logFilePath)) {
        std::cerr << "Error: [Reprocessor] Parsing log file failed." << std::endl;
        return {}; // 返回空向量表示失败
    }
    std::vector<DailyData> data = parser.getParsedData();

    // 3. 补全年份并格式化日期
    for (auto& daily : data) {
        if (daily.date.length() == 4) { // 只处理 "MMDD" 格式
            std::string month = daily.date.substr(0, 2);
            std::string day = daily.date.substr(2, 2);
            daily.date = std::to_string(yearToUse) + "-" + month + "-" + day;
        }
    }

    // 4. 调用DataProcessor计算容量
    DataProcessor::calculateVolume(data); 

    // 5. 应用名称映射
    for (auto& dailyData : data) {
        for (auto& project : dailyData.projects) {
            project.projectName = mapper.getFullName(project.projectName);
        }
    }

    // 6. 返回处理完成的数据
    return data;
}

std::string Reprocessor::formatDataToString(const std::vector<DailyData>& processedData)
{
    // 将格式化任务委托给 LogFormatter 模块
    return LogFormatter::format(processedData);
}