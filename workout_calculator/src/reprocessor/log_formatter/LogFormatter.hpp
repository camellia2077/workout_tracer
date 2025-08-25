#ifndef LOG_FORMATTER_H
#define LOG_FORMATTER_H

#include <string>
#include <vector>
#include "common/parsed_data.hpp"

class LogFormatter {
public:
    /**
     * @brief 将处理后的数据结构格式化为符合日志文件规范的单个字符串。
     * @param processedData 包含所有已处理和计算过的数据的向量。
     * @return 一个格式化后的字符串，可以直接写入.txt文件。
     */
    static std::string format(const std::vector<DailyData>& processedData);

private:
    /**
     * @brief 格式化单日的训练日志。
     * @param daily 单日的数据。
     * @return 格式化后的字符串。
     */
    static std::string formatDailyLog(const DailyData& daily);

    /**
     * @brief 格式化单个训练项目。
     * @param project 单个项目的数据。
     * @return 格式化后的字符串。
     */
    static std::string formatProject(const ProjectData& project);

    /**
     * @brief 将次数向量用 '+' 连接成字符串。
     * @param reps 次数向量。
     * @return 连接后的字符串，例如 "10+10+9"。
     */
    static std::string joinReps(const std::vector<int>& reps);
};

#endif // LOG_FORMATTER_H