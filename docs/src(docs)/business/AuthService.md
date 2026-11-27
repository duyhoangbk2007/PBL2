# AuthService

> Trạng thái: CHỐT MARKDOWN; AuthService đã hoàn thành trong phạm vi kiểm thử ngày 27/11/2026. `AuthService.h/.cpp` khớp hai khối code dưới đây 100%; biên dịch C++17 và ba bài test liên quan đều đạt.

## 1. Tổng quan

`AuthService` thuộc tầng Business, quản lý phiên đăng nhập và kiểm tra quyền của tài khoản hiện tại. Không trực tiếp đọc/ghi TXT, không sở hữu User và không tự cấp quyền cho ADMIN.

File liên quan:
- `src/business/AuthService.h`, `src/business/AuthService.cpp`
- `src/data_access/IRepository.h`, `UserRepository.h`, `RolePermissionRepository.h`
- `src/models/User.h`
- `tests/test_auth_service.cpp` (dự kiến)

Cơ sở thiết kế: nhánh Kiệt commit `625bdb6`, nhánh Hoàng commit `7ab0bee` (bao gồm RolePermissionRepository mới). Kết quả biên dịch và kiểm thử sau merge được ghi ở mục 7.

## 2. Kiến trúc

```text
GUI / Business Service
          |
      AuthService
       /       \
IRepository<User>  RolePermissionRepository
       ^                    |
 UserRepository       role_permissions.txt
```

DI qua constructor cho cả hai dependency. DIP áp dụng cho UserRepository; RolePermissionRepository là lớp đặc thù không có CRUD, giữ dependency cụ thể để tuân thủ thiết kế một interface chung. Không tạo interface mới. Repository phải tồn tại lâu hơn Service. Caller chịu trách nhiệm load dữ liệu và xử lý ngoại lệ load bảng quyền.

## 3. Thuộc tính và API

| Thuộc tính | Kiểu | Ý nghĩa |
|---|---|---|
| userRepo | IRepository<User>& | Truy xuất tài khoản |
| permissionRepo | RolePermissionRepository& | Tra cứu bảng quyền |
| currentUserId | std::string | ID phiên hiện tại, rỗng khi logout |

| Hàm | Kết quả / trách nhiệm |
|---|---|
| Constructor | Nhận hai dependency, phiên rỗng |
| findByUsername (private) | Con trỏ mượn hoặc nullptr |
| hasUserPermission (private) | Tra quyền cho User đã xác thực |
| login | true nếu thông tin hợp lệ; false nếu sai; ném lỗi nếu UserRepo chưa load |
| logout | Xóa ID phiên |
| getCurrentUser | User đang hoạt động hoặc nullptr |
| isLoggedIn | Kiểm tra phiên hợp lệ |
| hasPermission | false nếu phiên hoặc bảng quyền không hợp lệ, hoặc thiếu quyền |
| requireLogin | Tham chiếu User hoặc runtime_error |
| requirePermission | Tham chiếu User hoặc runtime_error |

Không dùng isAdmin(), isStaff(), requireAdmin(). Role chỉ dùng xác thực loại tài khoản và tra bảng quyền. Không yêu cầu bảng quyền đã load để login; khi chưa load, mọi yêu cầu quyền bị từ chối.

## 4. Mã nguồn đã chốt

### 4.1. AuthService.h

```cpp
#ifndef AUTH_SERVICE_H
#define AUTH_SERVICE_H

#include <string>
#include "../data_access/IRepository.h"
#include "../data_access/RolePermissionRepository.h"
#include "../models/User.h"

class AuthService
{
private:
    IRepository<User>& userRepo;
    RolePermissionRepository& permissionRepo;
    std::string currentUserId;

    const User* findByUsername(const std::string& username) const;
    bool hasUserPermission(const User& user,
                           const std::string& permission) const;

public:
    AuthService(IRepository<User>& userRepo,
                RolePermissionRepository& permissionRepo);

    bool login(const std::string& username,
               const std::string& password);
    void logout();

    const User* getCurrentUser() const;
    bool isLoggedIn() const;

    bool hasPermission(const std::string& permission) const;
    const User& requireLogin() const;
    const User& requirePermission(const std::string& permission) const;
};

#endif
```

