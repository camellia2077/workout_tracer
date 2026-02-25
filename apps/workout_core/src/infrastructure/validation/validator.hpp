// validator/validator.hpp
#ifndef VALIDATOR_VALIDATOR_HPP_
#define VALIDATOR_VALIDATOR_HPP_

#include "application/interfaces/i_mapping_provider.hpp"
#include <iostream>
#include <optional>
#include <regex>
#include <string>
#include <vector>

struct ValidationRules {
  std::regex year_regex;
  std::regex date_regex;
  std::regex note_regex;
  std::regex title_regex;
  std::regex content_regex;
};

class Validator {
public:
  explicit Validator(IMappingProvider& mapping_provider);

  [[nodiscard]] auto Validate(std::istream& input, const std::string& mapping_file_path) -> bool;

private:
  IMappingProvider& mapping_provider_;

  [[nodiscard]] auto LoadValidTitles(const std::string& mapping_file_path)
      -> std::optional<std::vector<std::string>>;

  [[nodiscard]] static auto CreateRules(const std::vector<std::string>& valid_titles)
      -> std::optional<ValidationRules>;
};

#endif // VALIDATOR_VALIDATOR_HPP_
