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

## Chi tiết triển khai từng hàm (AuthService.cpp)

### AuthService()

#### A. Tổng quan

- **Mục đích:** Khởi tạo Service với các Repository và AuthService.
- **Tham số:** `UserRepository& userRepo, RolePermissionRepository& permissionRepo) : userRepo(userRepo), permissionRepo(permissionRepo`.
- **Giá trị trả về:** `constructor` (không trả giá trị).
- **Điều kiện quan trọng:** điều kiện và lỗi thể hiện trực tiếp trong mã nguồn.

#### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Chuẩn bị và xử lý dữ liệu
    |
    v
Kết thúc
```

#### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1 — Chuẩn bị và xử lý dữ liệu.**

```cpp
AuthService::AuthService(UserRepository& userRepo, RolePermissionRepository& permissionRepo)
    : userRepo(userRepo),
      permissionRepo(permissionRepo)
{}
```

Các câu lệnh được thực thi theo đúng thứ tự trong `.cpp`; điều kiện và nhánh trả về nằm ngay trong code. Giá trị được khai báo ở đây được dùng trong các nhánh tiếp theo; hãy theo dõi nơi nó được trả về hoặc chuyển vào Repository để hiểu tác động lên trạng thái.

### login()

#### A. Tổng quan

- **Mục đích:** Kiểm tra dữ liệu đã nạp. Xem các nhánh cụ thể trong code.
- **Tham số:** `const string& username, const string& password`.
- **Giá trị trả về:** `bool`; các nhánh `return` được ghi đầy đủ bên dưới.
- **Điều kiện quan trọng:** kiểm tra trạng thái nạp và con trỏ trước khi dùng.

#### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Kiểm tra dữ liệu đã nạp
    |
    v
Tra cứu dữ liệu
    |
    v
Từ chối dữ liệu không hợp lệ
    |
    v
Từ chối dữ liệu không hợp lệ
    |
    v
Kết thúc
```

#### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1 — Kiểm tra dữ liệu đã nạp.**

```cpp
bool AuthService::login(const string& username, const string& password)
{
    // Xoa phien dang nhap cu
    logout();
    // Kiem tra du lieu da load
    if (!userRepo.isLoaded())
    {
        throw runtime_error("Chua nap du lieu tai khoan");
    }
    // Kiem tra thong tin dang nhap
```

`throw` chuyển lỗi cho bên gọi. Kiểm tra `isLoaded()` tránh dùng dữ liệu Repository chưa được nạp; nhánh không đạt kết thúc ngay theo giá trị `return` trong code. Ngoại lệ được chuyển cho caller xử lý; nó khác với `false` dùng cho trường hợp nghiệp vụ không đạt.

**Bước 2 — Tra cứu dữ liệu.**

```cpp
    if (username.empty() || password.empty())
    {
        return false;
    }
    // Tim tai khoan theo username
    const User* user = userRepo.findByUsername(username);
    if (user == nullptr)
    {
        return false;
    }
```

`nullptr` cho biết không có đối tượng để truy cập. `false` báo thao tác không thành công. Lệnh tra cứu trả con trỏ đến đối tượng đang được Repository quản lý. Con trỏ rỗng có nghĩa là không tìm thấy, nên bước sau chỉ được dùng đối tượng khi kiểm tra thành công. Nhánh `false` dừng hàm tại đây; những câu lệnh phía sau không chạy, nên không có cập nhật nào từ các bước sau.

**Bước 3 — Từ chối dữ liệu không hợp lệ.**

```cpp
    // Kiem tra mat khau
    if (user->getPassword() != password)
    {
        return false;
    }
    // Kiem tra trang thai tai khoan
    if (!user->isActive())
    {
        return false;
    }
```

`false` báo thao tác không thành công. Nhánh `false` dừng hàm tại đây; những câu lệnh phía sau không chạy, nên không có cập nhật nào từ các bước sau.

**Bước 4 — Từ chối dữ liệu không hợp lệ.**

```cpp
    // Kiem tra vai tro
    if (user->getRole() != UserRole::ADMIN && user->getRole() != UserRole::STAFF)
    {
        return false;
    }
    currentUserId = user->getUserId();
    return true;
}
```

