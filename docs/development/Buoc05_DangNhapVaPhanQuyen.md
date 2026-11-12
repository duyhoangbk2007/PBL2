# Bước 05 – Đăng nhập và phân quyền

## Kết quả kiểm thử 2026-11-12 — hoàn thành phần C++ Bước 05

Đã thêm và chạy `tests/test_auth.cpp`, kiểm thử tích hợp `AuthService → UserRepository → Repository<User> → FileIO → TXT` cùng các Model thật. Kết quả **14 nhóm PASS, 0 FAILED**, chương trình trả exit code 0; biên dịch C++17 không có cảnh báo. Không cần sửa mã nghiệp vụ sau lần chạy này.

Đã hoàn thành checklist C++ của Bước 05: Repository, Service, kiểm thử đăng nhập/phân quyền và tài liệu. Kết quả này không bao gồm GUI Qt, Service nghiệp vụ của các bước sau, tích hợp toàn ứng dụng, review/merge nhánh hoặc đóng gói. Các ghi nhận “chưa kiểm thử” bên dưới là lịch sử trước lần chạy này.

### Cách chạy lại từ thư mục gốc (PowerShell, MinGW g++)

```powershell
New-Item -ItemType Directory -Force build | Out-Null
$modelSources = @(Get-ChildItem -LiteralPath src/models -Filter '*.cpp' | ForEach-Object { $_.FullName })
g++ -std=c++17 -Wall -Wextra -pedantic -I . tests/test_auth.cpp src/business/AuthService.cpp src/data_access/UserRepository.cpp src/utils/FileIO.cpp @modelSources -o build/test_auth.exe
if ($LASTEXITCODE -ne 0) { throw 'Build test_auth failed' }
$env:PATH = 'C:\msys64\ucrt64\bin;' + $env:PATH
& .\build\test_auth.exe
if ($LASTEXITCODE -ne 0) { throw 'test_auth failed' }
```

Đường dẫn MSYS2 trên là toolchain của máy đã chạy; nếu cài nơi khác, dùng thư mục `bin` của toolchain tương ứng. Link các `.cpp` Model để đáp ứng các quan hệ giữa chúng; không link `src/main.cpp` vì runner có `main()` riêng. Không cần Qt hoặc framework test mới.

Runner tạo từng thư mục `pbl2_auth_tests_*` riêng trong thư mục tạm hệ điều hành, chỉ dùng đường dẫn file dưới thư mục đó và dọn sau khi chạy. Không đọc/ghi `data/users.txt` thật. File thực thi ở `build/` đã được `.gitignore` bỏ qua.

### Kết quả thực tế

```text
[PASS] Unloaded repository and missing file
[PASS] Admin login and permissions
[PASS] Staff login and permissions
[PASS] Invalid credentials clear previous session
[PASS] Logout and switch user
[PASS] Inactive Admin and Staff
[PASS] Disable account during session
[PASS] Malformed records block loading and saving
[PASS] Duplicate keys and retry after repair
[PASS] Empty account file
[PASS] Add/update and UTF-8 save/load
[PASS] Rejected mutations preserve accounts
[PASS] Reload rejection preserves session
[PASS] Write failure is reported
RESULT: 14 passed, 0 failed
```

Trong đó kiểm tra thông tin sai gồm 10 cặp đầu vào (không có tài khoản, sai mật khẩu, rỗng, khác hoa/thường và khoảng trắng); kiểm tra parser gồm 16 bản ghi sai. Mỗi lần đăng nhập thất bại đều kiểm tra phiên Admin cũ bị xóa. Admin và Staff đều qua `requireLogin()`, chỉ Admin qua `requireAdmin()`. Tài khoản bị khóa trong RAM bị chặn ngay khi kiểm tra quyền, và trạng thái khóa được giữ sau save/load.

