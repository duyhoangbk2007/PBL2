# StaffService

## 1. Tổng quan

`StaffService` là lớp thuộc tầng **Business Logic**, chịu trách nhiệm xử lý nghiệp vụ quản lý tài khoản nhân viên trong hệ thống rạp chiếu phim.

Lớp sử dụng `UserRepository` để truy xuất, thêm, cập nhật và lưu dữ liệu tài khoản; sử dụng `AuthService` để kiểm tra quyền Admin trước các thao tác quản lý nhân viên.

`StaffService` **không trực tiếp đọc/ghi file TXT** và **không có `StaffRepository` riêng**. Tên thuộc tính `staffRepo` là tham chiếu đến một đối tượng `UserRepository`, trong đó có cả tài khoản Admin và Staff.

**File liên quan:**
- `src/business/StaffService.h`
- `src/business/StaffService.cpp`
- `src/business/AuthService.h`
- `src/business/AuthService.cpp`
- `src/data_access/UserRepository.h`
- `src/data_access/UserRepository.cpp`
- `src/models/User.h`
- `src/models/User.cpp`
- `tests/test_staff_service.cpp`

## 2. Trách nhiệm và phạm vi

- Chỉ cho phép Admin quản lý và tra cứu nhân viên.
- Chỉ thao tác trên các `User` có `UserRole::STAFF`; không cho dùng StaffService để quản lý Admin.
- Thêm nhân viên, cập nhật thông tin, vô hiệu hóa tài khoản.
- Tìm kiếm theo ID hoặc username; duyệt và đếm riêng tài khoản Staff.
- Ủy quyền việc kiểm tra định dạng, trùng ID/username, lưu/nạp dữ liệu cho `UserRepository`.
- Không xóa vật lý tài khoản nhân viên để giữ thông tin phục vụ lịch sử nghiệp vụ.

## 3. Thuộc tính và khởi tạo

```cpp
private:
    UserRepository& staffRepo;
    AuthService& authService;
```

```cpp
StaffService(UserRepository& staffRepo, AuthService& authService);
```

Hai thuộc tính là **tham chiếu**, không tạo Repository hoặc AuthService mới. `AuthService` và `StaffService` cần sử dụng cùng đối tượng `UserRepository` để trạng thái đăng nhập và dữ liệu tài khoản đồng bộ.

Ví dụ:

```cpp
UserRepository userRepo("data/users.txt");
AuthService authService(userRepo);
StaffService staffService(userRepo, authService);

userRepo.load(); // Nap UserRepository mot lan truoc khi dang nhap
```

## 4. Phân quyền

| Thao tác | Chưa đăng nhập | Staff | Admin |
| --- | --- | --- | --- |
| Thêm nhân viên | Từ chối | Từ chối | Cho phép nếu hợp lệ |
| Cập nhật nhân viên | Từ chối | Từ chối | Cho phép nếu hợp lệ |
| Vô hiệu hóa nhân viên | Từ chối | Từ chối | Cho phép nếu hợp lệ |
| Tìm kiếm, lấy theo vị trí, đếm nhân viên | Từ chối | Từ chối | Cho phép |

Các hàm nghiệp vụ và tra cứu gọi `authService.requireAdmin()` trước khi xử lý. Nếu chưa đăng nhập hoặc không có quyền Admin, hàm **ném `std::runtime_error`**, không trả `false` hay `nullptr`.

**Lưu ý:** `load()` và `save()` trong phiên bản hiện tại chỉ chuyển tiếp đến Repository, **không gọi `requireAdmin()`**. Việc khởi tạo/nạp dữ liệu nên do luồng khởi động ứng dụng quản lý.

## 5. Danh sách hàm

| Hàm | Kiểu trả về | Chức năng |
| --- | --- | --- |
| `StaffService(UserRepository&, AuthService&)` | Constructor | Nhận các phụ thuộc dùng chung |
| `load()` | `bool` | Nạp dữ liệu User qua Repository |
| `save()` | `bool` | Lưu dữ liệu User qua Repository |
| `addStaff(User* staff)` | `bool` | Thêm tài khoản nhân viên |
| `updateStaff(const User& newValues)` | `bool` | Sửa thông tin nhân viên |
| `deactivateStaff(const string& staffId)` | `bool` | Vô hiệu hóa nhân viên |
| `findStaffById(const string& staffId)` | `const User*` | Tìm nhân viên theo ID |
| `findStaffByUsername(const string& username)` | `const User*` | Tìm nhân viên theo username |
| `getStaffAt(int index)` | `const User*` | Lấy nhân viên tại chỉ số trong danh sách Staff |
| `getStaffCount()` | `int` | Đếm tổng số tài khoản Staff |

