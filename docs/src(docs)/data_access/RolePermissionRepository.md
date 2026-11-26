# RolePermissionRepository

## 1. Tổng quan

`RolePermissionRepository` là lớp thuộc tầng Data Access,
chịu trách nhiệm nạp, kiểm tra và tra cứu bảng phân quyền
của hệ thống.

Lớp sử dụng cơ chế RBAC (Role-Based Access Control),
trong đó quyền được xác định thông qua cặp role và permission.

**File liên quan:**

- `src/data_access/RolePermissionRepository.h`
- `src/data_access/RolePermissionRepository.cpp`
- `data/role_permissions.txt`

**Trách nhiệm chính:**

- Nạp bảng quyền từ file TXT.
- Kiểm tra tính hợp lệ của role và permission.
- Kiểm tra các cặp quyền trùng lặp.
- Tra cứu quyền của một role.
- Quản lý trạng thái nạp dữ liệu.

RolePermissionRepository không quản lý tài khoản đăng nhập,
không trực tiếp kiểm tra quyền thực hiện nghiệp vụ và không
cung cấp chức năng thêm, sửa hoặc xóa quyền.

**Trạng thái:** CHỐT MARKDOWN. Hai khối mã ở mục 5 là mã chuẩn để triển khai; chưa hoàn thành kiểm thử.

## 2. Kiến trúc

```text
GUI
 |
 v
Business Service
 |
 v
AuthService
 |
 v
RolePermissionRepository
 |
 v
FileIO
 |
 v
role_permissions.txt
```

GUI sử dụng `AuthService::hasPermission()` để điều chỉnh
việc hiển thị các chức năng.

Business Service sử dụng `AuthService::requirePermission()`
để kiểm tra quyền trước khi thực hiện nghiệp vụ.

AuthService xác định tài khoản đang đăng nhập, lấy role
và gọi RolePermissionRepository để tra cứu quyền.

RolePermissionRepository chỉ quản lý bảng quyền.

Lớp không kế thừa `IRepository<T>` và không sử dụng
`Repository<T>` vì không quản lý danh sách Model theo CRUD.

## 3. Định dạng dữ liệu

File: `data/role_permissions.txt`

```text
role|permission
```

| Trường | Kiểu dữ liệu | Điều kiện |
| --- | --- | --- |
| role | string | ADMIN hoặc STAFF |
| permission | string | Thuộc danh mục mã quyền hợp lệ |

Ví dụ:

```text
ADMIN|MOVIE_VIEW
ADMIN|MOVIE_ADD
ADMIN|MOVIE_UPDATE
STAFF|MOVIE_VIEW
STAFF|TICKET_SELL
```

### Quy tắc dữ liệu

- Mỗi dòng chứa đúng một cặp role và permission.
- Hai trường được phân cách bằng một ký tự `|`.
- Không được để trống role hoặc permission.
- Role phải hợp lệ.
- Permission phải thuộc danh mục mã quyền của hệ thống.
- Không được trùng cặp role và permission.
- Hai role khác nhau có thể được cấp cùng một permission.
- File rỗng hợp lệ nhưng không cấp quyền nào.

Danh mục mã quyền hợp lệ không phải bảng cấp quyền.

Quyền thực tế của từng role được xác định hoàn toàn
từ nội dung file `role_permissions.txt`.

## 4. Thiết kế lớp

### 4.1. Thuộc tính

| Thuộc tính | Kiểu dữ liệu | Ý nghĩa |
| --- | --- | --- |
| filePath | string | Đường dẫn file quyền |
| permissions | vector<string> | Danh sách cặp role và permission |
| loaded | bool | Trạng thái nạp dữ liệu |

Mỗi phần tử trong `permissions` có dạng:

```text
ADMIN|MOVIE_ADD
```

Sử dụng `vector<string>` vì số lượng quyền của hệ thống
tương đối nhỏ, không cần cấu trúc dữ liệu phức tạp.

### 4.2. Các hàm private

| Hàm | Chức năng |
| --- | --- |
| isValidRole() | Kiểm tra role hợp lệ |
| isValidPermission() | Kiểm tra mã quyền hợp lệ |

