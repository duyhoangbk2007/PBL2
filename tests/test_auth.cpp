// B05 integration tests: real AuthService -> UserRepository -> isolated TXT.
// Build/run commands and scope: docs/development/Buoc05_DangNhapVaPhanQuyen.md.
#include "src/business/AuthService.h"
#include "src/data_access/UserRepository.h"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace fs = std::filesystem;
using Lines = std::vector<std::string>;

void require(bool condition, const std::string& message) {
    if (!condition) throw std::runtime_error(message);
}

template <typename Action>
std::string expectError(Action action) {
    try { action(); }
    catch (const std::runtime_error& error) { return error.what(); }
    throw std::runtime_error("Expected runtime_error, but operation succeeded");
}

struct Workspace {
    const fs::path parent = fs::canonical(fs::temp_directory_path());
    fs::path dir;

    Workspace() {
        const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
        for (int attempt = 0; attempt < 100; ++attempt) {
            dir = parent / ("pbl2_auth_tests_" + std::to_string(stamp) + "_" +
                            std::to_string(attempt));
            if (fs::create_directory(dir)) return;
        }
        throw std::runtime_error("Cannot create isolated workspace");
    }

    ~Workspace() {
        // Only remove the directory this runner created, directly inside TEMP.
        std::error_code error;
        const auto resolved = fs::weakly_canonical(dir, error);
        if (!error && resolved.parent_path() == parent &&
            resolved.filename().string().find("pbl2_auth_tests_") == 0) {
            fs::remove_all(resolved, error);
        }
        if (error) std::cerr << "Cannot clean test folder: " << error.message() << '\n';
    }
};

void writeLines(const fs::path& path, const Lines& lines) {
    std::ofstream file(path, std::ios::binary);
    for (const auto& line : lines) file << line << '\n';
    file.close();
    require(!file.fail(), "Cannot write test fixture");
}

Lines readLines(const fs::path& path) {
    std::ifstream file(path);
    require(file.is_open(), "Cannot read test fixture");
    Lines lines;
    std::string line;
    while (std::getline(file, line)) lines.push_back(line);
    require(!file.bad(), "Cannot finish reading fixture");
    return lines;
}

const Lines accounts = {
    "USR0001|admin|AdminPass|Admin User|ADMIN|1",
    "USR0002|kiet|StaffPass|Kiet User|STAFF|1",
    "USR0003|locked|LockedPass|Locked Staff|STAFF|0",
    "USR0004|lockedAdmin|LockedPass|Locked Admin|ADMIN|0"
};

struct Fixture {
    Workspace workspace;
    fs::path path = workspace.dir / "users.txt";
    UserRepository users{path.string()};
    AuthService auth{users};

    Fixture() {
        writeLines(path, accounts);
        users.load();
    }
};

void requireAnonymous(AuthService& auth) {
    require(!auth.isLoggedIn() && !auth.isAdmin() && !auth.isStaff(),
            "Anonymous/inactive session retained permissions");
    require(auth.getCurrentUser() == nullptr, "Anonymous session returned a user");
    expectError([&] { auth.requireLogin(); });
    expectError([&] { auth.requireAdmin(); });
}

void testUnloadedAndMissingFile() {
    Workspace workspace;
    const auto path = workspace.dir / "missing.txt";
    UserRepository users(path.string());
    AuthService auth(users);
    requireAnonymous(auth);
    expectError([&] { auth.login("admin", "AdminPass"); });
    expectError([&] { users.save(); });
    const auto message = expectError([&] { users.load(); });
    require(message.find(path.string()) != std::string::npos, "Missing file context");
    require(!fs::exists(path), "Missing file was silently created");
}

void testAdmin() {
    Fixture f;
    requireAnonymous(f.auth);
    require(f.auth.login("admin", "AdminPass"), "Admin login rejected");
    require(f.auth.isAdmin() && !f.auth.isStaff(), "Wrong Admin role");
    require(f.auth.requireAdmin().getUserId() == "USR0001", "Wrong Admin ID");
    require(f.auth.requireLogin().getUserId() == "USR0001", "Admin cannot use shared guard");
    require(f.auth.getCurrentUser() == f.users.findById("USR0001"), "Wrong current user");
    require(readLines(f.path) == accounts, "Login modified account file");
}

