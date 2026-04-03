// infrastructure/config/mapping_toml_reader.cpp

#include "infrastructure/config/mapping_toml_reader.hpp"

#include <iostream>
#include <string>
#include <toml++/toml.hpp>

namespace {

auto ParseConfig(const std::string& file_path, const toml::table& root)
    -> std::optional<MappingConfig> {
  MappingConfig config;

  for (const auto& [short_name, node] : root) {
    if (!node.is_table()) {
      std::cerr << "Error: [MappingTomlReader] Entry '" << short_name.str()
                << "' must be a TOML table in " << file_path << std::endl;
      return std::nullopt;
    }

    const toml::table* entry = node.as_table();
    const auto full_name = entry->get_as<std::string>("full_name");
    const auto type = entry->get_as<std::string>("type");
    if (full_name == nullptr || type == nullptr) {
      std::cerr << "Error: [MappingTomlReader] Entry '" << short_name.str()
                << "' must define string fields 'full_name' and 'type' in "
                << file_path << std::endl;
      return std::nullopt;
    }

    config.items.emplace(
        std::string(short_name.str()),
        MappingItem{.full_name = full_name->get(), .type = type->get()});
  }

  return config;
}

}  // namespace

auto MappingTomlReader::ReadFile(const std::string& file_path)
    -> std::optional<MappingConfig> {
  try {
    const toml::table root = toml::parse_file(file_path);
    return ParseConfig(file_path, root);
  } catch (const toml::parse_error& error) {
    std::cerr << "Error: [MappingTomlReader] Failed to parse TOML file "
              << file_path << ": " << error << std::endl;
    return std::nullopt;
  }
}
