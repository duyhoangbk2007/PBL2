
#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include "../../core/utils/TransactionManager.h"

using namespace std;

int main() {
    string filePath = "data/test_rollback/new_member.txt";
    string transactionPath = "data/transactions/txn_test_008";

    // Khong su dung lai du lieu test cu
    if (filesystem::exists(transactionPath) ||
        filesystem::exists(filePath)) {
        cout << "FAIL: Du lieu test da ton tai\n";
        return 1;
    }

    filesystem::create_directories("data/test_rollback");

    vector<TransactionFile> files = {
        {filePath, {"M002|Binh|10"}}
    };

    try {
        TransactionManager::execute(files);

        if (!filesystem::exists(filePath)) {
            cout << "PASS: Rollback da xoa file moi\n";
        }
        else {
            cout << "FAIL: File moi van con ton tai\n";
            return 1;
        }
    }
    catch (const exception& e) {
        cout << "FAIL: " << e.what() << '\n';
        return 1;
    }

    return 0;
}