## 6. Chi tiết các hàm

### 6.1. `load()`

```cpp
bool StaffService::load()
{
    return staffRepo.load();
}
```

Nạp **toàn bộ User**, gồm cả Admin và Staff. Theo `UserRepository` hiện tại, gọi `load()` khi đã nạp dữ liệu sẽ trả `false`. Không nên gọi lại từ nhiều Service nếu cùng dùng một Repository.

### 6.2. `save()`

```cpp
bool StaffService::save()
{
    return staffRepo.save();
}
```

Lưu **toàn bộ User** xuống file TXT thông qua Repository, không chỉ tài khoản Staff. Các hàm thêm/sửa/vô hiệu hóa cập nhật dữ liệu trong bộ nhớ; cần gọi `save()` để ghi xuống file.

### 6.3. `addStaff(User* staff)`

**Quy trình:**
1. Kiểm tra quyền Admin.
2. Từ chối con trỏ `nullptr` hoặc User không có role `STAFF`.
3. Gọi `staffRepo.add(staff)`.

`UserRepository::add()` chịu trách nhiệm kiểm tra dữ liệu hợp lệ, trùng ID, trùng username và trạng thái đã load.

**Kết quả:** `true` nếu thêm thành công, `false` nếu không hợp lệ; ngoại lệ nếu thiếu quyền. Khi thêm thành công, Repository sở hữu con trỏ `staff`; khi thêm thất bại, bên gọi vẫn phải quản lý và giải phóng con trỏ đó.

### 6.4. `updateStaff(const User& newValues)`

**Quy trình:**
1. Kiểm tra quyền Admin và Repository đã load.
2. Tìm tài khoản hiện tại bằng `newValues.getUserId()`.
3. Từ chối nếu không tồn tại hoặc role hiện tại/role mới không phải `STAFF`.
4. Từ chối nếu `active` mới khác `active` hiện tại.
5. Gọi `staffRepo.update(newValues)`.

Có thể cập nhật username, password và fullName. Không cho phép thay đổi role hoặc trạng thái `active` thông qua hàm này. Repository tiếp tục kiểm tra định dạng và trùng username. Nhân viên đã vô hiệu hóa vẫn có thể sửa thông tin nếu giữ nguyên `active = false`.

### 6.5. `deactivateStaff(const string& staffId)`

**Quy trình:**
1. Kiểm tra quyền Admin và Repository đã load.
2. Tìm User theo ID.
3. Từ chối nếu không tồn tại, không phải Staff hoặc đã vô hiệu hóa.
4. Tạo đối tượng `User updated` với ID, username, password, fullName, role giữ nguyên và `active = false`.
5. Gọi `staffRepo.update(updated)`.

Việc tạo đối tượng `updated` **không thêm User mới**. Đây là dữ liệu tạm để Repository cập nhật User hiện có. Tài khoản không bị xóa và vẫn có thể tra cứu; `AuthService::login()` sẽ từ chối tài khoản không hoạt động.

### 6.6. `findStaffById(const string& staffId)`

Kiểm tra quyền Admin, gọi `staffRepo.findById(staffId)`, sau đó lọc role `STAFF`. Trả `nullptr` nếu không tìm thấy hoặc ID thuộc Admin. Kết quả `const User*` chỉ dùng để đọc thông tin qua con trỏ trả về.

### 6.7. `findStaffByUsername(const string& username)`

Kiểm tra quyền Admin, gọi `staffRepo.findByUsername(username)` và lọc role `STAFF`. Trả `nullptr` nếu không tìm thấy hoặc tài khoản là Admin.

### 6.8. `getStaffAt(int index)`

Kiểm tra quyền Admin; từ chối chỉ số âm. Duyệt tất cả User bằng `staffRepo.getAt(i)` và dùng biến `staffIndex` chỉ tăng khi gặp role `STAFF`. Khi `staffIndex == index`, trả về User tương ứng. Trả `nullptr` nếu vượt phạm vi.

**Chú ý:** `index` là vị trí trong **danh sách Staff đã lọc**, không phải vị trí trong toàn bộ UserRepository; bắt đầu từ 0. Nhân viên không hoạt động vẫn nằm trong danh sách.

