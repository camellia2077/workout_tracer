// controller/FileProcessorHandler.hpp

#ifndef FILE_PROCESSOR_HANDLER_H
#define FILE_PROCESSOR_HANDLER_H

#include "ActionHandler.hpp"
#include "reprocessor/facade/Reprocessor.hpp"

// 这个类专门处理与原始日志文件相关的所有操作。

class FileProcessorHandler {
public:
    bool handle(const AppConfig& config);

private:
    bool writeStringToFile(const std::string& filepath, const std::string& content);
    Reprocessor reprocessor_;
};

#endif // FILE_PROCESSOR_HANDLER_H