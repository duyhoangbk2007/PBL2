# Task của Hoàng — lộ trình PBL2 15 bước

Theo [phân công chính](../PhanCongCongViec.md), Hoàng phụ trách quản lý rạp, giao diện Admin, build và đóng gói. [Tổng quan tiến độ](../TongQuanTienDo.md) ghi trạng thái chung; file này ghi checklist riêng của Hoàng. Chỉ đánh dấu `[x]` khi có bằng chứng trong mã/nhật ký. Các mục `[x]` ở Bước 01–02 xác nhận hiện vật hoặc kiểm tra đã đối chiếu, không xác nhận toàn bước hoàn thành hay quy tác giả của từng file. Lần rà soát này, `g++ -std=c++17 -Wall -Wextra -pedantic -fsyntax-only core/dal/models/*.cpp` chạy đạt. Chưa tạo branch hoặc PR.

**Quy tắc đồng bộ:** Sau khi hoàn thành một task, cập nhật file này và nhật ký bước nếu đã có. Chỉ đánh dấu bước chung hoàn thành ở `TongQuanTienDo.md` khi cả Hoàng và Kiệt đã xong, tích hợp và kiểm thử đạt. Khi tiếp tục Bước XX của Hoàng, đọc mục tương ứng, nhật ký bước và mã liên quan rồi chọn checkbox chưa hoàn thành tiếp theo. Không sửa Task của Kiệt khi chỉ làm phần Hoàng.

### Bước 01 – Khởi tạo dự án

**Trạng thái:** Đang thực hiện

**Branch:** Chưa tạo

**Checklist công việc:**

- [x] Đối chiếu cây thư mục và tài liệu cấu trúc hiện có.
- [ ] Hoàn thiện các thư mục tối thiểu phục vụ Bước 01.
- [ ] Viết `CMakeLists.txt` cấu hình C++17 và nguồn hiện có.
- [ ] Tạo `main.cpp` tối thiểu, build và chạy trên Windows.
- [ ] Viết và chạy kiểm thử liên quan.
- [ ] Review và tích hợp với phần của Kiệt.

**Điều kiện hoàn thành:** Build ứng dụng C++17 từ CMake và chạy được chương trình tối thiểu. Có bằng chứng biên dịch/chạy thử hoặc kiểm thử phù hợp và không còn lỗi nghiêm trọng chưa xử lý.

**Pull Request:** Chưa có

**Ghi chú:** Chưa có `CMakeLists.txt` hoặc `main.cpp` trong cây hiện tại; nhật ký Bước 01 chỉ xác nhận khảo sát.

### Bước 02 – Hoàn thiện Models và quan hệ con trỏ

**Trạng thái:** Đang thực hiện

**Branch:** Chưa tạo

**Checklist công việc:**

- [x] Có đủ `Movie`, `Room`, `Seat`, `Showtime` ở `core/dal/models/`.
- [x] Biên dịch cú pháp bốn Model và toàn bộ tám `.cpp` bằng C++17: đạt ở lần rà soát này.
- [ ] Chốt phạm vi quan hệ hai chiều thuộc Hoàng theo `docs/CauTrucThuMuc/models.md`: `Movie`–`Showtime`, `Room`–`Seat`, `Room`–`Showtime`. Phân biệt danh sách/con trỏ trong Model với chiều ngược tra cứu qua DALC.
- [ ] Với `Room`–`Seat`, bổ sung chiều `Seat → Room` sau khi thống nhất nơi sở hữu `Seat`; kiểm tra mã phòng, ghế trùng mã/vị trí và nối/gỡ nhất quán hai phía.
- [ ] Với `Movie`–`Showtime` và `Room`–`Showtime`, bổ sung chiều ngược đã chốt; kiểm tra mã tham chiếu, tránh trùng và gỡ liên kết trước khi xóa/tải lại object.
- [ ] Rà soát địa chỉ ổn định, vòng đời và copy/move của `DoublyLinkedList<T>` cùng các Model thuộc Hoàng chứa danh sách con trỏ.
- [ ] Viết và chạy kiểm thử các quan hệ thuộc Hoàng: mã đúng/sai, `nullptr`, trùng, hai chiều, gỡ liên kết và copy/move; ghi lệnh và kết quả.
- [ ] Review và tích hợp với phần của Kiệt.

**Điều kiện hoàn thành:** Bốn Model quản lý rạp liên kết nhất quán, DLL an toàn khi dùng và có kiểm thử tích hợp tám Model. Có bằng chứng biên dịch/chạy thử hoặc kiểm thử phù hợp và không còn lỗi nghiêm trọng chưa xử lý.

