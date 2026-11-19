# MemberService

## 1. Tổng quan

`MemberService` là lớp thuộc tầng **Business Logic**, chịu trách nhiệm xử lý các nghiệp vụ quản lý thành viên trong hệ thống quản lý rạp chiếu phim PBL2.

Lớp sử dụng:

- `MemberRepository` để thêm, cập nhật và truy xuất dữ liệu thành viên.
- `AuthService` để kiểm tra quyền của tài khoản đang đăng nhập.

`MemberService` không trực tiếp đọc hoặc ghi file TXT, không tự quản lý danh sách thành viên và không thay thế các kiểm tra dữ liệu đã được triển khai trong Repository.

**File liên quan:**

- `src/business/MemberService.h`
- `src/business/MemberService.cpp`
- `src/business/AuthService.h`
- `src/data_access/MemberRepository.h`
- `src/data_access/MemberRepository.cpp`
- `src/models/Member.h`
- `tests/test_member_service.cpp`

**Trạng thái:** Đã triển khai `MemberService.h/.cpp` với đủ 8 phương thức public ngoài constructor; `tests/test_member_service.cpp` biên dịch và chạy đạt **69 PASS, 0 FAIL** ngày 2026-11-19. Phạm vi hóa đơn/điểm giao dịch vẫn chưa triển khai.

---

## 2. Trách nhiệm của MemberService

`MemberService` có các trách nhiệm chính:

1. Kiểm tra quyền trước khi thực hiện nghiệp vụ quản lý thành viên.
2. Tiếp nhận yêu cầu thêm và cập nhật thành viên.
3. Cung cấp các chức năng tra cứu thành viên theo mã, số điện thoại và vị trí.
4. Cung cấp số lượng thành viên hiện có.
5. Chuyển các thao tác quản lý dữ liệu đến `MemberRepository`.
6. Đảm bảo các quy tắc nghiệp vụ đã thống nhất được áp dụng thông qua Service và Repository.

`MemberService` không chịu trách nhiệm:

- Sinh mã thành viên tự động.
- Trực tiếp đọc hoặc ghi file dữ liệu.
- Tự quản lý bộ nhớ của các thành viên đã được Repository sở hữu.
- Cộng hoặc trừ điểm tích lũy khi bán/hủy vé.
- Xóa thành viên.
- Quản lý hóa đơn của thành viên.

Những nghiệp vụ chưa có sẽ được bổ sung ở các bước phát triển sau khi có thiết kế phù hợp.

---

## 3. Các thành phần phụ thuộc

### 3.1. MemberRepository

`MemberService` sử dụng các phương thức đã tồn tại trong `MemberRepository`:

| Phương thức | Chức năng |
|---|---|
| `load()` | Nạp dữ liệu thành viên |
| `save()` | Lưu dữ liệu thành viên |
| `add(Member*)` | Thêm thành viên |
| `update(const Member&)` | Cập nhật thông tin |
| `findById(memberId)` | Tìm thành viên theo mã |
| `findByPhone(phone)` | Tìm theo số điện thoại |
| `getAt(index)` | Lấy thành viên theo vị trí |
| `count()` | Đếm số thành viên |
| `isLoaded()` | Kiểm tra trạng thái nạp dữ liệu |

Repository chịu trách nhiệm kiểm tra tính hợp lệ của dữ liệu, tính duy nhất của mã thành viên và số điện thoại.

### 3.2. AuthService

`MemberService` sử dụng:

```cpp
authService.requirePermission(permission);
```

Phương thức này kiểm tra tài khoản đang đăng nhập và quyền được cấp trong `RolePermissionRepository`.

Nếu chưa đăng nhập hoặc không có quyền, phương thức ném `runtime_error`.

### 3.3. Member Model

Thông tin thành viên gồm:

| Thuộc tính | Kiểu dữ liệu | Ý nghĩa |
|---|---|---|
| `memberId` | `string` | Mã thành viên |
| `fullName` | `string` | Họ và tên |
| `phone` | `string` | Số điện thoại |
| `points` | `int` | Điểm tích lũy |

Model còn quản lý quan hệ với hóa đơn, nhưng `MemberService` ở giai đoạn này chưa triển khai các nghiệp vụ liên quan đến hóa đơn.