### 4.2. AuthService.cpp

```cpp
#include "AuthService.h"
#include <stdexcept>

using namespace std;

AuthService::AuthService(IRepository<User>& userRepo,
                         RolePermissionRepository& permissionRepo)
    : userRepo(userRepo),
      permissionRepo(permissionRepo)
{
}

const User* AuthService::findByUsername(const string& username) const
{
    for (int i = 0; i < userRepo.count(); i++)
    {
        const User* user = userRepo.getAt(i);
        if (user != nullptr && user->getUsername() == username)
        {
            return user;
        }
    }
    return nullptr;
}

bool AuthService::hasUserPermission(const User& user,
                                    const string& permission) const
{
    if (!permissionRepo.isLoaded())
    {
        return false;
    }
    if (user.getRole() == UserRole::ADMIN)
    {
        return permissionRepo.hasPermission("ADMIN", permission);
    }
    if (user.getRole() == UserRole::STAFF)
    {
        return permissionRepo.hasPermission("STAFF", permission);
    }
    return false;
}

bool AuthService::login(const string& username,
                        const string& password)
{
    logout();
    if (!userRepo.isLoaded())
    {
        throw runtime_error("Chua nap du lieu tai khoan");
    }
    if (username.empty() || password.empty())
    {
        return false;
    }
    const User* user = findByUsername(username);
    if (user == nullptr || user->getPassword() != password || !user->isActive())
    {
        return false;
    }
    if (user->getRole() != UserRole::ADMIN && user->getRole() != UserRole::STAFF)
    {
        return false;
    }
    currentUserId = user->getUserId();
    return true;
}

void AuthService::logout()
{
    currentUserId.clear();
}

const User* AuthService::getCurrentUser() const
{
    if (currentUserId.empty() || !userRepo.isLoaded())
    {
        return nullptr;
    }
    const User* user = userRepo.findById(currentUserId);
    if (user == nullptr || !user->isActive())
    {
        return nullptr;
    }
    if (user->getRole() != UserRole::ADMIN && user->getRole() != UserRole::STAFF)
    {
        return nullptr;
    }
    return user;
}

bool AuthService::isLoggedIn() const
{
    return getCurrentUser() != nullptr;
}

bool AuthService::hasPermission(const string& permission) const
{
    const User* user = getCurrentUser();
    return user != nullptr && hasUserPermission(*user, permission);
}

const User& AuthService::requireLogin() const
{
    const User* user = getCurrentUser();
    if (user == nullptr)
    {
        throw runtime_error("Can dang nhap bang tai khoan dang hoat dong");
    }
    return *user;
}

const User& AuthService::requirePermission(const string& permission) const
{
    const User& user = requireLogin();
    if (!hasUserPermission(user, permission))
    {
        throw runtime_error("Khong co quyen: " + permission);
    }
    return user;
}
```

## 5. Giải thích từng hàm

### 5.1. Constructor

**Tổng quan:** Nhận tham chiếu hai Repository, không load hoặc cấp phát dữ liệu.

**Luồng hoạt động:**
```text
Nhận dependency -> khởi tạo hai tham chiếu -> currentUserId mặc định rỗng
```
**Giải thích chi tiết:** Initializer list `userRepo(userRepo)` và `permissionRepo(permissionRepo)` gắn tham chiếu thành viên với đối tượng caller truyền vào, không sao chép Repository. `currentUserId` là string mặc định rỗng. Thân hàm rỗng vì không cần thao tác khác. Không cần destructor riêng.

### 5.2. findByUsername()

**Tổng quan:** Nhận username bằng const reference, trả con trỏ User mượn hoặc nullptr. Caller login kiểm tra loaded trước.

