#include "JsonReader.h"
#include <fstream>
#include <iostream>

std::optional<nlohmann::json> JsonReader::readFile(const std::string& filePath) {
    std::ifstream jsonFile(filePath);
    if (!jsonFile.is_open()) {
        std::cerr << "Error: [JsonReader] Could not open file " << filePath << std::endl;
        return std::nullopt;
    }

    try {
        nlohmann::json data;
        jsonFile >> data;
        return data;
    } catch (nlohmann::json::parse_error& e) {
        std::cerr << "Error: [JsonReader] JSON parsing failed in file " << filePath << ": " << e.what() << std::endl;
        return std::nullopt;
    }
}