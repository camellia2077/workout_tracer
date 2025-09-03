// src/reprocessor/validator/LineValidator.cpp

#include "LineValidator.hpp"
#include <iostream>

LineValidator::LineValidator() {}

void LineValidator::validateLine(const std::string& line, const ValidationRules& rules, int& errorCount) {
    state_.lineCounter++;
    
    // --- 将 Validator.cpp 中的 validateLine 逻辑完整地移动到这里 ---
    // [NEW] 状态机现在从检查年份开始
    if (state_.currentState == StateType::EXPECTING_YEAR) {
        if (std::regex_match(line, rules.yearRegex)) {
            state_.currentState = StateType::EXPECTING_DATE;
            return;
        } else {
            std::cerr << "Error: [Validator] Invalid format at line " << state_.lineCounter
                      << ". Expected a year declaration (e.g., y2025) at the beginning of the file." << std::endl;
            errorCount++;
            state_.currentState = StateType::EXPECTING_DATE; 
            return;
        }
    }

    if (std::regex_match(line, rules.dateRegex)) {
        if (state_.lastDateLine > 0 && !state_.contentSeenForDate) {
            std::cerr << "Error: [Validator] The date entry at line " << state_.lastDateLine
                      << " is empty." << std::endl;
            errorCount++;
        }
        if (state_.currentState == StateType::EXPECTING_CONTENT) {
            std::cerr << "Error: [Validator] Unexpected date at line " << state_.lineCounter
                      << ". A content line was expected." << std::endl;
            errorCount++;
        }
        state_.lastDateLine = state_.lineCounter;
        state_.contentSeenForDate = false;
        state_.currentState = StateType::EXPECTING_TITLE;
        return;
    }

    if (line[0] == '+') {
        if (state_.currentState == StateType::EXPECTING_YEAR || state_.currentState == StateType::EXPECTING_DATE || state_.currentState == StateType::EXPECTING_TITLE) {
            std::cerr << "Error: [Validator] Invalid format at line " << state_.lineCounter
                      << ". Unexpected content line." << std::endl;
            errorCount++;
            return;
        }
        if (!std::regex_match(line, rules.contentRegex)) {
             std::cerr << "Error: [Validator] Malformed content line at " << state_.lineCounter
                      << ": \"" << line << "\"" << std::endl;
            errorCount++;
        }
        state_.contentSeenForDate = true;
        state_.currentState = StateType::EXPECTING_TITLE_OR_CONTENT;
        return;
    }
    
    if (std::regex_match(line, rules.titleRegex)) {
        if (state_.currentState == StateType::EXPECTING_YEAR || state_.currentState == StateType::EXPECTING_DATE) {
             std::cerr << "Error: [Validator] Invalid format at line " << state_.lineCounter
                      << ". Expected a date but found a title." << std::endl;
            errorCount++;
        } else if (state_.currentState == StateType::EXPECTING_CONTENT) {
             std::cerr << "Error: [Validator] Invalid format at line " << state_.lineCounter
                      << ". Expected a content line but found another title." << std::endl;
            errorCount++;
        }
        state_.currentState = StateType::EXPECTING_CONTENT;
        return;
    }

    std::cerr << "Error: [Validator] Unrecognized format at line " << state_.lineCounter << ": \"" << line << "\"" << std::endl;
    errorCount++;
}

void LineValidator::finalizeValidation(int& errorCount) {
    // --- 将 Validator.cpp 中文件末尾的检查逻辑移动到这里 ---
    if (state_.currentState == StateType::EXPECTING_YEAR) {
        std::cerr << "Error: [Validator] File is empty or does not start with a year declaration (e.g., y2025)." << std::endl;
        errorCount++;
    }
    else if (state_.lastDateLine > 0 && !state_.contentSeenForDate) {
        std::cerr << "Error: [Validator] The last date entry at line " << state_.lastDateLine
                  << " is empty and must contain at least one record." << std::endl;
        errorCount++;
    }
    else if (state_.currentState == StateType::EXPECTING_CONTENT) {
         std::cerr << "Error: [Validator] File ends unexpectedly after a title on line " 
                   << state_.lineCounter << ". Missing content line." << std::endl;
        errorCount++;
    }
}