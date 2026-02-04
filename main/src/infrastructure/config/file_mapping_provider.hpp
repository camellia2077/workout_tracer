// infrastructure/config/file_mapping_provider.hpp
#ifndef INFRASTRUCTURE_CONFIG_FILE_MAPPING_PROVIDER_HPP_
#define INFRASTRUCTURE_CONFIG_FILE_MAPPING_PROVIDER_HPP_

#include "application/interfaces/i_mapping_provider.hpp"
#include "common/json_reader.hpp"
#include <iostream>

class FileMappingProvider : public IMappingProvider {
public:
  [[nodiscard]] auto GetMappingData(const std::string& source) -> std::optional<CJsonPtr> override {
    return JsonReader::ReadFile(source);
  }
};

#endif // INFRASTRUCTURE_CONFIG_FILE_MAPPING_PROVIDER_HPP_
