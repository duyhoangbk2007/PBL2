# Task của Hoàng — lộ trình PBL2 15 bước

Theo [phân công chính](../PhanCongCongViec.md), Hoàng phụ trách quản lý rạp, giao diện Admin, build và đóng gói. [Tổng quan tiến độ](../TongQuanTienDo.md) ghi trạng thái chung; file này ghi checklist riêng của Hoàng. **Theo xác nhận mới của nhóm, Bước 01–03 đã hoàn thành.** Các checkbox và nhận xét trong ba mục đó là bằng chứng/lịch sử tại thời điểm ghi, không được tự đổi thành kết quả kiểm thử mới. Lần rà soát trước, `g++ -std=c++17 -Wall -Wextra -pedantic -fsyntax-only core/dal/models/*.cpp` chạy đạt. Nhánh hiện tại: `feature/b04-hoang-dalc`.

**Quy tắc đồng bộ:** Sau khi hoàn thành một task, cập nhật file này và nhật ký bước nếu đã có. Chỉ đánh dấu bước chung hoàn thành ở `TongQuanTienDo.md` khi cả Hoàng và Kiệt đã xong, tích hợp và kiểm thử đạt. Khi tiếp tục Bước XX của Hoàng, đọc mục tương ứng, nhật ký bước và mã liên quan rồi chọn checkbox chưa hoàn thành tiếp theo. Không sửa Task của Kiệt khi chỉ làm phần Hoàng.

### Bước 01 – Khởi tạo dự án

**Trạng thái hiện hành:** Đã hoàn thành theo xác nhận nhóm. **Tại mốc ghi checklist cũ:** Đang thực hiện.

**Branch:** `feature/b02-hoang-models-relation`

**Checklist công việc:**

- [x] Đối chiếu cây thư mục và tài liệu cấu trúc hiện có.
- [ ] Hoàn thiện các thư mục tối thiểu phục vụ Bước 01.
- [ ] Viết `CMakeLists.txt` cấu hình C++17 và nguồn hiện có.
- [x] Có `main.cpp` tối thiểu trong cây hiện tại.
- [ ] Build và chạy chương trình tối thiểu trên Windows.
- [ ] Viết và chạy kiểm thử liên quan.
- [ ] Review và tích hợp với phần của Kiệt.

**Điều kiện hoàn thành:** Build ứng dụng C++17 từ CMake và chạy được chương trình tối thiểu. Có bằng chứng biên dịch/chạy thử hoặc kiểm thử phù hợp và không còn lỗi nghiêm trọng chưa xử lý.

**Pull Request:** Chưa có

**Ghi chú:** Đã có `main.cpp`; chưa có `CMakeLists.txt` và chưa có bằng chứng build/chạy ứng dụng tối thiểu từ CMake. Commit `bf171f9` chỉ thêm `.gitignore` và bỏ theo dõi hai file `.exe`.

### Bước 02 – Xây dựng Models và quan hệ Model

**Trạng thái hiện hành:** Đã hoàn thành theo xác nhận nhóm. **Tại mốc ghi checklist cũ:** Đang thực hiện.

**Branch:** `feature/b02-hoang-models-relation`

**Checklist công việc:**

- [x] Có đủ `Movie`, `Room`, `Seat`, `Showtime` ở `core/dal/models/`.
- [x] Biên dịch cú pháp bốn Model và toàn bộ tám `.cpp` bằng C++17: đạt ở lần rà soát này.
- [x] Nối hai chiều và kiểm tra mã/trùng cho `Movie`–`Showtime`, `Room`–`Seat`, `Room`–`Showtime`.
- [ ] Viết và chạy kiểm thử các quan hệ trên, gồm mã sai, `nullptr`, trùng và kết quả ở cả hai phía.

**Điều kiện hoàn thành:** Bốn Model quản lý rạp liên kết nhất quán và có kiểm thử chạy đạt. Có bằng chứng biên dịch/chạy thử phù hợp và không còn lỗi nghiêm trọng chưa xử lý.

