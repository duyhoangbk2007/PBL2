# UserRepository — thiết kế refactor đã CHỐT MARKDOWN

## 1. Tổng quan

`UserRepository` thuộc Data Access, triển khai `IRepository<User>` và chứa `Repository<User>` để quản lý `User*`. Tài liệu này đã **CHỐT MARKDOWN**; mã ở mục 5 đã được triển khai khớp 100% vào `.h/.cpp`. `tests/test_user_repository.cpp` biên dịch và chạy ngày 25/11/2026 đạt **69 PASS, 0 FAIL** trong phạm vi bài test.

Phạm vi thiết kế đã xác định: bỏ hai map và `<unordered_map>`, chỉ dùng `Repository<User>` để lưu trữ; giữ schema `users.txt`, API công khai, quy tắc không xóa tài khoản và việc kiểm tra dữ liệu User. `load()` kiểm tra username trùng sau khi nạp; `update()` chỉ sửa đối tượng sau khi mọi kiểm tra đạt. `AuthService` chịu trách nhiệm xác thực và Permission, không chuyển việc phân quyền vào Repository.

Đối chiếu: [MovieRepository.md](MovieRepository.md), [RoomRepository.md](RoomRepository.md), [validation_rules.md](../../architecture/validation_rules.md), [business_rules.md](../../overview/business_rules.md), [data_format.md](../../overview/data_format.md).

## 2. Kiến trúc

```text
Service → IRepository<User> ← UserRepository
                                  │ composition
                                  ▼
                           Repository<User>
                                  │
                                  ▼
                       DoublyLinkedList<User*>
```

`UserRepository` tiếp tục kế thừa interface chung và dùng callback ID, parse, serialize của `Repository<User>`. Không tạo interface hoặc container mới. Bỏ map vì chúng lưu thêm hai tập con trỏ mượn, buộc `load`, `add`, `update` đồng bộ chỉ mục và xử lý rollback riêng; chúng không phải cấu trúc lưu trữ có quyền sở hữu. Sau khi bỏ map, tra ID dùng `repo.findById()`, tra username duyệt `repo.count()` và `repo.getAt()`. Cả hai phép tra cứu là tuyến tính theo số User; chưa có chỉ mục tăng tốc.

`Repository<User>` kiểm tra trạng thái loaded, null, trùng con trỏ, ID rỗng/trùng, vị trí và lỗi đọc/ghi. `UserRepository` kiểm tra định dạng User và username duy nhất. Service kiểm Permission, điều kiện nghiệp vụ và sinh ID. Không lặp kiểm tra chung trong `add()` chỉ để thay map.

## 3. Định dạng dữ liệu

Mỗi dòng `users.txt` có đúng sáu trường:

```text
userId|username|password|fullName|role|active
USR0001|admin|123456|Quan tri vien|ADMIN|1
```

ID có tiền tố `USR` và bốn chữ số; role chỉ `ADMIN` hoặc `STAFF`; active chỉ `0` hoặc `1`. `isValidUser()` kiểm tra ID, username/password/fullName bằng `Validator`, không nhận username có khoảng trắng ở đầu/cuối và chỉ chấp nhận hai giá trị role. Parser kiểm số trường, role và active trước khi tạo `User`, rồi gọi `isValidUser()`. Schema và cách serialize không đổi. Mật khẩu hiện lưu dưới dạng văn bản thuần theo định dạng dự án; không đổi định dạng trong đợt này.

## 4. Thiết kế lớp

| Thành phần | Vai trò |
|---|---|
| `Repository<User> repo` | Container sở hữu User và thực hiện lưu trữ chung duy nhất |
| `getUserId`, `stringToUser`, `userToString` | Callback cho generic Repository |
| `isValidUser` | Validation riêng của User |
| `findByUsername` | Duyệt User để kiểm tra username duy nhất và phục vụ truy vấn |

Header giữ các chữ ký public của mã hiện tại. Không khai báo `usersById`, `usersByUsername` hoặc include `<unordered_map>`. `load()` và `update()` tiếp tục override interface.

## 5. Mã nguồn dự kiến

Các khối sau là mã **đã CHỐT MARKDOWN** cho `.h/.cpp`, bao gồm `load()` và `update()`.

### 5.1. `UserRepository.h` đã triển khai

