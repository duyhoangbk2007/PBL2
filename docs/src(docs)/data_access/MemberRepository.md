# MemberRepository — thiết kế refactor đã CHỐT MARKDOWN

## 1. Tổng quan

`MemberRepository` thuộc Data Access, kế thừa `IRepository<Member>` và chứa `Repository<Member>` để quản lý `Member*`. Tài liệu đã **CHỐT MARKDOWN**; hai khối mã ở mục 5 là bản chuẩn để triển khai. Chưa ghi nhận kết quả kiểm thử cho bản refactor.

Lớp thực hiện bốn nhóm trách nhiệm: kiểm tra thuộc tính Member và số điện thoại duy nhất; chuyển đổi giữa Member và dòng TXT; điều phối `Repository<Member>` khi nạp, lưu, thêm và truy vấn; giữ chỉ mục phone nhất quán với danh sách Member. Lớp không cấp Permission, sinh ID, cộng/trừ điểm nghiệp vụ hoặc sở hữu hóa đơn.

**File liên quan:** `src/data_access/IRepository.h`, `Repository.h`, `MemberRepository.h/.cpp`, `src/models/Member.h`, `src/utils/Validator.h`. Cần phân biệt mã đã triển khai ở mục 5 với các bài test đang nằm trong kế hoạch mục 12.

Phạm vi đã thống nhất: bỏ `membersById`, giữ `membersByPhone` cho tra cứu số điện thoại; dùng `Repository<Member>` làm nơi lưu và sở hữu đối tượng duy nhất. Giữ schema `members.txt`, các chữ ký public và chính sách chưa xóa thành viên. Mã cho `load()`, `add()` và `update()` đã được chốt. Service xử lý Permission và nghiệp vụ điểm; Repository kiểm tính hợp lệ và toàn vẹn dữ liệu.

Đối chiếu: [MovieRepository.md](MovieRepository.md), [UserRepository.md](UserRepository.md), [validation_rules.md](../../architecture/validation_rules.md), [business_rules.md](../../overview/business_rules.md), [data_format.md](../../overview/data_format.md).

## 2. Kiến trúc

### 2.1. Quan hệ giữa các lớp

```text
Service → IRepository<Member> ← MemberRepository
                                      │ composition
                                      ▼
                              Repository<Member>
                                      │ sở hữu Member*
                                      ▼
                           DoublyLinkedList<Member*>

MemberRepository → membersByPhone → Member* mượn
```

### 2.2. Quan hệ với `IRepository<Member>`

Kế thừa công khai cho phép dùng một `MemberRepository` qua `IRepository<Member>&` hoặc `IRepository<Member>*`. Chín phương thức chung có `override`; `findByPhone()` là API riêng của concrete Repository và không được thêm vào interface chung. Interface không giữ dữ liệu hay quyền sở hữu `Member*`. `MemberService` hiện còn phụ thuộc concrete Repository; tài liệu này chỉ xác nhận DIP tại tầng Repository.

### 2.3. Quan hệ với `Repository<Member>` và chỉ mục phone

`Repository<Member>` đã tra ID bằng callback `getMemberId`, nên `membersById` lặp lại chức năng lưu trữ chung và phải đồng bộ thêm khi load/add. `findById()` chuyển tiếp trực tiếp đến `repo.findById()`. Số điện thoại là thuộc tính duy nhất ngoài ID; generic Repository không lập chỉ mục theo phone. Vì vậy giữ `membersByPhone` để tra cứu nhanh và kiểm tra trùng số. Map chỉ giữ con trỏ mượn, không được `delete` Member. Mã đã triển khai đồng bộ map khi load, add và update; rollback ở `update()` chỉ hoàn tác chỉ mục mới nếu setter ném, chưa bảo đảm hoàn tác toàn bộ thuộc tính.

Theo [validation_rules.md](../../architecture/validation_rules.md), `Repository<Member>` kiểm loaded, null, trùng con trỏ, ID rỗng/trùng, vị trí và lỗi lưu trữ. `MemberRepository` kiểm ID đúng dạng, tên, số điện thoại duy nhất, điểm không âm và các ràng buộc riêng khi thêm/cập nhật. Service kiểm Permission, điều kiện nghiệp vụ và sinh ID. Không kiểm lặp điều kiện tầng dưới đã bảo đảm nếu không cần cho an toàn ở tầng hiện tại.

## 3. Định dạng dữ liệu

Mỗi dòng `members.txt` có bốn trường theo đúng thứ tự, phân cách bằng `|`, không có header:

```text
memberId|fullName|phone|points
MEM0001|Nguyen Van A|0912345678|0
```

