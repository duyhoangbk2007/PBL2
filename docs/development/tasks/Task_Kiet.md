# Task của Kiệt — lộ trình PBL2 14 bước

Theo [phân công chính](../PhanCongCongViec.md), Kiệt phụ trách toàn bộ GUI Qt và tích hợp giao diện với Backend từ Bước 12. Các đóng góp Backend của Kiệt ở Bước 01–03 được giữ nguyên như lịch sử; Backend hiện hành do Hoàng phụ trách. [Tổng quan tiến độ](../TongQuanTienDo.md) ghi trạng thái chung; file này ghi checklist riêng của Kiệt. **Theo xác nhận của nhóm, Bước 01–03 đã hoàn thành.** Các checkbox và nhận xét trong ba mục đó là bằng chứng/lịch sử tại thời điểm ghi, không được tự đổi thành kết quả kiểm thử mới. Mã Model hiện hành ở `src/models/`.

**Quy tắc đồng bộ:** Sau khi hoàn thành một task, cập nhật file này và nhật ký bước nếu đã có. Chỉ đánh dấu bước chung hoàn thành ở `TongQuanTienDo.md` khi phần việc của bước đó có kiểm thử và nghiệm thu phù hợp; việc đổi phân công không làm thay đổi checkbox lịch sử. Khi tiếp tục Bước XX của Kiệt, đọc mục tương ứng, nhật ký bước và mã liên quan rồi chọn checkbox chưa hoàn thành tiếp theo. Không sửa Task của Hoàng khi chỉ làm phần Kiệt.

### Bước 01 – Khởi tạo dự án

**Trạng thái hiện hành:** Đã hoàn thành theo xác nhận nhóm. **Tại mốc ghi checklist cũ:** Đang thực hiện.

**Branch:** `feature/b02-hoang-models-relation`

**Checklist công việc:**

- [x] Đối chiếu README và tài liệu hiện có với cấu trúc mới.
- [x] Có `.gitignore` bỏ qua executable, object, thư mục build CMake và file tạm (commit `bf171f9`).

**Điều kiện hoàn thành:** README, Git và tài liệu hướng dẫn khởi tạo nhất quán, có thể làm theo. Có bằng chứng biên dịch/chạy thử hoặc kiểm thử phù hợp và không còn lỗi nghiêm trọng chưa xử lý.

**Pull Request:** Chưa có

**Ghi chú:** `README.md` và nhật ký đã có; `.gitignore` được thêm ở commit `bf171f9`, đồng thời `integration_test.exe` và `pbl2_test.exe` được bỏ theo dõi. `AGENTS.md` chưa có. Không coi việc có README là đã hoàn thiện hướng dẫn build.

### Bước 02 – Xây dựng Models và quan hệ Model

**Trạng thái hiện hành:** Đã hoàn thành theo xác nhận nhóm. **Tại mốc ghi checklist cũ:** Đang thực hiện.

**Branch:** `feature/b02-hoang-models-relation`

**Checklist công việc:**

- [x] Có đủ `User`, `Member`, `Ticket`, `Invoice` ở `src/models/`.
- [x] `Member::setPoints()` và `Ticket::markCancelled()` hiện có trong mã.
- [x] Biên dịch cú pháp toàn bộ tám Model bằng C++17: đạt ở lần rà soát này.
- [x] Nối các quan hệ `Invoice`–`Ticket`, `User/Member`–`Invoice`.
- [x] Kiểm tra trạng thái vé, điểm và tính nhất quán sau khi nối quan hệ.

**Điều kiện hoàn thành:** Bốn Model tài khoản/giao dịch có quan hệ nhất quán và được thử tích hợp với bốn Model của Hoàng. Có bằng chứng biên dịch/chạy thử hoặc kiểm thử phù hợp và không còn lỗi nghiêm trọng chưa xử lý.

