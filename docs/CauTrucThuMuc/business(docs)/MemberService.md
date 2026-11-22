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

## Chi tiết triển khai từng hàm (MemberService.cpp)

### MemberService()

#### A. Tổng quan

- **Mục đích:** Khởi tạo Service với các Repository và AuthService.
- **Tham số:** `MemberRepository& memberRepo, AuthService& authService) : memberRepo(memberRepo), authService(authService`.
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
MemberService::MemberService(MemberRepository& memberRepo, AuthService& authService)
    : memberRepo(memberRepo),
      authService(authService)
{
}
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
bool MemberService::load()
{
    return memberRepo.load();
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
bool MemberService::save()
{
    return memberRepo.save();
}
```

Repository thực hiện thao tác dữ liệu. `save()` ghi trạng thái Repository ra TXT tại thời điểm lời gọi; thay đổi RAM trước đó không tự đồng nghĩa với đã lưu thành công.

### addMember()

#### A. Tổng quan

- **Mục đích:** Kiểm tra quyền. Xem các nhánh cụ thể trong code.
- **Tham số:** `Member* member`.
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
Cập nhật qua Repository
    |
    v
Từ chối dữ liệu không hợp lệ
    |
    v
Chuẩn bị và xử lý dữ liệu
    |
    v
Kết thúc
```

#### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1 — Kiểm tra quyền.**

```cpp
bool MemberService::addMember(Member* member)
{
    authService.requirePermission("MEMBER_ADD");

    if (!memberRepo.isLoaded() || member == nullptr)
    {
        return false;
    }

    vector<string> existingIds;
```

`AuthService` kiểm tra quyền và có thể ném ngoại lệ. `nullptr` cho biết không có đối tượng để truy cập. `false` báo thao tác không thành công. Quyền `MEMBER_ADD` được kiểm tra trước bước xử lý kế tiếp; thiếu quyền sẽ ném ngoại lệ, vì vậy caller không nên xem đó là kết quả `false`. Kiểm tra `isLoaded()` tránh dùng dữ liệu Repository chưa được nạp; nhánh không đạt kết thúc ngay theo giá trị `return` trong code. Nhánh `false` dừng hàm tại đây; những câu lệnh phía sau không chạy, nên không có cập nhật nào từ các bước sau.

**Bước 2 — Tra cứu dữ liệu.**

```cpp

    for (int i = 0; i < memberRepo.count(); i++)
    {
        Member* current = memberRepo.getAt(i);
        if (current == nullptr)
        {
            return false;
        }
        existingIds.push_back(current->getMemberId());
    }
```

`nullptr` cho biết không có đối tượng để truy cập. `false` báo thao tác không thành công. Vòng `for` duyệt lần lượt các phần tử. Nhánh `false` dừng hàm tại đây; những câu lệnh phía sau không chạy, nên không có cập nhật nào từ các bước sau. Vòng lặp xử lý từng phần tử theo thứ tự; điều kiện bên trong quyết định phần tử được giữ, bỏ qua hay đưa vào kết quả.

**Bước 3 — Cập nhật qua Repository.**

```cpp
    string memberId = IdGenerator::generateNextId("MEM", existingIds);
    Member* newMember = new Member(memberId,
                                   member->getFullName(),
                                   member->getPhone(),
                                   member->getPoints());

    try
    {
        if (!memberRepo.add(newMember))
        {
```

Repository thực hiện thao tác dữ liệu. Khi `add()` thành công, Repository nhận đối tượng và quản lý vòng đời của nó; nếu thất bại, bên gọi vẫn phải xét trách nhiệm giải phóng con trỏ. Khối `try/catch` giữ lỗi phát sinh trong phạm vi xử lý này và chuyển sang nhánh khôi phục hoặc báo lỗi đã viết trong code. `new` cấp phát đối tượng trên heap; cần theo dõi thời điểm quyền sở hữu được chuyển sang Repository để tránh rò rỉ bộ nhớ.

**Bước 4 — Từ chối dữ liệu không hợp lệ.**

```cpp
            delete newMember;
            return false;
        }
    }
    catch (...)
    {
        delete newMember;
        throw;
    }
    return true;
```

`false` báo thao tác không thành công. Nhánh `false` dừng hàm tại đây; những câu lệnh phía sau không chạy, nên không có cập nhật nào từ các bước sau. Khối `try/catch` giữ lỗi phát sinh trong phạm vi xử lý này và chuyển sang nhánh khôi phục hoặc báo lỗi đã viết trong code. `delete` giải phóng vùng nhớ của đối tượng, sau đó không được dùng lại con trỏ trỏ đến vùng nhớ đó.

**Bước 5 — Chuẩn bị và xử lý dữ liệu.**

```cpp
}
```

Các câu lệnh được thực thi theo đúng thứ tự trong `.cpp`; điều kiện và nhánh trả về nằm ngay trong code. Giá trị được khai báo ở đây được dùng trong các nhánh tiếp theo; hãy theo dõi nơi nó được trả về hoặc chuyển vào Repository để hiểu tác động lên trạng thái.

### updateMember()

#### A. Tổng quan

- **Mục đích:** Kiểm tra quyền. Xem các nhánh cụ thể trong code.
- **Tham số:** `const Member& newValues`.
- **Giá trị trả về:** `bool`; các nhánh `return` được ghi đầy đủ bên dưới.
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
bool MemberService::updateMember(const Member& newValues)
{
    authService.requirePermission("MEMBER_UPDATE");
    return memberRepo.update(newValues);
}
```

`AuthService` kiểm tra quyền và có thể ném ngoại lệ. Repository thực hiện thao tác dữ liệu. Quyền `MEMBER_UPDATE` được kiểm tra trước bước xử lý kế tiếp; thiếu quyền sẽ ném ngoại lệ, vì vậy caller không nên xem đó là kết quả `false`. `update()` chuyển việc thay đổi dữ liệu cho Repository; kết quả trả về phản ánh kiểm tra và thao tác cập nhật ở tầng dữ liệu.

### findMemberById()

#### A. Tổng quan

- **Mục đích:** Kiểm tra quyền. Xem các nhánh cụ thể trong code.
- **Tham số:** `const string& memberId`.
- **Giá trị trả về:** `const Member*`; các nhánh `return` được ghi đầy đủ bên dưới.
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
const Member* MemberService::findMemberById(const string& memberId)
{
    authService.requirePermission("MEMBER_VIEW");
    return memberRepo.findById(memberId);
}
```

`AuthService` kiểm tra quyền và có thể ném ngoại lệ. Quyền `MEMBER_VIEW` được kiểm tra trước bước xử lý kế tiếp; thiếu quyền sẽ ném ngoại lệ, vì vậy caller không nên xem đó là kết quả `false`. Lệnh tra cứu trả con trỏ đến đối tượng đang được Repository quản lý. Con trỏ rỗng có nghĩa là không tìm thấy, nên bước sau chỉ được dùng đối tượng khi kiểm tra thành công.

### findMemberByPhone()

#### A. Tổng quan

- **Mục đích:** Kiểm tra quyền. Xem các nhánh cụ thể trong code.
- **Tham số:** `const string& phone`.
- **Giá trị trả về:** `const Member*`; các nhánh `return` được ghi đầy đủ bên dưới.
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
const Member* MemberService::findMemberByPhone(const string& phone)
{
    authService.requirePermission("MEMBER_VIEW");
    return memberRepo.findByPhone(phone);
}
```

`AuthService` kiểm tra quyền và có thể ném ngoại lệ. Quyền `MEMBER_VIEW` được kiểm tra trước bước xử lý kế tiếp; thiếu quyền sẽ ném ngoại lệ, vì vậy caller không nên xem đó là kết quả `false`. Lệnh tra cứu trả con trỏ đến đối tượng đang được Repository quản lý. Con trỏ rỗng có nghĩa là không tìm thấy, nên bước sau chỉ được dùng đối tượng khi kiểm tra thành công.

### getMemberAt()

#### A. Tổng quan

- **Mục đích:** Kiểm tra quyền. Xem các nhánh cụ thể trong code.
- **Tham số:** `int index`.
- **Giá trị trả về:** `const Member*`; các nhánh `return` được ghi đầy đủ bên dưới.
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
const Member* MemberService::getMemberAt(int index)
{
    authService.requirePermission("MEMBER_VIEW");
    return memberRepo.getAt(index);
}
```

`AuthService` kiểm tra quyền và có thể ném ngoại lệ. Quyền `MEMBER_VIEW` được kiểm tra trước bước xử lý kế tiếp; thiếu quyền sẽ ném ngoại lệ, vì vậy caller không nên xem đó là kết quả `false`.

### getMemberCount()

#### A. Tổng quan

- **Mục đích:** Kiểm tra quyền. Xem các nhánh cụ thể trong code.
- **Tham số:** `không có`.
- **Giá trị trả về:** `int`; các nhánh `return` được ghi đầy đủ bên dưới.
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
int MemberService::getMemberCount()
{
    authService.requirePermission("MEMBER_VIEW");
    return memberRepo.count();
}
```

`AuthService` kiểm tra quyền và có thể ném ngoại lệ. Quyền `MEMBER_VIEW` được kiểm tra trước bước xử lý kế tiếp; thiếu quyền sẽ ném ngoại lệ, vì vậy caller không nên xem đó là kết quả `false`.

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
