// src/reprocessor/Reprocessor.cpp

#include "reprocessor/Reprocessor.hpp"
#include "common/JsonReader.hpp" 

#include <iostream>
#include <chrono>
#include <ctime>   

bool Reprocessor::configure(const std::string& mappingFilePath) {
    mappingFilePath_ = mappingFilePath; // 保存路径
    auto jsonDataOpt = JsonReader::readFile(mappingFilePath);
    if (!jsonDataOpt) {
        std::cerr << "Error: [Reprocessor] Failed to read or parse mapping file." << std::endl;
        return false;
    }
    return mapper.loadMappings(jsonDataOpt.value());
}

/**
 * @brief [新] 验证接口的实现
 */
bool Reprocessor::validate(const std::string& logFilePath) const {
    // 使用存储的 mappingFilePath_ 进行验证
    return Validator::validate(logFilePath, mappingFilePath_);
}

/**
 * @brief [新] 转换接口的实现
 */
std::optional<std::vector<DailyData>> Reprocessor::convert(const std::string& logFilePath, std::optional<int> specifiedYear) {
    // 步骤 1: 解析文件
    if (!parser.parseFile(logFilePath)) {
        std::cerr << "Error: [Reprocessor] Parsing log file failed." << std::endl;
        return std::nullopt;
    }
    
    // 使用 const_cast 是因为 getParsedData 返回 const&，但后续处理需要修改数据
    // 这是一个设计上的权衡，也可以让 getParsedData 返回一个拷贝
    auto processedData = const_cast<std::vector<DailyData>&>(parser.getParsedData());
    if (processedData.empty()) {
        return processedData; // 返回一个空向量
    }

    // 步骤 2: 确定要使用的年份
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
    DateProcessor::completeDates(processedData, yearToUse);

    // 步骤 4: 计算训练容量
    VolumeCalculator::calculateVolume(processedData); 

    // 步骤 5: 应用名称和类型映射
    for (auto& dailyData : processedData) {
        for (auto& project : dailyData.projects) {
            ProjectMapping mapping = mapper.getMapping(project.projectName);
            project.projectName = mapping.fullName;
            project.type = mapping.type;
        }
    }
    
    return processedData;
}