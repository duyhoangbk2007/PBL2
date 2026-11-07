#ifndef PBL2_TRANSACTION_MANAGER_H
#define PBL2_TRANSACTION_MANAGER_H

#include <filesystem>
#include <string>
#include <vector>

class TransactionManager {
public:
    struct FileUpdate {
        std::string fileName;
        std::string content;
    };

    explicit TransactionManager(const std::string& dataDirectory);

    // Gọi sau khi khóa bộ dữ liệu, trước khi tải dữ liệu vào bộ nhớ.
    bool recover(std::string& error);

    // Nội dung phải được kiểm tra nghiệp vụ/schema trước khi gọi.
    bool commit(const std::vector<FileUpdate>& updates, std::string& error);

private:
    std::filesystem::path dataDirectory;
    bool ready = false;
};

#endif
