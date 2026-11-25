#ifndef MEMBER_REPOSITORY_H
#define MEMBER_REPOSITORY_H

#include <string>
#include <unordered_map>
#include "IRepository.h"
#include "Repository.h"
#include "../models/Member.h"

class MemberRepository : public IRepository<Member>
{
private:
    Repository<Member> repo;
    std::unordered_map<std::string, Member*> membersByPhone;

    static std::string getMemberId(Member* member);
    static Member* stringToMember(const std::string& line);
    static std::string memberToString(Member* member);

    static bool isValidMember(const Member* member);

public:
    MemberRepository(const std::string& filePath);

    // Doc va ghi du lieu
    bool load() override;
    bool save() override;

    // Quan ly du lieu
    bool add(Member* member) override;
    bool update(const Member& newValues) override;
    bool removeById(const std::string& memberId) override;

    // Truy van du lieu
    Member* findById(const std::string& memberId) override;
    Member* findByPhone(const std::string& phone);
    Member* getAt(int index) override;
    int count() override;
    bool isLoaded() override;
};

#endif
