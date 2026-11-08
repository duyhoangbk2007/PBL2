
#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include "../../core/utils/TransactionManager.h"
#include "../../core/utils/FileHandler.h"

using namespace std;

int main() {
    // Tao thu muc va file goc de test
    filesystem::create_directories("data/test_manifest");

    vector<string> oldLines = {"Movie cu"};

    if (!FileHandler::writeLines("data/test_manifest/movies.txt", oldLines)) {
        cout << "FAIL: Khong tao duoc file goc\n";
        return 1;
    }

    vector<TransactionFile> files = {
        {
            "data/test_manifest/movies.txt",
            {"Movie moi"}
        },
        {
            "data/test_manifest/members.txt",
            {"Member moi"}
        }
    };

    try {
        TransactionManager::execute(files);

        vector<string> lines;

        bool result = FileHandler::readlines(
            "data/transactions/txn_test_005/manifest.txt",
            lines
        );

        vector<string> expected = {
            "PREPARED",
            "2",
            "data/test_manifest/movies.txt|1",
            "data/test_manifest/members.txt|0"
        };

        if (result && lines == expected) {
            cout << "PASS: Manifest dung noi dung\n";
        }
        else {
            cout << "FAIL: Manifest sai noi dung\n";
            return 1;
        }
    }
    catch (const exception& e) {
        cout << "FAIL: " << e.what() << '\n';
        return 1;
    }

    return 0;
}
