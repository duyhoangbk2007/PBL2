#ifndef MEMBER_SERVICE_H
#define MEMBER_SERVICE_H

#include <string>
#include "../data_access/IRepository.h"
#include "../models/Member.h"
#include "AuthService.h"

class MemberService
{
private:
    IRepository<Member>& memberRepo;
    AuthService& authService;

public:
    MemberService(IRepository<Member>& memberRepo,
                  AuthService& authService);

    // Quan ly du lieu
    bool load();
    bool save();

    // Quan ly thanh vien
    bool addMember(Member* member);
    bool updateMember(const Member& newValues);

    // Tra cuu thanh vien
    const Member* findMemberById(const std::string& memberId);
    const Member* findMemberByPhone(const std::string& phone);
    const Member* getMemberAt(int index);
    int getMemberCount();
};

#endif
