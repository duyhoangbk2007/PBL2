# StaffService

> Trạng thái: CHỐT MARKDOWN; StaffService hoàn thành trong phạm vi kiểm thử ngày 27/11/2026. `StaffService.h/.cpp` khớp hai khối code dưới đây 100%; biên dịch C++17 và bốn bài test liên quan đều đạt.

## 1. Tổng quan

`StaffService` thuộc tầng Business, quản lý tài khoản nhân viên STAFF trên danh sách User dùng chung với AuthService. Không trực tiếp đọc/ghi TXT, không sở hữu User và không quản lý tài khoản ADMIN. Thiết kế dựa trên commit `a3bb136` và phương án B đã thống nhất.

File liên quan: `src/business/StaffService.h`, `src/business/StaffService.cpp`, `src/business/AuthService.h`, `src/data_access/IRepository.h`, `src/data_access/UserRepository.h/.cpp`, `src/models/User.h`, `src/utils/IdGenerator.h`, `tests/test_staff_service.cpp` (dự kiến).

## 2. Kiến trúc

```text
GUI
 |
StaffService --------> AuthService
 |
IRepository<User>
 ^
UserRepository ------> Repository<User> ------> users.txt
```

DIP qua IRepository<User>, DI qua constructor. AuthService và StaffService phải dùng cùng đối tượng Repository tài khoản trong ứng dụng. Các dependency phải tồn tại lâu hơn Service. Load/save là thao tác điều phối, không tự động gọi sau CRUD. Không reload khi các đối tượng liên kết còn sử dụng con trỏ User nếu chưa có cơ chế bảo đảm toàn vẹn.

## 3. Thuộc tính và API

| Thuộc tính | Kiểu | Ý nghĩa |
|---|---|---|
| userRepo | IRepository<User>& | Danh sách toàn bộ User |
| authService | AuthService& | Kiểm tra Permission |

| Hàm | Permission | Kết quả |
|---|---|---|
| Constructor | Không | Nhận dependency |
| load | Không | Kết quả nạp |
| save | Không | Kết quả lưu |
| addStaff | STAFF_ADD | Thêm bản sao với ID mới |
| updateStaff | STAFF_UPDATE | Sửa thông tin, giữ role và active |
| setStaffActive | STAFF_DEACTIVATE | Đặt trạng thái mong muốn |
| findStaffById | STAFF_VIEW | User STAFF hoặc nullptr |
| findStaffByUsername | STAFF_VIEW | User STAFF hoặc nullptr |
| getStaffAt | STAFF_VIEW | User STAFF theo chỉ số từ 0 hoặc nullptr |
| getStaffCount | STAFF_VIEW | Số STAFF, gồm inactive |

Hai thuộc tính, một constructor, chín hàm public, không có helper private. Thiếu quyền ném ngoại lệ từ AuthService; không chuyển thành false. Con trỏ truy vấn là con trỏ mượn chỉ đọc, không delete. Repository chịu trách nhiệm validation định dạng, username duy nhất và sở hữu con trỏ sau add thành công.

## 4. Mã nguồn đã chốt

### 4.1. StaffService.h

```cpp
#ifndef STAFF_SERVICE_H
#define STAFF_SERVICE_H

#include <string>
#include "../data_access/IRepository.h"
#include "../models/User.h"
#include "AuthService.h"

class StaffService
{
private:
    IRepository<User>& userRepo;
    AuthService& authService;

public:
    StaffService(IRepository<User>& userRepo,
                 AuthService& authService);

    // Quan ly du lieu
    bool load();
    bool save();

    // Quan ly nhan vien
    bool addStaff(User* staff);
    bool updateStaff(const User& newValues);
    bool setStaffActive(const std::string& staffId, bool active);

    // Tra cuu nhan vien
    const User* findStaffById(const std::string& staffId);
    const User* findStaffByUsername(const std::string& username);
    const User* getStaffAt(int index);
    int getStaffCount();
};

#endif
```

### 4.2. StaffService.cpp

