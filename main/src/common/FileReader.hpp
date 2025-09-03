// src/common/FileReader.hpp

#ifndef FILE_READER_H
#define FILE_READER_H

#include <string>
#include <vector>

class FileReader {
public:
    /**
     * @brief Recursively finds all files with a specific extension from a given path.
     *
     * If the path is a single file matching the extension, it returns a vector containing just that path.
     * If the path is a directory, it returns a vector of all matching file paths found within it and its subdirectories.
     *
     * @param path The directory or file path to search.
     * @param extension The file extension to search for (e.g., ".txt", ".json").
     * @return A vector of strings, where each string is the full path to a matching file.
     */
    static std::vector<std::string> findFilesByExtension(const std::string& path, const std::string& extension);
};

#endif // FILE_READER_H