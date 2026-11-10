# Nhật ký Bước 04 — Xây dựng DALC

## 2026-11-10 — Chuẩn bị thiết kế, chưa lập trình DALC

**Trạng thái lúc lập mục này:** Đang chuẩn bị, chưa có cặp file DALC. Hiện đã có bản nháp `MovieDALC.h/.cpp` trong `core/dal/dalc/`, nhưng chưa có bằng chứng nghiệm thu hoặc kiểm thử Bước 04. [Tổng quan tiến độ](TongQuanTienDo.md) và [phân công](PhanCongCongViec.md) là nguồn theo dõi chung; tài liệu thiết kế nằm ở [dalc.md](../CauTrucThuMuc/dal/dalc.md).

## 1. Mục tiêu và ranh giới

Bước 04 xây dựng tầng truy cập dữ liệu cho tám Model đã có: đọc file TXT, kiểm tra và chuyển bản ghi, giữ tập object trong bộ nhớ, tra cứu theo mã, dựng quan hệ, chuẩn bị nội dung ghi và phối hợp lưu dữ liệu. DALC là **Data Access Logic Components**, đặt dự kiến ở `core/dal/dalc/`.

Kiến trúc bắt buộc là **GUI → BLL → DAL → DB**; DAL gồm Model và DALC, DB là `data/*.txt`. GUI gọi BLL; BLL quyết định quyền và quy tắc nghiệp vụ rồi gọi DALC; DALC dùng tiện ích file để truy cập DB. DALC kiểm tra định dạng, khóa, tham chiếu và tính nhất quán dữ liệu, nhưng không quyết định giá, ưu đãi, quyền bán/hủy hoặc điều kiện lịch chiếu.

## 2. Phân công hiện hành

| Người phụ trách | DALC dự kiến | Phạm vi dữ liệu |
| --- | --- | --- |
| Hoàng | `MovieDALC`, `RoomDALC`, `SeatDALC`, `ShowtimeDALC` | Phim, phòng, ghế và suất chiếu; liên kết giữa bốn nhóm Model. |
| Kiệt | `UserDALC`, `MemberDALC`, `TicketDALC`, `InvoiceDALC`, `SequenceDALC`, `BookingDALC` | Tài khoản, thành viên, vé, hóa đơn, bộ đếm và phối hợp ghi giao dịch bán vé. |

Phân công giữ nguyên theo [PhanCongCongViec.md](PhanCongCongViec.md). Hai người cần thống nhất API dùng chung và cùng kiểm thử các liên kết cắt qua phạm vi phụ trách.

## 3. Checklist công việc

### Chung

- [ ] Chốt hợp đồng API giữa các DALC và BLL: kiểu kết quả/lỗi, đọc nhiều bản ghi, tra mã, cập nhật, chuẩn bị giao dịch và thời điểm cập nhật bộ nhớ.
- [ ] Chốt chủ sở hữu object, địa chỉ ổn định, thứ tự nạp/gỡ quan hệ và cách xử lý copy/move của Model cùng `DoublyLinkedList<T>`.
- [ ] Chốt cách parse/ghi đúng [data_format.md](../TongQuan/data_format.md): mọi file TXT dữ liệu, kể cả `sequences.txt`, không có dòng schema hoặc tiêu đề.
- [ ] Chốt cách đổi `IdGenerator::updatedSequences` (`std::string`) thành các dòng cho `TransactionFile::newLines` (`std::vector<std::string>`) mà không sinh dòng rỗng dư.
- [ ] Tích hợp đọc/ghi qua `FileHandler`, lưu nhiều file qua `TransactionManager`, phục hồi trước khi nạp dữ liệu; làm rõ cách xử lý lỗi và trạng thái bộ nhớ khi lưu thất bại.
- [ ] Review phần giao nhau, biên dịch C++17 và chạy kiểm thử tích hợp hai phần sau khi có mã.

### Hoàng