**Pull Request:** [PR #5](https://github.com/duyhoangbk2007/PBL2/pull/5), `feature/b02-hoang-models-relation` → `main`: đã đóng, chưa merge (kiểm tra GitHub 2026-11-06).

**Ghi chú:** `Movie`–`Showtime`, `Room`–`Seat`, `Room`–`Showtime` đã có liên kết hai chiều. Ghi chú cũ về việc cấm copy/move không còn đúng: người thực hiện dự án chủ động không dùng `= delete` trong cả tám Model. Quyền sở hữu object, gỡ liên kết và nạp lại dữ liệu sẽ xem xét ở DAL/Repository. Kiểm thử và tích hợp còn trong checklist chưa hoàn thành; các quan hệ giao dịch thuộc phần Kiệt.

### Bước 03 – Xây dựng tiện ích lưu trữ dữ liệu ban đầu

**Trạng thái hiện hành:** Đã hoàn thành theo xác nhận nhóm. `FileIO` đã có ở `src/utils/` và bài thử đọc/ghi cơ bản đã chạy đạt; các mục review/tích hợp còn mở trong checklist lịch sử tại thời điểm ghi.

**Branch lúc triển khai Bước 03:** `feature/b03-hoang-storage`. Hiện đang ở `feature/b04-hoang-dalc`.

**Checklist công việc:**

- [x] Có `FileIO::readLines(...)` và `FileIO::writeLines(...)` để đọc/ghi các dòng TXT.
- [x] Chuyển `FileIO.h/.cpp` vào `src/utils/` và cập nhật nơi gọi theo tên mới.
- [x] Biên dịch, chạy `test_all_utils.cpp`: phần `FileIO` đạt; lần kiểm tra sau chuyển thư mục có 4 PASS, 0 FAILED cho toàn bộ bài thử.
- [ ] Đối chiếu API `FileIO` với nhu cầu đọc/ghi toàn bộ file của Repository; kiểm tra cách báo lỗi và xử lý file thiếu.
- [ ] Review và tích hợp với `IdGenerator`, `DateTime` của Kiệt.
- [ ] Kiểm tra tích hợp sau khi merge.

**Điều kiện hoàn thành:** Đọc/ghi TXT qua `FileIO` đúng định dạng và báo lỗi khi thất bại; phần Hoàng, Kiệt được review và kiểm tra tích hợp. Không yêu cầu cơ chế giao dịch nhiều file.

**Pull Request:** Chưa tạo PR cho Bước 03.

**Ghi chú:** `FileIO` chỉ đọc/ghi dòng TXT; Repository kiểm tra định dạng, khóa và tham chiếu. Ghi nhiều file tuần tự không nguyên tử, nên phải kiểm tra từng lần ghi và báo lỗi rõ. `Validator` vẫn hoãn đến khi Repository cần hàm kiểm tra cụ thể. [Nhật ký Bước 03](../Buoc03_HaTangLuuTruDuLieu.md) giữ nguyên lịch sử các thử nghiệm trước khi đổi thiết kế; đó không còn là checklist công việc hiện hành.

### Bước 04 – Quản lý phim

**Phụ trách chính: Hoàng. Trạng thái: chuẩn bị viết lại từ đầu.** `src/data_access/MovieRepository.h/.cpp` cũ đã bị xóa khỏi cây làm việc và chưa từng được nghiệm thu; `MovieService` và kiểm thử chức năng phim chưa có. Xem [nhật ký Bước 04](../Buoc04_QuanLyPhim.md).

- [x] Chuyển bản nháp sang thư mục mới và dùng `FileIO`; kiểm tra cú pháp C++17.
- [ ] Viết lại `MovieRepository` từ đầu: đọc/ghi `movies.txt`, thêm/sửa/xóa, tra cứu, phát hiện mã trùng và dữ liệu lỗi; không dùng việc chuyển tên code cũ làm bằng chứng hoàn thành.
- [ ] Viết `MovieService`: thêm, sửa, tìm kiếm, cập nhật trạng thái và xóa khi đủ điều kiện; kiểm tra quyền và tham chiếu.
- [ ] Kiểm thử chức năng C++ với dữ liệu hợp lệ/lỗi, lưu rồi nạp lại; ghi lệnh, kết quả và cập nhật tài liệu.

**Điều kiện hoàn thành:** Repository và Service phim hoạt động, có bằng chứng kiểm thử; bản nháp riêng lẻ chưa đủ.

### Bước 05 – Đăng nhập và phân quyền

**Phụ trách chính: Kiệt. Trạng thái: chưa bắt đầu.**

- [ ] Hoàng thống nhất cách Service quản trị nhận thông tin vai trò từ `AuthService` của Kiệt.
- [ ] Hỗ trợ thử quyền Admin/Staff ở các chức năng Hoàng phụ trách khi API đăng nhập đã có.

### Bước 06 – Quản lý phòng và ghế

**Phụ trách chính: Hoàng. Trạng thái: chưa bắt đầu.**

- [ ] Cài `RoomRepository`, `SeatRepository` để đọc/ghi, tra cứu phòng và ghế.
- [ ] Cài `RoomService` quản lý phòng, sơ đồ ghế và loại ghế; kiểm tra trùng vị trí và quan hệ phòng–ghế.
- [ ] Kiểm thử C++ dữ liệu hợp lệ/lỗi, quyền, tham chiếu và nạp lại; cập nhật tài liệu.

### Bước 07 – Quản lý nhân viên

**Phụ trách chính: Kiệt. Trạng thái: chưa bắt đầu.**

- [ ] Hỗ trợ kiểm tra quyền Admin trong luồng quản trị khi Kiệt tích hợp `StaffService`.

### Bước 08 – Quản lý suất chiếu

**Phụ trách chính: Hoàng. Trạng thái: chưa bắt đầu.**

- [ ] Cài `ShowtimeRepository` và `ShowtimeService` để tạo, sửa, hủy, tra cứu suất.
- [ ] Kiểm tra quan hệ phim–phòng–suất, lịch cùng phòng và khoảng nghỉ theo quy tắc hiện hành.
- [ ] Kiểm thử C++ trường hợp lịch hợp lệ/trùng, tham chiếu sai và lưu/nạp; cập nhật tài liệu.

### Bước 09 – Quản lý thành viên

**Phụ trách chính: Kiệt. Trạng thái: chưa bắt đầu.**

- [ ] Thống nhất dữ liệu thành viên cần cho bán vé và thống kê khi API của Kiệt có sẵn.

### Bước 10 – Giá vé, ưu đãi và tích điểm

**Kiệt chính, Hoàng hỗ trợ. Trạng thái: chưa bắt đầu.**

- [ ] Hoàng kiểm tra giá theo loại ghế và dữ liệu ghế mà phần tính giá sử dụng.
- [ ] Cùng Kiệt thử mức giá/ưu đãi liên quan loại ghế; giữ quy tắc ưu đãi không cộng dồn.

### Bước 11 – Bán vé tại quầy

**Kiệt chính, Hoàng hỗ trợ tích hợp. Trạng thái: chưa bắt đầu.**

- [ ] Cung cấp và kiểm tra API phim, phòng, ghế, suất chiếu cho `BookingService`.
- [ ] Thử ghế thuộc đúng phòng/suất, ghế đã bán theo suất và lỗi ghi file liên quan.
- [ ] Kiểm thử tích hợp với phần Kiệt, ghi kết quả và cập nhật tài liệu.

### Bước 12 – Hủy vé, lịch sử hóa đơn và thống kê

**Cùng phụ trách; Hoàng làm thống kê. Trạng thái: chưa bắt đầu.**

- [ ] Cài `ReportService` dùng dữ liệu vé, hóa đơn và suất chiếu.
- [ ] Thống kê tiền bán/hoàn, doanh thu thuần, số vé bán/hủy và top phim.
- [ ] Kiểm thử C++ dữ liệu thường, vé hủy và trường hợp biên; phối hợp truy vấn lịch sử với Kiệt.

### Bước 13 – Xây dựng giao diện Qt

**Cùng phụ trách; Hoàng làm giao diện quản trị. Trạng thái: chưa bắt đầu.**

- [ ] Xây dựng `MovieManagementView`, `RoomManagementView`, `ShowtimeManagementView`, `ReportView`.
- [ ] Cùng Kiệt thống nhất `MainWindow`, dashboard Admin/Staff, `SeatMapView` và CMake/Qt.
- [ ] GUI chỉ gọi Service; kiểm tra hiển thị kết quả và lỗi.

### Bước 14 – Kiểm thử tích hợp và sửa lỗi

**Cùng phụ trách. Trạng thái: chưa bắt đầu.**

- [ ] Thử luồng Admin, phim/phòng/ghế/suất/thống kê qua GUI → Business → Data Access → TXT.
- [ ] Cùng Kiệt thử quyền, dữ liệu sai, lỗi ghi và sửa lỗi có bằng chứng.

### Bước 15 – Đóng gói, hoàn thiện tài liệu và bàn giao

**Cùng phụ trách. Trạng thái: chưa bắt đầu.**

- [ ] Hoàng phụ trách cấu hình build/đóng gói Windows và tài liệu kiến trúc.
- [ ] Cùng Kiệt thử bản đóng gói, hoàn thiện báo cáo, demo và bàn giao.

