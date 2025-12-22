// reprocessor/preprocessor/Preprocessor.cpp

#include "Preprocessor.hpp"
#include "reprocessor/preprocessor/date_processor/DateProcessor.hpp"
#include "reprocessor/preprocessor/volume_calculator/VolumeCalculator.hpp"
#include <iostream>

// [NEW] 实现辅助函数
void Preprocessor::mapProjectNames(std::vector<DailyData>& data, const ProjectNameMapper& mapper) {
    for (auto& dailyData : data) {
        for (auto& project : dailyData.projects) {
            ProjectMapping mapping = mapper.getMapping(project.projectName);
            project.projectName = mapping.fullName;
            project.type = mapping.type;
        }
    }
}

std::optional<std::vector<DailyData>> Preprocessor::process(const std::string& logFilePath, const ProjectNameMapper& mapper) {
    // 1. 解析文件
    if (!parser.parseFile(logFilePath)) {
        std::cerr << "Error: [Preprocessor] Parsing log file failed." << std::endl;
        return std::nullopt;
    }

    // 注意：这里发生了数据拷贝，因为我们需要修改数据，而 getParsedData 返回的是 const 引用
    auto processedData = parser.getParsedData();
    auto yearToUseOpt = parser.getParsedYear();

    if (!yearToUseOpt.has_value()) {
        std::cerr << "Error: [Preprocessor] Year could not be determined from the log file." << std::endl;
        return std::nullopt;
    }

    if (processedData.empty()) {
        return processedData;
    }

    // 2. 数据处理流水线
    // 补全日期
    DateProcessor::completeDates(processedData, yearToUseOpt.value());
    
    // 计算容量 (内部已重构优化)
    VolumeCalculator::calculateVolume(processedData);

    // [MODIFIED] 映射项目名称 (使用新的辅助函数)
    mapProjectNames(processedData, mapper);

    return processedData;
}