```cpp
#ifndef PBL2_USER_REPOSITORY_H
#define PBL2_USER_REPOSITORY_H

#include <string>
#include "IRepository.h"
#include "Repository.h"
#include "../models/User.h"

class UserRepository : public IRepository<User>
{
private:
    Repository<User> repo;

    static std::string getUserId(User* user);
    static User* stringToUser(const std::string& line);
    static std::string userToString(User* user);

    static bool isValidUser(const User* user);

public:
    UserRepository(const std::string& filePath);

    // Doc va ghi du lieu
    bool load() override;
    bool save() override;

    // Quan ly du lieu
    bool add(User* user) override;
    bool update(const User& newValues) override;
    bool removeById(const std::string& userId) override;

    // Truy van du lieu
    User* findById(const std::string& userId) override;
    User* findByUsername(const std::string& username);
    User* getAt(int index) override;
    int count() override;
    bool isLoaded() override;
};

#endif
```

### 5.2. `UserRepository.cpp` đã triển khai

```cpp
#include "UserRepository.h"
#include "../utils/Validator.h"
#include <stdexcept>
#include <vector>
using namespace std;

// Kiem tra du lieu User
bool UserRepository::isValidUser(const User* user)
{
    if (user == nullptr)
    {
        return false;
    }
    if (!Validator::isValidId(user->getUserId(), "USR"))
    {
        return false;
    }
    if (!Validator::isValidText(user->getUsername()) ||
        !Validator::isValidText(user->getPassword()) ||
        !Validator::isValidText(user->getFullName()))
    {
        return false;
    }
    const string& username = user->getUsername();
    if (username.front() == ' ' || username.back() == ' ')
    {
        return false;
    }
    if (user->getRole() != UserRole::ADMIN &&
        user->getRole() != UserRole::STAFF)
    {
        return false;
    }
    return true;
}

UserRepository::UserRepository(const string& filePath)
    : repo(filePath, getUserId, stringToUser, userToString)
{
}

string UserRepository::getUserId(User* user)
{
    if (user == nullptr)
    {
        return "";
    }
    return user->getUserId();
}

User* UserRepository::stringToUser(const string& line)
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

    if (fields.size() != 6)
    {
        throw runtime_error("Sai so luong truong User");
    }
    if (fields[4] != "ADMIN" && fields[4] != "STAFF")
    {
        throw runtime_error("Role User khong hop le");
    }
    UserRole role;
    if (fields[4] == "ADMIN")
    {
        role = UserRole::ADMIN;
    }
    else
    {
        role = UserRole::STAFF;
    }
    if (fields[5] != "0" && fields[5] != "1")
    {
        throw runtime_error("Trang thai User khong hop le");
    }
    bool active = (fields[5] == "1");

    User* user = new User(
        fields[0],
        fields[1],
        fields[2],
        fields[3],
        role,
        active
    );
    if (!isValidUser(user))
    {
        delete user;
        throw runtime_error("Du lieu User khong hop le");
    }
    return user;
}

// Chuyen User thanh mot dong TXT
string UserRepository::userToString(User* user)
{
    if (!isValidUser(user))
    {
        throw runtime_error("User khong hop le khi ghi file");
    }
    string role;
    if (user->getRole() == UserRole::ADMIN)
    {
        role = "ADMIN";
    }
    else
    {
        role = "STAFF";
    }
    return user->getUserId() + "|" +
           user->getUsername() + "|" +
           user->getPassword() + "|" +
           user->getFullName() + "|" +
           role + "|" +
           (user->isActive() ? "1" : "0");
}

// Doc du lieu User tu file
bool UserRepository::load()
{
    if (!repo.load())
    {
        return false;
    }

    try
    {
        for (int i = 0; i < repo.count(); i++)
        {
            User* user = repo.getAt(i);
            for (int j = i + 1; j < repo.count(); j++)
            {
                User* other = repo.getAt(j);
                if (user->getUsername() == other->getUsername())
                {
                    throw runtime_error("Trung username trong file User");
                }
            }
        }
    }
    catch (...)
    {
        repo.clear();
        throw;
    }
    return true;
}

// Luu du lieu User xuong file
bool UserRepository::save()
{
    return repo.save();
}

// Them User moi
bool UserRepository::add(User* user)
{
    if (!isValidUser(user))
    {
        return false;
    }
    if (findByUsername(user->getUsername()) != nullptr)
    {
        return false;
    }
    return repo.add(user);
}

// Cap nhat thong tin User
bool UserRepository::update(const User& newValues)
{
    if (!repo.isLoaded() || !isValidUser(&newValues))
    {
        return false;
    }

    User* user = repo.findById(newValues.getUserId());
    if (user == nullptr)
    {
        return false;
    }
    if (user->getRole() != newValues.getRole())
    {
        return false;
    }

    User* existingUser = findByUsername(newValues.getUsername());
    if (existingUser != nullptr && existingUser != user)
    {
        return false;
    }

    user->setUsername(newValues.getUsername());
    user->setPassword(newValues.getPassword());
    user->setFullName(newValues.getFullName());
    user->setActive(newValues.isActive());
    return true;
}

bool UserRepository::removeById(const string& userId)
{
    // Chua ho tro xoa tai khoan.
    (void)userId;
    return false;
}

User* UserRepository::findById(const string& userId)
{
    return repo.findById(userId);
}

User* UserRepository::findByUsername(const string& username)
{
    for (int i = 0; i < repo.count(); i++)
    {
        User* user = repo.getAt(i);
        if (user != nullptr && user->getUsername() == username)
        {
            return user;
        }
    }
    return nullptr;
}

User* UserRepository::getAt(int index)
{
    return repo.getAt(index);
}

int UserRepository::count()
{
    return repo.count();
}

bool UserRepository::isLoaded()
{
    return repo.isLoaded();
}
```

