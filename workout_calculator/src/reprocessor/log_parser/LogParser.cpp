// src/reprocessor/log_parser/LogParser.cpp

#include "LogParser.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <regex>

LogParser::LogParser() {}

const std::vector<DailyData>& LogParser::getParsedData() const {
    return allDailyData;
}

// [NEW] 实现获取年份的接口
std::optional<int> LogParser::getParsedYear() const {
    return parsedYear_;
}

bool LogParser::parseFile(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Error: [LogParser] Could not open file " << filePath << std::endl;
        return false;
    }

    allDailyData.clear();
    // [MODIFIED] 重置年份
    parsedYear_.reset(); 
    std::string line;
    DailyData currentDailyData;
    int lineCounter = 0;
    ProjectData* currentProject = nullptr; 
    
    // [NEW] 使用正则表达式来匹配年份行
    const std::regex yearRegex(R"(^y(\d{4})$)");

    while (std::getline(file, line)) {
        lineCounter++;
        line.erase(0, line.find_first_not_of(" \t\n\r"));
        line.erase(line.find_last_not_of(" \t\n\r") + 1);

        if (line.empty()) continue;

        std::smatch match;
        // [MODIFIED] 检查是否是年份行
        if (std::regex_match(line, match, yearRegex)) {
            if (!parsedYear_.has_value()) { // 只记录第一个找到的年份
                parsedYear_ = std::stoi(match[1].str());
            }
        }
        // 检查是否是日期行
        else if (line.length() == 4 && std::all_of(line.begin(), line.end(), ::isdigit)) {
            if (!currentDailyData.date.empty()) {
                allDailyData.push_back(currentDailyData);
            }
            currentDailyData = DailyData();
            currentDailyData.date = line;
            currentProject = nullptr;
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
                set.setNumber = static_cast<int>(currentProject->sets.size()) + 1;
                currentProject->sets.push_back(set);
            }
        }
        // 否则认为是项目名称行
        else {
            if (currentDailyData.date.empty() && !parsedYear_.has_value()) {
                 std::cerr << "Error: [LogParser] Project name found before a year/date line at line " << lineCounter << "." << std::endl;
                 return false;
            } else if (currentDailyData.date.empty()) {
                // 这是在年份之后，但在第一个日期之前的项目名称
                 std::cerr << "Error: [LogParser] Project name found before a date line at line " << lineCounter << "." << std::endl;
                 return false;
            }
            currentDailyData.projects.emplace_back();
            currentProject = &currentDailyData.projects.back();
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

// parseContentLine 函数保持不变
std::vector<SetData> LogParser::parseContentLine(const std::string& line, double& outWeight) {
    std::vector<SetData> sets;
    std::stringstream ss(line);
    char plusSign;
    ss >> plusSign;
    ss >> outWeight;

    std::string repsPart;
    std::getline(ss, repsPart);

    repsPart.erase(std::remove_if(repsPart.begin(), repsPart.end(), ::isspace), repsPart.end());

    std::stringstream reps_ss(repsPart);
    std::string rep_token;
    while(std::getline(reps_ss, rep_token, '+')) {
        if (!rep_token.empty()) {
            SetData currentSet;
            currentSet.reps = std::stoi(rep_token);
            sets.push_back(currentSet);
        }
    }
    return sets;
}