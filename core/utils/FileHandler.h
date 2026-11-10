#ifndef FILE_HANDLER_H
#define FILE_HANDLER_H

#include <string>
#include <vector>

class FileHandler {
public:
    static bool readlines(const std::string& path, std::vector<std::string>& lines);
    static bool writeLines(const std::string& path, const std::vector<std::string>& lines);
};

#endif