### 4.3. Các hàm public

| Hàm | Chức năng |
| --- | --- |
| RolePermissionRepository() | Khởi tạo Repository |
| load() | Nạp bảng quyền |
| hasPermission() | Tra cứu quyền của role |
| isLoaded() | Kiểm tra trạng thái nạp |

## 5. Mã nguồn đã chốt và triển khai

### 5.1. RolePermissionRepository.h

```cpp
#ifndef ROLE_PERMISSION_REPOSITORY_H
#define ROLE_PERMISSION_REPOSITORY_H

#include <string>
#include <vector>

class RolePermissionRepository
{
private:
    std::string filePath;
    std::vector<std::string> permissions;
    bool loaded;

    static bool isValidRole(const std::string& role);
    static bool isValidPermission(const std::string& permission);

public:
    RolePermissionRepository(const std::string& filePath);

    bool load();

    bool hasPermission(const std::string& role,
                       const std::string& permission) const;

    bool isLoaded() const;
};

#endif
```

### 5.2. RolePermissionRepository.cpp

```cpp
#include "RolePermissionRepository.h"
#include "../utils/FileIO.h"
#include <stdexcept>

using namespace std;

bool RolePermissionRepository::isValidRole(const string& role)
{
    return role == "ADMIN" || role == "STAFF";
}

bool RolePermissionRepository::isValidPermission(
    const string& permission)
{
    static const vector<string> validPermissions =
    {
        "MOVIE_VIEW",
        "MOVIE_ADD",
        "MOVIE_UPDATE",
        "MOVIE_DELETE",

        "ROOM_VIEW",
        "ROOM_ADD",
        "ROOM_UPDATE",
        "ROOM_DELETE",

        "SEAT_VIEW",
        "SEAT_ADD",
        "SEAT_UPDATE",
        "SEAT_DELETE",

        "SHOWTIME_VIEW",
        "SHOWTIME_ADD",
        "SHOWTIME_UPDATE",
        "SHOWTIME_CANCEL",
        "SHOWTIME_DELETE",

        "STAFF_VIEW",
        "STAFF_ADD",
        "STAFF_UPDATE",
        "STAFF_DEACTIVATE",

        "MEMBER_VIEW",
        "MEMBER_ADD",
        "MEMBER_UPDATE",

        "TICKET_SELL",
        "TICKET_CANCEL",

        "INVOICE_VIEW_OWN",
        "INVOICE_VIEW_ALL",

        "REPORT_VIEW",

        "PRICING_VIEW",
        "PRICING_UPDATE"
    };

    for (const string& item : validPermissions)
    {
        if (item == permission)
        {
            return true;
        }
    }

    return false;
}

RolePermissionRepository::RolePermissionRepository(
    const string& filePath)
    : filePath(filePath), loaded(false)
{
}

bool RolePermissionRepository::load()
{
    // Tu choi su dung bang quyen cu neu nap lai that bai
    loaded = false;
    permissions.clear();

    vector<string> lines;

    if (!FileIO::readLines(filePath, lines))
    {
        return false;
    }

    for (size_t i = 0; i < lines.size(); i++)
    {
        const string& line = lines[i];
        size_t pos = line.find('|');

        if (pos == string::npos ||
            pos == 0 ||
            pos == line.size() - 1 ||
            line.find('|', pos + 1) != string::npos)
        {
            throw runtime_error("Loi doc file " + filePath +
                                " tai dong " + to_string(i + 1) +
                                ": Sai dinh dang role|permission");
        }

        string role = line.substr(0, pos);
        string permission = line.substr(pos + 1);

        if (!isValidRole(role))
        {
            throw runtime_error("Loi doc file " + filePath +
                                " tai dong " + to_string(i + 1) +
                                ": Role khong hop le");
        }
        if (!isValidPermission(permission))
        {
            throw runtime_error("Loi doc file " + filePath +
                                " tai dong " + to_string(i + 1) +
                                ": Permission khong hop le");
        }

        for (size_t j = 0; j < i; j++)
        {
            if (lines[j] == line)
            {
                throw runtime_error("Loi doc file " + filePath +
                                    " tai dong " + to_string(i + 1) +
                                    ": Trung cap role va permission");
            }
        }
    }

    // Chi cap nhat bang quyen khi toan bo du lieu hop le
    permissions.swap(lines);

    loaded = true;
    return true;
}

bool RolePermissionRepository::hasPermission(
    const string& role,
    const string& permission) const
{
    if (!loaded)
    {
        return false;
    }

    string key = role + "|" + permission;

    for (const string& item : permissions)
    {
        if (item == key)
        {
            return true;
        }
    }

    return false;
}

bool RolePermissionRepository::isLoaded() const
{
    return loaded;
}
```

