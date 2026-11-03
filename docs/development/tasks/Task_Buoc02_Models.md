# Hướng dẫn phần việc của Kiệt — Bước 02

Tài liệu này chỉ diễn giải phần việc **Kiệt** phụ trách trong Bước 02; không bao gồm các đầu việc triển khai Model của Hoàng. Đây là hướng dẫn công việc, không phải xác nhận các mục đã hoàn thành. Checklist và trạng thái chính thức vẫn được ghi trong [Task_Kiet.md](./Task_Kiet.md) và [TongQuanTienDo.md](../TongQuanTienDo.md).

## Mục tiêu

Hoàn thiện bốn Model thuộc phạm vi Kiệt — `User`, `Member`, `Ticket`, `Invoice` — và làm rõ quan hệ của chúng, để có thể tạo, nối, sử dụng và kiểm tra các đối tượng an toàn. Model chỉ giữ dữ liệu và quan hệ; quyền, giá vé, điều kiện bán/hủy thuộc BLL ở các bước sau.

## Các việc cần làm

### 1. Đối chiếu bốn Model với schema

- [x] Rà soát các cặp header/implementation của `User`, `Member`, `Ticket`, `Invoice`; constructor, getter và dữ liệu lưu trong Model nhất quán.
- [x] Đối chiếu với schema trong `docs/TongQuan/data_format.md`: `User` có đủ 6 trường; `Member` đủ 4; `Invoice` đủ 6; `Ticket` đủ 14 trường theo đúng tên và thứ tự dữ liệu.
- [x] Xác nhận kiểu/enum thống nhất: `UserRole` là ADMIN/STAFF; `TicketType` là REGULAR/STUDENT; `TicketStatus` là SOLD/CANCELLED; tiền dùng `double`, điểm dùng `int`, cờ `active` dùng `bool`.
- [x] Xác nhận các con trỏ `Ticket.showtime/seat` và `Invoice.showtime/seller/member` là liên kết runtime, không phải trường ghi xuống TXT. `memberId` rỗng vẫn biểu diễn hóa đơn không gắn thành viên.

**Kết quả đối chiếu (2026-11-03):** Không phát hiện trường, kiểu dữ liệu hoặc enum nào lệch schema trong bốn Model của Kiệt. Schema cũng quy định các ràng buộc như username/phone duy nhất, điểm và tiền không âm, `paidAmount = basePrice - discountAmount`, dữ liệu hủy hợp lệ, hóa đơn có ít nhất một vé và tổng hóa đơn bằng tổng tiền vé. Các ràng buộc này chưa được Model tự kiểm tra; chúng thuộc Validator/DALC/BLL theo kiến trúc và sẽ được xem xét ở các việc sau. `Member::setPoints(const int&)` nhận đúng kiểu dữ liệu điểm nhưng dùng tham chiếu const cho `int` là không cần thiết; đây chỉ là nhận xét API, không phải sai lệch schema và chưa thay đổi code.

## Thứ tự thực hiện

Tham khảo cách các Model hiện có của Hoàng khai báo liên kết bằng forward declaration, con trỏ, `DoublyLinkedList` và `friend class`. Trước khi bắt đầu mỗi bước, đọc lại đúng các file hiện tại; giữ nguyên phần không liên quan. Đây là lộ trình hướng dẫn từng bước: **chỉ làm bước đang được yêu cầu, dừng lại sau khi kiểm tra xong và chờ yêu cầu tiếp theo**.

### Bước 1 — Bổ sung chiều `Ticket → Invoice`

Phạm vi file: `core/dal/models/Ticket.h/.cpp`.

- [x] Forward-declare `Invoice` trong header.
- [x] Thêm `Invoice* invoice = nullptr` vào `Ticket` dưới dạng tham chiếu không sở hữu.
- [x] Khai báo getter `getInvoice()` và `friend class Invoice` để `Invoice` có thể cập nhật liên kết riêng tư khi nối vé.
- [x] Định nghĩa getter trong `.cpp`; giữ nguyên constructor, các trường ID và API khác.
- [x] Kiểm tra cú pháp C++17 cho toàn bộ Model.

**Kết quả (2026-11-03):** Đã thêm forward declaration, con trỏ mặc định null, getter và `friend class Invoice` trong `Ticket.h/.cpp`. Đã chạy `g++ -std=c++17 -Wall -Wextra -pedantic-errors -fsyntax-only core/dal/models/*.cpp`: đạt. Constructor và trường `invoiceId` giữ nguyên. Bước này chỉ thêm chiều ngược; liên kết hai chiều hoạt động sau khi hoàn thành Bước 2.

### Bước 2 — Nối đồng bộ `Invoice ↔ Ticket`

Phạm vi file: `core/dal/models/Invoice.h/.cpp` và `Ticket.h/.cpp` nếu cần.

