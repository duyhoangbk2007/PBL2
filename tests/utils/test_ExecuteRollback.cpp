
#include <iostream>
#include <vector>
#include <string>
#include <filesystem>

#include "../../core/utils/TransactionManager.h"
#include "../../core/utils/FileHandler.h"

using namespace std;

int main()
{
    string folder = "data/test_execute_rollback";

    filesystem::create_directories(folder);

    vector<string> oldTickets = {"T001"};
    vector<string> oldMembers = {"M001|An|20"};

    vector<string> newTickets = {"T001", "T002"};
    vector<string> newMembers = {"M001|An|30"};

    if (!FileHandler::writeLines(folder + "/tickets.txt", oldTickets) ||
        !FileHandler::writeLines(folder + "/members.txt", oldMembers))
    {
        cout << "FAIL: Khong tao duoc du lieu test\n";
        return 1;
    }

    vector<TransactionFile> files = {
        {folder + "/tickets.txt", newTickets},
        {folder + "/members.txt", newMembers}};

    bool hasError = false;

    try
    {
        TransactionManager::execute(files);
    }
    catch (const exception &e)
    {
        hasError = true;
        cout << "Da bat duoc loi: " << e.what() << '\n';
    }

    vector<string> actualTickets;
    vector<string> actualMembers;

    bool result1 = FileHandler::readlines(
        folder + "/tickets.txt", actualTickets);

    bool result2 = FileHandler::readlines(
        folder + "/members.txt", actualMembers);

    if (hasError && result1 && result2 &&
        actualTickets == oldTickets &&
        actualMembers == oldMembers)
    {
        cout << "PASS: Execute da rollback thanh cong\n";
    }
    else
    {
        cout << "FAIL: Rollback khong dung\n";
        return 1;
    }

    return 0;
}