### 6.9. `getStaffCount()`

Kiểm tra quyền Admin, duyệt toàn bộ User và đếm những User có role `STAFF`. Kết quả **bao gồm cả nhân viên đã vô hiệu hóa**, không bao gồm Admin.

## 7. Quy tắc dữ liệu và lưu trữ

- Tài khoản Staff dùng model `User`, role `UserRole::STAFF`.
- ID User theo định dạng `USR` + 4 chữ số, khác `0000` (ví dụ `USR0002`).
- Username, password, fullName phải là văn bản hợp lệ theo `Validator`; username không được có dấu cách ở đầu/cuối.
- ID và username không được trùng với **bất kỳ** tài khoản User nào, kể cả Admin.
- `updateStaff()` không thay đổi ID, role hoặc `active`.
- `deactivateStaff()` đổi `active` sang `false` thay vì xóa tài khoản.
- File `users.txt` chứa cả Admin và Staff; `StaffService` không trực tiếp thao tác file.

## 8. Ví dụ sử dụng

```cpp
UserRepository userRepo("data/users.txt");
AuthService authService(userRepo);
StaffService staffService(userRepo, authService);

if (!staffService.load())
{
    // Xu ly loi nap du lieu
}

if (authService.login("admin01", "123456"))
{
    User* staff = new User(
        "USR0005", "staff05", "123456",
        "Nhan Vien Moi", UserRole::STAFF, true
    );

    if (!staffService.addStaff(staff))
    {
        delete staff; // Repository chua nhan quyen so huu
    }

    staffService.deactivateStaff("USR0005");
    staffService.save();
}
```

Đoạn ví dụ giả định tài khoản Admin tồn tại, đăng nhập thành công và ID/username nhân viên chưa trùng. Trong ứng dụng thực tế, cần xử lý ngoại lệ và kết quả trả về của từng thao tác.

## 9. Kiểm thử

**File:** `tests/test_staff_service.cpp`

**Mốc do người phát triển xác nhận trước đây:** 59/59 PASS, 0 FAIL. **Lần kiểm tra trên nhánh hiện tại:** biên dịch và chạy C++17, **67/67 PASS, 0 FAIL**, thoát mã 0. Hai kết quả thuộc hai lần chạy khác nhau.

Các nhóm đã kiểm thử:
- Nạp/lưu dữ liệu và không nạp lại khi đã load.
- Phân quyền khi chưa đăng nhập, Staff và Admin.
- Thêm Staff hợp lệ; từ chối dữ liệu lỗi, ID/username trùng và role Admin.
- Cập nhật Staff; không cho sửa Admin, đổi role hoặc đổi `active`.
- Vô hiệu hóa Staff; từ chối tài khoản Admin, không tồn tại hoặc đã vô hiệu hóa.
- Tra cứu theo ID, username, vị trí và đếm Staff.
- Lưu rồi nạp lại dữ liệu; xác minh trạng thái vô hiệu hóa và thông tin đăng nhập.

Biên dịch bằng PowerShell từ thư mục gốc:

```powershell
$src = Get-ChildItem -Path src -Recurse -Filter *.cpp |
    Where-Object { $_.Name -ne "main.cpp" } |
    ForEach-Object { $_.FullName }

g++ -std=c++17 -Wall -Wextra -I . -I src $src tests/test_staff_service.cpp -o test_staff_service.exe

if ($LASTEXITCODE -eq 0) {
    .\test_staff_service.exe
}
```

## 10. Lưu ý và hướng mở rộng

- `StaffService` dùng chung `UserRepository` với `AuthService`; cần load Repository một lần trước khi đăng nhập.
- `load()` và `save()` hiện không tự kiểm tra quyền; việc gọi chúng cần được kiểm soát ở luồng ứng dụng.
- Chưa có `activateStaff()` để kích hoạt lại tài khoản đã vô hiệu hóa.
- Chưa có thao tác xóa nhân viên; giữ tài khoản để bảo toàn liên kết lịch sử hóa đơn.
- Các thao tác thay đổi dữ liệu chưa tự động gọi `save()`; cần thống nhất thời điểm lưu dữ liệu trong ứng dụng.
- Không được xem kết quả test đạt là bằng chứng cho ràng buộc với hóa đơn/vé khi các Repository giao dịch chưa được tích hợp.
