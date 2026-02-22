// common/file_reader.cpp

#include "common/file_reader.hpp"

#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

auto FileReader::FindFilesByExtension(const std::string& path,
                                      const std::string& extension)
    -> std::vector<std::string> {
  std::vector<std::string> file_paths;

  if (!fs::exists(path)) {
    std::cerr << "Error: [FileReader] Path does not exist: " << path
              << std::endl;
    return file_paths;
  }

  if (fs::is_directory(path)) {
    std::cout << "[FileReader] Path is a directory. Recursively searching for '"
              << extension << "' files..." << std::endl;
    for (const auto& entry : fs::recursive_directory_iterator(path)) {
      if (entry.is_regular_file() && entry.path().extension() == extension) {
        file_paths.push_back(entry.path().string());
      }
    }
  } else if (fs::is_regular_file(path)) {
    if (fs::path(path).extension() == extension) {
      file_paths.push_back(path);
    } else {
      std::cerr << "Warning: [FileReader] Specified file does not have the '"
                << extension << "' extension: " << path << std::endl;
    }
  } else {
    std::cerr << "Error: [FileReader] Path is not a regular file or directory: "
              << path << std::endl;
  }

  if (file_paths.empty()) {
    std::cout << "[FileReader] No '" << extension
              << "' files were found at the specified path." << std::endl;
  }

  return file_paths;
}