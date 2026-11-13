# `src/` — mã nguồn ứng dụng

`src/` tập hợp mã C++17 của dự án. Tài liệu này là mục lục theo thư mục; [cauTrucThuMuc.md](cauTrucThuMuc.md) đã có cây file chi tiết và được giữ nguyên. Phạm vi mô tả hiện trạng ở đây dừng tại **Bước 03**; các thư mục cho chức năng sau chỉ có sườn trách nhiệm, không được xem là đã lập trình.

| Thư mục/file | Tài liệu tương ứng | Trách nhiệm |
| --- | --- | --- |
| `src/gui/` | [gui.md](gui.md) | Qt Widgets nhận thao tác và hiển thị kết quả; chưa triển khai ở Bước 01–03. |
| `src/business/` | [business.md](business.md) | Các Service kiểm tra nghiệp vụ và gọi Repository; chưa triển khai ở Bước 01–03. |
| `src/data_access/` | [data_access.md](data_access.md) | Repository giữ Model trong RAM và đọc/ghi TXT; chưa thuộc phạm vi hoàn thành Bước 01–03. |
| `src/models/` | [models.md](models.md) | Tám Model và các quan hệ đã khai báo trong Bước 02. |
| `src/data_structures/` | [data_structures.md](data_structures.md) | `DoublyLinkedList.h` tự cài đặt. |
| `src/utils/` | [utils.md](utils.md) | `FileIO`, `DateTime`, `IdGenerator` của hạ tầng ban đầu. |
| `src/main.cpp` | Mục dưới | Điểm chạy thử hiện tại, chưa khởi tạo GUI Qt. |

## `src/main.cpp` đến hết Bước 03

File hiện chỉ có `main()` in thông báo và trả `0`, dùng kiểm tra mã C++ liên kết được. Sườn hiện tại:

```cpp
int main() {
    // In thông báo kiểm tra chương trình thử.
    return 0;
}
```

Về sau GUI gọi Business → Data Access → `data/*.txt`. Model, cấu trúc dữ liệu và utils là thành phần hỗ trợ, không thêm tầng nghiệp vụ. Không đặt `services/`, `repositories/` hoặc `dalc/` làm thư mục trung gian trong `src/`.
