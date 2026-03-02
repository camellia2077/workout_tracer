#ifndef LOG_PARSER_H
#define LOG_PARSER_H

#include <string>
#include <vector>
#include "common/parsed_data.h"

class LogParser {
public:
    // 构造函数
    LogParser();

    // 解析指定路径的txt文件
    // 成功返回true, 失败返回false
    bool parseFile(const std::string& filePath);

    // 获取解析后的数据
    const std::vector<DailyData>& getParsedData() const;

private:
    std::vector<DailyData> allDailyData; // 存储所有解析出的数据

    // 内部辅助函数，用于解析内容行
    void parseContentLine(const std::string& line, ProjectData& projectData);
};

#endif // LOG_PARSER_H