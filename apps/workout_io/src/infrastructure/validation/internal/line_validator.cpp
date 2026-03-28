// validator/internal/line_validator.cpp

#include "infrastructure/validation/internal/line_validator.hpp"

#include <iostream>

#include "infrastructure/validation/validator.hpp"

LineValidator::LineValidator() = default;

auto LineValidator::ValidateLine(const std::string& line,
                                 const ValidationRules& rules, int& error_count)
    -> void {
  state_.line_counter++;

  if (HandleYearState(line, rules, error_count)) {
    return;
  }
  if (HandleDateMatch(line, rules, error_count)) {
    return;
  }
  if (HandleNoteMatch(line, rules, error_count)) {
    return;
  }
  if (HandleContentMatch(line, rules, error_count)) {
    return;
  }
  if (HandleTitleMatch(line, rules, error_count)) {
    return;
  }

  std::cerr << "Error: [Validator] Unrecognized format at line "
            << state_.line_counter << ": \"" << line << "\"" << std::endl;
  error_count++;
}

auto LineValidator::HandleYearState(const std::string& line,
                                    const ValidationRules& rules,
                                    int& error_count) -> bool {
  if (state_.current_state != StateType::EXPECTING_YEAR) {
    return false;
  }

  if (std::regex_match(line, rules.year_regex)) {
    state_.current_state = StateType::EXPECTING_DATE;
    return true;
  }

  std::cerr << "Error: [Validator] Invalid format at line "
            << state_.line_counter
            << ". Expected a year declaration (e.g., y2025) at the "
               "beginning of the file."
            << std::endl;
  error_count++;
  state_.current_state = StateType::EXPECTING_DATE;
  return true;
}

auto LineValidator::HandleDateMatch(const std::string& line,
                                    const ValidationRules& rules,
                                    int& error_count) -> bool {
  if (!std::regex_match(line, rules.date_regex)) {
    return false;
  }

  if (state_.last_date_line > 0 && !state_.content_seen_for_date) {
    std::cerr << "Error: [Validator] The date entry at line "
              << state_.last_date_line << " is empty." << std::endl;
    error_count++;
  }
  if (state_.current_state == StateType::EXPECTING_CONTENT) {
    std::cerr << "Error: [Validator] Unexpected date at line "
              << state_.line_counter << ". A content line was expected."
              << std::endl;
    error_count++;
  }
  state_.last_date_line = state_.line_counter;
  state_.content_seen_for_date = false;
  state_.note_seen_for_date = false;
  state_.current_state = StateType::EXPECTING_TITLE;
  return true;
}

auto LineValidator::HandleNoteMatch(const std::string& line,
                                    const ValidationRules& rules,
                                    int& error_count) -> bool {
  if (!std::regex_match(line, rules.note_regex)) {
    return false;
  }

  if (state_.current_state != StateType::EXPECTING_TITLE) {
    std::cerr << "Error: [Validator] Unexpected note at line "
              << state_.line_counter
              << ". Notes must appear immediately after a date line."
              << std::endl;
    error_count++;
    return true;
  }
  if (state_.note_seen_for_date) {
    std::cerr << "Error: [Validator] Duplicate note at line "
              << state_.line_counter << "." << std::endl;
    error_count++;
    return true;
  }
  state_.note_seen_for_date = true;
  return true;
}

auto LineValidator::HandleContentMatch(const std::string& line,
                                       const ValidationRules& rules,
                                       int& error_count) -> bool {
  if (line[0] != '+' && line[0] != '-') {
    return false;
  }

  if (state_.current_state == StateType::EXPECTING_YEAR ||
      state_.current_state == StateType::EXPECTING_DATE ||
      state_.current_state == StateType::EXPECTING_TITLE) {
    std::cerr << "Error: [Validator] Invalid format at line "
              << state_.line_counter << ". Unexpected content line."
              << std::endl;
    error_count++;
    return true;
  }
  if (!std::regex_match(line, rules.content_regex)) {
    std::cerr << "Error: [Validator] Malformed content line at "
              << state_.line_counter << ": \"" << line << "\"" << std::endl;
    error_count++;
  }
  state_.content_seen_for_date = true;
  state_.current_state = StateType::EXPECTING_TITLE_OR_CONTENT;
  return true;
}

auto LineValidator::HandleTitleMatch(const std::string& line,
                                     const ValidationRules& rules,
                                     int& error_count) -> bool {
  if (!std::regex_match(line, rules.title_regex)) {
    return false;
  }

  if (state_.current_state == StateType::EXPECTING_YEAR ||
      state_.current_state == StateType::EXPECTING_DATE) {
    std::cerr << "Error: [Validator] Invalid format at line "
              << state_.line_counter << ". Expected a date but found a title."
              << std::endl;
    error_count++;
  } else if (state_.current_state == StateType::EXPECTING_CONTENT) {
    std::cerr << "Error: [Validator] Invalid format at line "
              << state_.line_counter
              << ". Expected a content line but found another title."
              << std::endl;
    error_count++;
  }
  state_.current_state = StateType::EXPECTING_CONTENT;
  return true;
}

auto LineValidator::FinalizeValidation(int& error_count) const -> void {
  if (state_.current_state == StateType::EXPECTING_YEAR) {
    std::cerr << "Error: [Validator] File is empty or does not start with a "
                 "year declaration (e.g., y2025)."
              << std::endl;
    error_count++;
    return;
  }

  if (state_.last_date_line > 0 && !state_.content_seen_for_date) {
    std::cerr << "Error: [Validator] The last date entry at line "
              << state_.last_date_line
              << " is empty and must contain at least one record." << std::endl;
    error_count++;
    return;
  }

  if (state_.current_state == StateType::EXPECTING_CONTENT) {
    std::cerr
        << "Error: [Validator] File ends unexpectedly after a title on line "
        << state_.line_counter << ". Missing content line." << std::endl;
    error_count++;
  }
}
