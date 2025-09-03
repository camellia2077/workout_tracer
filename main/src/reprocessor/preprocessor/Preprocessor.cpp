// src/reprocessor/preprocessor/Preprocessor.cpp

#include "Preprocessor.hpp"
#include "reprocessor/preprocessor/date_processor/DateProcessor.hpp"
#include "reprocessor/preprocessor/volume_calculator/VolumeCalculator.hpp"
#include <iostream>

std::optional<std::vector<DailyData>> Preprocessor::process(const std::string& logFilePath, const ProjectNameMapper& mapper) {
    if (!parser.parseFile(logFilePath)) {
        std::cerr << "Error: [Preprocessor] Parsing log file failed." << std::endl;
        return std::nullopt;
    }

    auto processedData = parser.getParsedData();
    auto yearToUseOpt = parser.getParsedYear();

    if (!yearToUseOpt.has_value()) {
        std::cerr << "Error: [Preprocessor] Year could not be determined from the log file." << std::endl;
        return std::nullopt;
    }

    if (processedData.empty()) {
        return processedData;
    }

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