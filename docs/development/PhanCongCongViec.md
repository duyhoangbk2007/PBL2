# Phân công phát triển PBL2 — Hoàng và Kiệt

## Nguyên tắc chung

Dự án giữ **15 bước**. Nhóm xác nhận Bước 01–03 đã hoàn thành; [nhật ký Bước 01](Buoc01_KhoiTaoDuAn.md), [Bước 02](Buoc02_XayDungModels.md) và [Bước 03](Buoc03_HaTangLuuTruDuLieu.md) giữ nguyên lịch sử, bằng chứng và giới hạn kỹ thuật tại thời điểm thực hiện. Từ Bước 04, làm **theo chức năng hoàn chỉnh**: xây dựng Repository → Service → kiểm thử C++ → cập nhật tài liệu. Không chờ hoàn tất mọi Repository mới bắt đầu Service. GUI Qt được triển khai tập trung ở Bước 13.

Kiến trúc mục tiêu: **GUI (`src/gui/`) → Business (`src/business/`) → Data Access (`src/data_access/`) → DB (`data/*.txt`)**. `src/models/`, `src/data_structures/`, `src/utils/` hỗ trợ các tầng; `FileIO` chỉ đọc/ghi dòng TXT. Chỉ `DoublyLinkedList<T>` bắt buộc tự cài; cấu trúc khác dùng STL khi phù hợp. Các class ghi trong kế hoạch là **dự kiến**, trừ mã thực tế được nêu ở [tiến độ](TongQuanTienDo.md).

## Lộ trình và người phụ trách

| Bước | Nội dung | Phụ trách | Kết quả cần có |
| --- | --- | --- | --- |
| 01 | Khởi tạo dự án | Hoàng và Kiệt — đã hoàn thành | Giữ kết quả và nhật ký lịch sử. |
| 02 | Xây dựng Models và quan hệ Model | Hoàng và Kiệt — đã hoàn thành | Giữ tám Model và nhật ký lịch sử. |
| 03 | Xây dựng tiện ích lưu trữ dữ liệu ban đầu | Hoàng và Kiệt — đã hoàn thành | Giữ kết quả lịch sử; thiết kế hiện hành dùng `FileIO`, `DateTime`, `IdGenerator`. |
| 04 | Quản lý phim | Hoàng | `MovieRepository`, `MovieService`, CRUD/tìm kiếm/trạng thái, kiểm thử C++. |
| 05 | Đăng nhập và phân quyền | Kiệt | `UserRepository`, `AuthService`, kiểm tra tài khoản hoạt động và quyền Admin/Staff, kiểm thử. |
| 06 | Quản lý phòng và ghế | Hoàng | `RoomRepository`, `SeatRepository`, `RoomService`, sơ đồ/loại ghế, trùng vị trí, kiểm thử. |
| 07 | Quản lý nhân viên | Kiệt | `StaffService` dùng lại `UserRepository`, thêm/sửa/tìm/vô hiệu hóa Staff, kiểm tra quyền Admin, kiểm thử. |
| 08 | Quản lý suất chiếu | Hoàng | `ShowtimeRepository`, `ShowtimeService`, quan hệ phim–phòng–suất, trùng lịch, kiểm thử. |
| 09 | Quản lý thành viên | Kiệt | `MemberRepository`, `MemberService`, thông tin liên hệ và số điện thoại duy nhất, kiểm thử. |
| 10 | Giá vé, ưu đãi và tích điểm | Kiệt chính, Hoàng hỗ trợ | Xử lý giá theo cấu hình, ưu đãi học sinh/thành viên, tiền và điểm; Hoàng kiểm giá theo loại ghế; kiểm thử. |
| 11 | Bán vé tại quầy | Kiệt chính, Hoàng hỗ trợ tích hợp | `TicketRepository`, `InvoiceRepository`, `BookingService`, chọn/Undo ghế, hóa đơn nhiều vé, điểm và lưu TXT; kiểm thử. |
| 12 | Hủy vé, lịch sử hóa đơn và thống kê | Cả hai, chia theo nghiệp vụ | Kiệt xử lý hủy/hoàn/thu hồi điểm/lịch sử; Hoàng làm `ReportService` và thống kê; kiểm thử. |
| 13 | Xây dựng giao diện Qt | Cả hai | View của từng nhóm chức năng, `MainWindow`, dashboard, `SeatMapView`, CMake/Qt. |
| 14 | Kiểm thử tích hợp và sửa lỗi | Cả hai | Thử luồng GUI → Business → Data Access → TXT, quyền, dữ liệu và lỗi ghi; sửa lỗi được xác nhận. |
| 15 | Đóng gói, hoàn thiện tài liệu và bàn giao | Cả hai | Đóng gói, hướng dẫn, báo cáo, demo và bàn giao. |