Các bài kiểm thử còn xác nhận ID/username trùng bị từ chối, lỗi nạp không giữ dữ liệu/chỉ mục dở dang, không ghi đè file lỗi, đổi username cập nhật đúng chỉ mục, từ chối đổi role, giữ tài khoản Admin và tên UTF-8 sau lưu/nạp. Chưa mô phỏng mất điện hoặc lỗi cấp phát bộ nhớ; việc ghi TXT vẫn không nguyên tử như thiết kế đã ghi.

## Cập nhật 2026-11-12 — AuthService

Đã viết `src/business/AuthService.h/.cpp`, nhận tham chiếu tới `UserRepository`. Bước 05 vẫn đang thực hiện, chưa kiểm thử runtime hoặc tích hợp GUI. Các mục ngày 2026-11-11 bên dưới giữ nguyên ghi nhận lịch sử.

| Hàm | Hành vi |
| --- | --- |
| `AuthService(users)` | Khởi tạo chưa đăng nhập; `users` phải sống lâu hơn service. Dùng chung một AuthService cho các Service nghiệp vụ trong phiên ứng dụng, không copy. |
| `login(username, password)` | Xóa phiên cũ trước mỗi lần thử. Trả `true` khi đúng thông tin và tài khoản active, role hợp lệ; `false` khi sai thông tin/rỗng/inactive. Ném `runtime_error` nếu repo chưa load. |
| `logout()` | Xóa ID đang đăng nhập trong RAM. |
| `getCurrentUser()` | Tra lại ID trong repo và kiểm tra active/role; trả `const User*` mượn hoặc `nullptr`. Không delete con trỏ này. |
| `isLoggedIn()` | Có tài khoản đăng nhập hợp lệ tại thời điểm gọi hay không. |
| `isAdmin()` / `isStaff()` | Kiểm tra đúng vai trò tương ứng của tài khoản đang hoạt động. Admin không được coi là role Staff. |
| `requireLogin()` | Trả `const User&` nếu đã đăng nhập bằng Admin hoặc Staff; nếu không thì ném `runtime_error`. |
| `requireAdmin()` | Trả `const User&` nếu đang đăng nhập bằng Admin; nếu không thì ném `runtime_error`. |

Service chỉ lưu ID bất biến trong phiên, không sao chép User hoặc lưu lại mật khẩu. Đăng nhập đối chiếu nguyên chuỗi (không trim/đổi hoa thường) theo định dạng mật khẩu văn bản hiện hành của đồ án. Không đọc/ghi TXT trực tiếp và không tự load repo.

Mỗi lần kiểm tra quyền đều đọc trạng thái hiện tại trong RAM: tài khoản inactive không có quyền. Cờ inactive không tự xóa ID phiên; nếu bật lại tài khoản, phiên đó có thể hợp lệ lại. Muốn kết thúc phiên phải gọi `logout()`. Việc sửa file từ ngoài ứng dụng không được tự nạp lại.

Service quản trị phải gọi `requireAdmin()` trước thao tác. Nghiệp vụ cho cả Admin và Staff (ví dụ bán vé) gọi `requireLogin()`; dùng `getUserId()` của kết quả để ghi đúng người thao tác. Quy tắc xem hóa đơn của chính mình và các điều kiện nghiệp vụ cụ thể vẫn thuộc Service tương ứng. Không dùng riêng `isStaff()` để quyết định quyền bán vì Admin cũng được bán.

Ví dụ sử dụng khi đã có file tài khoản:

```cpp
#include "src/data_access/UserRepository.h"
#include "src/business/AuthService.h"

UserRepository users("data/users.txt");
users.load();
AuthService auth(users);
if (auth.login(username, password)) {
    const User& seller = auth.requireLogin();
    // seller.getUserId() la ma nguoi thao tac.
}
auth.logout();
```

Kiểm tra cú pháp đã thực hiện:

```text
g++ -std=c++17 -Wall -Wextra -pedantic -fsyntax-only src/business/AuthService.cpp src/data_access/UserRepository.cpp
```

Kết quả: exit code 0, không có cảnh báo. Chưa chạy kiểm thử hành vi; chưa xác nhận hoàn thành toàn bộ Bước 05.

