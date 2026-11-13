#include "FileIO.h"
#include <fstream>

using namespace std;

bool FileIO::readLines(const string& path, vector<string>& lines) {
    ifstream file(path);

    if (!file.is_open())
        return false;

    vector<string> temp;
    string line;

    while (getline(file, line)) {
        temp.push_back(line);
    }
    if (file.bad())
        return false;
    lines.swap(temp);
    return true;
}

bool FileIO::writeLines(const string& path, const vector<string>& lines) {
    ofstream file(path);
    if (!file.is_open())
        return false;
    for (const string& line : lines) {
        file << line << '\n';

        if (!file)
            return false;
    }
    file.close();
    return !file.fail();
}
    
