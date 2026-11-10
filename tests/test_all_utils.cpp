// PBL2 - One-file test runner for core/utils (C++17).
// Run from the repository root using the compile instructions below.
// Does NOT modify core code or real data: changes working directory to an isolated folder.
#include "core/utils/FileHandler.h"
#include "core/utils/DateTime.h"
#include "core/utils/IdGenerator.h"
#include "core/utils/TransactionManager.h"

#include <chrono>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace fs = std::filesystem;
using Lines = std::vector<std::string>;

struct Workspace {
    fs::path oldDir;
    fs::path dir;
    explicit Workspace() : oldDir(fs::current_path()) {
        const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
        dir = fs::temp_directory_path() / ("pbl2_utils_tests_" + std::to_string(stamp));
        fs::create_directories(dir);
        fs::current_path(dir);
    }
    ~Workspace() {
        try {
            fs::current_path(oldDir);
            std::error_code ec;
            fs::remove_all(dir, ec);
        } catch (...) {}
    }
};

void require(bool condition, const std::string& message) {
    if (!condition) throw std::runtime_error(message);
}
void expectThrows(const std::function<void()>& fn) {
    bool thrown = false;
    try { fn(); } catch (const std::exception&) { thrown = true; }
    require(thrown, "Expected exception, but call succeeded");
}
Lines read(const fs::path& file) {
    Lines lines;
    require(FileHandler::readlines(file.string(), lines), "Cannot read " + file.string());
    return lines;
}
void write(const fs::path& file, const Lines& lines) {
    require(FileHandler::writeLines(file.string(), lines), "Cannot write " + file.string());
}
void testFileHandler() {
    fs::create_directories("files");
    const Lines expected = {"alpha", "MOV|1", ""};
    write("files/sample.txt", expected);
    require(read("files/sample.txt") == expected, "Read/write mismatch");
    Lines preserved = {"keep"};
    require(!FileHandler::readlines("files/missing.txt", preserved), "Missing file unexpectedly readable");
    require(preserved == Lines{"keep"}, "Failed read modified output");
    require(!FileHandler::writeLines("missing_parent/new.txt", {"x"}), "Write into missing folder succeeded");
    write("files/sample.txt", {});
    require(read("files/sample.txt").empty(), "Empty write mismatch");
}
void testDateTime() {
    const std::string value = DateTime::getCurrentDateTime();
    require(value.size() == 19, "Timestamp length is not 19");
    for (std::size_t i = 0; i < value.size(); ++i) {
        if (i == 4 || i == 7) require(value[i] == '-', "Invalid date separator");
        else if (i == 10) require(value[i] == 'T', "Missing T separator");
        else if (i == 13 || i == 16) require(value[i] == ':', "Invalid time separator");
        else require(value[i] >= '0' && value[i] <= '9', "Non-digit in timestamp");
    }
    const int month = std::stoi(value.substr(5,2));
    const int day = std::stoi(value.substr(8,2));
    const int hour = std::stoi(value.substr(11,2));
    const int minute = std::stoi(value.substr(14,2));
    const int second = std::stoi(value.substr(17,2));
    require(month >= 1 && month <= 12 && day >= 1 && day <= 31, "Invalid calendar fields");
    require(hour <= 23 && minute <= 59 && second <= 59, "Invalid time fields");
}
void testIdGenerator() {
    fs::create_directories("ids");
    const Lines initial = {"USR|1", "MOV|12", "ROM|3", "SHW|4", "TKT|5", "INV|6", "MEM|7"};
    write("ids/sequences.txt", initial); // NO #schema=1, per current project decision.
    std::string id, updated;
    IdGenerator::generateNextId("MOV", "ids/sequences.txt", id, updated);
    require(id == "MOV0012", "Wrong ID for MOV|12 (current implementation uses 4 digits)");
    require(updated.find("MOV|13\n") != std::string::npos, "Counter was not incremented");
    require(updated.find("#schema") == std::string::npos, "Unexpected schema in output");
    require(read("ids/sequences.txt") == initial, "Generator unexpectedly wrote input file");

    IdGenerator::generateNextId("USR", "ids/sequences.txt", id, updated);
    require(id == "USR0001", "Wrong ID for USR|1");
    auto invalid = [&](const Lines& content, const std::string& entity = "MOV") {
        write("ids/invalid.txt", content);
        expectThrows([&] { IdGenerator::generateNextId(entity, "ids/invalid.txt", id, updated); });
        require(id.empty() && updated.empty(), "Outputs were not cleared after failure");
    };
    invalid({"MOV|1", "MOV|2"});
    invalid({"WRONG|1", "MOV|2"});
    invalid({"MOV|abc"});
    invalid({"MOV|"});
    invalid({"MOV|0"});
    invalid({"MOV|10001"});
    invalid({"USR|1"});
    invalid({"MOV 1"});
    invalid({"MOV|10000"});
    expectThrows([&] { IdGenerator::generateNextId("BAD", "ids/sequences.txt", id, updated); });
    expectThrows([&] { IdGenerator::generateNextId("MOV", "ids/missing.txt", id, updated); });
}
void makeRecoveryTxn(const fs::path& directory, const std::string& status,
                     const fs::path& original, const Lines& backup, bool existed) {
    fs::create_directories(directory / "backup");
    if (existed) write(directory / "backup" / original.filename(), backup);
    write(directory / "manifest.txt", {status, "1", original.string() + (existed ? "|1" : "|0")});
}
void testTransactionManager() {
    fs::create_directories("records");
    write("records/a.txt", {"old A"});
    write("records/b.txt", {"old B"});
    const std::vector<TransactionFile> updates = {
        {"records/a.txt", {"new A"}}, {"records/b.txt", {"new B"}}
    };
#ifdef TEST_ROLLBACK
    expectThrows([&] { TransactionManager::execute(updates); });
    require(read("records/a.txt") == Lines{"old A"}, "Rollback did not restore A");
    require(read("records/b.txt") == Lines{"old B"}, "Rollback did not preserve B");
#else
    TransactionManager::execute(updates);
    require(read("records/a.txt") == Lines{"new A"}, "Commit did not update A");
    require(read("records/b.txt") == Lines{"new B"}, "Commit did not update B");
    require(fs::is_empty("data/transactions"), "Committed transaction directory not cleaned");
    expectThrows([&] { TransactionManager::execute({}); });
    // Simulate interrupted PREPARED transaction (the original file was partially replaced).
    write("records/a.txt", {"partial update"});
    makeRecoveryTxn("recovery/txn_prepared", "PREPARED", "records/a.txt", {"new A"}, true);
    TransactionManager::recover("recovery");
    require(read("records/a.txt") == Lines{"new A"}, "Recovery did not restore backup");
    require(fs::is_empty("recovery"), "PREPARED transaction not cleaned");
    // Simulate COMMITTED: preserve the new file, clean its transaction folder.
    write("records/a.txt", {"committed A"});
    makeRecoveryTxn("recovery/txn_committed", "COMMITTED", "records/a.txt", {"new A"}, true);
    TransactionManager::recover("recovery");
    require(read("records/a.txt") == Lines{"committed A"}, "COMMITTED transaction was rolled back");
    require(fs::is_empty("recovery"), "COMMITTED transaction not cleaned");
    // An interrupted PREPARED transaction creating a NEW file should remove it.
    write("records/created.txt", {"partial new"});
    makeRecoveryTxn("recovery/txn_new", "PREPARED", "records/created.txt", {}, false);
    TransactionManager::recover("recovery");
    require(!fs::exists("records/created.txt"), "New file not removed during rollback");
    TransactionManager::recover("nonexistent_recovery_dir");
    fs::create_directories("recovery/txn_missing_manifest");
    expectThrows([&] { TransactionManager::recover("recovery"); });
#endif
}

int main() {
    int passed = 0, failed = 0;
    try {
        Workspace sandbox;
        const std::vector<std::pair<std::string, std::function<void()>>> tests = {
            {"FileHandler", testFileHandler},
            {"DateTime", testDateTime},
            {"IdGenerator", testIdGenerator},
            {"TransactionManager", testTransactionManager}
        };
        std::cout << "Isolated test folder: " << sandbox.dir << '\n';
#ifdef TEST_ROLLBACK
        std::cout << "MODE: TEST_ROLLBACK (forced failure after first file replacement)\n";
#else
        std::cout << "MODE: normal\n";
#endif
        for (const auto& item : tests) {
            try { item.second(); ++passed; std::cout << "[PASS] " << item.first << '\n'; }
            catch (const std::exception& e) { ++failed; std::cout << "[FAIL] " << item.first << ": " << e.what() << '\n'; }
        }
    } catch (const std::exception& e) {
        std::cerr << "Cannot initialize sandbox: " << e.what() << '\n';
        return 2;
    }
    std::cout << "RESULT: " << passed << " passed, " << failed << " failed\n";
    return failed == 0 ? 0 : 1;
}
