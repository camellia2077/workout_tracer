// converter/converter.cpp
#include "infrastructure/converter/converter.hpp"

#include <iostream>

#include "domain/services/date_service.hpp"
#include "domain/services/volume_service.hpp"

Converter::Converter(ILogParser& parser, IMappingProvider& mapping_provider)
    : parser_(parser), mapping_provider_(mapping_provider) {}

auto Converter::Configure(const std::string& mapping_file_path) -> bool {
  auto json_data_opt = mapping_provider_.GetMappingData(mapping_file_path);
  if (!json_data_opt.has_value()) {
    std::cerr << "Error: [Converter] Failed to read or parse mapping file: "
              << mapping_file_path << std::endl;
    return false;
  }

  if (!mapper_.LoadMappings(json_data_opt.value().get())) {
    std::cerr << "Error: [Converter] Failed to load mappings from JSON data."
              << std::endl;
    return false;
  }

  std::cout << "[Converter] Configuration successful. Mappings loaded from "
            << mapping_file_path << std::endl;
  return true;
}

auto Converter::MapProjectNames(std::vector<DailyData>& data) -> void {
  for (auto& daily_data : data) {
    for (auto& project : daily_data.projects_) {
      ProjectMapping mapping = mapper_.GetMapping(project.project_name_);
      project.project_name_ = mapping.full_name;
      project.type_ = mapping.type;
    }
  }
}

auto Converter::Convert(const std::string& log_file_path)
    -> std::optional<std::vector<DailyData>> {
  if (!parser_.ParseFile(log_file_path)) {
    std::cerr << "Error: [Converter] Parsing log file failed." << std::endl;
    return std::nullopt;
  }

  auto processed_data = parser_.GetParsedData();
  auto year_to_use_opt = parser_.GetParsedYear();

  if (!year_to_use_opt.has_value()) {
    std::cerr
        << "Error: [Converter] Year could not be determined from the log file."
        << std::endl;
    return std::nullopt;
  }

  if (processed_data.empty()) {
    return processed_data;
  }

  DateService::CompleteDates(processed_data, year_to_use_opt.value());
  VolumeService::CalculateVolume(processed_data);
  MapProjectNames(processed_data);

  return processed_data;
}