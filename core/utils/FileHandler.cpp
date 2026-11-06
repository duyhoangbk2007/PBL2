#include "FileHandler.h"
#include <fstream>

using namespace std;

bool FileHandler::readlines(const string& path, vector<string>& lines) {
    ifstream file(path);

    if (!file.is_open()) return false;
    lines.clear();
    string line;
    while (getline(file, line)) {
        lines.push_back(line);
    }
    return !file.bad();
}
bool FileHandler::writeLines(const string& path, const vector<string>& lines) {
    ofstream file(path);

    if (!file.is_open())
        return false;
    for (const string& line : lines) {
        file << line << '\n';
    }
    file.close();
    return !file.fail();
}