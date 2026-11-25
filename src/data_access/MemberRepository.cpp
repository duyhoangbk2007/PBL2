#include "MemberRepository.h"
#include "../utils/Validator.h"
#include <stdexcept>
#include <vector>
#include <string>
using namespace std;


static bool isValidPhone(const string& phone)
{
    if (phone.size() != 10 || phone[0] != '0')
    {
        return false;
    }
    for (char c : phone)
    {
        if (c < '0' || c > '9')
        {
            return false;
        }
    }
    return true;
}

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
    if (!isValidPhone(member->getPhone()))
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
    // kiểm tra points chỉ gồm chữ số
    for (char c : fields[3])
    {
        if (c < '0' || c > '9')
        {
            throw runtime_error("Points Member khong hop le");
        }
    }
    int points;
    //chuyển chuỗi points thành số nguyên
    try
    {
        size_t pos = 0;
        points = stoi(fields[3], &pos);
        if (pos != fields[3].size())
        {
            throw runtime_error("Points Member khong hop le");
        }
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

bool MemberRepository::load()
{
    if (repo.isLoaded())
    {
        throw runtime_error("Du lieu Member da duoc nap: " + repo.getFilePath());
    }
    if (!repo.load())
    {
        throw runtime_error("Khong the nap file Member: " + repo.getFilePath());
    }
    // xây dựng 2 map
    membersById.clear();
    membersByPhone.clear();
    try
    {
        for (int i = 0; i < repo.count(); i++)
        {
            Member* member = repo.getAt(i);
            if (membersById.find(member->getMemberId()) != membersById.end() ||
                membersByPhone.find(member->getPhone()) != membersByPhone.end())
            {
                throw runtime_error(
                    "Trung ID hoac phone trong file " +
                    repo.getFilePath() +
                    " tai dong " + to_string(i + 1)
                );
            }
            membersById[member->getMemberId()] = member;
            membersByPhone[member->getPhone()] = member;
        }
    }
    catch (...)
    {
        membersById.clear();
        membersByPhone.clear();
        repo.clear();
        throw;
    }
    return true;
}

bool MemberRepository::save()
{
    if (!repo.isLoaded())
    {
        throw runtime_error("Chua nap du lieu Member");
    }
    if (!repo.save())
    {
        throw runtime_error("Khong the ghi file Member: " + repo.getFilePath());
    }
    return true;
}

Member* MemberRepository::findById(const string& memberId)
{
    auto it = membersById.find(memberId);
    if (it == membersById.end())
    {
        return nullptr;
    }
    return it->second;
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
    if (findById(member->getMemberId()) != nullptr)
    {
        return false;
    }
    if (findByPhone(member->getPhone()) != nullptr)
    {
        return false;
    }
    // kiểm tra nếu có thành viên rồi thì không thêm nữa
    for (int i = 0; i < repo.count(); i++)
    {
        if (repo.getAt(i) == member)
        {
            return false;
        }
    }
    string memberId = member->getMemberId();
    string phone = member->getPhone();
    // Thêm vào 2 map
    membersById[memberId] = member;
    try
    {
        membersByPhone[phone] = member;
    }
    catch (...)
    {
        membersById.erase(memberId);
        throw;
    }

    try
    {
        if (!repo.add(member))
        {
            membersById.erase(memberId);
            membersByPhone.erase(phone);
            return false;
        }
    }
    catch (...)
    {
        membersById.erase(memberId);
        membersByPhone.erase(phone);
        throw;
    }
    return true;
}

// Cap nhat thong tin Member
bool MemberRepository::update(const Member& newValues)
{
    if (!repo.isLoaded() || !isValidMember(&newValues))
    {
        return false;
    }

    Member* member = findById(newValues.getMemberId());

    if (member == nullptr)
    {
        return false;
    }

    // Khong cho phep thay doi diem truc tiep
    if (member->getPoints() != newValues.getPoints())
    {
        return false;
    }

    // Kiem tra trung so dien thoai
    Member* existingMember = findByPhone(newValues.getPhone());

    if (existingMember != nullptr && existingMember != member)
    {
        return false;
    }

    string oldPhone = member->getPhone();
    string newPhone = newValues.getPhone();

    // Them khoa moi vao map truoc
    if (oldPhone != newPhone)
    {
        membersByPhone[newPhone] = member;
    }

    try
    {
        member->setFullName(newValues.getFullName());
        member->setPhone(newPhone);
    }
    catch (...)
    {
        if (oldPhone != newPhone)
        {
            membersByPhone.erase(newPhone);
        }
        throw;
    }

    // Xoa khoa cu sau khi cap nhat thanh cong
    if (oldPhone != newPhone)
    {
        membersByPhone.erase(oldPhone);
    }

    return true;
}

bool MemberRepository::removeById(const string& memberId)
{
    return false;
}
