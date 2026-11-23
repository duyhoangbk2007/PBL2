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

## Chi tiết triển khai từng hàm (StaffService.cpp)

### StaffService()

#### A. Tổng quan

- **Mục đích:** Khởi tạo Service với các Repository và AuthService.
- **Tham số:** `UserRepository& staffRepo, AuthService& authService) : staffRepo(staffRepo), authService(authService`.
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
StaffService::StaffService(UserRepository& staffRepo, AuthService& authService)
    : staffRepo(staffRepo), authService(authService)
{}
```

Các câu lệnh được thực thi theo đúng thứ tự trong `.cpp`; điều kiện và nhánh trả về nằm ngay trong code. Giá trị được khai báo ở đây được dùng trong các nhánh tiếp theo; hãy theo dõi nơi nó được trả về hoặc chuyển vào Repository để hiểu tác động lên trạng thái.

### load()

#### A. Tổng quan

- **Mục đích:** Kiểm tra dữ liệu đã nạp. Xem các nhánh cụ thể trong code.
- **Tham số:** `không có`.
- **Giá trị trả về:** `bool`; các nhánh `return` được ghi đầy đủ bên dưới.
- **Điều kiện quan trọng:** điều kiện và lỗi thể hiện trực tiếp trong mã nguồn.

#### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Kiểm tra dữ liệu đã nạp
    |
    v
Kết thúc
```

#### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1 — Kiểm tra dữ liệu đã nạp.**

```cpp
bool StaffService::load()
{
    return staffRepo.load();
}
```

Các câu lệnh được thực thi theo đúng thứ tự trong `.cpp`; điều kiện và nhánh trả về nằm ngay trong code. Giá trị được khai báo ở đây được dùng trong các nhánh tiếp theo; hãy theo dõi nơi nó được trả về hoặc chuyển vào Repository để hiểu tác động lên trạng thái.

### save()

#### A. Tổng quan

- **Mục đích:** Cập nhật qua Repository. Xem các nhánh cụ thể trong code.
- **Tham số:** `không có`.
- **Giá trị trả về:** `bool`; các nhánh `return` được ghi đầy đủ bên dưới.
- **Điều kiện quan trọng:** điều kiện và lỗi thể hiện trực tiếp trong mã nguồn.

#### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Cập nhật qua Repository
    |
    v
Kết thúc
```

#### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1 — Cập nhật qua Repository.**

```cpp
bool StaffService::save()
{
    return staffRepo.save();
}
```

Repository thực hiện thao tác dữ liệu. `save()` ghi trạng thái Repository ra TXT tại thời điểm lời gọi; thay đổi RAM trước đó không tự đồng nghĩa với đã lưu thành công.

### addStaff()

#### A. Tổng quan

- **Mục đích:** Kiểm tra quyền. Xem các nhánh cụ thể trong code.
- **Tham số:** `User* staff`.
- **Giá trị trả về:** `bool`; các nhánh `return` được ghi đầy đủ bên dưới.
- **Điều kiện quan trọng:** Kiểm tra quyền qua `AuthService`; kiểm tra trạng thái nạp và con trỏ trước khi dùng.

#### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Kiểm tra quyền
    |
    v
Tra cứu dữ liệu
    |
    v
Chuẩn bị và xử lý dữ liệu
    |
    v
Cập nhật qua Repository
    |
    v
Trả kết quả
    |
    v
Kết thúc
```

#### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1 — Kiểm tra quyền.**

```cpp
bool StaffService::addStaff(User* staff)
{
    authService.requirePermission("STAFF_ADD");

    if (!staffRepo.isLoaded() ||
        staff == nullptr ||
        staff->getRole() != UserRole::STAFF)
    {
        return false;
    }
```

`AuthService` kiểm tra quyền và có thể ném ngoại lệ. `nullptr` cho biết không có đối tượng để truy cập. `false` báo thao tác không thành công. Quyền `STAFF_ADD` được kiểm tra trước bước xử lý kế tiếp; thiếu quyền sẽ ném ngoại lệ, vì vậy caller không nên xem đó là kết quả `false`. Kiểm tra `isLoaded()` tránh dùng dữ liệu Repository chưa được nạp; nhánh không đạt kết thúc ngay theo giá trị `return` trong code. Nhánh `false` dừng hàm tại đây; những câu lệnh phía sau không chạy, nên không có cập nhật nào từ các bước sau.

**Bước 2 — Tra cứu dữ liệu.**

```cpp
    vector<string> existingIds;

    for (int i = 0; i < staffRepo.count(); i++)
    {
        User* current = staffRepo.getAt(i);
        if (current == nullptr)
        {
            return false;
        }
        existingIds.push_back(current->getUserId());
```