## 6. Giải thích thư viện và từ khóa

### 6.1. string

Sử dụng để lưu đường dẫn file, role, permission
và các cặp quyền.

### 6.2. vector

Sử dụng để lưu danh sách các dòng dữ liệu và bảng quyền.

`vector` có khả năng tự quản lý bộ nhớ và thay đổi
kích thước khi thêm hoặc xóa phần tử.

### 6.3. FileIO

Sử dụng `FileIO::readLines()` để đọc dữ liệu từ file TXT.

RolePermissionRepository không trực tiếp thao tác
với `ifstream`.

### 6.4. static

Hai hàm `isValidRole()` và `isValidPermission()` được
khai báo static vì không sử dụng thuộc tính của đối tượng.

Biến `validPermissions` được khai báo `static const`
để danh mục chỉ được khởi tạo một lần và không bị
thay đổi trong quá trình sử dụng.

### 6.5. const

Hai hàm `hasPermission()` và `isLoaded()` có từ khóa
const ở cuối khai báo vì chỉ truy vấn dữ liệu,
không thay đổi trạng thái Repository.

### 6.6. find() và string::npos

`find()` tìm vị trí xuất hiện của một ký tự trong chuỗi.

Nếu không tìm thấy, hàm trả về `string::npos`.

Ví dụ:

```cpp
string line = "ADMIN|MOVIE_ADD";
size_t pos = line.find('|');
```

Kết quả `pos = 5`.

Câu lệnh:

```cpp
line.find('|', pos + 1)
```

Tìm ký tự `|` tiếp theo sau vị trí phân cách đầu tiên,
nhằm đảm bảo mỗi dòng chỉ có đúng hai trường.

### 6.7. substr()

Dùng để lấy một phần của chuỗi.

```cpp
string role = line.substr(0, pos);
string permission = line.substr(pos + 1);
```

Với dòng `ADMIN|MOVIE_ADD`, kết quả là:

```text
role       = ADMIN
permission = MOVIE_ADD
```

### 6.8. swap()

permissions.swap(lines) hoán đổi nội dung của hai vector sau khi tất cả dòng đã qua kiểm tra. Bảng quyền chỉ được công bố sau thao tác này và loaded = true. Vector lines cũ, lúc đó rỗng vì permissions đã clear, được tự giải phóng.

### 6.9. runtime_error, to_string và chỉ số

`<stdexcept>` cung cấp `runtime_error`, còn `<string>` cung cấp `to_string()`; thông báo lỗi ghép `filePath`, số dòng `i + 1` và nguyên nhân. Chỉ số `i`/`j` có kiểu `size_t` để duyệt vector; số dòng báo cho người đọc bắt đầu từ 1, còn chỉ số vector bắt đầu từ 0. Ngoại lệ dữ liệu sai không bị nuốt. Không đọc được file vẫn trả `false`.

## 7. Giải thích các hàm private

### 7.1. isValidRole()

**Tổng quan:** Nhận const string& role, trả true chỉ cho ADMIN hoặc STAFF; không cấp quyền.

**Luồng hoạt động chi tiết:**

    role == ADMIN? → true
    không → role == STAFF? → true/false

**Giải thích chi tiết:** Toán tử || trả true khi một trong hai phép so sánh bằng chuỗi thành công. Hàm static không dùng trạng thái của Repository; role lạ hoặc rỗng trả false.