## Cập nhật 2026-11-11

Đã viết `src/data_access/UserRepository.h/.cpp` cho phần Kiệt. Bước 05 đang thực hiện; chưa có `AuthService` hoặc kiểm thử hành vi đăng nhập.

`UserRepository` dùng composition: chứa `Repository<User> repo`, truyền ba hàm static lấy ID, parse và serialize vào constructor. Không kế thừa repo chung. `Repository<User>` sở hữu User; hai `std::unordered_map` theo `userId` và `username` chỉ giữ con trỏ để tra cứu.

## API hiện có

| Hàm | Hành vi |
| --- | --- |
| `UserRepository(path)` | Nhận đường dẫn `users.txt`; chưa đọc hay tạo file. |
| `load()` | Nạp một lần thành công; kiểm tra dữ liệu rồi dựng hai chỉ mục. Lỗi dữ liệu/file ném `std::runtime_error`; khi lỗi nạp ban đầu, repo rỗng và không cho lưu, có thể sửa file rồi thử lại. |
| `save()` | Ghi toàn bộ tài khoản qua repo chung; ném lỗi nếu chưa load hoặc ghi thất bại. |
| `findById(id)` / `findByUsername(username)` | Trả `const User*`, không thấy trả `nullptr`. Username phân biệt hoa/thường. |
| `getAt(index)` / `count()` / `isLoaded()` | Duyệt theo thứ tự nạp/thêm, đếm và xem trạng thái. Index sai trả `nullptr`. |
| `add(User*)` | Cần load trước; kiểm tra dữ liệu, ID và username trùng; trả `bool`. Chỉ nhận quyền sở hữu khi trả `true`. |
| `update(const User&)` | Tìm theo ID và sửa username/password/fullName/active; chặn username trùng và đổi role. Giữ địa chỉ User cùng liên kết Invoice. Trả `bool`. |

`add()` và `update()` chỉ thay đổi RAM, cần gọi `save()` để lưu. ID phải được cấp từ luồng nghiệp vụ dùng `IdGenerator`; repo không tự cấp mã hoặc ghi `sequences.txt`. Chưa có API xóa tài khoản.

Parser yêu cầu đúng sáu trường `userId|username|password|fullName|role|active`, không có tiêu đề/schema; ID `USR0001`–`USR9999`, trường văn bản không rỗng/không chứa ký tự điều khiển hoặc `|`, username không có dấu cách đầu/cuối, role `ADMIN`/`STAFF`, active `0`/`1`. Lỗi nạp có đường dẫn và số dòng đối với bản ghi sai hoặc khóa trùng, không ghi mật khẩu vào thông báo lỗi.

Tài khoản inactive vẫn được nạp và tra cứu để giữ lịch sử. `AuthService` cần kiểm tra mật khẩu và active; Service kiểm tra quyền thao tác. Repository không tự quyết định người dùng nào được thêm/sửa tài khoản.

## Vòng đời và sử dụng

Repo không được copy. Không gọi `delete` trên con trỏ tra cứu. Sau `add()` thành công, bên gọi không tiếp tục sửa đối tượng bằng con trỏ cũ: dùng `update()` để giữ chỉ mục username đồng bộ. Repo phải sống lâu hơn các đối tượng giữ liên kết tới User. Không cho load lại sau lần thành công nhằm tránh phá các liên kết đang có.

```cpp
#include "src/data_access/UserRepository.h"

UserRepository users("data/users.txt");
users.load(); // File phai ton tai; file rong hop le.
const User* user = users.findByUsername("kiet");
// AuthService se xu ly user == nullptr, mat khau, active va role.
```

## Kiểm tra đã thực hiện

```text
g++ -std=c++17 -Wall -Wextra -pedantic -fsyntax-only src/data_access/UserRepository.cpp
```

Kết quả: exit code 0, không có cảnh báo. Đây là kiểm tra cú pháp, chưa kiểm thử runtime hoặc đăng nhập. Chưa có dữ liệu tài khoản thực được tạo/thay đổi.
