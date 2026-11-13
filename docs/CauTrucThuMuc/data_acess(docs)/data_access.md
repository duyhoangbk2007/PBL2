# `src/data_access/` — Tầng truy xuất dữ liệu

## 1. Tổng quan

Thư mục `src/data_access/` chứa các lớp Repository, chịu trách nhiệm quản lý dữ liệu Model trong RAM và thực hiện đọc/ghi dữ liệu từ file TXT.

Trong dự án PBL2, dữ liệu được lưu bằng file TXT thay vì cơ sở dữ liệu SQL.

Repository đóng vai trò trung gian giữa dữ liệu trong file và các đối tượng Model mà chương trình sử dụng.

Luồng xử lý cơ bản:

```text
File TXT
   |
   | FileIO::readLines()
   v
Repository<T>
   |
   | parse()
   v
Danh sách đối tượng Model trong RAM
   |
   | Service sử dụng dữ liệu
   v
Các chức năng nghiệp vụ
```

Khi lưu dữ liệu, quá trình diễn ra theo chiều ngược lại:

```text
Danh sách Model trong RAM
   |
   | serialize()
   v
Danh sách các dòng string
   |
   | FileIO::writeLines()
   v
File TXT
```

### Phân chia trách nhiệm

- **Model:** Chứa thông tin và các phương thức của đối tượng, ví dụ `Movie`, `Room`, `Showtime`.
- **Repository:** Quản lý danh sách đối tượng trong RAM, tìm kiếm, thêm và đọc/ghi file.
- **FileIO:** Thực hiện thao tác đọc/ghi file TXT.
- **Service:** Xử lý nghiệp vụ như kiểm tra quyền, điều kiện bán vé, hủy vé và các ràng buộc giữa nhiều đối tượng.

Repository không thay thế Service.
