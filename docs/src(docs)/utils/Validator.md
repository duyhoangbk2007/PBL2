# Validator

## 1. Tổng quan

`Validator` là lớp tiện ích thuộc tầng **Utils**, cung cấp các hàm `static` để kiểm tra định dạng và tính hợp lệ cơ bản của văn bản, ngày, mã định danh và ngày giờ trong dự án quản lý rạp chiếu phim PBL2.

Lớp không lưu trạng thái, không cần khởi tạo đối tượng và **không trực tiếp đọc/ghi file TXT**. Các Repository và Service có thể gọi các hàm kiểm tra trước khi xử lý dữ liệu.

**File liên quan:**
- `src/utils/Validator.h`
- `src/utils/Validator.cpp`
- `tests/test_validator.cpp` (bộ kiểm thử Validator đã được thực hiện trong quá trình phát triển)

## 2. Trách nhiệm và phạm vi

- Kiểm tra chuỗi văn bản có nội dung và không chứa ký tự phá vỡ định dạng TXT.
- Kiểm tra ngày theo định dạng `YYYY-MM-DD`, bao gồm số ngày mỗi tháng và năm nhuận.
- Kiểm tra mã ID theo tiền tố được truyền vào và phần số gồm bốn chữ số khác `0000`.
- Kiểm tra ngày giờ theo định dạng `YYYY-MM-DDTHH:MM:SS`.
- Chỉ **kiểm tra** dữ liệu, không tự sửa, chuẩn hóa hoặc lưu dữ liệu.
- Không thay thế các quy tắc nghiệp vụ riêng như kiểm tra ID trùng, ngày phát hành phim, thời gian bắt đầu suất chiếu hoặc quyền tài khoản.

## 3. Cấu trúc lớp

```cpp
class Validator
{
public:
    static bool isValidText(const string& text);
    static bool isValidDate(const string& date);
    static bool isValidId(const string& id, const string& prefix);
    static bool isValidDateTime(const string& dateTime);
};
```

Tất cả các hàm đều là `static`, nên gọi trực tiếp bằng tên lớp:

```cpp
bool valid = Validator::isValidDate("2026-11-17");
```

Không cần tạo `Validator validator;`. Các hàm trả về `true` nếu đầu vào thỏa điều kiện kiểm tra và `false` nếu không thỏa.

## 4. Quy tắc kiểm tra chung

| Nội dung | Quy tắc hiện tại |
| --- | --- |
| Kiểu đầu vào | `const std::string&` |
| Kiểu kết quả | `bool` |
| Dữ liệu đầu vào | Không bị chỉnh sửa |
| Lưu trữ | Không đọc hoặc ghi file |
| Ngoại lệ | Không chủ động ném ngoại lệ cho dữ liệu sai định dạng; sử dụng `stoi` sau khi đã kiểm tra cấu trúc và chữ số |

Các hàm kiểm tra **định dạng** chứ không kiểm tra sự tồn tại của bản ghi trong Repository.

## 5. Danh sách hàm

| Hàm | Tham số | Chức năng |
| --- | --- | --- |
| `isValidText(text)` | Chuỗi văn bản | Kiểm tra chuỗi có nội dung, không chứa `\|` hoặc ký tự điều khiển |
| `isValidDate(date)` | Chuỗi ngày | Kiểm tra ngày lịch hợp lệ |
| `isValidId(id, prefix)` | ID và tiền tố | Kiểm tra ID theo mẫu `XXXdddd` |
| `isValidDateTime(dateTime)` | Chuỗi ngày giờ | Kiểm tra ngày hợp lệ và giờ 24 tiếng |

## 6. Chi tiết các hàm

### 6.1. `isValidText(const string& text)`

**Mục đích:** Kiểm tra chuỗi có thể lưu dưới dạng một trường văn bản trong file TXT dùng dấu `|` làm ký tự phân cách.

**Quy trình:**
1. Duyệt từng byte của chuỗi dưới dạng `unsigned char`.
2. Trả `false` nếu gặp dấu `|`, ký tự có mã nhỏ hơn `32`, hoặc mã `127`.
3. Theo dõi xem chuỗi có ít nhất một ký tự khác dấu cách `' '` hay không.
4. Trả `true` nếu có nội dung và không chứa ký tự bị cấm.

**Ví dụ:**

```cpp
Validator::isValidText("Avengers"); // true
Validator::isValidText("A B");      // true
Validator::isValidText("");         // false
Validator::isValidText("   ");      // false
Validator::isValidText("A|B");      // false
Validator::isValidText("A\nB");     // false
```