`false` báo thao tác không thành công. Nhánh `false` dừng hàm tại đây; những câu lệnh phía sau không chạy, nên không có cập nhật nào từ các bước sau.

### logout()

#### A. Tổng quan

- **Mục đích:** Chuẩn bị và xử lý dữ liệu. Xem các nhánh cụ thể trong code.
- **Tham số:** `không có`.
- **Giá trị trả về:** `void`; các nhánh `return` được ghi đầy đủ bên dưới.
- **Điều kiện quan trọng:** điều kiện và lỗi thể hiện trực tiếp trong mã nguồn.

#### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Chuẩn bị và xử lý dữ liệu
    |
    v
Kết thúc
```

#### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1 — Chuẩn bị và xử lý dữ liệu.**

```cpp
void AuthService::logout()
{
    currentUserId.clear();
}
```

Các câu lệnh được thực thi theo đúng thứ tự trong `.cpp`; điều kiện và nhánh trả về nằm ngay trong code. Giá trị được khai báo ở đây được dùng trong các nhánh tiếp theo; hãy theo dõi nơi nó được trả về hoặc chuyển vào Repository để hiểu tác động lên trạng thái.

### getCurrentUser()

#### A. Tổng quan

- **Mục đích:** Kiểm tra dữ liệu đã nạp. Xem các nhánh cụ thể trong code.
- **Tham số:** `không có`.
- **Giá trị trả về:** `const User*`; các nhánh `return` được ghi đầy đủ bên dưới.
- **Điều kiện quan trọng:** kiểm tra trạng thái nạp và con trỏ trước khi dùng.

#### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Kiểm tra dữ liệu đã nạp
    |
    v
Trả kết quả
    |
    v
Trả kết quả
    |
    v
Kết thúc
```

#### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1 — Kiểm tra dữ liệu đã nạp.**

```cpp
const User* AuthService::getCurrentUser() const //lấy thông tin tài khoản đang đăng nhập vào hệ thống
{
    if (currentUserId.empty() || !userRepo.isLoaded())
    {
        return nullptr;
    }
    // Tim tai khoan hien tai
    const User* user = userRepo.findById(currentUserId);
    if (user == nullptr)
    {
```

`nullptr` cho biết không có đối tượng để truy cập. Kiểm tra `isLoaded()` tránh dùng dữ liệu Repository chưa được nạp; nhánh không đạt kết thúc ngay theo giá trị `return` trong code. Lệnh tra cứu trả con trỏ đến đối tượng đang được Repository quản lý. Con trỏ rỗng có nghĩa là không tìm thấy, nên bước sau chỉ được dùng đối tượng khi kiểm tra thành công. `nullptr` báo không có đối tượng phù hợp, để caller kiểm tra trước khi truy cập.

**Bước 2 — Trả kết quả.**

```cpp
        return nullptr;
    }
    // Kiem tra tai khoan con hoat dong
    if (!user->isActive())
    {
        return nullptr;
    }
    // Kiem tra vai tro
    if (user->getRole() != UserRole::ADMIN && user->getRole() != UserRole::STAFF)
    {
```

`nullptr` cho biết không có đối tượng để truy cập. `nullptr` báo không có đối tượng phù hợp, để caller kiểm tra trước khi truy cập.

**Bước 3 — Trả kết quả.**

```cpp
        return nullptr;
    }
    return user;
}
```

`nullptr` cho biết không có đối tượng để truy cập. `nullptr` báo không có đối tượng phù hợp, để caller kiểm tra trước khi truy cập.

### isLoggedIn()

#### A. Tổng quan

- **Mục đích:** Trả kết quả. Xem các nhánh cụ thể trong code.
- **Tham số:** `không có`.
- **Giá trị trả về:** `bool`; các nhánh `return` được ghi đầy đủ bên dưới.
- **Điều kiện quan trọng:** kiểm tra trạng thái nạp và con trỏ trước khi dùng.

#### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Trả kết quả
    |
    v