- [x] Sửa `Invoice::linkTicket()` để kiểm tra con trỏ khác null, `ticket.invoiceId == invoice.invoiceId`, `ticket.showtimeId == invoice.showtimeId`, vé không bị trùng và chưa thuộc Invoice khác.
- [x] Chỉ sau khi kiểm tra thành công, thêm vé vào `Invoice.tickets` và đặt `Ticket.invoice = this`. Gọi link lặp với cùng cặp object không tạo node trùng.
- [x] Không để `Invoice` sở hữu hay `delete` `Ticket`; quan hệ là hai con trỏ tham chiếu không sở hữu.
- [x] Kiểm tra cú pháp toàn bộ Model và chạy test tập trung các tình huống liên kết.

**Kết quả (2026-11-03):** `Invoice::linkTicket()` xác thực ID hóa đơn và suất chiếu; từ chối null, vé có cùng `ticketId` nhưng là object khác, hoặc vé đang thuộc Invoice khác. Nối lặp cùng object không tạo node trùng. Thêm node trước khi gán `Ticket.invoice`, nên nếu cấp phát node thất bại thì chưa tạo liên kết một phía. Không thêm API unlink vì hướng dẫn hiện tại chỉ yêu cầu nối hai chiều. Đã chạy `g++ -std=c++17 -Wall -Wextra -pedantic-errors -fsyntax-only core/dal/models/*.cpp` và test runtime các trường hợp null, ID sai, suất sai, link lặp, ID vé trùng và liên kết chéo: đều đạt.

### Bước 3 — Bổ sung chiều `User ↔ Invoice`

Phạm vi file: `core/dal/models/User.h/.cpp` và `Invoice.h/.cpp`.

- [x] Thêm danh sách `Invoice*` không sở hữu vào `User`, API đọc danh sách và `friend class Invoice` để `Invoice::linkSeller()` cập nhật danh sách.
- [x] Khi nối Seller, kiểm tra `invoice.staffId == user.userId`; nối cùng một cặp không thêm trùng; từ chối mã sai hoặc liên kết chéo.
- [x] Xác nhận tài khoản bị vô hiệu hóa vẫn giữ lịch sử hóa đơn.
- [x] Cấm copy/move `User` để tránh shallow-copy DLL và giữ địa chỉ ổn định.
- [x] Biên dịch toàn bộ Model và chạy test tập trung liên kết.

**Kết quả (2026-11-03):** `User` có `DoublyLinkedList<Invoice*>`, `getInvoiceCount()`/`getInvoiceAt()` và hàm nội bộ thêm danh sách chỉ cho `Invoice` gọi. `Invoice::linkSeller()` kiểm tra ID và xung đột, nối lặp cùng cặp không thêm node trùng. `User` không copy/move được vì DLL sở hữu node bằng con trỏ thô. Đã chạy kiểm tra cú pháp và test runtime null, ID sai, link lặp, ID hóa đơn trùng, liên kết chéo, chỉ số ngoài phạm vi và giữ lịch sử khi User inactive: đều đạt. Không thêm API unlink.

### Bước 4 — Bổ sung chiều `Member ↔ Invoice`

Phạm vi file: `core/dal/models/Member.h/.cpp` và `Invoice.h/.cpp`.

- [x] Thêm danh sách `Invoice*` không sở hữu vào `Member`, API đọc danh sách và `friend class Invoice`.
- [x] Với hóa đơn có `memberId`, kiểm tra ID khớp rồi cập nhật cả `Member.invoices` và `Invoice.member`; ngăn liên kết trùng/chéo.
- [x] Với `memberId` rỗng, không nối Member và không đưa hóa đơn vào danh sách của Member.
- [x] Chỉ triển khai thao tác nối theo hướng dẫn; không tự thêm API unlink.

**Kết quả (2026-11-03):** `Member` có `DoublyLinkedList<Invoice*>`, `getInvoiceCount()`/`getInvoiceAt()` và hàm nội bộ thêm danh sách chỉ cho `Invoice` gọi. `Invoice::linkMember()` kiểm tra ID và xung đột, sau đó nối hai phía; link lặp cùng cặp không thêm node trùng. Hóa đơn không có `memberId` tiếp tục không liên kết Member. `Member` không copy/move được vì DLL sở hữu node bằng con trỏ thô. Đã kiểm tra cú pháp toàn bộ Model và chạy test tập trung liên kết Member–Invoice.

### Bước 5 — Phối hợp quan hệ với Model của Hoàng

Các quan hệ cần thống nhất API và cách khởi tạo cùng Hoàng:

- `Showtime ↔ Ticket`
- `Showtime ↔ Invoice`
- `Seat ↔ Ticket`

Kiệt chỉ sửa phần `Ticket`/`Invoice` thuộc phạm vi mình; không tự ý sửa `Showtime`, `Seat` hoặc checklist của Hoàng. Xác nhận rõ ai cập nhật từng đầu và điều kiện ID/phòng/suất phải khớp.

