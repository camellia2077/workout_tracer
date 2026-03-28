// common/file_reader.hpp

#ifndef COMMON_FILE_READER_HPP_
#define COMMON_FILE_READER_HPP_

#include <string>
#include <vector>

class FileReader {
public:
  /**
   * @brief Recursively finds all files with a specific extension from a given path.
   * @param path The directory or file path to search.
   * @param extension The file extension to search for (e.g., ".txt", ".json").
   * @return A vector of strings containing full paths to matching files.
   */
  static auto FindFilesByExtension(const std::string& path, const std::string& extension) -> std::vector<std::string>;
};

#endif // COMMON_FILE_READER_HPP_