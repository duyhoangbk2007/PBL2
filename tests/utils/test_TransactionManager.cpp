
#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include "../../core/utils/TransactionManager.h"
#include "../../core/utils/FileHandler.h"

using namespace std;

int main() {
    vector<TransactionFile> files = {
        {
            "data/movies.txt",
            {
                "#schema=1",
                "MOV000001|Doraemon|Animation|100|0|2026-11-01|2026-11-30|1"
            }
        },
        {
            "data/members.txt",
            {
                "#schema=1",
                "MEM000001|Nguyen Van An|0123456789|30"
            }
        }
    };

    try {
        TransactionManager::execute(files);

        string folder = "data/transactions/txn_test_002/new/";

        vector<string> movieLines;
        vector<string> memberLines;

        bool movieRead = FileHandler::readlines(
            folder + "movies.txt", movieLines
        );

        bool memberRead = FileHandler::readlines(
            folder + "members.txt", memberLines
        );

        if (movieRead && memberRead &&
            movieLines == files[0].newLines &&
            memberLines == files[1].newLines) {
            cout << "PASS: prepareFiles ghi dung du lieu\n";
        }
        else {
            cout << "FAIL: Du lieu file moi khong dung\n";
            return 1;
        }
    }
    catch (const exception& e) {
        cout << "FAIL: " << e.what() << '\n';
        return 1;
    }

    return 0;
}
