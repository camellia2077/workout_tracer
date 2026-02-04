// converter/project_name_mapper.cpp

#include "infrastructure/converter/project_name_mapper.hpp"

#include <cjson/cJSON.h>

#include <iostream>

auto ProjectNameMapper::LoadMappings(const cJSON* json_root) -> bool {
  if (json_root == nullptr || cJSON_IsObject(json_root) == 0) {
    std::cerr << "Error: [NameMapper] Invalid JSON object." << std::endl;
    return false;
  }

  cJSON* child = json_root->child;
  while (child != nullptr) {
    if (child->string != nullptr) {
      std::string key = child->string;

      cJSON* full_name_item =
          cJSON_GetObjectItemCaseSensitive(child, "fullName");
      cJSON* type_item = cJSON_GetObjectItemCaseSensitive(child, "type");

      if (cJSON_IsString(full_name_item) != 0 &&
          (full_name_item->valuestring != nullptr) &&
          cJSON_IsString(type_item) != 0 &&
          (type_item->valuestring != nullptr)) {
        mappings_[key] = {.full_name = full_name_item->valuestring,
                          .type = type_item->valuestring};
      }
    }
    child = child->next;
  }
  return true;
}

auto ProjectNameMapper::GetMapping(const std::string& short_name) const
    -> ProjectMapping {
  auto map_it = mappings_.find(short_name);
  if (map_it != mappings_.end()) {
    return map_it->second;
  }
  return {.full_name = short_name, .type = "unknown"};
}