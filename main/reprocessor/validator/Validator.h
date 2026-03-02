#ifndef VALIDATOR_H
#define VALIDATOR_H

#include <string>
#include <vector>
#include <optional> // <-- ADDED: Needed for std::optional

class Validator {
public:
    /**
     * @brief 验证原始日志文件的格式是否符合规范。
     * @param logFilePath 要验证的日志文件路径。
     * @param mappingFilePath 用于提取有效项目名称的 mapping.json 文件路径。
     * @return 如果文件格式有效则返回true，否则返回false。
     */
    static bool validate(const std::string& logFilePath, const std::string& mappingFilePath);

private:
    // 新增一个私有辅助函数，用于从json文件加载标题
    static std::optional<std::vector<std::string>> loadValidTitles(const std::string& mappingFilePath);
};

#endif // VALIDATOR_H