# MemberRepository

## 1. Tổng quan

`MemberRepository` là lớp thuộc tầng **Data Access**, chịu trách nhiệm quản lý, lưu trữ và truy xuất dữ liệu thành viên (`Member`).

Lớp sử dụng `Repository<Member>` để tái sử dụng các chức năng quản lý danh sách, đọc và ghi dữ liệu từ file TXT.

Ngoài ra, `MemberRepository` sử dụng hai `unordered_map` để tăng tốc độ tìm kiếm thành viên theo mã và số điện thoại.

`MemberRepository` không tự thao tác luồng file: `load()`/`save()` dùng `Repository<Member>` và `FileIO`. Lớp không thực hiện phân quyền hoặc nghiệp vụ tích điểm khi bán/hủy vé.

**File liên quan:**
- `src/data_access/MemberRepository.h`
- `src/data_access/MemberRepository.cpp`
- `src/data_access/Repository.h`
- `src/models/Member.h`
- `src/models/Member.cpp`
- `src/utils/Validator.h`
- File dữ liệu `members.txt` theo đường dẫn truyền vào constructor; không giả định file dữ liệu thật đã tồn tại.
- `tests/test_member_repository.cpp`

## 2. Cấu trúc lớp

### 2.1. Thuộc tính

```cpp
private:
    Repository<Member> repo;
    unordered_map<string, Member*> membersById;
    unordered_map<string, Member*> membersByPhone;
```

| Thuộc tính | Chức năng |
|---|---|
| `repo` | Quản lý danh sách đối tượng Member, đọc và ghi file TXT. |
| `membersById` | Tìm kiếm thành viên theo mã `memberId`. |
| `membersByPhone` | Tìm kiếm thành viên theo số điện thoại. |

Hai `unordered_map` chỉ lưu con trỏ đến các đối tượng do `repo` quản lý, không sở hữu đối tượng và không tự giải phóng bộ nhớ.

### 2.2. Các hàm private

```cpp
static string getMemberId(Member* member);
static Member* stringToMember(const string& line);
static string memberToString(Member* member);
static bool isValidMember(const Member* member);
```

Ngoài ra, file `.cpp` có hàm hỗ trợ `static bool isValidPhone(const string& phone)` ở phạm vi file.

### 2.3. Các hàm public

```cpp
MemberRepository(const string& filePath);

bool load();
bool save();

bool add(Member* member);
bool update(const Member& newValues);

Member* findById(const string& memberId);
Member* findByPhone(const string& phone);

Member* getAt(int index);
int count();
bool isLoaded();
```

## 3. Định dạng dữ liệu

Dữ liệu thành viên được lưu tại đường dẫn TXT truyền cho constructor; trong ứng dụng mục tiêu là `data/members.txt`.

Mỗi dòng tương ứng với một thành viên:

```text
memberId|fullName|phone|points
```

Ví dụ:

```text
MEM0001|Nguyen Van A|0901234567|100
MEM0002|Tran Thi B|0912345678|50
MEM0003|Le Van C|0987654321|0
```

### Quy tắc dữ liệu

| Trường | Quy tắc |
|---|---|
| `memberId` | Đúng định dạng `MEMxxxx`, theo `Validator::isValidId()`, duy nhất và không thay đổi. |
| `fullName` | Hợp lệ theo `Validator::isValidText()`. |
| `phone` | Gồm 10 chữ số, bắt đầu bằng `0`, không trùng với thành viên khác. |
| `points` | Số nguyên không âm. Thành viên mới có 0 điểm. |

Danh sách hóa đơn (`invoices`) thuộc quan hệ giữa các model, không được lưu trực tiếp trong `members.txt`.

## 4. Chi tiết các hàm

### 4.1. Constructor

```cpp
MemberRepository::MemberRepository(const string& filePath)
    : repo(filePath, getMemberId, stringToMember, memberToString)
{
}
```

