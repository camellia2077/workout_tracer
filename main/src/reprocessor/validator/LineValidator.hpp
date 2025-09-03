// src/reprocessor/validator/LineValidator.hpp

#ifndef LINE_VALIDATOR_H
#define LINE_VALIDATOR_H

#include <string>
#include <regex>
#include <optional>

// 从旧 Validator.hpp 中移动过来的定义
enum class StateType {
    EXPECTING_YEAR,
    EXPECTING_DATE,
    EXPECTING_TITLE,
    EXPECTING_CONTENT,
    EXPECTING_TITLE_OR_CONTENT
};

struct ValidationRules {
    std::regex yearRegex;
    std::regex dateRegex;
    std::regex titleRegex;
    std::regex contentRegex;
};

class LineValidator {
public:
    LineValidator();

    // 验证单行，并更新内部状态
    void validateLine(const std::string& line, const ValidationRules& rules, int& errorCount);

    // 在文件末尾进行最终检查
    void finalizeValidation(int& errorCount);

private:
    struct ValidationState {
        StateType currentState = StateType::EXPECTING_YEAR;
        int lineCounter = 0;
        bool contentSeenForDate = false;
        int lastDateLine = 0;
    };

    ValidationState state_;
};

#endif // LINE_VALIDATOR_H