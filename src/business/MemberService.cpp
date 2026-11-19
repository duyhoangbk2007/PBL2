#include "MemberService.h"
using namespace std;

MemberService::MemberService(MemberRepository& memberRepo, AuthService& authService)
    : memberRepo(memberRepo),
      authService(authService)
{
}

bool MemberService::load()
{
    return memberRepo.load();
}

bool MemberService::save()
{
    return memberRepo.save();
}

bool MemberService::addMember(Member* member)
{
    authService.requirePermission("MEMBER_ADD");
    return memberRepo.add(member);
}

bool MemberService::updateMember(const Member& newValues)
{
    authService.requirePermission("MEMBER_UPDATE");
    return memberRepo.update(newValues);
}

const Member* MemberService::findMemberById(const string& memberId)
{
    authService.requirePermission("MEMBER_VIEW");
    return memberRepo.findById(memberId);
}

const Member* MemberService::findMemberByPhone(const string& phone)
{
    authService.requirePermission("MEMBER_VIEW");
    return memberRepo.findByPhone(phone);
}

const Member* MemberService::getMemberAt(int index)
{
    authService.requirePermission("MEMBER_VIEW");
    return memberRepo.getAt(index);
}

int MemberService::getMemberCount()
{
    authService.requirePermission("MEMBER_VIEW");
    return memberRepo.count();
}