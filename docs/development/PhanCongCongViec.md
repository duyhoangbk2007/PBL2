# Phân công phát triển PBL2 — Hoàng và Kiệt

## 1. Nguyên tắc

Giữ **15 bước**, bỏ bước riêng về cấu trúc dữ liệu. Chỉ `DoublyLinkedList<T>` bắt buộc tự cài; kiểm tra API, vòng đời và an toàn bộ nhớ của DLL trong Bước 02, rồi tích hợp và kiểm thử khi DALC dùng nó ở Bước 04. Các cấu trúc khác dùng STL khi phù hợp với chức năng: `std::unordered_map` cho tra mã trong DALC, `std::stack` cho Undo, `std::map` cho chỉ mục có thứ tự, `std::priority_queue` cho top phim, `std::vector` hoặc mảng hai chiều cho sơ đồ ghế. Không thêm cấu trúc chỉ để đủ số lượng.

Kiến trúc bắt buộc: **GUI (`ui/`) → BLL (`core/bll/`) → DAL (`core/dal/models/`, `core/dal/dalc/`) → DB (`data/*.txt`)**. Kết quả đi ngược chiều. GUI chỉ gọi BLL; BLL gọi DALC; DALC quản lý dữ liệu và đọc/ghi TXT. Model không tự đọc/ghi file. `core/data_structures/` là thành phần hỗ trợ.

## 2. Lộ trình và người phụ trách

Các tên class dưới đây là **kế hoạch**, chưa xác nhận đã triển khai. Trạng thái thực tế theo [Tổng quan tiến độ](TongQuanTienDo.md). Một bước chỉ hoàn thành khi phần Hoàng, Kiệt và tích hợp chung có bằng chứng kiểm thử.

| Bước | Công việc | Hoàng | Kiệt | Tích hợp/điểm kiểm tra |
| --- | --- | --- | --- | --- |
| 01 | Khởi tạo dự án | Cấu trúc thư mục, `CMakeLists.txt`, `main.cpp`, C++17 và build. | `README.md`, `.gitignore`, `AGENTS.md`, quy tắc GitHub/PR, tài liệu và nhật ký. | Kiểm tra cấu hình và build thật. |
| 02 | Hoàn thiện Models và quan hệ con trỏ | `Movie`, `Room`, `Seat`, `Showtime`; quan hệ phim/phòng/ghế/suất; kiểm DLL khi Model dùng. | `User`, `Member`, `Ticket`, `Invoice`; quan hệ hóa đơn/vé/người bán/thành viên; trạng thái vé và điểm. | Chốt chủ sở hữu object, vòng đời, copy/move; thử tích hợp 8 Model. |
| 03 | Hạ tầng lưu trữ dữ liệu | `FileHandler`, `TransactionManager`, đọc/ghi TXT, giao dịch, backup và rollback/phục hồi. | `IdGenerator`, `DateTime`. | Thống nhất API và schema TXT trước khi tích hợp; hoãn `Validator` sang DAL/Repository, chỉ cài hàm kiểm tra khi cần xử lý TXT. |
| 04 | Xây dựng DALC | `MovieDALC`, `RoomDALC`, `SeatDALC`, `ShowtimeDALC`. | `UserDALC`, `MemberDALC`, `TicketDALC`, `InvoiceDALC`, `SequenceDALC`, `BookingDALC`. | Tích hợp DLL/STL khi cần; kiểm đọc/ghi và liên kết Model. |
| 05 | Đăng nhập và phân quyền | Kiểm thử quyền Admin/Staff trong nghiệp vụ quản trị. | `AuthService`, `UserManager`, đăng nhập và quản lý nhân viên. | Quyền được BLL kiểm tra, không chỉ ẩn GUI. |
| 06 | Quản lý phim và thành viên | `MovieManager`: thêm/sửa/xóa hợp lệ, tìm kiếm, ngừng phim. | `MemberManager`: thành viên và quy tắc điểm. | Thử CRUD và dữ liệu tham chiếu. |
| 07 | Quản lý phòng và ghế | `RoomManager`, `SeatManager`: phòng, vị trí và loại ghế. | Sơ đồ ghế hai chiều; thử vị trí trùng, ghế đôi. | Dùng `std::vector`/mảng hai chiều nếu phù hợp. |
| 08 | Suất chiếu và giá vé | `ShowtimeManager`: tạo/sửa/hủy, trùng lịch, khoảng nghỉ. | `PricingConfig`: giá, ưu đãi, điểm thưởng. | Thử ràng buộc lịch và giá. |
| 09 | Chọn ghế và Undo | Cung cấp ghế trống/đã bán theo suất; thử điều kiện chọn. | `BookingService`: giỏ tạm, chọn ghế, Undo bằng `std::stack`, bỏ giao dịch/đổi suất. | Chưa ghi vé/hóa đơn khi bỏ giỏ. |
| 10 | Thanh toán và lưu vé/hóa đơn | Thử dữ liệu phòng/suất/ghế và lưu nhiều file, rollback, phục hồi. | Hoàn thiện `BookingService`: tạo `Ticket`/`Invoice`, tổng tiền, điểm, xác nhận thanh toán, lưu an toàn. | Chỉ báo thành công sau khi ghi đủ giao dịch. |
| 11 | Hủy vé, lịch sử và báo cáo | `ReportService`: tiền bán/hoàn, doanh thu thuần, số vé, top phim. | Hủy hợp lệ, hoàn tiền, thu hồi điểm, tra lịch sử vé/hóa đơn. | Giữ lịch sử; không xóa vật lý giao dịch. |
| 12 | Nền tảng Qt | `MainWindow`, `AdminDashboard`, điều hướng và giao diện chung. | `LoginView`, `StaffDashboard`, nối đăng nhập với BLL. | GUI không truy cập DAL/DB trực tiếp. |
| 13 | Giao diện chức năng | `MovieManagementView`, `RoomManagementView`, `ShowtimeManagementView`, `ReportView`, `MovieTableModel`. | `StaffManagementView`, `MemberManagementView`, `BookingView`, `SeatMapView`, `InvoiceHistoryView`, `TicketTableModel`. | Hiển thị kết quả/lỗi từ BLL. |
| 14 | Kiểm thử tích hợp toàn hệ thống | Chức năng Admin, phim/phòng/suất/báo cáo, dữ liệu và lỗi GUI quản trị. | Đăng nhập/Staff/thành viên/bán-hủy vé, dữ liệu lỗi, phục hồi, GUI bán vé. | Thử GUI → BLL → DAL → DB, quyền, bộ nhớ; sửa lỗi tìm được. |
| 15 | Đóng gói và hoàn thiện | CMake, build/đóng gói Windows, tài liệu kiến trúc. | Hướng dẫn dùng, tài liệu chức năng, báo cáo và demo. | Thử bản đóng gói và chuẩn bị bảo vệ. |