**Chức năng:** Khởi tạo `Repository<Member>` với đường dẫn file và ba hàm callback.

- `getMemberId`: lấy mã thành viên.
- `stringToMember`: chuyển chuỗi TXT thành đối tượng Member.
- `memberToString`: chuyển đối tượng Member thành chuỗi TXT.

Constructor chưa thực hiện nạp dữ liệu. Cần gọi `load()` trước khi sử dụng các thao tác thêm hoặc cập nhật.

### 4.2. `isValidPhone()`

```cpp
static bool isValidPhone(const string& phone);
```

**Chức năng:** Kiểm tra định dạng số điện thoại.

Điều kiện hợp lệ:

- Đúng 10 ký tự.
- Ký tự đầu tiên là `0`.
- Các ký tự đều thuộc khoảng `'0'` đến `'9'`.

Trả về `true` nếu hợp lệ, ngược lại trả về `false`.

Hàm chỉ kiểm tra định dạng, không kiểm tra số điện thoại đã tồn tại hay chưa.

### 4.3. `isValidMember()`

```cpp
static bool isValidMember(const Member* member);
```

**Chức năng:** Kiểm tra dữ liệu của một đối tượng Member.

Các điều kiện:

1. Con trỏ không phải `nullptr`.
2. `memberId` hợp lệ với tiền tố `MEM`.
3. `fullName` hợp lệ theo `Validator`.
4. `phone` hợp lệ theo `isValidPhone()`.
5. `points >= 0`.

Trả về `true` khi tất cả điều kiện hợp lệ.

Hàm không kiểm tra trùng ID hoặc số điện thoại. Những điều kiện này được xử lý trong `load()`, `add()` và `update()`.

### 4.4. `getMemberId()`

```cpp
static string getMemberId(Member* member);
```

**Chức năng:** Trả về mã thành viên.

- Con trỏ hợp lệ: trả về `member->getMemberId()`.
- Con trỏ `nullptr`: trả về chuỗi rỗng.

Hàm được truyền cho `Repository<Member>` để phục vụ các thao tác liên quan đến ID.

### 4.5. `stringToMember()`

```cpp
static Member* stringToMember(const string& line);
```

**Chức năng:** Chuyển một dòng TXT thành đối tượng `Member`.

**Quy trình:**

1. Tách chuỗi bằng ký tự `|`.
2. Kiểm tra có đúng 4 trường.
3. Kiểm tra chuỗi `points` chỉ chứa chữ số.
4. Chuyển `points` sang kiểu `int` bằng `stoi()`.
5. Tạo đối tượng Member bằng `new`.
6. Kiểm tra đối tượng bằng `isValidMember()`.
7. Trả về con trỏ Member nếu hợp lệ.

Nếu dữ liệu không hợp lệ, hàm ném `runtime_error`.

Nếu đối tượng đã được cấp phát nhưng kiểm tra thất bại, code giải phóng đối tượng trước khi ném ngoại lệ.

### 4.6. `memberToString()`

```cpp
static string memberToString(Member* member);
```

**Chức năng:** Chuyển đối tượng Member thành chuỗi để ghi xuống file TXT.

Hàm kiểm tra dữ liệu bằng `isValidMember()`, sau đó tạo chuỗi:

```text
memberId|fullName|phone|points
```

Sử dụng `to_string()` để chuyển điểm từ `int` sang `string`.

Nếu dữ liệu không hợp lệ, hàm ném `runtime_error`.

### 4.7. `load()`

```cpp
bool load();
```

**Chức năng:** Nạp dữ liệu từ file và xây dựng các chỉ mục tìm kiếm.

**Quy trình:**

1. Kiểm tra Repository chưa được load.
2. Gọi `repo.load()` để đọc dữ liệu.
3. Xóa nội dung hai map trước khi xây dựng chỉ mục.
4. Duyệt các thành viên đã nạp.
5. Kiểm tra trùng ID và số điện thoại.
6. Thêm con trỏ vào `membersById` và `membersByPhone`.
7. Trả về `true` khi thành công.

