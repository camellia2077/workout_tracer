// src/reprocessor/log_formatter/JsonFormatter.hpp

#ifndef JSON_FORMATTER_H
#define JSON_FORMATTER_H

#include "common/parsed_data.hpp"
#include <string>
#include <vector>

class JsonFormatter {
public:
    /**
     * @brief 将处理后的数据结构格式化为 JSON 字符串。
     * @param processedData 包含所有已处理和计算过的数据的向量。
     * @return 一个格式化后的 JSON 字符串。
     */
    static std::string format(const std::vector<DailyData>& processedData);
};

#endif // JSON_FORMATTER_H