## 3. Ranh giới phân công và phụ thuộc

Hoàng phụ trách nhóm quản lý rạp (`Movie`, `Room`, `Seat`, `Showtime`, DALC/Manager tương ứng), `ReportService`, giao diện Admin, build và đóng gói. Kiệt phụ trách nhóm tài khoản và bán vé (`User`, `Member`, `Ticket`, `Invoice`, DALC/Manager tương ứng), `AuthService`, `BookingService`, giá, giao dịch, điểm và giao diện Staff. Cả hai review API, Model, cấu hình hoặc file dùng chung.

Trước khi sửa API Model/DLL phải chốt chủ sở hữu object, địa chỉ ổn định và copy/move; xem [quan hệ Model](../CauTrucThuMuc/models.md). Trước Bước 03 chốt hợp đồng `FileHandler`–`TransactionManager` và [định dạng TXT](../TongQuan/data_format.md). Trước Bước 09–10 chốt dữ liệu ghế theo suất và giao dịch nhiều file. Trước Bước 12–13 chốt dữ liệu BLL trả cho Qt. Mỗi bên có thể dùng fixture riêng khi phần phụ thuộc chưa xong, rồi thử tích hợp trước nghiệm thu.

## 4. Task cá nhân và đồng bộ tiến độ

[Task của Hoàng](tasks/Task_Hoang.md) và [Task của Kiệt](tasks/Task_Kiet.md) tách checklist chi tiết của mỗi người theo đúng 15 bước trong bảng phân công này. File phân công là **nguồn chính về nhiệm vụ**; hai file Task ghi tiến độ riêng; [Tổng quan tiến độ](TongQuanTienDo.md) ghi trạng thái chung. Khi sửa phân công hoặc checklist, đối chiếu cả ba nguồn.

Khi Hoàng/Kiệt hoàn thành một mục, chỉ cập nhật Task của người đó và nhật ký/tiến độ liên quan sau khi kiểm chứng. Không tự sửa Task của người kia khi chỉ thực hiện phần việc của một người. Nếu một người xong trước, bước chung vẫn chưa hoàn thành; chỉ đánh dấu hoàn thành khi cả hai đã xong, tích hợp và kiểm thử đạt. Khi được yêu cầu tiếp tục Bước XX của một người, đọc Task của người đó, nhật ký bước nếu có và mã liên quan; chọn checkbox chưa hoàn thành tiếp theo, làm đúng phạm vi rồi cập nhật bằng chứng.

## 5. Git, nhật ký và nghiệm thu

Nhánh đặt khi bắt đầu việc theo `feature/bXX-hoang-ten-viec` hoặc `feature/bXX-kiet-ten-viec`; ví dụ Bước 03 mới là `feature/b03-hoang-storage` và `feature/b03-kiet-id-datetime`, Bước 04 là `feature/b04-hoang-dalc` và `feature/b04-kiet-dalc`. Không tạo trước nhánh cho các bước chưa bắt đầu. Mỗi PR ghi file đổi, lệnh/kết quả thử và phụ thuộc; người kia review phần giao nhau. Không push trực tiếp hoặc force push `main`.

Nhật ký đặt trong `docs/development/`. Giữ nguyên lịch sử [Bước 01](Buoc01_KhoiTaoDuAn.md) và [Bước 02](Buoc02_XayDungModels.md); chỉ tạo nhật ký Bước 03–15 khi thực sự bắt đầu. Bước 03 **mới** là hạ tầng lưu trữ, không phải cấu trúc dữ liệu. [Nhật ký Bước 03](Buoc03_HaTangLuuTruDuLieu.md) đã được tạo khi Hoàng bắt đầu phần việc; các bước sau chưa bắt đầu thì chưa tạo nhật ký.

Một bước chỉ hoàn thành khi có kết quả biên dịch/chạy hoặc kiểm thử phù hợp cho phần Hoàng, phần Kiệt và tích hợp chung, đối chiếu quy tắc nghiệp vụ và schema. Nếu một bên xong trước, ghi phần đã xác minh nhưng không đánh dấu cả bước hoàn thành. Kiểm tra cú pháp một file không thay cho thử tích hợp. Ghi lỗi, cách sửa và việc còn lại vào nhật ký của bước đã bắt đầu.