| STT | Trường | Kiểu trong Model | Kiểm tra khi parse |
|---|---|---|---|
| 1 | `memberId` | `std::string` | `Validator::isValidId(..., "MEM")` |
| 2 | `fullName` | `std::string` | `Validator::isValidText()` |
| 3 | `phone` | `std::string` | `Validator::isValidPhone()`, duy nhất giữa Member |
| 4 | `points` | `int` | Chuỗi không rỗng, chỉ có chữ số; `stoi` trong miền `int`; giá trị không âm |

ID có tiền tố `MEM` và bốn chữ số. Tên phải là văn bản hợp lệ; phone đúng mười chữ số và bắt đầu bằng `0`; points là số nguyên không âm. `stringToMember()` phải kiểm số trường và parse points trước khi tạo `Member`, rồi gọi `isValidMember()`. **Chuỗi points phải không rỗng và chỉ gồm chữ số ASCII `0`–`9`**; không nhận dấu `+`, dấu `-`, khoảng trắng hoặc chữ. `stoi` chuyển chuỗi số và báo lỗi nếu vượt phạm vi `int`. Không đổi schema, cách cấp ID hoặc nghiệp vụ điểm trong đợt này.

## 4. Thiết kế lớp

### 4.1. Thuộc tính

| Thành phần | Vai trò |
|---|---|
| `Repository<Member> repo` | Lưu trữ và sở hữu mọi `Member*` |
| `std::unordered_map<std::string, Member*> membersByPhone` | Chỉ mục mượn theo phone, không sở hữu Member |
| `getMemberId`, `stringToMember`, `memberToString` | Callback ID, parse, serialize cho generic Repository |
| `isValidMember` | Validation riêng của Member |

Header không có `membersById`; chỉ giữ `<unordered_map>` vì `membersByPhone` vẫn cần. Header dùng `std::` đầy đủ và tự include `<string>`, `<unordered_map>`. `.cpp` dùng `using namespace std;` sau toàn bộ include. Thứ tự nhóm: callback, validation, constructor, load/save, add/update/remove, truy vấn.

### 4.2. Các hàm private

| Hàm | Trả về | Vai trò |
|---|---|---|
| `getMemberId(Member*)` | `std::string` | Callback lấy ID, null trả chuỗi rỗng |
| `stringToMember(const std::string&)` | `Member*` | Parse một dòng TXT, cấp phát Member hợp lệ |
| `memberToString(Member*)` | `std::string` | Serialize Member thành một dòng TXT |
| `isValidMember(const Member*)` | `bool` | Kiểm thuộc tính Member bằng Validator và điểm không âm |

Các helper đều `static`, không truy cập trực tiếp trạng thái của `MemberRepository`. `getMemberId`, `stringToMember`, `memberToString` được truyền vào constructor của `Repository<Member>` dưới dạng callback.

### 4.3. Các hàm public

| Nhóm | Hàm | Trả về | Vai trò |
|---|---|---|---|
| Khởi tạo | `MemberRepository(filePath)` | — | Tạo `repo` và map rỗng, chưa load |
| TXT | `load()`, `save()` | `bool` | Nạp và ghi dữ liệu |
| Chỉnh sửa | `add(member)`, `update(newValues)`, `removeById(memberId)` | `bool` | Thêm, sửa; xóa luôn bị từ chối |
| Truy vấn | `findById(memberId)`, `findByPhone(phone)`, `getAt(index)` | `Member*` | Trả con trỏ mượn hoặc null |
| Trạng thái | `count()`, `isLoaded()` | `int`, `bool` | Đếm và xem trạng thái loaded |

`load`, `save`, `add`, `update`, `removeById`, `findById`, `getAt`, `count`, `isLoaded` là chín phương thức của interface. `findByPhone` phục vụ kiểm tra thuộc tính duy nhất ngoài ID.

## 5. Mã nguồn đã chốt

Các khối sau đã **CHỐT MARKDOWN**, bao gồm định nghĩa của `load()`, `add()` và `update()`. `Validator::isValidPhone()` đã tồn tại trong mã hiện tại với cùng logic helper cục bộ; MemberRepository gọi lại hàm này mà không thay đổi Validator.

### 5.1. `MemberRepository.h`

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
```

### 5.2. `MemberRepository.cpp`

```cpp
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
```

## 6. Thư viện và cú pháp

### 6.1. Thư viện và include

| Include | Nơi dùng | Lý do |
|---|---|---|
| `<string>` | Header | Khai báo `std::string` cho đường dẫn, ID và phone; header tự đủ include |
| `<unordered_map>` | Header | Khai báo `std::unordered_map<std::string, Member*> membersByPhone` |
| `IRepository.h` | Header | Kế thừa interface `IRepository<Member>` |
| `Repository.h` | Header | Thành viên `Repository<Member> repo` cần kiểu đầy đủ |
| `Member.h` | Header | Chữ ký, callback và validation dùng Model `Member` |
| `Validator.h` | `.cpp` | Gọi `isValidId`, `isValidText`, `isValidPhone` |
| `<vector>` | `.cpp` | `vector<string> fields` giữ bốn trường của một dòng TXT |
| `<stdexcept>` | `.cpp` | `runtime_error` báo lỗi parse và serialize |

Header dùng `std::string` và `std::unordered_map` theo quy ước namespace, không dùng `using namespace std;`. Sau toàn bộ `#include`, `.cpp` khai báo `using namespace std;`, nên dùng `string`, `vector`, `runtime_error` trong định nghĩa. `std::string` là kiểu lưu chuỗi, `to_string(int)` tạo trường points khi ghi; `stoi(string)` chuyển points khi đọc. Không include thư viện chỉ vì header khác tình cờ đã include.

