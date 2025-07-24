#include "Validator.h"
#include "common/JsonReader.h"
#include <fstream>
#include <iostream>
#include <sstream>

// --- 辅助函数的实现 ---

// 从 mapping.json 加载有效的项目缩写
std::optional<std::vector<std::string>> Validator::loadValidTitles(const std::string& mappingFilePath) {
    auto jsonDataOpt = JsonReader::readFile(mappingFilePath);
    if (!jsonDataOpt) {
        std::cerr << "Error: [Validator] Could not read or parse mapping file at: " << mappingFilePath << std::endl;
        return std::nullopt;
    }
    if (!jsonDataOpt.value().is_object()) {
        std::cerr << "Error: [Validator] Mapping file content is not a JSON object." << std::endl;
        return std::nullopt;
    }
    std::vector<std::string> titles;
    for (auto& el : jsonDataOpt.value().items()) {
        titles.push_back(el.key());
    }
    return titles;
}

// 根据项目缩写列表创建编译好的正则表达式规则
std::optional<Validator::ValidationRules> Validator::createRules(const std::vector<std::string>& validTitles) {
    if (validTitles.empty()) {
        std::cerr << "Warning: [Validator] No valid titles found in mapping file. Validation might not be accurate." << std::endl;
    }

    std::stringstream titleRegexPattern;
    titleRegexPattern << "^(";
    for (size_t i = 0; i < validTitles.size(); ++i) {
        titleRegexPattern << validTitles[i] << (i < validTitles.size() - 1 ? "|" : "");
    }
    titleRegexPattern << ")$";

    try {
        return ValidationRules{
            std::regex(R"(^\d{4}$)"),
            std::regex(titleRegexPattern.str()),
            std::regex(R"(^\+\s*\d+(\.\d+)?\s+\d+(\s*\+\s*\d+)*$)")
        };
    } catch (const std::regex_error& e) {
        std::cerr << "Error: [Validator] Failed to create regex rules: " << e.what() << std::endl;
        return std::nullopt;
    }
}

// 核心验证逻辑：只验证单行，并更新状态
bool Validator::validateLine(const std::string& line, ValidationState& state, const ValidationRules& rules) {
    // 状态1: 期望一个日期行
    if (state.expectingDate) {
        if (std::regex_match(line, rules.dateRegex)) {
            state.expectingDate = false;
            state.expectingTitleOrDate = true;
            return true;
        } else {
            std::cerr << "Error: [Validator] Invalid format at line " << state.lineCounter
                      << ". Expected a date (e.g., '0704'), but found: \"" << line << "\"" << std::endl;
            return false;
        }
    }

    // 状态2: 期望一个标题行或新的日期行
    if (state.expectingTitleOrDate) {
        if (std::regex_match(line, rules.dateRegex)) { // 新的日期
            state.expectingTitleOrDate = true;
            return true;
        }
        if (std::regex_match(line, rules.titleRegex)) { // 标题
            state.expectingTitleOrDate = false;
            return true;
        }
        std::cerr << "Error: [Validator] Invalid format at line " << state.lineCounter
                  << ". Expected a valid title (e.g., 'bp') or a new date, but found: \"" << line << "\"" << std::endl;
        return false;
    }

    // 状态3: (默认) 期望一个内容行
    if (std::regex_match(line, rules.contentRegex)) {
        state.expectingTitleOrDate = true;
        return true;
    }

    // 如果代码执行到这里，说明当前行不符合任何预期的格式
    std::cerr << "Error: [Validator] Invalid format at line " << state.lineCounter
              << ". Expected a content line (e.g., '+60 10+10'), but found: \"" << line << "\"" << std::endl;
    return false;
}


// --- 主函数的实现 ---

bool Validator::validate(const std::string& logFilePath, const std::string& mappingFilePath) {
    // 1. 加载和创建规则
    auto validTitlesOpt = loadValidTitles(mappingFilePath);
    if (!validTitlesOpt) return false;

    auto rulesOpt = createRules(validTitlesOpt.value());
    if (!rulesOpt) return false;
    const auto& rules = rulesOpt.value();

    // 2. 设置初始状态并打开文件
    std::ifstream file(logFilePath);
    if (!file.is_open()) {
        std::cerr << "Error: [Validator] Could not open file " << logFilePath << std::endl;
        return false;
    }

    ValidationState state;
    std::string line;

    // 3. 循环，将验证工作委托给 validateLine
    while (std::getline(file, line)) {
        state.lineCounter++;
        line.erase(0, line.find_first_not_of(" \t\n\r"));
        line.erase(line.find_last_not_of(" \t\n\r") + 1);

        if (line.empty()) {
            continue;
        }

        if (!validateLine(line, state, rules)) {
            return false; // 如果单行验证失败，则整个文件验证失败
        }
    }

    // 4. 文件末尾的最终状态检查
    if (state.expectingTitleOrDate && !state.expectingDate) {
        std::cerr << "Error: [Validator] File ended unexpectedly. The last title is missing its content line." << std::endl;
        return false;
    }

    return true;
}