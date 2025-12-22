// reprocessor/preprocessor/name_mapper/ProjectNameMapper.hpp

#ifndef NAME_MAPPER_H
#define NAME_MAPPER_H

#include <string>
#include <map>
#include "nlohmann/json.hpp"

// <<< 新增：用于存储映射信息的结构体
struct ProjectMapping {
    std::string fullName;
    std::string type;
};

class ProjectNameMapper {
public:
    bool loadMappings(const nlohmann::json& jsonData);

    // <<< 修改：返回包含全名和类型的结构体
    ProjectMapping getMapping(const std::string& shortName) const;

private:
    std::map<std::string, ProjectMapping> mappings;
};

#endif // NAME_MAPPER_H