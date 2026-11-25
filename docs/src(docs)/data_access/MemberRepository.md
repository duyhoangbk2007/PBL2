# MemberRepository

## 1. Tổng quan

`MemberRepository` là lớp thuộc tầng Data Access, chịu trách nhiệm quản lý dữ liệu thành viên trong hệ thống PBL2.

Lớp kế thừa `IRepository<Member>` để triển khai hợp đồng chung của Repository, đồng thời sử dụng `Repository<Member>` thông qua composition để quản lý danh sách đối tượng và đọc ghi file TXT.

MemberRepository chịu trách nhiệm:

- Kiểm tra tính hợp lệ của dữ liệu thành viên.
- Chuyển đổi dữ liệu giữa đối tượng Member và chuỗi TXT.
- Thêm, cập nhật và truy vấn thành viên.
- Kiểm tra tính duy nhất của số điện thoại.
- Quản lý chỉ mục tìm kiếm thành viên theo số điện thoại.
- Triển khai đầy đủ các hàm của `IRepository<Member>`.

MemberRepository không trực tiếp xử lý phân quyền, sinh mã thành viên hoặc các nghiệp vụ tích điểm thuộc tầng Business Logic.

**File liên quan:**

- `src/data_access/IRepository.h`
- `src/data_access/Repository.h`
- `src/data_access/MemberRepository.h`
- `src/data_access/MemberRepository.cpp`
- `src/models/Member.h`
- `src/utils/Validator.h`
- `src/utils/Validator.cpp`

**Trạng thái:** CHỐT MARKDOWN; đã triển khai mã nguồn `.h` và `.cpp` theo thiết kế refactor. Chưa xác nhận kết quả kiểm thử.

## 2. Kiến trúc

### 2.1. Quan hệ giữa các lớp

```text
MemberService (du kien)
     |
     | Dependency Injection
     v
IRepository<Member>
     ^
     | Public Inheritance
     |
MemberRepository
     |
     | Composition
     v
Repository<Member>
     |
     v
DoublyLinkedList<Member*>
```

Ngoài ra, MemberRepository sử dụng `unordered_map` để hỗ trợ tìm kiếm thành viên theo số điện thoại.

```text
MemberRepository
     |
     +-- Repository<Member>
     |        |
     |        v
     |   DoublyLinkedList<Member*>
     |
     +-- membersByPhone
              |
              v
         Member* (con tro muon)
```

### 2.2. Quan hệ với IRepository

MemberRepository kế thừa công khai từ `IRepository<Member>`.

```cpp
class MemberRepository : public IRepository<Member>
```

MemberRepository phải triển khai đầy đủ 9 hàm thuần ảo được khai báo trong interface.

Các hàm ghi đè được khai báo bằng từ khóa `override`.

Ngoài các hàm của interface, MemberRepository cung cấp thêm hàm `findByPhone()` để tìm kiếm thành viên theo số điện thoại.

Hàm `removeById()` vẫn được triển khai để đáp ứng interface nhưng không hỗ trợ xóa thành viên.

### 2.3. Quan hệ với Repository

MemberRepository chứa một đối tượng `Repository<Member>`.

```cpp
Repository<Member> repo;
```

Đây là quan hệ composition.

`Repository<Member>` chịu trách nhiệm:

- Quản lý danh sách thành viên.
- Quản lý bộ nhớ của các đối tượng Member.
- Đọc và ghi file TXT.
- Thêm và tìm kiếm đối tượng.
- Quản lý trạng thái nạp dữ liệu.

MemberRepository bổ sung các logic riêng của Member mà Repository chung không thể xử lý.

Theo `docs/architecture/validation_rules.md`, `Repository<Member>` kiểm tra trạng thái loaded và các điều kiện lưu trữ chung; MemberRepository kiểm tra thuộc tính Member, tính duy nhất của số điện thoại và các ràng buộc riêng khi thêm hoặc cập nhật thành viên.

### 2.4. Chỉ mục tìm kiếm theo số điện thoại

MemberRepository sử dụng một `unordered_map`.

```cpp
std::unordered_map<std::string, Member*> membersByPhone;
```

Trong đó:

- Key: Số điện thoại của thành viên.
- Value: Con trỏ đến đối tượng Member.

Ví dụ:

```text
0912345678 --> Member MEM0001
0987654321 --> Member MEM0002
0901234567 --> Member MEM0003
```

Việc sử dụng `unordered_map` giúp tìm kiếm thành viên theo số điện thoại nhanh hơn so với duyệt toàn bộ danh sách.

MemberRepository không sử dụng `membersById` vì chức năng tìm kiếm theo mã đã được cung cấp bởi `Repository<Member>`.

Map chỉ lưu con trỏ đến các đối tượng đang được Repository quản lý, không tạo thêm đối tượng Member.

Chỉ mục phải được đồng bộ khi thực hiện:

- `load()`.
- `add()`.
- `update()`.

## 3. Định dạng dữ liệu

Dữ liệu thành viên được lưu dưới dạng TXT.

Mỗi dòng chứa một thành viên với 4 trường, phân cách bằng ký tự `|`.

```text
memberId|fullName|phone|points
```

| STT | Trường | Kiểu dữ liệu | Ý nghĩa |
|---|---|---|---|
| 1 | memberId | string | Mã thành viên |
| 2 | fullName | string | Họ và tên thành viên |
| 3 | phone | string | Số điện thoại |
| 4 | points | int | Điểm tích lũy |

Ví dụ:

```text
MEM0001|Nguyen Van A|0912345678|0
MEM0002|Tran Van B|0987654321|150
```

Quy định:

- Mã thành viên sử dụng tiền tố `MEM`.
- Họ và tên phải đáp ứng quy định của Validator.
- Số điện thoại phải gồm đúng 10 chữ số và bắt đầu bằng 0.
- Số điện thoại không được trùng giữa các thành viên.
- Điểm tích lũy không được âm.
- Trường points trong file TXT không được rỗng và chỉ chứa chữ số từ 0 đến 9.
- Thành viên mới phải có điểm tích lũy bằng 0 khi gọi `add()`.

Số điện thoại được lưu dưới dạng `string` thay vì kiểu số nguyên để giữ nguyên chữ số 0 ở đầu.

Quá trình refactor không thay đổi định dạng dữ liệu hiện tại.

## 4. Thiết kế lớp

### 4.1. Thuộc tính

| Thuộc tính | Kiểu dữ liệu | Phạm vi | Chức năng |
|---|---|---|---|
| repo | Repository<Member> | private | Quản lý dữ liệu thành viên |
| membersByPhone | unordered_map<string, Member*> | private | Chỉ mục tìm kiếm theo số điện thoại |

MemberRepository không khai báo lại danh sách thành viên, đường dẫn file hoặc trạng thái nạp dữ liệu vì các thuộc tính này đã được quản lý bởi `Repository<Member>`.

`membersByPhone` chỉ hỗ trợ tìm kiếm và kiểm tra trùng số điện thoại, không chịu trách nhiệm quản lý bộ nhớ của các đối tượng Member.

