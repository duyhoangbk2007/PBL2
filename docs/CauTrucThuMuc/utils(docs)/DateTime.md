# DateTime

**Mã nguồn:** `core/utils/DateTime.h`, `core/utils/DateTime.cpp`.

`DateTime` hiện chỉ cung cấp thời điểm hiện tại ở dạng chuỗi để các phần khác có thể ghi dấu thời gian. Lớp chưa có hàm parse, kiểm tra ngày hợp lệ, cộng/trừ thời gian hoặc so sánh hai thời điểm.

## `getCurrentDateTime()`

```cpp
static std::string getCurrentDateTime();
```

Hàm lấy thời gian hệ thống bằng `std::time(nullptr)`, chuyển sang **giờ địa phương của máy đang chạy** bằng `std::localtime`, rồi định dạng bằng `std::strftime` thành `YYYY-MM-DDTHH:MM:SS`. Ví dụ một kết quả có thể là `2026-11-10T14:30:05`; đây là ví dụ định dạng, không phải thời gian cố định mà hàm trả về.

Hàm không nhận tham số. Nó trả chuỗi đã định dạng, hoặc ném `std::runtime_error` nếu `localtime` trả con trỏ rỗng hay `strftime` không định dạng được. Trong mã hiện tại, lời gọi `time(nullptr)` không được kiểm tra lỗi riêng.

### Giới hạn cần lưu ý

- Chuỗi không chứa múi giờ hoặc độ lệch UTC. Hai máy đặt múi giờ khác nhau có thể tạo chuỗi khác nhau cho cùng một thời điểm.
- Hàm phụ thuộc đồng hồ và cấu hình giờ địa phương của hệ điều hành; nó không xác nhận đồng hồ máy chính xác.
- `std::localtime` dùng vùng nhớ tĩnh trên nhiều thư viện C, nên API hiện tại chưa được thiết kế để gọi đồng thời từ nhiều luồng.
- Các quy tắc như “hủy vé trước giờ chiếu” hoặc kiểm tra trùng lịch chưa được cài trong lớp này; BLL phải xử lý khi các hàm ngày giờ cần thiết được bổ sung.

Xem [quy ước dữ liệu thời gian](../../TongQuan/data_format.md) để thống nhất định dạng khi lưu TXT.
