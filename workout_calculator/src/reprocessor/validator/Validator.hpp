// src/reprocessor/validator/Validator.hpp

#ifndef VALIDATOR_H
#define VALIDATOR_H

#include <string>
#include <vector>
#include <optional>
#include <regex>

// 新增：使用枚举来清晰地表示状态
enum class StateType {
    EXPECTING_DATE,             // 初始状态，或刚处理完一个完整记录后，期望新日期
    EXPECTING_TITLE,            // 刚遇到新日期后，期望项目标题
    EXPECTING_CONTENT,          // 刚遇到项目标题后，期望至少一个内容行
    EXPECTING_TITLE_OR_CONTENT  // 刚遇到一个内容行后，后面可以跟另一个内容行或一个新标题
};


class Validator {
public:
    /**
     * @brief (主接口) 验证日志文件的格式是否有效。
     * @param logFilePath 要验证的日志文件路径。
     * @param mappingFilePath 用于验证项目名称的映射文件路径。
     * @return 如果文件格式有效则返回true，否则返回false。
     */
    static bool validate(const std::string& logFilePath, const std::string& mappingFilePath);

private:
    // 存储正则表达式规则的结构体
    struct ValidationRules {
        std::regex dateRegex;
        std::regex titleRegex;
        std::regex contentRegex;
    };

    // 存储验证过程中的状态
    struct ValidationState {
        StateType currentState = StateType::EXPECTING_DATE; // 初始状态总是期望一个日期
        int lineCounter = 0;
        bool contentSeenForDate = false; // 标记当前日期是否已有内容
        int lastDateLine = 0;            // 记录上一个日期行的行号
    };

    // 从 mapping.json 加载有效标题
    static std::optional<std::vector<std::string>> loadValidTitles(const std::string& mappingFilePath);
    
    // 根据有效标题创建正则表达式规则
    static std::optional<ValidationRules> createRules(const std::vector<std::string>& validTitles);

    // 核心验证逻辑：验证单行内容并更新状态
    static void validateLine(const std::string& line, ValidationState& state, const ValidationRules& rules, int& errorCount);
};

#endif // VALIDATOR_H