Nếu phát hiện dữ liệu trùng hoặc xảy ra lỗi trong quá trình xây dựng chỉ mục, hàm dọn hai map, gọi `repo.clear()` và ném lại ngoại lệ.

**Lưu ý:** `Repository<Member>` chịu trách nhiệm đọc file và kiểm tra trùng ID. `MemberRepository` bổ sung việc kiểm tra trùng số điện thoại.

### 4.8. `save()`

```cpp
bool save();
```

**Chức năng:** Lưu danh sách thành viên xuống file TXT.

**Quy trình:**

1. Kiểm tra Repository đã load.
2. Gọi `repo.save()`.
3. Trả về `true` nếu ghi thành công.

Nếu chưa load hoặc ghi thất bại, hàm ném `runtime_error`.

Dữ liệu được ghi thông qua callback `memberToString()`.

### 4.9. `findById()`

```cpp
Member* findById(const string& memberId);
```

**Chức năng:** Tìm thành viên theo mã.

Hàm sử dụng:

```cpp
membersById.find(memberId);
```

- Tìm thấy: trả về con trỏ `Member*`.
- Không tìm thấy: trả về `nullptr`.

Độ phức tạp tìm kiếm trung bình: **O(1)**.

### 4.10. `findByPhone()`

```cpp
Member* findByPhone(const string& phone);
```

**Chức năng:** Tìm thành viên theo số điện thoại.

Hàm sử dụng `membersByPhone` để tra cứu.

- Tìm thấy: trả về `Member*`.
- Không tìm thấy: trả về `nullptr`.

Độ phức tạp tìm kiếm trung bình: **O(1)**.

### 4.11. `getAt()`

```cpp
Member* getAt(int index);
```

**Chức năng:** Lấy thành viên theo vị trí trong danh sách.

Gọi trực tiếp:

```cpp
return repo.getAt(index);
```

Nếu vị trí không hợp lệ, trả về `nullptr` theo cách triển khai của `Repository<Member>`.

### 4.12. `count()`

```cpp
int count();
```

**Chức năng:** Trả về tổng số thành viên đang được quản lý.

```cpp
return repo.count();
```

### 4.13. `isLoaded()`

```cpp
bool isLoaded();
```

**Chức năng:** Kiểm tra Repository đã nạp dữ liệu hay chưa.

```cpp
return repo.isLoaded();
```

### 4.14. `add()`

```cpp
bool add(Member* member);
```

**Chức năng:** Thêm một thành viên mới.

**Điều kiện:**

1. Repository đã load.
2. Member hợp lệ.
3. Điểm ban đầu bằng 0.
4. ID chưa tồn tại.
5. Số điện thoại chưa tồn tại.
6. Con trỏ chưa được Repository quản lý.

**Quy trình:**

1. Kiểm tra toàn bộ điều kiện.
2. Lấy ID và số điện thoại.
3. Thêm con trỏ vào `membersById`.
4. Thêm con trỏ vào `membersByPhone`.
5. Gọi `repo.add(member)`.
6. Trả về `true` nếu thành công.

Nếu thêm vào map thứ hai hoặc Repository thất bại, các chỉ mục đã thêm được hoàn tác.

**Giá trị trả về:**

- `true`: Thêm thành công.
- `false`: Không thỏa mãn điều kiện hoặc `repo.add()` trả về `false`.
- Ngoại lệ: Có lỗi hệ thống trong quá trình thao tác.

**Quản lý bộ nhớ:**

Khi thêm thành công, `Repository<Member>` nhận quyền sở hữu con trỏ và chịu trách nhiệm giải phóng đối tượng.

Nếu `add()` trả về `false`, đối tượng chưa được chuyển quyền sở hữu và bên gọi phải quản lý bộ nhớ của nó.

### 4.15. `update()`

```cpp
bool update(const Member& newValues);
```

