
#include <iostream>
#include <vector>
#include <string>
#include <filesystem>

#include "../../core/utils/TransactionManager.h"
#include "../../core/utils/FileHandler.h"

using namespace std;

int main() {
    string folder = "data/test_recovery_files";
    string transactionDir = "data/test_recovery_transactions";
    string txn = transactionDir + "/txn_case_1";

    string originalPath = folder + "/members.txt";

    // Khong dung lai du lieu test cu
    if (filesystem::exists(folder) ||
        filesystem::exists(txn)) {
        cout << "FAIL: Du lieu test da ton tai\n";
        return 1;
    }

    try {
        filesystem::create_directories(folder);
        filesystem::create_directories(txn + "/backup");
        filesystem::create_directories(txn + "/new");

        vector<string> oldData = {"M001|An|20"};
        vector<string> newData = {"M001|An|30"};

        // Gia lap file goc da bi thay bang du lieu moi
        if (!FileHandler::writeLines(originalPath, newData)) {
            throw runtime_error("Khong tao duoc file goc");
        }

        // Tao ban sao du lieu cu
        if (!FileHandler::writeLines(
                txn + "/backup/members.txt", oldData)) {
            throw runtime_error("Khong tao duoc backup");
        }

        // Tao manifest PREPARED
        vector<string> manifest = {
            "PREPARED",
            "1",
            originalPath + "|1"
        };

        if (!FileHandler::writeLines(
                txn + "/manifest.txt", manifest)) {
            throw runtime_error("Khong tao duoc manifest");
        }

        // Thu phuc hoi sau su co
        TransactionManager::recover(transactionDir);

        vector<string> result;

        bool readOK = FileHandler::readlines(
            originalPath, result
        );

        if (readOK && result == oldData) {
            cout << "PASS: Recovery khoi phuc dung du lieu cu\n";
        }
        else {
            cout << "FAIL: Recovery khong dung\n";
            return 1;
        }
    }
    catch (const exception& e) {
        cout << "FAIL: " << e.what() << '\n';
        return 1;
    }

    return 0;
}
