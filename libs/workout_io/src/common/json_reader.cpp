// common/json_reader.cpp

#include "common/json_reader.hpp"

#include <fstream>
#include <iostream>
#include <sstream>

auto JsonReader::ReadFile(const std::string& file_path)
    -> std::optional<CJsonPtr> {
  std::ifstream json_file(file_path);
  if (!json_file.is_open()) {
    std::cerr << "Error: [JsonReader] Could not open file " << file_path
              << std::endl;
    return std::nullopt;
  }

  std::stringstream buffer;
  buffer << json_file.rdbuf();
  std::string content = buffer.str();

  cJSON* raw_json = cJSON_Parse(content.c_str());
  if (raw_json == nullptr) {
    const char* error_ptr = cJSON_GetErrorPtr();
    std::cerr << "Error: [JsonReader] Parse failed: "
              << (error_ptr != nullptr ? error_ptr : "Unknown") << std::endl;
    return std::nullopt;
  }

  return MakeCJson(raw_json);
}