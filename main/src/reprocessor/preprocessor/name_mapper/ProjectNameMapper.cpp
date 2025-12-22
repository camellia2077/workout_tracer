// reprocessor/preprocessor/name_mapper/ProjectNameMapper.cpp

#include "reprocessor/preprocessor/name_mapper/ProjectNameMapper.hpp"
#include <iostream>
#include <cjson/cJSON.h> // 确保引入了 cJSON 头文件

// 1. 加载映射 (适配 cJSON)
bool ProjectNameMapper::loadMappings(const cJSON* jsonRoot) {
    if (!jsonRoot || !cJSON_IsObject(jsonRoot)) {
        std::cerr << "Error: [NameMapper] Invalid JSON object." << std::endl;
        return false;
    }

    // 遍历 JSON 对象的所有子节点
    cJSON* child = jsonRoot->child;
    while (child) {
        // child->string 是键 (如 "bp")
        if (child->string) {
            std::string key = child->string;
            
            cJSON* fullNameItem = cJSON_GetObjectItemCaseSensitive(child, "fullName");
            cJSON* typeItem = cJSON_GetObjectItemCaseSensitive(child, "type");

            if (cJSON_IsString(fullNameItem) && (fullNameItem->valuestring != nullptr) &&
                cJSON_IsString(typeItem) && (typeItem->valuestring != nullptr)) {
                
                mappings[key] = {
                    fullNameItem->valuestring,
                    typeItem->valuestring
                };
            }
        }
        child = child->next;
    }
    return true;
}

// 2 getMapping 实现
ProjectMapping ProjectNameMapper::getMapping(const std::string& shortName) const {
    auto it = mappings.find(shortName);
    if (it != mappings.end()) {
        return it->second;
    }
    // 如果没找到，返回一个默认值，类型为 "unknown"
    return {shortName, "unknown"};
}