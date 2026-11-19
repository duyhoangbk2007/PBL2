#ifndef MEMBER_REPOSITORY_H
#define MEMBER_REPOSITORY_H

#include <string>
#include <unordered_map>
#include "Repository.h"
#include "../models/Member.h"

using namespace std;

class MemberRepository
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

    bool load();
    bool save();

    bool add(Member* member);
    bool update(const Member& newValues);

    Member* findById(const string& memberId);
    Member* findByPhone(const string& phone);
    Member* getAt(int index);

    int count();
    bool isLoaded();
};

#endif
