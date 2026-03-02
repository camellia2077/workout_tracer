// Reprocessor.hpp

#ifndef REPROCESSOR_H
#define REPROCESSOR_H

#include <string>
#include <vector>
#include <optional>

#include "common/parsed_data.hpp"
#include "reprocessor/log_parser/LogParser.hpp"
#include "reprocessor/name_mapper/ProjectNameMapper.hpp"
#include "reprocessor/log_formatter/LogFormatter.hpp"
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
     * @brief (新) 仅验证日志文件格式的接口。
     * @param logFilePath 要验证的日志文件路径。
     * @param mappingFilePath 用于验证项目名称的映射文件路径。
     * @return 如果文件格式有效则返回true，否则返回false。
     */
    bool validateFile(const std::string& logFilePath, const std::string& mappingFilePath) const; // MODIFIED: Kept the correct version

    /**
     * @brief (新) 仅解析日志文件的接口（假定文件已通过验证）。
     * @param logFilePath 要解析的日志文件路径。
     * @return 如果解析成功，返回包含原始数据的optional；否则返回std::nullopt。
     */
    std::optional<std::vector<DailyData>> parseFile(const std::string& logFilePath);

    /**
     * @brief 数据处理接口：对解析后的数据进行所有处理步骤。
     * @param data 从parseFile获取的原始数据。此数据将被直接修改。
     * @param specifiedYear 可选参数，用于指定年份。
     */
    void processData(std::vector<DailyData>& data, std::optional<int> specifiedYear = std::nullopt);

    /**
     * @brief 将处理后的数据格式化为单个字符串。
     */
    std::string formatDataToString(const std::vector<DailyData>& processedData);

private:
    LogParser parser;
    ProjectNameMapper mapper;
};

#endif // REPROCESSOR_H