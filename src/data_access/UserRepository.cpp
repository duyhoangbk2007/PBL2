#include "UserRepository.h"

#include <memory>
#include <stdexcept>
#include <vector>

using namespace std;

namespace {
bool isValidText(const string& text)
{
    if (text.empty()) return false;
    for (unsigned char c : text) {
        if (c == '|' || c < 32 || c == 127) return false;
    }
    return true;
}

bool isValidUserId(const string& id)
{
    if (id.size() != 7 || id.substr(0, 3) != "USR") return false;
    for (size_t i = 3; i < id.size(); ++i) {
        if (id[i] < '0' || id[i] > '9') return false;
    }
    return id.substr(3) != "0000";
}
}

UserRepository::UserRepository(const string& filePath)
    : filePath(filePath), repo(filePath, getUserId, stringToUser, userToString)
{
}

string UserRepository::getUserId(User* user)
{
    return user->getUserId();
}

bool UserRepository::isValidUser(const User* user)
{
    if (user == nullptr || !isValidUserId(user->getUserId()) ||
        !isValidText(user->getUsername()) || !isValidText(user->getPassword()) ||
        !isValidText(user->getFullName())) {
        return false;
    }
    const string& username = user->getUsername();
    if (username.front() == ' ' || username.back() == ' ') return false;
    return user->getRole() == UserRole::ADMIN || user->getRole() == UserRole::STAFF;
}

User* UserRepository::stringToUser(const string& line)
{
    vector<string> fields;
    size_t start = 0;
    while (true) {
        size_t end = line.find('|', start);
        fields.push_back(line.substr(start, end == string::npos ? end : end - start));
        if (end == string::npos) break;
        start = end + 1;
    }

    // Tra nullptr de Repository<User>::load bao so dong loi.
    if (fields.size() != 6 || (fields[4] != "ADMIN" && fields[4] != "STAFF") ||
        (fields[5] != "0" && fields[5] != "1")) {
        return nullptr;
    }
    UserRole role = fields[4] == "ADMIN" ? UserRole::ADMIN : UserRole::STAFF;
    unique_ptr<User> user(new User(fields[0], fields[1], fields[2], fields[3],
                                  role, fields[5] == "1"));
    if (!isValidUser(user.get())) return nullptr;
    return user.release();
}

string UserRepository::userToString(User* user)
{
    if (!isValidUser(user)) throw runtime_error("Du lieu User khong hop le");
    return user->getUserId() + "|" + user->getUsername() + "|" +
           user->getPassword() + "|" + user->getFullName() + "|" +
           (user->getRole() == UserRole::ADMIN ? "ADMIN" : "STAFF") + "|" +
           (user->isActive() ? "1" : "0");
}

void UserRepository::load()
{
    // Nap lai se huy User cu, lam hong con tro tu Invoice/AuthService.
    if (repo.isLoaded()) throw runtime_error(filePath + ": UserRepository da load");
    try {
        if (!repo.load()) throw runtime_error("Khong doc duoc file");
        const int size = repo.count();
        for (int i = 0; i < size; ++i) {
            User* user = repo.getAt(i);
            usersById.emplace(user->getUserId(), user);
            if (!usersByUsername.emplace(user->getUsername(), user).second) {
                throw runtime_error("Trung username tai dong " + to_string(i + 1));
            }
        }
    }
    catch (const exception& e) {
        usersById.clear();
        usersByUsername.clear();
        repo.clear();
        // Khong dua noi dung ban ghi (co mat khau) vao thong bao loi.
        throw runtime_error(filePath + ": " + e.what());
    }
}

void UserRepository::save()
{
    if (!repo.isLoaded()) throw runtime_error(filePath + ": UserRepository chua load");
    try {
        if (!repo.save()) throw runtime_error("Khong ghi duoc file");
    }
    catch (const exception& e) {
        throw runtime_error(filePath + ": " + e.what());
    }
}

bool UserRepository::isLoaded() { return repo.isLoaded(); }
int UserRepository::count() { return repo.count(); }
const User* UserRepository::getAt(int index) { return repo.getAt(index); }

const User* UserRepository::findById(const string& userId) const
{
    auto found = usersById.find(userId);
    return found == usersById.end() ? nullptr : found->second;
}

const User* UserRepository::findByUsername(const string& username) const
{
    auto found = usersByUsername.find(username);
    return found == usersByUsername.end() ? nullptr : found->second;
}

bool UserRepository::add(User* user)
{
    if (!repo.isLoaded() || !isValidUser(user) ||
        findById(user->getUserId()) != nullptr ||
        findByUsername(user->getUsername()) != nullptr) {
        return false;
    }
    // Tao chi muc truoc khi chuyen quyen so huu cho repo.
    try {
        usersById.emplace(user->getUserId(), user);
        usersByUsername.emplace(user->getUsername(), user);
        if (repo.add(user)) return true;
    }
    catch (...) {
        usersById.erase(user->getUserId());
        usersByUsername.erase(user->getUsername());
        throw;
    }
    usersById.erase(user->getUserId());
    usersByUsername.erase(user->getUsername());
    return false;
}

bool UserRepository::update(const User& newValues)
{
    if (!repo.isLoaded() || !isValidUser(&newValues)) return false;
    auto found = usersById.find(newValues.getUserId());
    if (found == usersById.end()) return false;
    User* user = found->second;
    if (user->getRole() != newValues.getRole()) return false;
    const User* sameUsername = findByUsername(newValues.getUsername());
    if (sameUsername != nullptr && sameUsername != user) return false;

    const string oldUsername = user->getUsername();
    const string newUsername = newValues.getUsername();
    const bool renamed = oldUsername != newUsername;
    if (renamed) usersByUsername.emplace(newUsername, user);
    try {
        // Goi setter de giu nguyen danh sach Invoice, khong gan ca User.
        user->setPassword(newValues.getPassword());
        user->setFullName(newValues.getFullName());
        user->setUsername(newUsername);
    }
    catch (...) {
        if (renamed) usersByUsername.erase(newUsername);
        throw;
    }
    user->setActive(newValues.isActive());
    if (renamed) usersByUsername.erase(oldUsername);
    return true;
}
