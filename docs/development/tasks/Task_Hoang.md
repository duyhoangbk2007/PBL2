# Task của Hoàng — lộ trình PBL2 14 bước

Theo [phân công chính](../PhanCongCongViec.md), Hoàng phụ trách toàn bộ Backend từ Bước 01 đến hết Bước 11: Models, Data Access, Business Logic, nghiệp vụ và kiểm thử C++. Từ Bước 12, Hoàng hỗ trợ API và sửa lỗi Backend khi Kiệt tích hợp GUI. [Tổng quan tiến độ](../TongQuanTienDo.md) ghi trạng thái chung; file này ghi checklist riêng của Hoàng. **Theo xác nhận của nhóm, Bước 01–03 đã hoàn thành.** Các checkbox và nhận xét trong ba mục đó là bằng chứng/lịch sử tại thời điểm ghi, không được tự đổi thành kết quả kiểm thử mới. Lần rà soát trước đã kiểm tra cú pháp các Model bằng C++17; mã hiện hành ở `src/models/`.

**Quy tắc đồng bộ:** Sau khi hoàn thành một task, cập nhật file này và nhật ký bước nếu đã có. Chỉ đánh dấu bước chung hoàn thành ở `TongQuanTienDo.md` khi phần việc của bước đó có kiểm thử và nghiệm thu phù hợp; việc đổi phân công không làm thay đổi checkbox lịch sử. Khi tiếp tục Bước XX của Hoàng, đọc mục tương ứng, nhật ký bước và mã liên quan rồi chọn checkbox chưa hoàn thành tiếp theo. Không sửa Task của Kiệt khi chỉ làm phần Hoàng.

### Bước 01 – Khởi tạo dự án

**Trạng thái hiện hành:** Đã hoàn thành theo xác nhận nhóm. **Tại mốc ghi checklist cũ:** Đang thực hiện.

**Branch:** `feature/b02-hoang-models-relation`

**Checklist công việc:**

- [x] Đối chiếu cây thư mục và tài liệu cấu trúc hiện có.
- [ ] Hoàn thiện các thư mục tối thiểu phục vụ Bước 01.
- [ ] Hoàn thiện hướng dẫn build C++17 trong `README.md` khi đã có lệnh chạy được kiểm chứng.
- [ ] Viết `CMakeLists.txt` cấu hình C++17 và nguồn hiện có.
- [x] Có `main.cpp` tối thiểu trong cây hiện tại.
- [ ] Build và chạy chương trình tối thiểu trên Windows.
- [ ] Viết và chạy kiểm thử liên quan.

**Điều kiện hoàn thành:** Build ứng dụng C++17 từ CMake và chạy được chương trình tối thiểu. Có bằng chứng biên dịch/chạy thử hoặc kiểm thử phù hợp và không còn lỗi nghiêm trọng chưa xử lý.

**Pull Request:** Chưa có

**Ghi chú:** Đã có `main.cpp`; chưa có `CMakeLists.txt` và chưa có bằng chứng build/chạy ứng dụng tối thiểu từ CMake. Commit `bf171f9` chỉ thêm `.gitignore` và bỏ theo dõi hai file `.exe`.

### Bước 02 – Xây dựng Models và quan hệ Model

**Trạng thái hiện hành:** Đã hoàn thành theo xác nhận nhóm. **Tại mốc ghi checklist cũ:** Đang thực hiện.

**Branch:** `feature/b02-hoang-models-relation`

**Checklist công việc:**

- [x] Có đủ `Movie`, `Room`, `Seat`, `Showtime` ở `src/models/`.
- [x] Biên dịch cú pháp bốn Model và toàn bộ tám `.cpp` bằng C++17: đạt ở lần rà soát này.
- [x] Nối hai chiều và kiểm tra mã/trùng cho `Movie`–`Showtime`, `Room`–`Seat`, `Room`–`Showtime`.
- [x] Kiểm thử liên kết Movie/Room–Showtime và ghế/phòng qua các test hiện có: `test_showtime_link.cpp` đạt 31/31, `test_seat_repository.cpp` đạt 85/85; phạm vi các bài test này được ghi riêng.
- [ ] Kiểm thử tích hợp quan hệ giao dịch Invoice/Ticket/User/Member với các Model còn lại; bài `tests/model_relations_test.cpp` được nhắc trong nhật ký cũ nhưng không có trên nhánh hiện tại.

