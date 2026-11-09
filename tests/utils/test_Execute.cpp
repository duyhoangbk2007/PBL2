
#include <iostream>
#include <vector>
#include <string>
#include <filesystem>

#include "../../core/utils/TransactionManager.h"
#include "../../core/utils/FileHandler.h"

using namespace std;

int main() {
    string folder = "data/test_execute";

    filesystem::create_directories(folder);

    // Tao du lieu ban dau
    vector<string> oldTickets = {"T001|MOV001|A1"};
    vector<string> oldMembers = {"M001|An|20"};

    FileHandler::writeLines(folder + "/tickets.txt", oldTickets);
    FileHandler::writeLines(folder + "/members.txt", oldMembers);

    // Du lieu sau khi ban ve
    vector<string> newTickets = {
        "T001|MOV001|A1",
        "T002|MOV001|A2"
    };

    vector<string> newMembers = {"M001|An|30"};

    vector<TransactionFile> files = {
        {folder + "/tickets.txt", newTickets},
        {folder + "/members.txt", newMembers}
    };

    try {
        TransactionManager::execute(files);

        vector<string> actualTickets;
        vector<string> actualMembers;

        bool result1 = FileHandler::readlines(
            folder + "/tickets.txt", actualTickets
        );

        bool result2 = FileHandler::readlines(
            folder + "/members.txt", actualMembers
        );

        if (result1 && result2 &&
            actualTickets == newTickets &&
            actualMembers == newMembers) {
            cout << "PASS: Execute cap nhat dung du lieu\n";
        }
        else {
            cout << "FAIL: Du lieu sau giao dich khong dung\n";
            return 1;
        }
    }
    catch (const exception& e) {
        cout << "FAIL: " << e.what() << '\n';
        return 1;
    }

    return 0;
}
