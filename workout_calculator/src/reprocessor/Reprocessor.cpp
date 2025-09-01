// src/reprocessor/Reprocessor.cpp

#include "reprocessor/Reprocessor.hpp"
#include "common/JsonReader.hpp"
#include "reprocessor/validator/Validator.hpp" // [FIX] 为 validate 添加头文件
#include <iostream>

// [FIX] 添加了缺失的头文件
#include "reprocessor/date_processor/DateProcessor.hpp"
#include "reprocessor/volume_calculator/VolumeCalculator.hpp"
#include "reprocessor/name_mapper/ProjectNameMapper.hpp" 

// [FIX] 添加 configure 函数的实现
bool Reprocessor::configure(const std::string& mappingFilePath) {
    mappingFilePath_ = mappingFilePath;
    auto jsonDataOpt = JsonReader::readFile(mappingFilePath);
    if (!jsonDataOpt.has_value()) {
        std::cerr << "Error: [Reprocessor] Failed to read or parse mapping file: " << mappingFilePath << std::endl;
        return false;
    }
    if (!mapper.loadMappings(jsonDataOpt.value())) {
        std::cerr << "Error: [Reprocessor] Failed to load mappings from JSON data." << std::endl;
        return false;
    }
    std::cout << "[Reprocessor] Configuration successful. Mappings loaded from " << mappingFilePath << std::endl;
    return true;
}

// [FIX] 添加 validate 函数的实现
bool Reprocessor::validate(const std::string& logFilePath) const {
    // 调用静态的 Validator 类来执行验证
    return Validator::validate(logFilePath, mappingFilePath_);
}

// [MODIFIED] convert 函数实现保持更新
std::optional<std::vector<DailyData>> Reprocessor::convert(const std::string& logFilePath) {
    if (!parser.parseFile(logFilePath)) {
        std::cerr << "Error: [Reprocessor] Parsing log file failed." << std::endl;
        return std::nullopt;
    }
    
    auto processedData = const_cast<std::vector<DailyData>&>(parser.getParsedData());
    
    // 从解析器获取年份
    auto yearToUseOpt = parser.getParsedYear();
    if (!yearToUseOpt.has_value()) {
        std::cerr << "Error: [Reprocessor] Year could not be determined from the log file." << std::endl;
        std::cerr << "Please ensure the file starts with a year declaration (e.g., y2025)." << std::endl;
        return std::nullopt;
    }

    if (processedData.empty()) {
        return processedData;
    }

    // 使用从文件中解析出的年份
    DateProcessor::completeDates(processedData, yearToUseOpt.value());
    VolumeCalculator::calculateVolume(processedData); 

    for (auto& dailyData : processedData) {
        for (auto& project : dailyData.projects) {
            ProjectMapping mapping = mapper.getMapping(project.projectName);
            project.projectName = mapping.fullName;
            project.type = mapping.type;
        }
    }
    
    return processedData;
}