**Lưu ý:** Hàm **không cấm dấu cách đầu/cuối**; ví dụ `"  ABC  "` vẫn hợp lệ. Nếu trường dữ liệu như username không được có dấu cách đầu/cuối, Repository phải kiểm tra thêm. Hàm cũng không giới hạn độ dài, không tự loại bỏ khoảng trắng và không kiểm tra ngữ nghĩa của chuỗi.

### 6.2. `isValidDate(const string& date)`

**Mục đích:** Kiểm tra ngày lịch theo đúng định dạng `YYYY-MM-DD`.

**Quy trình:**
1. Chuỗi phải có đúng 10 ký tự, dấu `-` tại vị trí 4 và 7 (đánh số từ 0).
2. Các vị trí còn lại phải là chữ số ASCII từ `0` đến `9`.
3. Tách `year`, `month`, `day` bằng `stoi`.
4. Năm phải từ `1` trở lên, tháng từ `1` đến `12`.
5. Xác định số ngày của tháng, trong đó tháng 2 có 29 ngày nếu là năm nhuận.
6. Ngày phải nằm trong khoảng từ `1` đến số ngày của tháng.

**Quy tắc năm nhuận:**

```cpp
bool leapYear = (year % 400 == 0) ||
                (year % 4 == 0 && year % 100 != 0);
```

**Ví dụ:**

```cpp
Validator::isValidDate("2024-02-29"); // true
Validator::isValidDate("2025-02-29"); // false
Validator::isValidDate("2000-02-29"); // true
Validator::isValidDate("1900-02-29"); // false
Validator::isValidDate("2026-11-17"); // true
Validator::isValidDate("2026-13-01"); // false
Validator::isValidDate("2026-04-31"); // false
Validator::isValidDate("2026-1-07");  // false
```

**Lưu ý:** Hàm chỉ kiểm tra ngày có tồn tại trong lịch, không kiểm tra ngày đó nằm trong quá khứ, tương lai hoặc khoảng phát hành phim. Năm `0000` không hợp lệ.

### 6.3. `isValidId(const string& id, const string& prefix)`

**Mục đích:** Kiểm tra mã định danh theo mẫu `XXXdddd`, trong đó `XXX` phải khớp chính xác với `prefix` được cung cấp, còn `dddd` là bốn chữ số.

**Quy trình:**
1. `prefix` phải dài đúng 3 ký tự, `id` dài đúng 7 ký tự.
2. Ba ký tự đầu của `id` phải trùng `prefix`.
3. Bốn ký tự cuối phải là chữ số `0`–`9`.
4. Bốn ký tự cuối không được đồng thời là `0000`.

**Ví dụ:**

```cpp
Validator::isValidId("USR0001", "USR"); // true
Validator::isValidId("MOV0123", "MOV"); // true
Validator::isValidId("USR0000", "USR"); // false
Validator::isValidId("USR123",  "USR"); // false
Validator::isValidId("MOV0001", "USR"); // false
Validator::isValidId("USR12A4", "USR"); // false
```

**Lưu ý:** Hàm không tự xác định danh sách tiền tố hợp lệ và **không kiểm tra ID đã tồn tại hay chưa**. Ví dụ `isValidId("ABC0001", "ABC")` vẫn trả `true`. Quy tắc tiền tố được phép sử dụng và tính duy nhất của ID thuộc trách nhiệm của tầng gọi hàm.

### 6.4. `isValidDateTime(const string& dateTime)`

**Mục đích:** Kiểm tra thời điểm theo định dạng `YYYY-MM-DDTHH:MM:SS`.

**Quy trình:**
1. Chuỗi phải dài đúng 19 ký tự.
2. Ký tự tại vị trí 10 phải là `T`; vị trí 13 và 16 phải là `:`.
3. Mười ký tự đầu phải là ngày hợp lệ theo `isValidDate()`.
4. Các vị trí thuộc giờ, phút, giây phải là chữ số ASCII.
5. Giờ thuộc `00`–`23`; phút và giây thuộc `00`–`59`.

**Ví dụ:**

```cpp
Validator::isValidDateTime("2026-11-17T19:30:00"); // true
Validator::isValidDateTime("2024-02-29T23:59:59"); // true
Validator::isValidDateTime("2026-11-17T24:00:00"); // false
Validator::isValidDateTime("2026-11-17T19:60:00"); // false
Validator::isValidDateTime("2026-11-17 19:30:00"); // false
Validator::isValidDateTime("2025-02-29T12:00:00"); // false
```

