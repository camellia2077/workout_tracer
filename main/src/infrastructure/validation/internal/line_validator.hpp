// validator/internal/line_validator.hpp

#ifndef VALIDATOR_INTERNAL_LINE_VALIDATOR_HPP_
#define VALIDATOR_INTERNAL_LINE_VALIDATOR_HPP_

#include <optional>
#include <regex>
#include <string>

enum class StateType {
  EXPECTING_YEAR,
  EXPECTING_DATE,
  EXPECTING_TITLE,
  EXPECTING_CONTENT,
  EXPECTING_TITLE_OR_CONTENT
};

struct ValidationRules;

class LineValidator {
public:
  LineValidator();

  auto ValidateLine(const std::string& line, const ValidationRules& rules,
                    int& error_count) -> void;

  auto FinalizeValidation(int& error_count) const -> void;

private:
  struct ValidationState {
    StateType current_state = StateType::EXPECTING_YEAR;
    int line_counter = 0;
    bool content_seen_for_date = false;
    bool note_seen_for_date = false;
    int last_date_line = 0;
  };

  auto HandleYearState(const std::string& line, const ValidationRules& rules,
                      int& error_count) -> bool;
  auto HandleDateMatch(const std::string& line, const ValidationRules& rules,
                      int& error_count) -> bool;
  auto HandleNoteMatch(const std::string& line, const ValidationRules& rules,
                      int& error_count) -> bool;
  auto HandleContentMatch(const std::string& line, const ValidationRules& rules,
                         int& error_count) -> bool;
  auto HandleTitleMatch(const std::string& line, const ValidationRules& rules,
                       int& error_count) -> bool;

  ValidationState state_;
};

#endif // VALIDATOR_INTERNAL_LINE_VALIDATOR_HPP_
