
#include <iostream>
#include <filesystem>

using namespace std;

int main() {
    string path = "data/transactions/txn_test_001";

    try {
        filesystem::create_directories(path + "/new");
        filesystem::create_directories(path + "/backup");

        if (filesystem::is_directory(path + "/new") &&
            filesystem::is_directory(path + "/backup")) {
            cout << "PASS: Tao thu muc thanh cong\n";
        }
        else {
            cout << "FAIL: Thieu thu muc\n";
        }
    }
    catch (const filesystem::filesystem_error& e) {
        cout << "FAIL: " << e.what() << '\n';
        return 1;
    }

    return 0;
}