**Pull Request:** [PR #4](https://github.com/duyhoangbk2007/PBL2/pull/4), `feature/b02-kiet-model-relations` → `develop`: đang mở, chưa merge (kiểm tra GitHub 2026-11-06).

**Ghi chú:** `Invoice`–`Ticket`, `User`–`Invoice`, `Member`–`Invoice` đã có liên kết hai chiều; `Invoice::linkMember()` đã xử lý khách vãng lai và liên kết Member theo ID. `Invoice`–`Showtime`, `Ticket`–`Showtime` và `Ticket`–`Seat` hiện là một chiều. Người thực hiện dự án chủ động không dùng `= delete` trong cả tám Model; việc quản lý vòng đời object và gỡ liên kết sẽ xem xét ở DAL/Repository. Nhật ký cũ ghi test C++17 đã chạy đạt, nhưng `tests/model_relations_test.cpp` không có trong cây hiện tại; mục kiểm thử tích hợp vẫn chưa hoàn thành ở trạng thái hiện nay.

### Bước 03 – Xây dựng tiện ích lưu trữ dữ liệu ban đầu

**Trạng thái hiện hành:** Đã hoàn thành theo xác nhận nhóm. `IdGenerator` và `DateTime` đã có; hiện `DateTime` còn có hàm cộng phút và so sánh thời gian. Các checkbox dưới đây ghi đóng góp Backend lịch sử của Kiệt, không phải việc Backend mới được giao.

**Branch:** Mã hiện có trên nhánh `feature/b03-hoang-storage`; chưa xác nhận nhánh/PR riêng của Kiệt.

**Checklist công việc:**

- [x] Cài `IdGenerator` đã có đóng góp lịch sử; API hiện hành nhận các ID đang nạp và trả mã lớn nhất cộng một, không dùng `sequences.txt`.
- [x] Cài `DateTime::getCurrentDateTime()` lấy và định dạng thời gian hiện tại.
- [x] Kiểm thử cơ bản `IdGenerator` và định dạng đầu ra `DateTime` trong `test_all_utils.cpp`; lần chạy sau chuyển thư mục có 4 PASS, 0 FAILED cho toàn bộ bài thử.

**Điều kiện hoàn thành:** Mã và ngày giờ đúng định dạng. Có bằng chứng biên dịch/chạy thử hoặc kiểm thử phù hợp và không còn lỗi nghiêm trọng chưa xử lý.

**Pull Request:** Chưa có

**Ghi chú:** Kiệt phụ trách `IdGenerator` và `DateTime`, phối hợp với `FileIO` của Hoàng. Ghi chú về bộ đếm thuộc thiết kế cũ; API hiện hành không dùng `sequences.txt` và Service cấp mã từ các ID đang nạp. Tại thời điểm ghi chú cũ, `DateTime` mới có `getCurrentDateTime()`; hiện mã còn có `addMinutes()`, `isBefore()` và `isOverlapping()`. Phần kiểm tra Backend tiếp theo thuộc Hoàng. Review và tích hợp chính thức chưa được xác nhận. `Validator` hoãn sang Repository, chỉ cài hàm kiểm tra TXT thật sự cần dùng.

## Công việc hiện hành: GUI Bước 12–14

Các Bước 04–11 là Backend do Hoàng phụ trách; yêu cầu hiển thị và hợp đồng Service được phối hợp khi triển khai GUI. Bước 06 hiện có `ShowtimeService` và `test_showtime_service.cpp` đạt 89/89, nhưng hủy/xóa suất vẫn trả `false` để chờ `TicketRepository`. Ở Bước 08, `MemberRepository` đạt **63 PASS, 0 FAIL** và `MemberService` đạt **69 PASS, 0 FAIL** cho phạm vi hiện có; Kiệt dùng [Business–UI Contract](../../Business_UI_Contract.md#8-memberservice--quản-lý-thành-viên) khi tích hợp giao diện. Các kết quả Backend này không đánh dấu hoàn thành task GUI của Kiệt.

`PricingService` ở Bước 09 đã có bộ kiểm thử chung đạt **63 PASS, 0 FAIL** cho Model/Repository/Service. Luồng bán vé và ghi điểm thành viên chưa tích hợp; các task GUI vẫn để mở.

### Bước 12 – Xây dựng giao diện Qt

**Phụ trách toàn bộ GUI: Kiệt. Trạng thái: đang thực hiện theo trao đổi từ Kiệt; phần bán vé chờ chốt API Business.**

- [ ] Xây dựng `MainWindow`, dashboard Admin/Staff, `LoginView`, `MovieManagementView`, `RoomManagementView`, `ShowtimeManagementView`, `StaffManagementView`, `MemberManagementView`, `BookingView`, `SeatMapView`, `InvoiceHistoryView` và `ReportView` theo phạm vi chức năng đã thống nhất.
- [ ] Tích hợp các View với Service do Hoàng cung cấp; GUI chỉ hiển thị và nhận thao tác, không xử lý quyền hoặc quy tắc nghiệp vụ thay Backend.
- [ ] Cấu hình và kiểm tra build GUI Qt, trạng thái màn hình và thông báo lỗi.

### Bước 13 – Kiểm thử tích hợp và sửa lỗi

**Kiệt chủ trì GUI và tích hợp Backend; Hoàng xử lý lỗi Backend. Trạng thái: chưa bắt đầu.**

- [ ] Kiểm thử luồng đăng nhập, quản trị, chọn/bán/hủy vé, lịch sử và báo cáo qua GUI → Service → Repository → TXT.
- [ ] Ghi trường hợp lỗi có thể tái hiện; sửa lỗi GUI, phối hợp Hoàng sửa lỗi Backend, chạy lại luồng bị ảnh hưởng.

### Bước 14 – Đóng gói, hoàn thiện tài liệu và bàn giao

**Kiệt phụ trách GUI và bàn giao; Hoàng hỗ trợ Backend. Trạng thái: chưa bắt đầu.**

- [ ] Đóng gói ứng dụng GUI, viết hướng dẫn sử dụng và tài liệu màn hình; chuẩn bị báo cáo, demo và bàn giao.
- [ ] Phối hợp Hoàng kiểm tra build và các luồng tích hợp trong bản bàn giao.