**Pull Request:** Chưa có

**Ghi chú:** Hiện `Room → Seat` và `Showtime → Movie/Room` đã có; các chiều ngược tương ứng chưa có. Con trỏ Model dự kiến không sở hữu object, mã ID tiếp tục dùng để lưu TXT. Cần thống nhất chủ sở hữu và địa chỉ ổn định với Kiệt trước khi triển khai; các quan hệ `Ticket`/`Invoice`/`User`/`Member` thuộc phần Kiệt, không đánh dấu trong checklist này.

### Bước 03 – Hạ tầng lưu trữ dữ liệu

**Trạng thái:** Chưa bắt đầu

**Branch:** Chưa tạo

**Checklist công việc:**

- [ ] Thiết kế API `FileHandler` cho đọc/ghi TXT và lỗi I/O.
- [ ] Cài `Validator` kiểm trường, kiểu và định dạng theo schema.
- [ ] Thử file hợp lệ, sai định dạng, thiếu file và lỗi ghi.
- [ ] Viết và chạy kiểm thử liên quan.
- [ ] Review và tích hợp với phần của Kiệt.

**Điều kiện hoàn thành:** DALC có thể dùng tiện ích đọc/ghi TXT và nhận lỗi rõ ràng. Có bằng chứng biên dịch/chạy thử hoặc kiểm thử phù hợp và không còn lỗi nghiêm trọng chưa xử lý.

**Pull Request:** Chưa có

**Ghi chú:** Thống nhất hợp đồng file với `TransactionManager` của Kiệt; chưa bắt đầu Bước 03.

### Bước 04 – Xây dựng DALC

**Trạng thái:** Chưa bắt đầu

**Branch:** Chưa tạo

**Checklist công việc:**

- [ ] Cài `MovieDALC` đọc/ghi, tìm và kiểm mã phim.
- [ ] Cài `RoomDALC` và `SeatDALC`, đối chiếu ghế với phòng.
- [ ] Cài `ShowtimeDALC`, kiểm mã phim/phòng tham chiếu.
- [ ] Tích hợp DLL hoặc STL khi cần và thử nạp/lưu lại dữ liệu.
- [ ] Viết và chạy kiểm thử liên quan.
- [ ] Review và tích hợp với phần của Kiệt.

**Điều kiện hoàn thành:** Bốn DALC quản lý được bản ghi TXT đúng schema và trả dữ liệu cho BLL. Có bằng chứng biên dịch/chạy thử hoặc kiểm thử phù hợp và không còn lỗi nghiêm trọng chưa xử lý.

**Pull Request:** Chưa có

**Ghi chú:** Cần API lưu file Bước 03 và thống nhất quan hệ Model với DALC của Kiệt.

### Bước 05 – Đăng nhập và phân quyền

**Trạng thái:** Chưa bắt đầu

**Branch:** Chưa tạo

**Checklist công việc:**

- [ ] Lập ma trận quyền Admin/Staff cho thao tác quản trị.
- [ ] Thử truy cập hợp lệ và bị từ chối qua BLL.
- [ ] Kiểm tra nghiệp vụ quản trị không chỉ dựa vào ẩn nút GUI.
- [ ] Viết và chạy kiểm thử liên quan.
- [ ] Review và tích hợp với phần của Kiệt.

**Điều kiện hoàn thành:** Quyền Admin/Staff được kiểm tra trong BLL với ca thử thành công và từ chối. Có bằng chứng biên dịch/chạy thử hoặc kiểm thử phù hợp và không còn lỗi nghiêm trọng chưa xử lý.

**Pull Request:** Chưa có

**Ghi chú:** Phối hợp `AuthService` và `UserManager` của Kiệt.

### Bước 06 – Quản lý phim và thành viên

**Trạng thái:** Chưa bắt đầu

**Branch:** Chưa tạo

**Checklist công việc:**

- [ ] Cài `MovieManager` thêm và tìm phim với dữ liệu hợp lệ.
- [ ] Cài sửa/ngừng phim và điều kiện xóa khi chưa được tham chiếu.
- [ ] Thử mã trùng, dữ liệu sai và quyền thao tác.
- [ ] Viết và chạy kiểm thử liên quan.
- [ ] Review và tích hợp với phần của Kiệt.

**Điều kiện hoàn thành:** CRUD phim tuân thủ quyền và quy tắc tham chiếu. Có bằng chứng biên dịch/chạy thử hoặc kiểm thử phù hợp và không còn lỗi nghiêm trọng chưa xử lý.

