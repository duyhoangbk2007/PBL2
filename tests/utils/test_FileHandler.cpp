
#include <iostream>
#include <vector>
#include <string>
#include "../../core/utils/FileHandler.h"

using namespace std;

int main() {
    const string path = "test_filehandler.txt";

    // TEST 1: Ghi du lieu vao file
    cout << "TEST 1: Ghi file\n";

    vector<string> data = {
        "#schema=1",
        "MOV000001|Doraemon|Animation|100|0|2026-11-01|2026-11-30|1",
        "MOV000002|Conan|Detective|110|13|2026-11-01|2026-11-30|1"
    };

    bool writeResult = FileHandler::writeLines(path, data);

    if (writeResult)
        cout << "PASS: Ghi file thanh cong\n";
    else
        cout << "FAIL: Khong ghi duoc file\n";

    // TEST 2: Doc va kiem tra noi dung file
    cout << "\nTEST 2: Doc file\n";

    vector<string> lines;
    bool readResult = FileHandler::readlines(path, lines);

    if (readResult && lines == data)
        cout << "PASS: Noi dung doc dung\n";
    else
        cout << "FAIL: Noi dung doc sai\n";

    // TEST 3: Doc file khong ton tai
    cout << "\nTEST 3: Doc file khong ton tai\n";

    vector<string> oldData = {"Du lieu cu"};

    bool missingResult = FileHandler::readlines(
        "file_khong_ton_tai.txt", oldData
    );

    if (!missingResult &&
        oldData.size() == 1 &&
        oldData[0] == "Du lieu cu") {
        cout << "PASS: Bao loi va giu nguyen du lieu cu\n";
    }
    else {
        cout << "FAIL: Xu ly file khong ton tai sai\n";
    }
    
    // TEST 4: Ghi va doc file rong
    cout << "\nTEST 4: File rong\n";

    vector<string> emptyData;
    vector<string> emptyResult = {"Du lieu cu"};

    bool emptyWrite = FileHandler::writeLines(
        "test_empty.txt", emptyData
    );

    bool emptyRead = FileHandler::readlines(
        "test_empty.txt", emptyResult
    );

    if (emptyWrite && emptyRead && emptyResult.empty())
        cout << "PASS: Xu ly file rong dung\n";
    else
        cout << "FAIL: Xu ly file rong sai\n";

    // TEST 5: Ghi vao thu muc khong ton tai
    cout << "\nTEST 5: Duong dan khong hop le\n";

    bool invalidWrite = FileHandler::writeLines(
        "folder_khong_ton_tai/test.txt", data
    );

    if (!invalidWrite)
        cout << "PASS: Phat hien loi ghi file\n";
    else
        cout << "FAIL: Khong phat hien loi ghi\n";

    // TEST 6: Ghi de file cu
    cout << "\nTEST 6: Ghi de file\n";

    vector<string> newData = {
        "#schema=1",
        "MOV000003|Avengers|Action|120|13|2026-11-01|2026-11-30|1"
    };

    bool overwrite = FileHandler::writeLines(path, newData);

    vector<string> checkData;
    bool checkRead = FileHandler::readlines(path, checkData);

    if (overwrite && checkRead && checkData == newData)
        cout << "PASS: Ghi de file thanh cong\n";
    else
        cout << "FAIL: Ghi de file sai\n";

    return 0;
}
