// application/file_processor_handler.cpp

#include "application/file_processor_handler.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <utility>
#include <vector>

#include "common/file_reader.hpp"
#include "infrastructure/serializer/serializer.hpp"
#include "infrastructure/validation/validator.hpp"

namespace fs = std::filesystem;

FileProcessorHandler::FileProcessorHandler(ILogParser& parser,
                                           IMappingProvider& mapping_provider)
    : converter_(parser, mapping_provider), validator_(mapping_provider) {}

auto FileProcessorHandler::Handle(const AppConfig& config) -> bool {
  if (!converter_.Configure(config.mapping_path_)) {
    return false;
  }

  std::vector<std::string> files_to_process =
      FileReader::FindFilesByExtension(config.log_filepath_, ".txt");
  if (files_to_process.empty()) {
    std::cout << "Warning: No .txt files found to process." << std::endl;
    return true;
  }

  int success_count = 0;
  for (const auto& file_path : files_to_process) {
    if (ProcessSingleFile(file_path, config)) {
      success_count++;
    }
  }

  std::cout << "Processing complete. " << success_count << " of "
            << files_to_process.size() << " files handled successfully."
            << std::endl;
  return std::cmp_equal(success_count, files_to_process.size());
}

auto FileProcessorHandler::ProcessSingleFile(const std::string& file_path,
                                             const AppConfig& config) -> bool {
  std::cout << "===== File: " << file_path << " =====" << std::endl;
  bool result = false;

  if (config.action_ == ActionType::Validate) {
    std::cout << "Performing validation..." << std::endl;
    std::ifstream file(file_path);
    if (file.is_open() && validator_.Validate(file, config.mapping_path_)) {
      std::cout << "Validation successful." << std::endl;
      result = true;
    } else {
      std::cerr << "Validation failed." << std::endl;
    }
  } else if (config.action_ == ActionType::Convert) {
    std::cout << "Performing conversion..." << std::endl;
    std::ifstream val_file(file_path);
    if (!val_file.is_open() ||
        !validator_.Validate(val_file, config.mapping_path_)) {
      std::cerr << "Validation failed, skipping conversion." << std::endl;
    } else {
      val_file.close();
      auto processed_data_opt = converter_.Convert(file_path);

      if (processed_data_opt.has_value() &&
          !processed_data_opt.value().empty()) {
        try {
          const std::string kOutputDirBase = "reprocessed_json";
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
            result = true;
            std::cout << "Conversion successful." << std::endl;
          } else {
            std::cerr << "Error writing file." << std::endl;
          }

        } catch (const fs::filesystem_error& e) {
          std::cerr << "Filesystem error during output: " << e.what()
                    << std::endl;
        }
      } else if (processed_data_opt.has_value()) {
        std::cout << "Conversion resulted in no data, skipping output."
                  << std::endl;
        result = true;
      } else {
        std::cerr << "Conversion failed." << std::endl;
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