```cpp
#include "StaffService.h"
#include "../utils/IdGenerator.h"
#include <vector>
using namespace std;

StaffService::StaffService(IRepository<User>& userRepo,
                           AuthService& authService)
    : userRepo(userRepo), authService(authService)
{
}

bool StaffService::load()
{
    return userRepo.load();
}

bool StaffService::save()
{
    return userRepo.save();
}

bool StaffService::addStaff(User* staff)
{
    authService.requirePermission("STAFF_ADD");

    if (!userRepo.isLoaded() ||
        staff == nullptr ||
        staff->getRole() != UserRole::STAFF)
    {
        return false;
    }

    vector<string> existingIds;
    for (int i = 0; i < userRepo.count(); i++)
    {
        const User* current = userRepo.getAt(i);
        if (current == nullptr)
        {
            return false;
        }
        existingIds.push_back(current->getUserId());
    }

    string userId = IdGenerator::generateNextId("USR", existingIds);
    User* newStaff = new User(userId,
                              staff->getUsername(),
                              staff->getPassword(),
                              staff->getFullName(),
                              UserRole::STAFF,
                              staff->isActive());

    try
    {
        if (!userRepo.add(newStaff))
        {
            delete newStaff;
            return false;
        }
    }
    catch (...)
    {
        delete newStaff;
        throw;
    }
    return true;
}

bool StaffService::updateStaff(const User& newValues)
{
    authService.requirePermission("STAFF_UPDATE");

    if (!userRepo.isLoaded())
    {
        return false;
    }

    const User* current = userRepo.findById(newValues.getUserId());
    if (current == nullptr ||
        current->getRole() != UserRole::STAFF ||
        newValues.getRole() != UserRole::STAFF)
    {
        return false;
    }

    if (current->isActive() != newValues.isActive())
    {
        return false;
    }
    return userRepo.update(newValues);
}

bool StaffService::setStaffActive(const string& staffId, bool active)
{
    authService.requirePermission("STAFF_DEACTIVATE");

    if (!userRepo.isLoaded())
    {
        return false;
    }

    const User* staff = userRepo.findById(staffId);
    if (staff == nullptr ||
        staff->getRole() != UserRole::STAFF)
    {
        return false;
    }

    if (staff->isActive() == active)
    {
        return true;
    }

    User updated(staff->getUserId(),
                 staff->getUsername(),
                 staff->getPassword(),
                 staff->getFullName(),
                 staff->getRole(),
                 active);
    return userRepo.update(updated);
}

const User* StaffService::findStaffById(const string& staffId)
{
    authService.requirePermission("STAFF_VIEW");

    const User* staff = userRepo.findById(staffId);
    if (staff == nullptr || staff->getRole() != UserRole::STAFF)
    {
        return nullptr;
    }
    return staff;
}

const User* StaffService::findStaffByUsername(const string& username)
{
    authService.requirePermission("STAFF_VIEW");

    for (int i = 0; i < userRepo.count(); i++)
    {
        const User* user = userRepo.getAt(i);
        if (user != nullptr &&
            user->getRole() == UserRole::STAFF &&
            user->getUsername() == username)
        {
            return user;
        }
    }
    return nullptr;
}

const User* StaffService::getStaffAt(int index)
{
    authService.requirePermission("STAFF_VIEW");

    if (index < 0)
    {
        return nullptr;
    }

    int staffIndex = 0;
    for (int i = 0; i < userRepo.count(); i++)
    {
        const User* user = userRepo.getAt(i);
        if (user != nullptr && user->getRole() == UserRole::STAFF)
        {
            if (staffIndex == index)
            {
                return user;
            }
            staffIndex++;
        }
    }
    return nullptr;
}

int StaffService::getStaffCount()
{
    authService.requirePermission("STAFF_VIEW");

    int count = 0;
    for (int i = 0; i < userRepo.count(); i++)
    {
        const User* user = userRepo.getAt(i);
        if (user != nullptr && user->getRole() == UserRole::STAFF)
        {
            count++;
        }
    }
    return count;
}
```

## 5. Giải thích từng hàm

Header guard ngăn khai báo lặp. Header include trực tiếp string, interface, User và AuthService; `std::` chỉ rõ namespace. Hai reference private không sở hữu dependency. CPP include IdGenerator và vector để sinh ID, `using namespace std;` đặt sau include. Các khối ngoặc nhọn xác định phạm vi biến và thân điều kiện/hàm.

