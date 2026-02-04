// application/interfaces/i_log_parser.hpp
#ifndef APPLICATION_INTERFACES_I_LOG_PARSER_HPP_
#define APPLICATION_INTERFACES_I_LOG_PARSER_HPP_

#include "domain/models/workout_item.hpp"
#include <optional>
#include <string>
#include <vector>

class ILogParser {
public:
  virtual ~ILogParser() = default;

  // Parse the source (e.g., file path) and return the parsed data if successful
  virtual auto ParseFile(const std::string& source) -> bool = 0;

  // Get the parsed data
  virtual auto GetParsedData() const -> const std::vector<DailyData>& = 0;

  // Get the parsed year if available
  virtual auto GetParsedYear() const -> std::optional<int> = 0;
};

#endif // APPLICATION_INTERFACES_I_LOG_PARSER_HPP_
