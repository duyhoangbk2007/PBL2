# AuthService

## 1. Tổng quan

`AuthService` là lớp thuộc tầng **Business Logic**, chịu trách nhiệm xác thực tài khoản, quản lý phiên đăng nhập và kiểm tra quyền truy cập các chức năng của hệ thống.

Lớp sử dụng:

- `UserRepository`: Truy xuất thông tin tài khoản.
- `RolePermissionRepository`: Kiểm tra quyền được cấp cho từng vai trò.

`AuthService` không trực tiếp đọc hoặc ghi file TXT. Việc đọc dữ liệu được thực hiện thông qua các Repository.

**File liên quan:**

- `src/business/AuthService.h`
- `src/business/AuthService.cpp`
- `src/data_access/UserRepository.h`
- `src/data_access/RolePermissionRepository.h`
- `src/models/User.h`
- `data/role_permissions.txt`
- `tests/test_auth_permission.cpp`

## 2. Chức năng chính

`AuthService` thực hiện ba nhóm chức năng:

### 2.1. Xác thực tài khoản

- Đăng nhập bằng username và password.
- Kiểm tra tài khoản có tồn tại và đang hoạt động.
- Đăng xuất và xóa thông tin phiên đăng nhập.
- Truy xuất tài khoản đang đăng nhập.

### 2.2. Kiểm tra vai trò

Hệ thống hiện hỗ trợ hai vai trò:

- `ADMIN`
- `STAFF`

Các hàm `isAdmin()`, `isStaff()` và `requireAdmin()` được giữ lại để tương thích với code hiện tại.

### 2.3. Phân quyền RBAC

Hệ thống sử dụng cơ chế **Role-Based Access Control (RBAC)**.

Quyền truy cập được xác định dựa trên vai trò của người dùng và dữ liệu trong `role_permissions.txt`.

Hai hàm chính:

- `hasPermission()`: Kiểm tra người dùng có quyền hay không.
- `requirePermission()`: Yêu cầu người dùng phải có quyền trước khi thực hiện nghiệp vụ.

Các Service nghiệp vụ nên kiểm tra permission thay vì kiểm tra cứng vai trò.

## 3. Thuộc tính

```cpp
private:
    UserRepository& userRepo;
    RolePermissionRepository& permissionRepo;
    string currentUserId;
```

| Thuộc tính | Kiểu dữ liệu | Ý nghĩa |
|---|---|---|
| `userRepo` | `UserRepository&` | Tham chiếu đến Repository quản lý tài khoản |
| `permissionRepo` | `RolePermissionRepository&` | Tham chiếu đến Repository quản lý quyền |
| `currentUserId` | `string` | Lưu ID tài khoản đang đăng nhập |

### 3.1. Vì sao sử dụng tham chiếu?

`AuthService` không sở hữu dữ liệu tài khoản hoặc bảng quyền.

Hai Repository được khởi tạo bên ngoài và truyền vào constructor bằng tham chiếu.

Cách làm này giúp:

- Không sao chép dữ liệu Repository.
- Các Service có thể sử dụng chung một nguồn dữ liệu.
- Khi Repository cập nhật dữ liệu, `AuthService` sử dụng dữ liệu hiện tại.

Các Repository phải tồn tại trong suốt thời gian `AuthService` sử dụng chúng.

### 3.2. Vì sao chỉ lưu currentUserId?

Thay vì lưu con trỏ `User*`, `AuthService` chỉ lưu ID của tài khoản đăng nhập.

Khi cần thông tin người dùng, lớp sẽ gọi `userRepo.findById()`.

Nhờ đó, hệ thống có thể kiểm tra lại trạng thái tài khoản mỗi lần truy cập.

Nếu tài khoản bị vô hiệu hóa, `getCurrentUser()` sẽ trả về `nullptr`, kể cả khi tài khoản đã đăng nhập trước đó.

## 4. Constructor

