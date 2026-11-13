# AuthService

## 1. Tổng quan

`AuthService` là lớp thuộc tầng Business Logic, chịu trách nhiệm xác thực tài khoản, quản lý phiên đăng nhập hiện tại và kiểm tra quyền truy cập.

Lớp sử dụng `UserRepository` để truy xuất dữ liệu tài khoản, không trực tiếp đọc hoặc ghi file TXT.

**File liên quan:**
- `src/business/AuthService.h`
- `src/business/AuthService.cpp`
- `src/data_access/UserRepository.h`

## 2. Thuộc tính

### `UserRepository& userRepo`

Tham chiếu đến `UserRepository`, dùng để tìm kiếm tài khoản và kiểm tra trạng thái dữ liệu.

AuthService không sở hữu Repository, vì vậy `UserRepository` phải tồn tại lâu hơn `AuthService`.

### `string currentUserId`

Lưu ID của tài khoản đang đăng nhập.

- Chuỗi rỗng: chưa đăng nhập hoặc đã đăng xuất.
- Có ID: đã đăng nhập thành công trước đó, nhưng vẫn cần kiểm tra tài khoản còn hợp lệ.

AuthService lưu ID thay vì lưu trực tiếp con trỏ `User*`.

## 3. Các hàm

### 3.1. Constructor

```cpp
AuthService(UserRepository& userRepo);
```

Nhận tham chiếu đến `UserRepository` thông qua danh sách khởi tạo.

Không tự tạo Repository hoặc tự load dữ liệu.

### 3.2. `login()`

```cpp
bool login(const string& username, const string& password);
```

**Chức năng:** Xác thực tài khoản và tạo phiên đăng nhập.

**Quy trình:**

1. Gọi `logout()` để xóa phiên đăng nhập cũ.
2. Kiểm tra `UserRepository` đã load.
3. Kiểm tra username và password không rỗng.
4. Tìm tài khoản bằng `findByUsername()`.
5. Kiểm tra mật khẩu, trạng thái `active` và role.
6. Nếu hợp lệ, lưu `userId` vào `currentUserId`.

**Kết quả:**
- `true`: Đăng nhập thành công.
- `false`: Thông tin không hợp lệ hoặc tài khoản không hoạt động.
- Ném `runtime_error`: Repository chưa load.

**Lưu ý:** Mỗi lần gọi `login()` đều xóa phiên cũ, kể cả khi đăng nhập mới thất bại.

### 3.3. `logout()`

```cpp
void logout();
```

Xóa `currentUserId` bằng `clear()`.

Không xóa đối tượng User khỏi Repository.

### 3.4. `getCurrentUser()`

```cpp
const User* getCurrentUser() const;
```

**Chức năng:** Lấy tài khoản đang đăng nhập và kiểm tra tài khoản còn hợp lệ.

**Quy trình:**

1. Kiểm tra `currentUserId` và trạng thái load của Repository.
2. Tìm User bằng `findById(currentUserId)`.
3. Kiểm tra tài khoản còn hoạt động và role hợp lệ.
4. Trả về con trỏ User hoặc `nullptr`.

Sử dụng `const User*` vì AuthService chỉ cần đọc thông tin tài khoản.

Hàm này được sử dụng chung bởi các hàm kiểm tra đăng nhập và phân quyền.

### 3.5. `isLoggedIn()`

```cpp
bool isLoggedIn() const;
```

Trả về `true` nếu `getCurrentUser()` khác `nullptr`, ngược lại trả về `false`.

### 3.6. `isAdmin()`

```cpp
bool isAdmin() const;
```

Kiểm tra tài khoản hiện tại có role `ADMIN` hay không.

Trả về `false` nếu chưa đăng nhập hoặc tài khoản không hợp lệ.

### 3.7. `isStaff()`

```cpp
bool isStaff() const;
```

Tương tự `isAdmin()`, nhưng kiểm tra role `STAFF`.

Chỉ trả về `true` đối với tài khoản có role `STAFF`.

### 3.8. `requireLogin()`

```cpp
const User& requireLogin() const;
```

**Chức năng:** Bắt buộc người dùng phải đăng nhập hợp lệ trước khi thực hiện nghiệp vụ.

- Gọi `getCurrentUser()` để lấy tài khoản.
- Nếu trả về `nullptr`, ném `runtime_error`.
- Nếu hợp lệ, trả về `*user`.

Sử dụng `const User&` để trả về tham chiếu đến User mà không tạo bản sao.

Ví dụ:

```cpp
const User& user = authService.requireLogin();
string userId = user.getUserId();
```

### 3.9. `requireAdmin()`

```cpp
const User& requireAdmin() const;
```

**Chức năng:** Bắt buộc tài khoản hiện tại có quyền Admin.

- Gọi `requireLogin()` để kiểm tra đăng nhập.
- Kiểm tra role bằng `getRole()`.
- Nếu không phải Admin, ném `runtime_error`.
- Nếu hợp lệ, trả về tham chiếu User.

Dùng cho các nghiệp vụ chỉ Admin được phép thực hiện.

## 4. Luồng hoạt động

```text
UserRepository
      |
      v
  AuthService
      |
      +-- login()
      |      |
      |      +-- findByUsername()
      |      +-- Kiem tra thong tin
      |      +-- Luu currentUserId
      |
      +-- getCurrentUser()
      |      |
      |      +-- findById()
      |      +-- Kiem tra active va role
      |
      +-- isLoggedIn()
      +-- isAdmin()
      +-- isStaff()
      +-- requireLogin()
      +-- requireAdmin()
      |
      +-- logout()
```

## 5. Lưu ý

- Phải load `UserRepository` trước khi đăng nhập.
- AuthService chỉ quản lý một phiên đăng nhập tại một thời điểm.
- Trạng thái đăng nhập chỉ được lưu trong bộ nhớ, không tự khôi phục khi mở lại ứng dụng.
- Tài khoản bị vô hiệu hóa sẽ không còn được xem là đăng nhập hợp lệ.
- AuthService không trực tiếp thay đổi thông tin tài khoản.
- Các hàm `requireLogin()` và `requireAdmin()` có thể ném ngoại lệ, nên lớp gọi cần xử lý phù hợp.