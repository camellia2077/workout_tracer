// common/JsonReader.hpp
#ifndef JSON_READER_H
#define JSON_READER_H

#include "CJsonHelper.hpp"
#include <string>
#include <optional>

class JsonReader {
public:
    // 返回类型改为 CJsonPtr
    static std::optional<CJsonPtr> readFile(const std::string& filePath);
};

#endif