### 5.1. Constructor

**Tổng quan:** Nhận hai tham chiếu dependency; không load, không cấp phát và không có giá trị trả về.

**Luồng hoạt động chi tiết**
```text
Nhan userRepo, authService -> Gan tham chieu qua initializer list -> Ket thuc
```

**Giải thích chi tiết từng đoạn code:** `IRepository<User>&` cho phép truyền UserRepository hoặc lớp giả lập triển khai interface. `AuthService&` dùng phiên xác thực đã tồn tại. Dấu `:` mở initializer list; `userRepo(userRepo), authService(authService)` liên kết thuộc tính với tham số tương ứng trước khi vào thân hàm rỗng. Không sao chép Repository.

### 5.2. load()

**Tổng quan:** Không tham số, trả bool theo Repository, không kiểm tra quyền vì là thao tác khởi tạo dữ liệu.

**Luồng hoạt động chi tiết**
```text
userRepo.load() -> true/false: tra nguyen ket qua
                -> exception: truyen ra caller
```

**Giải thích chi tiết từng đoạn code:** `return userRepo.load();` gọi phương thức virtual trên dependency, Repository chịu trách nhiệm đọc và kiểm tra TXT. Không bắt exception, không tự load thêm dữ liệu khác.

### 5.3. save()

**Tổng quan:** Không tham số; trả kết quả lưu, không tự thay đổi User.

**Luồng hoạt động chi tiết**
```text
userRepo.save() -> true/false: tra nguyen ket qua
                -> exception: truyen ra caller
```

**Giải thích chi tiết từng đoạn code:** `return userRepo.save();` giao việc serialize và ghi file cho Repository. Không yêu cầu Permission riêng; caller chỉ gọi trong luồng lưu phù hợp. Không ngầm lưu khi add/update.

### 5.4. addStaff()

**Tổng quan:** Nhận con trỏ User đầu vào do caller quản lý; thêm bản sao STAFF với ID mới. Trả false nếu điều kiện không đạt hoặc Repository từ chối; ngoại lệ được truyền ra.

**Luồng hoạt động chi tiết**
```text
STAFF_ADD? -- khong --> exception
  |
Loaded, staff != nullptr, role STAFF? -- khong --> false
  |
Duyet tat ca User -> nullptr? -- co --> false
  |                  khong -> thu ID, tiep tuc
Sinh USR -> loi: exception
  |
Tao User moi -> loi cap phat/khoi tao: exception
  |
repo.add -> false: delete, false
         -> exception: delete, throw
         -> true: repo so huu, true
```

**Giải thích chi tiết từng đoạn code:** `requirePermission` chạy trước mọi thao tác; thiếu quyền ném lỗi. Điều kiện `||` đánh giá ngắn mạch nên không dereference staff null. `vector<string> existingIds` chứa ID của cả ADMIN lẫn STAFF; vòng for dùng `count()` và `getAt(i)`, con trỏ const chỉ đọc. Null trong danh sách khiến hàm dừng an toàn; `push_back` thêm ID vào vector. `generateNextId("USR", existingIds)` lấy số lớn nhất cộng một, ném lỗi khi ID sai hoặc hết dải. `new User(...)` cấp phát đối tượng mới, sao chép username/password/fullName/active, ép role STAFF và không sửa ID hay liên kết của đầu vào. Nếu constructor ném lỗi, C++ tự giải phóng vùng cấp phát chưa hoàn tất. `try` bao quanh add: nếu false, Service vẫn sở hữu bản sao nên delete; `catch (...)` bắt mọi ngoại lệ từ add, delete rồi `throw;` ném lại ngoại lệ gốc. Hợp đồng Repository phải bảo đảm không nhận quyền sở hữu khi add thất bại. Khi thành công, Repository sở hữu bản sao và `return true`; caller vẫn sở hữu đầu vào.

### 5.5. updateStaff()

**Tổng quan:** Nhận `const User& newValues`, dùng ID để tìm nhân viên; không đổi role/active, không tự save. Trả bool hoặc truyền ngoại lệ.

