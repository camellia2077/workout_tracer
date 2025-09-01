// src/reprocessor/Reprocessor.hpp

#ifndef REPROCESSOR_H
#define REPROCESSOR_H

#include <string>
#include <vector>
#include <optional>

#include "common/parsed_data.hpp"
#include "reprocessor/log_parser/LogParser.hpp"
#include "reprocessor/name_mapper/ProjectNameMapper.hpp"
#include "reprocessor/volume_calculator/VolumeCalculator.hpp"
#include "reprocessor/date_processor/DateProcessor.hpp"
#include "reprocessor/validator/Validator.hpp"

class Reprocessor {
public:
    /**
     * @brief 配置Reprocessor，加载名称映射文件。
     */
    bool configure(const std::string& mappingFilePath);

    /**
     * @brief [新] 验证单个日志文件的格式。
     * @param logFilePath 要验证的日志文件路径。
     * @return 如果文件格式有效则返回true，否则返回false。
     */
    bool validate(const std::string& logFilePath) const;

    /**
     * @brief [新] 转换单个日志文件为处理后的数据结构。
     * @param logFilePath 要转换的日志文件路径。
     * @param specifiedYear 可选的4位数年份。
     * @return 如果成功，返回包含所有处理后数据的optional；否则返回std::nullopt。
     */
    std::optional<std::vector<DailyData>> convert(const std::string& logFilePath, std::optional<int> specifiedYear);

private:
    LogParser parser;
    ProjectNameMapper mapper;
    std::string mappingFilePath_; // 存储 mapping.json 的路径以供验证器使用
};

#endif // REPROCESSOR_H