void testStaff() {
    Fixture f;
    require(f.auth.login("kiet", "StaffPass"), "Staff login rejected");
    require(f.auth.isStaff() && !f.auth.isAdmin(), "Wrong Staff role");
    require(f.auth.requireLogin().getUserId() == "USR0002", "Wrong Staff ID");
    expectError([&] { f.auth.requireAdmin(); });
    require(f.auth.isLoggedIn(), "Denied Admin action destroyed valid Staff session");
}

void testInvalidCredentialsClearOldSession() {
    Fixture f;
    const std::vector<std::pair<std::string, std::string>> invalid = {
        {"missing", "AdminPass"}, {"admin", "wrong"}, {"", "AdminPass"},
        {"admin", ""}, {"", ""}, {"Admin", "AdminPass"},
        {" admin", "AdminPass"}, {"admin ", "AdminPass"},
        {"admin", "adminpass"}, {"admin", "AdminPass "}
    };
    for (const auto& credentials : invalid) {
        require(f.auth.login("admin", "AdminPass"), "Cannot seed old Admin session");
        require(!f.auth.login(credentials.first, credentials.second),
                "Invalid credentials accepted");
        requireAnonymous(f.auth);
    }
    require(readLines(f.path) == accounts, "Failed logins modified account file");
}

void testLogoutAndSwitchUser() {
    Fixture f;
    require(f.auth.login("admin", "AdminPass"), "Admin login failed");
    require(f.auth.login("kiet", "StaffPass"), "Switch to Staff failed");
    require(f.auth.requireLogin().getUserId() == "USR0002", "Old identity retained");
    expectError([&] { f.auth.requireAdmin(); });
    f.auth.logout();
    requireAnonymous(f.auth);
    f.auth.logout();
    requireAnonymous(f.auth);
}

void testInactiveAccounts() {
    Fixture f;
    for (const std::string username : {"locked", "lockedAdmin"}) {
        require(f.users.findByUsername(username) != nullptr, "Inactive history missing");
        require(f.auth.login("admin", "AdminPass"), "Cannot seed Admin session");
        require(!f.auth.login(username, "LockedPass"), "Inactive account logged in");
        requireAnonymous(f.auth);
    }
}

void testDisableDuringSession() {
    for (const UserRole role : {UserRole::ADMIN, UserRole::STAFF}) {
        Fixture f;
        const bool admin = role == UserRole::ADMIN;
        const std::string username = admin ? "admin" : "kiet";
        const std::string password = admin ? "AdminPass" : "StaffPass";
        const std::string id = admin ? "USR0001" : "USR0002";
        require(f.auth.login(username, password), "Initial login failed");
        User disabled(id, username, password, "Disabled User", role, false);
        require(f.users.update(disabled), "Cannot disable account");
        requireAnonymous(f.auth);
        require(!f.auth.login(username, password), "Disabled account logged back in");
        f.users.save();
        UserRepository reloaded(f.path.string());
        reloaded.load();
        AuthService auth(reloaded);
        require(!auth.login(username, password), "Disabled status lost after save/load");
        requireAnonymous(auth);
    }
}