```cpp
AuthService(
    UserRepository& userRepo,
    RolePermissionRepository& permissionRepo
);
```

**Chức năng:** Khởi tạo `AuthService` với hai Repository.

**Triển khai:**

```cpp
AuthService::AuthService(
    UserRepository& userRepo,
    RolePermissionRepository& permissionRepo
)
    : userRepo(userRepo),
      permissionRepo(permissionRepo)
{
}
```

Constructor không tự động gọi `load()`.

Việc nạp dữ liệu phải được thực hiện trước khi sử dụng các chức năng xác thực và phân quyền.

## 5. Các hàm xác thực

### 5.1. login()

```cpp
bool login(
    const string& username,
    const string& password
);
```

**Chức năng:** Đăng nhập vào hệ thống.

**Quy trình:**

1. Gọi `logout()` để xóa phiên đăng nhập cũ.
2. Kiểm tra `UserRepository` đã được load.
3. Kiểm tra username và password không rỗng.
4. Tìm tài khoản bằng `findByUsername()`.
5. Kiểm tra mật khẩu.
6. Kiểm tra tài khoản đang hoạt động.
7. Kiểm tra vai trò hợp lệ.
8. Lưu `userId` vào `currentUserId`.

**Kết quả:**

- `true`: Đăng nhập thành công.
- `false`: Thông tin đăng nhập không hợp lệ, tài khoản không tồn tại, bị khóa hoặc có vai trò không hợp lệ.
- Ném `runtime_error`: Dữ liệu tài khoản chưa được load.

**Lưu ý:** Phiên đăng nhập cũ bị xóa trước khi kiểm tra thông tin đăng nhập mới.

Phiên bản hiện tại so sánh mật khẩu trực tiếp với dữ liệu trong Repository. Chưa triển khai cơ chế băm mật khẩu.

### 5.2. logout()

```cpp
void logout();
```

**Chức năng:** Đăng xuất khỏi hệ thống.

**Triển khai:**

```cpp
void AuthService::logout()
{
    currentUserId.clear();
}
```

Sau khi đăng xuất:

- `isLoggedIn()` trả về `false`.
- `getCurrentUser()` trả về `nullptr`.
- `hasPermission()` trả về `false`.

### 5.3. getCurrentUser()

```cpp
const User* getCurrentUser() const;
```

**Chức năng:** Lấy thông tin tài khoản đang đăng nhập.

**Quy trình:**

1. Kiểm tra `currentUserId` không rỗng.
2. Kiểm tra `UserRepository` đã load.
3. Tìm tài khoản theo ID.
4. Kiểm tra tài khoản còn hoạt động.
5. Kiểm tra vai trò thuộc `ADMIN` hoặc `STAFF`.
6. Trả về con trỏ `const User*`.

Nếu bất kỳ điều kiện nào không hợp lệ, trả về `nullptr`.

Việc trả về con trỏ `const User*` giúp bên gọi không thể trực tiếp sửa dữ liệu người dùng thông qua kết quả này.

### 5.4. isLoggedIn()

```cpp
bool isLoggedIn() const;
```

**Chức năng:** Kiểm tra phiên đăng nhập hiện tại có hợp lệ không.

**Triển khai:**

```cpp
return getCurrentUser() != nullptr;
```

Hàm không chỉ kiểm tra `currentUserId` có rỗng hay không mà còn kiểm tra tài khoản có tồn tại và đang hoạt động.

## 6. Các hàm kiểm tra vai trò

### 6.1. isAdmin()

```cpp
bool isAdmin() const;
```

Trả về `true` nếu tài khoản hiện tại hợp lệ và có vai trò `UserRole::ADMIN`.

### 6.2. isStaff()

```cpp
bool isStaff() const;
```

Trả về `true` nếu tài khoản hiện tại hợp lệ và có vai trò `UserRole::STAFF`.

### 6.3. requireLogin()

```cpp
const User& requireLogin() const;
```