### 4.2. Các hàm private

| Hàm | Kiểu trả về | Chức năng |
|---|---|---|
| getMemberId(Member* member) | string | Lấy mã thành viên |
| stringToMember(const string& line) | Member* | Chuyển chuỗi TXT thành Member |
| memberToString(Member* member) | string | Chuyển Member thành chuỗi TXT |
| isValidMember(const Member* member) | bool | Kiểm tra tính hợp lệ của thành viên |

Các hàm trên được khai báo `static` vì không cần truy cập trực tiếp vào thuộc tính của MemberRepository.

Ba hàm `getMemberId()`, `stringToMember()` và `memberToString()` được cung cấp cho `Repository<Member>` thông qua con trỏ hàm.

Việc kiểm tra số điện thoại được thực hiện thông qua `Validator::isValidPhone()`.

### 4.3. Các hàm public

| Hàm | Kiểu trả về | Chức năng | Override |
|---|---|---|---|
| MemberRepository(const string& filePath) | Không có | Constructor | Không |
| load() | bool | Nạp dữ liệu | Có |
| save() | bool | Lưu dữ liệu | Có |
| add(Member* member) | bool | Thêm thành viên | Có |
| update(const Member& newValues) | bool | Cập nhật thành viên | Có |
| removeById(const string& memberId) | bool | Không hỗ trợ xóa thành viên | Có |
| findById(const string& memberId) | Member* | Tìm thành viên theo mã | Có |
| findByPhone(const string& phone) | Member* | Tìm thành viên theo số điện thoại | Không |
| getAt(int index) | Member* | Lấy thành viên theo vị trí | Có |
| count() | int | Đếm số lượng thành viên | Có |
| isLoaded() | bool | Kiểm tra trạng thái nạp | Có |

Hàm `findByPhone()` là hàm riêng của MemberRepository, không thuộc `IRepository<Member>`.

Hàm `removeById()` luôn trả về `false` vì hệ thống không hỗ trợ xóa thành viên nhằm bảo toàn lịch sử giao dịch.
## 5. Mã nguồn

### 5.1. MemberRepository.h

**Đường dẫn:** `src/data_access/MemberRepository.h`

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

### 5.2. MemberRepository.cpp

**Đường dẫn:** `src/data_access/MemberRepository.cpp`

MemberRepository sử dụng Repository chung để quản lý danh sách thành viên, đồng thời duy trì chỉ mục số điện thoại trong các thao tác load, add và update.

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

## 6. Giải thích các thư viện và từ khóa

### 6.1. Thư viện string

```cpp
#include <string>
```

Cung cấp kiểu dữ liệu `string` và các hàm xử lý chuỗi.

Các hàm được sử dụng:

- `find()`: Tìm vị trí xuất hiện của ký tự.
- `substr()`: Lấy một phần của chuỗi.
- `empty()`: Kiểm tra chuỗi có rỗng hay không.
- `stoi()`: Chuyển chuỗi thành số nguyên.
- `to_string()`: Chuyển số thành chuỗi.

Trong MemberRepository, các hàm này được sử dụng chủ yếu để chuyển đổi dữ liệu giữa đối tượng Member và dòng TXT.

### 6.2. Thư viện vector

```cpp
#include <vector>
```

`vector` là cấu trúc dữ liệu có khả năng tự thay đổi kích thước.

Trong MemberRepository, vector được sử dụng để lưu các trường dữ liệu sau khi tách một dòng TXT.

```cpp
vector<string> fields;
```

Các hàm được sử dụng:

- `push_back()`: Thêm phần tử vào cuối vector.
- `size()`: Lấy số lượng phần tử.
- `fields[index]`: Truy cập phần tử theo vị trí.

Ví dụ, sau khi tách dòng:

```text
MEM0001|Nguyen Van A|0912345678|150
```

Vector chứa:

| Vị trí | Giá trị |
|---|---|
| fields[0] | MEM0001 |
| fields[1] | Nguyen Van A |
| fields[2] | 0912345678 |
| fields[3] | 150 |

Vector chỉ được sử dụng tạm thời trong quá trình chuyển đổi dữ liệu.

Danh sách thành viên chính vẫn được quản lý bằng DoublyLinkedList thông qua Repository.

### 6.3. Thư viện unordered_map

```cpp
#include <unordered_map>
```

`unordered_map` là cấu trúc dữ liệu lưu trữ các phần tử dưới dạng cặp khóa và giá trị (key-value).

```cpp
std::unordered_map<std::string, Member*> membersByPhone;
```

Trong đó:

- `string`: Kiểu dữ liệu của khóa, tương ứng với số điện thoại.
- `Member*`: Kiểu dữ liệu của giá trị, tương ứng với con trỏ Member.

Mỗi khóa trong map là duy nhất.

`unordered_map` sử dụng bảng băm (hash table), giúp tìm kiếm, thêm và xóa phần tử với độ phức tạp trung bình O(1).

Các phương thức và toán tử được sử dụng:

**Toán tử []**

```cpp
membersByPhone[phone] = member;
```

Thêm một cặp số điện thoại và con trỏ Member vào map.

Nếu khóa đã tồn tại, giá trị tương ứng sẽ được cập nhật. Vì vậy, cần kiểm tra trùng số điện thoại trước khi thực hiện thao tác này.

**Hàm find()**

```cpp
auto it = membersByPhone.find(phone);
```

Tìm phần tử có khóa tương ứng với số điện thoại.

Hàm trả về iterator đến phần tử nếu tìm thấy, ngược lại trả về `membersByPhone.end()`.

**Hàm end()**

```cpp
if (it == membersByPhone.end())
```

`end()` trả về iterator biểu thị vị trí sau phần tử cuối cùng.

Điều kiện trên được sử dụng để xác định số điện thoại không tồn tại trong map.

**Hàm erase()**

```cpp
membersByPhone.erase(phone);
```

Xóa phần tử có khóa tương ứng với số điện thoại.

Hàm được sử dụng khi thay đổi số điện thoại hoặc rollback thao tác thất bại.

**Hàm clear()**

```cpp
membersByPhone.clear();
```

Xóa toàn bộ phần tử trong map.

Do map chỉ lưu con trỏ, `erase()` và `clear()` không giải phóng các đối tượng Member mà những con trỏ này trỏ đến.

### 6.4. Iterator và từ khóa auto

```cpp
auto it = membersByPhone.find(phone);
```

Iterator là đối tượng hỗ trợ truy cập các phần tử trong container.

Từ khóa `auto` cho phép trình biên dịch tự xác định kiểu dữ liệu của biến dựa trên giá trị khởi tạo.

Câu lệnh trên tương đương với:

```cpp
unordered_map<string, Member*>::iterator it =
    membersByPhone.find(phone);
```

Mỗi phần tử trong `unordered_map` gồm hai thành phần:

- `it->first`: Khóa của phần tử.
- `it->second`: Giá trị của phần tử.

Trong MemberRepository:

```cpp
return it->second;
```