### 6.2. `unordered_map` và `iterator`

`unordered_map<Key, Value>` là bảng băm của thư viện chuẩn. Ở đây `Key` là `std::string` phone, `Value` là `Member*` mượn. `membersByPhone[phone] = member` tạo hoặc thay entry theo khóa; nó không sao chép hay sở hữu đối tượng Member. `find(phone)` trả **iterator**, tức vị trí của entry khi tìm được; nếu không có, nó trả `end()`. `end()` là vị trí đánh dấu sau phần tử cuối, không được dereference. Vì vậy `findByPhone()` so `it == membersByPhone.end()` trước khi dùng `it->second`. `auto` để compiler suy ra kiểu iterator dài của `unordered_map` mà không đổi hành vi.

Map cho phép tra phone trung bình O(1), nhưng vẫn phải giữ nhất quán với `repo`, nơi sở hữu và quản lý ID. Map không giữ quyền sở hữu, nên phải xóa entry trước khi `repo.clear()` giải phóng Member khi load lỗi. Trong `add()` và `update()`, `erase(key)` gỡ **chỉ mục**, không `delete Member*`. Trường hợp ngoại lệ khi setter trong `update()` đã làm đổi Model được mô tả riêng ở mục 8.5.

### 6.3. Từ khóa và cú pháp C++17

`static` trên helper cho phép truyền callback vào constructor generic Repository mà không cần một instance `MemberRepository`. `override` yêu cầu compiler kiểm chữ ký khớp hàm ảo của interface. `const Member*` và `const string&` ngăn sửa đầu vào qua tham số; `&newValues` lấy địa chỉ tham chiếu để gọi validator. `nullptr` là giá trị con trỏ rỗng; phải kiểm trước toán tử `->`. Danh sách khởi tạo sau `:` tạo `repo` trước thân constructor. `try`/`catch (...)` trong `load`, `add`, `update` phục vụ dọn chỉ mục và ném lại bằng `throw;`; không chuyển lỗi thành thành công. `catch (const exception&)` ở parser gom lỗi `stoi` thành lỗi points. `new` tạo `Member` trên heap; `delete` giải phóng khi validation sau parse thất bại. `(void)memberId;` đánh dấu tham số cố ý chưa dùng trong hàm xóa luôn từ chối.

## 7. Giải thích các hàm private

### 7.1. `isValidMember(const Member* member)`

**Chữ ký:** `static bool isValidMember(const Member* member)`; tham số là con trỏ chỉ đọc, có thể null. Hàm trả `true` khi ID, tên, phone và điểm hợp lệ; ngược lại trả `false`.

**A. Luồng xử lý**

```text
member == nullptr? ─có→ false
↓ không
ID hợp lệ với tiền tố MEM? ─không→ false
↓ có
fullName hợp lệ? ─không→ false
↓ có
phone hợp lệ? ─không→ false
↓ có
points < 0? ─có→ false
↓ không
true
```

**B. Giải thích code**

1. Chặn null trước khi đọc thuộc tính:

   ```cpp
   if (member == nullptr)
   {
       return false;
   }
   ```

2. Kiểm ID và tên bằng Validator; mỗi điều kiện sai kết thúc ngay:

   ```cpp
   if (!Validator::isValidId(member->getMemberId(), "MEM"))
   {
       return false;
   }
   if (!Validator::isValidText(member->getFullName()))
   {
       return false;
   }
   ```

3. Kiểm phone và điểm rồi trả kết quả. Tính duy nhất của phone do `load/add/update` kiểm:

   ```cpp
   if (!Validator::isValidPhone(member->getPhone()))
   {
       return false;
   }
   if (member->getPoints() < 0)
   {
       return false;
   }
   return true;
   ```

### 7.2. `getMemberId(Member* member)`

**Chữ ký:** `static string getMemberId(Member* member)` trong `.cpp` (`std::string` trong header). Callback nhận con trỏ Member, trả bản sao ID hoặc chuỗi rỗng nếu null.

**A. Luồng xử lý**

```text
member == nullptr? ─có→ return ""
↓ không
return member->getMemberId()
```

