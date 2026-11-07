#include "TransactionManager.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include <fstream>
#include <iterator>
#include <map>
#include <sstream>
#include <stdexcept>

namespace {
namespace fs = std::filesystem;

bool validName(const std::string& name) {
    return name == "users.txt" || name == "movies.txt" || name == "rooms.txt" ||
           name == "seats.txt" || name == "showtimes.txt" || name == "invoices.txt" ||
           name == "tickets.txt" || name == "members.txt" || name == "sequences.txt";
}

std::string readFile(const fs::path& path) {
    if (!fs::is_regular_file(path) || fs::is_symlink(path))
        throw std::runtime_error("File không hợp lệ: " + path.string());
    std::ifstream input(path, std::ios::binary);
    if (!input) throw std::runtime_error("Không đọc được: " + path.string());
    std::string content((std::istreambuf_iterator<char>(input)), {});
    if (input.bad()) throw std::runtime_error("Lỗi đọc: " + path.string());
    return content;
}

// Ghi và đẩy dữ liệu xuống đĩa trước khi thay file chính.
void writeFile(const fs::path& path, const std::string& content) {
    if (content.size() > MAXDWORD) throw std::runtime_error("File vượt giới hạn 4 GB.");
    HANDLE file = CreateFileW(path.c_str(), GENERIC_WRITE, 0, nullptr,
                              CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE)
        throw std::runtime_error("Không mở được: " + path.string());
    DWORD written = 0;
    const bool success = WriteFile(file, content.data(), static_cast<DWORD>(content.size()),
                                    &written, nullptr) && written == content.size() && FlushFileBuffers(file);
    CloseHandle(file);
    if (!success) throw std::runtime_error("Lỗi ghi: " + path.string());
}

void replaceFile(const fs::path& directory, const fs::path& target, const std::string& content) {
    const fs::path temporary = directory / "replace.tmp";
    writeFile(temporary, content);
    if (!MoveFileExW(temporary.c_str(), target.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
        throw std::runtime_error("Không thay được: " + target.string());
}

std::map<std::string, bool> readManifest(const fs::path& directory, std::string& state) {
    std::istringstream input(readFile(directory / "manifest.txt"));
    std::string line;
    std::getline(input, line);
    std::getline(input, state);
    if (line != "#transaction=1" || (state != "PREPARED" && state != "COMMITTED"))
        throw std::runtime_error("Manifest hỏng: " + directory.string());
    std::map<std::string, bool> files;
    while (std::getline(input, line)) {
        const std::string name = line.substr(0, line.find('|'));
        if (!validName(name) || (line != name + "|0" && line != name + "|1") || files.count(name))
            throw std::runtime_error("Manifest có dòng sai: " + line);
        files[name] = line.back() == '1';
        if (state == "PREPARED" && files[name]) readFile(directory / (name + ".bak"));
    }
    if (files.empty()) throw std::runtime_error("Manifest rỗng.");
    return files;
}

// Đánh dấu đã xử lý trước khi dọn, để lần khởi động sau có thể tiếp tục dọn.
bool cleanup(const fs::path& directory) {
    try {
        const fs::path done = directory.parent_path() / ("done-" + directory.filename().string().substr(4));
        if (!MoveFileExW(directory.c_str(), done.c_str(), MOVEFILE_WRITE_THROUGH)) return false;
        std::error_code ignored;
        fs::remove_all(done, ignored);
        return true;
    } catch (const std::exception&) {
        return false;
    }
}
}

TransactionManager::TransactionManager(const std::string& directory)
    : dataDirectory(std::filesystem::absolute(directory).lexically_normal()) {}

bool TransactionManager::recover(std::string& error) {
    ready = false;
    error.clear();
    try {
        if (!fs::is_directory(dataDirectory)) throw std::runtime_error("Thiếu thư mục dữ liệu.");
        const fs::path transactions = dataDirectory / "transactions";
        fs::create_directory(transactions);
        if (fs::is_symlink(transactions)) throw std::runtime_error("Vùng giao dịch không được là symlink.");
        // Lấy danh sách trước vì phục hồi sẽ đổi tên/xóa thư mục bên dưới.
        std::vector<fs::path> directories;
        for (const auto& item : fs::directory_iterator(transactions)) directories.push_back(item.path());
        for (const fs::path& directory : directories) {
            const std::string name = directory.filename().string();
            if (!fs::is_directory(directory) || fs::is_symlink(directory)) throw std::runtime_error("Vùng giao dịch không hợp lệ.");
            if (name.rfind("done-", 0) == 0) {
                std::error_code ignored;
                fs::remove_all(directory, ignored);
                continue;
            }
            if (name.rfind("txn-", 0) != 0) throw std::runtime_error("Tên giao dịch không hợp lệ.");
            if (fs::exists(directory / "manifest.txt")) {
                std::string state;
                const auto files = readManifest(directory, state);
                for (const auto& entry : files) {
                    const fs::path target = dataDirectory / entry.first;
                    if (state == "COMMITTED") {
                        if (readFile(target) != readFile(directory / (entry.first + ".new")))
                            throw std::runtime_error("File đã commit không khớp: " + entry.first);
                    } else if (entry.second) {
                        const std::string backup = readFile(directory / (entry.first + ".bak"));
                        if (!fs::exists(target) || readFile(target) != backup)
                            replaceFile(directory, target, backup);
                    } else if (fs::exists(target)) {
                        if (!fs::is_regular_file(target) || fs::is_symlink(target))
                            throw std::runtime_error("Không thể rollback: " + entry.first);
                        // Chuyển ra khỏi bộ dữ liệu bằng thao tác ghi bền vững trước khi dọn.
                        const fs::path removed = directory / (entry.first + ".removed");
                        if (!MoveFileExW(target.c_str(), removed.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
                            throw std::runtime_error("Không thể rollback: " + entry.first);
                    }
                }
            } else {
                // Chưa có manifest thì file chính chưa bị thay; chỉ kiểm tra và dọn vùng tạm.
                for (const auto& staged : fs::directory_iterator(directory)) {
                    if (!staged.is_regular_file() || staged.is_symlink())
                        throw std::runtime_error("Vùng chuẩn bị không hợp lệ.");
                }
            }
            if (!cleanup(directory)) throw std::runtime_error("Chưa dọn xong giao dịch.");
        }
        ready = true;
        return true;
    } catch (const std::exception& exception) {
        error = exception.what();
        return false;
    }
}

bool TransactionManager::commit(const std::vector<FileUpdate>& updates, std::string& error) {
    error.clear();
    if (!ready) {
        error = "Cần recover thành công trước khi ghi.";
        return false;
    }
    try {
        if (updates.empty()) throw std::runtime_error("Giao dịch rỗng.");
        std::map<std::string, bool> files;
        for (const FileUpdate& update : updates) {
            if (!validName(update.fileName) || files.count(update.fileName))
                throw std::runtime_error("Tên file sai hoặc bị lặp: " + update.fileName);
            files[update.fileName] = fs::exists(dataDirectory / update.fileName);
        }
        fs::path directory;
        unsigned int counter = 0;
        do {
            directory = dataDirectory / "transactions" / ("txn-" + std::to_string(GetCurrentProcessId()) +
                        "-" + std::to_string(GetTickCount64()) + "-" + std::to_string(counter++));
        } while (!fs::create_directory(directory));

        std::string fileList;
        for (const FileUpdate& update : updates) {
            const bool existed = files.at(update.fileName);
            if (existed) writeFile(directory / (update.fileName + ".bak"), readFile(dataDirectory / update.fileName));
            writeFile(directory / (update.fileName + ".new"), update.content);
            fileList += update.fileName + (existed ? "|1\n" : "|0\n");
        }
        replaceFile(directory, directory / "manifest.txt", "#transaction=1\nPREPARED\n" + fileList);
        for (const FileUpdate& update : updates)
            replaceFile(directory, dataDirectory / update.fileName, update.content);
        replaceFile(directory, directory / "manifest.txt", "#transaction=1\nCOMMITTED\n" + fileList);
        ready = cleanup(directory);
        return true;
    } catch (const std::exception& exception) {
        error = exception.what();
        std::string recoveryError;
        if (!recover(recoveryError)) error += "; Phục hồi thất bại: " + recoveryError;
        return false;
    }
}
