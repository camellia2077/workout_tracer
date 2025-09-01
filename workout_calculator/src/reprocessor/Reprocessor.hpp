// src/reprocessor/Reprocessor.hpp

#ifndef REPROCESSOR_H
#define REPROCESSOR_H

#include "common/parsed_data.hpp"
#include <optional>
#include <string>
#include <vector>

// [FIX] Added missing includes for LogParser and ProjectNameMapper
#include "reprocessor/log_parser/LogParser.hpp"
#include "reprocessor/name_mapper/ProjectNameMapper.hpp"

class Reprocessor {
public:
    bool configure(const std::string& mappingFilePath);
    bool validate(const std::string& logFilePath) const;
    
    // [MODIFIED] convert 函数不再需要年份参数
    std::optional<std::vector<DailyData>> convert(const std::string& logFilePath);

private:
    LogParser parser;
    ProjectNameMapper mapper;
    std::string mappingFilePath_;
};

#endif // REPROCESSOR_H