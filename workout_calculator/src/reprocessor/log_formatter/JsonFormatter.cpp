// src/reprocessor/log_formatter/JsonFormatter.cpp

#include "JsonFormatter.hpp"
#include "nlohmann/json.hpp"

// [MODIFIED] 使用 nlohmann::ordered_json 来保证键的顺序
using json = nlohmann::ordered_json;

/**
 * @brief Provides a JSON serialization rule for the SetData struct.
 */
void to_json(json& j, const SetData& s) {
    j = json{
        {"set", s.setNumber},
        {"weight", s.weight},
        {"unit", "kg"}, // Assuming the unit is always kg
        {"reps", s.reps},
        {"volume", s.volume}
    };
}

/**
 * @brief Provides a JSON serialization rule for the ProjectData struct.
 */
void to_json(json& j, const ProjectData& p) {
    j = json{
        {"name", p.projectName},
        {"totalVolume", p.totalVolume},
        {"sets", p.sets}
    };
}

/**
 * @brief Provides a JSON serialization rule for the DailyData struct.
 */
void to_json(json& j, const DailyData& d) {
    j = json{
        {"date", d.date},
        {"exercises", d.projects}
    };
}

std::string JsonFormatter::format(const std::vector<DailyData>& processedData) {
    if (processedData.empty() || processedData[0].projects.empty()) {
        return "{}";
    }
    
    // [MODIFIED] 使用 ordered_json 来确保键的插入顺序
    json j;

    // 1. 首先设置顶层 'type'
    j["type"] = processedData[0].projects[0].type;

    // 2. 然后创建 'sessions' 数组
    j["sessions"] = processedData;

    // 3. 返回格式化后的字符串
    return j.dump(4);
}