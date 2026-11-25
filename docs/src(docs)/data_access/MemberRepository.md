# MemberRepository

> **Trạng thái: ĐÃ CHỐT MARKDOWN — CHƯA TRIỂN KHAI, CHƯA KIỂM THỬ.**

## 1. Tổng quan

Concrete Repository kế thừa `IRepository<Member>` và chứa `Repository<Member>`; hai chỉ mục `membersById`, `membersByPhone` là con trỏ mượn. Chỉ refactor Repository; không chỉnh sửa Service, Model, interface chung hay schema TXT.

## 2. Kiến trúc

```text
IRepository<Member>
       ^ public inheritance
MemberRepository
       | composition
Repository<Member>
       |
DoublyLinkedList<Member*>
```

Service hiện tại không được thay đổi sang interface trong lượt này.

## 3. Định dạng dữ liệu

`memberId|fullName|phone|points` — đúng 4 trường, phân cách `|`; ID tiền tố `MEM`, điện thoại 10 chữ số bắt đầu bằng 0, điểm nguyên không âm. Ví dụ: `MEM0001|Nguyen Van A|0912345678|0`. Không thay đổi schema.

## 4. Thiết kế lớp

| Thành phần | Vai trò |
|---|---|
| `Repository<Member> repo` | Sở hữu các đối tượng Member |
| `membersById` | Chỉ mục theo ID, con trỏ mượn |
| `membersByPhone` | Chỉ mục theo điện thoại, con trỏ mượn |
| 4 hàm static trong lớp | Callback và validation |
| `isValidPhone` (static ở file .cpp) | Helper kiểm tra số điện thoại |
| 9 hàm override | Hợp đồng `IRepository<Member>` |
| `findByPhone` | API tra cứu riêng được giữ nguyên |

### 4.1. Quyết định đã chốt

Do API cũ không có xóa Member và Member có liên kết Invoice, `removeById(const string&)` **đã được chốt** chỉ trả `false`, không tác động dữ liệu, hai map hoặc liên kết Invoice.

## 5. Mã nguồn đã chốt — nguồn chuẩn để triển khai

### 5.1. `src/data_access/MemberRepository.h`

```cpp
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
    // map giúp tìm kiếm đối tượng member nhanh
    std::unordered_map<std::string, Member*> membersById;
    std::unordered_map<std::string, Member*> membersByPhone;

    static std::string getMemberId(Member* member);
    static Member* stringToMember(const std::string& line);
    static std::string memberToString(Member* member);
    static bool isValidMember(const Member* member);

public:
    MemberRepository(const std::string& filePath);

    bool load() override;
    bool save() override;

    bool add(Member* member) override;
    bool update(const Member& newValues) override;
    bool removeById(const std::string& memberId) override;

    Member* findById(const std::string& memberId) override;
    Member* findByPhone(const std::string& phone);
    Member* getAt(int index) override;

    int count() override;
    bool isLoaded() override;
};

#endif
```

### 5.2. `src/data_access/MemberRepository.cpp`

```cpp
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
```

## 6. Giải thích thư viện và từ khóa

- `<string>`: `string`, `find`, `substr`, `stoi`, `to_string`.
- `<unordered_map>`: chỉ mục ID và điện thoại; `find`, `erase`, `clear`, toán tử `[]`.
- `<vector>`: tách các trường TXT trong quá trình parse.
- `<stdexcept>`: `runtime_error` cho lỗi đọc/ghi/parse.
- `static`: callback không phụ thuộc một instance; helper `isValidPhone` có liên kết nội bộ trong translation unit.
- `override`: compiler kiểm tra chữ ký trùng với hàm ảo của interface.
- `public IRepository<Member>`: cho phép sử dụng qua `IRepository<Member>&`.
- `try/catch (...)`: khôi phục chỉ mục và ném lại exception.

## 7. Giải thích các hàm private và helper

### 7.1. `isValidPhone()`

**A. Tổng quan:** Kiểm tra số điện thoại.

**B. Luồng hoạt động ASCII:**

```text
Bắt đầu
   |
   v
Nếu độ dài khác 10 hoặc không bắt đầu bằng 0 thì false; duyệt từng ký tự, nếu ngoài 0–9 thì false; còn lại true.
   |
   v
Kết thúc
```

**C. Giải thích code và trạng thái:**
Không truy cập phone[0] khi chuỗi ngắn vì toán tử || dừng sớm.

### 7.2. `isValidMember()`

**A. Tổng quan:** Kiểm tra dữ liệu thành viên.

**B. Luồng hoạt động ASCII:**

```text
Bắt đầu
   |
   v
Null
   |
   v
false; kiểm tra MEM ID, tên, số điện thoại, điểm không âm
   |
   v
true nếu tất cả hợp lệ.
   |
   v
Kết thúc
```

**C. Giải thích code và trạng thái:**
Dùng Validator cho ID và tên, helper isValidPhone cho điện thoại.

