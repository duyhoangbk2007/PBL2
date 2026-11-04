# Task của Kiệt — lộ trình PBL2 15 bước

Theo [phân công chính](../PhanCongCongViec.md), Kiệt phụ trách tài khoản, bán vé, giao diện Staff và tài liệu sử dụng. [Tổng quan tiến độ](../TongQuanTienDo.md) ghi trạng thái chung; file này ghi checklist riêng của Kiệt. Chỉ đánh dấu `[x]` khi có bằng chứng trong mã/nhật ký. Các mục `[x]` ở Bước 01–02 xác nhận hiện vật hoặc kiểm tra đã đối chiếu, không xác nhận toàn bước hoàn thành hay quy tác giả của từng file. Lần rà soát trước, `g++ -std=c++17 -Wall -Wextra -pedantic -fsyntax-only core/dal/models/*.cpp` chạy đạt. Hiện ở branch `feature/b02-hoang-models-relation`; chưa có PR.

**Quy tắc đồng bộ:** Sau khi hoàn thành một task, cập nhật file này và nhật ký bước nếu đã có. Chỉ đánh dấu bước chung hoàn thành ở `TongQuanTienDo.md` khi cả Hoàng và Kiệt đã xong, tích hợp và kiểm thử đạt. Khi tiếp tục Bước XX của Kiệt, đọc mục tương ứng, nhật ký bước và mã liên quan rồi chọn checkbox chưa hoàn thành tiếp theo. Không sửa Task của Hoàng khi chỉ làm phần Kiệt.

### Bước 01 – Khởi tạo dự án

**Trạng thái:** Đang thực hiện

**Branch:** `feature/b02-hoang-models-relation`

**Checklist công việc:**

- [x] Đối chiếu README và tài liệu hiện có với cấu trúc mới.
- [ ] Hoàn thiện README với lệnh build/chạy đã kiểm chứng.
- [x] Có `.gitignore` bỏ qua executable, object, thư mục build CMake và file tạm (commit `bf171f9`).
- [ ] Soạn `AGENTS.md` và quy tắc GitHub, branch, Pull Request.
- [ ] Tổ chức liên kết tài liệu và nhật ký cho lộ trình mới.
- [ ] Viết và chạy kiểm thử liên quan.
- [ ] Review và tích hợp với phần của Hoàng.

**Điều kiện hoàn thành:** README, Git và tài liệu hướng dẫn khởi tạo nhất quán, có thể làm theo. Có bằng chứng biên dịch/chạy thử hoặc kiểm thử phù hợp và không còn lỗi nghiêm trọng chưa xử lý.

**Pull Request:** Chưa có

**Ghi chú:** `README.md` và nhật ký đã có; `.gitignore` được thêm ở commit `bf171f9`, đồng thời `integration_test.exe` và `pbl2_test.exe` được bỏ theo dõi. `AGENTS.md` chưa có. Không coi việc có README là đã hoàn thiện hướng dẫn build.

### Bước 02 – Hoàn thiện Models và quan hệ con trỏ

**Trạng thái:** Đang thực hiện

**Branch:** `feature/b02-hoang-models-relation`

**Checklist công việc:**

- [x] Có đủ `User`, `Member`, `Ticket`, `Invoice` ở `core/dal/models/`.
- [x] `Member::setPoints()` và `Ticket::markCancelled()` hiện có trong mã.
- [x] Biên dịch cú pháp toàn bộ tám Model bằng C++17: đạt ở lần rà soát này.
- [x] Nối các quan hệ `Invoice`–`Ticket`, `User/Member`–`Invoice`; cấm copy/move các Model chứa danh sách con trỏ.
- [x] Kiểm tra trạng thái vé, điểm và tính nhất quán sau khi nối quan hệ.
- [ ] Khôi phục `tests/model_relations_test.cpp` vào cây hiện tại và chạy lại kiểm thử tích hợp.

**Điều kiện hoàn thành:** Bốn Model tài khoản/giao dịch có quan hệ nhất quán và được thử tích hợp với bốn Model của Hoàng. Có bằng chứng biên dịch/chạy thử hoặc kiểm thử phù hợp và không còn lỗi nghiêm trọng chưa xử lý.

**Pull Request:** Chưa có

**Ghi chú:** Nhật ký Bước 02 ghi nhận test C++17 đã chạy đạt, nhưng `tests/model_relations_test.cpp` không có trong cây hiện tại nên chưa thể chạy lại. `User`/`Member`/`Invoice` đã cấm copy/move. Việc quản lý vòng đời object sẽ xử lý khi có DALC ở Bước 04; Bước 02 còn chờ test có thể chạy lại.

