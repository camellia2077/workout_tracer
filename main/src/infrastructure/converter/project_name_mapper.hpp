// converter/project_name_mapper.hpp

#ifndef CONVERTER_PROJECT_NAME_MAPPER_HPP_
#define CONVERTER_PROJECT_NAME_MAPPER_HPP_

#include <map>
#include <string>

#include <cjson/cJSON.h>

// 用于存储映射信息的结构体
struct ProjectMapping {
  std::string full_name;
  std::string type;
};

class ProjectNameMapper {
public:
  // [MODIFIED] 函数签名更新为接收 cJSON 指针
  [[nodiscard]] auto LoadMappings(const cJSON* json_root) -> bool;

  [[nodiscard]] auto GetMapping(const std::string& short_name) const -> ProjectMapping;

private:
  std::map<std::string, ProjectMapping> mappings_;
};

#endif // CONVERTER_PROJECT_NAME_MAPPER_HPP_