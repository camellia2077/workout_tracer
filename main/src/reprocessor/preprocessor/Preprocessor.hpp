// reprocessor/preprocessor/Preprocessor.hpp

#ifndef PREPROCESSOR_H
#define PREPROCESSOR_H

#include "common/parsed_data.hpp"
#include "reprocessor/preprocessor/log_parser/LogParser.hpp"
#include "reprocessor/preprocessor/name_mapper/ProjectNameMapper.hpp"
#include <optional>
#include <string>
#include <vector>

class Preprocessor {
public:
    std::optional<std::vector<DailyData>> process(const std::string& logFilePath, const ProjectNameMapper& mapper);

private:
    LogParser parser;

    // [NEW] 辅助函数：负责遍历数据并将项目简称映射为全名和类型
    void mapProjectNames(std::vector<DailyData>& data, const ProjectNameMapper& mapper);
};

#endif // PREPROCESSOR_H