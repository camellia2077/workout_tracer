#include "LogFormatter.hpp"
#include <format>
#include <sstream>
#include <numeric>

// --- 新的私有辅助函数 ---

/**
 * @brief 将次数向量用 '+' 连接成字符串。
 */
std::string LogFormatter::joinReps(const std::vector<int>& reps) {
    if (reps.empty()) {
        return "";
    }
    
    // 使用 stringstream 来高效拼接
    std::stringstream reps_ss;
    reps_ss << reps[0];
    for (size_t i = 1; i < reps.size(); ++i) {
        reps_ss << "+" << reps[i];
    }
    return reps_ss.str();
}

/**
 * @brief 格式化单个训练项目。
 */
std::string LogFormatter::formatProject(const ProjectData& project) {
    // 1. 拼接 reps 字符串
    std::string reps_str = joinReps(project.reps);

    // 2. 格式化项目名称和内容行，并为占位符添加数字序号
    return std::format("{0}\n+ {1:.2f} {2}({3})\n", 
                       project.projectName, // {0} -> project.projectName
                       project.weight,  // {1:.2f} -> project.weight (保留两位小数)
                       reps_str, // {2} -> reps_str
                       static_cast<int>(project.volume));// {3} -> static_cast<int>(project.volume)
}

/**
 * @brief 格式化单日的训练日志。
 */
std::string LogFormatter::formatDailyLog(const DailyData& daily) {
    std::string result;
    // 1. 写入日期
    result += std::format("{}\n", daily.date);

    // 2. 遍历并格式化当天的所有项目
    for (const auto& project : daily.projects) {
        result += formatProject(project);
    }
    return result;
}


// --- 公共接口函数 (现在非常简洁) ---

std::string LogFormatter::format(const std::vector<DailyData>& processedData) {
    if (processedData.empty() || processedData[0].projects.empty()) {
        return ""; // 如果没有数据，返回空字符串
    }

    // <<< 新增：在文件开头标注类型
    // 从第一个数据点获取类型，因为传入此函数的数据都应是同一类型
    const std::string& type = processedData[0].projects[0].type;
    std::string result = std::format("type: {}\n", type);

    result.reserve(processedData.size() * 150);

    for (const auto& daily : processedData) {
        result += "\n";
        result += formatDailyLog(daily);
    }

    return result;
}