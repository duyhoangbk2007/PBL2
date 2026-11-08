
#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include "../../core/utils/TransactionManager.h"
#include "../../core/utils/FileHandler.h"

using namespace std;

int main() {
    string originalPath = "data/test_replace/members.txt";
    string transactionPath = "data/transactions/txn_test_006";

    // Tranh su dung lai thu muc giao dich cu
    if (filesystem::exists(transactionPath)) {
        cout << "FAIL: Thu muc giao dich da ton tai\n";
        return 1;
    }

    filesystem::create_directories("data/test_replace");

    vector<string> oldLines = {"M001|An|20"};
    vector<string> newLines = {"M001|An|30"};

    // Tao file goc voi du lieu cu
    if (!FileHandler::writeLines(originalPath, oldLines)) {
        cout << "FAIL: Khong tao duoc file goc\n";
        return 1;
    }

    vector<TransactionFile> files = {
        {originalPath, newLines}
    };

    try {
        TransactionManager::execute(files);

        vector<string> actualLines;
        vector<string> backupLines;

        bool readOriginal = FileHandler::readlines(
            originalPath, actualLines
        );

        bool readBackup = FileHandler::readlines(
            transactionPath + "/backup/members.txt",
            backupLines
        );

        if (readOriginal && readBackup &&
            actualLines == newLines &&
            backupLines == oldLines) {
            cout << "PASS: Replace dung, Backup duoc giu nguyen\n";
        }
        else {
            cout << "FAIL: Du lieu khong dung\n";
            return 1;
        }
    }
    catch (const exception& e) {
        cout << "FAIL: " << e.what() << '\n';
        return 1;
    }

    return 0;
}
