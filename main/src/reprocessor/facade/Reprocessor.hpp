// reprocessor/facade/Reprocessor.hpp

#ifndef REPROCESSOR_H
#define REPROCESSOR_H

#include "common/parsed_data.hpp"
#include "reprocessor/preprocessor/Preprocessor.hpp"
#include "reprocessor/preprocessor/name_mapper/ProjectNameMapper.hpp"
#include <optional>
#include <string>
#include <vector>

class Reprocessor {
public:
    bool configure(const std::string& mappingFilePath);
    bool validate(const std::string& logFilePath) const;
    std::optional<std::vector<DailyData>> convert(const std::string& logFilePath);

private:
    Preprocessor preprocessor;
    ProjectNameMapper mapper;
    std::string mappingFilePath_;
};

#endif // REPROCESSOR_H