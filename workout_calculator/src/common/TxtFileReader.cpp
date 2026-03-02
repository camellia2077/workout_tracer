#include "TxtFileReader.hpp"
#include <filesystem>
#include <iostream>

// Using a namespace alias for convenience
namespace fs = std::filesystem;

std::vector<std::string> TxtFileReader::getTxtFilePaths(const std::string& path) {
    std::vector<std::string> filePaths;

    if (!fs::exists(path)) {
        std::cerr << "Error: [TxtFileReader] Path does not exist: " << path << std::endl;
        return filePaths;
    }

    if (fs::is_directory(path)) {
        std::cout << "[TxtFileReader] Path is a directory. Recursively searching for .txt files..." << std::endl;
        for (const auto& entry : fs::recursive_directory_iterator(path)) {
            if (entry.is_regular_file() && entry.path().extension() == ".txt") {
                filePaths.push_back(entry.path().string());
            }
        }
    } else if (fs::is_regular_file(path)) {
        if (fs::path(path).extension() == ".txt") {
            filePaths.push_back(path);
        } else {
             std::cerr << "Warning: [TxtFileReader] Specified file is not a .txt file: " << path << std::endl;
        }
    } else {
        std::cerr << "Error: [TxtFileReader] Path is not a regular file or directory: " << path << std::endl;
    }

    if (filePaths.empty()) {
        std::cout << "[TxtFileReader] No .txt files were found at the specified path." << std::endl;
    }

    return filePaths;
}