**Luồng hoạt động:**
```text
Duyệt i từ 0 đến count()-1
  -> getAt(i) -> null? có: bỏ qua
              -> username trùng? có: trả User
Hết danh sách -> nullptr
```
**Giải thích chi tiết:** `count()` và `getAt()` thuộc interface chung. `const User*` không cho sửa User qua con trỏ. Toán tử `&&` đánh giá ngắn mạch, không gọi getter khi null. So sánh username chính xác, phân biệt hoa thường như Repository cũ. Không tìm thấy trả nullptr. Không delete con trỏ mượn. Độ phức tạp phụ thuộc triển khai getAt; không giả định truy cập chỉ số O(1).

### 5.3. hasUserPermission()

**Tổng quan:** Helper private nhận User đã xác thực và mã quyền, trả bool. Dùng chung cho hai API quyền để tránh truy vấn User lần thứ hai.

**Luồng hoạt động:**
```text
Bảng quyền loaded? không -> false
  có -> ADMIN? -> tra ADMIN|permission
     -> STAFF? -> tra STAFF|permission
     -> role khác -> false
```
**Giải thích chi tiết:** `isLoaded()` chặn bảng quyền chưa nạp hoặc nạp lại lỗi. Hai phép so sánh enum chỉ chọn khóa role để tra cứu, không cấp quyền mặc định. `permissionRepo.hasPermission()` trả kết quả thực tế từ bảng quyền. Role không hợp lệ bị từ chối. Helper không kiểm tra phiên độc lập vì chỉ được gọi sau getCurrentUser/requireLogin.

### 5.4. login()

**Tổng quan:** Nhận username/password, trả true khi xác thực thành công, false khi sai thông tin hoặc tài khoản không hợp lệ; ném runtime_error khi UserRepo chưa load.

**Luồng hoạt động:**
```text
logout -> UserRepo loaded? không -> throw
  có -> username/password rỗng? có -> false
      -> tìm username -> null/sai password/inactive? có -> false
      -> role hợp lệ? không -> false
      -> lưu ID -> true
```
**Giải thích chi tiết:** logout chạy trước mọi kiểm tra để lần đăng nhập thất bại không giữ phiên cũ. `empty()` kiểm chuỗi rỗng. Helper tìm tài khoản qua interface. `||` đánh giá ngắn mạch nên không dereference nullptr. So sánh password trực tiếp giữ hành vi hiện tại, không bổ sung hash trong phạm vi này. Role ADMIN/STAFF là loại tài khoản được hỗ trợ, không phải quyền nghiệp vụ. Chỉ sau tất cả kiểm tra mới gán ID. Không giữ con trỏ User lâu dài, không load/save tự động. Bảng quyền chưa load không ngăn xác thực danh tính nhưng ngăn thực hiện nghiệp vụ cần quyền.

### 5.5. logout()

**Tổng quan:** Không tham số, không trả giá trị, kết thúc phiên.

**Luồng hoạt động:**
```text
currentUserId.clear() -> kết thúc
```
**Giải thích chi tiết:** `clear()` đưa string về rỗng; gọi nhiều lần vẫn an toàn. Không xóa User, không sửa file hoặc bảng quyền.

### 5.6. getCurrentUser()

**Tổng quan:** Trả con trỏ const User mượn nếu phiên hợp lệ, ngược lại nullptr.

**Luồng hoạt động:**
```text
ID rỗng hoặc Repo chưa load? có -> nullptr
  không -> findById -> null/inactive? có -> nullptr
        -> role hợp lệ? không -> nullptr
        -> trả User
```
**Giải thích chi tiết:** Kiểm tra ID và trạng thái Repository trước khi truy vấn. `findById` lấy đối tượng hiện tại thay vì giữ con trỏ cũ. Kiểm tra active và role mỗi lần truy vấn để không cho tài khoản inactive thực hiện nghiệp vụ. Hàm const không sửa ID phiên. Vì vậy nếu cùng ID xuất hiện lại hoặc được kích hoạt lại, phiên có thể hợp lệ trở lại; đây là hành vi cũ cần xem xét ở bước quản lý tài khoản, chưa thay đổi. Con trỏ trả về chỉ hợp lệ trong vòng đời đối tượng Repository quản lý.

