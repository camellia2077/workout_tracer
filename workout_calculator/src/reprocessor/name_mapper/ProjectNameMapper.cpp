// src/reprocessor/name_mapper/ProjectNameMapper.cpp

#include "reprocessor/name_mapper/ProjectNameMapper.hpp"
#include <iostream>

// <<< 修改：适配新的 JSON 结构
bool ProjectNameMapper::loadMappings(const nlohmann::json& jsonData) {
    if (!jsonData.is_object()) {
        std::cerr << "Error: [NameMapper] Provided JSON data is not an object." << std::endl;
        return false;
    }

    for (auto& el : jsonData.items()) {
        const auto& value = el.value();
        if (value.is_object() && value.contains("fullName") && value.contains("type")) {
            mappings[el.key()] = {
                value["fullName"].get<std::string>(),
                value["type"].get<std::string>()
            };
        }
    }
    return true;
}

// <<< 修改：返回 ProjectMapping 结构体
ProjectMapping ProjectNameMapper::getMapping(const std::string& shortName) const {
    auto it = mappings.find(shortName);
    if (it != mappings.end()) {
        return it->second;
    }
    // 如果没找到，返回一个默认值，类型为 "unknown"
    return {shortName, "unknown"};
}