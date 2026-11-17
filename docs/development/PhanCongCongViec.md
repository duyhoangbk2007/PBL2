# Phân công phát triển PBL2 — Hoàng và Kiệt

## Nguyên tắc chung

Dự án giữ **14 bước**. Nhóm xác nhận Bước 01–03 đã hoàn thành; phân công hiện hành: Hoàng phụ trách toàn bộ Backend ở Bước 01–11, Kiệt phụ trách toàn bộ GUI và tích hợp giao diện từ Bước 12. Phần việc lịch sử của mỗi người ở các bước đã qua được giữ nguyên; [nhật ký Bước 01](Buoc01_KhoiTaoDuAn.md), [Bước 02](Buoc02_XayDungModels.md) và [Bước 03](Buoc03_HaTangLuuTruDuLieu.md) giữ nguyên lịch sử, bằng chứng và giới hạn kỹ thuật tại thời điểm thực hiện. Từ Bước 04, làm **theo chức năng hoàn chỉnh**: xây dựng Repository → Service → kiểm thử C++ → cập nhật tài liệu. Không chờ hoàn tất mọi Repository mới bắt đầu Service. GUI Qt được triển khai tập trung ở Bước 12.

Kiến trúc mục tiêu: **GUI (`src/gui/`) → Business (`src/business/`) → Data Access (`src/data_access/`) → DB (`data/*.txt`)**. `src/models/`, `src/data_structures/`, `src/utils/` hỗ trợ các tầng; `FileIO` chỉ đọc/ghi dòng TXT. Chỉ `DoublyLinkedList<T>` bắt buộc tự cài; cấu trúc khác dùng STL khi phù hợp. Các class ghi trong kế hoạch là **dự kiến**, trừ mã thực tế được nêu ở [tiến độ](TongQuanTienDo.md).

## Lộ trình và người phụ trách

| Bước | Nội dung | Phụ trách | Kết quả cần có |
| --- | --- | --- | --- |
| 01 | Khởi tạo dự án | Hoàng — Backend; Kiệt tham gia theo lịch sử | Giữ kết quả và nhật ký lịch sử. |
| 02 | Xây dựng Models và quan hệ Model | Hoàng — Backend; Kiệt tham gia theo lịch sử | Giữ tám Model và nhật ký lịch sử. |
| 03 | Xây dựng tiện ích lưu trữ dữ liệu ban đầu | Hoàng — Backend; Kiệt tham gia theo lịch sử | Giữ kết quả lịch sử; thiết kế hiện hành dùng `FileIO`, `DateTime`, `IdGenerator`. |
| 04 | Quản lý phim | Hoàng | `MovieRepository` và `MovieService` đã có test riêng đạt; tiếp tục nghiệp vụ đổi trạng thái, lỗi file và kiểm thử bổ sung trước nghiệm thu. |
| 05 | Đăng nhập và phân quyền | Hoàng | `UserRepository`, `AuthService`, kiểm tra tài khoản hoạt động và quyền Admin/Staff; [nhật ký Bước 05](Buoc05_DangNhapVaPhanQuyen.md) ghi 9/14 test đạt, còn 5 ca lỗi. |
| 06 | Quản lý phòng, ghế và suất chiếu | Hoàng | `RoomRepository`, `SeatRepository`, `ShowtimeRepository`; `RoomService`, `SeatService`, `ShowtimeService`; sơ đồ ghế, lịch cùng phòng và nghỉ 15 phút, kiểm thử. |
| 07 | Quản lý nhân viên | Hoàng | `StaffService` dùng lại `UserRepository`, thêm/sửa/tìm/vô hiệu hóa Staff, kiểm tra quyền Admin, kiểm thử. |
| 08 | Quản lý thành viên | Hoàng | `MemberRepository`, `MemberService`, thông tin liên hệ và số điện thoại duy nhất, kiểm thử. |
| 09 | Giá vé, ưu đãi và tích điểm | Hoàng | Xử lý giá theo cấu hình, loại ghế, ưu đãi học sinh/thành viên, tiền và điểm; kiểm thử C++. |
| 10 | Bán vé tại quầy | Hoàng | `TicketRepository`, `InvoiceRepository`, `BookingService`, chọn/Undo ghế, hóa đơn nhiều vé, điểm và lưu TXT; kiểm thử. |
| 11 | Hủy vé, lịch sử hóa đơn và thống kê | Hoàng | Xử lý hủy/hoàn/thu hồi điểm/lịch sử, `ReportService` và thống kê; kiểm thử C++. |
| 12 | Xây dựng giao diện Qt | Kiệt | Toàn bộ View Admin/Staff, `MainWindow`, dashboard, `SeatMapView` và cấu hình build GUI Qt; kết nối với API Backend. |
| 13 | Kiểm thử tích hợp và sửa lỗi | Kiệt — tích hợp GUI; Hoàng — lỗi Backend | Kiệt thử luồng GUI → Business → Data Access → TXT; Hoàng sửa lỗi Backend được xác nhận từ tích hợp. |
| 14 | Đóng gói, hoàn thiện tài liệu và bàn giao | Kiệt — GUI/bàn giao; Hoàng — hỗ trợ Backend | Đóng gói ứng dụng, hướng dẫn, báo cáo, demo; Hoàng cung cấp tài liệu và hỗ trợ build Backend. |