**Điều kiện hoàn thành:** Bốn Model quản lý rạp liên kết nhất quán và có kiểm thử chạy đạt. Có bằng chứng biên dịch/chạy thử phù hợp và không còn lỗi nghiêm trọng chưa xử lý.

**Pull Request:** [PR #5](https://github.com/duyhoangbk2007/PBL2/pull/5), `feature/b02-hoang-models-relation` → `main`: đã đóng, chưa merge (kiểm tra GitHub 2026-11-06).

**Ghi chú:** `Movie`–`Showtime`, `Room`–`Seat`, `Room`–`Showtime` đã có liên kết hai chiều. Ghi chú cũ về việc cấm copy/move không còn đúng: người thực hiện dự án chủ động không dùng `= delete` trong cả tám Model. Quyền sở hữu object, gỡ liên kết và nạp lại dữ liệu sẽ xem xét ở DAL/Repository. Kiểm thử và tích hợp còn trong checklist chưa hoàn thành; các quan hệ giao dịch thuộc phần Kiệt.

### Bước 03 – Xây dựng tiện ích lưu trữ dữ liệu ban đầu

**Trạng thái hiện hành:** Đã hoàn thành theo xác nhận nhóm. `FileIO` đã có ở `src/utils/` và bài thử đọc/ghi cơ bản đã chạy đạt; các mục review/tích hợp còn mở trong checklist lịch sử tại thời điểm ghi.

**Branch lúc triển khai Bước 03:** `feature/b03-hoang-storage`. Hiện đang ở `feature/b06-hoang-room-seat-management`.

**Checklist công việc:**

- [x] Có `FileIO::readLines(...)` và `FileIO::writeLines(...)` để đọc/ghi các dòng TXT.
- [x] Chuyển `FileIO.h/.cpp` vào `src/utils/` và cập nhật nơi gọi theo tên mới.
- [x] Biên dịch, chạy `test_all_utils.cpp`: phần `FileIO` đạt; Codex chạy lại C++17 ngày 2026-11-17 với mã giao dịch lịch sử, kết quả 4 PASS, 0 FAILED.
- [ ] Đối chiếu API `FileIO` với nhu cầu đọc/ghi toàn bộ file của Repository; kiểm tra cách báo lỗi và xử lý file thiếu.

**Điều kiện hoàn thành:** Đọc/ghi TXT qua `FileIO` đúng định dạng và báo lỗi khi thất bại; phần Hoàng, Kiệt được review và kiểm tra tích hợp. Không yêu cầu cơ chế giao dịch nhiều file.

**Pull Request:** Chưa tạo PR cho Bước 03.

**Ghi chú:** `FileIO` chỉ đọc/ghi dòng TXT; Repository kiểm tra định dạng, khóa và tham chiếu. Ghi nhiều file tuần tự không nguyên tử, nên phải kiểm tra từng lần ghi và báo lỗi rõ. `Validator` vẫn hoãn đến khi Repository cần hàm kiểm tra cụ thể. [Nhật ký Bước 03](../Buoc03_HaTangLuuTruDuLieu.md) giữ nguyên lịch sử các thử nghiệm trước khi đổi thiết kế; đó không còn là checklist công việc hiện hành.

### Bước 04 – Quản lý phim

**Phụ trách: Hoàng. Trạng thái: đang thực hiện; chưa đủ nghiệp vụ đổi trạng thái phim.** Xem [nhật ký Bước 04](../Buoc04_QuanLyPhim.md).

- [x] Có `MovieRepository`; `test_movie_repository.cpp` chạy đạt 40/40.
- [x] Có `MovieService` cho thêm/sửa/xóa/tra cứu với kiểm tra quyền; `test_movie_service.cpp` chạy đạt 45/45 cho phạm vi hiện có.
- [ ] Hoàn thiện nghiệp vụ bật/tắt trạng thái phim và điều kiện tham chiếu theo quy tắc dự án; API hiện tại từ chối đổi `active` trong `updateMovie()`.
- [ ] Kiểm tra lỗi ghi, thông báo file/dòng khi nạp sai và kiểm thử lại phần nghiệp vụ mới trước khi nghiệm thu toàn Bước 04.

**Kết quả kiểm tra:** Codex biên dịch C++17 và chạy hai bài test trên trong thư mục tạm ngày 2026-11-17; cả hai thoát mã 0. Kết quả này không xác nhận chức năng đổi trạng thái chưa có.

### Bước 05 – Đăng nhập và phân quyền

**Phụ trách: Hoàng. Trạng thái: đang thực hiện; chưa nghiệm thu.** Xem [nhật ký Bước 05](../Buoc05_DangNhapVaPhanQuyen.md).

- [x] Có mã `UserRepository` và `AuthService`; các luồng đăng nhập và quyền cơ bản đã đạt trong `test_auth.cpp`.
- [ ] Hoàn thiện hợp đồng lỗi khi thiếu file/chưa load, dữ liệu sai hoặc trùng, reload và lỗi ghi.
- [ ] Chạy lại toàn bộ `test_auth.cpp` đạt; lần kiểm tra C++17 ngày 2026-11-17 đạt **9/14**, còn 5 ca không đạt.
- [ ] Kiểm tra phân quyền trong các Service và nghiệm thu Bước 05.

### Bước 06 – Quản lý phòng, ghế và suất chiếu

**Phụ trách: Hoàng. Trạng thái: đang thực hiện; phần tạo/sửa/tra cứu suất đã có test đạt, hủy/xóa còn phụ thuộc vé.** Xem [nhật ký Bước 06](../Buoc06_QuanLyPhongVaGhe.md).

- [x] Có `RoomRepository`, `RoomService`; test C++17 riêng đạt lần lượt **78/78** và **54/54**.
- [x] Có `SeatRepository`, `SeatService`; test C++17 riêng đạt lần lượt **85/85** và **59/59**.
- [x] Có `ShowtimeRepository`; test C++17 riêng đạt **89/89**, gồm lịch cùng phòng với khoảng nghỉ 15 phút và liên kết Movie/Room.
- [x] Kiểm thử liên kết Movie/Room–Showtime bằng `test_showtime_link.cpp` đạt **31/31**.
- [x] Có `ShowtimeService` cho tạo/sửa/tra cứu, kiểm tra quyền và lịch chiếu; `test_showtime_service.cpp` **89/89 PASS, 0 FAIL** khi chạy C++17 ngày 2026-11-17.
- [ ] Hoàn thiện `cancelShowtime()` và `removeShowtime()` sau khi có `TicketRepository` để kiểm tra ràng buộc vé; hiện hai hàm trả `false` và test chỉ xác nhận hành vi từ chối này.
- [ ] Kiểm tra tích hợp phòng/ghế/suất với vé, lỗi file, con trỏ và quyền; chưa có đủ các Repository vé để nghiệm thu toàn Bước 06.

**Kết quả kiểm tra:** Codex biên dịch và chạy các bài test trên trong thư mục tạm ngày 2026-11-17; tất cả thoát mã 0. Kết quả chỉ xác nhận phạm vi của các bài test hiện có.

**Kiểm tra bổ sung 2026-11-17:** Biên dịch và chạy riêng `test_showtime_service.cpp`: **89 PASS, 0 FAIL**, thoát mã 0. Kết quả không nghiệm thu hủy/xóa hoặc toàn Bước 06.

### Bước 07 – Quản lý nhân viên

**Phụ trách: Hoàng. Trạng thái: `StaffService` và test riêng đã đạt; còn các giới hạn nêu dưới đây.** Xem [Bước 07](../Buoc07_QuanLyNhanVien.md).

- [x] Dùng chung `UserRepository` với `AuthService`; `StaffService` kiểm tra quyền Admin trước khi thêm/sửa/vô hiệu hóa/tra cứu Staff.
- [x] Triển khai thêm, sửa, tìm theo ID/username, duyệt/đếm Staff và vô hiệu hóa tài khoản; giữ Admin ngoài các thao tác này.
- [x] Kiểm thử riêng `test_staff_service.cpp`: mốc đã ghi **59/59 PASS**; lần chạy trên nhánh hiện tại **67/67 PASS, 0 FAIL** với C++17.
- [ ] Đối chiếu phụ thuộc lịch sử hóa đơn khi các Repository giao dịch có sẵn; chưa có xóa vật lý hoặc kích hoạt lại Staff.

### Bước 08 – Quản lý thành viên

**Phụ trách: Hoàng. Trạng thái: `MemberRepository` đã có test riêng đạt; `MemberService` chưa triển khai.** Xem [Bước 08](../Buoc08_QuanLyThanhVien.md).

- [x] Triển khai `MemberRepository`: nạp/lưu, thêm, cập nhật họ tên/số điện thoại, tra ID/phone; kiểm tra số điện thoại duy nhất, điểm không âm và không cho thêm mới với điểm khác 0 hoặc đổi điểm qua `update()`.
- [x] Chạy `test_member_repository.cpp` với C++17 trên nhánh hiện tại: **63 PASS, 0 FAIL**, gồm dữ liệu hợp lệ/sai/trùng và lưu/nạp lại.
- [ ] Triển khai `MemberService` cho phân quyền và nghiệp vụ quản lý thành viên; viết và chạy test Service.
- [ ] Kiểm tra liên kết hóa đơn, cập nhật điểm trong luồng bán/hủy vé khi các Repository và Service giao dịch có sẵn.

### Bước 09 – Giá vé, ưu đãi và tích điểm

**Phụ trách: Hoàng. Trạng thái: chưa bắt đầu.**

- [ ] Cài quy tắc giá theo loại ghế/vé và ưu đãi học sinh/thành viên; lấy mức giảm hợp lệ cao nhất, không cộng dồn.
- [ ] Tính tiền, điểm và kiểm thử C++ các trường hợp biên; chỉ tạo `PricingService` nếu có lý do rõ.

### Bước 10 – Bán vé tại quầy

**Phụ trách: Hoàng. Trạng thái: chưa bắt đầu.**

- [ ] Triển khai `TicketRepository`, `InvoiceRepository`, `BookingService` và luồng chọn/Undo ghế trước thanh toán.
- [ ] Kiểm tra ghế theo suất, xác nhận thu tiền, tạo hóa đơn nhiều vé, cập nhật điểm và ghi TXT.
- [ ] Kiểm thử C++ trùng ghế, tham chiếu, lỗi ghi và tích hợp các Repository; cung cấp API cho GUI của Kiệt.

### Bước 11 – Hủy vé, lịch sử hóa đơn và thống kê

**Phụ trách: Hoàng. Trạng thái: chưa bắt đầu.**

- [ ] Mở rộng `BookingService` cho hủy/hoàn vé, thu hồi điểm và giữ lịch sử; cung cấp truy vấn hóa đơn.
- [ ] Triển khai `ReportService` cho tiền bán/hoàn, doanh thu thuần, vé bán/hủy và top phim.
- [ ] Kiểm thử C++ nghiệp vụ hủy, quyền, lịch sử, thống kê và trường hợp biên; bàn giao hợp đồng dữ liệu cho GUI.

### Bước 12 – Xây dựng giao diện Qt

**Phụ trách GUI: Kiệt. Trạng thái: chưa bắt đầu.**

- [ ] Hoàng cung cấp và giải thích API Service/Model cần cho các màn hình; hỗ trợ sửa lỗi Backend được xác nhận khi Kiệt tích hợp.
Kiệt thực hiện View, `MainWindow` và sơ đồ ghế GUI trong bước này.

### Bước 13 – Kiểm thử tích hợp và sửa lỗi

**Kiệt chủ trì tích hợp GUI; Hoàng phụ trách lỗi Backend. Trạng thái: chưa bắt đầu.**

- [ ] Phối hợp kiểm tra luồng GUI → Service → Repository → TXT; sửa lỗi Backend có bằng chứng từ kiểm thử tích hợp.
- [ ] Kiểm thử lại các phần Backend bị sửa và ghi kết quả.

### Bước 14 – Đóng gói, hoàn thiện tài liệu và bàn giao

**Kiệt phụ trách GUI và bàn giao; Hoàng hỗ trợ Backend. Trạng thái: chưa bắt đầu.**

- [ ] Cung cấp hướng dẫn build Backend, tài liệu kiến trúc và xử lý lỗi Backend khi đóng gói.
- [ ] Phối hợp Kiệt kiểm tra bản bàn giao; không nhận phần lập trình GUI.