---

## 4. Cấu trúc lớp hiện tại

### 4.1. Thuộc tính

```cpp
private:
    MemberRepository& memberRepo;
    AuthService& authService;
```

**`memberRepo`:**

- Tham chiếu đến Repository quản lý thành viên.
- Không tạo bản sao Repository.
- Không trực tiếp sở hữu hoặc giải phóng Repository.

**`authService`:**

- Tham chiếu đến Service quản lý đăng nhập và phân quyền.
- Đảm bảo các nghiệp vụ chỉ được thực hiện khi tài khoản có quyền phù hợp.

Hai đối tượng được tham chiếu phải tồn tại trong suốt thời gian sử dụng `MemberService`.

### 4.2. Constructor

```cpp
MemberService(
    MemberRepository& memberRepo,
    AuthService& authService
);
```

**Mục đích:** Khởi tạo `MemberService` với các đối tượng phụ thuộc.

**Nguyên tắc:**

- Nhận tham chiếu từ bên ngoài.
- Không tự tạo Repository hoặc AuthService.
- Không tự động gọi `load()`.
- Không tự động đăng nhập tài khoản.

---

## 5. Danh sách phương thức

| STT | Phương thức | Kiểu trả về | Permission |
|---|---|---|---|
| 1 | `load()` | `bool` | Nội bộ |
| 2 | `save()` | `bool` | Nội bộ |
| 3 | `addMember(Member*)` | `bool` | `MEMBER_ADD` |
| 4 | `updateMember(const Member&)` | `bool` | `MEMBER_UPDATE` |
| 5 | `findMemberById(const string&)` | `const Member*` | `MEMBER_VIEW` |
| 6 | `findMemberByPhone(const string&)` | `const Member*` | `MEMBER_VIEW` |
| 7 | `getMemberAt(int)` | `const Member*` | `MEMBER_VIEW` |
| 8 | `getMemberCount()` | `int` | `MEMBER_VIEW` |

---

## 6. Hành vi các phương thức đã triển khai

### 6.1. load()

**Khai báo:**

```cpp
bool load();
```

**Mục đích:** Nạp dữ liệu thành viên từ file thông qua Repository.

**Luồng xử lý:**

1. Gọi `memberRepo.load()`.
2. Trả về kết quả của Repository.

**Lưu ý:**

- Không yêu cầu đăng nhập hoặc permission.
- Chỉ được gọi trong quá trình khởi tạo dữ liệu ứng dụng.
- Không được dùng như một chức năng nạp lại dữ liệu tùy ý từ GUI.
- Repository hiện tại ném `runtime_error` nếu dữ liệu đã được nạp hoặc không thể nạp file.

### 6.2. save()

**Khai báo:**

```cpp
bool save();
```

**Mục đích:** Lưu dữ liệu thành viên thông qua Repository.

**Luồng xử lý:**

1. Gọi `memberRepo.save()`.
2. Trả về kết quả của Repository.

**Lưu ý:**

- Không yêu cầu permission riêng.
- Chỉ được gọi trong luồng quản lý lưu trữ dữ liệu nội bộ.
- Không mặc định tự động lưu sau mỗi lần thêm hoặc cập nhật.
- Repository hiện tại ném `runtime_error` nếu chưa nạp dữ liệu hoặc ghi file thất bại.

### 6.3. addMember()

**Khai báo:**

```cpp
bool addMember(Member* member);
```

**Mục đích:** Thêm một thành viên mới vào hệ thống.

**Permission:** `MEMBER_ADD`

**Luồng xử lý:**

1. Gọi `authService.requirePermission("MEMBER_ADD")`.
2. Gọi `memberRepo.add(member)`.
3. Trả về kết quả từ Repository.

**Quy tắc nghiệp vụ:**

- Thành viên không được `nullptr`.
- Mã thành viên phải hợp lệ và duy nhất.
- Họ tên phải hợp lệ.
- Số điện thoại gồm đúng 10 chữ số, bắt đầu bằng `0`.
- Số điện thoại không được trùng với thành viên khác.
- Điểm tích lũy ban đầu phải bằng `0`.
- Repository phải được nạp trước khi thêm.

Các điều kiện này đã được `MemberRepository::add()` kiểm tra nên Service không cần triển khai lại.

