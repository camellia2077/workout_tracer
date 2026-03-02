#include "Validator.hpp"
#include "common/JsonReader.hpp"
#include <fstream>
#include <iostream>
#include <sstream>

// --- loadValidTitles 和 createRules 函数保持不变 ---

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

// 已修改：实现了新的验证逻辑
void Validator::validateLine(const std::string& line, ValidationState& state, const ValidationRules& rules, int& errorCount) {
    // 检查是否是日期行
    if (std::regex_match(line, rules.dateRegex)) {
        // 如果这不是文件中的第一个日期，则检查上一个日期段是否有效
        if (state.lastDateLine > 0 && !state.contentSeenForDate) {
            std::cerr << "Error: [Validator] The date entry at line " << state.lastDateLine
                      << " is empty and must contain at least one record." << std::endl;
            errorCount++;
        }
        // 重置状态以迎接新的一天
        state.lastDateLine = state.lineCounter;
        state.contentSeenForDate = false;
        state.expectingDate = false;
        state.expectingTitleOrDate = true;
        return;
    }

    // 如果程序期望一个日期但得到的不是，则报错
    if (state.expectingDate) {
        std::cerr << "Error: [Validator] Invalid format at line " << state.lineCounter
                  << ". Expected a date (e.g., '0704'), but found: \"" << line << "\"" << std::endl;
        errorCount++;
        return;
    }

    // 状态 2: 期望一个标题行
    if (state.expectingTitleOrDate) {
        if (std::regex_match(line, rules.titleRegex)) {
            state.expectingTitleOrDate = false; // 成功匹配标题，下一行期望内容
        } else {
            std::cerr << "Error: [Validator] Invalid format at line " << state.lineCounter
                      << ". Expected a valid title (e.g., 'bp'), but found: \"" << line << "\"" << std::endl;
            errorCount++;
            state.expectingTitleOrDate = false; // 容错：假定这个错误行消耗了标题槽位
        }
        return;
    }

    // 状态 3: (默认) 期望一个内容行
    if (std::regex_match(line, rules.contentRegex)) {
        state.contentSeenForDate = true;     // **关键**：标记此日期已有内容
        state.expectingTitleOrDate = true; // 内容之后，期望新标题
    } else {
        std::cerr << "Error: [Validator] Invalid format at line " << state.lineCounter
                  << ". Expected a content line (e.g., '+60 10+10'), but found: \"" << line << "\"" << std::endl;
        errorCount++;
        state.expectingTitleOrDate = true; // 容错：假定这个错误行消耗了内容槽位
    }
}

// --- 主函数实现 ---
bool Validator::validate(const std::string& logFilePath, const std::string& mappingFilePath) {
    auto validTitlesOpt = loadValidTitles(mappingFilePath);
    if (!validTitlesOpt) return false;

    auto rulesOpt = createRules(validTitlesOpt.value());
    if (!rulesOpt) return false;
    const auto& rules = rulesOpt.value();

    std::ifstream file(logFilePath);
    if (!file.is_open()) {
        std::cerr << "Error: [Validator] Could not open file " << logFilePath << std::endl;
        return false;
    }

    ValidationState state;
    int errorCount = 0;
    std::string line;

    while (std::getline(file, line)) {
        state.lineCounter++;
        line.erase(0, line.find_first_not_of(" \t\n\r"));
        line.erase(line.find_last_not_of(" \t\n\r") + 1);

        if (line.empty()) {
            continue;
        }

        validateLine(line, state, rules, errorCount);
    }

    // 已修改：文件末尾的最终检查
    // 1. 移除对“dangling title”的检查
    // 2. 检查最后一个日期段是否为空
    if (state.lastDateLine > 0 && !state.contentSeenForDate) {
        std::cerr << "Error: [Validator] The last date entry at line " << state.lastDateLine
                  << " is empty and must contain at least one record." << std::endl;
        errorCount++;
    }

    return errorCount == 0;
}