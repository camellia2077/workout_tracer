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

  // Keep titles data-driven from mapping.toml. New exercise keys should only
  // require TOML updates, not regex template edits in C++.
  std::unordered_set<std::string> title_set;
  title_set.reserve(valid_titles.size());
  for (const auto& title : valid_titles) {
    title_set.insert(title);
  }
  try {
    return ValidationRules{
        .year_regex = std::regex(R"(^y\d{4}$)"),
        .month_regex = std::regex(R"(^m(0[1-9]|1[0-2])$)", std::regex::icase),
        .date_regex = std::regex(R"(^\d{4}$)"),
        .note_regex = std::regex(R"(^r\s+.+$)"),
        .content_regex = std::regex(
            R"(^[+-]\s*\d+(\.\d+)?(kg|l|lb|lbs)?\s+\d+(\s*\+\s*\d+)*(\s*(?://|#|;).*)?$)",
            std::regex::icase),
        .valid_titles = std::move(title_set)};
  } catch (const std::regex_error& e) {
    std::cerr << "Error: [Validator] Failed to create regex rules: " << e.what()
              << std::endl;
    return std::nullopt;
  }
}

auto Validator::LoadValidTitles(const std::string& mapping_file_path)
    -> std::optional<std::vector<std::string>> {
  auto mapping_data_opt = mapping_provider_.GetMappingData(mapping_file_path);
  if (!mapping_data_opt.has_value()) {
    std::cerr << "Error: [Validator] Could not read or parse mapping file at: "
              << mapping_file_path << std::endl;
    return std::nullopt;
  }

  std::vector<std::string> titles;
  titles.reserve(mapping_data_opt->items.size());
  // Only section keys are accepted as title tokens.
  for (const auto& item : mapping_data_opt->items) {
    titles.push_back(item.first);
  }
  return titles;
}