- [ ] Đối chiếu API và quan hệ hiện có của `Movie`, `Room`, `Seat`, `Showtime` trước khi thiết kế bốn DALC.
- [ ] Thiết kế API và cách giữ/tra cứu object cho `MovieDALC`, `RoomDALC`, `SeatDALC`, `ShowtimeDALC`.
- [ ] Đọc, kiểm tra, chuyển đổi và chuẩn bị ghi `movies.txt`, `rooms.txt`, `seats.txt`, `showtimes.txt` không có dòng schema; báo rõ bản ghi sai và mã tham chiếu thiếu.
- [ ] Dựng và kiểm tra liên kết phim–suất, phòng–ghế, phòng–suất; thử nạp lại và xử lý xóa/cập nhật không để con trỏ treo.
- [ ] Kiểm thử CRUD dữ liệu, trùng ID/vị trí, tham chiếu sai, lỗi đọc/ghi và tích hợp với phần Kiệt.

### Kiệt

- [ ] Đối chiếu API và quan hệ hiện có của `User`, `Member`, `Ticket`, `Invoice` cùng `IdGenerator` trước khi thiết kế sáu DALC.
- [ ] Thiết kế API và cách giữ/tra cứu object cho `UserDALC`, `MemberDALC`, `TicketDALC`, `InvoiceDALC`, `SequenceDALC`, `BookingDALC`.
- [ ] Đọc, kiểm tra, chuyển đổi và chuẩn bị ghi `users.txt`, `members.txt`, `tickets.txt`, `invoices.txt` không có dòng schema; đọc/ghi `sequences.txt` theo dạng `entity|nextValue`.
- [ ] Dựng quan hệ hóa đơn–người bán/thành viên/suất và vé–hóa đơn/suất/ghế; giữ vé/hóa đơn lịch sử.
- [ ] Phối hợp nội dung ghi vé, hóa đơn, thành viên và bộ đếm trong một giao dịch kỹ thuật; để BLL quyết định điều kiện bán, tiền và điểm.
- [ ] Kiểm thử mã trùng, bộ đếm sai/hết mã, tham chiếu sai, ghi nhiều file thất bại và tích hợp với phần Hoàng.

## 4. API, Model và phụ thuộc cần thống nhất

`MovieDALC` hiện có API nháp; API chung giữa các DALC chưa chốt. Trước khi viết các DALC còn lại, cần quyết định nơi sở hữu tám loại object; cách trả tham chiếu/con trỏ mà không cho bên ngoài vô tình phá chỉ mục; quy ước báo lỗi gồm đường dẫn, số dòng và nguyên nhân; phạm vi một thao tác nạp; và cách bảo đảm cập nhật file cùng bộ nhớ không lệch nhau. Cần đối chiếu từng hàm `link...` trong header/implementation Model hiện tại, vì [dal/models.md](../CauTrucThuMuc/dal/models.md) là **đề xuất cũ**, không phải bằng chứng mọi quan hệ đã có.

Các phụ thuộc đã tồn tại: `FileHandler::readlines/writeLines` làm I/O dòng; `IdGenerator::generateNextId` chỉ trả mã và chuỗi bộ đếm mới; `TransactionManager::execute/recover` xử lý nhóm file; `DateTime` hiện chỉ có `getCurrentDateTime`; `DoublyLinkedList<T>` có các thao tác node cơ bản nhưng không có ngữ nghĩa copy an toàn cho danh sách sở hữu node. Không tự thêm API Model hoặc đổi thuật toán DLL trong bước chuẩn bị tài liệu này.

## 5. Kiến thức cần tìm hiểu

- Parse CSV đơn giản theo dấu `|` nhưng giữ trường rỗng cuối dòng, kiểm số lượng trường, enum, boolean, số nguyên, số thực và UTF-8 theo tài liệu dữ liệu.
- Quản lý vòng đời C++: chủ sở hữu duy nhất, `std::unique_ptr` hoặc lựa chọn khác giữ địa chỉ object ổn định, tham chiếu không sở hữu và gỡ liên kết trước khi giải phóng.
- Chỉ mục STL theo ID/username, cập nhật chỉ mục khi sửa dữ liệu, và giới hạn của `DoublyLinkedList<T>` khi duyệt hoặc sao chép.
- Phục hồi giao dịch nhiều file, cách chuẩn bị bản ghi trước khi commit, rollback, lỗi I/O và kiểm tra dữ liệu lúc khởi động.

## 6. Lỗi, rủi ro và vướng mắc đang mở