void testMalformedRecords() {
    const Lines badRecords = {
        "USR0005|other|pw|Name|STAFF", "USR0005|other|pw|Name|STAFF|1|extra",
        "USR0000|other|pw|Name|STAFF|1", "MOV0005|other|pw|Name|STAFF|1",
        "USR005|other|pw|Name|STAFF|1", "USR0005|other|pw|Name|OWNER|1",
        "USR0005|other|pw|Name|STAFF|true", "USR0005|other|pw|Name|STAFF|",
        "USR0005||pw|Name|STAFF|1", "USR0005|other||Name|STAFF|1",
        "USR0005|other|pw||STAFF|1", "USR0005| other|pw|Name|STAFF|1",
        "USR0005|other |pw|Name|STAFF|1", "USR0005|other|pw|Bad\tName|STAFF|1",
        "#schema=1", ""
    };
    for (const auto& record : badRecords) {
        Workspace workspace;
        const auto path = workspace.dir / "invalid.txt";
        const Lines input = {accounts[0], record};
        writeLines(path, input);
        UserRepository users(path.string());
        const auto message = expectError([&] { users.load(); });
        require(message.find(path.string()) != std::string::npos &&
                message.find("dong 2") != std::string::npos, "Missing file/line error");
        require(!users.isLoaded() && users.count() == 0, "Partial load was retained");
        require(users.findById("USR0001") == nullptr &&
                users.findByUsername("admin") == nullptr, "Stale index after failed load");
        expectError([&] { users.save(); });
        require(readLines(path) == input, "Invalid input was overwritten");
    }
}

void testDuplicateKeysAndRetry() {
    for (const std::string duplicate : {
             "USR0001|other|SecretValue|Other|STAFF|1",
             "USR0005|admin|SecretValue|Other|STAFF|1"}) {
        Workspace workspace;
        const auto path = workspace.dir / "users.txt";
        writeLines(path, {accounts[0], duplicate});
        UserRepository users(path.string());
        const auto message = expectError([&] { users.load(); });
        require(message.find(path.string()) != std::string::npos &&
                message.find("dong 2") != std::string::npos, "Duplicate lacks context");
        require(message.find("SecretValue") == std::string::npos, "Password in error");
        require(!users.isLoaded() && users.count() == 0, "Duplicate load partly accepted");
        writeLines(path, accounts);
        users.load();
        AuthService auth(users);
        require(auth.login("admin", "AdminPass"), "Retry after repair failed");
    }
}

void testEmptyFile() {
    Workspace workspace;
    const auto path = workspace.dir / "empty.txt";
    writeLines(path, {});
    UserRepository users(path.string());
    users.load();
    require(users.isLoaded() && users.count() == 0, "Empty file rejected");
    require(users.getAt(-1) == nullptr && users.getAt(0) == nullptr, "Invalid index accepted");
    AuthService auth(users);
    require(!auth.login("admin", "AdminPass"), "Empty repository authenticated user");
    requireAnonymous(auth);
    users.save();
    require(readLines(path).empty(), "Empty save added schema/header");
}

void testAddUpdateAndRoundTrip() {
    Fixture f;
    auto added = std::make_unique<User>("USR0005", "Kiet", " new pass ",
                                        u8"Nguyễn Kiệt", UserRole::STAFF, true);
    require(f.users.add(added.get()), "Valid distinct case username rejected");
    added.release(); // Ownership transferred only after successful add.
    require(f.auth.login("Kiet", " new pass "), "Exact password with spaces failed");
    const User* original = f.auth.getCurrentUser();
    User renamed("USR0005", "newKiet", "ChangedPass", u8"Nguyễn Kiệt",
                 UserRole::STAFF, true);
    require(f.users.update(renamed), "Valid update rejected");
    require(f.auth.getCurrentUser() == original, "Update replaced user address");
    require(f.users.findByUsername("Kiet") == nullptr, "Old username index retained");
    require(f.users.findByUsername("newKiet") == original, "New username index missing");
    require(!f.auth.login("newKiet", " new pass "), "Old password accepted");
    require(f.auth.login("newKiet", "ChangedPass"), "Updated credentials rejected");
    require(readLines(f.path) == accounts, "Mutation saved without explicit save");
    f.users.save();
    Lines expected = accounts;
    expected.push_back(u8"USR0005|newKiet|ChangedPass|Nguyễn Kiệt|STAFF|1");
    require(readLines(f.path) == expected, "Unexpected TXT round-trip contents");
    UserRepository reloaded(f.path.string());
    reloaded.load();
    require(reloaded.count() == 5, "Accounts lost during save/load");
    AuthService auth(reloaded);
    require(auth.login("newKiet", "ChangedPass"), "Saved credentials rejected");
    require(auth.requireLogin().getFullName() == u8"Nguyễn Kiệt", "UTF-8 name corrupted");
    require(auth.login("admin", "AdminPass") && auth.isAdmin(), "Admin lost on save");
}

