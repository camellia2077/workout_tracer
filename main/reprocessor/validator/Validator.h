#ifndef VALIDATOR_H
#define VALIDATOR_H

#include <string>
#include <vector>
#include <optional>
#include <regex> // 新增

class Validator {
public:
    static bool validate(const std::string& logFilePath, const std::string& mappingFilePath);

private:
    // 内部结构体，用于封装验证规则
    struct ValidationRules {
        std::regex dateRegex;
        std::regex titleRegex;
        std::regex contentRegex;
    };

    // 内部结构体，用于封装验证状态
    struct ValidationState {
        bool expectingDate = true;
        bool expectingTitleOrDate = false;
        int lineCounter = 0;
    };

    // 加载项目缩写
    static std::optional<std::vector<std::string>> loadValidTitles(const std::string& mappingFilePath);

    // 根据加载的标题创建验证规则
    static std::optional<ValidationRules> createRules(const std::vector<std::string>& validTitles);

    // 对单行进行验证，并更新状态
    static bool validateLine(const std::string& line, ValidationState& state, const ValidationRules& rules);
};

#endif // VALIDATOR_H