**B. Giải thích code**

```cpp
if (member == nullptr)
{
    return "";
}
return member->getMemberId();
```

Điều kiện null ngăn dereference; nhánh còn lại dùng getter qua `->`. Generic Repository gọi callback để tra và kiểm ID; nó không chuyển quyền sở hữu con trỏ cho callback.

### 7.3. `stringToMember(const string& line)`

**Chữ ký:** `static Member* stringToMember(const string& line)` trong `.cpp` (`std::string` trong header). Hàm nhận một dòng TXT qua tham chiếu chỉ đọc, trả `Member*` mới hoặc ném `runtime_error`.

**A. Luồng xử lý**

```text
Tách line bằng '|' vào fields
↓
fields.size() == 4? ─không→ throw
↓ có
points không rỗng và chỉ gồm '0'..'9'? ─không→ throw
↓ có
stoi(fields[3]) ─exception→ catch → throw runtime_error
↓ thành công
new Member → isValidMember(member)? ─không→ delete member → throw
↓ có
return member
```

**B. Giải thích code**

1. Tách chuỗi và đếm trường:

   ```cpp
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
   ```

   `start` đánh dấu đầu trường; `find` và `substr` giữ cả trường cuối rỗng. Kiểm số trường trước khi đọc `fields[3]`:

   ```cpp
   if (fields.size() != 4)
   {
       throw runtime_error("Sai so luong truong Member");
   }
   ```
2. Kiểm chuỗi points rồi chuyển sang `int`:

   ```cpp
   if (fields[3].empty())
   {
       throw runtime_error("Points Member khong hop le");
   }
   for (char c : fields[3])
   {
       if (c < '0' || c > '9')
       {
           throw runtime_error("Points Member khong hop le");
       }
   }
   ```

   Vòng ký tự loại dấu, khoảng trắng và hậu tố; do đó `stoi` không cần tham số `pos`. `catch` đổi lỗi chuyển số hoặc tràn `int` thành lỗi points. Chuỗi `"0001"` được nhận là 1:

   ```cpp
   int points;
   try
   {
       points = stoi(fields[3]);
   }
   catch (const exception&)
   {
       throw runtime_error("Points Member khong hop le");
   }
   ```

3. Cấp phát đối tượng và kiểm lại các thuộc tính. Nếu sai, giải phóng trước khi ném; nếu đúng, generic Repository nhận quyền sở hữu trong quá trình load. ID trùng do generic Repository kiểm, phone trùng do `MemberRepository::load()` kiểm:

   ```cpp
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
   ```

### 7.4. `memberToString(Member* member)`

**Chữ ký:** `static string memberToString(Member* member)` trong `.cpp`. Hàm nhận con trỏ Member, trả một dòng TXT bốn trường hoặc ném `runtime_error` khi dữ liệu sai.

**A. Luồng xử lý**

```text
isValidMember(member)? ─không→ throw runtime_error
↓ có
Nối memberId|fullName|phone|points
↓
return string
```

**B. Giải thích code**

1. Kiểm cả null và thuộc tính sai trước khi dùng getter:

   ```cpp
   if (!isValidMember(member))
   {
       throw runtime_error("Member khong hop le khi ghi file");
   }
   ```

2. Nối đúng thứ tự schema và chuyển điểm sang chuỗi. Callback tạo dòng TXT; `repo.save()` chịu trách nhiệm ghi file:

   ```cpp
   return member->getMemberId() + "|" +
          member->getFullName() + "|" +
          member->getPhone() + "|" +
          to_string(member->getPoints());
   ```

## 8. Giải thích các hàm public

### 8.1. Constructor `MemberRepository(const string& filePath)`

**Chữ ký:** `MemberRepository(const string& filePath)` trong `.cpp` (`std::string` trong header). Nhận đường dẫn TXT, không có giá trị trả về và không tự load.

**A. Luồng xử lý**

```text
Nhận filePath
↓
Khởi tạo repo(filePath, getMemberId, stringToMember, memberToString)
↓
Khởi tạo membersByPhone rỗng → kết thúc
```

**B. Giải thích code**

```cpp
MemberRepository::MemberRepository(const string& filePath)
    : repo(filePath, getMemberId, stringToMember, memberToString)
{
}
```

Danh sách khởi tạo cung cấp đường dẫn và ba callback cho `repo`; map được khởi tạo mặc định rỗng. Chưa có Member nào được nạp hoặc cấp phát.

### 8.2. `load()`

**Chữ ký:** `bool load() override`. Không có tham số; trả `false` nếu `repo.load()` từ chối, `true` khi map hoàn chỉnh; lỗi parse hoặc phone trùng được ném ra.

**A. Luồng xử lý**

