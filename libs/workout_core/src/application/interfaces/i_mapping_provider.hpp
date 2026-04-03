// application/interfaces/i_mapping_provider.hpp

#ifndef APPLICATION_INTERFACES_I_MAPPING_PROVIDER_HPP_
#define APPLICATION_INTERFACES_I_MAPPING_PROVIDER_HPP_

#include <optional>
#include <string>

#include "application/interfaces/mapping_config.hpp"

class IMappingProvider {
public:
  virtual ~IMappingProvider() = default;
  [[nodiscard]] virtual auto GetMappingData(const std::string& source)
      -> std::optional<MappingConfig> = 0;
};

#endif // APPLICATION_INTERFACES_I_MAPPING_PROVIDER_HPP_
