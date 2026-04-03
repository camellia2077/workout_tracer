// converter/log_parser.cpp

#include "infrastructure/converter/log_parser.hpp"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>
#include <utility>

#include "domain/services/weight_unit_service.hpp"

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

  return ParseInput(file);
}

auto LogParser::ParseText(std::string_view text) -> bool {
  std::istringstream input_stream{std::string(text)};
  return ParseInput(input_stream);
}

auto LogParser::ParseInput(std::istream& input) -> bool {
  all_daily_data_.clear();
  parsed_year_.reset();
  parsed_month_.reset();

  std::string line;
  ParserState state;

  while (std::getline(input, line)) {
    state.line_counter_++;
    line = Trim(line);
    if (line.empty()) {
      continue;
    }

    bool success = true;
    const char marker =
        static_cast<char>(std::tolower(static_cast<unsigned char>(line[0])));
    if (marker == 'y') {
      success = HandleYearLine(line);
    } else if (marker == 'm') {
      success = HandleMonthLine(line, state);
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

  return true;
}

auto LogParser::HandleYearLine(const std::string& line) -> bool {
  const std::regex kYearRegex(R"(^y(\d{4})$)");
  std::smatch match;
  if (!std::regex_match(line, match, kYearRegex)) {
    std::cerr << "Error: [LogParser] Invalid year line '" << line
              << "'. Expected format yYYYY." << std::endl;
    return false;
  }

  parsed_year_ = std::stoi(match[1].str());
  // Force each year section to declare a fresh month explicitly.
  parsed_month_.reset();
  return true;
}

auto LogParser::HandleMonthLine(const std::string& line, ParserState& state)
    -> bool {
  if (!parsed_year_.has_value()) {
    std::cerr << "Error: [LogParser] Month line found before year declaration "
                 "at line "
              << state.line_counter_ << "." << std::endl;
    return false;
  }

  const std::regex kMonthRegex(R"(^m(0[1-9]|1[0-2])$)", std::regex::icase);
  std::smatch match;
  if (!std::regex_match(line, match, kMonthRegex)) {
    std::cerr << "Error: [LogParser] Invalid month line '" << line
              << "'. Expected format mMM." << std::endl;
    return false;
  }
  parsed_month_ = std::stoi(match[1].str());
  return true;
}

auto LogParser::HandleDateLine(const std::string& line, ParserState& state)
    -> bool {
  if (!parsed_year_.has_value()) {
    std::cerr << "Error: [LogParser] Date found before year declaration at "
                 "line "
              << state.line_counter_ << "." << std::endl;
    return false;
  }
  if (!parsed_month_.has_value()) {
    std::cerr << "Error: [LogParser] Date found before month declaration at "
                 "line "
              << state.line_counter_ << "." << std::endl;
    return false;
  }

  if (!state.current_daily_data_.date_.empty()) {
    all_daily_data_.push_back(state.current_daily_data_);
  }

  const std::string expected_month =
      parsed_month_.value() < 10 ? "0" + std::to_string(parsed_month_.value())
                                 : std::to_string(parsed_month_.value());
  if (line.substr(0, 2) != expected_month) {
    std::cerr << "Error: [LogParser] Date month mismatch. Expected "
              << expected_month << ", got " << line.substr(0, 2) << " at line "
              << state.line_counter_ << "." << std::endl;
    return false;
  }

  state.current_daily_data_ = DailyData();
  // Keep MMDD internally so DateService can still compose final YYYY-MM-DD
  // from parsed year and this validated month/day payload.
  state.current_daily_data_.date_ = expected_month + line.substr(2, 2);
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
  double weight_kg = 0.0;
  std::string original_unit;
  double original_weight_value = 0.0;
  std::vector<SetData> parsed_sets = ParseContentLine(
      main_part, weight_kg, original_unit, original_weight_value);
  if (parsed_sets.empty()) {
    std::cerr << "Error: [LogParser] Failed to parse content line at line "
              << state.line_counter_ << "." << std::endl;
    return false;
  }
  for (auto& set_item : parsed_sets) {
    set_item.weight_kg_ = weight_kg;
    set_item.original_unit_ = original_unit;
    set_item.original_weight_value_ = original_weight_value;
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

auto LogParser::ParseContentLine(const std::string& line, double& out_weight_kg,
                                 std::string& out_original_unit,
                                 double& out_original_weight_value)
    -> std::vector<SetData> {
  std::vector<SetData> sets;
  static const std::regex kContentRegex(
      R"(^([+-])\s*(\d+(?:\.\d+)?)([A-Za-z]*)\s+(.+)$)");

  std::smatch match;
  if (!std::regex_match(line, match, kContentRegex)) {
    return sets;
  }

  const char sign_char = match[1].str()[0];
  const double absolute_value = std::stod(match[2].str());
  const double signed_value =
      (sign_char == '-') ? -absolute_value : absolute_value;
  const auto normalized_unit =
      WeightUnitService::NormalizeOriginalUnit(match[3].str());
  if (!normalized_unit.has_value()) {
    return sets;
  }

  out_original_unit = normalized_unit.value();
  out_original_weight_value = signed_value;
  out_weight_kg = WeightUnitService::ConvertToKg(out_original_weight_value,
                                                 out_original_unit);

  const std::string reps_part = match[4].str();

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