`load()` trả `false` khi `repo.load()` trả `false`; lỗi parse và ID từ `Repository<User>` tiếp tục truyền ra theo hợp đồng của tầng chung. Nếu kiểm tra username sau nạp thất bại, hàm clear toàn bộ User vừa nạp rồi ném lại lỗi. `update()` giữ ID và địa chỉ đối tượng, kiểm tra role và username trước các setter; không thêm map hoặc rollback riêng.

## 6. Thư viện và cú pháp

- Header include trực tiếp `<string>` cho `std::string`; `IRepository.h`, `Repository.h`, `User.h` cho interface, container và Model. Không dùng `using namespace std;` hoặc `<unordered_map>` trong header.
- `.cpp` dùng `<vector>` để tách một dòng TXT và `<stdexcept>` để báo lỗi parse/serialize. `using namespace std;` đặt sau mọi include; vì vậy `string`, `vector`, `runtime_error` trong định nghĩa không cần `std::`.
- Callback `static` không cần đối tượng `UserRepository`; `override` bảo đảm chữ ký khớp interface. `new` cấp phát User khi parse; `delete` giải phóng trước khi ném lỗi validation.

## 7. Giải thích các hàm private

### 7.1. `isValidUser(const User* user)`

Mục đích: xác minh đối tượng, trả `false` nếu null, ID sai, các trường văn bản sai, username có dấu cách ở biên hoặc role ngoài `ADMIN`/`STAFF`.

```text
null? → false → ID hợp lệ? → văn bản hợp lệ? → username không có dấu cách biên?
                                      → role hợp lệ? → true
Mỗi nhánh sai ở bước tương ứng → false.
```

`Validator::isValidText` chạy trước `front()`/`back()` nên chuỗi rỗng bị loại trước khi đọc ký tự. `const string&` mượn username, không sao chép và không sửa Model. `&&` và `||` kết hợp các điều kiện; hàm không kiểm tra quyền.

### 7.2. `getUserId(User* user)`

Mục đích: callback lấy ID; null trả chuỗi rỗng, còn lại trả bản sao ID.

```text
user null? → return ""; ngược lại → user->getUserId() → return string.
```

Kiểm tra null ngăn dereference; generic Repository dùng callback này khi tra và kiểm ID.

### 7.3. `stringToUser(const string& line)`

Mục đích: parse một dòng TXT thành `User*`; dữ liệu sai ném `runtime_error`.

```text
tách '|': find/substr/push_back → đúng 6 trường?
  không → throw; có → role ADMIN/STAFF? → active 0/1?
  sai → throw; đúng → new User → isValidUser?
  sai → delete + throw; đúng → return User*.
```

