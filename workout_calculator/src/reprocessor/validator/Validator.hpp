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
    struct ValidationRules {
        std::regex dateRegex;
        std::regex titleRegex;
        std::regex contentRegex;
    };

    // 已修改：添加了新的状态字段
    struct ValidationState {
        bool expectingDate = true;
        bool expectingTitleOrDate = false;
        int lineCounter = 0;
        bool contentSeenForDate = false; // 新增：标记当前日期是否已有内容
        int lastDateLine = 0;            // 新增：记录上一个日期行的行号
    };

    static std::optional<std::vector<std::string>> loadValidTitles(const std::string& mappingFilePath);
    static std::optional<ValidationRules> createRules(const std::vector<std::string>& validTitles);
    static void validateLine(const std::string& line, ValidationState& state, const ValidationRules& rules, int& errorCount);
};

#endif // VALIDATOR_H