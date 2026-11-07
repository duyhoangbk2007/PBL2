
#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include "../../core/utils/TransactionManager.h"
#include "../../core/utils/FileHandler.h"

using namespace std;

int main() {
    // Tao du lieu test rieng, khong dung file that
    filesystem::create_directories("data/test_backup");

    vector<string> oldLines = {"Movie cu", "Gia ve cu"};

    if (!FileHandler::writeLines("data/test_backup/movies.txt", oldLines)) {
        cout << "FAIL: Khong tao duoc file goc\n";
        return 1;
    }

    vector<TransactionFile> files = {
        {
            "data/test_backup/movies.txt",
            {"Movie moi", "Gia ve moi"}
        }
    };

    try {
        TransactionManager::execute(files);

        vector<string> backupLines;

        bool result = FileHandler::readlines(
            "data/transactions/txn_test_004/backup/movies.txt",
            backupLines
        );

        if (result && backupLines == oldLines) {
            cout << "PASS: Backup dung du lieu cu\n";
        }
        else {
            cout << "FAIL: Backup khong dung\n";
            return 1;
        }
    }
    catch (const exception& e) {
        cout << "FAIL: " << e.what() << '\n';
        return 1;
    }

    return 0;
}
