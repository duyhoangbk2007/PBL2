# Nhật ký Bước 03 — Hạ tầng lưu trữ dữ liệu

## 2026-11-06 — Hoàng bắt đầu phần việc

**Trạng thái:** Đang thực hiện. Branch hiện tại: `feature/b03-hoang-storage`.

Theo phân công, Hoàng phụ trách `FileHandler`, `TransactionManager`, đọc/ghi TXT, giao dịch, backup và rollback/phục hồi dữ liệu; Kiệt phụ trách `IdGenerator` và `DateTime`. Hoàng và Kiệt cần thống nhất API và schema TXT trước khi tích hợp. Giữ nguyên code `FileHandler.h/.cpp` hiện tại; chỉ tích hợp với `TransactionManager` sau khi thống nhất API.

**Hiện trạng đã kiểm tra:** `core/utils/FileHandler.h/.cpp` đã có trong cây làm việc. `FileHandler` hiện cung cấp `readlines(path, lines)` và `writeLines(path, lines)`, trả về `bool`. `Validator.h` chỉ khai báo `isInteger`, `isBoolean`, `isDate`; `Validator.cpp` chưa có phần cài đặt, nên hai file đã được xóa khỏi branch hiện tại. Chưa thấy bằng chứng biên dịch, chạy thử hoặc kiểm thử cho Bước 03. `Validator` được hoãn sang bước DAL/Repository: khi đọc, chuyển đổi và kiểm tra dữ liệu TXT, chỉ xác định và cài những hàm kiểm tra thực sự cần dùng.

**Việc tiếp theo của Hoàng:**

- [ ] Chốt API giữa `FileHandler` và `TransactionManager`: cách báo lỗi I/O, trạng thái dữ liệu khi đọc thất bại và cách ghi an toàn; sau đó mới tích hợp.
- [ ] Xây dựng `TransactionManager` cho giao dịch nhiều TXT, backup và rollback/phục hồi dữ liệu.
- [ ] Thử đọc/ghi file hợp lệ, thiếu file, lỗi ghi giữa chừng và phục hồi; ghi lệnh và kết quả thực tế vào nhật ký.
- [ ] Biên dịch, chạy kiểm thử phù hợp rồi review/tích hợp với `IdGenerator` và `DateTime` của Kiệt.

**Việc tiếp theo của Kiệt:** Cài `IdGenerator` và `DateTime` theo schema TXT; thử mã và ngày giờ hợp lệ/không hợp lệ, ghi kết quả kiểm thử rồi phối hợp tích hợp với phần Hoàng.

**Điều kiện nghiệm thu:** Chỉ đánh dấu phần Hoàng hoàn thành khi `FileHandler` và `TransactionManager` được kiểm thử theo phạm vi trên. Bước 03 chung chỉ hoàn thành sau khi `IdGenerator`, `DateTime` của Kiệt và tích hợp đều đạt; không yêu cầu `Validator` ở bước này.
