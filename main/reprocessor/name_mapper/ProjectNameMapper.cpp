#include "reprocessor/name_mapper/ProjectNameMapper.h"
#include <iostream>

bool ProjectNameMapper::loadMappings(const nlohmann::json& jsonData) {
    // 检查传入的是否是一个JSON对象
    if (!jsonData.is_object()) {
        std::cerr << "Error: [NameMapper] Provided JSON data is not an object." << std::endl;
        return false;
    }

    // 遍历JSON对象并填充我们的map
    for (auto& el : jsonData.items()) {
        if (el.value().is_string()) {
            mappings[el.key()] = el.value();
        }
    }
    return true;
}

std::string ProjectNameMapper::getFullName(const std::string& shortName) const {
    auto it = mappings.find(shortName);
    if (it != mappings.end()) {
        // 如果找到了映射，返回全名
        return it->second;
    }
    // 如果没找到，返回原始名称
    return shortName;
}