### 7.2. isValidPermission()

**Tổng quan:** Nhận const string& permission, trả true khi mã nằm trong danh mục 31 mã của business_rules.md; mã lạ/rỗng trả false. Danh mục này xác định mã hợp lệ, không phải quyền mặc định của role.

**Luồng hoạt động chi tiết:**

    khởi tạo static const vector<string> một lần
      → duyệt từng item
        → item == permission? → true
      → hết danh sách → false

**Giải thích chi tiết:** Vector const cục bộ static lưu đúng 31 mã, được khởi tạo một lần và không thể sửa qua hàm. Vòng for tham chiếu const string& để tránh sao chép mã. Gặp mã trùng đầu vào thì return true ngay; hết vòng trả false. Không tự cấp quyền cho ADMIN, không đọc file khác và không dùng Permission.h.

## 8. Giải thích các hàm public

### 8.1. Constructor

**Tổng quan:** Nhận const string& filePath, không trả giá trị. Giữ đường dẫn file quyền, loaded ban đầu false.

**Luồng hoạt động chi tiết:**

    filePath → initializer list gán filePath và loaded=false
             → permissions tự khởi tạo rỗng

**Giải thích chi tiết:** Danh sách khởi tạo tạo filePath từ tham số và cờ loaded=false; vector permissions mặc định rỗng. Constructor không đọc file. Caller truyền đường dẫn đến file role_permissions.txt được dùng trong ứng dụng; lớp không tự tạo file dữ liệu.

### 8.2. load()

**Tổng quan:** Không tham số. Trả true cho file hợp lệ hoặc rỗng, false khi không đọc được file; ném runtime_error có đường dẫn, số dòng và nguyên nhân khi dòng sai. Cho phép nạp lại. Sau bất kỳ lần nạp thất bại, bảng quyền không còn hiệu lực.

**Luồng hoạt động chi tiết:**

    loaded=false → permissions.clear()
      → FileIO::readLines(filePath, lines)?
        không → false
        có → for i = 0..lines.size()-1
          → kiểm đúng một dấu | và hai trường không rỗng
             sai → runtime_error(file, i+1, định dạng)
          → tách role, permission
          → isValidRole? sai → runtime_error(file, i+1, role)
          → isValidPermission? sai → runtime_error(file, i+1, permission)
          → for j = 0..i-1: lines[j] == line?
             có → runtime_error(file, i+1, trùng cặp)
      → permissions.swap(lines) → loaded=true → true

**Giải thích chi tiết từng đoạn:** loaded=false và permissions.clear() chạy trước đọc file, thu hồi mọi quyền cũ ngay cả khi lần nạp mới thất bại. vector<string> lines là vùng chứa cục bộ của FileIO::readLines; đọc không thành công trả false, không đổi cờ. Không có tempPermissions và không đưa dòng chưa hợp lệ vào permissions.

Vòng ngoài dùng i để vừa truy cập lines[i], vừa báo số dòng i+1. line là tham chiếu const tới dòng hiện tại. find('|') lấy dấu phân cách đầu; npos, vị trí đầu/cuối hoặc find tiếp thấy dấu thứ hai đều là sai định dạng. Mỗi nhánh sai ném runtime_error ghép filePath, số dòng và nguyên nhân; chuỗi rỗng cũng bị chặn ở điều kiện định dạng.

substr(0, pos) và substr(pos+1) tạo hai trường. isValidRole kiểm ADMIN/STAFF; isValidPermission kiểm danh mục 31 mã. Hai nhánh lỗi riêng cho biết trường nào sai. Vòng trong j < i so từng dòng trước đó với dòng hiện tại; trùng cặp nguyên văn bị ném lỗi, còn cùng permission ở hai role khác nhau được phép.

Sau khi duyệt hết, permissions.swap(lines) công bố toàn bộ bảng quyền trong một bước, rồi loaded=true. File rỗng không chạy vòng lặp, swap vector rỗng và loaded=true, nhưng không cấp quyền. Lỗi dữ liệu hoặc ngoại lệ đọc/cấp phát chuyển lên caller; do cờ đã false và vector đã clear, hasPermission tiếp tục từ chối. Không có transaction ngoài bộ nhớ.