void testRejectedMutations() {
    Fixture f;
    auto duplicateId = std::make_unique<User>("USR0001", "other", "pw", "Name", UserRole::STAFF, true);
    auto duplicateName = std::make_unique<User>("USR0005", "admin", "pw", "Name", UserRole::STAFF, true);
    auto invalid = std::make_unique<User>("USR0005", "other", "bad|pw", "Name", UserRole::STAFF, true);
    require(!f.users.add(nullptr) && !f.users.add(duplicateId.get()) &&
            !f.users.add(duplicateName.get()) && !f.users.add(invalid.get()), "Invalid add accepted");
    User renameConflict("USR0002", "admin", "pw", "Name", UserRole::STAFF, true);
    User roleChange("USR0002", "kiet", "pw", "Name", UserRole::ADMIN, true);
    User missing("USR9999", "other", "pw", "Name", UserRole::STAFF, true);
    require(!f.users.update(renameConflict) && !f.users.update(roleChange) &&
            !f.users.update(missing), "Invalid update accepted");
    require(f.auth.login("kiet", "StaffPass") && f.auth.isStaff(), "Rejected update changed account");
    f.users.save();
    require(readLines(f.path) == accounts, "Rejected mutations changed stored data");
}

void testReloadRejectedPreservesSession() {
    Fixture f;
    require(f.auth.login("admin", "AdminPass"), "Initial login failed");
    const User* original = f.auth.getCurrentUser();
    expectError([&] { f.users.load(); });
    require(f.auth.getCurrentUser() == original && f.auth.isAdmin(), "Reload broke session");
    require(f.users.count() == 4, "Reload changed records");
}

void testWriteFailureReported() {
    Fixture f;
    // Replace only our temporary fixture file with a directory to force I/O failure.
    require(fs::remove(f.path), "Cannot remove temporary fixture");
    require(fs::create_directory(f.path), "Cannot prepare write failure");
    const auto message = expectError([&] { f.users.save(); });
    require(message.find(f.path.string()) != std::string::npos, "Write error lacks path");
    require(f.users.count() == 4, "Write failure lost in-memory accounts");
}

int main() {
    const std::vector<std::pair<std::string, std::function<void()>>> tests = {
        {"Unloaded repository and missing file", testUnloadedAndMissingFile},
        {"Admin login and permissions", testAdmin},
        {"Staff login and permissions", testStaff},
        {"Invalid credentials clear previous session", testInvalidCredentialsClearOldSession},
        {"Logout and switch user", testLogoutAndSwitchUser},
        {"Inactive Admin and Staff", testInactiveAccounts},
        {"Disable account during session", testDisableDuringSession},
        {"Malformed records block loading and saving", testMalformedRecords},
        {"Duplicate keys and retry after repair", testDuplicateKeysAndRetry},
        {"Empty account file", testEmptyFile},
        {"Add/update and UTF-8 save/load", testAddUpdateAndRoundTrip},
        {"Rejected mutations preserve accounts", testRejectedMutations},
        {"Reload rejection preserves session", testReloadRejectedPreservesSession},
        {"Write failure is reported", testWriteFailureReported}
    };
    int passed = 0;
    int failed = 0;
    for (const auto& test : tests) {
        try {
            test.second();
            ++passed;
            std::cout << "[PASS] " << test.first << '\n';
        }
        catch (const std::exception& error) {
            ++failed;
            std::cout << "[FAIL] " << test.first << ": " << error.what() << '\n';
        }
    }
    std::cout << "RESULT: " << passed << " passed, " << failed << " failed\n";
    return failed == 0 ? 0 : 1;
}