Kết thúc
```

#### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1 — Trả kết quả.**

```cpp
bool AuthService::isLoggedIn() const // kiểm tra hiện tại tài khoản có đăng nhập hợp lệ hay k
{
    return getCurrentUser() != nullptr;
}
```

`nullptr` cho biết không có đối tượng để truy cập. Giá trị được khai báo ở đây được dùng trong các nhánh tiếp theo; hãy theo dõi nơi nó được trả về hoặc chuyển vào Repository để hiểu tác động lên trạng thái.

### isAdmin()

#### A. Tổng quan

- **Mục đích:** Trả kết quả. Xem các nhánh cụ thể trong code.
- **Tham số:** `không có`.
- **Giá trị trả về:** `bool`; các nhánh `return` được ghi đầy đủ bên dưới.
- **Điều kiện quan trọng:** kiểm tra trạng thái nạp và con trỏ trước khi dùng.

#### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Trả kết quả
    |
    v
Kết thúc
```

#### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1 — Trả kết quả.**

```cpp
bool AuthService::isAdmin() const
{
    const User* user = getCurrentUser();
    return user != nullptr && user->getRole() == UserRole::ADMIN;
}
```

`nullptr` cho biết không có đối tượng để truy cập. Giá trị được khai báo ở đây được dùng trong các nhánh tiếp theo; hãy theo dõi nơi nó được trả về hoặc chuyển vào Repository để hiểu tác động lên trạng thái.

### isStaff()

#### A. Tổng quan

- **Mục đích:** Trả kết quả. Xem các nhánh cụ thể trong code.
- **Tham số:** `không có`.
- **Giá trị trả về:** `bool`; các nhánh `return` được ghi đầy đủ bên dưới.
- **Điều kiện quan trọng:** kiểm tra trạng thái nạp và con trỏ trước khi dùng.

#### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Trả kết quả
    |
    v
Kết thúc
```

#### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1 — Trả kết quả.**

```cpp
bool AuthService::isStaff() const
{
    const User* user = getCurrentUser();
    return user != nullptr && user->getRole() == UserRole::STAFF;
}
```

`nullptr` cho biết không có đối tượng để truy cập. Giá trị được khai báo ở đây được dùng trong các nhánh tiếp theo; hãy theo dõi nơi nó được trả về hoặc chuyển vào Repository để hiểu tác động lên trạng thái.

### requireLogin()

#### A. Tổng quan

- **Mục đích:** Kiểm tra quyền. Xem các nhánh cụ thể trong code.
- **Tham số:** `không có`.
- **Giá trị trả về:** `const User&`; các nhánh `return` được ghi đầy đủ bên dưới.
- **Điều kiện quan trọng:** Kiểm tra quyền qua `AuthService`; kiểm tra trạng thái nạp và con trỏ trước khi dùng.

#### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Kiểm tra quyền
    |
    v
Kết thúc
```

#### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1 — Kiểm tra quyền.**

```cpp
const User& AuthService::requireLogin() const //yêu cầu đăng nhập để thực hiện nghiệp vụ
{
    const User* user = getCurrentUser();
    if (user == nullptr) {
        throw runtime_error("Can dang nhap bang tai khoan dang hoat dong");
    }
    return *user;
}
```

`AuthService` kiểm tra quyền và có thể ném ngoại lệ. `nullptr` cho biết không có đối tượng để truy cập. `throw` chuyển lỗi cho bên gọi. `requireLogin()` bảo đảm có tài khoản đang đăng nhập trước khi truy vấn hoặc cập nhật. Ngoại lệ được chuyển cho caller xử lý; nó khác với `false` dùng cho trường hợp nghiệp vụ không đạt.

### requireAdmin()

#### A. Tổng quan

- **Mục đích:** Kiểm tra quyền. Xem các nhánh cụ thể trong code.
- **Tham số:** `không có`.
- **Giá trị trả về:** `const User&`; các nhánh `return` được ghi đầy đủ bên dưới.
- **Điều kiện quan trọng:** Kiểm tra quyền qua `AuthService`; điều kiện và lỗi thể hiện trực tiếp trong mã nguồn.

#### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Kiểm tra quyền
    |
    v
