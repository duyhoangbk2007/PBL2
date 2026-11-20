# PricingRepository — Data Access bảng giá (Bước 09)

## 1. Mục đích và cấu trúc

`PricingRepository` ở `src/data_access/PricingRepository.h/.cpp` quản lý **một** đối tượng `Pricing`, không có ID và không dùng `Repository<T>`. Lớp giữ `string filePath`, `Pricing pricing`, `bool loaded`; đọc/ghi TXT bằng `FileIO`. Repository không kiểm tra quyền, không tính giảm giá hoặc điểm. Đường dẫn file do caller truyền vào constructor; ứng dụng dự kiến dùng `data/pricing.txt`.

## 2. Định dạng TXT

File có đúng năm dòng `KEY|VALUE`, không có schema hoặc tiêu đề:

```text
STANDARD|70000
VIP|100000
COUPLE|160000
STUDENT_DISCOUNT|10
MEMBER_DISCOUNT|5
```

Ba giá dùng VND; hai tỷ lệ là phần trăm (`10` nghĩa là 10%). `load()` chấp nhận mọi thứ tự dòng nếu đủ đúng năm khóa, mỗi khóa xuất hiện một lần. Mỗi dòng phải có đúng một dấu `|`, khóa và giá trị không rỗng. `Validator::isValidNumber()` yêu cầu chuỗi số thập phân có chữ số, có thể có một dấu `.` và dấu `-` đầu chuỗi; `stod()` chuyển giá trị, sau đó code loại số không hữu hạn. Giá ghế phải `> 0`; tỷ lệ giảm phải trong `[0,100]` theo `Pricing::isValid()`. `save()` và `update()` ghi đúng thứ tự ví dụ trên bằng `std::to_string()`, nên giá trị có thể hiện sáu chữ số thập phân.

## 3. API hiện có

| Hàm | Hành vi |
| --- | --- |
| `PricingRepository(const string& filePath)` | Lưu đường dẫn, khởi tạo `loaded = false`; chưa đọc hoặc tạo file. |
| `load()` | Đọc đủ năm dòng, kiểm tra khóa/dữ liệu rồi mới gán `pricing` và đặt `loaded = true`; trả `false` nếu không đọc được hoặc dữ liệu sai, giữ nguyên cấu hình và trạng thái nạp trước đó. |
| `save()` | Trả `false` nếu chưa load hoặc `pricing.isValid()` sai; nếu hợp lệ, ghi toàn bộ năm dòng và trả kết quả từ `FileIO::writeLines()`. |
| `update(const Pricing& newPricing)` | Trả `false` nếu chưa load hoặc giá trị không hợp lệ; ghi toàn bộ cấu hình mới xuống file trước, chỉ gán vào RAM nếu ghi trả `true`. |
| `getPricing() const` | Trả `const Pricing*` trỏ vào đối tượng nội bộ khi đã load; chưa load trả `nullptr`. |
| `isLoaded() const` | Trả trạng thái `loaded`. |

`update()` đã ghi file khi thành công; không cần gọi `save()` để ghi lại cùng thay đổi. `save()` dùng cho cấu hình đang nạp. Repository không tự tạo file mặc định khi thiếu: `load()` trả `false`. Con trỏ từ `getPricing()` là con trỏ mượn, không được `delete` hoặc giữ lâu qua cập nhật.

## 4. Lỗi và giới hạn cần biết

- Khóa lạ/trùng/thiếu, số dòng khác 5, dòng sai trường, số sai định dạng, không hữu hạn hoặc ngoài giới hạn đều làm `load()` trả `false`. Code hiện tại **không ném lỗi kèm file/dòng** cho các trường hợp này.
- `load()` chỉ thay `pricing` và `loaded` sau khi đọc và kiểm tra toàn bộ file thành công. Nếu reload lỗi sau lần nạp hợp lệ, `isLoaded()` vẫn là `true` và `getPricing()` vẫn trả cấu hình cũ; nếu chưa từng nạp thành công, trạng thái vẫn chưa load.
- `save()` và `update()` trả `false` khi ghi thất bại. `update()` giữ nguyên cấu hình trong RAM nếu `FileIO::writeLines()` trả `false`, nhưng code không dùng file tạm/thay thế nguyên tử và không bảo đảm file đích còn nguyên nếu ghi dở.
- `Pricing::isValid()` kiểm tra `std::isfinite()` cho cả năm thuộc tính. Vì `save()` và `update()` đều gọi hàm này, đường cập nhật trực tiếp qua Repository cũng từ chối `NaN`/vô cực; `load()` còn kiểm tra số hữu hạn sau khi parse.

## 5. Kiểm thử

`tests/test_pricing.cpp` kiểm tra Model, Repository và Service trong cùng một chương trình. Lần biên dịch/chạy C++17 ngày 2026-11-20 đạt **69 PASS, 0 FAIL**, thoát mã 0. Phần Repository đã thử nạp file chuẩn, cập nhật/lưu/nạp lại, thiếu khóa, số sai, khóa trùng và giữ cấu hình cũ khi reload lỗi; kết quả 69 là **toàn bài test**, không phải số ca riêng của Repository. Test chưa có ca trực tiếp cho `NaN`/vô cực hoặc lỗi ghi file dở và chưa chứng minh cơ chế báo file/dòng.
