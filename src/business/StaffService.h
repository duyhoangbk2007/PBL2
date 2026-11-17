#ifndef STAFF_SERVICE_H
#define STAFF_SERVICE_H

#include <string>
#include "../data_access/UserRepository.h"
#include "AuthService.h"

class StaffService
{
private:
    UserRepository& staffRepo;
    AuthService& authService;

public:
    StaffService(UserRepository& staffRepo, AuthService& authService);

    bool load();
    bool save();

    // Quan ly nhan vien
    bool addStaff(User* staff);
    bool updateStaff(const User& newValues);
    bool deactivateStaff(const std::string& staffId);

    // Tra cuu nhan vien
    const User* findStaffById(const std::string& staffId);
    const User* findStaffByUsername(const std::string& username);
    const User* getStaffAt(int index);
    int getStaffCount();
};

#endif