Trả về con trỏ Member tương ứng với số điện thoại đã tìm thấy.

Cần kiểm tra iterator khác `end()` trước khi truy cập `first` hoặc `second`.

### 6.5. Thư viện stdexcept

```cpp
#include <stdexcept>
```

Cung cấp các lớp exception tiêu chuẩn của C++.

MemberRepository sử dụng `runtime_error` để thông báo lỗi khi dữ liệu không hợp lệ.

Ví dụ:

```cpp
throw runtime_error("Sai so luong truong Member");
```

Exception được truyền về lớp gọi để xử lý.

### 6.6. Cơ chế try-catch

Khối `try-catch` được sử dụng để xử lý ngoại lệ phát sinh trong quá trình thực hiện chương trình.

Ví dụ:

```cpp
try
{
    points = stoi(fields[3]);
}
catch (const exception&)
{
    throw runtime_error("Points Member khong hop le");
}
```

Nếu quá trình chuyển đổi points xảy ra lỗi, khối `catch` sẽ bắt ngoại lệ và ném một `runtime_error` với thông báo phù hợp.

Ngoài ra, MemberRepository sử dụng:

```cpp
catch (...)
{
    membersByPhone.clear();
    repo.clear();
    throw;
}
```

Trong đó:

- `catch (...)`: Bắt mọi loại ngoại lệ.
- `throw;`: Ném lại ngoại lệ đang được xử lý.

Cơ chế này giúp khôi phục trạng thái dữ liệu trước khi truyền lỗi về lớp gọi.

Việc khôi phục trạng thái khi thao tác thất bại được gọi là rollback.

### 6.7. Từ khóa static

```cpp
static std::string getMemberId(Member* member);
```

Hàm thành viên static thuộc về lớp thay vì thuộc về một đối tượng cụ thể.

Hàm static không có con trỏ `this` và không thể trực tiếp truy cập các thuộc tính không static của lớp.

Các hàm `getMemberId()`, `stringToMember()` và `memberToString()` được khai báo static để có thể truyền vào Repository dưới dạng con trỏ hàm thông thường.

Hàm `isValidMember()` cũng được khai báo static vì chỉ kiểm tra đối tượng Member được truyền vào.

### 6.8. Từ khóa override

```cpp
bool load() override;
```

Từ khóa `override` xác nhận rằng hàm của MemberRepository đang ghi đè một hàm ảo trong `IRepository<Member>`.

Nếu chữ ký không tương thích, trình biên dịch sẽ báo lỗi.

Từ khóa `override` chỉ được sử dụng tại phần khai báo hàm trong lớp, không sử dụng tại phần định nghĩa hàm trong file `.cpp`.

### 6.9. Public inheritance

```cpp
class MemberRepository : public IRepository<Member>
```

MemberRepository kế thừa công khai từ interface.

Nhờ đó, có thể sử dụng đối tượng MemberRepository thông qua tham chiếu hoặc con trỏ `IRepository<Member>`.

Ví dụ:

```cpp
MemberRepository memberRepo("data/members.txt");

IRepository<Member>& repo = memberRepo;

repo.load();
```

Hàm được thực hiện là `MemberRepository::load()`.

Hàm `findByPhone()` không được khai báo trong `IRepository<Member>`, vì vậy không thể gọi hàm này trực tiếp thông qua tham chiếu interface.
## 7. Giải thích các hàm private

### 7.1. Hàm isValidMember()

```cpp
static bool isValidMember(const Member* member);
```

**Chức năng:**

Kiểm tra toàn bộ dữ liệu của một đối tượng Member.

**Tham số:**

- `member`: Con trỏ đến đối tượng Member cần kiểm tra.

**Giá trị trả về:**

- `true`: Dữ liệu hợp lệ.
- `false`: Dữ liệu không hợp lệ.

**Quy trình xử lý:**

1. Kiểm tra con trỏ khác `nullptr`.
2. Kiểm tra mã thành viên bằng `Validator::isValidId()`.
3. Kiểm tra họ và tên bằng `Validator::isValidText()`.
4. Kiểm tra số điện thoại bằng `Validator::isValidPhone()`.
5. Kiểm tra điểm tích lũy không âm.
6. Trả về `true` nếu tất cả điều kiện đều hợp lệ.

Hàm nhận `const Member*` vì chỉ đọc dữ liệu, không thay đổi thông tin thành viên.

Hàm được sử dụng trong:

- `stringToMember()`.
- `memberToString()`.
- `add()`.
- `update()`.

Việc sử dụng chung một hàm kiểm tra giúp tránh lặp lại logic validation.

Hàm không kiểm tra trùng mã hoặc trùng số điện thoại vì đây là các điều kiện phụ thuộc vào danh sách thành viên hiện tại.

### 7.2. Hàm getMemberId()

```cpp
static string getMemberId(Member* member);
```

**Chức năng:**

Lấy khóa định danh của đối tượng Member.

**Giá trị trả về:**

- Mã thành viên nếu con trỏ khác `nullptr`.
- Chuỗi rỗng nếu con trỏ là `nullptr`.

Repository chung không biết mỗi Model sử dụng hàm getter nào để lấy ID.

Vì vậy, MemberRepository cung cấp hàm `getMemberId()` cho `Repository<Member>`.

Repository sử dụng hàm này để xác định khóa, kiểm tra trùng khóa và tìm kiếm đối tượng.

### 7.3. Hàm stringToMember()

```cpp
static Member* stringToMember(const string& line);
```

**Chức năng:**

Chuyển một dòng dữ liệu TXT thành đối tượng Member.

Hàm này còn được gọi là hàm parse.

**Tham số:**

- `line`: Dòng dữ liệu thành viên đọc từ file TXT.

**Giá trị trả về:**

Con trỏ đến đối tượng Member mới được tạo.

Nếu dữ liệu không hợp lệ, hàm ném `runtime_error`.

**Bước 1: Tách các trường dữ liệu**

```cpp
vector<string> fields;
size_t start = 0;
```

- `fields`: Lưu các trường sau khi tách.
- `start`: Vị trí bắt đầu của trường hiện tại.

```cpp
size_t pos = line.find('|', start);
```

Tìm ký tự `|` đầu tiên kể từ vị trí `start`.

`size_t` là kiểu số nguyên không dấu thường được sử dụng để biểu diễn kích thước hoặc vị trí trong chuỗi và các container.

```cpp
if (pos == string::npos)
{
    fields.push_back(line.substr(start));
    break;
}
```

`string::npos` là giá trị đặc biệt biểu thị không tìm thấy vị trí phù hợp.

Khi không còn ký tự `|`, lấy phần chuỗi còn lại làm trường cuối cùng và kết thúc vòng lặp.

Nếu vẫn tìm thấy ký tự phân cách:

```cpp
fields.push_back(line.substr(start, pos - start));
start = pos + 1;
```

- `substr(start, pos - start)` lấy nội dung của trường hiện tại.
- `start = pos + 1` chuyển sang trường tiếp theo.

**Bước 2: Kiểm tra số lượng trường**

