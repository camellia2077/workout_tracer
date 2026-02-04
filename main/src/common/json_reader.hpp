// common/json_reader.hpp

#ifndef COMMON_JSON_READER_HPP_
#define COMMON_JSON_READER_HPP_

#include "common/c_json_helper.hpp"
#include <optional>
#include <string>

class JsonReader {
public:
  // 返回类型改为 CJsonPtr
  static auto ReadFile(const std::string& file_path) -> std::optional<CJsonPtr>;
};

#endif // COMMON_JSON_READER_HPP_