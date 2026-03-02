#ifndef LOG_FORMATTER_H
#define LOG_FORMATTER_H

#include <string>
#include <vector>
#include "common/parsed_data.h"

class LogFormatter {
public:
    /**
     * @brief 将处理后的数据结构格式化为符合日志文件规范的单个字符串。
     * @param processedData 包含所有已处理和计算过的数据的向量。
     * @return 一个格式化后的字符串，可以直接写入.txt文件。
     */
    static std::string format(const std::vector<DailyData>& processedData);
};

#endif // LOG_FORMATTER_H