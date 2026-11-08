#ifndef TRANSACTION_MANAGER_H
#define TRANSACTION_MANAGER_H

#include <string>
#include <vector>

struct TransactionFile {
    std::string path;
    std::vector<std::string> newLines;
};

class TransactionManager
{
public:
    static void execute(const std::vector<TransactionFile>& files); //thực hiện giao dịch lưu nhiều file txt
    static void recover(); //phục hồi giao dịch đang dở khi khởi động ứng dụng
private:
    static void prepareFiles(const std::vector<TransactionFile>& files, const std::string& transactionPath);
    static void writeManifest(const std::vector<TransactionFile>& files, const std::string& transactionPath, const std::string& status);
    static void replaceFiles(const std::vector<TransactionFile>& files, const std::string& transactionPath);
    static void rollback(const std::string& transactionPath);
};

#endif