Các bước 04–12 bao gồm kiểm thử chức năng C++ và cập nhật tài liệu ngay trong bước; chúng không chờ GUI Qt. Không đánh dấu bước hoàn thành chỉ vì Model hoặc bản nháp Repository đã tồn tại. [Task Hoàng](tasks/Task_Hoang.md), [Task Kiệt](tasks/Task_Kiet.md) ghi việc cá nhân; [Tổng quan tiến độ](TongQuanTienDo.md) ghi trạng thái chung.

## Phân công cụ thể và phụ thuộc

**Hoàng:** Bước 04 phim; Bước 06 phòng/ghế; Bước 08 suất chiếu; Bước 12 thống kê với `ReportService`; Bước 13 các màn hình `MovieManagementView`, `RoomManagementView`, `ShowtimeManagementView`, `ReportView`. Hoàng hỗ trợ Kiệt kiểm giá theo loại ghế ở Bước 10 và tích hợp phim/phòng/ghế/suất ở Bước 11.

**Kiệt:** Bước 05 đăng nhập/quyền; Bước 07 quản lý nhân viên bằng `StaffService` và `UserRepository` có sẵn từ Bước 05; Bước 09 thành viên; chủ trì Bước 10–11 giá và bán vé; Bước 12 hủy vé/lịch sử; Bước 13 các màn hình `LoginView`, `StaffManagementView`, `MemberManagementView`, `BookingView`, `InvoiceHistoryView`. Chỉ tạo `PricingService` nếu có lý do rõ; không mặc định tạo `StaffRepository`.

**Cùng làm:** thống nhất API, `MainWindow`, dashboard Admin/Staff, `SeatMapView`, CMake/Qt, kiểm thử tích hợp, đóng gói và tài liệu. Mỗi file mã nên có một người phụ trách chính; thống nhất trước khi sửa chung.

| Đợt song song | Hoàng | Kiệt |
| --- | --- | --- |
| 1 | Bước 04 — phim | Bước 05 — đăng nhập |
| 2 | Bước 06 — phòng/ghế | Bước 07 — nhân viên |
| 3 | Bước 08 — suất chiếu | Bước 09 — thành viên |
| 4 | Hỗ trợ tích hợp | Bước 10–11 — giá và bán vé |
| 5 | Bước 12 — thống kê | Bước 12 — hủy vé và lịch sử |
| 6 | Bước 13–15 — GUI, kiểm thử, hoàn thiện | Bước 13–15 — GUI, kiểm thử, hoàn thiện |

Đợt là cách làm song song, **không phải bước mới**. Suất chiếu cần dữ liệu phim/phòng; bán vé cần phim/phòng/ghế/suất; thống kê cần vé/hóa đơn; quyền Admin/Staff cần được kiểm tra trong Service liên quan. Hai người có thể tiến hành phần độc lập đồng thời sau khi chốt API dùng chung.

## Theo dõi và nghiệm thu

Giữ tên [nhật ký Bước 04](Buoc04_XayDungDALC.md) để các liên kết cũ hoạt động; tên file phản ánh lịch sử DALC, còn nội dung mục tiêu hiện là **quản lý phim**. Không tạo sẵn nhật ký trống cho Bước 05–15. Bước chỉ được nghiệm thu khi phần phụ trách có mã, kiểm thử và tài liệu tương ứng; không suy ra kết quả từ tên file dự kiến. Ghi nhiều file TXT tuần tự không nguyên tử, nên kiểm tra kết quả ghi và báo lỗi; không dùng cơ chế giao dịch nhiều file trong thiết kế mục tiêu.
