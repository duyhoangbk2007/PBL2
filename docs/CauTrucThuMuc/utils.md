# Utils — Các tiện ích dùng chung

## 1. Giới thiệu

Thư mục `core/utils/` chứa các class hỗ trợ được sử dụng ở nhiều nơi trong hệ thống quản lý rạp chiếu phim.

Các class trong thư mục này không trực tiếp xử lý nghiệp vụ bán vé hay quản lý phim.

## 2. FileHandler

**File:** `FileHandler.h`, `FileHandler.cpp`

**Chức năng:** Hỗ trợ đọc và ghi dữ liệu dạng văn bản.

Các hàm:

- `readlines(path, lines)`: Đọc các dòng từ file vào `vector<string>`, trả về `bool` cho biết thành công hay thất bại.
- `writeLines(path, lines)`: Ghi danh sách dòng vào file, trả về `bool` cho biết kết quả.

**Trạng thái:** Đã cài đặt và kiểm thử các thao tác cơ bản.

## 3. TransactionManager

**File:** `TransactionManager.h`, `TransactionManager.cpp`

**Chức năng dự kiến:** Quản lý việc cập nhật nhiều file trong một giao dịch, có khả năng sao lưu và phục hồi khi xảy ra lỗi.

### 3.1. TransactionFile

Gồm hai thuộc tính:

- `path`: Đường dẫn file gốc cần cập nhật.
- `newLines`: Nội dung mới của file.

### 3.2. prepareFiles()

Chức năng hiện đã cài đặt:

1. Tạo thư mục `new/` và `backup/` trong thư mục giao dịch.
2. Duyệt các file cần cập nhật.
3. Ghi dữ liệu mới vào thư mục `new/`.
4. Nếu file gốc tồn tại, sao chép dữ liệu cũ sang `backup/`.
5. Phát sinh Exception khi ghi dữ liệu mới hoặc sao lưu thất bại.

### 3.3. Các hàm chưa hoàn thiện

- `writeManifest()`: Ghi trạng thái giao dịch.
- `replaceFiles()`: Thay thế dữ liệu gốc.
- `rollback()`: Khôi phục dữ liệu khi thất bại.
- `recover()`: Xử lý giao dịch chưa hoàn tất khi khởi động lại.
- `execute()`: Hiện chỉ gọi `prepareFiles()` để kiểm thử, chưa thực hiện giao dịch hoàn chỉnh.

## 4. Kiểm thử

- `test_FileHandler.cpp`: Đã kiểm thử các thao tác đọc, ghi cơ bản.
- `test_TransactionManager.cpp`: Đã kiểm thử việc chuẩn bị dữ liệu mới.
- `test_Backup.cpp`: Đã kiểm thử sao lưu file gốc và xử lý trường hợp backup đã tồn tại.

## 5. Công việc tiếp theo

- Hoàn thiện kiểm tra đường dẫn và tên file trùng nhau.
- Xây dựng manifest với các trạng thái giao dịch.
- Triển khai commit, rollback và recovery.
- Kiểm thử các trường hợp thất bại giữa giao dịch.

**Lưu ý:** TransactionManager hiện chưa bảo đảm tính nguyên tử hoặc an toàn khi chương trình bị tắt đột ngột. Không sử dụng phiên bản này cho dữ liệu nghiệp vụ thật.
