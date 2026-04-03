// converter/project_name_mapper.hpp

#ifndef CONVERTER_PROJECT_NAME_MAPPER_HPP_
#define CONVERTER_PROJECT_NAME_MAPPER_HPP_

#include <map>
#include <string>

#include "application/interfaces/mapping_config.hpp"

class ProjectNameMapper {
public:
  [[nodiscard]] auto LoadMappings(const MappingConfig& config) -> bool;

  [[nodiscard]] auto GetMapping(const std::string& short_name) const
      -> MappingItem;

private:
  std::map<std::string, MappingItem> mappings_;
};

#endif // CONVERTER_PROJECT_NAME_MAPPER_HPP_
