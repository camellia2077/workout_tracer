// infrastructure/config/file_mapping_provider.cpp

#include "infrastructure/config/file_mapping_provider.hpp"

#include "infrastructure/config/mapping_toml_reader.hpp"

auto FileMappingProvider::GetMappingData(const std::string& source)
    -> std::optional<MappingConfig> {
  return MappingTomlReader::ReadFile(source);
}
