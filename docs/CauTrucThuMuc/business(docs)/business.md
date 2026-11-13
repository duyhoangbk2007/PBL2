# `src/business/` — xử lý nghiệp vụ

**Đến hết Bước 03:** thư mục có trong cây mục tiêu nhưng **chưa có class Service**. Từ Bước 04, mỗi chức năng được làm cùng Repository cần thiết và kiểm thử C++; không đợi hoàn tất toàn bộ tầng truy xuất dữ liệu. Tài liệu này chỉ ghi sườn trách nhiệm trước khi lập trình.

## Ý nghĩa và ranh giới

Class hậu tố `Service` nhận yêu cầu từ GUI hoặc chương trình thử, kiểm tra đầu vào, quyền và quy tắc nghiệp vụ, rồi gọi Repository. Service không đọc/ghi TXT trực tiếp. Model giữ dữ liệu; `FileIO` chỉ làm I/O dòng. Các quy tắc bán vé, ưu đãi, hủy vé và điểm nằm trong [business_rules.md](../TongQuan/business_rules.md).

## Sườn luồng của một Service

```text
Yêu cầu chức năng
  → kiểm tra dữ liệu và quyền
  → dùng Repository để tra/cập nhật Model
  → trả kết quả hoặc lỗi cho bên gọi
```

Đây là **sườn xử lý**, không phải chữ ký API đã có. Chỉ tạo Service khi chức năng đến lượt triển khai; ví dụ `MovieService` cho phim, `AuthService` cho đăng nhập. Chưa mặc định tạo một Service cho mỗi Model hoặc thêm lớp trừu tượng không cần thiết.

Xem [data_access.md](data_access.md) cho trách nhiệm Repository và [phân công](../development/PhanCongCongViec.md) cho thứ tự chức năng.