**Pull Request:** Chưa có

**Ghi chú:** Cần `MovieDALC` Bước 04; Kiệt triển khai thành viên song song.

### Bước 07 – Quản lý phòng và ghế

**Trạng thái:** Chưa bắt đầu

**Branch:** Chưa tạo

**Checklist công việc:**

- [ ] Cài `RoomManager` cho phòng, trạng thái và kích thước.
- [ ] Cài `SeatManager` cho vị trí, loại ghế và kiểm trùng.
- [ ] Kiểm tra điều kiện sửa phòng/ghế đã có dữ liệu liên quan.
- [ ] Viết và chạy kiểm thử liên quan.
- [ ] Review và tích hợp với phần của Kiệt.

**Điều kiện hoàn thành:** Phòng và ghế được quản lý nhất quán, không tạo vị trí trùng hoặc phá dữ liệu lịch sử. Có bằng chứng biên dịch/chạy thử hoặc kiểm thử phù hợp và không còn lỗi nghiêm trọng chưa xử lý.

**Pull Request:** Chưa có

**Ghi chú:** Phối hợp Kiệt về sơ đồ hai chiều và ghế đôi.

### Bước 08 – Quản lý suất chiếu và giá vé

**Trạng thái:** Chưa bắt đầu

**Branch:** Chưa tạo

**Checklist công việc:**

- [ ] Cài `ShowtimeManager` tạo/sửa/hủy suất theo quyền.
- [ ] Kiểm tra thời gian, trùng lịch cùng phòng và khoảng nghỉ.
- [ ] Thử suất tham chiếu phim/phòng không hợp lệ.
- [ ] Viết và chạy kiểm thử liên quan.
- [ ] Review và tích hợp với phần của Kiệt.

**Điều kiện hoàn thành:** Lịch chiếu không có xung đột và tuân thủ điều kiện sửa/hủy. Có bằng chứng biên dịch/chạy thử hoặc kiểm thử phù hợp và không còn lỗi nghiêm trọng chưa xử lý.

**Pull Request:** Chưa có

**Ghi chú:** Kiệt phụ trách cấu hình giá và điểm, cần thống nhất dữ liệu đầu ra.

### Bước 09 – Chọn ghế và Undo

**Trạng thái:** Chưa bắt đầu

**Branch:** Chưa tạo

**Checklist công việc:**

- [ ] Tạo truy vấn trạng thái ghế trống/đã bán theo suất.
- [ ] Kiểm tra ghế thuộc phòng của suất và không bị bán trùng.
- [ ] Cung cấp dữ liệu trạng thái cho `BookingService` qua BLL.
- [ ] Viết và chạy kiểm thử liên quan.
- [ ] Review và tích hợp với phần của Kiệt.

**Điều kiện hoàn thành:** Booking nhận được trạng thái ghế đúng theo `(showtimeId, seatId)`. Có bằng chứng biên dịch/chạy thử hoặc kiểm thử phù hợp và không còn lỗi nghiêm trọng chưa xử lý.

**Pull Request:** Chưa có

**Ghi chú:** Kiệt giữ giỏ tạm và Undo bằng `std::stack`; không ghi file ở giai đoạn chọn tạm.

### Bước 10 – Thanh toán và lưu vé/hóa đơn

**Trạng thái:** Chưa bắt đầu

**Branch:** Chưa tạo

**Checklist công việc:**

- [ ] Thử phòng, suất và ghế ngay trước khi ghi giao dịch.
- [ ] Thử lưu nhiều file thành công và lỗi giữa chừng.
- [ ] Kiểm tra rollback/phục hồi không để vé, hóa đơn, điểm lệch nhau.
- [ ] Viết và chạy kiểm thử liên quan.
- [ ] Review và tích hợp với phần của Kiệt.

**Điều kiện hoàn thành:** Giao dịch nhiều file có bằng chứng thành công và phục hồi khi lỗi. Có bằng chứng biên dịch/chạy thử hoặc kiểm thử phù hợp và không còn lỗi nghiêm trọng chưa xử lý.

**Pull Request:** Chưa có

**Ghi chú:** Phối hợp `BookingService`, `BookingDALC` và `TransactionManager` của Kiệt.

### Bước 11 – Hủy vé, lịch sử và báo cáo

**Trạng thái:** Chưa bắt đầu

**Branch:** Chưa tạo

**Checklist công việc:**

