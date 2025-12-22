// reprocessor/validator/Validator.cpp

#include "Validator.hpp"
#include "common/JsonReader.hpp"
#include "common/CJsonHelper.hpp" // 引入辅助头文件以使用 CJsonPtr
#include <fstream>
#include <iostream>
#include <sstream>

bool Validator::validate(const std::string& logFilePath, const std::string& mappingFilePath) {
    // 步骤 1: 创建规则
    auto validTitlesOpt = loadValidTitles(mappingFilePath);
    if (!validTitlesOpt) return false;

    auto rulesOpt = createRules(validTitlesOpt.value());
    if (!rulesOpt) return false;
    const auto& rules = rulesOpt.value();

    // 步骤 2: 读取文件并使用 LineValidator
    std::ifstream file(logFilePath);
    if (!file.is_open()) {
        std::cerr << "Error: [Validator] Could not open file " << logFilePath << std::endl;
        return false;
    }

    LineValidator lineValidator;
    int errorCount = 0;
    std::string line;

    while (std::getline(file, line)) {
        line.erase(0, line.find_first_not_of(" \t\n\r"));
        line.erase(line.find_last_not_of(" \t\n\r") + 1);

        if (line.empty()) continue;

        lineValidator.validateLine(line, rules, errorCount);
    }
    
    lineValidator.finalizeValidation(errorCount);

    return errorCount == 0;
}

std::optional<ValidationRules> Validator::createRules(const std::vector<std::string>& validTitles) {
    if (validTitles.empty()) {
        std::cerr << "Warning: [Validator] No valid titles found in mapping file." << std::endl;
    }
    std::stringstream titleRegexPattern;
    titleRegexPattern << "^(";
    for (size_t i = 0; i < validTitles.size(); ++i) {
        titleRegexPattern << validTitles[i] << (i < validTitles.size() - 1 ? "|" : "");
    }
    titleRegexPattern << ")$";
    try {
        return ValidationRules{
            std::regex(R"(^y\d{4}$)"),
            std::regex(R"(^\d{4}$)"),
            std::regex(titleRegexPattern.str()),
            std::regex(R"(^[+-]\s*\d+(\.\d+)?(lbs|kg|LBS|KG)?\s+\d+(\s*\+\s*\d+)*$)")
        };
    } catch (const std::regex_error& e) {
        std::cerr << "Error: [Validator] Failed to create regex rules: " << e.what() << std::endl;
        return std::nullopt;
    }
}

// [FIXED] 适配 cJSON 的实现
std::optional<std::vector<std::string>> Validator::loadValidTitles(const std::string& mappingFilePath) {
    auto jsonDataOpt = JsonReader::readFile(mappingFilePath);
    if (!jsonDataOpt) {
        std::cerr << "Error: [Validator] Could not read or parse mapping file at: " << mappingFilePath << std::endl;
        return std::nullopt;
    }

    // 获取 cJSON 指针
    cJSON* root = jsonDataOpt.value().get();

    if (!cJSON_IsObject(root)) {
        std::cerr << "Error: [Validator] Mapping file content is not a JSON object." << std::endl;
        return std::nullopt;
    }

    std::vector<std::string> titles;
    // 遍历 JSON 对象的所有 Key
    cJSON* child = root->child;
    while (child) {
        // cJSON 对象的 child->string 就是 Key (例如 "bp")
        if (child->string) {
            titles.push_back(child->string);
        }
        child = child->next;
    }
    return titles;
}