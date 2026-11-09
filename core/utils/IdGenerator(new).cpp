#include "IdGenerator(new).h"

#include <charconv>
#include <fstream>
#include <limits>
#include <map>
#include <stdexcept>

using namespace std;

namespace { //giới hạn hàm này trong idGen.cpp, chỉ phục vụ 1 file .cpp
    bool isSupportedEntity(const string& entity) {
        return entity == "USR" || entity == "MOV" || entity == "ROM" ||
           entity == "SHW" || entity == "TKT" || entity == "INV" ||
           entity == "MEM";
        }
}

void IdGenerator::generateNextId(
    const string& entity,
    const string& sequencesPath,
    string& id,
    string& updatedSequences)
{

    id.clear();
    updatedSequences.clear();
    if (!isSupportedEntity(entity)) {
        throw runtime_error("Loai ma khong hop le: " + entity);
    }
    ifstream input(sequencesPath);
    if (!input) {
        throw runtime_error("Khong mo duoc file sequences: " + sequencesPath);
    }

    map<string, long long> sequences; //key là Entity, value là giá trị tương ứng
    string line;
    if (!getline(input, line) || (line != "#schema=1" && line != "#schema=1\r")) {
        throw runtime_error("File sequences thieu dong #schema=1");
    }

    size_t lineNumber = 1;
    while (getline(input, line)) {
        ++lineNumber;
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        if (line.empty()) {
            continue;
        }
        size_t separator = line.find('|');

        string rowEntity = line.substr(0, separator);

        string number = separator == string::npos
            ? ""
            : line.substr(separator + 1);

        long long nextValue = 0;

        auto parsed = from_chars(
            number.data(),
            number.data() + number.size(),
            nextValue
        );

        if (parsed.ec != errc{} ||
            parsed.ptr != number.data() + number.size() ||
            !isSupportedEntity(rowEntity) ||
            nextValue <= 0 ||
            sequences.count(rowEntity) != 0) {
            throw runtime_error(
                sequencesPath + ":" + to_string(lineNumber) +
                ": Du lieu khong hop le: " + line
            );
        }

        sequences[rowEntity] = nextValue;
    }

    if (!input.eof()) {
        throw runtime_error("Loi khi doc file sequences");
    }

    auto sequence = sequences.find(entity);

    if (sequence == sequences.end()) {
        throw runtime_error("Khong tim thay bo dem: " + entity);
    }

    if (sequence->second == numeric_limits<long long>::max()) {
        throw runtime_error("Bo dem da dat gioi han: " + entity);
    }

    string number = to_string(sequence->second);

    if (number.size() < 6) {
        number.insert(0, 6 - number.size(), '0');
    }

    ++sequence->second;

    updatedSequences = "#schema=1\n";

    for (const auto& entry : sequences) {
        updatedSequences += entry.first + "|" +
                            to_string(entry.second) + '\n';
    }

    id = entity + number;
}
