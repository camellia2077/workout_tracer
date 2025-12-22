// /reprocessor/preprocessor/log_formatter/JsonFormatter.cpp

#include "JsonFormatter.hpp"
#include "nlohmann/json.hpp"
#include <string>
#include <cmath> // for std::abs

using json = nlohmann::ordered_json;

void to_json(json& j, const SetData& s) {
    if (s.weight < 0) {
        j = json{
            {"set", s.setNumber},
            {"elastic_band", std::abs(s.weight)},
            {"unit", "lbs"},
            {"reps", s.reps},
            {"volume", 0.0}
        };
    } else {
        j = json{
            {"set", s.setNumber},
            {"weight", s.weight},
            {"unit", "kg"},
            {"reps", s.reps},
            {"volume", s.volume}
        };
    }
}

void to_json(json& j, const ProjectData& p) {
    j = json{
        {"name", p.projectName},
        {"type", p.type}, // [NEW] 显式包含动作类型 (push/pull/squat)
        {"totalVolume", p.totalVolume},
        {"sets", p.sets}
    };
}

void to_json(json& j, const DailyData& d) {
    j = json{
        {"date", d.date},
        {"exercises", d.projects}
    };
}

std::string JsonFormatter::format(const std::vector<DailyData>& processedData) {
    if (processedData.empty()) {
        return "{}";
    }
    
    // 1. 获取起始日期作为 Cycle ID (例如: "2025-07-05")
    const std::string& start_date = processedData[0].date;
    std::string cycle_id = start_date; 
    
    size_t total_days = processedData.size();

    // 2. 构建 JSON
    json j;
    j["cycle_id"] = cycle_id;
    j["type"] = "mixed"; // [MODIFIED] 标记为混合类型
    j["total_days"] = total_days;
    j["sessions"] = processedData; // 直接序列化所有数据，不按类型拆分

    return j.dump(4);
}