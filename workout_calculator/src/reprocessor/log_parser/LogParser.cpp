// src/reprocessor/log_parser/LogParser.cpp

#include "LogParser.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

LogParser::LogParser() {}

const std::vector<DailyData>& LogParser::getParsedData() const {
    return allDailyData;
}

// 内部辅助函数，现在返回一个SetData的向量
std::vector<SetData> LogParser::parseContentLine(const std::string& line, double& outWeight) {
    std::vector<SetData> sets;
    std::stringstream ss(line);
    char plusSign;
    ss >> plusSign; // 读取'+'
    ss >> outWeight; // 读取重量

    std::string repsPart;
    std::getline(ss, repsPart); // 读取剩余的部分，即次数

    repsPart.erase(std::remove_if(repsPart.begin(), repsPart.end(), ::isspace), repsPart.end());

    std::stringstream reps_ss(repsPart);
    std::string rep_token;
    while(std::getline(reps_ss, rep_token, '+')) {
        if (!rep_token.empty()) {
            SetData currentSet;
            currentSet.reps = std::stoi(rep_token);
            // 其他字段 (weight, setNumber) 在主循环中填充
            sets.push_back(currentSet);
        }
    }
    return sets;
}

bool LogParser::parseFile(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Error: [LogParser] Could not open file " << filePath << std::endl;
        return false;
    }

    allDailyData.clear();
    std::string line;
    DailyData currentDailyData;
    int lineCounter = 0;
    
    // 追踪当前项目，以便向其追加数据
    ProjectData* currentProject = nullptr; 

    while (std::getline(file, line)) {
        lineCounter++;
        line.erase(0, line.find_first_not_of(" \t\n\r"));
        line.erase(line.find_last_not_of(" \t\n\r") + 1);

        if (line.empty()) continue;

        // 检查是否是日期行
        if (line.length() == 4 && std::all_of(line.begin(), line.end(), ::isdigit)) {
            if (!currentDailyData.date.empty()) {
                allDailyData.push_back(currentDailyData);
            }
            currentDailyData = DailyData();
            currentDailyData.date = line;
            currentProject = nullptr; // 新的一天，重置当前项目指针
        } 
        // 检查是否是内容行
        else if (line[0] == '+') {
            if (!currentProject) {
                std::cerr << "Error: [LogParser] Content line found without a preceding project name at line " << lineCounter << "." << std::endl;
                return false;
            }
            double weight;
            std::vector<SetData> parsedSets = parseContentLine(line, weight);
            for (auto& set : parsedSets) {
                set.weight = weight;
                // 关键：组号是连续递增的
                set.setNumber = static_cast<int>(currentProject->sets.size()) + 1;
                currentProject->sets.push_back(set);
            }
        }
        // 否则认为是项目名称行
        else {
            if (currentDailyData.date.empty()) {
                std::cerr << "Error: [LogParser] Project name found before a date line at line " << lineCounter << "." << std::endl;
                return false;
            }
            currentDailyData.projects.emplace_back(); // 创建一个新的 ProjectData
            currentProject = &currentDailyData.projects.back(); // 指向这个新创建的 Project
            currentProject->projectName = line;
            currentProject->line_number = lineCounter;
        }
    }
    
    if (!currentDailyData.date.empty()) {
        allDailyData.push_back(currentDailyData);
    }

    file.close();
    return true;
}