`nullptr` cho biết không có đối tượng để truy cập. `false` báo thao tác không thành công. Vòng `for` duyệt lần lượt các phần tử. Nhánh `false` dừng hàm tại đây; những câu lệnh phía sau không chạy, nên không có cập nhật nào từ các bước sau. Vòng lặp xử lý từng phần tử theo thứ tự; điều kiện bên trong quyết định phần tử được giữ, bỏ qua hay đưa vào kết quả.

**Bước 3 — Chuẩn bị và xử lý dữ liệu.**

```cpp
    }

    string userId = IdGenerator::generateNextId("USR", existingIds);

    User* newStaff = new User(userId,
                              staff->getUsername(),
                              staff->getPassword(),
                              staff->getFullName(),
                              UserRole::STAFF,
                              staff->isActive());
```

Các câu lệnh được thực thi theo đúng thứ tự trong `.cpp`; điều kiện và nhánh trả về nằm ngay trong code. `new` cấp phát đối tượng trên heap; cần theo dõi thời điểm quyền sở hữu được chuyển sang Repository để tránh rò rỉ bộ nhớ.

**Bước 4 — Cập nhật qua Repository.**

```cpp

    try
    {
        if (!staffRepo.add(newStaff))
        {
            delete newStaff;
            return false;
        }
    }
    catch (...)
```

`false` báo thao tác không thành công. Repository thực hiện thao tác dữ liệu. Khi `add()` thành công, Repository nhận đối tượng và quản lý vòng đời của nó; nếu thất bại, bên gọi vẫn phải xét trách nhiệm giải phóng con trỏ. Nhánh `false` dừng hàm tại đây; những câu lệnh phía sau không chạy, nên không có cập nhật nào từ các bước sau. Khối `try/catch` giữ lỗi phát sinh trong phạm vi xử lý này và chuyển sang nhánh khôi phục hoặc báo lỗi đã viết trong code. `delete` giải phóng vùng nhớ của đối tượng, sau đó không được dùng lại con trỏ trỏ đến vùng nhớ đó.

**Bước 5 — Trả kết quả.**

```cpp
    {
        delete newStaff;
        throw;
    }
    return true;
}
```

Các câu lệnh được thực thi theo đúng thứ tự trong `.cpp`; điều kiện và nhánh trả về nằm ngay trong code. `delete` giải phóng vùng nhớ của đối tượng, sau đó không được dùng lại con trỏ trỏ đến vùng nhớ đó.

### updateStaff()

#### A. Tổng quan

- **Mục đích:** Kiểm tra quyền. Xem các nhánh cụ thể trong code.
- **Tham số:** `const User& newValues`.
- **Giá trị trả về:** `bool`; các nhánh `return` được ghi đầy đủ bên dưới.
- **Điều kiện quan trọng:** Kiểm tra quyền qua `AuthService`; kiểm tra trạng thái nạp và con trỏ trước khi dùng.

#### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Kiểm tra quyền
    |
    v
Cập nhật qua Repository
    |
    v
Kết thúc
```

#### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1 — Kiểm tra quyền.**

```cpp
bool StaffService::updateStaff(const User& newValues)
{
    authService.requirePermission("STAFF_UPDATE");
    if (!staffRepo.isLoaded())
    {
        return false;
    }
    User* current = staffRepo.findById(newValues.getUserId());
    if (current == nullptr ||
        current->getRole() != UserRole::STAFF ||
```

`AuthService` kiểm tra quyền và có thể ném ngoại lệ. `nullptr` cho biết không có đối tượng để truy cập. `false` báo thao tác không thành công. Quyền `STAFF_UPDATE` được kiểm tra trước bước xử lý kế tiếp; thiếu quyền sẽ ném ngoại lệ, vì vậy caller không nên xem đó là kết quả `false`. Kiểm tra `isLoaded()` tránh dùng dữ liệu Repository chưa được nạp; nhánh không đạt kết thúc ngay theo giá trị `return` trong code. Lệnh tra cứu trả con trỏ đến đối tượng đang được Repository quản lý. Con trỏ rỗng có nghĩa là không tìm thấy, nên bước sau chỉ được dùng đối tượng khi kiểm tra thành công. Nhánh `false` dừng hàm tại đây; những câu lệnh phía sau không chạy, nên không có cập nhật nào từ các bước sau.

**Bước 2 — Cập nhật qua Repository.**

```cpp
        newValues.getRole() != UserRole::STAFF)
    {
        return false;
    }
    if (current->isActive() != newValues.isActive())
    {
        return false;
    }
    return staffRepo.update(newValues);
}
```

`false` báo thao tác không thành công. Repository thực hiện thao tác dữ liệu. `update()` chuyển việc thay đổi dữ liệu cho Repository; kết quả trả về phản ánh kiểm tra và thao tác cập nhật ở tầng dữ liệu. Nhánh `false` dừng hàm tại đây; những câu lệnh phía sau không chạy, nên không có cập nhật nào từ các bước sau.

### deactivateStaff()

#### A. Tổng quan

- **Mục đích:** Kiểm tra quyền. Xem các nhánh cụ thể trong code.
- **Tham số:** `const string& staffId`.
- **Giá trị trả về:** `bool`; các nhánh `return` được ghi đầy đủ bên dưới.
- **Điều kiện quan trọng:** Kiểm tra quyền qua `AuthService`; kiểm tra trạng thái nạp và con trỏ trước khi dùng.

#### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Kiểm tra quyền
    |
    v
Từ chối dữ liệu không hợp lệ
    |
    v
Cập nhật qua Repository
    |
    v
Kết thúc
```

#### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1 — Kiểm tra quyền.**

```cpp
bool StaffService::deactivateStaff(const string& staffId)
{
    authService.requirePermission("STAFF_DEACTIVATE");
    if (!staffRepo.isLoaded())
    {
        return false;
    }
    User* staff = staffRepo.findById(staffId);
    if (staff == nullptr ||
        staff->getRole() != UserRole::STAFF ||
```

`AuthService` kiểm tra quyền và có thể ném ngoại lệ. `nullptr` cho biết không có đối tượng để truy cập. `false` báo thao tác không thành công. Quyền `STAFF_DEACTIVATE` được kiểm tra trước bước xử lý kế tiếp; thiếu quyền sẽ ném ngoại lệ, vì vậy caller không nên xem đó là kết quả `false`. Kiểm tra `isLoaded()` tránh dùng dữ liệu Repository chưa được nạp; nhánh không đạt kết thúc ngay theo giá trị `return` trong code. Lệnh tra cứu trả con trỏ đến đối tượng đang được Repository quản lý. Con trỏ rỗng có nghĩa là không tìm thấy, nên bước sau chỉ được dùng đối tượng khi kiểm tra thành công. Nhánh `false` dừng hàm tại đây; những câu lệnh phía sau không chạy, nên không có cập nhật nào từ các bước sau.

**Bước 2 — Từ chối dữ liệu không hợp lệ.**

```cpp
        !staff->isActive())
    {
        return false;
    }
    User updated(
        staff->getUserId(),
        staff->getUsername(),
        staff->getPassword(),
        staff->getFullName(),
        staff->getRole(),
```

`false` báo thao tác không thành công. Nhánh `false` dừng hàm tại đây; những câu lệnh phía sau không chạy, nên không có cập nhật nào từ các bước sau.

**Bước 3 — Cập nhật qua Repository.**

```cpp
        false
    );
    return staffRepo.update(updated);
}
```

Repository thực hiện thao tác dữ liệu. `update()` chuyển việc thay đổi dữ liệu cho Repository; kết quả trả về phản ánh kiểm tra và thao tác cập nhật ở tầng dữ liệu.

### findStaffById()

#### A. Tổng quan

- **Mục đích:** Kiểm tra quyền. Xem các nhánh cụ thể trong code.
- **Tham số:** `const string& staffId`.
- **Giá trị trả về:** `const User*`; các nhánh `return` được ghi đầy đủ bên dưới.
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
const User* StaffService::findStaffById(const string& staffId)
{
    authService.requirePermission("STAFF_VIEW");
    const User* staff = staffRepo.findById(staffId);
    if (staff == nullptr || staff->getRole() != UserRole::STAFF)
    {
        return nullptr;
    }
    return staff;
}
```

`AuthService` kiểm tra quyền và có thể ném ngoại lệ. `nullptr` cho biết không có đối tượng để truy cập. Quyền `STAFF_VIEW` được kiểm tra trước bước xử lý kế tiếp; thiếu quyền sẽ ném ngoại lệ, vì vậy caller không nên xem đó là kết quả `false`. Lệnh tra cứu trả con trỏ đến đối tượng đang được Repository quản lý. Con trỏ rỗng có nghĩa là không tìm thấy, nên bước sau chỉ được dùng đối tượng khi kiểm tra thành công. `nullptr` báo không có đối tượng phù hợp, để caller kiểm tra trước khi truy cập.

### findStaffByUsername()

#### A. Tổng quan

- **Mục đích:** Kiểm tra quyền. Xem các nhánh cụ thể trong code.
- **Tham số:** `const string& username`.
- **Giá trị trả về:** `const User*`; các nhánh `return` được ghi đầy đủ bên dưới.
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
const User* StaffService::findStaffByUsername(const string& username)
{
    authService.requirePermission("STAFF_VIEW");
    const User* staff = staffRepo.findByUsername(username);
    if (staff == nullptr || staff->getRole() != UserRole::STAFF)
    {
        return nullptr;
    }
    return staff;
}
```