### 8.3. hasPermission()

**Tổng quan:** Nhận const string& role và permission, trả bool; tra cứu bảng quyền đã nạp, không thay đổi trạng thái.

**Luồng hoạt động chi tiết:**

    !loaded? → false
    key = role + "|" + permission
    duyệt permissions → item == key? → true
                      → hết danh sách → false

**Giải thích chi tiết:** Điều kiện đầu từ chối mọi quyền khi chưa nạp hoặc nạp lại lỗi. Phép nối chuỗi tạo cặp đúng schema; vòng for so toàn bộ cặp, tìm thấy thì trả true. Không xác thực lại role/permission đầu vào vì bảng đã hợp lệ sau load; không có quyền dự phòng cho ADMIN.

### 8.4. isLoaded()

**Tổng quan:** Không tham số; trả bool trạng thái bảng quyền đang dùng, không đổi dữ liệu.

**Luồng hoạt động chi tiết:**

    isLoaded() → return loaded

**Giải thích chi tiết:** Trả cờ chỉ được đặt true sau khi toàn file qua validation và swap. Load mới đặt false trước mọi thao tác, kể cả lần nạp lại.

## 9. Quản lý bộ nhớ

RolePermissionRepository sử dụng string và vector
để quản lý dữ liệu.

Không sử dụng con trỏ động, new hoặc delete.

Không cần tự định nghĩa destructor.

Vector lines là biến cục bộ, được tự động giải phóng khi kết thúc hàm load(). Sau `permissions.swap(lines)`, vector này nhận trạng thái cũ đã được clear. Không tạo `tempPermissions` và không cấp phát đối tượng Model.

## 10. DIP và phân quyền

RolePermissionRepository không kế thừa IRepository<T>
vì không có các nghiệp vụ CRUD tương ứng.

Việc trừu tượng hóa dependency giữa AuthService và
RolePermissionRepository sẽ được xem xét khi refactor
tầng Business.

### Nguyên tắc phân quyền

- GUI sử dụng hasPermission() để điều chỉnh giao diện.
- Service sử dụng requirePermission() để bảo vệ nghiệp vụ.
- AuthService quản lý phiên đăng nhập và kiểm tra quyền.
- RolePermissionRepository tra cứu bảng quyền.
- Các Repository khác không xử lý phân quyền.

Role không tự cấp quyền.

Mọi quyền thao tác phải được xác định từ bảng quyền
đã nạp thành công.

### 10.1. Tương thích với nơi gọi load()

Đã rà soát `src/`: không thấy lời gọi trực tiếp `RolePermissionRepository::load()` trong mã chạy hiện tại. `AuthService` giữ tham chiếu repository và kiểm tra `isLoaded()` trong `hasPermission()`; khi lần nạp mới thất bại, kết quả tra quyền là `false`. Tầng khởi tạo ứng dụng hoặc caller tương lai phải xử lý `runtime_error` cho file sai, không chỉ dựa vào giá trị `false`; nhánh không đọc được file vẫn trả `false`.

`tests/legacy_test/test_role_permission_repository.cpp` hiện dùng `check(!repo.load())` cho các dòng sai định dạng, role sai và cặp trùng; sau thay đổi dự kiến, những lời gọi này sẽ ném ngoại lệ và test cũ không còn tương thích nếu chưa đổi kỳ vọng. Các test tích hợp cũ khác gọi `permissionRepo.load()` với file hợp lệ và dùng giá trị `bool`; đường hợp lệ vẫn giữ `true`, nhưng file sai cần được caller bắt ngoại lệ. Chưa sửa test hoặc mã gọi trong lần thiết kế này.

## 11. Các thay đổi so với phiên bản cũ

