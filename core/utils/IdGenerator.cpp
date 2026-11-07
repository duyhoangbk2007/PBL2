#include "IdGenerator.h"

#include <charconv>
#include <fstream>
#include <limits>
#include <map>

namespace {
bool isSupportedEntity(const std::string& entity) {
    return entity == "USR" || entity == "MOV" || entity == "ROM" ||
           entity == "SHW" || entity == "TKT" || entity == "INV" ||
           entity == "MEM";
}
}

bool IdGenerator::generateNextId(const std::string& entity,
                                 const std::string& sequencesPath,
                                 std::string& id,
                                 std::string& updatedSequences,
                                 std::string& error) {
    id.clear();
    updatedSequences.clear();
    error.clear();

    if (!isSupportedEntity(entity)) {
        error = "Loại mã không hợp lệ: " + entity;
        return false;
    }

    std::ifstream input(sequencesPath);
    if (!input) {
        error = "Không mở được file sequences: " + sequencesPath;
        return false;
    }

    std::map<std::string, long long> sequences;
    std::string line;
    if (!std::getline(input, line) || (line != "#schema=1" && line != "#schema=1\r")) {
        error = "File sequences thiếu dòng #schema=1.";
        return false;
    }
    std::size_t lineNumber = 1;
    while (std::getline(input, line)) {
        ++lineNumber;
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty()) continue;

        const std::size_t separator = line.find('|');
        const std::string rowEntity = line.substr(0, separator);
        const std::string number = separator == std::string::npos ? "" : line.substr(separator + 1);
        long long nextValue = 0;
        const auto parsed = std::from_chars(number.data(), number.data() + number.size(), nextValue);
        if (parsed.ec != std::errc{} || parsed.ptr != number.data() + number.size() || !isSupportedEntity(rowEntity) ||
            nextValue <= 0 || sequences.count(rowEntity) != 0) {
            error = sequencesPath + ":" + std::to_string(lineNumber) + ": Dữ liệu không hợp lệ: " + line;
            return false;
        }
        sequences[rowEntity] = nextValue;
    }
    if (!input.eof()) {
        error = "Lỗi khi đọc file sequences.";
        return false;
    }

    auto sequence = sequences.find(entity);
    if (sequence == sequences.end()) {
        error = "Không tìm thấy bộ đếm: " + entity;
        return false;
    }
    if (sequence->second == std::numeric_limits<long long>::max()) {
        error = "Bộ đếm đã đạt giới hạn: " + entity;
        return false;
    }

    std::string number = std::to_string(sequence->second);
    if (number.size() < 6) number.insert(0, 6 - number.size(), '0');
    ++sequence->second;

    updatedSequences = "#schema=1\n";
    for (const auto& entry : sequences) {
        updatedSequences += entry.first + '|' + std::to_string(entry.second) + '\n';
    }
    id = entity + number;
    return true;
}