`start` và `pos` giữ vị trí tách; `string::npos` đánh dấu hết dấu phân cách, kể cả trường cuối rỗng. `fields[4]` ánh xạ enum, `fields[5]` ánh xạ bool. Sau `new`, nhánh validation sai phải `delete`; nhánh thành công giao con trỏ cho caller là generic Repository khi load. `load()` kiểm tra username trùng giữa các User sau khi nạp toàn bộ danh sách.

### 7.4. `userToString(User* user)`

Mục đích: chuyển User hợp lệ thành sáu trường TXT; User sai ném `runtime_error`.

```text
isValidUser? → sai: throw; đúng: chọn chuỗi role → nối sáu trường bằng '|'
→ active thành "1"/"0" → return string.
```

`role` là chuỗi cục bộ; toán tử `+` nối trường; toán tử ba ngôi ánh xạ bool. Hàm chỉ tạo chuỗi, không tự ghi file.

## 8. Giải thích các hàm public

### 8.1. Constructor

`UserRepository(const string& filePath)` nhận đường dẫn, không trả về và không tự load.

```text
filePath → danh sách khởi tạo repo(filePath, getUserId, stringToUser, userToString)
→ kết thúc constructor.
```

`repo` được xây dựng trước thân `{}`; ba tên hàm static được truyền như callback. Không tạo map hoặc `User` ở bước này.

### 8.2. `load()`

Không tham số; trả `false` khi `repo.load()` trả `false`, trả `true` khi nạp và kiểm username thành công, ném `runtime_error` nếu username trùng. Lỗi từ generic Repository được truyền nguyên ra ngoài.

```text
repo.load() → false? → return false
     │ true
     ▼
try → i = 0; i < repo.count(); i++
       └→ user = repo.getAt(i)
          j = i + 1; j < repo.count(); j++
            └→ other = repo.getAt(j)
               username bằng nhau? → throw runtime_error
     │ hết hai vòng lặp
     ▼
return true
Nếu kiểm tra trong try ném lỗi → catch (...) → repo.clear() → throw lại.
```

`repo.load()` đã kiểm file, parse, ID và trạng thái loaded, nên không kiểm lại. Vòng ngoài chọn mỗi User, vòng trong bắt đầu ở `i + 1` để chỉ so từng cặp một lần và không so đối tượng với chính nó. `repo.getAt()` lấy con trỏ do generic Repository sở hữu; sau khi nạp thành công, danh sách này không có User null theo hợp đồng tầng chung. So sánh `string` giữ phân biệt hoa/thường. Số lượng tài khoản Admin và Staff dự kiến nhỏ, nên hai vòng lặp O(n²) đơn giản hơn việc thêm một chỉ mục phải đồng bộ. `runtime_error` trong nhánh trùng đi qua `catch (...)`; `repo.clear()` xóa User và đặt lại trạng thái chưa loaded trước khi `throw;` ném lại ngoại lệ. Nếu `repo.load()` tự ném lỗi trước `try`, tầng chung đã xử lý danh sách tạm của nó; hàm này không clear dữ liệu khác.

### 8.3. `save()`

Không tham số; trả nguyên `bool` từ `repo.save()`.

```text
repo.save() → true/false (hoặc exception từ serializer) → return cùng kết quả.
```

Generic Repository kiểm loaded, serialize từng User và ghi file. Thay đổi dự kiến bỏ lớp bọc exception hiện tại; cần kiểm thử lại hợp đồng lỗi với caller khi triển khai. Không tự save trong `add()` hoặc `update()`.

### 8.4. `add(User* user)`

Thêm User hợp lệ có username duy nhất; trả `bool`.

```text
isValidUser? → sai: false; đúng → findByUsername?
  đã có: false; chưa có → repo.add(user) → return bool.
```

Kiểm riêng username trước khi chuyển quyền sở hữu. `repo.add()` tự kiểm loaded, null/trùng con trỏ, ID rỗng/trùng; không lặp các kiểm tra đó. Nếu trả `false`, caller vẫn sở hữu `user`; nếu thành công, generic Repository sở hữu. Không còn cập nhật map hay rollback chỉ mục. Khi `repo.add()` ném exception, không nuốt ngoại lệ.

### 8.5. `update()`

Nhận `const User& newValues`; trả `false` nếu chưa load, dữ liệu sai, ID không tồn tại, role khác hoặc username thuộc User khác; trả `true` sau khi cập nhật.