```text
repo.load() ─false→ return false
↓ true
try: duyệt i từ 0 đến repo.count()-1
↓ mỗi Member
findByPhone(phone) khác null? ─có→ throw runtime_error
↓ không
membersByPhone[phone] = member → lặp
↓ duyệt hết
return true
Ngoại lệ trong try → catch (...) → membersByPhone.clear()
↓
repo.clear() → throw lại
```

**B. Giải thích code**

1. Gọi tầng lưu trữ:

   ```cpp
   if (!repo.load())
   {
       return false;
   }
   ```

   Generic Repository đã xử lý file, trạng thái loaded, parse và ID trùng. Hàm không xóa map trước load; nếu `repo.load()` trả false thì map và repo giữ nguyên trạng thái của nhánh đó.
2. Duyệt dữ liệu đã nạp và lập chỉ mục:

   ```cpp
   for (int i = 0; i < repo.count(); i++)
   {
       Member* member = repo.getAt(i);
       if (findByPhone(member->getPhone()) != nullptr)
       {
           throw runtime_error("Trung phone trong file Member");
       }
       membersByPhone[member->getPhone()] = member;
   }
   ```

   `findByPhone` chỉ thấy các Member trước đó trong map; gặp phone trùng thì ném lỗi. Toán tử `[]` thêm con trỏ mượn; không kiểm lại ID vì `repo.load()` đã làm.
3. Nếu bước 2 ném, dọn map **trước** khi `repo.clear()` giải phóng Member, rồi ném lại. Chỉ sau khi duyệt hết mới trả `true`. Ngoại lệ từ `repo.load()` xảy ra trước `try` và do generic Repository tự dọn danh sách tạm:

   ```cpp
   catch (...)
   {
       membersByPhone.clear();
       repo.clear();
       throw;
   }
   return true;
   ```

### 8.3. `save()`

**Chữ ký:** `bool save() override`. Không có tham số; trả kết quả của generic Repository, có thể truyền exception từ serializer.

**A. Luồng xử lý**

```text
repo.save()
↓
return true/false; exception từ serializer → truyền ra
```

**B. Giải thích code**

```cpp
bool MemberRepository::save()
{
    return repo.save();
}
```

Generic Repository kiểm loaded, serialize từng Member rồi ghi TXT. Hàm không dùng map và không tự save từ `add()` hay `update()`; kết quả false hoặc exception không bị đổi thành công.

### 8.4. `add(Member* member)`

**Chữ ký:** `bool add(Member* member) override`. Nhận con trỏ do caller sở hữu trước khi thêm; trả `true` khi repo nhận quyền sở hữu, `false` khi điều kiện không đạt; exception được truyền lại sau dọn map.

**A. Luồng xử lý**

```text
loaded và isValidMember? ─không→ false
↓ có
points == 0? ─không→ false
↓ có
phone chưa có trong map? ─không→ false
↓ có
Sao chép phone → chèn membersByPhone[phone] = member
↓
try: repo.add(member) ─true→ return true
↓ false                              └─exception→ catch → erase(phone) → throw
erase(phone) → return false
```

**B. Giải thích code**

1. Kiểm trước khi thay trạng thái:

   ```cpp
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
   ```

   Loaded được kiểm vì map sẽ chèn trước `repo.add()`. `isValidMember` chặn null; điểm ban đầu bằng 0 và phone duy nhất là quy tắc riêng. Generic Repository kiểm ID/con trỏ trùng.
2. Giữ khóa cần rollback rồi thêm con trỏ mượn. Nếu chèn map ném trước `try`, `repo.add()` chưa chạy và caller vẫn sở hữu Member:

   ```cpp
   string phone = member->getPhone();
   membersByPhone[phone] = member;
   ```

3. `repo.add(member)` thành công thì trả `true` và repo nhận quyền sở hữu. Nếu ném, xóa key và ném lại; nếu trả false, xóa key bên ngoài `try` rồi trả false:

   ```cpp
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
   ```

### 8.5. `update(const Member& newValues)`

**Chữ ký:** `bool update(const Member& newValues) override`. Nhận giá trị mới chỉ đọc; trả false khi chưa load, dữ liệu sai, ID thiếu, điểm khác hoặc phone thuộc Member khác; trả true sau sửa. Setter có thể ném exception.

**A. Luồng xử lý**

```text
loaded và isValidMember(&newValues)? ─không→ false
↓ có
repo.findById(ID) ─null→ false
↓ có Member
points bằng nhau? ─không→ false
↓ có
phone mới thuộc Member khác? ─có→ false
↓ không
Lưu oldPhone, newPhone
↓
phone không đổi? ─có→ setFullName → true
↓ đổi
membersByPhone[newPhone] = member
↓
try: setFullName → setPhone ─exception→ catch → erase(newPhone) → throw
↓ thành công
erase(oldPhone) → true
```

**B. Giải thích code**