**Kết quả:**

- `true`: Thêm thành viên thành công vào dữ liệu trong bộ nhớ.
- `false`: Không đáp ứng điều kiện thêm.
- Ngoại lệ: Không có quyền hoặc lỗi phát sinh từ tầng dưới.

**Quản lý bộ nhớ:**

Khi thêm thành công, Repository tiếp nhận quản lý đối tượng `Member*`.

Bên gọi không được tự `delete` đối tượng sau khi thêm thành công.

Nếu thêm thất bại, Repository không tiếp nhận quyền sở hữu; bên gọi chịu trách nhiệm xử lý con trỏ theo quy ước quản lý bộ nhớ của ứng dụng.

### 6.4. updateMember()

**Khai báo:**

```cpp
bool updateMember(const Member& newValues);
```

**Mục đích:** Cập nhật thông tin của thành viên đã tồn tại.

**Permission:** `MEMBER_UPDATE`

**Luồng xử lý:**

1. Gọi `authService.requirePermission("MEMBER_UPDATE")`.
2. Gọi `memberRepo.update(newValues)`.
3. Trả về kết quả từ Repository.

**Quy tắc nghiệp vụ:**

- Thành viên cần cập nhật phải tồn tại.
- `memberId` là mã định danh bất biến.
- Chỉ cập nhật thông tin họ tên và số điện thoại.
- Họ tên và số điện thoại mới phải hợp lệ.
- Số điện thoại mới không được trùng thành viên khác.
- Không cho phép thay đổi điểm tích lũy thông qua hàm này.

Repository hiện tại từ chối cập nhật nếu `newValues.getPoints()` khác điểm của thành viên đang lưu.

**Kết quả:**

- `true`: Cập nhật thành công trong bộ nhớ.
- `false`: Thành viên không tồn tại hoặc dữ liệu không đáp ứng quy tắc.
- Ngoại lệ: Không có quyền hoặc lỗi phát sinh từ tầng dưới.

**Lưu ý cho UI:** Khi tạo đối tượng `newValues`, phải giữ nguyên `memberId` và `points` hiện tại. UI không được sử dụng `updateMember()` để chỉnh sửa điểm.

### 6.5. findMemberById()

**Khai báo:**

```cpp
const Member* findMemberById(const string& memberId);
```

**Mục đích:** Tìm thành viên theo mã định danh.

**Permission:** `MEMBER_VIEW`

**Luồng xử lý:**

1. Kiểm tra quyền `MEMBER_VIEW`.
2. Gọi `memberRepo.findById(memberId)`.
3. Trả về con trỏ `const Member*`.

**Kết quả:**

- Khác `nullptr`: Tìm thấy thành viên.
- `nullptr`: Không tìm thấy.
- Ngoại lệ: Chưa đăng nhập hoặc không có quyền.

### 6.6. findMemberByPhone()

**Khai báo:**

```cpp
const Member* findMemberByPhone(const string& phone);
```

**Mục đích:** Tra cứu thành viên theo số điện thoại.

**Permission:** `MEMBER_VIEW`

**Luồng xử lý:**

1. Kiểm tra quyền `MEMBER_VIEW`.
2. Gọi `memberRepo.findByPhone(phone)`.
3. Trả về `const Member*`.

**Kết quả:**

- Khác `nullptr`: Tìm thấy thành viên.
- `nullptr`: Không tìm thấy.
- Ngoại lệ: Chưa đăng nhập hoặc không có quyền.

Số điện thoại là duy nhất trong dữ liệu thành viên.

### 6.7. getMemberAt()

**Khai báo:**

```cpp
const Member* getMemberAt(int index);
```

**Mục đích:** Lấy thành viên tại vị trí chỉ định.

**Permission:** `MEMBER_VIEW`

**Luồng xử lý:**

1. Kiểm tra quyền `MEMBER_VIEW`.
2. Gọi `memberRepo.getAt(index)`.
3. Trả về `const Member*`.

**Quy ước:**

- Chỉ số bắt đầu từ `0`.
- Vị trí hợp lệ thuộc khoảng `0` đến `getMemberCount() - 1`.
- Vị trí không hợp lệ được xử lý theo hành vi của Repository.

**Ứng dụng:** Hiển thị danh sách thành viên trên giao diện.