### Bước 03 – Hạ tầng lưu trữ dữ liệu

**Trạng thái:** Chưa bắt đầu

**Branch:** Chưa tạo

**Checklist công việc:**

- [ ] Cài `TransactionManager` ghi nhiều TXT như một giao dịch.
- [ ] Cài `IdGenerator` đọc/cập nhật `sequences.txt` không tái dùng mã.
- [ ] Cài `DateTime` parse/định dạng ngày giờ theo schema.
- [ ] Thử lỗi ghi giữa chừng, rollback và phục hồi.
- [ ] Viết và chạy kiểm thử liên quan.
- [ ] Review và tích hợp với phần của Hoàng.

**Điều kiện hoàn thành:** Giao dịch nhiều file phục hồi được, mã và ngày giờ đúng định dạng. Có bằng chứng biên dịch/chạy thử hoặc kiểm thử phù hợp và không còn lỗi nghiêm trọng chưa xử lý.

**Pull Request:** Chưa có

**Ghi chú:** Thống nhất API với `FileHandler`/`Validator` của Hoàng; Bước 03 chưa bắt đầu.

### Bước 04 – Xây dựng DALC

**Trạng thái:** Chưa bắt đầu

**Branch:** Chưa tạo

**Checklist công việc:**

- [ ] Cài `UserDALC` và `MemberDALC` cho tài khoản/thành viên.
- [ ] Cài `TicketDALC` và `InvoiceDALC`, giữ dữ liệu lịch sử.
- [ ] Cài `SequenceDALC` cho bộ đếm mã.
- [ ] Cài `BookingDALC` phối hợp ghi vé/hóa đơn/điểm/bộ đếm.
- [ ] Dùng STL khi cần tra cứu; thử mã trùng và dữ liệu sai.
- [ ] Viết và chạy kiểm thử liên quan.
- [ ] Review và tích hợp với phần của Hoàng.

**Điều kiện hoàn thành:** Sáu DALC quản lý đúng bản ghi TXT và giao dịch, cung cấp dữ liệu cho BLL. Có bằng chứng biên dịch/chạy thử hoặc kiểm thử phù hợp và không còn lỗi nghiêm trọng chưa xử lý.

**Pull Request:** Chưa có

**Ghi chú:** Phối hợp DALC phim/phòng/suất/ghế của Hoàng và hạ tầng Bước 03.

### Bước 05 – Đăng nhập và phân quyền

**Trạng thái:** Chưa bắt đầu

**Branch:** Chưa tạo

**Checklist công việc:**

- [ ] Cài `AuthService` xác thực tài khoản hoạt động.
- [ ] Cài `UserManager` quản lý Staff theo quyền Admin.
- [ ] Thử đăng nhập sai, tài khoản vô hiệu và quyền từ chối.
- [ ] Viết và chạy kiểm thử liên quan.
- [ ] Review và tích hợp với phần của Hoàng.

**Điều kiện hoàn thành:** Đăng nhập và phân quyền hoạt động qua BLL, có ca thử thành công và từ chối. Có bằng chứng biên dịch/chạy thử hoặc kiểm thử phù hợp và không còn lỗi nghiêm trọng chưa xử lý.

**Pull Request:** Chưa có

**Ghi chú:** Hoàng thử quyền ở các nghiệp vụ quản trị; GUI không tự quyết định quyền.

### Bước 06 – Quản lý phim và thành viên

**Trạng thái:** Chưa bắt đầu

**Branch:** Chưa tạo

**Checklist công việc:**

- [ ] Cài `MemberManager` tạo, tra và sửa thông tin liên hệ.
- [ ] Kiểm tra số điện thoại/mã trùng và dữ liệu sai.
- [ ] Giữ điểm chỉ thay đổi qua nghiệp vụ hợp lệ.
- [ ] Viết và chạy kiểm thử liên quan.
- [ ] Review và tích hợp với phần của Hoàng.

**Điều kiện hoàn thành:** Thành viên được quản lý đúng quy tắc và điểm không bị sửa tùy ý từ GUI. Có bằng chứng biên dịch/chạy thử hoặc kiểm thử phù hợp và không còn lỗi nghiêm trọng chưa xử lý.

**Pull Request:** Chưa có

**Ghi chú:** Hoàng làm `MovieManager`; dữ liệu điểm dùng lại ở Bước 10–11.

### Bước 07 – Quản lý phòng và ghế

**Trạng thái:** Chưa bắt đầu

**Branch:** Chưa tạo

**Checklist công việc:**

