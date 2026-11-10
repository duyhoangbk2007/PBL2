# Tổng quan tiến độ PBL2 — Hoàng và Kiệt

Lộ trình dưới đây giữ **15 bước** theo [phân công mới](PhanCongCongViec.md). Bước 03 cũ về cấu trúc dữ liệu đã bỏ; Bước 03 mới là **Hạ tầng lưu trữ dữ liệu**. Chỉ `DoublyLinkedList<T>` bắt buộc tự cài. DLL được rà soát cùng Models ở Bước 02 và tích hợp/kiểm thử khi DALC dùng ở Bước 04; STL được dùng ở bước có chức năng cần đến. Không có bước riêng để tạo CTDL.

Kiến trúc áp dụng: **GUI → BLL → DAL → DB**. GUI ở `ui/`, BLL ở `core/bll/`, DAL gồm `core/dal/models/` và `core/dal/dalc/`, DB là `data/*.txt`. Model không tự truy cập TXT. Bảng dưới ghi **trạng thái thực tế**, không lấy tên file dự kiến làm bằng chứng hoàn thành. Checklist riêng nằm ở [Task Hoàng](tasks/Task_Hoang.md) và [Task Kiệt](tasks/Task_Kiet.md); [phân công](PhanCongCongViec.md) là nguồn chính. Chỉ đổi trạng thái chung sang hoàn thành khi hai người đã xong, tích hợp và kiểm thử đạt.

| Bước | Nội dung | Hoàng | Kiệt | Trạng thái chung | Nhật ký |
| --- | --- | --- | --- | --- | --- |
| 01 | Khởi tạo dự án | Thư mục, CMake, `main.cpp`, build C++17 | README, `.gitignore`, `AGENTS.md`, Git/PR, tài liệu | Đang thực hiện: có mã Model/tài liệu và Git; chưa có CMake hoặc `main.cpp`, chưa xác minh build ứng dụng. | [Bước 01](Buoc01_KhoiTaoDuAn.md) |
| 02 | Hoàn thiện Models và quan hệ con trỏ | `Movie`, `Room`, `Seat`, `Showtime`, DLL khi được dùng | `User`, `Member`, `Ticket`, `Invoice`, trạng thái/điểm | Đang thực hiện: có 8 cặp Model, các quan hệ hai chiều đã ghi trong nhật ký và một số liên kết một chiều; kiểm thử/tích hợp còn mở. Quyết định không dùng `= delete` là chủ động; vòng đời object và gỡ liên kết xem xét ở DAL/Repository. PR #5 đã đóng, chưa merge; PR #4 đang mở. | [Bước 02](Buoc02_XayDungModels.md) |
| 03 | Hạ tầng lưu trữ dữ liệu | `FileHandler`, `TransactionManager`, TXT, backup/rollback/phục hồi | `IdGenerator`, `DateTime` | **Đã kiểm thử chức năng cơ bản – sẵn sàng chuyển sang Bước 04; còn một số giới hạn cần kiểm tra bổ sung.** Bốn tiện ích đã có mã. Người dùng xác nhận đã chạy `test_all_utils.cpp` và không có FAILED; Codex chưa chạy lại. Chưa có bằng chứng riêng cho bản `-DTEST_ROLLBACK`, độ bền vững khi lỗi lưu trữ và tích hợp sau merge. | [Bước 03](Buoc03_HaTangLuuTruDuLieu.md) |
| 04 | Xây dựng DALC | Movie/Room/Seat/Showtime DALC | User/Member/Ticket/Invoice/Sequence/Booking DALC | Chưa bắt đầu; tích hợp DLL/STL khi cần. | Chưa tạo |
| 05 | Đăng nhập và phân quyền | Thử quyền Admin/Staff trong quản trị | `AuthService`, `UserManager` | Chưa bắt đầu. | Chưa tạo |
| 06 | Quản lý phim và thành viên | `MovieManager` | `MemberManager` | Chưa bắt đầu. | Chưa tạo |
| 07 | Quản lý phòng và ghế | `RoomManager`, `SeatManager` | Sơ đồ ghế hai chiều, thử vị trí/ghế đôi | Chưa bắt đầu. | Chưa tạo |
| 08 | Quản lý suất chiếu và giá vé | `ShowtimeManager`, trùng lịch/khoảng nghỉ | `PricingConfig`, giá/ưu đãi/điểm | Chưa bắt đầu. | Chưa tạo |
| 09 | Chọn ghế và Undo | Ghế trống/đã bán theo suất | `BookingService`, giỏ tạm, `std::stack` Undo | Chưa bắt đầu. | Chưa tạo |
| 10 | Thanh toán và lưu vé/hóa đơn | Thử dữ liệu và rollback/phục hồi nhiều file | Hoàn thiện Booking, tạo vé/hóa đơn, điểm và lưu | Chưa bắt đầu. | Chưa tạo |
| 11 | Hủy vé, lịch sử và báo cáo | `ReportService`, doanh thu/vé/top phim | Hủy, hoàn tiền, thu hồi điểm, lịch sử | Chưa bắt đầu. | Chưa tạo |
| 12 | Nền tảng Qt | `MainWindow`, `AdminDashboard` | `LoginView`, `StaffDashboard` | Chưa bắt đầu. | Chưa tạo |
| 13 | Giao diện các chức năng | View quản trị, báo cáo, `MovieTableModel` | View nhân viên/booking/lịch sử, `TicketTableModel` | Chưa bắt đầu. | Chưa tạo |
| 14 | Kiểm thử tích hợp toàn hệ thống | Quản trị, dữ liệu và lỗi GUI | Staff, bán/hủy, dữ liệu lỗi, phục hồi | Chưa bắt đầu; cần thử luồng GUI → BLL → DAL → DB. | Chưa tạo |
| 15 | Đóng gói và hoàn thiện | CMake, gói Windows, kiến trúc | Hướng dẫn, báo cáo, demo | Chưa bắt đầu. | Chưa tạo |

## Bằng chứng hiện có và lịch sử

- Cây làm việc hiện có tám cặp Model ở `core/dal/models/`, `core/data_structures/DoublyLinkedList.h`, bốn tiện ích Bước 03 trong `core/utils/` và `main.cpp` dùng để thử. Chưa có CMake, DALC, BLL, GUI hoặc file TXT nghiệp vụ trong `data/`. Không coi việc có cấu trúc mục tiêu trong tài liệu là đã triển khai.
- [Nhật ký Bước 01](Buoc01_KhoiTaoDuAn.md) và [Bước 02](Buoc02_XayDungModels.md) giữ các mốc lịch sử, kể cả nhận xét về cấu trúc cũ. Một số đoạn phản ánh cây làm việc tại thời điểm ghi, không phải hiện trạng của commit `9925283`.
- Bước 03 đã có bốn tiện ích trong `core/utils/` và bài test chung `tests/test_all_utils.cpp`. Kết quả không có FAILED là do người dùng xác nhận, không phải kết quả Codex tự chạy trong lần cập nhật này. `sequences.txt` không có dòng `#schema=1`; quyết định đã được ghi trong tài liệu định dạng dữ liệu. Những kiểm tra còn mở nằm trong [nhật ký Bước 03](Buoc03_HaTangLuuTruDuLieu.md).
