#ifndef MEMBER_SERVICE_H
#define MEMBER_SERVICE_H

#include <string>
#include "../data_access/MemberRepository.h"
#include "AuthService.h"

using namespace std;

class MemberService
{
private:
    MemberRepository& memberRepo;
    AuthService& authService;

public:
    MemberService(MemberRepository& memberRepo, AuthService& authService);

    bool load();
    bool save();

    bool addMember(Member* member);
    bool updateMember(const Member& newValues);

    const Member* findMemberById(const string& memberId);
    const Member* findMemberByPhone(const string& phone);
    const Member* getMemberAt(int index);
    int getMemberCount();
};

#endif