### Bước 6 — Kiểm thử, rà soát vòng đời và ghi kết quả

- [x] Tạo và chạy test có liên kết cả bốn Model của Kiệt; kiểm tra constructor/getter và cả hai chiều của ba quan hệ.
- [x] Thử null, ID hóa đơn/nhân viên/thành viên/suất không khớp, bản ghi trùng, một Ticket nối nhầm Invoice, nhiều vé trên một Invoice, hóa đơn khách vãng lai và link lặp.
- [x] Kiểm tra `Ticket::markCancelled(...)` giữ nguyên `pointsEarned`, cập nhật đúng trạng thái/thông tin hủy/hoàn tiền; xác nhận tổng tiền ban đầu của Invoice không đổi khi hủy vé.
- [ ] Rà soát vòng đời các Model chứa DLL: `User`/`Member` cấm copy/move; `Invoice` vẫn dùng thao tác mặc định nên còn rủi ro shallow-copy chưa xử lý.
- [x] Chạy kiểm tra cú pháp C++17, liên kết/chạy test và ghi kết quả vào nhật ký Bước 02.
- [x] Cập nhật checklist Kiệt; review/tích hợp với Hoàng vẫn để chờ, không tính là đã hoàn thành khi Bước 5 bị bỏ qua.

**Kết quả (2026-11-03):** Thêm `tests/model_relations_test.cpp`, kiểm thử constructor/getter và ba liên kết `Invoice ↔ Ticket`, `User ↔ Invoice`, `Member ↔ Invoice`; gồm null, ID không khớp, liên kết trùng/chéo, nhiều vé, khách vãng lai, User inactive và index ngoài phạm vi. Kiểm tra hủy vé giữ nguyên điểm, cập nhật metadata/trạng thái/refund và không đổi tổng Invoice. `User`/`Member` cấm copy/move. `Invoice` hiện vẫn có thao tác copy/move mặc định; do đó rủi ro shallow-copy DLL còn mở và không được đánh dấu hoàn tất. Cả `g++ -std=c++17 -Wall -Wextra -pedantic-errors -fsyntax-only core/dal/models/*.cpp` và `g++ -std=c++17 -Wall -Wextra -pedantic-errors -I. tests/model_relations_test.cpp core/dal/models/*.cpp -o <temp-exe>` rồi chạy executable đều đạt. Các con trỏ quan hệ là không sở hữu; chưa thấy DAL/Repository tạo và giữ các Model trong `core`, nên vòng đời khi tích hợp thực tế chưa thể xác nhận. Bước 5 được bỏ qua theo yêu cầu và việc review/tích hợp với Hoàng còn chờ.

## Quy tắc quan hệ và dữ liệu

- ID là nguồn chuẩn lưu trong DATA; con trỏ và danh sách chỉ là quan hệ runtime, không ghi địa chỉ con trỏ xuống file.
- Tất cả con trỏ/list trong các Model này là **không sở hữu**; object được giữ bởi nơi lưu trữ/DAL.
- Khi nối quan hệ hai chiều, xác thực ID và cập nhật cả hai đầu; rà soát vòng đời object để tránh con trỏ treo.
- BLL/Service mới chịu trách nhiệm quyền, điều kiện bán/hủy, và kiểm tra điểm không âm/không tràn. Model không thay thế kiểm tra nghiệp vụ.
- Bước 01 hiện tạm bỏ qua theo yêu cầu; việc đó không có nghĩa Bước 01 hoàn thành.

## Điều kiện hoàn thành

- Ba quan hệ `Invoice ↔ Ticket`, `User ↔ Invoice`, `Member ↔ Invoice` nhất quán hai chiều; kiểm tra mã đúng, không có liên kết trùng/sai.
- Các quan hệ dùng với `Showtime`/`Seat` được phối hợp và xác nhận với Hoàng (chưa hoàn thành vì bỏ qua Bước 5).
- Vòng đời object và hành vi copy/move của các Model chứa DLL không gây con trỏ treo hoặc giải phóng lặp.
- Trạng thái vé và cách giữ điểm nhất quán với quy tắc nghiệp vụ.
- Các `.cpp` thuộc phạm vi Kiệt biên dịch C++17; chương trình kiểm thử phần Kiệt liên kết và chạy đạt.
- Các điểm giao tiếp với Model ngoài phạm vi đã được xác nhận; checklist và nhật ký Kiệt được cập nhật.

## Tài liệu tham khảo

- [Phân công công việc](../PhanCongCongViec.md)
- [Checklist của Kiệt](./Task_Kiet.md)
- [Nhật ký Bước 02](../Buoc02_XayDungModels.md)
- [Đề xuất quan hệ Model](../../CauTrucThuMuc/models.md)
- [Định dạng dữ liệu](../../TongQuan/data_format.md)
- [Quy tắc nghiệp vụ](../../TongQuan/business_rules.md)
