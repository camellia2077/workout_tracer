// src/reprocessor/Reprocessor.hpp

#ifndef REPROCESSOR_H
#define REPROCESSOR_H

#include <string>
#include <vector>
#include <optional>

#include "common/parsed_data.hpp"
#include "reprocessor/log_parser/LogParser.hpp"
#include "reprocessor/name_mapper/ProjectNameMapper.hpp"
// [REMOVED] 下面这一行已被删除
// #include "reprocessor/log_formatter/LogFormatter.hpp" 
#include "reprocessor/volume_calculator/VolumeCalculator.hpp"
#include "reprocessor/date_processor/DateProcessor.hpp"
#include "reprocessor/validator/Validator.hpp"

class Reprocessor {
public:
    bool configure(const std::string& mappingFilePath);
    bool validateFile(const std::string& logFilePath, const std::string& mappingFilePath) const;
    std::optional<std::vector<DailyData>> parseFile(const std::string& logFilePath);
    void processData(std::vector<DailyData>& data, std::optional<int> specifiedYear = std::nullopt);

private:
    LogParser parser;
    ProjectNameMapper mapper;
};

#endif // REPROCESSOR_H