**Chức năng:** Yêu cầu tài khoản phải đăng nhập hợp lệ.

- Hợp lệ: Trả về `const User&`.
- Không hợp lệ: Ném `runtime_error`.

Hàm được tái sử dụng trong `requireAdmin()` và `requirePermission()`.

### 6.4. requireAdmin()

```cpp
const User& requireAdmin() const;
```

**Chức năng:** Yêu cầu người dùng phải có vai trò `ADMIN`.

**Quy trình:**

1. Gọi `requireLogin()`.
2. Kiểm tra `UserRole::ADMIN`.
3. Nếu không phải Admin, ném `runtime_error`.
4. Nếu hợp lệ, trả về `const User&`.

**Lưu ý:** `requireAdmin()` kiểm tra vai trò trực tiếp, không sử dụng bảng permission.

Hàm được giữ lại để tương thích với các Service cũ. Khi tích hợp RBAC vào nghiệp vụ, nên thay bằng `requirePermission()` tại những vị trí phù hợp.

## 7. Các hàm phân quyền RBAC

### 7.1. hasPermission()

```cpp
bool hasPermission(
    const string& permission
) const;
```

**Chức năng:** Kiểm tra tài khoản hiện tại có permission được yêu cầu hay không.

**Quy trình:**

1. Gọi `getCurrentUser()`.
2. Nếu không có tài khoản hợp lệ, trả về `false`.
3. Kiểm tra `RolePermissionRepository` đã load.
4. Xác định vai trò của người dùng.
5. Chuyển `UserRole` thành chuỗi `ADMIN` hoặc `STAFF`.
6. Gọi `permissionRepo.hasPermission(role, permission)`.
7. Trả về kết quả kiểm tra.

**Ví dụ:**

```cpp
auth.hasPermission("MOVIE_ADD");
```

Nếu người dùng đang đăng nhập có vai trò `STAFF`, hàm sẽ kiểm tra:

```cpp
permissionRepo.hasPermission("STAFF", "MOVIE_ADD");
```

**Các trường hợp trả về `false`:**

- Chưa đăng nhập.
- Tài khoản không tồn tại hoặc bị vô hiệu hóa.
- Repository phân quyền chưa load.
- Vai trò không hợp lệ.
- Permission không được cấp cho vai trò.

Hàm không tự động cho phép Admin thực hiện mọi thao tác.

Admin cũng phải được cấp permission tương ứng trong file TXT.

### 7.2. requirePermission()

```cpp
const User& requirePermission(
    const string& permission
) const;
```

**Chức năng:** Yêu cầu tài khoản phải có permission trước khi thực hiện nghiệp vụ.

**Triển khai:**

```cpp
const User& AuthService::requirePermission(
    const string& permission
) const
{
    const User& user = requireLogin();

    if (!hasPermission(permission))
    {
        throw runtime_error("Khong co quyen: " + permission);
    }

    return user;
}
```

**Giải thích:**

- `requireLogin()`: Đảm bảo tài khoản đang đăng nhập hợp lệ.
- `hasPermission()`: Kiểm tra permission theo vai trò.
- `throw runtime_error(...)`: Từ chối thao tác nếu thiếu quyền.
- `return user`: Trả về tài khoản nếu được phép.

**Ví dụ:**

```cpp
auth.requirePermission("MOVIE_ADD");
```

Nếu người dùng không có quyền, hàm ném ngoại lệ:

```text
Khong co quyen: MOVIE_ADD
```

Nếu chưa đăng nhập, ngoại lệ đến từ `requireLogin()`.

**Phân biệt hai hàm:**

| Tiêu chí | `hasPermission()` | `requirePermission()` |
|---|---|---|
| Mục đích | Kiểm tra quyền | Bắt buộc có quyền |
| Kết quả | `bool` | `const User&` |
| Thiếu quyền | `false` | Ném ngoại lệ |
| Chưa đăng nhập | `false` | Ném ngoại lệ |
| Sử dụng | Kiểm tra điều kiện, hiển thị chức năng | Bảo vệ nghiệp vụ trong Service |