```text
repo.isLoaded() và isValidUser(&newValues)? → sai: false
     │ đúng
     ▼
repo.findById(ID) → null? → false
     │ có User
     ▼
role bằng nhau? → không: false
     │ có
     ▼
findByUsername(username mới) → User khác? → false
     │ chưa có hoặc là chính User cần sửa
     ▼
setUsername → setPassword → setFullName → setActive → true.
```

`&newValues` lấy địa chỉ tham chiếu để dùng validator; `repo.findById()` tìm theo ID bất biến. Kiểm null trước `->`. So role giữ ràng buộc không đổi vai trò qua cập nhật; `findByUsername()` cho phép giữ username cũ của chính User, từ chối username của người khác. Sau mọi kiểm tra, các setter chạy đúng thứ tự đã thống nhất: username, password, fullName, active. Không gán toàn bộ `User`, nên ID, địa chỉ và liên kết Invoice vẫn giữ. Không có map, biến username cũ, `try/catch` hay rollback riêng; nếu một setter ném ngoại lệ sau khi setter trước đã chạy, hàm không bảo đảm cập nhật nguyên tử và ngoại lệ truyền ra ngoài. Hàm không tự save.

### 8.6. `removeById(const string& userId)`

Nhận ID, luôn trả `false`; không xóa hoặc thay đổi bộ nhớ.

```text
userId → (void)userId để tránh cảnh báo tham số chưa dùng → return false.
```

Giữ nguyên chính sách chưa hỗ trợ xóa tài khoản; không gọi `repo.removeById()`.

### 8.7. `findById(const string& userId)`

Trả `User*` mượn hoặc `nullptr` từ generic Repository.

```text
userId → repo.findById(userId) → return User*/nullptr.
```

Không tra map; generic Repository duyệt danh sách và so ID bằng callback.

### 8.8. `findByUsername(const string& username)`

Trả User đầu tiên có username khớp hoặc `nullptr`.

```text
i = 0 → i < repo.count()?
  có → repo.getAt(i) → null? bỏ qua : so username
      khớp → return user; khác → i++ và lặp
  hết danh sách → return nullptr.
```

`getAt()` trả con trỏ mượn. Điều kiện null trước `->` bảo vệ truy cập; so sánh `string` là so sánh chính xác, có phân biệt chữ hoa/thường theo hành vi hiện tại. Không tạo container phụ; độ phức tạp tối đa O(n).

### 8.9. `getAt(int index)`

Chuyển tiếp `repo.getAt(index)`; trả con trỏ mượn hoặc null nếu vị trí sai.

```text
index → repo.getAt(index) → return User*/nullptr.
```

### 8.10. `count()`

Chuyển tiếp `repo.count()`; trả số User hiện trong bộ nhớ, không tự load.

```text
repo.count() → return int.
```

### 8.11. `isLoaded()`

Chuyển tiếp `repo.isLoaded()`; trả trạng thái loaded, không đọc file.

```text
repo.isLoaded() → return bool.
```

## 9. Quản lý bộ nhớ

`Repository<User>` sở hữu `User*` sau `add()` thành công hoặc `load()` thành công; `clear()`, hủy Repository và xóa qua generic Repository giải phóng đối tượng được sở hữu. `removeById()` của UserRepository luôn từ chối nên không xóa User. `findById`, `findByUsername`, `getAt` trả con trỏ mượn; không được `delete` từ caller. Con trỏ mất hiệu lực khi generic Repository clear/hủy; quan hệ `User`–`Invoice` không đồng nghĩa quyền sở hữu Invoice.

Nếu `add()` thất bại, caller vẫn chịu trách nhiệm giải phóng đối tượng của mình. Trong parser, User vừa cấp phát phải được giải phóng nếu validation thất bại. Nếu `load()` phát hiện username trùng sau khi generic Repository nạp xong, `catch` gọi `repo.clear()` để giải phóng toàn bộ User vừa nạp và đặt lại loaded. Con trỏ mượn từ lần nạp đó không còn hợp lệ sau clear. `update()` sửa cùng đối tượng nên không chuyển quyền sở hữu.

## 10. Áp dụng DIP

