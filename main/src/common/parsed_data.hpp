// src/common/parsed_data.hpp

#ifndef PARSED_DATA_H
#define PARSED_DATA_H

#include <string>
#include <vector>
#include <numeric>

// 定义每一组的具体数据
struct SetData {
    int setNumber;      // 组号 (e.g., 1, 2, 3...)
    double weight;      // 这组的重量
    int reps;           // 这组的次数
    double volume {0.0}; // 这组的容量 (weight * reps)
};

struct ProjectData {
    std::string projectName; // 运动的名称
    std::string type;        // 运动的类型,例如卧推是push
    std::vector<SetData> sets; // 包含所有组的向量
    double totalVolume {0.0}; // 这个项目的总容量
    int line_number;         // 该项目在文件中的起始行号
};

struct DailyData {
    // [FIXED] 确保这里使用的是 std::string 而不是 std.string
    std::string date;
    std::vector<ProjectData> projects;
};

#endif // PARSED_DATA_H