### 6.8. getMemberCount()

**Khai báo:**

```cpp
int getMemberCount();
```

**Mục đích:** Trả về tổng số thành viên đang được quản lý.

**Permission:** `MEMBER_VIEW`

**Luồng xử lý:**

1. Kiểm tra quyền `MEMBER_VIEW`.
2. Gọi `memberRepo.count()`.
3. Trả về số lượng thành viên.

**Kết quả:** Số lượng thành viên trong Repository.

---

## 7. Phân quyền RBAC

`MemberService` sử dụng ba permission:

| Permission | Nghiệp vụ | ADMIN mặc định | STAFF mặc định |
|---|---|---|---|
| `MEMBER_VIEW` | Tra cứu thành viên | Có | Có |
| `MEMBER_ADD` | Thêm thành viên | Có | Có |
| `MEMBER_UPDATE` | Cập nhật thành viên | Có | Có |

Quyền thực tế được xác định bởi `data/role_permissions.txt` đã nạp, không mặc định dựa vào vai trò.

Ví dụ:

```cpp
authService.requirePermission("MEMBER_ADD");
```

Nếu tài khoản chưa đăng nhập hoặc không có quyền, thao tác bị từ chối trước khi gọi Repository.

**Nguyên tắc:**

- Service phải kiểm tra quyền ở mỗi lần gọi nghiệp vụ.
- Không sử dụng `requireAdmin()` thay cho permission.
- Không tự động cho phép Admin bỏ qua bảng quyền.
- UI có thể gọi `hasPermission()` để điều chỉnh chức năng hiển thị, nhưng Backend vẫn phải kiểm tra quyền.

---

## 8. Quy tắc điểm tích lũy

Điểm tích lũy là dữ liệu nghiệp vụ được quản lý theo hoạt động bán và hủy vé.

### 8.1. Khi tạo thành viên

Thành viên mới bắt buộc có:

```cpp
points == 0
```

### 8.2. Khi cập nhật thông tin

`updateMember()` không được thay đổi điểm.

Điểm của `newValues` phải bằng điểm đang lưu.

### 8.3. Khi bán hoặc hủy vé

Việc cộng hoặc thu hồi điểm sẽ được thiết kế trong nghiệp vụ bán/hủy vé và hóa đơn ở các bước tiếp theo.

`MemberService` phiên bản này không cung cấp:

```cpp
addPoints(...)
subtractPoints(...)
setPoints(...)
```

Không được sử dụng trực tiếp `Member::setPoints()` từ giao diện để thay đổi điểm tích lũy.

---

## 9. Quy ước kết quả và ngoại lệ

| Tình huống | Hành vi |
|---|---|
| Thao tác thành công | Trả về `true` |
| Dữ liệu không đáp ứng điều kiện | Trả về `false` |
| Không tìm thấy thành viên | Trả về `nullptr` |
| Chưa đăng nhập | `runtime_error` từ AuthService |
| Không có permission | `runtime_error` từ AuthService |
| Lỗi nạp hoặc lưu dữ liệu | Ngoại lệ từ Repository |

`MemberService` không tự ý chuyển ngoại lệ phân quyền thành `false`.

Các phương thức thêm và cập nhật chỉ thay đổi dữ liệu trong bộ nhớ; việc ghi ra file được thực hiện thông qua `save()` trong luồng lưu trữ của ứng dụng.

---

## 10. Tích hợp với giao diện

Giao diện phải gọi `MemberService`, không gọi trực tiếp `MemberRepository` để thực hiện nghiệp vụ.

Các API hiện có cho giao diện:

| Chức năng giao diện | API sử dụng |
|---|---|
| Hiển thị danh sách | `getMemberCount()`, `getMemberAt()` |
| Tìm theo mã | `findMemberById()` |
| Tìm theo số điện thoại | `findMemberByPhone()` |
| Thêm thành viên | `addMember()` |
| Sửa thông tin | `updateMember()` |
| Xem điểm tích lũy | `Member::getPoints()` trên đối tượng tra cứu |

Các con trỏ trả về là `const Member*`, không được dùng để sửa trực tiếp dữ liệu.

UI phải xử lý các trường hợp không tìm thấy, thao tác thất bại và ngoại lệ phân quyền.