**Luồng hoạt động chi tiết**
```text
STAFF_UPDATE? -- khong --> exception
  |
Loaded? -- khong --> false
  |
Tim theo ID -> ton tai va ca hai role STAFF? -- khong --> false
  |
Active giong nhau? -- khong --> false
  |
userRepo.update(newValues) -> tra bool / truyen exception
```

**Giải thích chi tiết từng đoạn code:** `const User&` tránh sao chép và không sửa đầu vào. Permission được kiểm tra trước. `isLoaded()` bảo đảm có dữ liệu để đối chiếu nghiệp vụ. `findById(newValues.getUserId())` trả con trỏ mượn; điều kiện null và hai role ngăn sửa ADMIN hoặc đổi role. So sánh `isActive()` ngăn dùng STAFF_UPDATE để vượt qua STAFF_DEACTIVATE. `return userRepo.update(newValues);` giao validation username/password/fullName, trùng username và cập nhật tại chỗ cho Repository. Không thay đổi ID hoặc liên kết hóa đơn.

### 5.6. setStaffActive()

**Tổng quan:** Nhận ID và bool active; true để kích hoạt, false để vô hiệu hóa. Trả true nếu đã đúng trạng thái hoặc cập nhật thành công, false nếu không hợp lệ. Dùng STAFF_DEACTIVATE cho cả hai chiều theo phương án B.

**Luồng hoạt động chi tiết**
```text
STAFF_DEACTIVATE? -- khong --> exception
  |
Loaded? -- khong --> false
  |
Tim ID -> ton tai va STAFF? -- khong --> false
  |
Trang thai == active? -- co --> true
  |
Tao User tam giu thong tin, doi active
  |
userRepo.update(updated) -> tra bool / truyen exception
```

**Giải thích chi tiết từng đoạn code:** `const string& staffId` không sao chép chuỗi, `bool active` là trạng thái đích. Kiểm tra quyền trước cả nhánh không cần cập nhật. Kiểm tra loaded rồi tìm User bằng ID; null hoặc ADMIN bị từ chối. Nếu trạng thái đã đúng, trả true không gọi update. `User updated(...)` là biến cục bộ trên stack, giữ ID, username, password, fullName, role, chỉ thay active; không sao chép danh sách hóa đơn. Repository update đối tượng gốc tại chỗ nên giữ liên kết. `return userRepo.update(updated);` truyền kết quả, biến tạm tự hủy khi ra khỏi hàm. Không tự save hoặc logout. AuthService kiểm tra lại trạng thái khi dùng phiên; nếu tài khoản được kích hoạt lại trước khi ID phiên được xóa, phiên cũ có thể hợp lệ trở lại — giới hạn của AuthService hiện tại, không sửa trong phạm vi này.

### 5.7. findStaffById()

**Tổng quan:** Nhận ID, trả con trỏ const STAFF (kể cả inactive) hoặc nullptr; thiếu quyền ném lỗi.

**Luồng hoạt động chi tiết**
```text
STAFF_VIEW? -- khong --> exception
  |
findById -> null hoac khong phai STAFF? -- co --> nullptr
  | khong
return staff
```

**Giải thích chi tiết từng đoạn code:** `requirePermission` kiểm tra quyền xem. `findById` gọi interface; không kiểm tra loaded lặp ở truy vấn, theo hợp đồng Repository hiện hành. Điều kiện `||` bảo vệ truy cập null và lọc ADMIN. `return staff;` trả con trỏ mượn chỉ đọc, không chuyển quyền sở hữu.

### 5.8. findStaffByUsername()

**Tổng quan:** Nhận username phân biệt hoa/thường, tìm STAFF kể cả inactive; trả nullptr nếu không tìm thấy.

**Luồng hoạt động chi tiết**
```text
STAFF_VIEW? -- khong --> exception
  |
Duyet User -> non-null, STAFF, username trung? -- co --> return user
  | khong: tiep tuc
Het danh sach -> nullptr
```

