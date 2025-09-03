// src/reprocessor/validator/Validator.hpp

#ifndef VALIDATOR_H
#define VALIDATOR_H

#include "LineValidator.hpp" // 引入新的头文件
#include <string>
#include <vector>
#include <optional>

class Validator {
public:
    // 公共接口保持不变
    static bool validate(const std::string& logFilePath, const std::string& mappingFilePath);

private:
    // 私有辅助函数负责规则创建和标题加载
    static std::optional<std::vector<std::string>> loadValidTitles(const std::string& mappingFilePath);
    static std::optional<ValidationRules> createRules(const std::vector<std::string>& validTitles);
};

#endif // VALIDATOR_H