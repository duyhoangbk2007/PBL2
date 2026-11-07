#include "TransactionManager.h"
#include "FileHandler.h"
#include <iostream>
#include <stdexcept>
#include <filesystem>

using namespace std;

void TransactionManager::prepareFiles(const vector<TransactionFile>& files, const string& transactionPath) {
    filesystem::create_directories(transactionPath + "/new");
    filesystem::create_directories(transactionPath + "/backup");

    for (size_t i = 0; i < files.size(); i++) {
        filesystem::path p(files[i].path);
        string fileName = p.filename().string();
        
        string newPath = transactionPath + "/new/" + fileName;
        string backupPath = transactionPath + "/backup/" + fileName;

        //new
        if (!FileHandler::writeLines(newPath, files[i].newLines)) {
            throw runtime_error("Khong the ghi file moi: " + newPath);
        }

        //backup
        if (filesystem::exists(files[i].path))  { //kiểm tra đường dẫn file gốc có tồn tại không
            bool copied = filesystem::copy_file(files[i].path, backupPath, filesystem::copy_options::none);
            if (copied == false)
                throw runtime_error("Khong the tao backup: " + backupPath);
        }
    }
}

void TransactionManager::writeManifest(const string& transactionPath,const string& status) {
    // Se cai dat o buoc sau
}

void TransactionManager::replaceFiles(const vector<TransactionFile>& files,const string& transactionPath) {
    // Se cai dat o buoc sau
}

void TransactionManager::rollback(const string& transactionPath) {
    // Se cai dat o buoc sau
}

void TransactionManager::execute(const vector<TransactionFile>& files) {
    string transactionPath = "data/transactions/txn_test_004";
    prepareFiles(files, transactionPath);
}

void TransactionManager::recover() {
    // Se cai dat sau
}
