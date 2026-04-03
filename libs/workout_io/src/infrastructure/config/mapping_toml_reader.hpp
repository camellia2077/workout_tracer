// infrastructure/config/mapping_toml_reader.hpp

#ifndef INFRASTRUCTURE_CONFIG_MAPPING_TOML_READER_HPP_
#define INFRASTRUCTURE_CONFIG_MAPPING_TOML_READER_HPP_

#include <optional>
#include <string>

#include "application/interfaces/mapping_config.hpp"

class MappingTomlReader {
public:
  [[nodiscard]] static auto ReadFile(const std::string& file_path)
      -> std::optional<MappingConfig>;
};

#endif  // INFRASTRUCTURE_CONFIG_MAPPING_TOML_READER_HPP_
