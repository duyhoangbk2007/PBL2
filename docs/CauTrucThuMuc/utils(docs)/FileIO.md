# FileIO — tiện ích đọc/ghi TXT mục tiêu

Tài liệu này mô tả `FileIO` hiện có tại `src/utils/`. Tên `FileHandler` chỉ dùng khi nhắc tới lịch sử đổi tên mã.

`FileIO.h/.cpp` đã đặt trong `src/utils/`. Class này chỉ đọc các dòng từ file TXT và ghi toàn bộ các dòng xuống file TXT. Hai thao tác hiện có là `readLines(path, lines)` và `writeLines(path, lines)`, đều trả `bool`. `FileIO` không hiểu Model, định dạng bản ghi, quyền hay nghiệp vụ.

Luồng đọc: Repository gọi `FileIO` → nhận dòng TXT → kiểm tra định dạng, khóa, tham chiếu → tạo Model trong RAM. Luồng ghi: Repository chuyển Model thành dòng TXT → gọi `FileIO` ghi toàn bộ file khi nghiệp vụ yêu cầu → kiểm tra kết quả và báo lỗi. Không cần append từng bản ghi hoặc cơ chế tự lưu phức tạp.

Nếu ghi lỗi giữa chừng, file đích có thể chỉ có một phần dữ liệu. Ghi nhiều file tuần tự không nguyên tử; lỗi hoặc gián đoạn có thể khiến các file không nhất quán. Không âm thầm bỏ qua dữ liệu lỗi hay báo thành công khi một lần ghi thất bại. Thiết kế mục tiêu không dùng `TransactionManager`.

## Hiện trạng mã nguồn

`src/utils/FileIO.h/.cpp` đã được chuyển và đổi tên từ `core/utils/FileHandler.h/.cpp`; phương thức đọc đã đổi từ `readlines` thành `readLines`. Code `MovieRepository` cũ từng dùng API mới nhưng chưa được nghiệm thu; hai file Repository hiện đã bị xóa khỏi cây làm việc và sẽ được viết lại trong [Bước 04](../../development/Buoc04_QuanLyPhim.md).
