#include "MemberService.h"
#include "../utils/IdGenerator.h"
#include <vector>
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

    if (!memberRepo.isLoaded() || member == nullptr)
    {
        return false;
    }

    vector<string> existingIds;

    for (int i = 0; i < memberRepo.count(); i++)
    {
        Member* current = memberRepo.getAt(i);
        if (current == nullptr)
        {
            return false;
        }
        existingIds.push_back(current->getMemberId());
    }
    string memberId = IdGenerator::generateNextId("MEM", existingIds);
    Member* newMember = new Member(memberId,
                                   member->getFullName(),
                                   member->getPhone(),
                                   member->getPoints());

    try
    {
        if (!memberRepo.add(newMember))
        {
            delete newMember;
            return false;
        }
    }
    catch (...)
    {
        delete newMember;
        throw;
    }
    return true;
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