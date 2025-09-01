// src/reprocessor/log_parser/LogParser.hpp

#ifndef LOG_PARSER_H
#define LOG_PARSER_H

#include <string>
#include <vector>
#include <optional> // 需要 optional
#include "common/parsed_data.hpp"

class LogParser {
public:
    LogParser();
    bool parseFile(const std::string& filePath);
    const std::vector<DailyData>& getParsedData() const;

    // [NEW] 新增一个接口用于获取解析到的年份
    std::optional<int> getParsedYear() const;

private:
    std::vector<DailyData> allDailyData;
    // [NEW] 用于存储从文件中解析出的年份
    std::optional<int> parsedYear_; 
    
    std::vector<SetData> parseContentLine(const std::string& line, double& outWeight);
};

#endif // LOG_PARSER_H