**Chức năng:** Cập nhật họ tên và số điện thoại của thành viên.

**Điều kiện:**

1. Repository đã load.
2. Dữ liệu mới hợp lệ.
3. ID thành viên tồn tại.
4. Điểm không thay đổi.
5. Số điện thoại mới không thuộc thành viên khác.

**Quy trình:**

1. Tìm thành viên theo ID.
2. Kiểm tra điểm và số điện thoại mới.
3. Lưu số điện thoại cũ.
4. Nếu số điện thoại thay đổi, thêm khóa mới vào `membersByPhone`.
5. Cập nhật họ tên và số điện thoại của đối tượng.
6. Xóa khóa số điện thoại cũ khỏi map.
7. Trả về `true`.

Nếu điều kiện không hợp lệ, trả về `false`.

Nếu phát sinh ngoại lệ trong quá trình cập nhật, code hiện tại có xử lý hoàn tác chỉ mục số điện thoại mới. Việc bảo đảm hoàn tác đầy đủ trạng thái đối tượng trong mọi tình huống ngoại lệ cần được tiếp tục rà soát.

## 5. Quy tắc nghiệp vụ

### 5.1. Mã thành viên

- Mã có tiền tố `MEM`.
- Mã phải duy nhất.
- Không được thay đổi mã sau khi tạo.
- Repository từ chối mã trùng với thành viên đang được nạp. Chưa có cơ chế riêng để kiểm tra mã từng được cấp nhưng không còn trong file.

### 5.2. Số điện thoại

- Đúng định dạng quy định.
- Không được trùng giữa các thành viên.
- Được phép cập nhật nếu số mới chưa thuộc thành viên khác.
- Khi thay đổi phải cập nhật đồng bộ `membersByPhone`.

### 5.3. Điểm thành viên

- Không được âm.
- Thành viên mới có 0 điểm.
- `update()` không cho phép sửa điểm.
- Nghiệp vụ cộng/thu hồi điểm khi bán/hủy vé chưa triển khai ở B08; cần phối hợp với vé và hóa đơn khi các thành phần đó có mã.

### 5.4. Xóa thành viên

Phiên bản hiện tại chưa cung cấp `removeById()`.

Theo quy tắc nghiệp vụ, không được xóa thành viên đã có hóa đơn. Việc kiểm tra tham chiếu hóa đơn cần được thiết kế trước khi bổ sung chức năng xóa.

## 6. Xử lý lỗi

| Trường hợp | Cách xử lý |
|---|---|
| Load khi đã load | Ném `runtime_error` |
| Không đọc được file | Ném `runtime_error` |
| File sai định dạng | Ném `runtime_error` |
| Trùng ID hoặc phone khi load | Ném `runtime_error`, dọn dữ liệu nạp lỗi |
| Save khi chưa load | Ném `runtime_error` |
| Ghi file thất bại | Ném `runtime_error` |
| Add dữ liệu không hợp lệ | Trả về `false` |
| Add trùng ID hoặc phone | Trả về `false` |
| Update ID không tồn tại | Trả về `false` |
| Update trùng phone | Trả về `false` |
| Update thay đổi điểm | Trả về `false` |

`add()` và `update()` có thể ném ngoại lệ hệ thống khi thao tác với các cấu trúc dữ liệu.

## 7. Quản lý bộ nhớ và đồng bộ dữ liệu

`Repository<Member>` sở hữu các đối tượng Member đã thêm thành công hoặc đã nạp từ file.

Hai map chỉ lưu địa chỉ của các đối tượng đó.

Các nguyên tắc cần tuân thủ:

- Không tự `delete` con trỏ đã thuộc Repository.
- Không lưu một đối tượng nhiều lần trong Repository.
- Không để map chứa con trỏ không còn hợp lệ.
- Khi thêm thất bại, hoàn tác các chỉ mục đã thêm.
- Khi đổi số điện thoại, xóa khóa cũ sau khi cập nhật thành công.
- Không tự ý thay đổi ID hoặc số điện thoại của đối tượng thông qua con trỏ bên ngoài Repository, vì có thể làm chỉ mục không đồng bộ.

