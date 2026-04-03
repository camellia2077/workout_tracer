// validator/internal/line_validator.cpp

#include "infrastructure/validation/internal/line_validator.hpp"

#include <algorithm>
#include <cctype>
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
  if (HandleMonthMatch(line, rules, error_count)) {
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
    // Enforce explicit month declaration after year (yYYYY -> mMM -> MMDD).
    state_.current_state = StateType::EXPECTING_MONTH;
    state_.current_month.reset();
    return true;
  }

  std::cerr << "Error: [Validator] Invalid format at line "
            << state_.line_counter
            << ". Expected a year declaration (e.g., y2025) at the "
               "beginning of the file."
            << std::endl;
  error_count++;
  state_.current_state = StateType::EXPECTING_MONTH;
  return true;
}

auto LineValidator::HandleMonthMatch(const std::string& line,
                                     const ValidationRules& rules,
                                     int& error_count) -> bool {
  if (state_.current_state != StateType::EXPECTING_MONTH) {
    return false;
  }

  if (std::regex_match(line, rules.month_regex)) {
    state_.current_month = std::stoi(line.substr(1, 2));
    state_.current_state = StateType::EXPECTING_DATE;
    return true;
  }

  if (std::regex_match(line, rules.date_regex)) {
    std::cerr
        << "Error: [Validator] Missing month declaration before date at line "
        << state_.line_counter << "." << std::endl;
    error_count++;
    state_.current_state = StateType::EXPECTING_DATE;
    return false;
  }

  std::cerr << "Error: [Validator] Invalid format at line "
            << state_.line_counter
            << ". Expected a month declaration (e.g., m03) after year line."
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

  if (state_.current_state == StateType::EXPECTING_MONTH) {
    std::cerr
        << "Error: [Validator] Missing month declaration before date at line "
        << state_.line_counter << "." << std::endl;
    error_count++;
    state_.current_state = StateType::EXPECTING_DATE;
  }

  const int month_from_date = std::stoi(line.substr(0, 2));
  // mMM is the canonical month source; MMDD must match it to prevent
  // cross-month drift inside one monthly file.
  if (state_.current_month.has_value()) {
    if (state_.current_month.value() != month_from_date) {
      std::cerr << "Error: [Validator] Date month mismatch at line "
                << state_.line_counter << ". Expected month "
                << (state_.current_month.value() < 10 ? "0" : "")
                << state_.current_month.value() << ", got " << line.substr(0, 2)
                << "." << std::endl;
      error_count++;
    }
  } else {
    state_.current_month = month_from_date;
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
      state_.current_state == StateType::EXPECTING_MONTH ||
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

auto LineValidator::ExtractMainToken(const std::string& line) -> std::string {
  // Titles may carry inline comments, so compare only the pre-comment token.
  auto trim = [](std::string value) -> std::string {
    auto not_space = [](unsigned char ch) { return std::isspace(ch) == 0; };
    const auto first = std::find_if(value.begin(), value.end(), not_space);
    if (first == value.end()) {
      return "";
    }
    const auto last =
        std::find_if(value.rbegin(), value.rend(), not_space).base();
    return std::string(first, last);
  };

  const size_t slash_pos = line.find("//");
  const size_t hash_pos = line.find('#');
  const size_t sem_pos = line.find(';');
  size_t cut_pos = std::string::npos;
  for (const size_t pos : {slash_pos, hash_pos, sem_pos}) {
    if (pos == std::string::npos) {
      continue;
    }
    if (cut_pos == std::string::npos || pos < cut_pos) {
      cut_pos = pos;
    }
  }

  if (cut_pos == std::string::npos) {
    return trim(line);
  }
  return trim(line.substr(0, cut_pos));
}

auto LineValidator::HandleTitleMatch(const std::string& line,
                                     const ValidationRules& rules,
                                     int& error_count) -> bool {
  const std::string token = ExtractMainToken(line);
  if (token.empty()) {
    return false;
  }
  if (rules.valid_titles.find(token) == rules.valid_titles.end()) {
    return false;
  }

  if (state_.current_state == StateType::EXPECTING_MONTH) {
    std::cerr << "Error: [Validator] Invalid format at line "
              << state_.line_counter
              << ". Expected a month declaration but found a title."
              << std::endl;
    error_count++;
  } else if (state_.current_state == StateType::EXPECTING_YEAR ||
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
  if (state_.current_state == StateType::EXPECTING_MONTH) {
    std::cerr << "Error: [Validator] File ends unexpectedly after year line. "
                 "Missing month declaration (e.g., m03)."
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