```cpp
if (fields.size() != 4)
{
    throw runtime_error("Sai so luong truong Member");
}
```

Mỗi dòng dữ liệu Member phải có đúng 4 trường.

Nếu thiếu hoặc thừa trường, dữ liệu được xem là không hợp lệ.

**Bước 3: Kiểm tra định dạng points**

```cpp
if (fields[3].empty())
{
    throw runtime_error("Points Member khong hop le");
}
```

Không cho phép trường points là chuỗi rỗng.

```cpp
for (char c : fields[3])
{
    if (c < '0' || c > '9')
    {
        throw runtime_error("Points Member khong hop le");
    }
}
```

Duyệt từng ký tự để bảo đảm chuỗi points chỉ chứa chữ số từ `0` đến `9`.

Ví dụ:

| Giá trị | Kết quả |
|---|---|
| `"0"` | Hợp lệ |
| `"150"` | Hợp lệ |
| `"-10"` | Không hợp lệ |
| `"+10"` | Không hợp lệ |
| `"10.5"` | Không hợp lệ |
| `"100abc"` | Không hợp lệ |
| `" 100"` | Không hợp lệ |

Việc kiểm tra trước giúp loại bỏ các chuỗi không đúng định dạng trước khi gọi `stoi()`.

**Bước 4: Chuyển đổi points thành số nguyên**

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

Hàm `stoi()` chuyển chuỗi points thành số nguyên.

Do chuỗi đã được kiểm tra chỉ chứa chữ số, không cần sử dụng tham số thứ hai của `stoi()` để kiểm tra số ký tự đã xử lý.

Khối `try-catch` xử lý lỗi chuyển đổi, đặc biệt khi giá trị vượt quá phạm vi của kiểu `int`.

**Bước 5: Tạo đối tượng Member**

```cpp
Member* member = new Member(
    fields[0],
    fields[1],
    fields[2],
    points
);
```

Sử dụng `new Member(...)` để cấp phát đối tượng trên vùng nhớ động.

**Bước 6: Kiểm tra đối tượng**

```cpp
if (!isValidMember(member))
{
    delete member;
    throw runtime_error("Du lieu Member khong hop le");
}
```

Nếu dữ liệu không hợp lệ, phải giải phóng đối tượng vừa tạo trước khi ném exception để tránh rò rỉ bộ nhớ.

Nếu hợp lệ, trả về con trỏ để Repository tiếp nhận và quản lý.

### 7.4. Hàm memberToString()

```cpp
static string memberToString(Member* member);
```

**Chức năng:**

Chuyển một đối tượng Member thành một dòng dữ liệu TXT.

Hàm này còn được gọi là hàm serialize.

**Tham số:**

- `member`: Con trỏ đến đối tượng cần chuyển đổi.

**Giá trị trả về:**

Chuỗi chứa đầy đủ 4 trường dữ liệu của Member.

**Quy trình xử lý:**

1. Kiểm tra tính hợp lệ của Member.
2. Lấy dữ liệu thông qua các getter.
3. Chuyển điểm tích lũy thành chuỗi bằng `to_string()`.
4. Ghép các trường bằng ký tự `|`.
5. Trả về chuỗi kết quả.

```cpp
return member->getMemberId() + "|" +
       member->getFullName() + "|" +
       member->getPhone() + "|" +
       to_string(member->getPoints());
```

Hàm `to_string()` được sử dụng vì points có kiểu `int`, trong khi dữ liệu cần ghi xuống file có kiểu `string`.

MemberRepository chỉ tạo chuỗi dữ liệu. Việc ghi chuỗi xuống file do Repository thực hiện.

## 8. Giải thích các hàm public

### 8.1. Constructor

```cpp
MemberRepository::MemberRepository(const string& filePath)
    : repo(filePath, getMemberId, stringToMember, memberToString)
{
}
```

**Chức năng:**

Khởi tạo MemberRepository và đối tượng `Repository<Member>` bên trong.

**Tham số:**

- `filePath`: Đường dẫn file lưu dữ liệu thành viên.

Constructor sử dụng danh sách khởi tạo thành viên (member initializer list) để khởi tạo trực tiếp thuộc tính `repo`.

Repository nhận 4 tham số:

| Tham số | Chức năng |
|---|---|
| filePath | Đường dẫn file dữ liệu |
| getMemberId | Hàm lấy khóa của Member |
| stringToMember | Hàm chuyển chuỗi thành Member |
| memberToString | Hàm chuyển Member thành chuỗi |

Ba hàm cuối được truyền dưới dạng con trỏ hàm.

Thuộc tính `membersByPhone` được khởi tạo mặc định là một map rỗng.

Constructor không trực tiếp nạp dữ liệu. Việc nạp dữ liệu được thực hiện thông qua `load()`.

### 8.2. Hàm load()

```cpp
bool load() override;
```

**Chức năng:**

Nạp dữ liệu thành viên từ file TXT vào bộ nhớ và xây dựng chỉ mục tìm kiếm theo số điện thoại.

**Giá trị trả về:**

- `true`: Nạp dữ liệu thành công.
- `false`: Nạp dữ liệu không thành công hoặc không được phép nạp lại.

Nếu dữ liệu không hợp lệ hoặc xảy ra ngoại lệ trong quá trình xây dựng chỉ mục, hàm truyền exception về lớp gọi.

**Quy trình xử lý:**

Bước 1: Nạp dữ liệu thông qua Repository chung.

```cpp
if (!repo.load())
{
    return false;
}
```

Repository sử dụng `stringToMember()` để chuyển từng dòng TXT thành đối tượng Member.

Repository đồng thời kiểm tra các điều kiện lưu trữ chung, bao gồm mã thành viên không được trùng.

Nếu `repo.load()` trả về `false`, hàm kết thúc ngay.

Bước 2: Duyệt danh sách thành viên.

```cpp
for (int i = 0; i < repo.count(); i++)
{
    Member* member = repo.getAt(i);
```

Duyệt từng đối tượng Member đã được nạp để xây dựng chỉ mục số điện thoại.

Không cần kiểm tra lại dữ liệu Member vì các đối tượng đã được kiểm tra trong quá trình parse.

Bước 3: Kiểm tra trùng số điện thoại.

```cpp
if (findByPhone(member->getPhone()) != nullptr)
{
    throw runtime_error("Trung phone trong file Member");
}
```

Nếu số điện thoại đã tồn tại trong map, dữ liệu được xem là không hợp lệ.

Repository chung chỉ kiểm tra trùng ID nên MemberRepository phải tự kiểm tra điều kiện này.

Bước 4: Thêm thành viên vào chỉ mục.

```cpp
membersByPhone[member->getPhone()] = member;
```

Sử dụng số điện thoại làm khóa và con trỏ Member làm giá trị.

Map chỉ lưu con trỏ đến đối tượng đã được Repository quản lý, không tạo thêm đối tượng Member.

Bước 5: Xử lý ngoại lệ.

```cpp
catch (...)
{
    membersByPhone.clear();
    repo.clear();
    throw;
}
```

