// converter/log_parser.cpp

#include "infrastructure/converter/log_parser.hpp"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>
#include <utility>

LogParser::LogParser() = default;

auto LogParser::GetParsedData() const -> const std::vector<DailyData>& {
  return all_daily_data_;
}

auto LogParser::GetParsedYear() const -> std::optional<int> {
  return parsed_year_;
}

auto LogParser::Trim(std::string_view value) -> std::string {
  const std::string_view kWhitespace = " \t\n\r";
  const size_t kStart = value.find_first_not_of(kWhitespace);
  if (kStart == std::string_view::npos) {
    return "";
  }
  const size_t kEnd = value.find_last_not_of(kWhitespace);
  return std::string(value.substr(kStart, kEnd - kStart + 1));
}

auto LogParser::SplitComment(std::string_view value)
    -> std::pair<std::string, std::string> {
  const size_t kSlashPos = value.find("//");
  const size_t kHashPos = value.find('#');
  const size_t kSemPos = value.find(';');

  auto pick_min = [](size_t first, size_t second) -> size_t {
    if (first == std::string::npos) {
      return second;
    }
    if (second == std::string::npos) {
      return first;
    }
    return std::min(first, second);
  };

  const size_t kPos = pick_min(kSlashPos, pick_min(kHashPos, kSemPos));

  std::string main_part(value);
  std::string note_part;
  if (kPos != std::string::npos) {
    main_part = std::string(value.substr(0, kPos));
    size_t note_start = kPos;
    if (value.compare(kPos, 2, "//") == 0) {
      note_start += 2;
    } else {
      note_start += 1;
    }
    note_part = std::string(value.substr(note_start));
  }

  return {Trim(main_part), Trim(note_part)};
}

auto LogParser::IsNoteLine(std::string_view text) -> bool {
  if (text.size() < 2 || text[0] != 'r') {
    return false;
  }
  return std::isspace(static_cast<unsigned char>(text[1])) != 0;
}

auto LogParser::ParseFile(const std::string& file_path) -> bool {
  std::ifstream file(file_path);
  if (!file.is_open()) {
    std::cerr << "Error: [LogParser] Could not open file " << file_path
              << std::endl;
    return false;
  }

  all_daily_data_.clear();
  parsed_year_.reset();

  std::string line;
  ParserState state;

  while (std::getline(file, line)) {
    state.line_counter_++;
    line = Trim(line);
    if (line.empty()) {
      continue;
    }

    bool success = true;
    if (line[0] == 'y') {
      success = HandleYearLine(line);
    } else if (line.length() == 4 && std::ranges::all_of(line, ::isdigit)) {
      success = HandleDateLine(line, state);
    } else if (IsNoteLine(line)) {
      success = HandleNoteLine(line, state);
    } else if (line[0] == '+' || line[0] == '-') {
      success = HandleContentLine(line, state);
    } else {
      success = HandleProjectLine(line, state);
    }

    if (!success) {
      return false;
    }
  }

  if (!state.current_daily_data_.date_.empty()) {
    all_daily_data_.push_back(state.current_daily_data_);
  }

  file.close();
  return true;
}

auto LogParser::HandleYearLine(const std::string& line) -> bool {
  const std::regex kYearRegex(R"(^y(\d{4})$)");
  std::smatch match;
  if (std::regex_match(line, match, kYearRegex)) {
    if (!parsed_year_.has_value()) {
      parsed_year_ = std::stoi(match[1].str());
    }
  }
  return true;
}

auto LogParser::HandleDateLine(const std::string& line, ParserState& state)
    -> bool {
  if (!state.current_daily_data_.date_.empty()) {
    all_daily_data_.push_back(state.current_daily_data_);
  }
  state.current_daily_data_ = DailyData();
  state.current_daily_data_.date_ = line;
  state.current_project_ = nullptr;
  return true;
}

auto LogParser::HandleNoteLine(const std::string& line, ParserState& state)
    -> bool {
  if (state.current_daily_data_.date_.empty()) {
    std::cerr << "Error: [LogParser] Note found before a date line at line "
              << state.line_counter_ << "." << std::endl;
    return false;
  }
  if (!state.current_daily_data_.projects_.empty()) {
    std::cerr
        << "Error: [LogParser] Note must appear before any project at line "
        << state.line_counter_ << "." << std::endl;
    return false;
  }
  if (!state.current_daily_data_.note_.empty()) {
    std::cerr << "Error: [LogParser] Duplicate note at line "
              << state.line_counter_ << "." << std::endl;
    return false;
  }
  std::string note = Trim(line.substr(1));
  if (note.empty()) {
    std::cerr << "Error: [LogParser] Empty note at line " << state.line_counter_
              << "." << std::endl;
    return false;
  }
  state.current_daily_data_.note_ = note;
  return true;
}

auto LogParser::HandleContentLine(const std::string& line, ParserState& state)
    -> bool {
  if (state.current_project_ == nullptr) {
    std::cerr << "Error: [LogParser] Content line found without a "
                 "preceding project name at line "
              << state.line_counter_ << "." << std::endl;
    return false;
  }
  auto [main_part, note_part] = SplitComment(line);
  if (main_part.empty()) {
    std::cerr << "Error: [LogParser] Empty content line at line "
              << state.line_counter_ << "." << std::endl;
    return false;
  }
  double weight = 0.0;
  std::vector<SetData> parsed_sets = ParseContentLine(main_part, weight);
  for (auto& set_item : parsed_sets) {
    set_item.weight_ = weight;
    set_item.note_ = note_part;
    set_item.set_number_ =
        static_cast<int>(state.current_project_->sets_.size()) + 1;
    state.current_project_->sets_.push_back(set_item);
  }
  return true;
}

auto LogParser::HandleProjectLine(const std::string& line, ParserState& state)
    -> bool {
  if (state.current_daily_data_.date_.empty()) {
    std::cerr << "Error: [LogParser] Project name found before a year/date "
                 "line at line "
              << state.line_counter_ << "." << std::endl;
    return false;
  }
  state.current_daily_data_.projects_.emplace_back();
  state.current_project_ = &state.current_daily_data_.projects_.back();
  auto [proj_name, proj_note] = SplitComment(line);
  if (proj_name.empty()) {
    std::cerr << "Error: [LogParser] Empty project name at line "
              << state.line_counter_ << "." << std::endl;
    return false;
  }
  state.current_project_->project_name_ = proj_name;
  state.current_project_->note_ = proj_note;
  state.current_project_->line_number_ = state.line_counter_;
  return true;
}

auto LogParser::ParseContentLine(const std::string& line, double& out_weight)
    -> std::vector<SetData> {
  std::vector<SetData> sets;
  std::stringstream str_stream(line);

  char sign_char;
  str_stream >> sign_char;

  double val;
  str_stream >> val;

  out_weight = (sign_char == '-') ? -val : val;

  std::string reps_part;
  std::getline(str_stream, reps_part);

  std::string clean_reps;
  for (char char_item : reps_part) {
    if (std::isdigit(static_cast<unsigned char>(char_item)) != 0 ||
        char_item == '+') {
      clean_reps += char_item;
    }
  }

  std::stringstream reps_ss(clean_reps);
  std::string rep_token;
  while (std::getline(reps_ss, rep_token, '+')) {
    if (!rep_token.empty()) {
      SetData current_set;
      try {
        current_set.reps_ = std::stoi(rep_token);
        sets.push_back(current_set);
      } catch (const std::exception& e) {
        std::cerr << "Warning: Failed to parse reps from token '" << rep_token
                  << "': " << e.what() << std::endl;
      }
    }
  }
  return sets;
}
