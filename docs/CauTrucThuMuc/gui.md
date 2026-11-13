# `src/gui/` — giao diện Qt Widgets

**Đến hết Bước 03:** thư mục đã được dành chỗ trong cây `src/`, nhưng **chưa có mã GUI Qt**. Giao diện dự kiến xây dựng ở Bước 13 sau khi các chức năng C++ được triển khai và kiểm thử.

## Ý nghĩa và ranh giới

GUI nhận dữ liệu nhập, gọi Service và hiển thị kết quả/lỗi. GUI không mở file TXT, không gọi Repository trực tiếp và không tự quyết định quyền, giá hoặc điều kiện bán/hủy. Tên class màn hình theo hậu tố `View`; `MainWindow` là cửa sổ chính.

## Sườn tổ chức trước khi lập trình

```text
src/gui/
├── MainWindow.h/.cpp          cửa sổ chính, điều hướng
├── LoginView.h/.cpp           màn hình đăng nhập
├── *ManagementView.h/.cpp     màn hình quản lý theo chức năng
├── BookingView.h/.cpp         màn hình bán vé
└── ReportView.h/.cpp          màn hình thống kê
```

Đây là **sơ đồ tên file dự kiến**, chưa có file mã tương ứng. Nếu dùng Qt Designer, chỉ tạo `.ui` cho màn hình thật sự cần. Khi viết một View, sườn luồng là: lấy thao tác → gọi Service tương ứng → hiển thị dữ liệu hoặc lỗi trả về. Không tự tạo màn hình chỉ để khớp cây tài liệu.

Xem [business.md](business.md) cho tầng được GUI gọi và [lộ trình](../development/PhanCongCongViec.md) cho phân công GUI Bước 13.
