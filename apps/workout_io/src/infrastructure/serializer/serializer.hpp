// serializer/serializer.hpp

#ifndef SERIALIZER_SERIALIZER_HPP_
#define SERIALIZER_SERIALIZER_HPP_

#include "domain/models/workout_item.hpp"
#include <cjson/cJSON.h>
#include <string>
#include <vector>

class Serializer {
public:
  [[nodiscard]] static auto Serialize(const std::vector<DailyData>& data) -> std::string;
  [[nodiscard]] static auto Deserialize(const cJSON* root) -> std::vector<DailyData>;

private:
  [[nodiscard]] static auto CreateSetJson(const SetData& set_data) -> cJSON*;
  [[nodiscard]] static auto ParseSetJson(const cJSON* json_set) -> SetData;
};

#endif // SERIALIZER_SERIALIZER_HPP_
