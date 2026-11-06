# Nhật ký Bước 03 — Hạ tầng lưu trữ dữ liệu

## 2026-11-06 — Hoàng bắt đầu phần việc

**Trạng thái:** Đang thực hiện. Branch hiện tại: `feature/b03-hoang-storage`.

Theo phân công, Hoàng phụ trách `FileHandler`, `Validator`, đọc/ghi TXT và kiểm tra định dạng; Kiệt phụ trách `TransactionManager`, `IdGenerator`, `DateTime` và phục hồi giao dịch. Hai phần cần thống nhất API và schema TXT trước khi tích hợp.

**Hiện trạng đã kiểm tra:** `core/utils/FileHandler.h/.cpp` và `core/utils/Validator.h/.cpp` đã có trong cây làm việc nhưng chưa được Git theo dõi. `FileHandler` hiện cung cấp `readlines(path, lines)` và `writeLines(path, lines)`, trả về `bool`. `Validator.h` khai báo `isInteger`, `isBoolean`, `isDate`; `Validator.cpp` chưa có phần cài đặt. Chưa thấy bằng chứng biên dịch, chạy thử hoặc kiểm thử cho Bước 03. Các file này chưa được ghi nhận là hoàn thành.

**Việc tiếp theo của Hoàng:**

- [ ] Chốt hợp đồng `FileHandler` với phần giao dịch của Kiệt: cách báo lỗi I/O, trạng thái dữ liệu khi đọc thất bại và cách ghi an toàn.
- [ ] Hoàn thiện `Validator` theo định dạng và schema trong `docs/TongQuan/data_format.md`.
- [ ] Thử đọc/ghi file hợp lệ, file sai định dạng, thiếu file và lỗi ghi; ghi lệnh và kết quả thực tế vào nhật ký.
- [ ] Biên dịch, chạy kiểm thử phù hợp rồi review/tích hợp với phần Kiệt.

**Điều kiện nghiệm thu:** Chỉ đánh dấu phần Hoàng hoàn thành khi các tiện ích được cài đặt và có kết quả kiểm thử. Bước 03 chung chỉ hoàn thành sau khi phần Kiệt và tích hợp đều đạt.