- [ ] Xác định dữ liệu đầu vào cho sơ đồ ghế hai chiều.
- [ ] Dựng sơ đồ bằng mảng hai chiều hoặc `std::vector` khi phù hợp.
- [ ] Thử vị trí trùng, ngoài phòng và ghế đôi.
- [ ] Viết và chạy kiểm thử liên quan.
- [ ] Review và tích hợp với phần của Hoàng.

**Điều kiện hoàn thành:** Sơ đồ phản ánh chính xác phòng/ghế, phát hiện dữ liệu vị trí sai. Có bằng chứng biên dịch/chạy thử hoặc kiểm thử phù hợp và không còn lỗi nghiêm trọng chưa xử lý.

**Pull Request:** Chưa có

**Ghi chú:** Hoàng giữ `RoomManager`/`SeatManager`; thống nhất dữ liệu trả cho GUI.

### Bước 08 – Quản lý suất chiếu và giá vé

**Trạng thái:** Chưa bắt đầu

**Branch:** Chưa tạo

**Checklist công việc:**

- [ ] Xác định mức giá trong `PricingConfig` theo quy tắc đã thống nhất.
- [ ] Cài tính giá, giảm giá không cộng dồn và điểm thưởng.
- [ ] Thử ghế thường/VIP/đôi và loại vé hợp lệ.
- [ ] Viết và chạy kiểm thử liên quan.
- [ ] Review và tích hợp với phần của Hoàng.

**Điều kiện hoàn thành:** Giá và điểm tính đúng, có ca thử cho các trường hợp ưu đãi. Có bằng chứng biên dịch/chạy thử hoặc kiểm thử phù hợp và không còn lỗi nghiêm trọng chưa xử lý.

**Pull Request:** Chưa có

**Ghi chú:** Hoàng giữ `ShowtimeManager`; BLL cần thời điểm và trạng thái suất.

### Bước 09 – Chọn ghế và Undo

**Trạng thái:** Chưa bắt đầu

**Branch:** Chưa tạo

**Checklist công việc:**

- [ ] Cài giỏ chọn ghế tạm trong `BookingService`.
- [ ] Dùng `std::stack` để Undo lựa chọn trước thanh toán.
- [ ] Xử lý bỏ giao dịch/đổi suất, dọn giỏ và lịch sử Undo.
- [ ] Thử chọn trùng, Undo rỗng và ghế vừa bị bán.
- [ ] Viết và chạy kiểm thử liên quan.
- [ ] Review và tích hợp với phần của Hoàng.

**Điều kiện hoàn thành:** Chọn/Undo đúng, bỏ giỏ không sinh vé, hóa đơn hoặc điểm. Có bằng chứng biên dịch/chạy thử hoặc kiểm thử phù hợp và không còn lỗi nghiêm trọng chưa xử lý.

**Pull Request:** Chưa có

**Ghi chú:** Hoàng cung cấp trạng thái ghế theo suất; không tự cài Stack riêng.

### Bước 10 – Thanh toán và lưu vé/hóa đơn

**Trạng thái:** Chưa bắt đầu

**Branch:** Chưa tạo

**Checklist công việc:**

- [ ] Hoàn thiện `BookingService` xác nhận điều kiện thu tiền.
- [ ] Tạo `Ticket`/`Invoice`, tính tổng và điểm thành viên.
- [ ] Gọi DALC lưu đồng bộ vé, hóa đơn, điểm và bộ đếm.
- [ ] Chỉ trả thành công sau khi giao dịch lưu đủ; thử lỗi giữa chừng.
- [ ] Viết và chạy kiểm thử liên quan.
- [ ] Review và tích hợp với phần của Hoàng.

**Điều kiện hoàn thành:** Giao dịch bán vé nhất quán, có bằng chứng lưu thành công và rollback. Có bằng chứng biên dịch/chạy thử hoặc kiểm thử phù hợp và không còn lỗi nghiêm trọng chưa xử lý.

**Pull Request:** Chưa có

**Ghi chú:** Hoàng thử phòng/suất/ghế và phục hồi; cần `BookingDALC`/`TransactionManager`.

### Bước 11 – Hủy vé, lịch sử và báo cáo

**Trạng thái:** Chưa bắt đầu

**Branch:** Chưa tạo

**Checklist công việc:**

- [ ] Kiểm tra quyền và điều kiện hủy vé trước suất.
- [ ] Tính hoàn tiền, thu hồi điểm và giữ bản ghi hủy.
- [ ] Tra cứu lịch sử vé/hóa đơn theo quyền và bộ lọc.
- [ ] Viết và chạy kiểm thử liên quan.
- [ ] Review và tích hợp với phần của Hoàng.

