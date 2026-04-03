// application/interfaces/mapping_config.hpp

#ifndef APPLICATION_INTERFACES_MAPPING_CONFIG_HPP_
#define APPLICATION_INTERFACES_MAPPING_CONFIG_HPP_

#include <map>
#include <string>

struct MappingItem {
  std::string full_name;
  std::string type;
};

struct MappingConfig {
  std::map<std::string, MappingItem> items;
};

#endif  // APPLICATION_INTERFACES_MAPPING_CONFIG_HPP_