Các bước 04–11 bao gồm kiểm thử chức năng C++ và cập nhật tài liệu ngay trong bước; chúng không chờ GUI Qt. Không đánh dấu bước hoàn thành chỉ vì Model hoặc bản nháp Repository đã tồn tại. [Task Hoàng](tasks/Task_Hoang.md), [Task Kiệt](tasks/Task_Kiet.md) ghi việc cá nhân; [Tổng quan tiến độ](TongQuanTienDo.md) ghi trạng thái chung.

## Phân công cụ thể và phụ thuộc

**Hoàng:** Chịu trách nhiệm toàn bộ Backend ở Bước 01–11: Models, cấu trúc dữ liệu và tiện ích, Repository, Service, phân quyền, phòng/ghế/suất, nhân viên, thành viên, giá/điểm, bán/hủy vé, lịch sử, báo cáo và kiểm thử C++. Tại Bước 12–14, Hoàng cung cấp API, sửa lỗi Backend phát hiện khi tích hợp và tài liệu kỹ thuật liên quan; không phụ trách lập trình View.

**Kiệt:** Chịu trách nhiệm toàn bộ giao diện Qt từ Bước 12, gồm màn hình Admin/Staff, `MainWindow`, dashboard, `SeatMapView`, luồng thao tác và hiển thị lỗi. Kiệt tích hợp GUI với Backend qua Service, kiểm thử luồng giao diện, đóng gói phần GUI và chuẩn bị hướng dẫn sử dụng/demo. Ở Bước 01–03, Kiệt đã tham gia Backend theo lịch sử; việc đổi phân công không xóa kết quả đó.

**Phối hợp:** Hoàng cung cấp hợp đồng Service và dữ liệu cho GUI; Kiệt báo lỗi tích hợp có thể tái hiện để Hoàng xử lý phần Backend. Không chuyển việc kiểm tra quyền/nghiệp vụ vào View; Kiệt chỉ gọi Service.

| Giai đoạn | Hoàng | Kiệt |
| --- | --- | --- |
| Bước 01–03 | Backend và tích hợp phần lịch sử đã thực hiện | Ghi nhận đóng góp lịch sử; không nhận thêm việc Backend |
| Bước 04–11 | Toàn bộ Repository, Service, nghiệp vụ và kiểm thử C++ | Chuẩn bị yêu cầu dữ liệu/giao diện khi cần, không triển khai Backend |
| Bước 12 | Cung cấp API và hỗ trợ tích hợp Backend | Xây dựng toàn bộ GUI Qt và nối Service |
| Bước 13 | Sửa lỗi Backend có bằng chứng | Chủ trì kiểm thử tích hợp GUI–Backend |
| Bước 14 | Hỗ trợ build Backend, tài liệu kỹ thuật | Đóng gói GUI, hướng dẫn, demo và bàn giao |

Suất chiếu cần dữ liệu phim/phòng; bán vé cần phim/phòng/ghế/suất; thống kê cần vé/hóa đơn. Hoàng hoàn thiện các phụ thuộc này ở Backend trước khi Kiệt tích hợp giao diện. Các bước giữ nguyên thứ tự 01–14; việc có thể chuẩn bị hợp đồng GUI sớm không đánh dấu bước sau đã bắt đầu.

## Theo dõi và nghiệm thu

[Nhật ký Bước 04](Buoc04_QuanLyPhim.md) ghi kế hoạch quản lý phim hiện hành; [Bước 05](Buoc05_DangNhapVaPhanQuyen.md) ghi phần đăng nhập/phân quyền và giữ riêng mốc lịch sử DALC. `MovieRepository` hiện đã có; đoạn về code cũ thuộc lịch sử Bước 04. Không tạo sẵn nhật ký trống cho các bước chưa bắt đầu. Bước chỉ được nghiệm thu khi phần phụ trách có mã, kiểm thử và tài liệu tương ứng; không suy ra kết quả từ tên file dự kiến. Ghi nhiều file TXT tuần tự không nguyên tử, nên kiểm tra kết quả ghi và báo lỗi; không dùng cơ chế giao dịch nhiều file trong thiết kế mục tiêu.
