#include "MemberRepository.h"
#include "../utils/Validator.h"
#include <stdexcept>
#include <vector>
using namespace std;

// Kiem tra du lieu Member
bool MemberRepository::isValidMember(const Member* member)
{
    if (member == nullptr)
    {
        return false;
    }
    if (!Validator::isValidId(member->getMemberId(), "MEM"))
    {
        return false;
    }
    if (!Validator::isValidText(member->getFullName()))
    {
        return false;
    }
    if (!Validator::isValidPhone(member->getPhone()))
    {
        return false;
    }
    if (member->getPoints() < 0)
    {
        return false;
    }
    return true;
}

MemberRepository::MemberRepository(const string& filePath)
    : repo(filePath, getMemberId, stringToMember, memberToString)
{
}

string MemberRepository::getMemberId(Member* member)
{
    if (member == nullptr)
    {
        return "";
    }
    return member->getMemberId();
}

Member* MemberRepository::stringToMember(const string& line)
{
    vector<string> fields;
    size_t start = 0;

    // Tach cac truong du lieu theo dau |
    while (true)
    {
        size_t pos = line.find('|', start);
        if (pos == string::npos)
        {
            fields.push_back(line.substr(start));
            break;
        }
        fields.push_back(line.substr(start, pos - start));
        start = pos + 1;
    }

    if (fields.size() != 4)
    {
        throw runtime_error("Sai so luong truong Member");
    }
    if (fields[3].empty())
    {
        throw runtime_error("Points Member khong hop le");
    }
    // Chi nhan chuoi points gom chu so
    for (char c : fields[3])
    {
        if (c < '0' || c > '9')
        {
            throw runtime_error("Points Member khong hop le");
        }
    }

    int points;
    try
    {
        points = stoi(fields[3]);
    }
    catch (const exception&)
    {
        throw runtime_error("Points Member khong hop le");
    }

    Member* member = new Member(
        fields[0],
        fields[1],
        fields[2],
        points
    );
    if (!isValidMember(member))
    {
        delete member;
        throw runtime_error("Du lieu Member khong hop le");
    }
    return member;
}

// Chuyen Member thanh mot dong TXT
string MemberRepository::memberToString(Member* member)
{
    if (!isValidMember(member))
    {
        throw runtime_error("Member khong hop le khi ghi file");
    }
    return member->getMemberId() + "|" +
           member->getFullName() + "|" +
           member->getPhone() + "|" +
           to_string(member->getPoints());
}

// Doc du lieu Member tu file
bool MemberRepository::load()
{
    if (!repo.load())
    {
        return false;
    }

    try
    {
        for (int i = 0; i < repo.count(); i++)
        {
            Member* member = repo.getAt(i);
            if (findByPhone(member->getPhone()) != nullptr)
            {
                throw runtime_error("Trung phone trong file Member");
            }
            membersByPhone[member->getPhone()] = member;
        }
    }
    catch (...)
    {
        membersByPhone.clear();
        repo.clear();
        throw;
    }
    return true;
}

// Luu du lieu Member xuong file
bool MemberRepository::save()
{
    return repo.save();
}

// Them Member moi
bool MemberRepository::add(Member* member)
{
    if (!repo.isLoaded() || !isValidMember(member))
    {
        return false;
    }
    if (member->getPoints() != 0)
    {
        return false;
    }
    if (findByPhone(member->getPhone()) != nullptr)
    {
        return false;
    }

    string phone = member->getPhone();
    membersByPhone[phone] = member;
    try
    {
        if (repo.add(member))
        {
            return true;
        }
    }
    catch (...)
    {
        membersByPhone.erase(phone);
        throw;
    }
    membersByPhone.erase(phone);
    return false;
}

// Cap nhat thong tin Member
bool MemberRepository::update(const Member& newValues)
{
    if (!repo.isLoaded() || !isValidMember(&newValues))
    {
        return false;
    }

    Member* member = repo.findById(newValues.getMemberId());
    if (member == nullptr)
    {
        return false;
    }
    if (member->getPoints() != newValues.getPoints())
    {
        return false;
    }

    Member* existingMember = findByPhone(newValues.getPhone());
    if (existingMember != nullptr && existingMember != member)
    {
        return false;
    }

    string oldPhone = member->getPhone();
    string newPhone = newValues.getPhone();
    if (oldPhone == newPhone)
    {
        member->setFullName(newValues.getFullName());
        return true;
    }

    membersByPhone[newPhone] = member;
    try
    {
        member->setFullName(newValues.getFullName());
        member->setPhone(newPhone);
    }
    catch (...)
    {
        membersByPhone.erase(newPhone);
        throw;
    }
    membersByPhone.erase(oldPhone);
    return true;
}

bool MemberRepository::removeById(const string& memberId)
{
    (void)memberId;
    return false;
}

Member* MemberRepository::findById(const string& memberId)
{
    return repo.findById(memberId);
}

Member* MemberRepository::findByPhone(const string& phone)
{
    auto it = membersByPhone.find(phone);
    if (it == membersByPhone.end())
    {
        return nullptr;
    }
    return it->second;
}

Member* MemberRepository::getAt(int index)
{
    return repo.getAt(index);
}

int MemberRepository::count()
{
    return repo.count();
}

bool MemberRepository::isLoaded()
{
    return repo.isLoaded();
}