**Lưu ý:** Hàm không kiểm tra thời điểm nằm trong tương lai hay không, không kiểm tra múi giờ, và không hỗ trợ hậu tố `Z`, độ lệch múi giờ hoặc giây nhuận `60`. Những quy tắc như suất chiếu phải bắt đầu trong tương lai được xử lý ở `ShowtimeService` và các tiện ích thời gian liên quan.

## 7. Sử dụng trong kiến trúc dự án

- **Model:** Chứa dữ liệu và các thao tác trên đối tượng; không thay thế kiểm tra định dạng tại ranh giới dữ liệu.
- **Repository:** Sử dụng `Validator` khi xác thực dữ liệu đọc từ TXT hoặc dữ liệu chuẩn bị thêm/cập nhật, kết hợp kiểm tra trùng ID, tham chiếu và các ràng buộc riêng.
- **Service:** Thực hiện kiểm tra nghiệp vụ, phân quyền và điều kiện phụ thuộc trạng thái hệ thống. Ví dụ `ShowtimeService::isValidSchedule()` gọi `Validator::isValidDateTime()` nhưng còn phải kiểm tra thời điểm bắt đầu, ngày phát hành và thời gian kết thúc theo thời lượng phim.

Ví dụ:

```cpp
if (!Validator::isValidDateTime(showtime.getStartAt()))
{
    return false;
}
```

`Validator` không có `load()`, `save()` hoặc tham chiếu đến Repository vì nó không sở hữu dữ liệu.

## 8. Ví dụ sử dụng

```cpp
#include "utils/Validator.h"
using namespace std;

bool isMovieInputValid(const string& movieId,
                       const string& title,
                       const string& releaseDate)
{
    return Validator::isValidId(movieId, "MOV") &&
           Validator::isValidText(title) &&
           Validator::isValidDate(releaseDate);
}
```

Đây chỉ là ví dụ kiểm tra **một phần đầu vào**; một phim hợp lệ còn cần các trường và quy tắc khác như thời lượng, độ tuổi, ngày kết thúc và trạng thái.

## 9. Kiểm thử

**File:** `tests/test_validator.cpp` (đã được dùng để kiểm thử Validator trong quá trình phát triển dự án).

Các nhóm cần bao phủ:
- Chuỗi rỗng, chỉ có dấu cách, có dấu `|`, ký tự điều khiển và văn bản hợp lệ.
- Ngày hợp lệ/không hợp lệ; tháng 2, năm nhuận chia hết cho 4, 100 và 400.
- ID đúng/sai độ dài, tiền tố, phần chữ số và hậu tố `0000`.
- Ngày giờ đúng/sai cấu trúc; giờ `24`, phút/giây `60` và ngày không tồn tại.

**Lệnh biên dịch PowerShell từ thư mục gốc:**

```powershell
$src = Get-ChildItem -Path src -Recurse -Filter *.cpp |
    Where-Object { $_.Name -ne "main.cpp" } |
    ForEach-Object { $_.FullName }

g++ -std=c++17 -Wall -Wextra -I . -I src $src tests/test_validator.cpp -o test_validator.exe

if ($LASTEXITCODE -eq 0) {
    .\test_validator.exe
}
```

Kết quả PASS/FAIL cần lấy từ lần chạy kiểm thử thực tế; tài liệu này không tự xác nhận lại kết quả.

## 10. Lưu ý và hướng mở rộng

- `isValidText()` cho phép dấu cách ở đầu/cuối; các trường có yêu cầu chặt hơn phải kiểm tra riêng.
- `isValidId()` chỉ kiểm tra mẫu ID tương ứng với tiền tố truyền vào, không kiểm tra tính duy nhất hoặc danh sách tiền tố được phép.
- `isValidDate()` và `isValidDateTime()` kiểm tra tính hợp lệ của lịch, không so sánh hai thời điểm.
- Chưa có kiểm tra độ dài tối đa hoặc quy tắc Unicode chuyên biệt cho văn bản.
- Không nên đưa các quy tắc nghiệp vụ như phân quyền, lịch chiếu hoặc kiểm tra vé vào `Validator`.
- Nếu bổ sung hàm mới, cần cập nhật đồng thời `Validator.h`, `Validator.cpp`, bộ kiểm thử và tài liệu này.
