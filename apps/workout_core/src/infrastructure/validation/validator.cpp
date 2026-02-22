// validator/validator.cpp
#include "infrastructure/validation/validator.hpp"

#include <iostream>
#include <sstream>

#include "internal/line_validator.hpp"

Validator::Validator(IMappingProvider& mapping_provider)
    : mapping_provider_(mapping_provider) {}

auto Validator::Validate(std::istream& input,
                         const std::string& mapping_file_path) -> bool {
  auto valid_titles_opt = LoadValidTitles(mapping_file_path);
  if (!valid_titles_opt.has_value()) {
    return false;
  }

  auto rules_opt = CreateRules(valid_titles_opt.value());
  if (!rules_opt.has_value()) {
    return false;
  }
  const auto& rules = rules_opt.value();

  LineValidator line_validator;
  int error_count = 0;
  std::string line;

  while (std::getline(input, line)) {
    line.erase(0, line.find_first_not_of(" \t\n\r"));
    line.erase(line.find_last_not_of(" \t\n\r") + 1);

    if (line.empty()) {
      continue;
    }

    line_validator.ValidateLine(line, rules, error_count);
  }

  line_validator.FinalizeValidation(error_count);

  return error_count == 0;
}

auto Validator::CreateRules(const std::vector<std::string>& valid_titles)
    -> std::optional<ValidationRules> {
  if (valid_titles.empty()) {
    std::cerr << "Warning: [Validator] No valid titles found in mapping file."
              << std::endl;
  }
  std::stringstream title_regex_pattern;
  title_regex_pattern << "^(";
  for (size_t i = 0; i < valid_titles.size(); ++i) {
    title_regex_pattern << valid_titles[i]
                        << (i < valid_titles.size() - 1 ? "|" : "");
  }
  title_regex_pattern << ")$";
  std::string title_with_comment = title_regex_pattern.str();
  if (!title_with_comment.empty() && title_with_comment.back() == '$') {
    title_with_comment.pop_back();
    title_with_comment += R"((\s*(?://|#|;).*)?$)";
  }
  try {
    return ValidationRules{
        .year_regex = std::regex(R"(^y\d{4}$)"),
        .date_regex = std::regex(R"(^\d{4}$)"),
        .note_regex = std::regex(R"(^r\s+.+$)"),
        .title_regex = std::regex(title_with_comment),
        .content_regex = std::regex(
            R"(^[+-]\s*\d+(\.\d+)?(lbs|kg|LBS|KG)?\s+\d+(\s*\+\s*\d+)*(\s*(?://|#|;).*)?$)")};
  } catch (const std::regex_error& e) {
    std::cerr << "Error: [Validator] Failed to create regex rules: " << e.what()
              << std::endl;
    return std::nullopt;
  }
}

auto Validator::LoadValidTitles(const std::string& mapping_file_path)
    -> std::optional<std::vector<std::string>> {
  auto json_data_opt = mapping_provider_.GetMappingData(mapping_file_path);
  if (!json_data_opt.has_value()) {
    std::cerr << "Error: [Validator] Could not read or parse mapping file at: "
              << mapping_file_path << std::endl;
    return std::nullopt;
  }

  cJSON* root = json_data_opt.value().get();

  if (cJSON_IsObject(root) == 0) {
    std::cerr << "Error: [Validator] Mapping file content is not a JSON object."
              << std::endl;
    return std::nullopt;
  }

  std::vector<std::string> titles;
  cJSON* child = root->child;
  while (child != nullptr) {
    if (child->string != nullptr) {
      titles.emplace_back(child->string);
    }
    child = child->next;
  }
  return titles;
}
