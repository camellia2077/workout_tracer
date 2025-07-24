#ifndef TXT_FILE_READER_H
#define TXT_FILE_READER_H

#include <string>
#include <vector>

class TxtFileReader {
public:
    /**
     * @brief Recursively finds all .txt files from a given path.
     *
     * If the path is a single .txt file, it returns a vector containing just that path.
     * If the path is a directory, it returns a vector of all .txt file paths found within it and its subdirectories.
     *
     * @param path The directory or file path to search.
     * @return A vector of strings, where each string is the full path to a .txt file.
     */
    static std::vector<std::string> getTxtFilePaths(const std::string& path);

private:
    // No private methods are needed for this specific functionality.
};

#endif // TXT_FILE_READER_H