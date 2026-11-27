#ifndef STAFF_SERVICE_H
#define STAFF_SERVICE_H

#include <string>
#include "../data_access/IRepository.h"
#include "../models/User.h"
#include "AuthService.h"

class StaffService
{
private:
    IRepository<User>& userRepo;
    AuthService& authService;

public:
    StaffService(IRepository<User>& userRepo,
                 AuthService& authService);

    // Quan ly du lieu
    bool load();
    bool save();

    // Quan ly nhan vien
    bool addStaff(User* staff);
    bool updateStaff(const User& newValues);
    bool setStaffActive(const std::string& staffId, bool active);

    // Tra cuu nhan vien
    const User* findStaffById(const std::string& staffId);
    const User* findStaffByUsername(const std::string& username);
    const User* getStaffAt(int index);
    int getStaffCount();
};

#endif