Header kế thừa `IRepository<User>` và override đủ chín hàm chung: `load`, `save`, `add`, `update`, `removeById`, `findById`, `getAt`, `count`, `isLoaded`. `findByUsername` là API bổ sung của UserRepository, không thêm vào interface chung. Generic `Repository<User>` vẫn là thành viên private. DIP tại UserRepository đã triển khai và được kiểm thử qua interface; việc Service nhận interface thay vì concrete là công việc khác, chưa được xác nhận ở đây.

## 11. Phạm vi thay đổi dự kiến

- Bỏ `<unordered_map>`, hai map và mọi đoạn đồng bộ/rollback chỉ phục vụ map.
- Sắp xếp lớp và định nghĩa hàm theo mẫu MovieRepository; header dùng `std::string`, `.cpp` dùng `using namespace std;` sau include.
- `save`, `findById` chuyển tiếp trực tiếp; `findByUsername` duyệt danh sách; `add` chỉ kiểm User và username trước `repo.add`.
- Giữ parse/serialize, validation, các hàm truy vấn, constructor và chính sách không xóa tài khoản về mặt logic.
- `load` kiểm username bằng hai vòng lặp và clear khi phát hiện lỗi; `update` kiểm đủ điều kiện trước bốn setter. Mã nguồn C++ hiện tại đã triển khai theo Markdown chốt.

## 12. Kế hoạch kiểm thử

**Kết quả thực tế (25/11/2026):** Biên dịch C++17 `tests/test_user_repository.cpp` với mọi `.cpp` trong `src/` trừ `main.cpp`, chạy đạt **69 PASS, 0 FAIL** (exit code 0). Test dùng các file dữ liệu riêng và xóa chúng khi kết thúc. Các mục đánh dấu dưới đây chỉ phản ánh trường hợp được bài test thực sự kiểm tra.

- [x] Kiểm tra interface, trạng thái, `count`, tra ID/username và chỉ số trước/sau load; username có và không có, phân biệt chữ hoa/thường.
- [x] Load file hợp lệ/rỗng/thiếu; sai số trường, ID, role, active, username trùng, ID trùng; kiểm trạng thái rỗng sau lỗi và load lại sau khi sửa file.
- [x] Add hợp lệ, null, chưa load, ID trùng, con trỏ trùng, username trùng, dữ liệu sai; kiểm số lượng và con trỏ sau add thành công, caller xóa con trỏ khi add thất bại.
- [x] Update thuộc tính, giữ username, từ chối ID thiếu/role đổi/username trùng/dữ liệu sai; giữ ID, địa chỉ và số lượng, không đổi thuộc tính khi trả `false`.
- [x] `removeById` luôn từ chối và không đổi dữ liệu; save trước load, save/load lại và đối chiếu thuộc tính.
- [x] Gọi chín hàm chung qua `IRepository<User>&`, truy vấn qua con trỏ interface và kiểm destructor ảo.
- [ ] Kiểm liên kết Invoice khi update, exception giữa các setter, lỗi serializer/ghi file và tác động tới Service liên quan.

Kết quả trên không xác nhận các trường hợp còn để trống hoặc toàn bộ nghiệp vụ Service.

## 13. Tiến độ

- [x] Đọc quy chuẩn, thiết kế DIP, validation, nghiệp vụ, định dạng dữ liệu và mẫu Movie/Room.
- [x] Rà soát mã UserRepository hiện tại và xác định phần phụ thuộc map.
- [x] Viết bản thiết kế dự kiến cho các hàm đã được chỉ định.
- [x] Bổ sung mã dự kiến, luồng và xử lý lỗi cho `load()` và `update()` theo thiết kế đã thống nhất.
- [x] Review toàn bộ Markdown và xác nhận **CHỐT MARKDOWN**.
- [x] Triển khai `.h/.cpp`, kiểm tra cú pháp C++17 và đối chiếu 100% với Markdown.
- [x] Chạy `tests/test_user_repository.cpp` ngày 25/11/2026: 69 PASS, 0 FAIL.

## 14. Kết luận

UserRepository đã refactor theo DIP và chỉ dùng `Repository<User>` để lưu User; hai `unordered_map` và logic đồng bộ chỉ mục đã được loại bỏ. Validation riêng của User nằm ở `isValidUser()`; điều kiện lưu trữ chung do `Repository<User>` đảm nhiệm. Mã `.h/.cpp` khớp Markdown đã chốt; test UserRepository đạt **69 PASS, 0 FAIL** trong phạm vi đã liệt kê ở mục 12.