### 7.3. `getMemberId()`

**A. Tổng quan:** Trả khóa ID.

**B. Luồng hoạt động ASCII:**

```text
Bắt đầu
   |
   v
Null
   |
   v
chuỗi rỗng; ngược lại
   |
   v
getMemberId().
   |
   v
Kết thúc
```

**C. Giải thích code và trạng thái:**
Callback static được Repository<Member> sử dụng để tra khóa.

### 7.4. `stringToMember()`

**A. Tổng quan:** Chuyển TXT thành Member.

**B. Luồng hoạt động ASCII:**

```text
Bắt đầu
   |
   v
Tách theo |
   |
   v
phải có 4 trường
   |
   v
kiểm tra points chỉ gồm số
   |
   v
stoi và kiểm tra đủ chuỗi
   |
   v
new Member
   |
   v
isValidMember
   |
   v
trả con trỏ; sai thì throw và giải phóng đối tượng nếu đã cấp phát.
   |
   v
Kết thúc
```

**C. Giải thích code và trạng thái:**
Dữ liệu lỗi không được bỏ qua; đối tượng được trả cho Repository quản lý khi load thành công.

### 7.5. `memberToString()`

**A. Tổng quan:** Chuyển Member thành TXT.

**B. Luồng hoạt động ASCII:**

```text
Bắt đầu
   |
   v
Kiểm tra hợp lệ
   |
   v
nối ID|tên|điện thoại|điểm
   |
   v
trả chuỗi.
   |
   v
Kết thúc
```

**C. Giải thích code và trạng thái:**
Sai dữ liệu sẽ throw runtime_error; không ghi file ở đây.

## 8. Giải thích các hàm public

### 8.1. `MemberRepository()`

**A. Tổng quan:** Khởi tạo repository.

**B. Luồng hoạt động ASCII:**

```text
Bắt đầu
   |
   v
Nhận filePath
   |
   v
khởi tạo Repository<Member> với 3 callback
   |
   v
kết thúc.
   |
   v
Kết thúc
```

**C. Giải thích code và trạng thái:**
Hai unordered_map mặc định rỗng; constructor chưa load file.

### 8.2. `load()`

**A. Tổng quan:** Nạp dữ liệu và xây hai map.

**B. Luồng hoạt động ASCII:**

```text
Bắt đầu
   |
   v
Đã load
   |
   v
throw; repo.load thất bại
   |
   v
throw; xóa chỉ mục cũ; duyệt và phát hiện ID/phone trùng
   |
   v
throw; nếu lỗi xóa map, repo.clear rồi ném lại; thành công true.
   |
   v
Kết thúc
```

**C. Giải thích code và trạng thái:**
Map chỉ mượn con trỏ từ Repository; rollback tránh con trỏ treo khi load lỗi.

### 8.3. `save()`

**A. Tổng quan:** Ghi dữ liệu.

**B. Luồng hoạt động ASCII:**

```text
Bắt đầu
   |
   v
Chưa load
   |
   v
throw; repo.save thất bại
   |
   v
throw; thành công true.
   |
   v
Kết thúc
```

**C. Giải thích code và trạng thái:**
Giữ nguyên hợp đồng ngoại lệ của code cũ.

### 8.4. `add()`

**A. Tổng quan:** Thêm thành viên.

**B. Luồng hoạt động ASCII:**

```text
Bắt đầu
   |
   v
Chưa load/dữ liệu sai/điểm khác 0/trùng ID/trùng phone/trùng pointer
   |
   v
false; thêm hai map; repo.add; nếu thất bại rollback map; thành công true.
   |
   v
Kết thúc
```

**C. Giải thích code và trạng thái:**
Chỉ sau repo.add thành công Repository sở hữu pointer; nếu thất bại caller sở hữu.

### 8.5. `update()`

**A. Tổng quan:** Cập nhật thành viên.

**B. Luồng hoạt động ASCII:**

```text
Bắt đầu
   |
   v
Kiểm tra loaded và dữ liệu; tìm ID; từ chối đổi điểm hoặc trùng phone; thêm khóa phone mới; setFullName và setPhone; rollback khóa mới nếu exception; xóa khóa cũ; true.
   |
   v
Kết thúc
```

**C. Giải thích code và trạng thái:**
Không đổi ID, điểm hay địa chỉ đối tượng; liên kết Invoice được giữ nguyên.

### 8.6. `removeById()`

**A. Tổng quan:** Không hỗ trợ xóa thành viên.

**B. Luồng hoạt động ASCII:**

```text
Bắt đầu
   |
   v
Nhận memberId
   |
   v
trả false.
   |
   v
Kết thúc
```

**C. Giải thích code và trạng thái:**
Không xóa dữ liệu, không thay đổi map hay liên kết Invoice; tham số hiện chưa sử dụng.

### 8.7. `findById()`

