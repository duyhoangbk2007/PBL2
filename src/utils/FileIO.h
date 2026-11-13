#ifndef PBL2_FILE_IO_H
#define PBL2_FILE_IO_H

#include <string>
#include <vector>

class FileIO {
public:
    static bool readLines(const std::string& path, std::vector<std::string>& lines);
    static bool writeLines(const std::string& path, const std::vector<std::string>& lines);
};

#endif