**Giải thích chi tiết từng đoạn code:** Permission kiểm tra trước. Vòng for từ 0 đến count()-1 thay cho API riêng findByUsername của Concrete Repository. `getAt(i)` trả con trỏ mượn. `&&` đánh giá ngắn mạch: chỉ đọc role và username khi con trỏ hợp lệ. `==` so sánh chính xác chuỗi, không chuẩn hóa. Khi khớp trả ngay, hết vòng lặp trả nullptr. Không tạo helper vì chỉ một hàm dùng logic này.

### 5.9. getStaffAt()

**Tổng quan:** Nhận chỉ số từ 0 trong danh sách đã lọc STAFF, không phải chỉ số toàn bộ User; trả con trỏ const hoặc nullptr.

**Luồng hoạt động chi tiết**
```text
STAFF_VIEW? -- khong --> exception
  |
index am? -- co --> nullptr
  |
staffIndex = 0; duyet User
  |-- null/ADMIN --> bo qua
  |-- STAFF --> staffIndex == index? -- co --> return user
  |                             khong --> staffIndex++, tiep tuc
Het danh sach -> nullptr
```

**Giải thích chi tiết từng đoạn code:** Kiểm tra quyền trước cả index âm. `staffIndex` là bộ đếm riêng, `i` là vị trí trong Repository. Chỉ khi user khác null và role STAFF mới so sánh chỉ số, trả user nếu đúng, nếu không tăng staffIndex bằng `++`. Không lọc active, không thay đổi thứ tự lưu trữ. Hết vòng for trả nullptr nếu index vượt phạm vi.

### 5.10. getStaffCount()

**Tổng quan:** Không tham số; trả số lượng STAFF gồm cả inactive, không tính ADMIN.

**Luồng hoạt động chi tiết**
```text
STAFF_VIEW? -- khong --> exception
  |
count = 0; duyet User
  |-- non-null va STAFF --> count++
  |-- nguoc lai --> bo qua
Het danh sach -> return count
```

**Giải thích chi tiết từng đoạn code:** Kiểm tra Permission trước. Biến `int count = 0` là bộ đếm cục bộ, khác với phương thức `userRepo.count()` trả tổng số User. Vòng for lấy từng con trỏ, điều kiện `&&` lọc null và ADMIN; `count++` tăng đúng một cho mỗi STAFF. Trả 0 nếu không có STAFF, không sửa dữ liệu.

## 6. Nội dung thay đổi và điểm cần ghi nhận

- Đổi dependency cụ thể thành IRepository<User>, đổi staffRepo thành userRepo.
- Thay deactivateStaff bằng setStaffActive; dùng STAFF_DEACTIVATE cho hai chiều, trạng thái đã đúng trả true. Cần đồng bộ caller cũ sau khi xác định phạm vi; chưa rà soát toàn bộ GUI.
- `tests/legacy_test/test_staff_service.cpp` còn sáu lời gọi `deactivateStaff()`; chưa sửa test cũ hoặc caller ngoài phạm vi StaffService.
- Giữ kiểm tra role của đối tượng đích, không dùng role của người thao tác để cấp quyền.
- Không thêm removeStaff, không đổi schema, không thêm Permission, không tự save.
- Giữ quyền STAFF_ADD cho việc tạo tài khoản với active do đầu vào cung cấp như code cũ; không áp dụng STAFF_DEACTIVATE cho tạo mới.
- data_format.md còn mô tả unordered_map không phù hợp với UserRepository hiện tại; ghi nhận để cập nhật tài liệu liên quan, không sửa Repository trong bước này.
- Password vẫn lưu văn bản theo phạm vi môn học; API trả const User* không phải cơ chế che giấu password. GUI không nên hiển thị password.
- Phiên lưu ID có giới hạn khi tài khoản được kích hoạt lại; không thay đổi AuthService trong bước này. UserRepository hiện không hỗ trợ xóa, nên không phát sinh tái sử dụng ID do xóa qua StaffService.

## 7. Kiểm thử

