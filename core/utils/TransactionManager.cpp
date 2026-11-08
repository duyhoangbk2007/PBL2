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

void TransactionManager::writeManifest(const std::vector<TransactionFile>& files, const string& transactionPath,const string& status) {
    vector<string> lines;
    string manifestPath = transactionPath + "/manifest.txt";

    if (status == "PREPARED") {
        // Trang thai giao dich
        lines.push_back(status);
        // So luong file
        lines.push_back(to_string(files.size()));
    
        for (size_t i = 0; i < files.size(); i++) {
            string existed = "0";
            if (filesystem::exists(files[i].path)) {
                existed = "1";
            }
            lines.push_back(files[i].path + "|" + existed);
        }
        // Ghi manifest
        if (!FileHandler::writeLines(manifestPath, lines)) {
            throw runtime_error("Khong the ghi manifest: " + manifestPath);
        }
    }
    
    else if (status == "COMMITTED") {
        if (!FileHandler::readlines(manifestPath, lines)) {
            throw runtime_error("Khong the doc manifest");
        }

        if (lines.size() < 2 || lines[0] != "PREPARED") {
            throw runtime_error("Manifest khong hop le");
        }

        lines[0] = "COMMITTED";

        string tempPath = transactionPath + "/manifest.tmp";

        if (!FileHandler::writeLines(tempPath, lines)) {
            throw runtime_error("Khong the ghi manifest tam");
        }

        filesystem::rename(tempPath, manifestPath);
    }
    else {
        throw runtime_error("Trang thai giao dich khong hop le");
    }
}

void TransactionManager::replaceFiles(const vector<TransactionFile>& files,const string& transactionPath) {
    for (size_t i = 0; i < files.size(); i++) {
        filesystem::path p(files[i].path);
        string fileName = p.filename().string();

        string newPath = transactionPath + "/new/" + fileName;
        string backupPath = transactionPath + "/backup/" + fileName;

        //is_regular_file() dùng để kiểm tra một đường dẫn có trỏ đến file thông thường (txt,cpp,h) hay không.
        if (!filesystem::is_regular_file(newPath)) {
            throw runtime_error("Khong tim thay file moi: " + newPath);
        }

        if (filesystem::exists(files[i].path) && !filesystem::is_regular_file(backupPath)) {
            throw runtime_error("Khong tim thay backup: " + backupPath);
        }
    }
    // Thay the file goc bang du lieu moi
    for (size_t i = 0; i < files.size(); i++) {
        filesystem::path p(files[i].path);
        string fileName = p.filename().string();

        string newPath = transactionPath + "/new/" + fileName;
        filesystem::copy_file(newPath, files[i].path, filesystem::copy_options::overwrite_existing);
    }
    //overwrite_existing cho phép ghi đè file gốc đã tồn tại.
}


void TransactionManager::rollback(const string& transactionPath) {
    vector<string> lines;
    string manifestPath = transactionPath + "/manifest.txt";

    if (!FileHandler::readlines(manifestPath, lines)) {
        throw runtime_error("Khong the doc manifest");
    }

    if (lines.size() < 2 || lines[0] != "PREPARED") {
        throw runtime_error("Manifest khong hop le");
    }
    // Bat dau tu dong thu 3, bo qua status va so luong file
    for (size_t i = 2; i < lines.size(); i++) {
        size_t pos = lines[i].find('|');

        if (pos == string::npos) { //npos là hằng số tĩnh, thường được định nghĩa bằng -1
            throw runtime_error("Dong manifest khong hop le");
        }
        //substr - cắt chuỗi ở vị trí pos
        string originalPath = lines[i].substr(0, pos);
        string existed = lines[i].substr(pos + 1);

        string fileName = filesystem::path(originalPath).filename().string();
        string backupPath = transactionPath + "/backup/" + fileName;

        if (existed == "1") {
            if (!filesystem::is_regular_file(backupPath)) {
                throw runtime_error("Khong tim thay backup");
            }

            filesystem::copy_file(backupPath, originalPath, filesystem::copy_options::overwrite_existing);
        }
        else if (existed == "0") {
            filesystem::remove(originalPath);
        }
        else {
            throw runtime_error("Trang thai file khong hop le");
        }
    }
}


void TransactionManager::execute(const vector<TransactionFile>& files) {
    string transactionPath = "data/transactions/txn_test_008";

    prepareFiles(files, transactionPath);
    writeManifest(files, transactionPath, "PREPARED");
    replaceFiles(files, transactionPath);
    rollback(transactionPath);
}

void TransactionManager::recover() {
    // Se cai dat sau
}
