// Reprocessor.h

#ifndef REPROCESSOR_H
#define REPROCESSOR_H

#include <string>
#include <vector>
#include <optional>

#include "common/parsed_data.h"
#include "reprocessor/log_parser/LogParser.h"
#include "reprocessor/name_mapper/ProjectNameMapper.h"
#include "reprocessor/log_formatter/LogFormatter.h"

class Reprocessor {
public:
    /**
     * @brief 配置Reprocessor，加载名称映射文件。
     * @param mappingFilePath 包含项目名称映射的JSON文件路径。
     * @return 如果配置成功返回true，否则返回false。
     */
    bool configure(const std::string& mappingFilePath);

    /**
     * @brief 处理一个原始日志文件，返回包含完整信息的结构化数据。
     * @param logFilePath 要处理的日志文件路径。
     * @param specifiedYear 可选参数，用于指定年份。如果未提供，则自动使用当前系统年份。
     * @return 包含所有处理后数据的向量。如果处理失败则返回空向量。
     */
    std::vector<DailyData> processLogFile(
        const std::string& logFilePath,
        std::optional<int> specifiedYear = std::nullopt
    );

    /**
     * @brief 将处理后的数据格式化为单个字符串。
     * 这个方法在内部调用 LogFormatter 来完成实际的格式化工作。
     * @param processedData 经过 processLogFile 处理后的数据。
     * @return 一个格式化后的字符串，可以直接写入文件。
     */
    std::string formatDataToString(const std::vector<DailyData>& processedData);

private:
    LogParser parser;
    ProjectNameMapper mapper;
};

#endif // REPROCESSOR_H