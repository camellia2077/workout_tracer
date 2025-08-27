// src/common/parsed_data.hpp

#ifndef PARSED_DATA_H
#define PARSED_DATA_H

#include <string>
#include <vector>
#include <numeric>

struct ProjectData {
    std::string projectName; // 运动的名称
    std::string type;        // 运动的类型,例如卧推是push
    double weight;  // 使用的重量
    std::vector<int> reps; // 每组的重复次数
    double volume {0.0}; // 体积
    int line_number;    // 该数据在文件中的行号
};

struct DailyData {
    std::string date;
    std::vector<ProjectData> projects;
};

#endif // PARSED_DATA_H