`AuthService` kiểm tra quyền và có thể ném ngoại lệ. `nullptr` cho biết không có đối tượng để truy cập. Quyền `STAFF_VIEW` được kiểm tra trước bước xử lý kế tiếp; thiếu quyền sẽ ném ngoại lệ, vì vậy caller không nên xem đó là kết quả `false`. Lệnh tra cứu trả con trỏ đến đối tượng đang được Repository quản lý. Con trỏ rỗng có nghĩa là không tìm thấy, nên bước sau chỉ được dùng đối tượng khi kiểm tra thành công. `nullptr` báo không có đối tượng phù hợp, để caller kiểm tra trước khi truy cập.

### getStaffAt()

#### A. Tổng quan

- **Mục đích:** Kiểm tra quyền. Xem các nhánh cụ thể trong code.
- **Tham số:** `int index`.
- **Giá trị trả về:** `const User*`; các nhánh `return` được ghi đầy đủ bên dưới.
- **Điều kiện quan trọng:** Kiểm tra quyền qua `AuthService`; kiểm tra trạng thái nạp và con trỏ trước khi dùng.

#### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Kiểm tra quyền
    |
    v
Tra cứu dữ liệu
    |
    v
Trả kết quả
    |
    v
Kết thúc
```

#### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1 — Kiểm tra quyền.**

```cpp
const User* StaffService::getStaffAt(int index)
{
    authService.requirePermission("STAFF_VIEW");
    if (index < 0)
    {
        return nullptr;
    }
    int staffIndex = 0;
    for (int i = 0; i < staffRepo.count(); i++)
    {
```

`AuthService` kiểm tra quyền và có thể ném ngoại lệ. `nullptr` cho biết không có đối tượng để truy cập. Vòng `for` duyệt lần lượt các phần tử. Quyền `STAFF_VIEW` được kiểm tra trước bước xử lý kế tiếp; thiếu quyền sẽ ném ngoại lệ, vì vậy caller không nên xem đó là kết quả `false`. `nullptr` báo không có đối tượng phù hợp, để caller kiểm tra trước khi truy cập. Vòng lặp xử lý từng phần tử theo thứ tự; điều kiện bên trong quyết định phần tử được giữ, bỏ qua hay đưa vào kết quả.

**Bước 2 — Tra cứu dữ liệu.**

```cpp
        const User* user = staffRepo.getAt(i);
        if (user != nullptr && user->getRole() == UserRole::STAFF)
        {
            if (staffIndex == index)
            {
                return user;
            }
            staffIndex++;
        }
    }
```

`nullptr` cho biết không có đối tượng để truy cập. Giá trị được khai báo ở đây được dùng trong các nhánh tiếp theo; hãy theo dõi nơi nó được trả về hoặc chuyển vào Repository để hiểu tác động lên trạng thái.

**Bước 3 — Trả kết quả.**

```cpp
    return nullptr;
}
```

`nullptr` cho biết không có đối tượng để truy cập. `nullptr` báo không có đối tượng phù hợp, để caller kiểm tra trước khi truy cập.

### getStaffCount()

#### A. Tổng quan

- **Mục đích:** Kiểm tra quyền. Xem các nhánh cụ thể trong code.
- **Tham số:** `không có`.
- **Giá trị trả về:** `int`; các nhánh `return` được ghi đầy đủ bên dưới.
- **Điều kiện quan trọng:** Kiểm tra quyền qua `AuthService`; kiểm tra trạng thái nạp và con trỏ trước khi dùng.

#### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Kiểm tra quyền
    |
    v
Trả kết quả
    |
    v
Kết thúc
```

#### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1 — Kiểm tra quyền.**

```cpp
int StaffService::getStaffCount()
{
    authService.requirePermission("STAFF_VIEW");
    int count = 0;
    for (int i = 0; i < staffRepo.count(); i++)
    {
        const User* user = staffRepo.getAt(i);
        if (user != nullptr && user->getRole() == UserRole::STAFF)
        {
            count++;
```

`AuthService` kiểm tra quyền và có thể ném ngoại lệ. `nullptr` cho biết không có đối tượng để truy cập. Vòng `for` duyệt lần lượt các phần tử. Quyền `STAFF_VIEW` được kiểm tra trước bước xử lý kế tiếp; thiếu quyền sẽ ném ngoại lệ, vì vậy caller không nên xem đó là kết quả `false`. Vòng lặp xử lý từng phần tử theo thứ tự; điều kiện bên trong quyết định phần tử được giữ, bỏ qua hay đưa vào kết quả.

**Bước 2 — Trả kết quả.**

```cpp
        }
    }
    return count;
}
```

Các câu lệnh được thực thi theo đúng thứ tự trong `.cpp`; điều kiện và nhánh trả về nằm ngay trong code. Giá trị được khai báo ở đây được dùng trong các nhánh tiếp theo; hãy theo dõi nơi nó được trả về hoặc chuyển vào Repository để hiểu tác động lên trạng thái.

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
