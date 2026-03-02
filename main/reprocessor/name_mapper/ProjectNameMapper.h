#ifndef NAME_MAPPER_H
#define NAME_MAPPER_H

#include <string>
#include <map>
#include "nlohmann/json.hpp" // 需要包含json头文件

class ProjectNameMapper {
public:
    /**
     * @brief 从一个已经解析的JSON对象中加载映射规则。
     * @param jsonData 包含映射关系的nlohmann::json对象。
     * @return 如果加载成功返回true，否则返回false。
     */
    bool loadMappings(const nlohmann::json& jsonData);

    // 获取映射后的全名。如果找不到映射，则返回原始名称。
    std::string getFullName(const std::string& shortName) const;

private:
    std::map<std::string, std::string> mappings;
};

#endif // NAME_MAPPER_H