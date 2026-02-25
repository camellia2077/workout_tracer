// application/file_processor_handler.cpp

#include "application/file_processor_handler.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <utility>
#include <vector>

#include "infrastructure/serializer/serializer.hpp"
#include "infrastructure/validation/validator.hpp"

namespace fs = std::filesystem;

namespace {
auto IsSupportedLogExtension(const fs::path& file_path) -> bool {
  const std::string extension = file_path.extension().string();
  return extension == ".txt" || extension == ".md";
}

auto CollectLogFiles(const std::string& path) -> std::vector<std::string> {
  std::vector<std::string> file_paths;

  if (!fs::exists(path)) {
    std::cerr << "Error: Input path does not exist: " << path << std::endl;
    return file_paths;
  }

  if (fs::is_directory(path)) {
    for (const auto& entry : fs::recursive_directory_iterator(path)) {
      if (!entry.is_regular_file()) {
        continue;
      }
      if (IsSupportedLogExtension(entry.path())) {
        file_paths.push_back(entry.path().string());
      }
    }
    return file_paths;
  }

  if (fs::is_regular_file(path)) {
    const fs::path file_path(path);
    if (IsSupportedLogExtension(file_path)) {
      file_paths.push_back(path);
      return file_paths;
    }

    std::cerr << "Warning: Unsupported log extension for file: " << path
              << std::endl;
    return file_paths;
  }

  std::cerr << "Error: Input path is neither file nor directory: " << path
            << std::endl;
  return file_paths;
}
}  // namespace

FileProcessorHandler::FileProcessorHandler(ILogParser& parser,
                                           IMappingProvider& mapping_provider)
    : converter_(parser, mapping_provider), validator_(mapping_provider) {}

auto FileProcessorHandler::Handle(const AppConfig& config) -> AppExitCode {
  if (!converter_.Configure(config.mapping_path_)) {
    return AppExitCode::kProcessingError;
  }

  std::vector<std::string> files_to_process =
      CollectLogFiles(config.log_filepath_);
  if (files_to_process.empty()) {
    std::cout << "Warning: No .txt or .md files found to process." << std::endl;
    return AppExitCode::kSuccess;
  }

  int success_count = 0;
  AppExitCode last_error = AppExitCode::kSuccess;

  for (const auto& file_path : files_to_process) {
    AppExitCode result = ProcessSingleFile(file_path, config);
    if (result == AppExitCode::kSuccess) {
      success_count++;
    } else {
      last_error = result;
    }
  }

  std::cout << "Processing complete. " << success_count << " of "
            << files_to_process.size() << " files handled successfully."
            << std::endl;

  return (success_count == static_cast<int>(files_to_process.size()))
             ? AppExitCode::kSuccess
             : last_error;
}

auto FileProcessorHandler::ProcessSingleFile(const std::string& file_path,
                                             const AppConfig& config)
    -> AppExitCode {
  std::cout << "===== File: " << file_path << " =====" << std::endl;
  AppExitCode result = AppExitCode::kUnknownError;

  if (config.action_ == ActionType::Validate) {
    std::cout << "Performing validation..." << std::endl;
    std::ifstream file(file_path);
    if (file.is_open()) {
      if (validator_.Validate(file, config.mapping_path_)) {
        std::cout << "Validation successful." << std::endl;
        result = AppExitCode::kSuccess;
      } else {
        std::cerr << "Validation failed." << std::endl;
        result = AppExitCode::kValidationError;
      }
    } else {
      std::cerr << "Error: Failed to open file for validation: " << file_path
                << std::endl;
      result = AppExitCode::kFileNotFound;
    }
  } else if (config.action_ == ActionType::Convert) {
    std::cout << "Performing conversion..." << std::endl;
    std::ifstream val_file(file_path);
    if (!val_file.is_open()) {
      std::cerr << "Error: Failed to open file: " << file_path << std::endl;
      result = AppExitCode::kFileNotFound;
    } else if (!validator_.Validate(val_file, config.mapping_path_)) {
      std::cerr << "Validation failed, skipping conversion." << std::endl;
      result = AppExitCode::kValidationError;
    } else {
      val_file.close();
      auto processed_data_opt = converter_.Convert(file_path);

      if (processed_data_opt.has_value() &&
          !processed_data_opt.value().empty()) {
        try {
          const std::string kOutputDirBase = "output/data";
          fs::path reprocessed_base_path =
              fs::path(config.base_path_) / kOutputDirBase;

          fs::create_directories(reprocessed_base_path);

          std::string base_filename =
              fs::path(file_path).stem().string() + ".json";
          fs::path output_filepath = reprocessed_base_path / base_filename;

          std::string output_content =
              Serializer::Serialize(processed_data_opt.value());

          std::cout << "Writing converted data to '" << output_filepath.string()
                    << "'..." << std::endl;
          if (WriteStringToFile(output_filepath.string(), output_content)) {
            result = AppExitCode::kSuccess;
            std::cout << "Conversion successful." << std::endl;
          } else {
            std::cerr << "Error writing file." << std::endl;
            result = AppExitCode::kProcessingError;
          }

        } catch (const fs::filesystem_error& e) {
          std::cerr << "Filesystem error during output: " << e.what()
                    << std::endl;
          result = AppExitCode::kProcessingError;
        }
      } else if (processed_data_opt.has_value()) {
        std::cout << "Conversion resulted in no data, skipping output."
                  << std::endl;
        result = AppExitCode::kSuccess;
      } else {
        std::cerr << "Conversion failed." << std::endl;
        result = AppExitCode::kProcessingError;
      }
    }
  }

  std::cout << "====================================\n" << std::endl;
  return result;
}

auto FileProcessorHandler::WriteStringToFile(const std::string& file_path,
                                             const std::string& content)
    -> bool {
  std::ofstream file(file_path);
  if (!file.is_open()) {
    std::cerr << "Error: Failed to open output file: " << file_path
              << std::endl;
    return false;
  }
  file << content;
  file.close();
  return true;
}

auto FileProcessorHandler::ProcessFile(const FileProcessingOptions& options)
    -> std::optional<std::vector<DailyData>> {
  if (!converter_.Configure(options.mapping_path_)) {
    return std::nullopt;
  }

  std::cout << "Validating file: " << options.file_path_ << std::endl;
  std::ifstream val_file(options.file_path_);
  if (!val_file.is_open() ||
      !validator_.Validate(val_file, options.mapping_path_)) {
    std::cerr << "Validation failed for " << options.file_path_ << std::endl;
    return std::nullopt;
  }
  val_file.close();

  std::cout << "Converting file: " << options.file_path_ << std::endl;
  return converter_.Convert(options.file_path_);
}
