// src/reprocessor/validator/Validator.cpp

#include "Validator.hpp"
#include "common/JsonReader.hpp"
#include <fstream>
#include <iostream>
#include <sstream>

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

        if (line.empty()) continue;

        validateLine(line, state, rules, errorCount);
    }
    
    // [MODIFIED] 结束时检查是否连年份都没有
    if (state.currentState == StateType::EXPECTING_YEAR) {
        std::cerr << "Error: [Validator] File is empty or does not start with a year declaration (e.g., y2025)." << std::endl;
        errorCount++;
    }
    else if (state.lastDateLine > 0 && !state.contentSeenForDate) {
        std::cerr << "Error: [Validator] The last date entry at line " << state.lastDateLine
                  << " is empty and must contain at least one record." << std::endl;
        errorCount++;
    }
    else if (state.currentState == StateType::EXPECTING_CONTENT) {
         std::cerr << "Error: [Validator] File ends unexpectedly after a title on line " 
                   << state.lineCounter << ". Missing content line." << std::endl;
        errorCount++;
    }

    return errorCount == 0;
}

std::optional<Validator::ValidationRules> Validator::createRules(const std::vector<std::string>& validTitles) {
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
        // [MODIFIED] 添加了年份的正则表达式
        return ValidationRules{
            std::regex(R"(^y\d{4}$)"), // e.g., y2025
            std::regex(R"(^\d{4}$)"),
            std::regex(titleRegexPattern.str()),
            std::regex(R"(^\+\s*\d+(\.\d+)?\s+\d+(\s*\+\s*\d+)*$)")
        };
    } catch (const std::regex_error& e) {
        std::cerr << "Error: [Validator] Failed to create regex rules: " << e.what() << std::endl;
        return std::nullopt;
    }
}

void Validator::validateLine(const std::string& line, ValidationState& state, const ValidationRules& rules, int& errorCount) {
    // [NEW] 状态机现在从检查年份开始
    if (state.currentState == StateType::EXPECTING_YEAR) {
        if (std::regex_match(line, rules.yearRegex)) {
            state.currentState = StateType::EXPECTING_DATE; // 年份找到后，期望日期
            return;
        } else {
            std::cerr << "Error: [Validator] Invalid format at line " << state.lineCounter
                      << ". Expected a year declaration (e.g., y2025) at the beginning of the file." << std::endl;
            errorCount++;
            // 发生严重格式错误，后续校验可能无意义，可以考虑直接返回
            // 为了提供更多错误，我们暂时让它继续
            state.currentState = StateType::EXPECTING_DATE; // 假设年份缺失，继续往下找日期
            return;
        }
    }

    // [MODIFIED] 旧的逻辑被整合到新的状态机流程中
    if (std::regex_match(line, rules.dateRegex)) {
        if (state.lastDateLine > 0 && !state.contentSeenForDate) {
            std::cerr << "Error: [Validator] The date entry at line " << state.lastDateLine
                      << " is empty." << std::endl;
            errorCount++;
        }
        if (state.currentState == StateType::EXPECTING_CONTENT) {
            std::cerr << "Error: [Validator] Unexpected date at line " << state.lineCounter
                      << ". A content line was expected." << std::endl;
            errorCount++;
        }
        state.lastDateLine = state.lineCounter;
        state.contentSeenForDate = false;
        state.currentState = StateType::EXPECTING_TITLE;
        return;
    }

    if (line[0] == '+') {
        if (state.currentState == StateType::EXPECTING_YEAR || state.currentState == StateType::EXPECTING_DATE || state.currentState == StateType::EXPECTING_TITLE) {
            std::cerr << "Error: [Validator] Invalid format at line " << state.lineCounter
                      << ". Unexpected content line." << std::endl;
            errorCount++;
            return;
        }
        if (!std::regex_match(line, rules.contentRegex)) {
             std::cerr << "Error: [Validator] Malformed content line at " << state.lineCounter
                      << ": \"" << line << "\"" << std::endl;
            errorCount++;
        }
        state.contentSeenForDate = true;
        state.currentState = StateType::EXPECTING_TITLE_OR_CONTENT;
        return;
    }
    
    if (std::regex_match(line, rules.titleRegex)) {
        if (state.currentState == StateType::EXPECTING_YEAR || state.currentState == StateType::EXPECTING_DATE) {
             std::cerr << "Error: [Validator] Invalid format at line " << state.lineCounter
                      << ". Expected a date but found a title." << std::endl;
            errorCount++;
        } else if (state.currentState == StateType::EXPECTING_CONTENT) {
             std::cerr << "Error: [Validator] Invalid format at line " << state.lineCounter
                      << ". Expected a content line but found another title." << std::endl;
            errorCount++;
        }
        state.currentState = StateType::EXPECTING_CONTENT;
        return;
    }

    std::cerr << "Error: [Validator] Unrecognized format at line " << state.lineCounter << ": \"" << line << "\"" << std::endl;
    errorCount++;
}

// loadValidTitles 函数保持不变
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