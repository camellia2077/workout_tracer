// src/reprocessor/log_formatter/LogFormatter.cpp

#include "LogFormatter.hpp"
#include <format>
#include <sstream>
#include <numeric>
#include <map> // 需要 map 来根据重量分组

// 将次数向量用 '+' 连接成字符串 (这个函数可以复用)
std::string LogFormatter::joinReps(const std::vector<int>& reps) {
    if (reps.empty()) return "";
    std::stringstream reps_ss;
    reps_ss << reps[0];
    for (size_t i = 1; i < reps.size(); ++i) {
        reps_ss << "+" << reps[i];
    }
    return reps_ss.str();
}

/**
 * @brief 格式化单个训练项目 (重写)
 */
std::string LogFormatter::formatProject(const ProjectData& project) {
    std::string result;
    result += std::format("{}\n", project.projectName);

    // 使用 map 按重量对组进行分组
    std::map<double, std::vector<int>> setsByWeight;
    for (const auto& set : project.sets) {
        setsByWeight[set.weight].push_back(set.reps);
    }

    // 遍历 map，为每个重量生成一行输出
    for (const auto& pair : setsByWeight) {
        double weight = pair.first;
        const auto& reps = pair.second;
        
        std::string reps_str = joinReps(reps);
        double line_volume = weight * std::accumulate(reps.begin(), reps.end(), 0);

        result += std::format("+ {:.2f} {}({:.0f})\n", 
                              weight,
                              reps_str,
                              line_volume);
    }
    
    // 添加项目总容量的注释行
    result += std::format("Total Volume: {:.0f}\n", project.totalVolume);

    return result;
}

// formatDailyLog 和 format 函数基本保持不变，只是调用新的 formatProject
std::string LogFormatter::formatDailyLog(const DailyData& daily) {
    std::string result;
    result += std::format("{}\n", daily.date);

    for (const auto& project : daily.projects) {
        result += formatProject(project);
    }
    return result;
}

std::string LogFormatter::format(const std::vector<DailyData>& processedData) {
    if (processedData.empty()) return "";

    const std::string& type = processedData[0].projects[0].type;
    std::string result = std::format("type: {}\n", type);

    result.reserve(processedData.size() * 200);

    for (const auto& daily : processedData) {
        result += "\n";
        result += formatDailyLog(daily);
    }

    return result;
}