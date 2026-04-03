// converter/log_parser.hpp

#ifndef CONVERTER_LOG_PARSER_HPP_
#define CONVERTER_LOG_PARSER_HPP_

#include "application/interfaces/i_log_parser.hpp"
#include "domain/models/workout_item.hpp"
#include <istream>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

class LogParser : public ILogParser {
public:
  LogParser();
  
  auto ParseFile(const std::string& file_path) -> bool override;

  auto ParseText(std::string_view text) -> bool override;
  
  auto GetParsedData() const -> const std::vector<DailyData>& override;

  auto GetParsedYear() const -> std::optional<int> override;

private:
  struct ParserState {
    DailyData current_daily_data_;
    ProjectData* current_project_ = nullptr;
    int line_counter_ = 0;
  };

  std::vector<DailyData> all_daily_data_;
  std::optional<int> parsed_year_;
  std::optional<int> parsed_month_;

  [[nodiscard]] static auto Trim(std::string_view value) -> std::string;
  [[nodiscard]] static auto SplitComment(std::string_view value) -> std::pair<std::string, std::string>;
  [[nodiscard]] static auto IsNoteLine(std::string_view text) -> bool;
  [[nodiscard]] static auto ParseContentLine(const std::string& line,
                                             double& out_weight_kg,
                                             std::string& out_original_unit,
                                             double& out_original_weight_value)
      -> std::vector<SetData>;

  [[nodiscard]] auto HandleYearLine(const std::string& line) -> bool;
  [[nodiscard]] auto HandleMonthLine(const std::string& line, ParserState& state)
      -> bool;
  [[nodiscard]] auto HandleDateLine(const std::string& line, ParserState& state) -> bool;
  [[nodiscard]] static auto HandleNoteLine(const std::string& line, ParserState& state) -> bool;
  [[nodiscard]] static auto HandleContentLine(const std::string& line, ParserState& state) -> bool;
  [[nodiscard]] static auto HandleProjectLine(const std::string& line, ParserState& state) -> bool;
  auto ParseInput(std::istream& input) -> bool;
};

#endif // CONVERTER_LOG_PARSER_HPP_
