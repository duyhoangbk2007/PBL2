# Task của Kiệt — lộ trình PBL2 15 bước

Theo [phân công chính](../PhanCongCongViec.md), Kiệt phụ trách tài khoản, bán vé, giao diện Staff và tài liệu sử dụng. [Tổng quan tiến độ](../TongQuanTienDo.md) ghi trạng thái chung; file này ghi checklist riêng của Kiệt. **Theo xác nhận mới của nhóm, Bước 01–03 đã hoàn thành.** Các checkbox và nhận xét trong ba mục đó là bằng chứng/lịch sử tại thời điểm ghi, không được tự đổi thành kết quả kiểm thử mới. Lần rà soát trước, `g++ -std=c++17 -Wall -Wextra -pedantic -fsyntax-only core/dal/models/*.cpp` chạy đạt. Nhánh hiện tại: `feature/b04-hoang-dalc`.

**Quy tắc đồng bộ:** Sau khi hoàn thành một task, cập nhật file này và nhật ký bước nếu đã có. Chỉ đánh dấu bước chung hoàn thành ở `TongQuanTienDo.md` khi cả Hoàng và Kiệt đã xong, tích hợp và kiểm thử đạt. Khi tiếp tục Bước XX của Kiệt, đọc mục tương ứng, nhật ký bước và mã liên quan rồi chọn checkbox chưa hoàn thành tiếp theo. Không sửa Task của Hoàng khi chỉ làm phần Kiệt.

### Bước 01 – Khởi tạo dự án

**Trạng thái hiện hành:** Đã hoàn thành theo xác nhận nhóm. **Tại mốc ghi checklist cũ:** Đang thực hiện.

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

### Bước 02 – Xây dựng Models và quan hệ Model

**Trạng thái hiện hành:** Đã hoàn thành theo xác nhận nhóm. **Tại mốc ghi checklist cũ:** Đang thực hiện.

**Branch:** `feature/b02-hoang-models-relation`

**Checklist công việc:**

- [x] Có đủ `User`, `Member`, `Ticket`, `Invoice` ở `core/dal/models/`.
- [x] `Member::setPoints()` và `Ticket::markCancelled()` hiện có trong mã.
- [x] Biên dịch cú pháp toàn bộ tám Model bằng C++17: đạt ở lần rà soát này.
- [x] Nối các quan hệ `Invoice`–`Ticket`, `User/Member`–`Invoice`.
- [x] Kiểm tra trạng thái vé, điểm và tính nhất quán sau khi nối quan hệ.
- [ ] Khôi phục `tests/model_relations_test.cpp` vào cây hiện tại và chạy lại kiểm thử tích hợp.

**Điều kiện hoàn thành:** Bốn Model tài khoản/giao dịch có quan hệ nhất quán và được thử tích hợp với bốn Model của Hoàng. Có bằng chứng biên dịch/chạy thử hoặc kiểm thử phù hợp và không còn lỗi nghiêm trọng chưa xử lý.

