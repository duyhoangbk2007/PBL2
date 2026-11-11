# `src/data_access/` — truy xuất dữ liệu TXT

**Phạm vi Bước 01–03:** chưa có Repository được nghiệm thu như một chức năng. Hai file `MovieRepository.h/.cpp` từng được chuyển từ `MovieDALC` nhưng đã bị xóa khỏi cây làm việc; code cũ chưa được nghiệm thu và Hoàng sẽ viết lại từ đầu ở Bước 04. Tài liệu này chỉ chuẩn bị ranh giới thư mục và sườn chung.

## Ý nghĩa và ranh giới

Repository quản lý tập Model trong RAM, gọi `FileIO` để đọc dòng TXT, kiểm tra định dạng/khóa/tham chiếu, chuyển dòng thành Model và làm chiều ngược lại khi lưu. Quyền, giá, ưu đãi và điều kiện bán/hủy thuộc Service. Không dùng thư mục con `repositories/` hoặc `dalc/`.

## Sườn luồng đọc/ghi

```text
Đọc: Repository → FileIO.readLines → kiểm tra từng dòng → Model trong RAM
Ghi: Model trong RAM → Repository tạo các dòng → FileIO.writeLines → kiểm tra kết quả
```

Mọi file TXT, kể cả `sequences.txt`, bắt đầu ngay bằng bản ghi đầu tiên, không có schema hoặc tiêu đề. Khi dữ liệu lỗi, báo file/dòng/nguyên nhân; không âm thầm bỏ bản ghi rồi ghi đè. Việc ghi nhiều file tuần tự không nguyên tử và có thể gây lệch dữ liệu nếu bị gián đoạn.

Đây là **sườn thiết kế**, chưa cam kết chữ ký Repository hoặc tạo thêm class mới. Xem [utils.md](utils.md) cho API `FileIO` hiện có, [models.md](models.md) cho đối tượng dữ liệu, [data_format.md](../TongQuan/data_format.md) cho hợp đồng TXT và [nhật ký Bước 04](../development/Buoc04_QuanLyPhim.md) cho kế hoạch viết lại chức năng phim.