Nếu xảy ra lỗi trong quá trình xây dựng chỉ mục:

- `membersByPhone.clear()`: Xóa toàn bộ chỉ mục số điện thoại.
- `repo.clear()`: Giải phóng danh sách Member và đặt lại trạng thái loaded.
- `throw;`: Ném lại ngoại lệ ban đầu.

Chỉ mục được xóa trước khi giải phóng các đối tượng Member để không giữ lại các con trỏ không hợp lệ.

Bước 6: Trả về kết quả.

```cpp
return true;
```

Sau khi duyệt thành công toàn bộ danh sách, dữ liệu Member và chỉ mục số điện thoại đã được đồng bộ.

**Luồng hoạt động chi tiết:**

```text
load()
    |
    v
repo.load()
    |
    +-- Trả false --> return false
    |
    +-- Ném exception --> Truyền lỗi về lớp gọi
    |
    v
Duyệt từng Member
    |
    v
Kiểm tra trùng số điện thoại
    |
    +-- Trùng --> Ném exception
    |
    v
Thêm vào membersByPhone
    |
    +-- Còn Member --> Tiếp tục duyệt
    |
    v
return true

Nếu xảy ra exception khi xây dựng map:
    |
    v
membersByPhone.clear()
    |
    v
repo.clear()
    |
    v
throw
```

**Lưu ý:**

- Không cần xây dựng chỉ mục theo mã thành viên.
- Không cần kiểm tra lại toàn bộ thuộc tính Member sau khi nạp.
- Nếu xây dựng chỉ mục thất bại, không giữ lại danh sách được nạp dở dang.
- Có thể thực hiện lại `load()` sau khi dữ liệu lỗi đã được sửa và Repository đã rollback.

### 8.3. Hàm save()

```cpp
bool MemberRepository::save()
{
    return repo.save();
}
```

**Chức năng:**

Lưu danh sách thành viên hiện tại xuống file TXT.

**Giá trị trả về:**

- `true`: Lưu thành công.
- `false`: Lưu thất bại.

Repository sử dụng `memberToString()` để chuyển từng đối tượng Member thành chuỗi trước khi ghi file.

MemberRepository không trực tiếp sử dụng `ofstream` hoặc tự triển khai cơ chế ghi file.

Chỉ mục `membersByPhone` không được ghi xuống file vì có thể xây dựng lại khi gọi `load()`.

### 8.4. Hàm add()

```cpp
bool add(Member* member) override;
```

**Chức năng:**

Thêm một thành viên mới vào Repository và cập nhật chỉ mục số điện thoại.

**Tham số:**

- `member`: Con trỏ đến đối tượng Member cần thêm.

**Giá trị trả về:**

- `true`: Thêm thành công.
- `false`: Thêm thất bại.

**Quy trình xử lý:**

Bước 1: Kiểm tra trạng thái Repository và dữ liệu Member.

```cpp
if (!repo.isLoaded() || !isValidMember(member))
{
    return false;
}
```

Kiểm tra Repository đã được nạp và đối tượng Member có dữ liệu hợp lệ.

Cần kiểm tra loaded tại đây vì MemberRepository sẽ thay đổi chỉ mục trước khi gọi `repo.add()`.

Bước 2: Kiểm tra điểm tích lũy ban đầu.

```cpp
if (member->getPoints() != 0)
{
    return false;
}
```

Thành viên mới phải có điểm tích lũy bằng 0.

Điều kiện này chỉ áp dụng khi thêm thành viên mới, không áp dụng khi nạp dữ liệu từ file.

Bước 3: Kiểm tra trùng số điện thoại.

```cpp
if (findByPhone(member->getPhone()) != nullptr)
{
    return false;
}
```

Không cho phép thêm thành viên nếu số điện thoại đã được sử dụng.

Bước 4: Thêm vào chỉ mục số điện thoại.

```cpp
string phone = member->getPhone();
membersByPhone[phone] = member;
```

Lưu số điện thoại vào biến `phone` để sử dụng khi cần rollback.

Thêm con trỏ Member vào map trước khi chuyển quyền sở hữu cho Repository.

Bước 5: Thêm thành viên vào Repository.

```cpp
try
{
    if (repo.add(member))
    {
        return true;
    }
}
```

Repository chung kiểm tra trùng con trỏ, ID rỗng và trùng ID trước khi thêm đối tượng.

MemberRepository không cần lặp lại các kiểm tra này.

Nếu thêm thành công, Repository tiếp nhận quyền sở hữu Member và hàm trả về `true`.

Bước 6: Rollback khi thêm thất bại.

```cpp
catch (...)
{
    membersByPhone.erase(phone);
    throw;
}
```

Nếu `repo.add()` ném exception, xóa chỉ mục vừa thêm rồi ném lại ngoại lệ.

```cpp
membersByPhone.erase(phone);
return false;
```

Nếu `repo.add()` trả về `false`, xóa chỉ mục vừa thêm và trả về `false`.

Việc rollback giúp tránh trường hợp map chứa thành viên chưa được Repository tiếp nhận.

**Luồng hoạt động chi tiết:**

```text
add(member)
    |
    v
Kiểm tra loaded và dữ liệu Member
    |
    +-- Không hợp lệ --> false
    |
    v
Kiểm tra points bằng 0
    |
    +-- Không hợp lệ --> false
    |
    v
Kiểm tra trùng số điện thoại
    |
    +-- Trùng --> false
    |
    v
Thêm vào membersByPhone
    |
    v
repo.add(member)
    |
    +-- Thành công --> true
    |
    +-- Trả false --> Xóa chỉ mục --> false
    |
    +-- Exception --> Xóa chỉ mục --> throw
```

**Lưu ý:**

- MemberRepository không tự sinh mã thành viên.
- Mã thành viên phải được chuẩn bị trước khi gọi `add()`.
- Khi thêm thành công, Repository tiếp nhận quyền sở hữu đối tượng.
- Khi thêm thất bại, đối tượng mới chưa được Repository tiếp nhận vẫn thuộc quyền sở hữu của bên gọi.
- Hàm không tự động lưu dữ liệu xuống file.

### 8.5. Hàm update()

```cpp
bool update(const Member& newValues) override;
```

**Chức năng:**

Cập nhật thông tin của một thành viên đã tồn tại và đồng bộ chỉ mục khi thay đổi số điện thoại.

**Tham số:**

- `newValues`: Đối tượng chứa thông tin mới của thành viên.

Sử dụng `const Member&` để tránh sao chép đối tượng và không thay đổi dữ liệu đầu vào thông qua tham số.

**Giá trị trả về:**

- `true`: Cập nhật thành công.
- `false`: Cập nhật thất bại.

**Quy trình xử lý:**

Bước 1: Kiểm tra trạng thái Repository và dữ liệu mới.

```cpp
if (!repo.isLoaded() || !isValidMember(&newValues))
{
    return false;
}
```

Toán tử `&` lấy địa chỉ của đối tượng `newValues` để truyền vào hàm `isValidMember()`.

