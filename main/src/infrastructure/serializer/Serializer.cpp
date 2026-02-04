// serializer/serializer.cpp

#include "infrastructure/serializer/serializer.hpp"

#include <cmath>
#include <iostream>

#include "common/c_json_helper.hpp"

auto Serializer::CreateSetJson(const SetData& set_data) -> cJSON* {
  cJSON* j_set = cJSON_CreateObject();
  cJSON_AddNumberToObject(j_set, "set", set_data.set_number_);
  if (!set_data.note_.empty()) {
    cJSON_AddStringToObject(j_set, "note", set_data.note_.c_str());
  }

  if (set_data.weight_ < 0) {
    cJSON_AddNumberToObject(j_set, "elastic_band", std::abs(set_data.weight_));
    cJSON_AddStringToObject(j_set, "unit", "lbs");
    cJSON_AddNumberToObject(j_set, "reps", set_data.reps_);
    cJSON_AddNumberToObject(j_set, "volume", 0.0);
  } else {
    cJSON_AddNumberToObject(j_set, "weight", set_data.weight_);
    cJSON_AddStringToObject(j_set, "unit", "kg");
    cJSON_AddNumberToObject(j_set, "reps", set_data.reps_);
    cJSON_AddNumberToObject(j_set, "volume", set_data.volume_);
  }
  return j_set;
}

auto Serializer::Serialize(const std::vector<DailyData>& processed_data)
    -> std::string {
  if (processed_data.empty()) {
    return "{}";
  }

  CJsonPtr root = MakeCJson(cJSON_CreateObject());

  const std::string& start_date = processed_data[0].date_;
  cJSON_AddStringToObject(root.get(), "cycle_id", start_date.c_str());
  cJSON_AddStringToObject(root.get(), "type", "mixed");
  cJSON_AddNumberToObject(root.get(), "total_days",
                           static_cast<double>(processed_data.size()));

  cJSON* j_sessions = cJSON_AddArrayToObject(root.get(), "sessions");

  for (const auto& daily : processed_data) {
    cJSON* j_daily = cJSON_CreateObject();
    cJSON_AddStringToObject(j_daily, "date", daily.date_.c_str());
    if (!daily.note_.empty()) {
      cJSON_AddStringToObject(j_daily, "note", daily.note_.c_str());
    }

    cJSON* j_exercises = cJSON_AddArrayToObject(j_daily, "exercises");

    for (const auto& proj : daily.projects_) {
      cJSON* j_proj = cJSON_CreateObject();
      cJSON_AddStringToObject(j_proj, "name", proj.project_name_.c_str());
      cJSON_AddStringToObject(j_proj, "type", proj.type_.c_str());
      if (!proj.note_.empty()) {
        cJSON_AddStringToObject(j_proj, "note", proj.note_.c_str());
      }
      cJSON_AddNumberToObject(j_proj, "totalVolume", proj.total_volume_);

      cJSON* j_sets = cJSON_AddArrayToObject(j_proj, "sets");
      for (const auto& set_item : proj.sets_) {
        cJSON_AddItemToArray(j_sets, CreateSetJson(set_item));
      }

      cJSON_AddItemToArray(j_exercises, j_proj);
    }
    cJSON_AddItemToArray(j_sessions, j_daily);
  }

  char* json_string = cJSON_Print(root.get());
  std::string result(json_string);
  cJSON_free(json_string);

  return result;
}

static auto GetString(const cJSON* item, const char* key,
                      const std::string& default_val = "") -> std::string {
  cJSON* obj = cJSON_GetObjectItemCaseSensitive(item, key);
  if (cJSON_IsString(obj) != 0 && (obj->valuestring != nullptr)) {
    return {obj->valuestring};
  }
  return default_val;
}

static auto GetDouble(const cJSON* item, const char* key,
                      double default_val = 0.0) -> double {
  cJSON* obj = cJSON_GetObjectItemCaseSensitive(item, key);
  if (cJSON_IsNumber(obj) != 0) {
    return obj->valuedouble;
  }
  return default_val;
}

static auto GetInt(const cJSON* item, const char* key, int default_val = 0)
    -> int {
  cJSON* obj = cJSON_GetObjectItemCaseSensitive(item, key);
  if (cJSON_IsNumber(obj) != 0) {
    return obj->valueint;
  }
  return default_val;
}

auto Serializer::ParseSetJson(const cJSON* json_set) -> SetData {
  SetData set_data;
  set_data.set_number_ = GetInt(json_set, "set");
  set_data.reps_ = GetInt(json_set, "reps");
  set_data.volume_ = GetDouble(json_set, "volume");
  set_data.note_ = GetString(json_set, "note");

  double weight = GetDouble(json_set, "weight", 0.0);
  double elastic = GetDouble(json_set, "elastic_band", 0.0);

  if (elastic > 0) {
    set_data.weight_ = -elastic;
  } else {
    set_data.weight_ = weight;
  }

  return set_data;
}

auto Serializer::Deserialize(const cJSON* root) -> std::vector<DailyData> {
  std::vector<DailyData> all_data;
  if (root == nullptr || cJSON_IsObject(root) == 0) {
    return all_data;
  }

  cJSON* sessions = cJSON_GetObjectItemCaseSensitive(root, "sessions");
  cJSON* session = nullptr;

  cJSON_ArrayForEach(session, sessions) {
    DailyData daily;
    daily.date_ = GetString(session, "date");
    daily.note_ = GetString(session, "note");

    cJSON* exercises = cJSON_GetObjectItemCaseSensitive(session, "exercises");
    cJSON* exercise = nullptr;

    cJSON_ArrayForEach(exercise, exercises) {
      ProjectData proj;
      proj.project_name_ = GetString(exercise, "name");
      proj.type_ = GetString(exercise, "type");
      proj.note_ = GetString(exercise, "note");
      proj.total_volume_ = GetDouble(exercise, "totalVolume");

      cJSON* sets = cJSON_GetObjectItemCaseSensitive(exercise, "sets");
      cJSON* set_item = nullptr;

      cJSON_ArrayForEach(set_item, sets) {
        proj.sets_.push_back(ParseSetJson(set_item));
      }
      daily.projects_.push_back(proj);
    }
    all_data.push_back(daily);
  }

  return all_data;
}