Ngày 27/11/2026, `tests/test_staff_service.cpp` dùng TXT tạm riêng, AuthService và UserRepository thực, cùng `IRepository<User>` giả lập để kiểm tra DIP/DI và nhánh lỗi. Biên dịch C++17 với dependency cần thiết thành công. Kết quả: **StaffService Passed: 32, Failed: 0, Total: 32; AuthService Passed: 31, Failed: 0, Total: 31; MovieService Passed: 41, Failed: 0, Total: 41; RoomService Passed: 43, Failed: 0, Total: 43**.

- [x] Constructor DI qua `IRepository<User>&`; gọi đủ 9 hàm public, load/save, file thiếu/sai, load lặp và lưu rồi nạp lại bằng fixture riêng.
- [x] Thêm STAFF: sinh ID từ toàn bộ User, đầu vào vẫn thuộc caller; null, ADMIN, username trùng, dữ liệu sai và hết dải ID; mock `add()` trả false hoặc ném ngoại lệ.
- [x] Cập nhật STAFF: giữ ID/địa chỉ, từ chối ID thiếu, ADMIN, role/active thay đổi qua `updateStaff()` và username trùng.
- [x] `setStaffActive()` dùng `STAFF_DEACTIVATE` cho cả bật/tắt: trạng thái đã đúng, ID thiếu, ADMIN, mock `update()` false/exception; giữ thông tin và liên kết Invoice đã nối trực tiếp với User.
- [x] Truy vấn theo ID/username, phân biệt hoa thường, lọc ADMIN, gồm inactive, index âm/vượt phạm vi, danh sách không có STAFF và mock `getAt()` trả nullptr.
- [x] Permission-only: chưa đăng nhập, ADMIN thiếu quyền, STAFF được cấp quyền và phiên bị vô hiệu hóa; nhánh Staff Repository chưa load được kiểm tra bằng mock riêng sau khi AuthService đã xác thực.
- [x] Hồi quy AuthService 31/31, MovieService 41/41 và RoomService 43/43 cùng ngày. Không sửa test legacy.
- [ ] Chưa kiểm thử lỗi ghi TXT và tích hợp nạp Invoice từ Repository thực; sáu lời gọi `deactivateStaff()` trong `tests/legacy_test/test_staff_service.cpp` vẫn chờ cập nhật khi xử lý test legacy.

Ví dụ PowerShell (dự kiến, điều chỉnh dependency theo checkout thực tế):

```powershell
$services = @('AuthService.cpp','MovieService.cpp','RoomService.cpp','StaffService.cpp')
$sources = Get-ChildItem src -Recurse -Filter *.cpp | Where-Object {
    $_.FullName -match '[\\/](models|utils|data_access|data_structures)[\\/]' -or
    ($_.FullName -match '[\\/]business[\\/]' -and $_.Name -in $services)
} | ForEach-Object { $_.FullName }
g++ -std=c++17 -I src tests/test_staff_service.cpp $sources -o test_staff_service.exe
if ($LASTEXITCODE -eq 0) { .\test_staff_service.exe }
```

Đã biên dịch C++17 và chạy lần lượt StaffService, AuthService, MovieService, RoomService với các module và dependency cần thiết; bốn bài đều exit code 0. Không suy ra toàn bộ ứng dụng biên dịch thành công; các Service chưa refactor còn lỗi tương thích API riêng.

## 8. Bàn giao

- [x] Rà soát StaffService và dependency.
- [x] Thống nhất phương án B.
- [x] Soạn Markdown và CHỐT MARKDOWN.
- [x] Triển khai `StaffService.h/.cpp` khớp 100% với Markdown; DIP qua `IRepository<User>&`, DI và Permission-only, thay `deactivateStaff()` bằng `setStaffActive()`.
- [x] Kiểm tra cú pháp C++17 và biên dịch/liên kết trong phạm vi Service cùng dependency cần thiết thành công.
- [x] Viết và chạy `tests/test_staff_service.cpp`: Passed 32, Failed 0, Total 32; hoàn thành StaffService trong phạm vi đã kiểm thử.
- [x] Hồi quy AuthService 31/31, MovieService 41/41, RoomService 43/43 PASS ngày 27/11/2026.
- [x] Ghi nhận giới hạn kiểm thử và cập nhật tài liệu tiến độ.
- [ ] Commit/push do người phụ trách thực hiện.
