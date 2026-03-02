#include "Validator.h"
#include "common/JsonReader.h" // <-- ADDED: Required for JSON parsing
#include <fstream>
#include <regex>
#include <iostream>
#include <optional>   // <-- ADDED: Needed for std::optional
#include <sstream>    // <-- ADDED: Needed for std::stringstream

// 实现加载标题的私有方法
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

// MODIFIED: Corrected the function signature to match Validator.h
bool Validator::validate(const std::string& logFilePath, const std::string& mappingFilePath) {
    // 步骤 1: 加载有效的标题
    auto validTitlesOpt = loadValidTitles(mappingFilePath);
    if (!validTitlesOpt) {
        return false; // 加载失败，直接返回
    }
    const auto& validTitles = validTitlesOpt.value();

    // MODIFIED: Open the correct file passed as parameter
    std::ifstream file(logFilePath);
    if (!file.is_open()) {
        std::cerr << "Error: [Validator] Could not open file " << logFilePath << std::endl;
        return false;
    }

    std::string line;
    int lineCounter = 0;
    bool expectingDate = true;
    bool expectingTitleOrDate = false;

    // --- 动态构建标题的正则表达式 ---
    if (validTitles.empty()) {
        std::cerr << "Warning: [Validator] No valid titles found in mapping file. Validation might not be accurate." << std::endl;
    }
    std::stringstream titleRegexPattern;
    titleRegexPattern << "^(";
    for (size_t i = 0; i < validTitles.size(); ++i) {
        // Escape special regex characters if any, though not expected for current titles
        titleRegexPattern << validTitles[i] << (i < validTitles.size() - 1 ? "|" : "");
    }
    titleRegexPattern << ")$";
    const std::regex titleRegex(titleRegexPattern.str()); // This is the dynamic regex
    // --- 正则表达式构建结束 ---

    const std::regex dateRegex(R"(^\d{4}$)");
    const std::regex contentRegex(R"(^\+\s*\d+(\.\d+)?\s+\d+(\s*\+\s*\d+)*$)");

    while (std::getline(file, line)) {
        lineCounter++;
        line.erase(0, line.find_first_not_of(" \t\n\r"));
        line.erase(line.find_last_not_of(" \t\n\r") + 1);

        if (line.empty()) {
            continue;
        }

        if (expectingDate) {
            if (std::regex_match(line, dateRegex)) {
                expectingDate = false;
                expectingTitleOrDate = true;
                continue;
            } else {
                std::cerr << "Error: [Validator] Invalid format at line " << lineCounter
                          << ". Expected a date (e.g., '0704'), but found: \"" << line << "\"" << std::endl;
                return false;
            }
        }

        if (expectingTitleOrDate) {
            if (std::regex_match(line, dateRegex)) {
                expectingDate = false;
                expectingTitleOrDate = true;
                continue;
            }
            // MODIFIED: Use the dynamically generated regex for titles
            if (std::regex_match(line, titleRegex)) {
                expectingTitleOrDate = false;
                continue;
            }
            else {
                std::cerr << "Error: [Validator] Invalid format at line " << lineCounter
                          << ". Expected a valid title (like 'bp', 'sq', etc.) or a new date, but found: \"" << line << "\"" << std::endl;
                return false;
            }
        }
        
        if (std::regex_match(line, contentRegex)) {
            expectingTitleOrDate = true;
            continue;
        }

        std::cerr << "Error: [Validator] Invalid format at line " << lineCounter
                  << ". Expected a content line (e.g., '+60 10+10'), but found: \"" << line << "\"" << std::endl;
        return false;
    }

    if (expectingTitleOrDate && !expectingDate) {
         std::cerr << "Error: [Validator] File ended unexpectedly. The last title is missing its content line." << std::endl;
         return false;
    }
    
    return true;
}