**Pull Request:** [PR #4](https://github.com/duyhoangbk2007/PBL2/pull/4), `feature/b02-kiet-model-relations` → `develop`: đang mở, chưa merge (kiểm tra GitHub 2026-11-06).

**Ghi chú:** `Invoice`–`Ticket`, `User`–`Invoice`, `Member`–`Invoice` đã có liên kết hai chiều; `Invoice::linkMember()` đã xử lý khách vãng lai và liên kết Member theo ID. `Invoice`–`Showtime`, `Ticket`–`Showtime` và `Ticket`–`Seat` hiện là một chiều. Người thực hiện dự án chủ động không dùng `= delete` trong cả tám Model; việc quản lý vòng đời object và gỡ liên kết sẽ xem xét ở DAL/Repository. Nhật ký cũ ghi test C++17 đã chạy đạt, nhưng `tests/model_relations_test.cpp` không có trong cây hiện tại; mục kiểm thử tích hợp vẫn chưa hoàn thành ở trạng thái hiện nay.

### Bước 03 – Xây dựng tiện ích lưu trữ dữ liệu ban đầu

**Trạng thái hiện hành:** Đã hoàn thành theo xác nhận nhóm. `IdGenerator` và chức năng lấy thời gian hiện tại của `DateTime` đã có; checklist cũ vẫn ghi parse ngày giờ và tích hợp chính thức còn mở tại thời điểm đó.

**Branch:** Mã hiện có trên nhánh `feature/b03-hoang-storage`; chưa xác nhận nhánh/PR riêng của Kiệt.

**Checklist công việc:**

- [x] Cài `IdGenerator` đọc `sequences.txt`, tạo mã và trả `updatedSequences` để bên gọi lưu khi tạo bản ghi; file bộ đếm không có dòng `#schema=1`.
- [x] Cài `DateTime::getCurrentDateTime()` lấy và định dạng thời gian hiện tại.
- [x] Kiểm thử cơ bản `IdGenerator` và định dạng đầu ra `DateTime` trong `test_all_utils.cpp`; lần chạy sau chuyển thư mục có 4 PASS, 0 FAILED cho toàn bộ bài thử.
- [x] Viết và chạy kiểm thử liên quan cho API hiện có (`test_all_utils.cpp`); Codex đã chạy lại sau khi chuyển mã sang `src/`.
- [ ] Review và tích hợp với phần của Hoàng.

**Điều kiện hoàn thành:** Mã và ngày giờ đúng định dạng. Có bằng chứng biên dịch/chạy thử hoặc kiểm thử phù hợp và không còn lỗi nghiêm trọng chưa xử lý.

**Pull Request:** Chưa có

**Ghi chú:** Kiệt phụ trách `IdGenerator` và `DateTime`, phối hợp với `FileIO` của Hoàng. `IdGenerator` chỉ tính nội dung bộ đếm mới, chưa tự ghi `sequences.txt`; Repository cần lưu nội dung này khi tạo bản ghi. `DateTime` hiện chỉ có `getCurrentDateTime()`, chưa parse hoặc kiểm tra đầu vào ngày giờ; vì vậy mục thử ngày giờ hợp lệ/không hợp lệ vẫn mở. Review và tích hợp chính thức chưa được xác nhận. `Validator` hoãn sang Repository, chỉ cài hàm kiểm tra TXT thật sự cần dùng.

### Bước 04 – Quản lý phim

**Phụ trách chính: Hoàng. Trạng thái: chuẩn bị viết lại từ đầu.** `MovieRepository` cũ chuyển từ `MovieDALC` đã bị xóa khỏi cây làm việc và chưa nghiệm thu. Hoàng sẽ viết lại Repository, xây dựng `MovieService` và kiểm thử C++; Kiệt phối hợp thống nhất API cần dùng cho các bước sau.

- [ ] Kiệt thống nhất API phim cần cho đăng nhập, bán vé và các chức năng sau khi Hoàng triển khai.

### Bước 05 – Đăng nhập và phân quyền

**Phụ trách chính: Kiệt. Trạng thái: hoàn thành phần C++ Bước 05.** Ngày 2026-11-12, `tests/test_auth.cpp` biên dịch không cảnh báo và chạy đạt **14 nhóm PASS, 0 FAILED**, bao gồm tích hợp Repository/Service/TXT. Xem [lệnh chạy, kết quả và phạm vi](../Buoc05_DangNhapVaPhanQuyen.md). GUI và tích hợp các chức năng khác thuộc các bước sau.

- [x] Cài `UserRepository` đọc/ghi `users.txt` và tra ID/tên đăng nhập; kiểm thử tích hợp đạt.
- [x] Cài `AuthService` đăng nhập/đăng xuất, kiểm tra tài khoản hoạt động và vai trò Admin/Staff; kiểm thử tích hợp đạt.
- [x] Kiểm thử C++ đúng/sai thông tin, tài khoản vô hiệu hóa và quyền; cập nhật tài liệu (14 nhóm đạt, 2026-11-12).

### Bước 06 – Quản lý phòng và ghế

**Phụ trách chính: Hoàng. Trạng thái: chưa bắt đầu.**

- [ ] Kiệt thống nhất dữ liệu phòng/ghế cần cho chọn ghế và bán vé với Hoàng.

### Bước 07 – Quản lý nhân viên

**Phụ trách chính: Kiệt. Trạng thái: chưa bắt đầu.**

- [ ] Tái sử dụng `UserRepository` từ Bước 05, chỉ tạo Repository mới nếu có nhu cầu thực tế.
- [ ] Cài `StaffService` thêm, sửa, tìm kiếm, vô hiệu hóa Staff; kiểm tra quyền Admin.
- [ ] Kiểm thử C++ tài khoản trùng, trạng thái, quyền và lưu/nạp; cập nhật tài liệu.

### Bước 08 – Quản lý suất chiếu

**Phụ trách chính: Hoàng. Trạng thái: chưa bắt đầu.**

- [ ] Kiệt thống nhất API suất chiếu cần cho bán vé; phối hợp thử dữ liệu tham chiếu khi Hoàng hoàn thành.

### Bước 09 – Quản lý thành viên

**Phụ trách chính: Kiệt. Trạng thái: chưa bắt đầu.**

- [ ] Cài `MemberRepository` và `MemberService` thêm, sửa, tìm kiếm thành viên.
- [ ] Kiểm tra số điện thoại duy nhất và không cho sửa điểm trực tiếp.
- [ ] Kiểm thử C++ dữ liệu hợp lệ/trùng/sai và lưu/nạp; cập nhật tài liệu.

### Bước 10 – Giá vé, ưu đãi và tích điểm

**Kiệt chính, Hoàng hỗ trợ. Trạng thái: chưa bắt đầu.**

- [ ] Xử lý giá theo cấu hình, loại vé, loại ghế và ưu đãi học sinh/thành viên; lấy mức giảm hợp lệ cao nhất.
- [ ] Tính tiền và điểm theo quy tắc hiện hành; chỉ tạo `PricingService` nếu có lý do rõ.
- [ ] Hoàng hỗ trợ kiểm giá theo loại ghế; Kiệt kiểm thử C++ giá, ưu đãi và điểm, cập nhật tài liệu.

### Bước 11 – Bán vé tại quầy

**Kiệt chính, Hoàng hỗ trợ tích hợp. Trạng thái: chưa bắt đầu.**

- [ ] Cài `TicketRepository`, `InvoiceRepository` và `BookingService`.
- [ ] Chọn suất/ghế, kiểm tra ghế còn trống theo suất, Undo trước thanh toán và tính tổng tiền.
- [ ] Xác nhận thu tiền, tạo một hóa đơn nhiều vé, cập nhật điểm và ghi TXT qua Repository/`FileIO`.
- [ ] Cùng Hoàng tích hợp phim/phòng/ghế/suất; kiểm thử C++ bán vé, trùng ghế và lỗi ghi; cập nhật tài liệu.

### Bước 12 – Hủy vé, lịch sử hóa đơn và thống kê

**Cùng phụ trách; Kiệt làm hủy/lịch sử. Trạng thái: chưa bắt đầu.**

- [ ] Mở rộng `BookingService` để hủy vé theo quyền và thời hạn, hoàn tiền, thu hồi điểm.
- [ ] Giữ lịch sử vé đã hủy; cung cấp truy vấn hóa đơn cho `ReportService` của Hoàng.
- [ ] Kiểm thử C++ hủy hợp lệ/không hợp lệ, điểm và lịch sử; cập nhật tài liệu.

### Bước 13 – Xây dựng giao diện Qt

**Cùng phụ trách; Kiệt làm giao diện Staff. Trạng thái: chưa bắt đầu.**

- [ ] Xây dựng `LoginView`, `StaffManagementView`, `MemberManagementView`, `BookingView`, `InvoiceHistoryView`.
- [ ] Cùng Hoàng thống nhất `MainWindow`, dashboard Admin/Staff, `SeatMapView` và CMake/Qt.
- [ ] GUI chỉ gọi Service; kiểm tra hiển thị kết quả và lỗi.

### Bước 14 – Kiểm thử tích hợp và sửa lỗi

**Cùng phụ trách. Trạng thái: chưa bắt đầu.**

- [ ] Thử đăng nhập, quyền, thành viên, bán/hủy vé và lịch sử qua GUI → Business → Data Access → TXT.
- [ ] Cùng Hoàng thử dữ liệu sai, lỗi ghi và sửa lỗi có bằng chứng.

### Bước 15 – Đóng gói, hoàn thiện tài liệu và bàn giao

**Cùng phụ trách. Trạng thái: chưa bắt đầu.**

- [ ] Kiệt phụ trách hướng dẫn sử dụng, tài liệu chức năng và báo cáo.
- [ ] Cùng Hoàng thử bản đóng gói, chuẩn bị demo và bàn giao.
