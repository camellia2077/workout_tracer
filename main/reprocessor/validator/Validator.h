#ifndef VALIDATOR_H
#define VALIDATOR_H

#include <string>
#include <vector>
#include <optional>
#include <regex>

class Validator {
public:
    static bool validate(const std::string& logFilePath, const std::string& mappingFilePath);

private:
    // ... (ValidationRules and ValidationState structs remain the same) ...
    struct ValidationRules {
        std::regex dateRegex;
        std::regex titleRegex;
        std::regex contentRegex;
    };

    struct ValidationState {
        bool expectingDate = true;
        bool expectingTitleOrDate = false;
        int lineCounter = 0;
    };

    static std::optional<std::vector<std::string>> loadValidTitles(const std::string& mappingFilePath);
    static std::optional<ValidationRules> createRules(const std::vector<std::string>& validTitles);

    /**
     * @brief 对单行进行验证，并在发现错误时增加错误计数。
     * @param line 当前要验证的行。
     * @param state 当前的验证状态（将被修改）。
     * @param rules 包含正则表达式的规则。
     * @param errorCount 错误计数器的引用（将被修改）。
     */
    static void validateLine(const std::string& line, ValidationState& state, const ValidationRules& rules, int& errorCount); // MODIFIED
};

#endif // VALIDATOR_H