// reprocessor/preprocessor/log_formatter/JsonFormatter.cpp

#include "JsonFormatter.hpp"
#include "nlohmann/json.hpp"
#include <string> // [ADD] 引入 string 以便使用 to_string

// [MODIFIED] 使用 nlohmann::ordered_json 来保证键的顺序
using json = nlohmann::ordered_json;

/**
 * @brief Provides a JSON serialization rule for the SetData struct.
 */
void to_json(json& j, const SetData& s) {
    // [MODIFIED] 根据重量正负区分普通组和弹力带组
    if (s.weight < 0) {
        j = json{
            {"set", s.setNumber},
            {"elastic_band", std::abs(s.weight)}, // 使用正值记录弹力带磅数
            {"unit", "lbs"}, // 弹力带通常使用 lbs
            {"reps", s.reps},
            {"volume", 0.0} // 不记录容量
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

// [MODIFIED] 重写 format 函数以实现更标准的 JSON 结构
std::string JsonFormatter::format(const std::vector<DailyData>& processedData) {
    if (processedData.empty() || processedData[0].projects.empty()) {
        return "{}";
    }
    
    // 1. 获取生成周期ID和统计信息所需的数据
    const std::string& first_date = processedData[0].date;
    const std::string& type = processedData[0].projects[0].type;
    size_t total_days = processedData.size();

    // 2. 创建周期ID
    std::string cycle_id = first_date + "-" + type;

    // 3. 构建新的、结构固定的JSON对象
    json j;

    j["cycle_id"] = cycle_id; // <<< 推荐的修改
    j["type"] = type;
    j["total_days"] = total_days;
    j["sessions"] = processedData;

    // 4. 返回格式化后的字符串
    return j.dump(4);
}