// reprocessor/facade/Reprocessor.cpp

#include "reprocessor/facade/Reprocessor.hpp"
#include "common/JsonReader.hpp"
#include "reprocessor/validator/Validator.hpp"
#include <iostream>

bool Reprocessor::configure(const std::string& mappingFilePath) {
    mappingFilePath_ = mappingFilePath;
    auto jsonDataOpt = JsonReader::readFile(mappingFilePath);
    if (!jsonDataOpt.has_value()) {
        std::cerr << "Error: [Reprocessor] Failed to read or parse mapping file: " << mappingFilePath << std::endl;
        return false;
    }
    if (!mapper.loadMappings(jsonDataOpt.value())) {
        std::cerr << "Error: [Reprocessor] Failed to load mappings from JSON data." << std::endl;
        return false;
    }
    std::cout << "[Reprocessor] Configuration successful. Mappings loaded from " << mappingFilePath << std::endl;
    return true;
}

bool Reprocessor::validate(const std::string& logFilePath) const {
    return Validator::validate(logFilePath, mappingFilePath_);
}

std::optional<std::vector<DailyData>> Reprocessor::convert(const std::string& logFilePath) {
    return preprocessor.process(logFilePath, mapper);
}