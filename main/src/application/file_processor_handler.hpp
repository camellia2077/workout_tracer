// application/file_processor_handler.hpp

#ifndef APPLICATION_FILE_PROCESSOR_HANDLER_HPP_
#define APPLICATION_FILE_PROCESSOR_HANDLER_HPP_

#include "application/action_handler.hpp"
#include "application/interfaces/i_log_parser.hpp"
#include "application/interfaces/i_mapping_provider.hpp"
#include "infrastructure/converter/converter.hpp"
#include "infrastructure/validation/validator.hpp"

// 这个类专门处理与原始日志文件相关的所有操作。
struct FileProcessingOptions {
  std::string file_path_;
  std::string mapping_path_;
};

class FileProcessorHandler {
public:
  FileProcessorHandler(ILogParser& parser, IMappingProvider& mapping_provider);
  
  [[nodiscard]] auto Handle(const AppConfig& config) -> bool;
  [[nodiscard]] auto ProcessFile(const FileProcessingOptions& options)
      -> std::optional<std::vector<DailyData>>;

private:
  [[nodiscard]] static auto WriteStringToFile(const std::string& file_path,
                                              const std::string& content) -> bool;
  
  [[nodiscard]] auto ProcessSingleFile(const std::string& file_path, const AppConfig& config) -> bool;

  Converter converter_;
  Validator validator_;
};

#endif // APPLICATION_FILE_PROCESSOR_HANDLER_HPP_