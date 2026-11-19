# RolePermissionRepository

## 1. Tổng quan

`RolePermissionRepository` thuộc tầng Data Access, chịu trách nhiệm đọc bảng phân quyền từ file TXT và kiểm tra một vai trò có được gán một mã quyền hay không. Repository không đăng nhập người dùng, không tự cấp hoặc thu hồi quyền và không trực tiếp xử lý nghiệp vụ.

**Các file liên quan:**
- `src/data_access/RolePermissionRepository.h`
- `src/data_access/RolePermissionRepository.cpp`
- `src/utils/FileIO.h`, `src/utils/FileIO.cpp`
- `data/role_permissions.txt`
- `tests/test_role_permission_repository.cpp`

## 2. Định dạng dữ liệu

Mỗi dòng trong `data/role_permissions.txt` có dạng:

```text
ROLE|PERMISSION
```

Ví dụ:

```text
ADMIN|MOVIE_ADD
ADMIN|MOVIE_DELETE
STAFF|MOVIE_VIEW
```

- `ROLE`: hiện hỗ trợ `ADMIN` hoặc `STAFF`.
- `PERMISSION`: mã quyền mà các Service sử dụng để kiểm tra.
- Không có dòng schema hoặc tiêu đề.
- Phiên bản đơn giản hiện tại **không đối chiếu** `PERMISSION` với danh sách 31 mã quyền đã thiết kế (gồm `PRICING_VIEW` và `PRICING_UPDATE`); vì vậy mã sai chính tả nhưng đúng định dạng vẫn có thể được nạp.

## 3. Thuộc tính

| Thuộc tính | Kiểu dữ liệu | Mục đích |
|---|---|---|
| `filePath` | `string` | Đường dẫn file TXT |
| `permissions` | `vector<string>` | Lưu nguyên từng dòng `ROLE|PERMISSION` |
| `loaded` | `bool` | Cho biết lần nạp gần nhất thành công hay không |

Không cần Model riêng hoặc kế thừa `Repository<T>`.

## 4. Các hàm

### 4.1. `RolePermissionRepository(const string& filePath)`

Lưu đường dẫn file và đặt `loaded = false`. Vector ban đầu rỗng.

### 4.2. `bool load()`

1. Đặt `loaded = false`, xóa quyền cũ.
2. Gọi `FileIO::readLines(filePath, lines)` để đọc file TXT vào `vector<string>`.
3. Duyệt từng dòng, kiểm tra có đúng một dấu `|`, hai phần không rỗng và vai trò là `ADMIN` hoặc `STAFF`.
4. Kiểm tra không có dòng trùng nhau.
5. Lưu các dòng hợp lệ vào `permissions`.
6. Nếu tất cả đều hợp lệ, đặt `loaded = true` và trả về `true`; ngược lại trả về `false`.

**Lưu ý:** Với cách cài đặt đã thảo luận, `permissions` được thêm dần trong vòng lặp. Nếu một dòng phía sau bị lỗi, vector có thể còn dữ liệu đọc dở, nhưng `loaded = false` nên `hasPermission()` luôn từ chối. Có thể cải thiện bằng vector tạm và chỉ gán vào `permissions` sau khi kiểm tra xong toàn bộ file.

**Lưu ý về xuống dòng:** `FileIO::readLines()` trên nhánh `main` chưa tự loại bỏ ký tự `\r` còn lại khi đọc file CRLF; nên thống nhất xử lý khi tích hợp để tránh sai lệch dữ liệu.

### 4.3. `bool isLoaded() const`

Trả về `loaded` để kiểm tra Repository đã nạp dữ liệu thành công chưa.

### 4.4. `bool hasPermission(const string& role, const string& permission) const`

- Nếu `loaded == false`, trả về `false`.
- Ghép `role + "|" + permission` thành khóa cần tìm.
- Duyệt `permissions`; tìm thấy chuỗi khớp hoàn toàn thì trả về `true`, nếu không thì `false`.

Ví dụ: `hasPermission("STAFF", "MOVIE_VIEW")` tìm chuỗi `STAFF|MOVIE_VIEW`.

## 5. Quy tắc và giới hạn

- File không tồn tại hoặc đọc lỗi: `load()` trả về `false`.
- Dòng thiếu dấu `|`, có nhiều dấu `|`, thiếu role hoặc permission: từ chối.
- Role khác `ADMIN`/`STAFF`: từ chối.
- Bản ghi trùng: từ chối.
- File rỗng: được nạp thành công nhưng không cấp quyền nào.
- Khi chưa nạp thành công: `hasPermission()` luôn trả về `false` (*fail closed*).
- Repository chỉ đọc bảng quyền; chưa có `save()`, `grantPermission()` hoặc `revokePermission()`.
- Việc thay đổi file TXT cần gọi lại `load()` (hoặc khởi động lại ứng dụng nếu ứng dụng chỉ nạp lúc khởi động).
- Quyền xem dữ liệu của chính mình như `INVOICE_VIEW_OWN` vẫn cần Service kiểm tra chủ sở hữu dữ liệu, không chỉ dựa vào mã quyền.

## 6. Ví dụ sử dụng

```cpp
RolePermissionRepository repo("data/role_permissions.txt");

if (!repo.load())
{
    // Khong cho phep thuc hien thao tac can quyen
    return;
}

bool allowed = repo.hasPermission("STAFF", "MOVIE_VIEW");
```

## 7. Kiểm thử

File: `tests/test_role_permission_repository.cpp`.

Các nhóm kiểm thử đã xây dựng: trạng thái trước khi load; nạp file hợp lệ; quyền tồn tại và không tồn tại; nạp lại file; file không tồn tại; thiếu hoặc thừa dấu `|`; role/permission rỗng; role không hợp lệ; dòng trùng; lỗi giữa file; file rỗng.

**Kết quả:** Người thực hiện đã xác nhận hoàn tất chạy test; chưa ghi số `Passed/Failed` vì chưa nhận được phần `SUMMARY` cụ thể.

Lệnh biên dịch tham khảo tại thư mục gốc (Git Bash):

```bash
g++ -std=c++17 -Wall -Wextra -I./src tests/test_role_permission_repository.cpp src/data_access/RolePermissionRepository.cpp src/utils/*.cpp -o test_role_permission.exe
./test_role_permission.exe
```

## 8. Hướng phát triển tiếp theo

1. Tích hợp `RolePermissionRepository` vào `AuthService`.
2. Bổ sung `AuthService::hasPermission()` và `AuthService::requirePermission()`.
3. Cập nhật các Service để kiểm tra mã quyền cụ thể thay vì chỉ kiểm tra cứng `requireAdmin()`.
4. Kiểm thử phân quyền ở tầng Service trước khi tích hợp giao diện Qt.