### 5.7. isLoggedIn()

**Tổng quan:** Trả bool trạng thái phiên hiện tại.

**Luồng hoạt động:**
```text
getCurrentUser() -> khác nullptr? -> true/false
```
**Giải thích chi tiết:** Tái sử dụng toàn bộ điều kiện xác thực phiên, không chỉ kiểm tra ID rỗng. Không kiểm tra Permission vì đăng nhập và phân quyền là hai việc khác nhau.

### 5.8. hasPermission()

**Tổng quan:** Nhận mã quyền, trả bool; phù hợp để GUI kiểm tra khả năng hiển thị chức năng.

**Luồng hoạt động:**
```text
getCurrentUser -> null? có -> false
  không -> hasUserPermission -> true/false
```
**Giải thích chi tiết:** Chỉ truy vấn tài khoản một lần. `&&` ngăn dereference null; `*user` truyền đối tượng theo tham chiếu const. Helper tra bảng quyền hiện tại, không cache quyền. Mã lạ/rỗng không có trong bảng hợp lệ nên false. Không bắt hoặc nuốt ngoại lệ bất thường từ dependency.

### 5.9. requireLogin()

**Tổng quan:** Trả const User& nếu phiên hợp lệ, ném runtime_error nếu không.

**Luồng hoạt động:**
```text
getCurrentUser -> null? có -> throw
  không -> return *user
```
**Giải thích chi tiết:** `<stdexcept>` cung cấp runtime_error. Dereference chỉ thực hiện sau kiểm tra null. Tham chiếu trả về không sao chép User và không chuyển quyền sở hữu. Không thay thế requirePermission cho nghiệp vụ cần quyền.

### 5.10. requirePermission()

**Tổng quan:** Nhận mã quyền, trả const User& khi được phép; ném runtime_error nếu chưa đăng nhập hoặc thiếu quyền.

**Luồng hoạt động:**
```text
requireLogin -> lỗi? -> ngoại lệ truyền lên caller
  thành công -> hasUserPermission -> false? -> throw
                                  -> true -> return user
```
**Giải thích chi tiết:** requireLogin xác thực và lấy User một lần. Helper dùng chính User đó thay vì gọi hasPermission rồi truy vấn lại. Thông báo thiếu quyền nối mã permission để caller biết quyền cần thiết. Trả cùng tham chiếu User để Service có thể lấy ID người thực hiện. Không tự cấp quyền ADMIN, không thay đổi phiên hoặc dữ liệu.

## 6. Thay đổi và điểm cần chốt

- Chuyển UserRepository sang IRepository<User>; giữ RolePermissionRepository cụ thể.
- Thêm helper findByUsername; giữ nguyên API UserRepository để các caller cũ dùng tới lượt refactor.
- Đề xuất thêm helper hasUserPermission (khác thiết kế sơ bộ một helper) để dùng chung logic và tránh truy vấn lặp.
- Loại bỏ isAdmin, isStaff, requireAdmin. Không sửa các Service chưa đến lượt; ghi nhận lỗi caller và xử lý sau.
- Giữ login/logout, thông báo lỗi, kiểm tra active, role và cơ chế ID phiên hiện tại.
- Không thay đổi TXT, Model, Repository, cơ chế mật khẩu hay tự động load/save.
- Giới hạn: DIP chỉ áp dụng cho UserRepo; chưa xử lý khả năng phiên hồi sinh khi ID được tái sử dụng/kích hoạt lại. Mật khẩu so sánh trực tiếp là hành vi cũ, không phải thiết kế bảo mật sản xuất.

## 7. Kiểm thử