Bước 2: Tìm thành viên cần cập nhật.

```cpp
Member* member = repo.findById(newValues.getMemberId());
if (member == nullptr)
{
    return false;
}
```

Mã thành viên trong `newValues` được sử dụng để xác định đối tượng cần cập nhật.

Nếu thành viên không tồn tại, hàm trả về `false`.

Bước 3: Kiểm tra điểm tích lũy.

```cpp
if (member->getPoints() != newValues.getPoints())
{
    return false;
}
```

Không cho phép thay đổi điểm tích lũy trực tiếp thông qua hàm cập nhật thông tin thành viên.

Việc cộng hoặc trừ điểm thuộc nghiệp vụ của tầng Business Logic.

Bước 4: Kiểm tra trùng số điện thoại.

```cpp
Member* existingMember = findByPhone(newValues.getPhone());
if (existingMember != nullptr && existingMember != member)
{
    return false;
}
```

Không cho phép sử dụng số điện thoại đã thuộc về một thành viên khác.

Nếu số điện thoại tìm được thuộc chính thành viên đang cập nhật, dữ liệu vẫn hợp lệ.

Bước 5: Xử lý trường hợp không thay đổi số điện thoại.

```cpp
string oldPhone = member->getPhone();
string newPhone = newValues.getPhone();
if (oldPhone == newPhone)
{
    member->setFullName(newValues.getFullName());
    return true;
}
```

Nếu số điện thoại không thay đổi, chỉ cần cập nhật họ và tên.

Không cần thêm hoặc xóa phần tử trong map.

Bước 6: Thêm chỉ mục số điện thoại mới và cập nhật thông tin.

```cpp
membersByPhone[newPhone] = member;
try
{
    member->setFullName(newValues.getFullName());
    member->setPhone(newPhone);
}
```

Thêm số điện thoại mới vào map trước khi cập nhật đối tượng Member.

Chỉ mục cũ vẫn được giữ lại trong quá trình cập nhật để có thể rollback nếu xảy ra ngoại lệ.

Bước 7: Xử lý ngoại lệ.

```cpp
catch (...)
{
    membersByPhone.erase(newPhone);
    throw;
}
```

Nếu setter ném exception, xóa chỉ mục số điện thoại mới và ném lại ngoại lệ.

Chỉ mục số điện thoại cũ vẫn được giữ lại.

Bước 8: Xóa chỉ mục số điện thoại cũ.

```cpp
membersByPhone.erase(oldPhone);
return true;
```

Sau khi cập nhật thành công, xóa số điện thoại cũ khỏi map.

Lúc này chỉ mục số điện thoại mới trỏ đến đối tượng Member đã được cập nhật.

**Luồng hoạt động chi tiết:**

```text
update(newValues)
    |
    v
Kiểm tra loaded và dữ liệu mới
    |
    +-- Không hợp lệ --> false
    |
    v
Tìm Member theo ID
    |
    +-- Không tồn tại --> false
    |
    v
Kiểm tra points không thay đổi
    |
    +-- Thay đổi --> false
    |
    v
Kiểm tra trùng số điện thoại
    |
    +-- Thuộc Member khác --> false
    |
    v
So sánh số điện thoại cũ và mới
    |
    +-- Giống nhau --> Cập nhật họ tên --> true
    |
    v
Thêm chỉ mục số điện thoại mới
    |
    v
Cập nhật họ tên và số điện thoại
    |
    +-- Exception --> Xóa chỉ mục mới --> throw
    |
    v
Xóa chỉ mục số điện thoại cũ
    |
    v
return true
```

**Lý do cập nhật trực tiếp đối tượng:**

Member có thể đang được các đối tượng Invoice tham chiếu.

Nếu xóa Member cũ và tạo Member mới, các con trỏ đang tham chiếu đến Member cũ có thể trở thành con trỏ không hợp lệ.

Cập nhật trực tiếp bằng setter giúp giữ nguyên địa chỉ của đối tượng và các liên kết hiện tại.

**Lưu ý:**

- Không thay đổi mã thành viên.
- Không thay đổi điểm tích lũy.
- Không thay thế đối tượng Member hiện tại bằng đối tượng mới.
- Phải đồng bộ chỉ mục khi thay đổi số điện thoại.
- Rollback hiện tại chỉ hoàn tác chỉ mục số điện thoại mới, không bảo đảm hoàn tác họ tên nếu setter họ tên đã thành công trước khi setter số điện thoại ném exception.
- Hàm không tự động lưu dữ liệu xuống file.

### 8.6. Hàm removeById()

```cpp
bool MemberRepository::removeById(const string& memberId)
{
    (void)memberId;
    return false;
}
```

**Chức năng:**

Triển khai hàm xóa theo hợp đồng của `IRepository<Member>`.

**Tham số:**

- `memberId`: Mã thành viên cần xóa.

**Giá trị trả về:**

Luôn trả về `false`.

Hệ thống không hỗ trợ xóa thành viên nhằm bảo toàn thông tin thành viên và lịch sử hóa đơn liên quan.

Do đó, hàm không thực hiện tìm kiếm, xóa đối tượng hoặc thay đổi chỉ mục số điện thoại.

```cpp
(void)memberId;
```

Biểu thức trên được sử dụng để tránh cảnh báo tham số không được sử dụng khi biên dịch.

Hàm vẫn phải được triển khai vì `removeById()` là hàm thuần ảo trong `IRepository<Member>`.

### 8.7. Hàm findById()

```cpp
Member* MemberRepository::findById(const string& memberId)
{
    return repo.findById(memberId);
}
```

**Chức năng:**

Tìm thành viên dựa trên mã thành viên.

**Tham số:**

- `memberId`: Mã thành viên cần tìm.

**Giá trị trả về:**

- Con trỏ Member nếu tìm thấy.
- `nullptr` nếu không tìm thấy.

Hàm gọi trực tiếp Repository chung vì không cần xử lý logic riêng.

MemberRepository không cần sử dụng thêm map để tìm kiếm theo mã.

Con trỏ trả về là con trỏ mượn.

Bên gọi không được tự ý giải phóng đối tượng.

### 8.8. Hàm findByPhone()

```cpp
Member* MemberRepository::findByPhone(const string& phone)
{
    auto it = membersByPhone.find(phone);
    if (it == membersByPhone.end())
    {
        return nullptr;
    }
    return it->second;
}
```

**Chức năng:**

Tìm thành viên dựa trên số điện thoại.

**Tham số:**

- `phone`: Số điện thoại cần tìm.

**Giá trị trả về:**

- Con trỏ Member nếu tìm thấy.
- `nullptr` nếu không tìm thấy.

**Quy trình xử lý:**

Bước 1: Tìm số điện thoại trong map.

```cpp
auto it = membersByPhone.find(phone);
```

Hàm `find()` trả về iterator đến phần tử có khóa tương ứng với số điện thoại.

Bước 2: Kiểm tra kết quả tìm kiếm.

```cpp
if (it == membersByPhone.end())
{
    return nullptr;
}
```