1. Kiểm loaded và dữ liệu trước khi sửa Model; tìm đúng đối tượng theo ID bất biến:

   ```cpp
   if (!repo.isLoaded() || !isValidMember(&newValues))
   {
       return false;
   }
   Member* member = repo.findById(newValues.getMemberId());
   if (member == nullptr)
   {
       return false;
   }
   ```

2. Chặn sửa điểm trực tiếp. Phone mới được dùng nếu chưa có hoặc đang thuộc chính Member này; các nhánh false chưa gọi setter:

   ```cpp
   if (member->getPoints() != newValues.getPoints())
   {
       return false;
   }
   Member* existingMember = findByPhone(newValues.getPhone());
   if (existingMember != nullptr && existingMember != member)
   {
       return false;
   }
   ```

3. Lưu hai khóa trước khi sửa. Nếu phone giữ nguyên, chỉ đổi tên; map và phone không đổi:

   ```cpp
   string oldPhone = member->getPhone();
   string newPhone = newValues.getPhone();
   if (oldPhone == newPhone)
   {
       member->setFullName(newValues.getFullName());
       return true;
   }
   ```

4. Khi đổi phone, thêm key mới trước, rồi gọi hai setter theo thứ tự. Thành công thì bỏ key cũ; ID, địa chỉ và liên kết Invoice vẫn giữ nguyên:

   ```cpp
   membersByPhone[newPhone] = member;
   try
   {
       member->setFullName(newValues.getFullName());
       member->setPhone(newPhone);
   }
   ```

5. Nếu setter ném, chỉ xóa key mới và ném lại; sau khi setter thành công mới xóa key cũ. **Giới hạn rollback:** tên hoặc phone có thể đã đổi trước ngoại lệ; mã không phục hồi toàn bộ Model, nên map có thể không còn khớp thuộc tính nếu setter sửa rồi ném:

   ```cpp
   catch (...)
   {
       membersByPhone.erase(newPhone);
       throw;
   }
   membersByPhone.erase(oldPhone);
   return true;
   ```

### 8.6. `removeById(const string& memberId)`

**Chữ ký:** `bool removeById(const string& memberId) override`. Nhận ID nhưng luôn trả false để giữ lịch sử giao dịch.

**A. Luồng xử lý**

```text
Nhận memberId
↓
(void)memberId
↓
return false
```

**B. Giải thích code**

```cpp
(void)memberId;
return false;
```

Ép sang `void` đánh dấu tham số cố ý không dùng. Hàm không gọi `repo.removeById()`, không đổi map hay giải phóng Member, kể cả khi ID tồn tại.

### 8.7. `findById(const string& memberId)`

**Chữ ký:** `Member* findById(const string& memberId) override`. Nhận ID, trả con trỏ mượn hoặc `nullptr`.

**A. Luồng xử lý**

```text
Nhận memberId
↓
repo.findById(memberId)
↓
return Member*/nullptr
```

**B. Giải thích code**

```cpp
return repo.findById(memberId);
```

Generic Repository duyệt danh sách bằng callback ID. Không cần map ID; caller không sở hữu con trỏ trả về.

### 8.8. `findByPhone(const string& phone)`

**Chữ ký:** `Member* findByPhone(const string& phone)`. Nhận phone, trả con trỏ mượn hoặc `nullptr`.

**A. Luồng xử lý**

```text
membersByPhone.find(phone)
↓
iterator == end()? ─có→ return nullptr
↓ không
return it->second
```

**B. Giải thích code**

```cpp
auto it = membersByPhone.find(phone);
if (it == membersByPhone.end())
{
    return nullptr;
}
return it->second;
```

`auto` suy ra kiểu iterator của `unordered_map`; so với `end()` trước khi lấy `second` tránh dereference iterator không hợp lệ. `second` là `Member*` mượn; hàm không xác minh lại thuộc tính phone trong Model.

### 8.9. `getAt(int index)`

**Chữ ký:** `Member* getAt(int index) override`. Nhận vị trí từ 0, trả con trỏ mượn hoặc null nếu ngoài phạm vi.

**A. Luồng xử lý**

```text
Nhận index
↓
repo.getAt(index)
↓
return Member*/nullptr
```

**B. Giải thích code**

```cpp
return repo.getAt(index);
```

Generic Repository kiểm vị trí qua danh sách liên kết; không đọc file hoặc map.

### 8.10. `count()`

**Chữ ký:** `int count() override`. Không có tham số, trả số Member đang được repo lưu.

**A. Luồng xử lý**

```text
repo.count()
↓
return int
```

**B. Giải thích code**

```cpp
return repo.count();
```

Đếm danh sách gốc thay vì entry map; trước load hoặc sau rollback có thể bằng 0.

### 8.11. `isLoaded()`