Ngày 27/11/2026, `tests/test_auth_service.cpp` dùng TXT tạm và `IRepository<User>` giả lập để kiểm tra DIP/DI và số lần truy vấn; bảng quyền dùng Repository thật. Biên dịch C++17 với dependency cần thiết thành công. Kết quả: **AuthService Passed: 31, Failed: 0, Total: 31; MovieService Passed: 41, Failed: 0, Total: 41; RoomService Passed: 43, Failed: 0, Total: 43**.

- [x] Constructor DI qua `IRepository<User>&`, phiên ban đầu, logout lặp; gọi đủ 7 hàm public và gián tiếp kiểm tra 2 helper private.
- [x] UserRepo chưa load: login ném lỗi, phiên không hợp lệ; login ADMIN/STAFF, đầu vào rỗng/sai, tài khoản inactive và role không hợp lệ qua mock.
- [x] Login thất bại xóa phiên cũ, chuyển tài khoản; `getCurrentUser()`/`isLoggedIn()` sau logout, vô hiệu hóa, mất User và Repo không loaded.
- [x] Permission-only: ADMIN không mặc nhiên có quyền, STAFF được cấp theo bảng; chưa login, bảng quyền chưa load/rỗng, mã quyền lạ/rỗng và quyền thiếu đều bị từ chối.
- [x] Nạp lại bảng quyền cấp/thu hồi quyền; nạp lại thất bại do mã sai hoặc file thiếu không dùng quyền cũ.
- [x] `requireLogin()`/`requirePermission()` trả đúng User hoặc ném ngoại lệ; `requirePermission()` chỉ truy vấn ID một lần; mock `getAt()` trả nullptr được bỏ qua an toàn.
- [x] Hồi quy MovieService 41/41 và RoomService 43/43 sau refactor AuthService.
- [ ] Chưa kiểm thử tái sử dụng User ID sau khi xóa, mọi dạng TXT quyền sai định dạng và hồi quy GUI/toàn ứng dụng.

Biên dịch PowerShell (dự kiến, chạy từ thư mục gốc):

```powershell
$src = Get-ChildItem src/models,src/utils,src/data_access -Recurse -Filter *.cpp | ForEach-Object { $_.FullName }
g++ -std=c++17 tests/test_auth_service.cpp src/business/AuthService.cpp $src -o test_auth_service.exe
if ($LASTEXITCODE -eq 0) { .\test_auth_service.exe }
```

Đã biên dịch và chạy lần lượt ba bài test trên bằng C++17 với các module và dependency cần thiết; mỗi bài đều exit code 0. Không suy ra toàn bộ ứng dụng biên dịch thành công; BookingService, ReportService và các Service chưa refactor còn dùng API AuthService cũ, sẽ xử lý khi đến lượt.

## 8. Bàn giao và tiến độ

- [x] Rà soát API và dependency tại các commit đã nêu.
- [x] Soạn thiết kế Markdown và CHỐT MARKDOWN.
- [x] Triển khai `AuthService.h/.cpp` đúng bản chốt; đối chiếu hai file khớp Markdown 100%.
- [x] Biên dịch cú pháp C++17 ba Service và biên dịch/liên kết hai cấu hình Movie/Room cùng dependency cần thiết thành công.
- [x] Viết và chạy `tests/test_auth_service.cpp`: Passed 31, Failed 0, Total 31 ngày 27/11/2026; hoàn thành AuthService trong phạm vi đã kiểm thử.
- [x] Hồi quy MovieService: 41/41 PASS; RoomService: 43/43 PASS cùng ngày.
- [x] Cập nhật AuthService.md và tài liệu tiến độ theo kết quả thực tế.
- [ ] Commit/push theo quyết định của nhóm.

Service tiếp theo dự kiến StaffService. BookingService và các Service chưa refactor vẫn thuộc kế hoạch, không xử lý lỗi tương thích trước lượt. Không đánh dấu toàn bộ ứng dụng biên dịch thành công chỉ dựa vào test riêng.