Việc cập nhật các thuộc tính có ảnh hưởng đến chỉ mục phải được thực hiện thông qua phương thức quản lý thích hợp.

## 8. Kiểm thử

**File:** `tests/test_member_repository.cpp`

**Kết quả biên dịch và chạy lại trên nhánh hiện tại ngày 2026-11-18:**

```text
===== SUMMARY =====
Passed: 63
Failed: 0
Total: 63
```

### Các nhóm kiểm thử

| Nhóm | Nội dung |
|---|---|
| Load | File rỗng, file hợp lệ, load hai lần |
| Truy xuất | `findById`, `findByPhone`, `getAt`, `count`, `isLoaded` |
| Add | Thành công, trùng ID, trùng phone, dữ liệu không hợp lệ |
| Update | Đổi họ tên, đổi phone, giữ phone cũ, trùng phone, thay đổi điểm |
| Save | Save và load lại, save trước load |
| File lỗi | Trùng ID, trùng phone, thiếu trường, điểm sai định dạng, điểm tràn số |
| Trạng thái | Kiểm tra dữ liệu và chỉ mục sau thao tác thất bại |

Kết quả PASS xác nhận các trường hợp đã chạy đều đạt. Chưa khẳng định đã bao phủ mọi tình huống lỗi cấp phát bộ nhớ, lỗi ghi file giữa chừng hoặc ngoại lệ trong setter.

### Lệnh biên dịch và chạy test

Sử dụng PowerShell tại thư mục gốc dự án:

```powershell
$src = Get-ChildItem src -Recurse -Filter *.cpp |
    Where-Object { $_.Name -ne "main.cpp" } |
    ForEach-Object { $_.FullName }

g++ -std=c++17 -Wall -Wextra -I . -I src $src tests/test_member_repository.cpp -o test_member_repository.exe

if ($LASTEXITCODE -eq 0) {
    .\test_member_repository.exe
}
```

## 9. Phạm vi trách nhiệm

### MemberRepository chịu trách nhiệm

- Đọc và ghi dữ liệu thành viên.
- Kiểm tra định dạng dữ liệu.
- Kiểm tra trùng mã và số điện thoại.
- Thêm, cập nhật và tìm kiếm thành viên.
- Duy trì tính nhất quán giữa Repository và các chỉ mục.

### MemberRepository không chịu trách nhiệm

- Kiểm tra quyền Admin hoặc Staff.
- Tính điểm từ số tiền thanh toán.
- Cộng hoặc thu hồi điểm khi bán/hủy vé.
- Thực hiện giao dịch lưu đồng thời vé, hóa đơn và điểm.
- Xây dựng giao diện quản lý thành viên.
- Tự động liên kết danh sách hóa đơn của thành viên.

Các nghiệp vụ này sẽ được xử lý tại `MemberService`, `BookingService` và các thành phần liên quan.

## 10. Kết luận

`MemberRepository` đã có các chức năng cơ bản để quản lý dữ liệu thành viên:

- Nạp và lưu dữ liệu TXT.
- Thêm thành viên mới.
- Cập nhật họ tên và số điện thoại.
- Tìm kiếm theo ID và số điện thoại.
- Kiểm tra dữ liệu hợp lệ và trùng lặp.
- Đồng bộ các chỉ mục tìm kiếm.

**Trạng thái:** Đã triển khai và kiểm thử riêng đạt **63 PASS, 0 FAIL**. Kết quả này chỉ xác nhận phạm vi Repository, không xác nhận `MemberService` hoặc toàn B08 đã hoàn thành.

**Bước tiếp theo:** Thiết kế `MemberService` để xử lý phân quyền và nghiệp vụ quản lý thành viên, sau đó triển khai và kiểm thử theo cùng quy trình.