**Chữ ký:** `bool isLoaded() override`. Không có tham số, trả trạng thái loaded của generic Repository.

**A. Luồng xử lý**

```text
repo.isLoaded()
↓
return bool
```

**B. Giải thích code**

```cpp
return repo.isLoaded();
```

Hàm không đọc file; file rỗng vẫn có thể loaded true với `count() == 0`.

## 9. Quản lý bộ nhớ

### 9.1. Quyền sở hữu đối tượng

`Repository<Member>` sở hữu `Member*` sau add thành công hoặc load thành công, giải phóng chúng khi clear/hủy. `membersByPhone` chỉ giữ con trỏ mượn; map không xóa Member. `findById`, `findByPhone`, `getAt` trả con trỏ mượn. Nếu add thất bại, caller vẫn sở hữu đối tượng và phải giải phóng khi thích hợp. `removeById` luôn trả `false` nên không giải phóng đối tượng.

### 9.2. Con trỏ mượn và thứ tự hủy

Khi `load()` phát hiện phone trùng hoặc thao tác map ném, `catch` xóa chỉ mục trước rồi mới gọi `repo.clear()` để không giữ con trỏ treo. Nếu `add()` thất bại hoặc ném sau khi tạo key phone, hàm xóa key trước khi trả/throw; caller tiếp tục sở hữu Member. `update()` đổi phone giữ cùng địa chỉ đối tượng và đổi khóa map; khi setter ném chỉ hoàn tác key mới, **không** phục hồi toàn bộ thuộc tính. Khi hủy đối tượng Repository, map là thành viên khai báo sau `repo`, nên map bị hủy trước `repo`; không có quyền sở hữu Member trong map. Quan hệ `Member`–`Invoice` không đồng nghĩa sở hữu Invoice.

### 9.3. Destructor và giới hạn rollback

`MemberRepository` không khai báo destructor riêng. Destructor mặc định hủy `membersByPhone` rồi `repo` theo thứ tự ngược khai báo; generic Repository giải phóng Member mà nó sở hữu. Destructor ảo của `IRepository<Member>` cho phép xóa qua con trỏ interface. Khi `load()` lỗi sau khi nạp, `membersByPhone.clear()` rồi `repo.clear()` phục hồi trạng thái rỗng. Khi `update()` có exception giữa các setter, việc gỡ key mới không phục hồi toàn bộ Model; cần xem đây là giới hạn dữ liệu chứ không mô tả như transaction nguyên tử.

## 10. Áp dụng DIP

### 10.1. Trước khi refactor

Mã MemberRepository trước refactor đã khai báo kế thừa `IRepository<Member>` nhưng giữ thêm hai map ID và phone. Map ID lặp việc tra ID của `Repository<Member>`; mỗi lần load/add phải đồng bộ cả hai map và danh sách gốc. Cấu trúc này làm tăng số nơi cần rollback khi lỗi.

### 10.2. Sau khi refactor Repository

Header override đủ chín hàm chung của `IRepository<Member>`: `load`, `save`, `add`, `update`, `removeById`, `findById`, `getAt`, `count`, `isLoaded`. `findByPhone` là API riêng, không thêm vào interface chung. Generic `Repository<Member>` vẫn nằm trong lớp concrete theo composition. Việc Service phụ thuộc interface là công việc riêng; tài liệu này không đánh dấu Service đã hoàn tất DIP.

Vì `MemberService` còn phụ thuộc concrete trong mã hiện tại, công việc DIP của Service chưa được nhận là hoàn tất. Map phone không thay generic Repository hay interface; nó chỉ là một chỉ mục phụ của concrete Repository.

## 11. Phạm vi refactor đã triển khai

### 11.1. Header và cấu trúc dữ liệu

- Giữ `IRepository<Member>` và `Repository<Member>`; bỏ thành viên `membersById`, giữ `membersByPhone` là con trỏ mượn.
- Header tự include `<string>`, `<unordered_map>` và dùng đầy đủ `std::`; không thêm public API hoặc thay schema.

### 11.2. Định nghĩa, validation và rollback

- Dùng `Validator::isValidPhone()` đã có, bỏ helper cục bộ với cùng logic; giữ các điều kiện validation còn lại.
- Chuẩn hóa header và `.cpp` theo thứ tự, comment, định dạng và namespace của MovieRepository.
- `save`, `findById` chuyển tiếp trực tiếp; `findByPhone`, `getAt`, `count`, `isLoaded`, constructor, callback, serializer và chính sách không xóa giữ logic hiện tại.
- `stringToMember` kiểm points không rỗng, chỉ chứa chữ số và gọi `stoi(fields[3])` trong `try/catch`, không kiểm `pos` vì vòng ký tự đã loại hậu tố sai.
- `load`, `add`, `update` đã triển khai theo Markdown đã chốt, với giới hạn rollback được giải thích ở mục 8.5.

