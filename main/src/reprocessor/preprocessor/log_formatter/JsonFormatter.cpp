// reprocessor/preprocessor/log_formatter/JsonFormatter.cpp
#include "JsonFormatter.hpp"
#include "common/CJsonHelper.hpp"
#include <cmath>

// 辅助函数：添加 Set 数据
cJSON* create_set_json(const SetData& s) {
    cJSON* jSet = cJSON_CreateObject();
    cJSON_AddNumberToObject(jSet, "set", s.setNumber);
    
    if (s.weight < 0) {
        cJSON_AddNumberToObject(jSet, "elastic_band", std::abs(s.weight));
        cJSON_AddStringToObject(jSet, "unit", "lbs");
        cJSON_AddNumberToObject(jSet, "reps", s.reps);
        cJSON_AddNumberToObject(jSet, "volume", 0.0);
    } else {
        cJSON_AddNumberToObject(jSet, "weight", s.weight);
        cJSON_AddStringToObject(jSet, "unit", "kg");
        cJSON_AddNumberToObject(jSet, "reps", s.reps);
        cJSON_AddNumberToObject(jSet, "volume", s.volume);
    }
    return jSet;
}

std::string JsonFormatter::format(const std::vector<DailyData>& processedData) {
    if (processedData.empty()) return "{}";

    // 1. 创建根对象
    CJsonPtr root = make_cjson(cJSON_CreateObject());

    const std::string& start_date = processedData[0].date;
    cJSON_AddStringToObject(root.get(), "cycle_id", start_date.c_str());
    cJSON_AddStringToObject(root.get(), "type", "mixed");
    cJSON_AddNumberToObject(root.get(), "total_days", processedData.size());

    // 2. 创建 Sessions 数组
    cJSON* jSessions = cJSON_AddArrayToObject(root.get(), "sessions");

    for (const auto& daily : processedData) {
        cJSON* jDaily = cJSON_CreateObject();
        cJSON_AddStringToObject(jDaily, "date", daily.date.c_str());

        // Exercises 数组
        cJSON* jExercises = cJSON_AddArrayToObject(jDaily, "exercises");
        
        for (const auto& proj : daily.projects) {
            cJSON* jProj = cJSON_CreateObject();
            cJSON_AddStringToObject(jProj, "name", proj.projectName.c_str());
            cJSON_AddStringToObject(jProj, "type", proj.type.c_str());
            cJSON_AddNumberToObject(jProj, "totalVolume", proj.totalVolume);

            // Sets 数组
            cJSON* jSets = cJSON_AddArrayToObject(jProj, "sets");
            for (const auto& s : proj.sets) {
                cJSON_AddItemToArray(jSets, create_set_json(s));
            }
            
            cJSON_AddItemToArray(jExercises, jProj);
        }
        cJSON_AddItemToArray(jSessions, jDaily);
    }

    // 3. 打印为字符串 (需要手动释放 char*)
    char* jsonString = cJSON_Print(root.get());
    std::string result(jsonString);
    cJSON_free(jsonString); // 重要：释放打印缓冲区

    return result;
}