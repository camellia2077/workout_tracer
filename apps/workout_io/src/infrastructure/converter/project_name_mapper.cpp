// converter/project_name_mapper.cpp

#include "infrastructure/converter/project_name_mapper.hpp"

#include <iostream>

auto ProjectNameMapper::LoadMappings(const MappingConfig& config) -> bool {
  mappings_.clear();
  for (const auto& [short_name, item] : config.items) {
    mappings_.emplace(short_name, item);
  }
  return true;
}

auto ProjectNameMapper::GetMapping(const std::string& short_name) const
    -> MappingItem {
  auto map_it = mappings_.find(short_name);
  if (map_it != mappings_.end()) {
    return map_it->second;
  }
  return {.full_name = short_name, .type = "unknown"};
}
