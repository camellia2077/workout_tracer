// application/action_handler.hpp

#ifndef APPLICATION_ACTION_HANDLER_HPP_
#define APPLICATION_ACTION_HANDLER_HPP_

#include <string>

enum class ActionType { Validate, Convert, Insert, Export, Ingest, QueryPR, ListExercises, QueryCycles, QueryVolume };

struct AppConfig {
  ActionType action_;
  std::string log_filepath_;
  std::string mapping_path_;
  std::string base_path_;
  std::string type_filter_;
  std::string cycle_id_filter_;
};

class ActionHandler {
public:
  static auto Run(const AppConfig& config) -> bool;
};

#endif // APPLICATION_ACTION_HANDLER_HPP_