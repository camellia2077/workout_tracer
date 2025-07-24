#include "Validator.h"
#include "common/JsonReader.h"
#include <fstream>
#include <iostream>
#include <sstream>

// --- loadValidTitles 和 createRules 函数保持不变 ---

std::optional<std::vector<std::string>> Validator::loadValidTitles(const std::string& mappingFilePath) {
    // ... (无改动)
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
    // ... (无改动)
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

// 已修改：此函数现在可以正确处理无效标题后的状态转换
void Validator::validateLine(const std::string& line, ValidationState& state, const ValidationRules& rules, int& errorCount) {
    // 状态 1: 期望一个日期行
    if (state.expectingDate) {
        if (std::regex_match(line, rules.dateRegex)) {
            state.expectingDate = false;
            state.expectingTitleOrDate = true;
        } else {
            std::cerr << "Error: [Validator] Invalid format at line " << state.lineCounter
                      << ". Expected a date (e.g., '0704'), but found: \"" << line << "\"" << std::endl;
            errorCount++;
        }
        return; 
    }

    // 状态 2: 期望一个标题行或新的日期行
    if (state.expectingTitleOrDate) {
        if (std::regex_match(line, rules.dateRegex)) {
            state.expectingTitleOrDate = true; // 状态不变，下一行继续期望标题或新日期
        } else if (std::regex_match(line, rules.titleRegex)) {
            state.expectingTitleOrDate = false; // 成功匹配标题，下一行期望内容
        } else {
            std::cerr << "Error: [Validator] Invalid format at line " << state.lineCounter
                      << ". Expected a valid title (e.g., 'bp') or a new date, but found: \"" << line << "\"" << std::endl;
            errorCount++;
            // **关键修复点**：即使标题无效，也认为它消耗了一个标题槽位，
            // 因此下一行应该期望是内容，而不是另一个标题。
            state.expectingTitleOrDate = false;
        }
        return; 
    }

    // 状态 3: (默认) 期望一个内容行
    if (std::regex_match(line, rules.contentRegex)) {
        state.expectingTitleOrDate = true; // 内容之后，期望新标题或新日期
    } else {
        std::cerr << "Error: [Validator] Invalid format at line " << state.lineCounter
                  << ". Expected a content line (e.g., '+60 10+10'), but found: \"" << line << "\"" << std::endl;
        errorCount++;
        // **容错**：如果内容行也错了，我们假定这个错误日志条目结束，
        // 并开始期待下一个是新的标题或日期。
        state.expectingTitleOrDate = true; 
    }
}


// --- 主函数实现 (无改动) ---

bool Validator::validate(const std::string& logFilePath, const std::string& mappingFilePath) {
    // ... (无改动)
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
    std::string line;
    int errorCount = 0;

    while (std::getline(file, line)) {
        state.lineCounter++;
        line.erase(0, line.find_first_not_of(" \t\n\r"));
        line.erase(line.find_last_not_of(" \t\n\r") + 1);

        if (line.empty()) {
            continue;
        }

        validateLine(line, state, rules, errorCount);
    }

    if (state.expectingTitleOrDate && !state.expectingDate) {
        std::cerr << "Error: [Validator] File ended unexpectedly. The last title is missing its content line." << std::endl;
        errorCount++;
    }

    return errorCount == 0;
}