**Điều kiện hoàn thành:** Hủy hợp lệ giữ lịch sử, số tiền/điểm và báo cáo khớp. Có bằng chứng biên dịch/chạy thử hoặc kiểm thử phù hợp và không còn lỗi nghiêm trọng chưa xử lý.

**Pull Request:** Chưa có

**Ghi chú:** Hoàng giữ `ReportService`; không xóa vật lý vé/hóa đơn đã giao dịch.

### Bước 12 – Nền tảng Qt

**Trạng thái:** Chưa bắt đầu

**Branch:** Chưa tạo

**Checklist công việc:**

- [ ] Tạo `LoginView` nhận thông tin và hiển thị kết quả BLL.
- [ ] Tạo `StaffDashboard` và điều hướng chức năng Staff.
- [ ] Nối đăng nhập với `AuthService` qua BLL.
- [ ] Viết và chạy kiểm thử liên quan.
- [ ] Review và tích hợp với phần của Hoàng.

**Điều kiện hoàn thành:** Đăng nhập và màn hình Staff chạy được, không truy cập DAL/DB từ GUI. Có bằng chứng biên dịch/chạy thử hoặc kiểm thử phù hợp và không còn lỗi nghiêm trọng chưa xử lý.

**Pull Request:** Chưa có

**Ghi chú:** Phối hợp `MainWindow` và bố cục chung của Hoàng.

### Bước 13 – Giao diện các chức năng

**Trạng thái:** Chưa bắt đầu

**Branch:** Chưa tạo

**Checklist công việc:**

- [ ] Tạo `StaffManagementView` và `MemberManagementView`.
- [ ] Tạo `BookingView`, `SeatMapView`, `InvoiceHistoryView`.
- [ ] Tạo `TicketTableModel` khi bảng vé cần Qt Model/View.
- [ ] Hiển thị lỗi, quyền và trạng thái từ BLL đúng luồng.
- [ ] Viết và chạy kiểm thử liên quan.
- [ ] Review và tích hợp với phần của Hoàng.

**Điều kiện hoàn thành:** Các màn hình Staff/bán vé gọi BLL và hiển thị đúng kết quả. Có bằng chứng biên dịch/chạy thử hoặc kiểm thử phù hợp và không còn lỗi nghiêm trọng chưa xử lý.

**Pull Request:** Chưa có

**Ghi chú:** Hoàng làm giao diện Admin; thống nhất dữ liệu/điều hướng chung.

### Bước 14 – Kiểm thử tích hợp toàn hệ thống

**Trạng thái:** Chưa bắt đầu

**Branch:** Chưa tạo

**Checklist công việc:**

- [ ] Thử đăng nhập, Staff, thành viên và luồng bán/hủy vé.
- [ ] Thử dữ liệu sai, lỗi ghi và phục hồi giao dịch.
- [ ] Kiểm tra lỗi GUI bán vé và luồng GUI → BLL → DAL → DB.
- [ ] Viết và chạy kiểm thử liên quan.
- [ ] Review và tích hợp với phần của Hoàng.

**Điều kiện hoàn thành:** Các ca Staff/booking và tích hợp đạt; lỗi nghiêm trọng đã xử lý. Có bằng chứng biên dịch/chạy thử hoặc kiểm thử phù hợp và không còn lỗi nghiêm trọng chưa xử lý.

**Pull Request:** Chưa có

**Ghi chú:** Hoàng thử quản trị; cùng kiểm quyền, bộ nhớ và dữ liệu dùng chung.

### Bước 15 – Đóng gói và hoàn thiện

**Trạng thái:** Chưa bắt đầu

**Branch:** Chưa tạo

**Checklist công việc:**

- [ ] Hoàn thiện hướng dẫn cài đặt và sử dụng sau khi chạy thử.
- [ ] Tổng hợp tài liệu chức năng và báo cáo.
- [ ] Chuẩn bị demo bán vé/hủy vé và nội dung bảo vệ.
- [ ] Viết và chạy kiểm thử liên quan.
- [ ] Review và tích hợp với phần của Hoàng.

**Điều kiện hoàn thành:** Hướng dẫn, báo cáo và demo khớp bản đóng gói đã chạy thử. Có bằng chứng biên dịch/chạy thử hoặc kiểm thử phù hợp và không còn lỗi nghiêm trọng chưa xử lý.

**Pull Request:** Chưa có

**Ghi chú:** Hoàng đóng gói Windows; cả hai kiểm bản cuối và chuẩn bị bảo vệ.
