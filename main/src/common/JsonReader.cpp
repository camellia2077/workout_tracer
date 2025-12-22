// common/JsonReader.cpp
#include "JsonReader.hpp"
#include <fstream>
#include <sstream>
#include <iostream>

std::optional<CJsonPtr> JsonReader::readFile(const std::string& filePath) {
    std::ifstream jsonFile(filePath);
    if (!jsonFile.is_open()) {
        std::cerr << "Error: [JsonReader] Could not open file " << filePath << std::endl;
        return std::nullopt;
    }

    // 1. 读取整个文件到 string
    std::stringstream buffer;
    buffer << jsonFile.rdbuf();
    std::string content = buffer.str();

    // 2. 使用 cJSON 解析
    cJSON* raw_json = cJSON_Parse(content.c_str());
    if (raw_json == nullptr) {
        const char* error_ptr = cJSON_GetErrorPtr();
        std::cerr << "Error: [JsonReader] Parse failed: " << (error_ptr ? error_ptr : "Unknown") << std::endl;
        return std::nullopt;
    }

    // 3. 移交所有权给智能指针
    return make_cjson(raw_json);
}