# Nhật ký Bước 03 — Hạ tầng lưu trữ dữ liệu

## 2026-11-06 — Hoàng bắt đầu phần việc

**Trạng thái:** Đang thực hiện. Branch hiện tại: `feature/b03-hoang-storage`.

Theo phân công, Hoàng phụ trách `FileHandler`, `TransactionManager`, đọc/ghi TXT, giao dịch, backup và rollback/phục hồi dữ liệu; Kiệt phụ trách `IdGenerator` và `DateTime`. Hoàng và Kiệt cần thống nhất API trước khi tích hợp. Giữ nguyên code `FileHandler.h/.cpp` hiện tại; chỉ tích hợp với `TransactionManager` sau khi thống nhất API.

**Hiện trạng đã kiểm tra:** `core/utils/FileHandler.h/.cpp` đã có trong cây làm việc. `FileHandler` hiện cung cấp `readlines(path, lines)` và `writeLines(path, lines)`, trả về `bool`. `Validator.h` chỉ khai báo `isInteger`, `isBoolean`, `isDate`; `Validator.cpp` chưa có phần cài đặt, nên hai file đã được xóa khỏi branch hiện tại. Chưa thấy bằng chứng biên dịch, chạy thử hoặc kiểm thử cho Bước 03. `Validator` được hoãn sang bước DAL/Repository: khi đọc, chuyển đổi và kiểm tra dữ liệu TXT, chỉ xác định và cài những hàm kiểm tra thực sự cần dùng.

**Việc tiếp theo của Hoàng:**

- [ ] Chốt API giữa `FileHandler` và `TransactionManager`: cách báo lỗi I/O, trạng thái dữ liệu khi đọc thất bại và cách ghi an toàn; sau đó mới tích hợp.
- [ ] Xây dựng `TransactionManager` cho giao dịch nhiều TXT, backup và rollback/phục hồi dữ liệu.
- [ ] Thử đọc/ghi file hợp lệ, thiếu file, lỗi ghi giữa chừng và phục hồi; ghi lệnh và kết quả thực tế vào nhật ký.
- [ ] Biên dịch, chạy kiểm thử phù hợp rồi review/tích hợp với `IdGenerator` và `DateTime` của Kiệt.

**Việc tiếp theo của Kiệt:** Cài `IdGenerator` và `DateTime` ; thử mã và ngày giờ hợp lệ/không hợp lệ, ghi kết quả kiểm thử rồi phối hợp tích hợp với phần Hoàng.

**Điều kiện nghiệm thu:** Chỉ đánh dấu phần Hoàng hoàn thành khi `FileHandler` và `TransactionManager` được kiểm thử theo phạm vi trên. Bước 03 chung chỉ hoàn thành sau khi `IdGenerator`, `DateTime` của Kiệt và tích hợp đều đạt; không yêu cầu `Validator` ở bước này.

## 2026-11-09 — Hoàn thiện các chức năng chính của TransactionManager

**Trạng thái:** Phần Hoàng đã triển khai và kiểm thử các luồng chính; chuẩn bị tích hợp với phần Kiệt.

### 1. Công việc đã thực hiện

- [x] Xây dựng `FileHandler` với chức năng đọc và ghi file TXT.
- [x] Xây dựng `TransactionManager` để quản lý giao dịch nhiều file.
- [x] Cài `prepareFiles()` tạo file mới và backup dữ liệu cũ.
- [x] Cài `writeManifest()` ghi trạng thái PREPARED và COMMITTED.
- [x] Cài `replaceFiles()` thay thế file gốc bằng dữ liệu mới.
- [x] Cài `rollback()` khôi phục dữ liệu khi giao dịch thất bại.
- [x] Cài `execute()` điều phối toàn bộ giao dịch.
- [x] Cài `recover()` phục hồi các giao dịch khi khởi động lại.
- [x] Bổ sung dọn thư mục giao dịch sau commit thành công.
- [x] Bổ sung dọn thư mục sau rollback thành công.
- [x] Bổ sung dọn thư mục sau khi recovery xử lý xong.
- [x] Kiểm thử các luồng execute, rollback và recovery.

### 2. Kết quả kiểm thử

Các bài kiểm thử được biên dịch bằng g++ với chuẩn C++17.

| Bài kiểm thử | Kết quả |
| --- | --- |
| Execute cập nhật dữ liệu nhiều file | PASS |
| Execute giả lập lỗi và rollback | PASS |
| Recovery với manifest PREPARED | PASS |
| Recovery với manifest COMMITTED | PASS |
| Dọn thư mục giao dịch sau khi xử lý | PASS trong các trường hợp đã thử |

Các bài kiểm thử trên sử dụng dữ liệu TXT giả lập, chưa phải luồng bán vé hoàn chỉnh.

### 3. Vướng mắc và giới hạn

- Các giao dịch lỗi trước khi ghi PREPARED đã có xử lý dọn dữ liệu tạm trong `execute()`, nhưng chưa được kiểm thử độc lập.
- Chưa kiểm chứng đầy đủ trường hợp ghi manifest lỗi hoặc backup bị thiếu/hỏng.
- Chưa bảo đảm tính bền vững tuyệt đối khi mất điện đột ngột.
- Cơ chế backup hiện sử dụng tên file; cần tránh truyền nhiều đường dẫn có cùng tên file trong một giao dịch.
- Các file test có thể được dọn khỏi cây dự án theo quyết định của nhóm; kết quả kiểm thử được ghi lại trong nhật ký.
- `Validator` tiếp tục được hoãn sang bước DAL/Repository.

### 4. Phần tiếp theo

- [ ] Kiểm tra lại các thay đổi local và loại bỏ dữ liệu test không cần thiết.
- [ ] Commit và push phần Hoàng lên `feature/b03-hoang-storage`.
- [ ] Tạo Pull Request và tích hợp phần Hoàng theo quy trình Git của nhóm.
- [ ] Tích hợp `IdGenerator` và `DateTime` của Kiệt.
- [ ] Biên dịch và kiểm tra tương thích sau khi tích hợp.
- [ ] Cập nhật tiến độ chung Bước 03.

**Kết luận:** Các chức năng lưu trữ chính của Hoàng đã hoạt động trong các trường hợp kiểm thử cơ bản. Bước 03 chung chưa hoàn thành vì còn chờ phần Kiệt và kiểm tra tích hợp.
