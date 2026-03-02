#include "LogParser.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

LogParser::LogParser() {}

const std::vector<DailyData>& LogParser::getParsedData() const {
    return allDailyData;
}

bool LogParser::parseFile(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filePath << std::endl;
        return false;
    }

    allDailyData.clear(); // 开始新的解析前清空旧数据
    std::string line;
    DailyData currentDailyData;
    bool newDate = true;

    while (std::getline(file, line)) {
        // 移除字符串两端的空白字符
        line.erase(0, line.find_first_not_of(" \t\n\r"));
        line.erase(line.find_last_not_of(" \t\n\r") + 1);

        if (line.empty()) {
            continue; // 跳过空行
        }

        // 检查是否是日期行 (假设日期行是4位数字)
        if (line.length() == 4 && std::all_of(line.begin(), line.end(), ::isdigit)) {
            if (!currentDailyData.date.empty()) {
                allDailyData.push_back(currentDailyData);
            }
            currentDailyData = DailyData();
            currentDailyData.date = line;
            newDate = true;
        } 
        // 否则认为是项目行或内容行
        else {
            if (currentDailyData.date.empty()) {
                // 如果还没有日期就开始了项目，说明格式错误
                std::cerr << "Error: Project line found before a date line." << std::endl;
                return false;
            }
            
            // 奇数行（相对日期行后）为项目名
            if(newDate)
            {
                ProjectData newProject;
                newProject.projectName = line; // 直接存入原始项目名
                currentDailyData.projects.push_back(newProject);
                newDate = false;
            }
            // 偶数行为内容行
            else
            {
                if (!currentDailyData.projects.empty()) {
                    parseContentLine(line, currentDailyData.projects.back());
                }
                newDate = true;
            }
        }
    }
    
    // 将最后一个日期的数据存入
    if (!currentDailyData.date.empty()) {
        allDailyData.push_back(currentDailyData);
    }

    file.close();
    return true;
}

void LogParser::parseContentLine(const std::string& line, ProjectData& projectData) {
    std::stringstream ss(line);
    char plusSign;
    ss >> plusSign; // 读取'+'
    ss >> projectData.weight; // 读取重量

    std::string repsPart;
    std::getline(ss, repsPart); // 读取剩余的部分，即次数

    // 移除repsPart中的空格
    repsPart.erase(std::remove_if(repsPart.begin(), repsPart.end(), ::isspace), repsPart.end());

    std::stringstream reps_ss(repsPart);
    std::string rep_token;
    while(std::getline(reps_ss, rep_token, '+')) {
        if (!rep_token.empty()) {
            projectData.reps.push_back(std::stoi(rep_token));
        }
    }
}