- `AGENTS.md` không có trong cây làm việc khi rà soát; quy trình Bước 04 dựa trên phân công và các tài liệu hiện có.
- Đường dẫn `docs/CauTrucThuMuc/models.md` được nhắc ở một số tài liệu cũ không tồn tại; file hiện có là `docs/CauTrucThuMuc/dal/models.md` và nội dung của nó cần đối chiếu với mã Model hiện tại.
- Quyết định hiện tại: mọi file TXT dữ liệu, kể cả `sequences.txt`, không có `#schema=1`. Mục này cập nhật quy ước cũ chỉ miễn schema cho `sequences.txt`. Chưa có DALC thực hiện chuyển `updatedSequences` sang `TransactionFile::newLines`; cần chốt hợp đồng, không thay quy tắc cấp ID.
- `MovieDALC::load()` hiện bỏ qua dòng đầu vì giả định đó là schema, nên sẽ bỏ mất bản ghi đầu tiên của `movies.txt` theo định dạng mới. Đây là vấn đề ở mã hiện có; chưa sửa code trong lần cập nhật tài liệu này.
- Các Model giữ con trỏ qua lại; sao chép object hoặc danh sách DLL theo giá trị, nạp lại hoặc xóa khi còn tham chiếu có thể tạo con trỏ treo hoặc xóa lặp. API gỡ liên kết chưa được chốt.
- `TransactionManager` hiện thay file bằng sao chép từng file và chưa chứng minh bền vững khi mất điện; bản thử `-DTEST_ROLLBACK` chưa có kết quả riêng trong nhật ký Bước 03. Cần bảo toàn lỗi và không báo thành công sớm.
- Thứ tự nạp, chính sách khi thiếu file/tham chiếu và quyền sở hữu tập object giữa 10 DALC chưa được thống nhất. Không được âm thầm bỏ bản ghi lỗi rồi ghi đè dữ liệu.

## 7. Kiểm thử cần thực hiện

- [ ] Đọc bộ dữ liệu hợp lệ không có dòng schema ở bất kỳ file TXT nào; ghi rồi nạp lại giữ nguyên bản ghi đầu tiên, dữ liệu và quan hệ.
- [ ] Từ chối file thiếu, dòng schema/tiêu đề không hợp lệ, dòng sai số trường/kiểu/enum, mã trùng và bộ đếm sai; báo đúng file/dòng.
- [ ] Từ chối tham chiếu thiếu hoặc không khớp: ghế–phòng, suất–phim/phòng, hóa đơn–người bán/suất/thành viên, vé–hóa đơn/suất/ghế.
- [ ] Thử quan hệ hai chiều hiện có và chính sách truy vấn cho chiều chưa có; thử nạp lại/xóa/cập nhật không làm hỏng chỉ mục hoặc con trỏ.
- [ ] Thử giao dịch tạo dữ liệu kèm tăng bộ đếm, lỗi ghi giữa chừng, phục hồi PREPARED/COMMITTED, và trạng thái bộ nhớ sau thất bại.
- [ ] Biên dịch và chạy thử tích hợp C++17 cho phần Hoàng, phần Kiệt và cả hai cùng nhau; ghi lệnh/kết quả thực tế vào nhật ký sau khi có mã.

## 8. Điều kiện nghiệm thu và bước tiếp theo

Bước 04 chỉ hoàn thành khi 10 DALC theo phân công thực sự có mã, đọc/ghi đúng định dạng, kiểm tra tham chiếu và giữ object an toàn; phần riêng của cả hai và kiểm thử tích hợp có bằng chứng; lỗi nghiêm trọng về dữ liệu/giao dịch đã được xử lý hoặc nêu rõ giới hạn chấp nhận. Checklist ở trên hiện chưa được đánh dấu hoàn thành.

**Phần tiếp theo:**

- [ ] Hoàng và Kiệt thống nhất API, quyền sở hữu object, lỗi nạp dữ liệu và giao dịch trước khi viết mã.
- [ ] Triển khai theo phần được phân công, ghi vấn đề và lệnh/kết quả kiểm thử vào nhật ký này khi công việc thực sự diễn ra.
- [ ] Sau nghiệm thu Bước 04, tiếp tục Bước 05 theo lộ trình; không chuyển trạng thái các bước sau chỉ vì đã viết tài liệu này.
