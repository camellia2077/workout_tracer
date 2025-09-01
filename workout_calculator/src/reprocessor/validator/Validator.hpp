// src/reprocessor/validator/Validator.hpp

#ifndef VALIDATOR_H
#define VALIDATOR_H

#include <string>
#include <vector>
#include <optional>
#include <regex>

// [MODIFIED] 添加了 EXPECTING_YEAR 状态
enum class StateType {
    EXPECTING_YEAR,
    EXPECTING_DATE,
    EXPECTING_TITLE,
    EXPECTING_CONTENT,
    EXPECTING_TITLE_OR_CONTENT
};


class Validator {
public:
    static bool validate(const std::string& logFilePath, const std::string& mappingFilePath);

private:
    struct ValidationRules {
        // [NEW] 新增年份的正则表达式
        std::regex yearRegex;
        std::regex dateRegex;
        std::regex titleRegex;
        std::regex contentRegex;
    };

    struct ValidationState {
        // [MODIFIED] 初始状态现在是期望年份
        StateType currentState = StateType::EXPECTING_YEAR;
        int lineCounter = 0;
        bool contentSeenForDate = false;
        int lastDateLine = 0;
    };

    static std::optional<std::vector<std::string>> loadValidTitles(const std::string& mappingFilePath);
    static std::optional<ValidationRules> createRules(const std::vector<std::string>& validTitles);
    static void validateLine(const std::string& line, ValidationState& state, const ValidationRules& rules, int& errorCount);
};

#endif // VALIDATOR_H