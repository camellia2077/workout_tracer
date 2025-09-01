// src/reprocessor/log_parser/LogParser.hpp

#ifndef LOG_PARSER_H
#define LOG_PARSER_H

#include <string>
#include <vector>
#include "common/parsed_data.hpp"

class LogParser {
public:
    LogParser();
    bool parseFile(const std::string& filePath);
    const std::vector<DailyData>& getParsedData() const;

private:
    std::vector<DailyData> allDailyData;

    // [FIXED] 更新函数声明以匹配 .cpp 文件中的实现
    std::vector<SetData> parseContentLine(const std::string& line, double& outWeight);
};

#endif // LOG_PARSER_H