// converter/converter.hpp

#ifndef CONVERTER_CONVERTER_HPP_
#define CONVERTER_CONVERTER_HPP_

#include "application/interfaces/i_log_parser.hpp"
#include "application/interfaces/i_mapping_provider.hpp"
#include "domain/models/workout_item.hpp"
#include "infrastructure/converter/log_parser.hpp"
#include "infrastructure/converter/project_name_mapper.hpp"
#include <optional>
#include <string>
#include <vector>

class Converter {
public:
  Converter(ILogParser& parser, IMappingProvider& mapping_provider);
  
  auto Configure(const std::string& mapping_file_path) -> bool;
  
  auto Convert(const std::string& log_file_path) -> std::optional<std::vector<DailyData>>;

private:
  ILogParser& parser_;
  IMappingProvider& mapping_provider_;
  ProjectNameMapper mapper_;

  auto MapProjectNames(std::vector<DailyData>& data) -> void;
};

#endif // CONVERTER_CONVERTER_HPP_