Nếu iterator bằng `end()`, số điện thoại không tồn tại trong map.

Bước 3: Trả về đối tượng Member.

```cpp
return it->second;
```

`it->second` là giá trị của phần tử trong map, tương ứng với con trỏ Member.

Hàm có độ phức tạp tìm kiếm trung bình O(1).

**Lưu ý:**

- Không cần duyệt toàn bộ danh sách Member.
- Không sử dụng toán tử `[]` để tìm kiếm vì toán tử này có thể tạo phần tử mới nếu khóa chưa tồn tại.
- Con trỏ trả về là con trỏ mượn.
- Hàm không thuộc `IRepository<Member>`.

### 8.9. Hàm getAt()

```cpp
Member* MemberRepository::getAt(int index)
{
    return repo.getAt(index);
}
```

**Chức năng:**

Lấy thành viên tại một vị trí trong danh sách.

**Tham số:**

- `index`: Vị trí của thành viên, bắt đầu từ 0.

**Giá trị trả về:**

- Con trỏ Member nếu vị trí hợp lệ.
- `nullptr` nếu vị trí không hợp lệ.

Hàm thường được sử dụng kết hợp với `count()` để duyệt danh sách.

Ví dụ:

```cpp
for (int i = 0; i < memberRepo.count(); i++)
{
    Member* member = memberRepo.getAt(i);

    if (member != nullptr)
    {
        // Xu ly thong tin thanh vien
    }
}
```

### 8.10. Hàm count()

```cpp
int MemberRepository::count()
{
    return repo.count();
}
```

**Chức năng:**

Trả về số lượng thành viên hiện có trong Repository.

Nếu danh sách rỗng, hàm trả về `0`.

Hàm lấy số lượng thành viên từ Repository chung thay vì lấy kích thước của `membersByPhone`.

Repository là nơi quản lý danh sách thành viên chính.

### 8.11. Hàm isLoaded()

```cpp
bool MemberRepository::isLoaded()
{
    return repo.isLoaded();
}
```

**Chức năng:**

Kiểm tra trạng thái nạp dữ liệu.

**Giá trị trả về:**

- `true`: Repository đã được nạp dữ liệu.
- `false`: Repository chưa được nạp dữ liệu.

MemberRepository không khai báo thuộc tính `loaded` riêng mà sử dụng trạng thái của `Repository<Member>`.

Nếu xảy ra lỗi khi xây dựng chỉ mục số điện thoại, `repo.clear()` sẽ đặt lại trạng thái loaded.
## 9. Quản lý bộ nhớ

### 9.1. Quyền sở hữu đối tượng

`Repository<Member>` chịu trách nhiệm quản lý bộ nhớ của các đối tượng Member.

Quy định:

- Đối tượng Member mới do bên gọi quản lý trước khi thêm.
- Khi `add()` thành công, quyền sở hữu được chuyển sang Repository.
- Khi `add()` thất bại, bên gọi chịu trách nhiệm giải phóng đối tượng mới chưa được tiếp nhận.
- Không giải phóng lại đối tượng đã thuộc quyền sở hữu của Repository.
- `membersByPhone` chỉ lưu con trỏ mượn, không sở hữu đối tượng Member.

### 9.2. Con trỏ mượn

Các hàm `findById()`, `findByPhone()` và `getAt()` trả về con trỏ mượn.

Ví dụ:

```cpp
Member* member = memberRepo.findByPhone("0912345678");
```

Không được thực hiện:

```cpp
delete member;
```

Việc giải phóng đối tượng thuộc trách nhiệm của Repository.

Không tự ý thay đổi số điện thoại thông qua con trỏ mượn vì có thể làm mất đồng bộ với `membersByPhone`.

### 9.3. Destructor

MemberRepository không cần định nghĩa destructor riêng.

`IRepository<T>` đã khai báo destructor ảo:

```cpp
virtual ~IRepository() = default;
```

Khi MemberRepository bị hủy, các thuộc tính được hủy theo thứ tự ngược với thứ tự khai báo:

1. `membersByPhone`: Giải phóng các phần tử trong map.
2. `repo`: Giải phóng các đối tượng Member đang quản lý.

Các đối tượng Invoice đang tham chiếu đến Member phải được xử lý trước khi MemberRepository bị hủy.

## 10. Áp dụng Dependency Inversion Principle

### 10.1. Trước khi refactor

MemberService phụ thuộc trực tiếp vào MemberRepository.

```cpp
class MemberService
{
private:
    MemberRepository& memberRepo;
};
```

MemberService phải biết lớp triển khai cụ thể là MemberRepository.

### 10.2. Sau khi refactor

MemberService dự kiến phụ thuộc vào interface.

```cpp
class MemberService
{
private:
    IRepository<Member>& memberRepo;
};
```

MemberRepository được truyền vào MemberService thông qua constructor.

Đây là Constructor Injection.

Tuy nhiên, `findByPhone()` là hàm riêng của MemberRepository, không thuộc `IRepository<Member>`.

Cách sử dụng chức năng tìm kiếm theo số điện thoại thông qua interface sẽ được xem xét khi refactor MemberService.

## 11. Các thay đổi sau refactor

### 11.1. Thay đổi trong MemberRepository.h

- Kế thừa công khai `IRepository<Member>`.
- Bổ sung `override` cho 9 hàm của interface.
- Loại bỏ thuộc tính `membersById`.
- Giữ lại thuộc tính `membersByPhone`.
- Sử dụng `std::` thay cho `using namespace std;` trong header.

### 11.2. Thay đổi trong MemberRepository.cpp

| Hàm | Nội dung thay đổi |
|---|---|
| isValidMember() | Sử dụng Validator::isValidPhone() |
| stringToMember() | Kiểm tra points chỉ chứa chữ số trước khi gọi stoi() |
| load() | Chỉ xây dựng chỉ mục số điện thoại |
| save() | Gọi trực tiếp repo.save() |
| add() | Loại bỏ kiểm tra lưu trữ trùng lặp và đồng bộ map |
| update() | Đơn giản hóa quá trình cập nhật chỉ mục số điện thoại |
| removeById() | Luôn trả về false |
| findById() | Gọi trực tiếp repo.findById() |

Hàm kiểm tra số điện thoại được chuyển sang `Validator` để có thể tái sử dụng tại các lớp khác.

### 11.3. Các thành phần giữ nguyên

- Thuộc tính `Repository<Member> repo`.
- Chức năng tìm kiếm theo số điện thoại.
- Định dạng file TXT.
- Cấu trúc Model Member.
- Cơ chế quản lý bộ nhớ.
- Các liên kết giữa Member và Invoice.
- Quy định không hỗ trợ xóa thành viên.

### 11.4. Rà soát theo validation_rules.md

`Repository<Member>` chịu trách nhiệm kiểm tra:

- Trạng thái nạp dữ liệu.
- Con trỏ nullptr.
- Trùng con trỏ.
- ID rỗng hoặc trùng ID.
- Các điều kiện lưu trữ chung.

MemberRepository chịu trách nhiệm kiểm tra:

- Tính hợp lệ của dữ liệu Member.
- Tính duy nhất của số điện thoại.
- Điểm tích lũy ban đầu bằng 0 khi thêm.
- Không thay đổi điểm tích lũy thông qua update().
- Tính nhất quán của chỉ mục số điện thoại.

Các kiểm tra loaded trong `add()` và `update()` được giữ lại vì hai hàm này trực tiếp thay đổi dữ liệu hoặc chỉ mục.

## 12. Kế hoạch kiểm thử

`tests/test_member_repository.cpp` đã biên dịch với C++17 và toàn bộ `.cpp` trong `src/` (trừ `main.cpp`), chạy ngày 25/11/2026: **204 PASS, 0 FAIL**. Các ô đã đánh dấu chỉ phản ánh trường hợp có trong bài test này.

### 12.1. Kiểm thử nạp dữ liệu

- [x] Nạp file hợp lệ, file rỗng và file không tồn tại.
- [x] Nạp file thiếu hoặc thừa trường.
- [x] Nạp file có mã thành viên không hợp lệ hoặc trùng mã.
- [x] Nạp file có họ tên không hợp lệ.
- [x] Nạp file có số điện thoại không hợp lệ hoặc trùng số.
- [x] Nạp file có points rỗng, sai định dạng hoặc vượt phạm vi int.
- [x] Kiểm tra trường hợp nạp dữ liệu nhiều lần.
- [x] Kiểm tra rollback danh sách và map khi nạp thất bại.
- [x] Nạp lại dữ liệu sau khi sửa file lỗi.

### 12.2. Kiểm thử thêm thành viên

- [x] Thêm thành viên hợp lệ.
- [x] Thêm khi Repository chưa được nạp.
- [x] Thêm con trỏ nullptr.
- [x] Thêm thành viên có dữ liệu không hợp lệ.
- [x] Thêm thành viên có mã trùng.
- [x] Thêm thành viên có số điện thoại trùng.
- [x] Thêm thành viên có điểm tích lũy khác 0.
- [x] Thêm lại con trỏ đã được quản lý.
- [x] Kiểm tra chỉ mục sau khi thêm thành công.
- [x] Kiểm tra rollback chỉ mục khi thêm thất bại do trùng ID.

### 12.3. Kiểm thử cập nhật thành viên

- [x] Cập nhật khi Repository chưa được nạp.
- [x] Cập nhật thành viên không tồn tại.
- [x] Cập nhật bằng dữ liệu không hợp lệ.
- [x] Cập nhật họ tên.
- [x] Cập nhật số điện thoại.
- [x] Cập nhật bằng số điện thoại của thành viên khác.
- [x] Cập nhật khi điểm tích lũy thay đổi.
- [x] Kiểm tra số điện thoại cũ không còn trong map.
- [x] Kiểm tra số điện thoại mới trỏ đúng đối tượng.
- [x] Kiểm tra mã và địa chỉ đối tượng được giữ nguyên.
- [ ] Kiểm tra liên kết Invoice được giữ nguyên.

### 12.4. Kiểm thử xóa thành viên

- [x] Xóa thành viên đang tồn tại.
- [x] Xóa thành viên không tồn tại.
- [x] Xóa khi Repository chưa được nạp.
- [x] Kiểm tra các trường hợp trên đều trả về false.
- [x] Kiểm tra danh sách và chỉ mục không thay đổi.

### 12.5. Kiểm thử truy vấn

- [x] Tìm thành viên theo mã.
- [x] Tìm thành viên theo số điện thoại.
- [x] Tìm thành viên không tồn tại.
- [x] Lấy thành viên tại vị trí hợp lệ.
- [x] Lấy thành viên tại vị trí không hợp lệ.
- [x] Kiểm tra count().
- [x] Kiểm tra isLoaded().

### 12.6. Kiểm thử lưu dữ liệu

- [x] Lưu danh sách thành viên hợp lệ.
- [x] Lưu danh sách thành viên rỗng.
- [x] Lưu khi Repository chưa được nạp.
- [x] Kiểm tra đúng định dạng TXT.
- [x] Nạp lại dữ liệu sau khi lưu.
- [x] Kiểm tra dữ liệu và chỉ mục sau khi nạp lại.

### 12.7. Kiểm thử IRepository

- [x] Sử dụng tham chiếu IRepository<Member>&.
- [x] Sử dụng con trỏ IRepository<Member>*.
- [x] Gọi đầy đủ 9 hàm thông qua interface.
- [x] Kiểm tra destructor ảo bằng xóa qua con trỏ interface.

### 12.8. Kiểm thử Validator::isValidPhone()

- [x] Số điện thoại hợp lệ.
- [x] Số điện thoại rỗng.
- [x] Số điện thoại thiếu hoặc thừa chữ số.
- [x] Số điện thoại không bắt đầu bằng 0.
- [x] Số điện thoại chứa chữ cái, khoảng trắng hoặc ký tự đặc biệt.

## 13. Tiến độ triển khai

- [x] Thiết kế MemberRepository kế thừa IRepository.
- [x] Xác định các hàm cần override.
- [x] Rà soát logic MemberRepository.h và MemberRepository.cpp.
- [x] Loại bỏ chỉ mục membersById.
- [x] Chuẩn hóa validation.
- [x] Thiết kế cơ chế đồng bộ và rollback chỉ mục số điện thoại.
- [x] Xây dựng nội dung tài liệu MemberRepository.md.
- [x] Review và CHỐT MARKDOWN.
- [x] Triển khai MemberRepository.h.
- [x] Triển khai MemberRepository.cpp.
- [x] Chuẩn hóa tài liệu theo MovieRepository.md.
- [x] Biên dịch mã nguồn cùng bài test MemberRepository bằng C++17.
- [x] Kiểm thử MemberRepository trong phạm vi `tests/test_member_repository.cpp`.
- [x] Kiểm thử IRepository thông qua MemberRepository.
- [x] Cập nhật kết quả kiểm thử và tiến độ refactor.

**Kết quả kiểm thử:** `tests/test_member_repository.cpp` ngày 25/11/2026: **204 PASS, 0 FAIL**. Refactor, chuẩn hóa mã nguồn, tài liệu và kiểm thử MemberRepository trong phạm vi bài test đã hoàn thành. Liên kết Invoice và tình huống setter ném ngoại lệ chưa được bài test này kiểm chứng.

## 14. Kết luận

MemberRepository là Concrete Repository triển khai `IRepository<Member>` và sử dụng `Repository<Member>` thông qua composition.

Sau refactor, MemberRepository triển khai đầy đủ 9 hàm của interface, loại bỏ chỉ mục ID dư thừa và giữ lại chỉ mục số điện thoại.

Thiết kế giúp giảm logic trùng lặp, bảo đảm tính nhất quán của dữ liệu và chuẩn bị cho việc áp dụng Dependency Inversion Principle tại tầng Business Logic.

Cơ chế quản lý bộ nhớ, cấu trúc Model và định dạng file TXT được giữ nguyên.