### 11.3. Thành phần và quy tắc giữ nguyên

Không thay Model `Member`, `IRepository<T>`, `Repository<T>`, định dạng TXT, cách cấp ID, Permission hay quy tắc nghiệp vụ điểm. `removeById()` vẫn luôn từ chối xóa để bảo vệ lịch sử. Validator chung đã có `isValidPhone()` đúng logic trước đó; refactor chỉ tái sử dụng hàm này. Service không được đánh dấu hoàn tất trong phạm vi tài liệu này.

## 12. Kế hoạch kiểm thử

Đã kiểm tra cú pháp C++17 cho các `.cpp` trong `src/` (trừ `main.cpp`), nhưng **chưa chạy test cho bản refactor**. Khi viết và chạy test, biên dịch một file test mỗi lần với các `.cpp` phù hợp trong `src/`, trừ `main.cpp`; dùng file dữ liệu test riêng, không ghi đè dữ liệu thật.

### 12.1. Load và parse

- [ ] Load file hợp lệ, rỗng, thiếu; trạng thái, count và truy vấn trước/sau load, load hai lần.
- [ ] File sai số trường, ID sai/trùng, tên/phone sai, phone trùng; kiểm lỗi, map và repo cùng rỗng sau rollback, sửa file rồi retry.
- [ ] Points rỗng, dấu `+/-`, dấu cách, chữ, ký tự điều khiển, giá trị tràn `int` và các biên `0`/`INT_MAX` theo schema.

### 12.2. Add

- [ ] Thêm hợp lệ; từ chối chưa load, null, ID trùng, con trỏ trùng, phone trùng, điểm ban đầu khác 0 và từng thuộc tính sai.
- [ ] Kiểm `membersByPhone` và `repo` nhất quán khi `repo.add()` trả false hoặc ném; kiểm caller giữ quyền sở hữu khi add thất bại.

### 12.3. Update

- [ ] Đổi tên với phone giữ nguyên; đổi phone; kiểm khóa cũ biến mất, khóa mới trỏ cùng địa chỉ Member.
- [ ] Từ chối ID thiếu, phone thuộc Member khác, điểm thay đổi và dữ liệu sai; xác minh ID, địa chỉ, count và liên kết Invoice không đổi ở nhánh thành công.
- [ ] Kiểm giới hạn khi setter ném: key mới bị gỡ, nhưng không khẳng định tên/phone được hoàn tác; ghi nhận trạng thái Model và map thực tế.

### 12.4. Remove, truy vấn và DIP

- [ ] `removeById()` luôn false và không đổi dữ liệu; `findById` qua generic repo, `findByPhone` qua map, `getAt` ở chỉ số hợp lệ/âm/vượt phạm vi.
- [ ] Biên dịch và gọi chín phương thức chung qua `IRepository<Member>&` hoặc `IRepository<Member>*`; kiểm destructor ảo.

### 12.5. Save và bộ nhớ

- [ ] Save trước load, save hợp lệ rồi load lại; đối chiếu bốn trường TXT và điểm.
- [ ] Lỗi serializer/ghi file; con trỏ mượn không bị caller xóa; clear/hủy không tạo con trỏ treo trong map.

Chưa ghi PASS/FAIL vì chưa chạy bài test liên quan cho bản refactor.

## 13. Tiến độ

- [x] Đọc quy chuẩn, thiết kế DIP, validation, nghiệp vụ, định dạng dữ liệu và mẫu Movie/User.
- [x] Rà soát mã MemberRepository và Validator hiện tại.
- [x] Ghi thiết kế dự kiến cho cấu trúc một map và các hàm đã rà soát.
- [x] Bổ sung mã dự kiến và giải thích `load()`, `add()`, `update()` theo thiết kế đã thống nhất.
- [x] Chốt quy tắc chuỗi points không rỗng, chỉ gồm chữ số `0`–`9` và dùng `stoi(fields[3])`.
- [x] Review toàn bộ tài liệu và xác nhận **CHỐT MARKDOWN**.
- [x] Triển khai `.h/.cpp`, kiểm tra cú pháp C++17 và đối chiếu mã với Markdown.
- [ ] Viết và chạy test liên quan, ghi nhận kết quả thực tế.

## 14. Kết luận

Thiết kế giữ `IRepository<Member>`, `Repository<Member>` và chỉ mục `membersByPhone`, bỏ map ID. `Validator::isValidPhone()` thay helper cục bộ cùng logic. Quy tắc points và mã `load()`, `add()`, `update()` đã **CHỐT MARKDOWN** và triển khai; rollback ở `update()` chỉ hoàn tác key mới. Hai file MemberRepository khớp mã ở mục 5 và đã qua kiểm tra cú pháp C++17; **chưa chạy test** cho bản refactor.
