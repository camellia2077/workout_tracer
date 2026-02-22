// cli/framework/command.hpp
#ifndef CLI_FRAMEWORK_COMMAND_HPP_
#define CLI_FRAMEWORK_COMMAND_HPP_

#include "application/action_handler.hpp" // For AppConfig
#include <string>
#include <vector>

namespace cli {
namespace framework {

class Command {
public:
  virtual ~Command() = default;
  
  [[nodiscard]] virtual auto GetName() const -> std::string = 0;
  
  [[nodiscard]] virtual auto GetCategory() const -> std::string = 0;
  
  [[nodiscard]] virtual auto GetDescription() const -> std::string = 0;
  
  virtual auto Parse(const std::vector<std::string>& args,
                     AppConfig& config) -> bool = 0;
};

} // namespace framework
} // namespace cli

#endif // CLI_FRAMEWORK_COMMAND_HPP_
