#ifndef MEMBER_REPOSITORY_H
#define MEMBER_REPOSITORY_H

#include <string>
#include <unordered_map>
#include "IRepository.h"
#include "Repository.h"
#include "../models/Member.h"

using namespace std;

class MemberRepository : public IRepository<Member>
{
private:
    Repository<Member> repo;
    // map giúp tìm kiếm đối tượng member nhanh
    unordered_map<string, Member*> membersById;
    unordered_map<string, Member*> membersByPhone;

    static string getMemberId(Member* member);
    static Member* stringToMember(const string& line);
    static string memberToString(Member* member);
    static bool isValidMember(const Member* member);

public:
    MemberRepository(const string& filePath);

    bool load() override;
    bool save() override;

    bool add(Member* member) override;
    bool update(const Member& newValues) override;
    bool removeById(const string& memberId) override;

    Member* findById(const string& memberId) override;
    Member* findByPhone(const string& phone);
    Member* getAt(int index) override;

    int count() override;
    bool isLoaded() override;
};

#endif