- [ ] Cài `ReportService` tính tiền bán, tiền hoàn, doanh thu thuần.
- [ ] Thống kê số vé bán/hủy và top phim theo dữ liệu hợp lệ.
- [ ] Thử bộ lọc thời gian, nhân viên/phim và dữ liệu hủy.
- [ ] Viết và chạy kiểm thử liên quan.
- [ ] Review và tích hợp với phần của Kiệt.

**Điều kiện hoàn thành:** Báo cáo khớp vé/hóa đơn lưu và xử lý hoàn tiền. Có bằng chứng biên dịch/chạy thử hoặc kiểm thử phù hợp và không còn lỗi nghiêm trọng chưa xử lý.

**Pull Request:** Chưa có

**Ghi chú:** Cần dữ liệu lịch sử do Kiệt quản lý; có thể dùng `std::priority_queue` nếu phù hợp.

### Bước 12 – Nền tảng Qt

**Trạng thái:** Chưa bắt đầu

**Branch:** Chưa tạo

**Checklist công việc:**

- [ ] Tạo `MainWindow` và điều hướng màn hình theo BLL.
- [ ] Tạo `AdminDashboard` với các lối vào quản trị.
- [ ] Thiết lập bố cục, tài nguyên và build Qt Widgets.
- [ ] Viết và chạy kiểm thử liên quan.
- [ ] Review và tích hợp với phần của Kiệt.

**Điều kiện hoàn thành:** Cửa sổ quản trị mở được, điều hướng đúng và không truy cập DAL trực tiếp. Có bằng chứng biên dịch/chạy thử hoặc kiểm thử phù hợp và không còn lỗi nghiêm trọng chưa xử lý.

**Pull Request:** Chưa có

**Ghi chú:** Phối hợp luồng đăng nhập của Kiệt.

### Bước 13 – Giao diện các chức năng

**Trạng thái:** Chưa bắt đầu

**Branch:** Chưa tạo

**Checklist công việc:**

- [ ] Tạo `MovieManagementView` và `RoomManagementView`.
- [ ] Tạo `ShowtimeManagementView` và `ReportView`.
- [ ] Tạo `MovieTableModel` khi bảng phim cần Qt Model/View.
- [ ] Viết và chạy kiểm thử liên quan.
- [ ] Review và tích hợp với phần của Kiệt.

**Điều kiện hoàn thành:** Các màn hình quản trị gọi BLL và hiển thị kết quả/lỗi đúng. Có bằng chứng biên dịch/chạy thử hoặc kiểm thử phù hợp và không còn lỗi nghiêm trọng chưa xử lý.

**Pull Request:** Chưa có

**Ghi chú:** Kiệt làm các màn hình Staff; thống nhất dữ liệu BLL trả về.

### Bước 14 – Kiểm thử tích hợp toàn hệ thống

**Trạng thái:** Chưa bắt đầu

**Branch:** Chưa tạo

**Checklist công việc:**

- [ ] Thử luồng Admin quản lý phim, phòng, suất và báo cáo.
- [ ] Thử dữ liệu sai, quyền từ chối và lỗi GUI quản trị.
- [ ] Kiểm tra tích hợp GUI → BLL → DAL → DB và lỗi bộ nhớ.
- [ ] Viết và chạy kiểm thử liên quan.
- [ ] Review và tích hợp với phần của Kiệt.

**Điều kiện hoàn thành:** Các ca quản trị và tích hợp đạt; lỗi nghiêm trọng đã xử lý. Có bằng chứng biên dịch/chạy thử hoặc kiểm thử phù hợp và không còn lỗi nghiêm trọng chưa xử lý.

**Pull Request:** Chưa có

**Ghi chú:** Phối hợp Kiệt thử luồng Staff/booking, đặc biệt dữ liệu dùng chung.

### Bước 15 – Đóng gói và hoàn thiện

**Trạng thái:** Chưa bắt đầu

**Branch:** Chưa tạo

**Checklist công việc:**

- [ ] Hoàn thiện cấu hình CMake cho build phát hành.
- [ ] Đóng gói chương trình Windows và chạy thử bản đóng gói.
- [ ] Tổng hợp tài liệu kiến trúc và chuẩn bị phần bảo vệ.
- [ ] Viết và chạy kiểm thử liên quan.
- [ ] Review và tích hợp với phần của Kiệt.

**Điều kiện hoàn thành:** Bản Windows chạy được và tài liệu kiến trúc khớp mã. Có bằng chứng biên dịch/chạy thử hoặc kiểm thử phù hợp và không còn lỗi nghiêm trọng chưa xử lý.

**Pull Request:** Chưa có

**Ghi chú:** Kiệt chuẩn bị hướng dẫn, báo cáo và demo; cả hai thử gói cuối.