Kết thúc
```

#### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1 — Kiểm tra quyền.**

```cpp
const User& AuthService::requireAdmin() const
{
    const User& user = requireLogin();
    if (user.getRole() != UserRole::ADMIN) { 
       throw runtime_error("Thao tac yeu cau quyen Admin");
    }
    return user;
}
```

`AuthService` kiểm tra quyền và có thể ném ngoại lệ. `throw` chuyển lỗi cho bên gọi. `requireAdmin()` xác nhận vai trò Admin; khi không đạt, hàm dừng bằng ngoại lệ trước khi thay đổi dữ liệu. `requireLogin()` bảo đảm có tài khoản đang đăng nhập trước khi truy vấn hoặc cập nhật. Ngoại lệ được chuyển cho caller xử lý; nó khác với `false` dùng cho trường hợp nghiệp vụ không đạt.

### hasPermission()

#### A. Tổng quan

- **Mục đích:** Kiểm tra dữ liệu đã nạp. Xem các nhánh cụ thể trong code.
- **Tham số:** `const string& permission`.
- **Giá trị trả về:** `bool`; các nhánh `return` được ghi đầy đủ bên dưới.
- **Điều kiện quan trọng:** kiểm tra trạng thái nạp và con trỏ trước khi dùng.

#### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Kiểm tra dữ liệu đã nạp
    |
    v
Từ chối dữ liệu không hợp lệ
    |
    v
Trả kết quả
    |
    v
Kết thúc
```

#### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1 — Kiểm tra dữ liệu đã nạp.**

```cpp
bool AuthService::hasPermission(const string& permission) const
{
    const User* user = getCurrentUser();
    if (user == nullptr || !permissionRepo.isLoaded())
    {
        return false;
    }
    string role;
    if (user->getRole() == UserRole::ADMIN)
    {
```

`nullptr` cho biết không có đối tượng để truy cập. `false` báo thao tác không thành công. Kiểm tra `isLoaded()` tránh dùng dữ liệu Repository chưa được nạp; nhánh không đạt kết thúc ngay theo giá trị `return` trong code. Nhánh `false` dừng hàm tại đây; những câu lệnh phía sau không chạy, nên không có cập nhật nào từ các bước sau.

**Bước 2 — Từ chối dữ liệu không hợp lệ.**

```cpp
        role = "ADMIN";
    }
    else if (user->getRole() == UserRole::STAFF)
    {
        role = "STAFF";
    }
    else
    {
        return false;
    }
```

`false` báo thao tác không thành công. Nhánh `false` dừng hàm tại đây; những câu lệnh phía sau không chạy, nên không có cập nhật nào từ các bước sau.

**Bước 3 — Trả kết quả.**

```cpp
    return permissionRepo.hasPermission(role, permission);
}
```

Các câu lệnh được thực thi theo đúng thứ tự trong `.cpp`; điều kiện và nhánh trả về nằm ngay trong code. Giá trị được khai báo ở đây được dùng trong các nhánh tiếp theo; hãy theo dõi nơi nó được trả về hoặc chuyển vào Repository để hiểu tác động lên trạng thái.

### requirePermission()

#### A. Tổng quan

- **Mục đích:** Kiểm tra quyền. Xem các nhánh cụ thể trong code.
- **Tham số:** `const string& permission`.
- **Giá trị trả về:** `const User&`; các nhánh `return` được ghi đầy đủ bên dưới.
- **Điều kiện quan trọng:** Kiểm tra quyền qua `AuthService`; điều kiện và lỗi thể hiện trực tiếp trong mã nguồn.

#### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Kiểm tra quyền
    |
    v
Kết thúc
```

#### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1 — Kiểm tra quyền.**

```cpp
const User& AuthService::requirePermission(const string& permission) const
{
    const User& user = requireLogin();
    if (!hasPermission(permission))
    {
        throw runtime_error("Khong co quyen: " + permission);
    }
    return user;
}
```

`AuthService` kiểm tra quyền và có thể ném ngoại lệ. `throw` chuyển lỗi cho bên gọi. `requireLogin()` bảo đảm có tài khoản đang đăng nhập trước khi truy vấn hoặc cập nhật. Ngoại lệ được chuyển cho caller xử lý; nó khác với `false` dùng cho trường hợp nghiệp vụ không đạt.

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
| `MemberService` | Xem thành viên | `MEMBER_VIEW` |
| `MemberService` | Sửa thành viên | `MEMBER_UPDATE` |
| Nghiệp vụ bán vé | Bán vé | `TICKET_SELL` |

Bảng trên gồm các quyền đã dùng trong Service hiện có và quyền dành cho nghiệp vụ bán vé dự kiến; quyền có mã không xác nhận chức năng bán vé đã triển khai.

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
