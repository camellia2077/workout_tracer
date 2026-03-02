#include "LogFormatter.hpp"
#include <sstream>
#include <iomanip>

std::string LogFormatter::format(const std::vector<DailyData>& processedData) {
    std::stringstream ss;

    // 遍历每一天的数据
    for (const auto& daily : processedData) {
        // 根据您的要求，在每个日期条目前添加两行空行以增强可读性
        ss << "\n";

        // 1. 写入处理后的完整日期 (YYYY-MM-DD)
        ss << daily.date << "\n";

        // 2. 遍历当天的所有项目
        for (const auto& project : daily.projects) {
            // 写入项目全名
            ss << project.projectName << "\n";

            // 写入内容行，例如 "+ 60.00 10+10+9+8"
            ss << "+ " << std::fixed << std::setprecision(2) << project.weight << " ";
            
            // 拼接次数
            for (size_t i = 0; i < project.reps.size(); ++i) {
                ss << project.reps[i];
                if (i < project.reps.size() - 1) {
                    ss << "+";
                }
            }
            // 新增：在末尾添加总容量,并转为整数
            ss << "(" << static_cast<int>(project.volume) << ")";
            // 原有的换行符
            ss << "\n";
        }
    }

    return ss.str();
}