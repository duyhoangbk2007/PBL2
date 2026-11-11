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

## 2026-11-10 — Rà soát tiến độ bốn tiện ích trước Bước 04

**Trạng thái hiện tại:** Đã kiểm thử chức năng cơ bản – sẵn sàng chuyển sang Bước 04; còn một số giới hạn cần kiểm tra bổ sung. Mục ngày 2026-11-06 và 2026-11-09 phía trên là lịch sử tại thời điểm ghi; trạng thái mới không thay thế kết quả thử cũ.

### 1. Checklist hiện tại

- [x] Hoàng: `FileHandler` và `TransactionManager` đã có mã đọc/ghi TXT, chuẩn bị file mới, backup, manifest, execute, rollback và recover.
- [x] Kiệt: `IdGenerator` đã có mã đọc `sequences.txt`, cấp ID và trả nội dung bộ đếm mới; `DateTime` đã có `getCurrentDateTime()`.
- [x] Quyết định định dạng: `sequences.txt` chỉ gồm `entity|nextValue`, không có `#schema=1`; tài liệu định dạng đã cập nhật, mã `IdGenerator` hiện đọc đúng dạng này.
- [x] Người dùng xác nhận đã chạy `test_all_utils.cpp` bản thông thường và kết quả không có FAILED. Đây là kết quả **người dùng báo cáo**, không phải lần chạy kiểm thử của Codex khi cập nhật nhật ký.
- [ ] Chạy và ghi kết quả riêng bản biên dịch `-DTEST_ROLLBACK`; chưa có bằng chứng PASS cho nhánh giả lập lỗi này.
- [ ] Kiểm tra thêm các tình huống lỗi lưu trữ và phục hồi; bài thử chức năng cơ bản chưa chứng minh độ bền vững khi gián đoạn hoặc mất điện.
- [ ] Review/merge phần việc hai người và kiểm tra tích hợp sau merge.
- [ ] Hoàn thiện, kiểm thử phần parse/kiểm tra ngày giờ nếu vẫn thuộc API `DateTime` cần cho DALC; API hiện tại mới lấy thời gian hiện tại.

### 2. Bằng chứng kiểm thử và phạm vi

`tests/test_all_utils.cpp` hiện có trong cây làm việc, gọi cả bốn tiện ích. Bản thông thường thử đọc/ghi file, định dạng thời gian hiện tại, cấp ID từ file không có schema và một số dữ liệu ID sai, execute thành công, phục hồi PREPARED/COMMITTED với dữ liệu giả lập. Người dùng báo cáo đã chạy file này và không có FAILED; không có lệnh build hoặc log đầu ra được cung cấp để ghi kết quả chi tiết từng ca. Codex chỉ đọc mã test trong lần cập nhật tài liệu này.

Trong mã test, nhánh kiểm tra lỗi cố ý sau khi thay file đầu tiên nằm dưới `#ifdef TEST_ROLLBACK`. Bản chạy thông thường không thực hiện nhánh này. Kết quả rollback ghi ở mốc 2026-11-09 là kết quả kiểm thử trước đó trong nhật ký; không được dùng thay cho bằng chứng đã chạy riêng `test_all_utils.cpp` với `-DTEST_ROLLBACK`.

### 3. Giới hạn và vướng mắc còn lại

- `TransactionManager` sao chép đè từng file, chưa có xác nhận ghi bền vững bằng cơ chế đồng bộ xuống đĩa; mất điện giữa giao dịch chưa được kiểm chứng.
- Backup đặt theo tên file cuối đường dẫn, nên hai file trùng tên trong một giao dịch có thể xung đột.
- `recover()` hiện báo lỗi khi thư mục giao dịch thiếu manifest; lỗi ghi manifest và backup thiếu/hỏng vẫn cần thử riêng.
- Chưa có bằng chứng review/merge và kiểm tra lại sau merge. Bài test Utils chưa phải luồng lưu vé/hóa đơn/điểm thật.
- `DateTime` chưa có hàm parse/kiểm tra ngày giờ; `IdGenerator` trả `updatedSequences` nhưng chưa tự lưu bộ đếm, nên DALC cần phối hợp lưu cùng dữ liệu mới.

### 4. Công việc tiếp theo

- [ ] Chạy riêng bản `-DTEST_ROLLBACK` và lưu lệnh/kết quả; mở rộng thử lỗi manifest, backup, thư mục giao dịch dở và mất điện theo khả năng môi trường.
- [ ] Review API bốn tiện ích, tích hợp chính thức và chạy lại kiểm thử sau merge.
- [ ] Thống nhất nhu cầu parse/kiểm tra ngày giờ khi thiết kế DALC; chỉ cài các hàm thật sự cần, giữ `Validator` theo kế hoạch hoãn hiện tại.
- [ ] Bắt đầu Bước 04 – DALC theo phân công đã chốt, dùng `IdGenerator` và `TransactionManager` qua tầng DALC; chưa triển khai Bước 04 trong lần cập nhật này.

**Kết luận tại mốc trên:** Bước 03 đủ kết quả chức năng cơ bản để bắt đầu Bước 04. Việc kiểm tra độ bền vững, nhánh `-DTEST_ROLLBACK` và tích hợp chính thức vẫn là việc còn mở.

> **Cập nhật sau tái cấu trúc:** Nhật ký trên giữ nguyên những việc đã làm và kết quả kiểm thử tại thời điểm đó. `FileHandler` đã đổi thành `FileIO` ở `src/utils/`; Model và DLL đã chuyển vào `src/`, bản nháp `MovieDALC` thành `MovieRepository` ở `src/data_access/`. `TransactionManager` cũ nằm tại `tests/legacy/`, không thuộc luồng ứng dụng mục tiêu. Xem [Bước 04](Buoc04_XayDungDALC.md) và [định dạng dữ liệu](../TongQuan/data_format.md). Ghi tuần tự nhiều file không nguyên tử.
