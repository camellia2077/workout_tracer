// reprocessor/preprocessor/name_mapper/ProjectNameMapper.hpp

#ifndef NAME_MAPPER_H
#define NAME_MAPPER_H

#include <string>
#include <map>

#include <cjson/cJSON.h> 

// 用于存储映射信息的结构体
struct ProjectMapping {
    std::string fullName;
    std::string type;
};

class ProjectNameMapper {
public:
    // [MODIFIED] 函数签名更新为接收 cJSON 指针
    bool loadMappings(const cJSON* jsonData);

    ProjectMapping getMapping(const std::string& shortName) const;

private:
    std::map<std::string, ProjectMapping> mappings;
};

#endif // NAME_MAPPER_H