---

## 11. Kiểm thử đã chạy

File kiểm thử:

`tests/test_member_service.cpp`

Biên dịch riêng bài test với C++17 cùng các `.cpp` trong `src/` (trừ `main.cpp`) và chạy trên nhánh hiện tại ngày 2026-11-19: **69 PASS, 0 FAIL**, thoát mã 0. Bài test dùng các file TXT tạm, dọn sau khi chạy. Các nhóm dưới đây mô tả phạm vi đã kiểm tra; chưa có kiểm thử tích hợp với hóa đơn/vé.

### 11.1. Nhóm quản lý dữ liệu

- Nạp dữ liệu thành công.
- Lưu dữ liệu thành công.
- Gọi load khi đã nạp.
- Gọi save khi chưa nạp.
- Xử lý file dữ liệu không hợp lệ.

### 11.2. Nhóm thêm thành viên

- Có quyền và dữ liệu hợp lệ.
- Chưa đăng nhập.
- Không có quyền.
- Con trỏ `nullptr`.
- Mã thành viên không hợp lệ hoặc trùng.
- Số điện thoại không hợp lệ hoặc trùng.
- Họ tên không hợp lệ.
- Điểm ban đầu khác `0`.

### 11.3. Nhóm cập nhật thành viên

- Cập nhật họ tên thành công.
- Cập nhật số điện thoại thành công.
- Không tìm thấy thành viên.
- Số điện thoại mới trùng.
- Dữ liệu không hợp lệ.
- Cố gắng thay đổi điểm tích lũy.
- Chưa đăng nhập hoặc thiếu quyền.

### 11.4. Nhóm tra cứu

- Tìm theo mã tồn tại và không tồn tại.
- Tìm theo số điện thoại tồn tại và không tồn tại.
- Lấy thành viên theo vị trí hợp lệ.
- Truy cập vị trí không hợp lệ.
- Đếm số lượng thành viên.
- Kiểm tra quyền `MEMBER_VIEW`.

### 11.5. Nhóm phân quyền

- Admin có permission.
- Staff có permission.
- Tài khoản thiếu permission.
- Tài khoản đã bị vô hiệu hóa.
- Đã đăng xuất.
- Admin thiếu `MEMBER_ADD` hoặc `MEMBER_UPDATE` dù đã đăng nhập.

Test sử dụng file dữ liệu tạm, không chỉnh sửa dữ liệu thật của ứng dụng.

---

## 12. Giới hạn hiện tại

`MemberService` phiên bản đầu chưa hỗ trợ:

- Xóa thành viên.
- Cộng hoặc trừ điểm trực tiếp.
- Tra cứu lịch sử hóa đơn của thành viên.
- Tìm kiếm gần đúng theo tên.
- Phân trang hoặc sắp xếp danh sách.
- Tự động lưu sau mỗi thao tác.

Các chức năng trên chỉ được bổ sung sau khi thống nhất nghiệp vụ và cập nhật tài liệu.

---

## 13. Checklist triển khai hiện tại

- [x] Thống nhất thiết kế lớp `MemberService`.
- [x] Xác định đầy đủ 8 phương thức.
- [x] Xác định các permission.
- [x] Thống nhất quy tắc không sửa điểm trực tiếp.
- [x] Hoàn thành nội dung tài liệu thiết kế.
- [x] Có tài liệu `MemberService.md` đồng bộ với code hiện tại.
- [x] Triển khai `MemberService.h/.cpp` với đủ 8 phương thức public ngoài constructor.
- [x] Kiểm tra `MEMBER_VIEW`, `MEMBER_ADD`, `MEMBER_UPDATE` qua `AuthService::requirePermission()`.
- [x] Viết, biên dịch và chạy `tests/test_member_service.cpp`: **69 PASS, 0 FAIL**.
- [ ] Tích hợp nghiệp vụ điểm và quan hệ hóa đơn/vé khi các thành phần giao dịch sẵn sàng; chưa đánh dấu hoàn thành toàn Bước 08.

**Kết luận:** `MemberService` sử dụng `MemberRepository` để quản lý dữ liệu và `AuthService` để kiểm tra quyền, giữ cấu trúc đồng bộ với các Service đã triển khai trong PBL2.