**A. Tổng quan:** Tra cứu theo ID.

**B. Luồng hoạt động ASCII:**

```text
Bắt đầu
   |
   v
Tra membersById
   |
   v
không có trả nullptr
   |
   v
có trả con trỏ.
   |
   v
Kết thúc
```

**C. Giải thích code và trạng thái:**
Con trỏ mượn, caller không delete.

### 8.8. `findByPhone()`

**A. Tổng quan:** Tra cứu theo số điện thoại.

**B. Luồng hoạt động ASCII:**

```text
Bắt đầu
   |
   v
Tra membersByPhone
   |
   v
không có trả nullptr
   |
   v
có trả con trỏ.
   |
   v
Kết thúc
```

**C. Giải thích code và trạng thái:**
Không thuộc 9 hàm interface nhưng phải giữ API cũ.

### 8.9. `getAt()`

**A. Tổng quan:** Truy cập theo chỉ số.

**B. Luồng hoạt động ASCII:**

```text
Bắt đầu
   |
   v
Gọi repo.getAt(index)
   |
   v
trả kết quả.
   |
   v
Kết thúc
```

**C. Giải thích code và trạng thái:**
Con trỏ mượn; chỉ số không hợp lệ có thể trả nullptr.

### 8.10. `count()`

**A. Tổng quan:** Đếm số bản ghi.

**B. Luồng hoạt động ASCII:**

```text
Bắt đầu
   |
   v
Gọi repo.count()
   |
   v
trả số lượng.
   |
   v
Kết thúc
```

**C. Giải thích code và trạng thái:**
Không thay đổi map.

### 8.11. `isLoaded()`

**A. Tổng quan:** Kiểm tra trạng thái.

**B. Luồng hoạt động ASCII:**

```text
Bắt đầu
   |
   v
Gọi repo.isLoaded()
   |
   v
trả bool.
   |
   v
Kết thúc
```

**C. Giải thích code và trạng thái:**
Không thực hiện load.

## 9. Quản lý bộ nhớ

- `Repository<Member>` sở hữu `Member*` sau khi `add()` thành công hoặc load thành công; destructor của `Repository` giải phóng.
- Hai map chỉ mượn con trỏ, không `delete`.
- `findById`, `findByPhone`, `getAt` trả con trỏ mượn.
- `load()` lỗi xây map sẽ xóa cả hai map rồi `repo.clear()`; `add()` lỗi rollback map.
- Không khai báo destructor riêng: destructor mặc định hủy map rồi hủy `repo`; không xóa Member bằng `removeById`.

## 10. Áp dụng Dependency Inversion Principle

Trước: `class MemberRepository`. Sau: `class MemberRepository : public IRepository<Member>`. 9 hàm interface có `override`, nhưng Service không thay đổi ở giai đoạn này.

## 11. Các thay đổi sau refactor

- Header: include `IRepository.h`, kế thừa công khai, 9 `override`, khai báo `removeById`.
- CPP: giữ nguyên toàn bộ logic cũ, bổ sung duy nhất định nghĩa `removeById` trả `false`.
- Không thay đổi ID, schema, điểm, hai map, link Invoice, ngoại lệ hay API cũ.

## 12. Kế hoạch kiểm thử

- [ ] Biên dịch C++17 tất cả `.cpp` liên quan.
- [ ] Tạo `IRepository<Member>&` và gọi đủ 9 hàm.
- [ ] `removeById` luôn `false`, không đổi `count`, map, điểm hay Invoice.
- [ ] Load TXT hợp lệ; kiểm tra trùng ID/phone, points sai, số trường sai.
- [ ] Add hợp lệ, điểm ban đầu khác 0, trùng ID, trùng phone, nullptr, trùng pointer.
- [ ] Update tên/phone; từ chối đổi điểm, phone trùng; giữ liên kết Invoice.
- [ ] Kiểm tra rollback map khi lỗi load/add.
- [ ] Save và load lại dữ liệu; kiểm tra format 4 trường.
- [ ] Chạy test Member và test Service liên quan; báo riêng lỗi ngoài phạm vi.

## 13. Tiến độ triển khai

- [x] Đọc mã nguồn và đối chiếu mẫu MovieRepository.md.
- [x] Thiết kế dự kiến và ghi đủ mã nguồn `.h/.cpp`.
- [x] Review và CHỐT MARKDOWN (bao gồm phương án removeById).
- [ ] Sao chép nguyên văn mã nguồn vào dự án.
- [ ] Biên dịch và kiểm thử thực tế.

## 14. Kết luận

Bản Markdown **đã chốt** chỉ bổ sung hợp đồng `IRepository<Member>` và hàm xóa chưa hỗ trợ. Có thể triển khai nguyên văn hai khối mã ở mục 5, sau đó biên dịch và kiểm thử. Chưa xác nhận đã triển khai hoặc kiểm thử thành công.