- Bỏ khởi tạo loaded trùng lặp trong header.
- Bổ sung isValidRole().
- Bổ sung isValidPermission().
- Kiểm tra đầy đủ danh mục mã quyền.
- Kiểm tra trực tiếp vector `lines` bằng hai vòng lặp chỉ số, không tạo `tempPermissions`.
- Dữ liệu sai ném `runtime_error` có đường dẫn, số dòng và nguyên nhân; chỉ lỗi không đọc được file trả `false`.
- Không giữ bảng quyền nạp dở khi load thất bại.
- Giữ nguyên khả năng nạp lại bảng quyền.
- Giữ nguyên các hàm public.
- Chuẩn hóa thứ tự hàm, định dạng và comment.
- Không thay đổi định dạng file TXT.

## 12. Kế hoạch kiểm thử

File kiểm thử dự kiến:

`tests/test_role_permission_repository.cpp`

### 12.1. Trạng thái ban đầu

- [ ] Repository chưa load.
- [ ] Tra cứu quyền trước khi load.

### 12.2. Load

- [ ] File hợp lệ.
- [ ] File rỗng.
- [ ] File không tồn tại.
- [ ] Thiếu ký tự phân cách.
- [ ] Thừa ký tự phân cách.
- [ ] Role rỗng.
- [ ] Permission rỗng.
- [ ] Role không hợp lệ.
- [ ] Permission không hợp lệ.
- [ ] Kiểm tra đầy đủ 31 mã quyền.
- [ ] Trùng cặp role và permission.
- [ ] Cùng permission nhưng khác role.
- [ ] File có dòng hợp lệ trước dòng lỗi.
- [ ] Xác minh ngoại lệ có đúng đường dẫn, số dòng bắt đầu từ 1 và nguyên nhân cho từng loại lỗi.
- [ ] Load lại sau lỗi đọc hoặc lỗi validation phải giữ `loaded=false`, bảng quyền rỗng và `hasPermission()==false`; sửa file rồi load lại thành công.

### 12.3. Tra cứu quyền

- [ ] Role được cấp quyền.
- [ ] Role không được cấp quyền.
- [ ] Role không hợp lệ.
- [ ] Permission không hợp lệ.
- [ ] ADMIN không được tự động cấp quyền.
- [ ] STAFF được cấp quyền theo nội dung file.

### 12.4. Nạp lại

- [ ] Nạp lại file hợp lệ.
- [ ] Nạp lại sau khi thay đổi bảng quyền.
- [ ] Quyền cũ không còn hiệu lực sau khi bị thu hồi.
- [ ] Nạp lại thất bại.
- [ ] Không sử dụng quyền cũ khi nạp lại thất bại.
- [ ] Nạp lại thành công sau lần nạp lỗi.
- [ ] Rà soát các test cũ đang kỳ vọng `load()==false` cho dữ liệu sai; xác nhận hợp đồng ngoại lệ mới trước khi chạy hồi quy.

## 13. Tiến độ triển khai

- [x] Rà soát mã nguồn hiện tại.
- [x] Xác định các nội dung cần refactor.
- [x] Xây dựng bản thiết kế Markdown.
- [x] Rà soát các nơi gọi `load()` và ghi nhận điểm không tương thích của test cũ.
- [x] Rà soát và thống nhất thiết kế.
- [x] CHỐT MARKDOWN.
- [x] Triển khai RolePermissionRepository.h.
- [x] Triển khai RolePermissionRepository.cpp.
- [x] Đối chiếu hai file mã nguồn với các khối mã ở mục 5.
- [ ] Biên dịch.
- [ ] Kiểm thử.
- [ ] Cập nhật kết quả kiểm thử và tiến độ.

## 14. Kết luận

RolePermissionRepository chịu trách nhiệm quản lý
và tra cứu bảng quyền của hệ thống.

Lớp giữ nguyên kiến trúc đơn giản. Bản thiết kế đã CHỐT MARKDOWN và triển khai validation 31 mã, lỗi có file/dòng cho dữ liệu sai và từ chối toàn bộ quyền khi nạp lại thất bại. Chưa kiểm thử các thay đổi này.

Cơ chế phân quyền nghiệp vụ vẫn được thực hiện tập trung
thông qua AuthService và các Business Service.