## 8. Cơ chế phân quyền bằng file TXT

File:

```text
data/role_permissions.txt
```

Định dạng mỗi dòng:

```text
ROLE|PERMISSION_CODE
```

Ví dụ:

```text
ADMIN|MOVIE_VIEW
ADMIN|MOVIE_ADD
ADMIN|MOVIE_UPDATE
ADMIN|MOVIE_DELETE
STAFF|MOVIE_VIEW
STAFF|TICKET_SELL
```

Mỗi dòng biểu diễn một permission được cấp cho một vai trò.

### 8.1. Cấp quyền

Muốn Staff được thêm phim, bổ sung:

```text
STAFF|MOVIE_ADD
```

Sau khi nạp lại `RolePermissionRepository`, các tài khoản Staff hợp lệ sẽ có quyền `MOVIE_ADD`.

### 8.2. Thu hồi quyền

Muốn Staff không được thêm phim, xóa dòng:

```text
STAFF|MOVIE_ADD
```

Sau khi nạp lại bảng quyền, `hasPermission("MOVIE_ADD")` sẽ trả về `false` đối với Staff.

### 8.3. Nguyên tắc phân quyền

- Quyền được cấp theo vai trò, không cấp riêng cho từng tài khoản.
- Admin không tự động có toàn bộ quyền.
- Permission không có trong bảng quyền sẽ bị từ chối.
- Bảng quyền chưa load thì không cho phép thực hiện nghiệp vụ cần permission.
- Thay đổi file TXT cần được nạp lại để có hiệu lực.
- `AuthService` không tự theo dõi thay đổi file TXT.

## 9. Tích hợp vào các Service nghiệp vụ

Các Service nên yêu cầu permission tương ứng với từng thao tác.

**Ví dụ cách kiểm tra cũ:**

```cpp
auth.requireAdmin();
```

**Cách kiểm tra theo RBAC:**

```cpp
auth.requirePermission("MOVIE_ADD");
```

Ví dụ về ánh xạ permission:

| Service | Nghiệp vụ | Permission |
|---|---|---|
| `MovieService` | Xem phim | `MOVIE_VIEW` |
| `MovieService` | Thêm phim | `MOVIE_ADD` |
| `MovieService` | Sửa phim | `MOVIE_UPDATE` |
| `MovieService` | Xóa phim | `MOVIE_DELETE` |
| `RoomService` | Thêm phòng | `ROOM_ADD` |
| `SeatService` | Sửa ghế | `SEAT_UPDATE` |
| `ShowtimeService` | Tạo suất chiếu | `SHOWTIME_ADD` |
| `ShowtimeService` | Hủy suất chiếu | `SHOWTIME_CANCEL` |
| `MemberService` | Thêm thành viên | `MEMBER_ADD` |
| Nghiệp vụ bán vé | Bán vé | `TICKET_SELL` |

Bảng trên thể hiện cách tích hợp dự kiến, không có nghĩa tất cả Service đã được cập nhật.

**Quan trọng:** Phân quyền RBAC không thay thế các quy tắc nghiệp vụ.

Ví dụ, dù có quyền `SHOWTIME_CANCEL`, người dùng vẫn phải đáp ứng các điều kiện hủy suất chiếu theo quy tắc nghiệp vụ.

Tương tự, quyền `INVOICE_VIEW_OWN` không tự kiểm tra hóa đơn có thuộc về người dùng hiện tại hay không. Service quản lý hóa đơn vẫn phải kiểm tra quyền sở hữu dữ liệu.

## 10. Thứ tự khởi tạo và load

Ví dụ:

```cpp
UserRepository userRepo("data/users.txt");

RolePermissionRepository permissionRepo(
    "data/role_permissions.txt"
);

if (!userRepo.load() || !permissionRepo.load())
{
    throw runtime_error("Khong the nap du lieu xac thuc");
}

AuthService auth(userRepo, permissionRepo);
```

**Thứ tự đề xuất:**

1. Khởi tạo `UserRepository`.
2. Khởi tạo `RolePermissionRepository`.
3. Load dữ liệu tài khoản.
4. Load dữ liệu phân quyền.
5. Khởi tạo `AuthService`.
6. Sử dụng `AuthService` trong các Service nghiệp vụ.

`AuthService` không tự load dữ liệu và không sở hữu vòng đời của các Repository.

## 11. Kiểm thử

File test:

```text
tests/test_auth_permission.cpp
```

Các nhóm kiểm thử:

1. Trạng thái chưa đăng nhập.
2. Đăng nhập Admin.
3. Kiểm tra quyền Admin.
4. Đăng nhập Staff.
5. Kiểm tra quyền Staff.
6. Từ chối thao tác khi thiếu permission.
7. Đăng nhập sai và đăng xuất.
8. Tài khoản bị vô hiệu hóa.
9. Repository phân quyền chưa load.
10. Cấp và thu hồi quyền thông qua TXT rồi load lại.

Test sử dụng các file TXT tạm để không thay đổi dữ liệu thật trong `data/`.

### 11.1. Biên dịch bằng PowerShell

Chạy từ thư mục gốc PBL2:

```powershell
g++ -std=c++17 -Wall -Wextra -I./src tests/test_auth_permission.cpp src/models/*.cpp src/data_access/*.cpp src/utils/*.cpp src/business/AuthService.cpp -o test_auth_permission.exe
```

### 11.2. Chạy test

```powershell
.\test_auth_permission.exe
```

### 11.3. Kết quả

Bộ test RBAC đã được chạy và người phát triển xác nhận tất cả test đều pass.

## 12. Các lưu ý và giới hạn hiện tại

1. Hệ thống chỉ hỗ trợ hai vai trò `ADMIN` và `STAFF`.
2. Permission được cấu hình bằng file TXT, chưa có giao diện cấp hoặc thu hồi quyền.
3. Không cấp permission riêng cho từng tài khoản.
4. Quyền mới có hiệu lực sau khi Repository nạp lại dữ liệu.
5. `requireAdmin()` vẫn tồn tại để tương thích với code cũ.
6. Các Service nghiệp vụ cần được cập nhật để sử dụng `requirePermission()`.
7. Việc kiểm tra quyền phải được thực hiện tại tầng Business Logic, không chỉ tại giao diện.
8. `AuthService` hiện quản lý một phiên đăng nhập bằng `currentUserId`.
9. Mật khẩu hiện được so sánh trực tiếp, chưa sử dụng cơ chế băm mật khẩu.
10. `RolePermissionRepository` hiện kiểm tra định dạng dòng, vai trò và dòng trùng; chưa đối chiếu mã permission với toàn bộ danh sách mã quyền hợp lệ.

## 13. Kết luận

`AuthService` đã được mở rộng từ lớp xác thực và kiểm tra vai trò thành lớp hỗ trợ phân quyền RBAC.

Hai hàm mới:

```cpp
bool hasPermission(const string& permission) const;

const User& requirePermission(const string& permission) const;
```

giúp các Service kiểm tra quyền dựa trên cấu hình thay vì phụ thuộc trực tiếp vào `ADMIN` hoặc `STAFF`.

**Trạng thái hiện tại:**

- Hoàn thành tích hợp `RolePermissionRepository` vào `AuthService`.
- Hoàn thành kiểm thử RBAC.
- Chưa hoàn thành việc thay thế kiểm tra vai trò bằng kiểm tra permission trong toàn bộ các Service nghiệp vụ.

**Công việc tiếp theo:** Tích hợp `requirePermission()` vào `MovieService`, kiểm thử lại và cập nhật tài liệu liên quan.