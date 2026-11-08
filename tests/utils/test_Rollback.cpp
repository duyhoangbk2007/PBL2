
#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include "../../core/utils/TransactionManager.h"
#include "../../core/utils/FileHandler.h"

using namespace std;

int main() {
    string originalPath = "data/test_rollback/members.txt";
    string transactionPath = "data/transactions/txn_test_007";

    if (filesystem::exists(transactionPath)) {
        cout << "FAIL: Thu muc giao dich da ton tai\n";
        return 1;
    }

    filesystem::create_directories("data/test_rollback");

    vector<string> oldLines = {"M001|An|20"};
    vector<string> newLines = {"M001|An|30"};

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

        bool result = FileHandler::readlines(
            originalPath, actualLines
        );

        if (result && actualLines == oldLines) {
            cout << "PASS: Rollback khoi phuc dung du lieu cu\n";
        }
        else {
            cout << "FAIL: Rollback khong thanh cong\n";
            return 1;
        }
    }
    catch (const exception& e) {
        cout << "FAIL: " << e.what() << '\n';
        return 1;
    }

    return 0;
}
