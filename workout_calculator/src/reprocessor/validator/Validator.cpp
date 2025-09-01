// src/reprocessor/validator/Validator.cpp

#include "Validator.hpp"
#include "common/JsonReader.hpp"
#include <fstream>
#include <iostream>
#include <sstream>

/**
 * @brief (主函数) 验证日志文件的完整流程。
 */
bool Validator::validate(const std::string& logFilePath, const std::string& mappingFilePath) {
    // 步骤 1: 加载并解析 mapping.json 以获取所有合法的项目简称
    auto validTitlesOpt = loadValidTitles(mappingFilePath);
    if (!validTitlesOpt) {
        // 如果无法加载标题，验证无法继续
        return false;
    }

    // 步骤 2: 根据加载的标题动态创建正则表达式规则
    auto rulesOpt = createRules(validTitlesOpt.value());
    if (!rulesOpt) {
        // 如果无法创建规则，验证无法继续
        return false;
    }
    const auto& rules = rulesOpt.value();

    // 步骤 3: 打开要验证的日志文件
    std::ifstream file(logFilePath);
    if (!file.is_open()) {
        std::cerr << "Error: [Validator] Could not open file " << logFilePath << std::endl;
        return false;
    }

    // 步骤 4: 初始化状态机和错误计数器
    ValidationState state;
    int errorCount = 0;
    std::string line;

    // 步骤 5: 逐行读取文件并进行验证
    while (std::getline(file, line)) {
        state.lineCounter++;
        // 预处理：移除行首和行尾的空白字符
        line.erase(0, line.find_first_not_of(" \t\n\r"));
        line.erase(line.find_last_not_of(" \t\n\r") + 1);

        if (line.empty()) {
            continue; // 跳过空行
        }

        // 调用核心辅助函数来处理当前行
        validateLine(line, state, rules, errorCount);
    }

    // 步骤 6: 文件读取完毕后，进行最终检查
    // 检查最后一个日期段是否为空（即只有日期行，没有任何内容）
    if (state.lastDateLine > 0 && !state.contentSeenForDate) {
        std::cerr << "Error: [Validator] The last date entry at line " << state.lastDateLine
                  << " is empty and must contain at least one record." << std::endl;
        errorCount++;
    }
    // 检查文件是否以一个未完成的记录结尾（例如，只有标题没有内容）
    if (state.currentState == StateType::EXPECTING_CONTENT) {
         std::cerr << "Error: [Validator] File ends unexpectedly after a title on line " 
                   << state.lineCounter << ". Missing content line." << std::endl;
        errorCount++;
    }

    // 步骤 7: 根据错误计数返回最终结果
    return errorCount == 0;
}


// --- 私有辅助函数的实现 ---

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

void Validator::validateLine(const std::string& line, ValidationState& state, const ValidationRules& rules, int& errorCount) {
    // 情况 1: 检查是否是日期行
    if (std::regex_match(line, rules.dateRegex)) {
        // 在遇到新日期之前，检查上一个日期段是否有效
        if (state.lastDateLine > 0 && !state.contentSeenForDate) {
            std::cerr << "Error: [Validator] The date entry at line " << state.lastDateLine
                      << " is empty and must contain at least one record." << std::endl;
            errorCount++;
        }
        if (state.currentState == StateType::EXPECTING_CONTENT) {
            std::cerr << "Error: [Validator] Unexpected date at line " << state.lineCounter
                      << ". A content line was expected after the previous title." << std::endl;
            errorCount++;
        }
        // 重置状态以迎接新的一天
        state.lastDateLine = state.lineCounter;
        state.contentSeenForDate = false;
        state.currentState = StateType::EXPECTING_TITLE; // 新的一天，期望一个标题
        return;
    }

    // 情况 2: 检查是否是内容行 (以'+'开头)
    if (line[0] == '+') {
        if (state.currentState == StateType::EXPECTING_DATE || state.currentState == StateType::EXPECTING_TITLE) {
            std::cerr << "Error: [Validator] Invalid format at line " << state.lineCounter
                      << ". Unexpected content line. Expected a " 
                      << (state.currentState == StateType::EXPECTING_DATE ? "date" : "title")
                      << " first." << std::endl;
            errorCount++;
            return; // 提前返回以避免进一步的逻辑错误
        }
        if (!std::regex_match(line, rules.contentRegex)) {
             std::cerr << "Error: [Validator] Invalid format at line " << state.lineCounter
                      << ". Malformed content line: \"" << line << "\"" << std::endl;
            errorCount++;
        }
        state.contentSeenForDate = true;
        // 关键修改: 在内容行之后，可以接受另一个内容行或一个新的标题
        state.currentState = StateType::EXPECTING_TITLE_OR_CONTENT;
        return;
    }
    
    // 情况 3: 检查是否是标题行
    if (std::regex_match(line, rules.titleRegex)) {
        if (state.currentState == StateType::EXPECTING_DATE) {
             std::cerr << "Error: [Validator] Invalid format at line " << state.lineCounter
                      << ". Expected a date but found a title." << std::endl;
            errorCount++;
        } else if (state.currentState == StateType::EXPECTING_CONTENT) {
             std::cerr << "Error: [Validator] Invalid format at line " << state.lineCounter
                      << ". Expected a content line but found another title." << std::endl;
            errorCount++;
        }
        state.currentState = StateType::EXPECTING_CONTENT; // 标题之后，必须跟一个内容行
        return;
    }

    // 情况 4: 如果以上都不是，则格式错误
    std::cerr << "Error: [Validator] Unrecognized format at line " << state.lineCounter << ": \"" << line << "\"" << std::endl;
    errorCount++;
    // 进入容错状态，假定下一行可以是新标题或新内容
    state.currentState = StateType::EXPECTING_TITLE_OR_CONTENT;
}