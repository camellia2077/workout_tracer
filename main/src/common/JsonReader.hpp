// common/JsonReader.hpp
#ifndef JSON_READER_H
#define JSON_READER_H

#include <string>
#include <optional>
#include "nlohmann/json.hpp"

class JsonReader {
public:
    /**
     * @brief 读取并解析一个JSON文件。
     * @param filePath 文件的路径。
     * @return 如果成功，返回一个包含解析后数据的 std::optional<nlohmann::json>。
     * 如果文件打不开或解析失败，则返回 std::nullopt。
     */
    static std::optional<nlohmann::json> readFile(const std::string& filePath);
};

#endif // JSON_READER_H