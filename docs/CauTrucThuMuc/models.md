# Đề xuất quan hệ hai chiều giữa các Model

Tài liệu này là **ý tưởng chỉnh sửa**, chưa phải mô tả những hàm/con trỏ đã cài đầy đủ. Tám Model hiện ở `core/dal/models/`. Hiện `Room` đã giữ danh sách `Seat*`, nhưng `Seat` mới giữ `roomId`, chưa giữ `Room*`. Tương tự, một số lớp đã có con trỏ chiều từ bản ghi con tới bản ghi cha, còn chiều ngược lại chưa đầy đủ. Lần này chỉ chốt thiết kế trên tài liệu, chưa sửa `.h/.cpp`, DLL hoặc định dạng TXT.

## 1. Nguyên tắc chung

- **Hai chiều truy cập:** nếu cần đi từ A sang B và từ B về A, mỗi phía có một tham chiếu phù hợp. Quan hệ 1–N dùng danh sách ở phía 1 và một con trỏ ở phía N. Quan hệ tùy chọn cho phép `nullptr`.
- **Một chủ sở hữu:** danh sách/con trỏ trong Model chỉ là **tham chiếu không sở hữu**. DALC hoặc một kho đối tượng chung giữ vòng đời các object thật. Hủy Model không `delete` object ở đầu kia. Việc `Invoice` chứa nhiều vé là quan hệ nghiệp vụ, chưa đồng nghĩa `Invoice` sở hữu bộ nhớ của `Ticket`.
- **Mã vẫn là nguồn lưu trữ:** các trường `roomId`, `movieId`, `invoiceId`... tiếp tục ghi vào TXT. Con trỏ và danh sách chỉ dựng trong RAM sau khi đọc file; không ghi địa chỉ con trỏ xuống TXT.
- **Một thao tác nối cho cả hai phía:** lớp điều phối quan hệ ở DALC kiểm tra mã, tránh lặp, rồi cập nhật cả hai đầu. Không để code bên ngoài tự gán từng con trỏ hoặc thêm vào danh sách riêng lẻ. Khi gỡ/xóa/tải lại, DALC gỡ cả hai phía trước khi hủy object.
- **Địa chỉ ổn định:** object đã được lớp khác trỏ tới không được di chuyển tùy ý. Cần chọn nơi lưu giữ có địa chỉ ổn định và quy định copy/move. `DoublyLinkedList<T>` hiện sở hữu node bằng con trỏ thô nhưng chưa có ngữ nghĩa sao chép an toàn; không sao chép Model chứa DLL theo giá trị khi chưa xử lý vấn đề này.
- **Không đưa nghiệp vụ vào Model:** Model chỉ giữ và đọc quan hệ nội tại. Quyền, giá, điều kiện bán/hủy vẫn thuộc BLL; DALC quản lý bản ghi và dựng/gỡ quan hệ.

## 2. Bảng quan hệ cần hỗ trợ hai chiều

Ký hiệu `DoublyLinkedList<X*>` trong bảng là **đề xuất cấu trúc tham chiếu**, chưa phải yêu cầu cài ngay cho mọi quan hệ. Với tập rất lớn hoặc chỉ cần tra cứu theo mã, DALC có thể cung cấp chiều ngược qua chỉ mục/truy vấn; cần xác nhận với giảng viên nếu yêu cầu “hai chiều” bắt buộc nằm trực tiếp trong từng class.

| Quan hệ | Số lượng | Chiều từ phía 1 | Chiều ngược từ phía N | Hiện trạng và bổ sung đề xuất |
| --- | --- | --- | --- | --- |
| `Room` – `Seat` | Một phòng có 0..N ghế; một ghế thuộc đúng 1 phòng | `Room.seats: DoublyLinkedList<Seat*>` | `Seat.room: Room*` | `Room.seats`, `Seat.roomId` đã có; **thêm `Seat.room`** và getter. Kiểm tra `seat.roomId == room.roomId`. |
| `Movie` – `Showtime` | Một phim có 0..N suất; một suất chiếu đúng 1 phim | `Movie.showtimes: DoublyLinkedList<Showtime*>` | `Showtime.movie: Movie*` | `Showtime.movie` đã có; **thêm danh sách ở `Movie`**. Kiểm tra `movieId` khớp. |
| `Room` – `Showtime` | Một phòng có 0..N suất theo thời gian; một suất dùng đúng 1 phòng | `Room.showtimes: DoublyLinkedList<Showtime*>` | `Showtime.room: Room*` | `Showtime.room` đã có; **thêm danh sách suất ở `Room`**. Danh sách này khác danh sách ghế. |
| `Showtime` – `Ticket` | Một suất có 0..N vé; một vé thuộc đúng 1 suất | `Showtime.tickets: DoublyLinkedList<Ticket*>` | `Ticket.showtime: Showtime*` | `Ticket.showtime` đã có; **thêm danh sách vé ở `Showtime`**. Vé hủy vẫn còn trong lịch sử. |
| `Invoice` – `Ticket` | Một hóa đơn hợp lệ có 1..N vé; một vé thuộc đúng 1 hóa đơn | `Invoice.tickets: DoublyLinkedList<Ticket*>` | `Ticket.invoice: Invoice*` | `Invoice.tickets`, `Ticket.invoiceId` đã có; **thêm `Ticket.invoice`**. Kiểm tra cả `invoiceId` và `showtimeId`. |
| `User` – `Invoice` | Một người bán có 0..N hóa đơn; một hóa đơn có đúng 1 người bán | `User.invoices: DoublyLinkedList<Invoice*>` | `Invoice.seller: User*` | `Invoice.seller` đã có; **thêm danh sách ở `User`**. Không đổi lịch sử người bán khi tài khoản bị vô hiệu hóa. |
| `Member` – `Invoice` | Một thành viên có 0..N hóa đơn; một hóa đơn có 0..1 thành viên | `Member.invoices: DoublyLinkedList<Invoice*>` | `Invoice.member: Member*` hoặc `nullptr` | `Invoice.member` đã có; **thêm danh sách ở `Member`**. `memberId` rỗng tương ứng `nullptr`. |
| `Showtime` – `Invoice` | Một suất có 0..N hóa đơn; một hóa đơn thuộc đúng 1 suất | `Showtime.invoices: DoublyLinkedList<Invoice*>` | `Invoice.showtime: Showtime*` | `Invoice.showtime` đã có; **thêm danh sách hóa đơn ở `Showtime`**. |
| `Seat` – `Ticket` | Một ghế có 0..N vé qua nhiều suất; một vé ứng với đúng 1 ghế | `Seat.tickets: DoublyLinkedList<Ticket*>` | `Ticket.seat: Seat*` | `Ticket.seat` đã có; **thêm danh sách lịch sử ở `Seat`** nếu cần duyệt trực tiếp. Trùng ghế khi bán kiểm theo `(showtimeId, seatId)`, không theo `seatId` riêng. |

Không cần thêm `Ticket.room`: có thể đi `Ticket → Showtime → Room` hoặc `Ticket → Seat → Room`. Hai đường phải dẫn tới cùng một `Room`. Với `Member` không gắn hóa đơn, không thêm liên kết vào danh sách nào.

## 3. Ví dụ cụ thể: `Room` ↔ `Seat`

```text
Room R01
  seats ──→ [Seat A01, Seat A02]
                  │         │
                  └─ room ──┴──→ Room R01

seats.txt: mỗi Seat vẫn lưu roomId = R01
```

Đề xuất trong `Seat.h`: khai báo trước `class Room;`, thêm `Room* room = nullptr;` và `const Room* getRoom() const;`. Trong `Room` giữ danh sách `Seat*` hiện có. DALC tải phòng và ghế từ TXT, tìm `Room` bằng `Seat.roomId`, rồi thực hiện **một thao tác nối hai phía**. Thao tác đó từ chối `nullptr`, mã phòng không khớp, ghế trùng mã/vị trí và ghế đã gắn với phòng khác. Sau khi nối thành công, `room.findSeat(seatId) == &seat` và `seat.getRoom() == &room` phải cùng đúng.

Để tránh trạng thái nối một nửa, thiết kế thao tác nối theo thứ tự: kiểm tra toàn bộ điều kiện trước; thêm tham chiếu vào danh sách phòng; chỉ khi thêm thành công mới đặt con trỏ phòng trong ghế. Nếu thao tác thêm có thể thất bại, API DLL cần báo kết quả hoặc DALC phải có cách hoàn tác. Chi tiết này cần kiểm tra API DLL thực tế trước khi viết mã. Không gọi `delete seat` từ `Room`.

Khi xóa ghế hoặc nạp lại dữ liệu, DALC gỡ ghế khỏi danh sách của phòng và đặt `Seat.room = nullptr` **trước** khi hủy/đổi địa chỉ object. Khi đổi `roomId`, không sửa chuỗi riêng lẻ: phải gỡ khỏi phòng cũ, xác thực phòng mới và nối lại cả hai chiều. Nếu không cho chuyển ghế giữa phòng thì giữ `roomId` bất biến sau khi tạo.

## 4. Thứ tự dựng, kiểm tra và gỡ quan hệ

1. DALC đọc tất cả bản ghi TXT, tạo object theo mã và giữ ở địa chỉ ổn định. Các trường mã vẫn tồn tại dù con trỏ chưa nối.
2. Dựng các liên kết gốc: `Movie ↔ Showtime`, `Room ↔ Seat`, `Room ↔ Showtime`, `User/Member ↔ Invoice`, `Showtime ↔ Invoice`.
3. Dựng `Invoice ↔ Ticket`, `Showtime ↔ Ticket`, `Seat ↔ Ticket` sau khi có đủ hóa đơn, suất và ghế. Kiểm tra vé và hóa đơn cùng suất; ghế thuộc phòng của suất.
4. Với mã bắt buộc không tìm thấy đối tượng, báo lỗi dữ liệu hoặc từ chối nạp bản ghi theo chính sách DALC; không tạo liên kết sai. Chỉ `Invoice.member` được rỗng khi `memberId` rỗng.
5. Khi cập nhật/xóa/nạp lại, gỡ tất cả tham chiếu ngược trước khi object biến mất. Vé và hóa đơn đã phát sinh giao dịch **không xóa vật lý**; trạng thái hủy vẫn lưu để tra cứu.

Vì có chu trình tham chiếu, destructor của từng Model không được đi theo con trỏ để hủy object liên quan. Nơi sở hữu object hủy chúng theo quy trình đã gỡ liên kết. Nếu không bảo đảm địa chỉ ổn định hoặc gỡ liên kết đầy đủ, nên tạm dùng mã ID và tra qua DALC cho chiều ngược thay vì lưu thêm con trỏ thô.

## 5. API đề xuất và phạm vi triển khai

| Nhóm API | Ý tưởng |
| --- | --- |
| Đọc quan hệ 1–1 | Getter trả `const Room*`, `const Movie*`, `const Invoice*`... khi chỉ cần đọc; `nullptr` trước khi liên kết hoặc với thành viên tùy chọn. |
| Đọc quan hệ 1–N | Cung cấp `count`, `getAt` hoặc hàm duyệt có kiểm soát; **không trả `DoublyLinkedList` theo giá trị**. |
| Nối/gỡ | DALC cung cấp thao tác như `connectRoomSeat(Room&, Seat&)`, `disconnectRoomSeat(...)` để cập nhật hai phía. Nếu cần hàm `link` trong Model, giới hạn nơi gọi và định nghĩa rõ hợp đồng nhất quán. |
| Kiểm tra bất biến | Test mỗi liên kết theo cả hai chiều, mã khớp, không trùng, không con trỏ treo và không xóa lặp. |

Triển khai nên bắt đầu với `Room ↔ Seat`, kiểm thử sau đó mới mở rộng từng cặp. Trước khi thêm nhiều `DoublyLinkedList<X*>` vào Model, chốt ngữ nghĩa copy/move của DLL và Model chứa DLL; tránh sửa thuật toán DLL chỉ để hoàn thành tài liệu. Các tên hàm trong bảng là **gợi ý**, không phải API đã cam kết. BLL vẫn gọi DALC; GUI vẫn chỉ gọi BLL.

## 6. Các quyết định cần thống nhất trước khi sửa mã

- Chủ sở hữu và nơi lưu object của từng Model là gì, và địa chỉ của chúng được giữ ổn định bằng cách nào?
- Có bắt buộc lưu danh sách ngược trong **từng class** cho mọi cặp, hay một số chiều ngược được DALC cung cấp bằng truy vấn/chỉ mục? Cách thứ hai giảm nhiều danh sách và thao tác đồng bộ.
- DLL và Model chứa DLL có được sao chép/di chuyển không? Nếu có, quy tắc sao chép và cập nhật con trỏ là gì?
- Các quan hệ lịch sử (`User/Member/Seat` với `Invoice/Ticket`) được giữ trong bộ nhớ đến khi nào? Khi dữ liệu lớn, có thể truy vấn theo mã qua DALC thay cho danh sách trực tiếp.
- Khi bản ghi tham chiếu thiếu hoặc sai, ứng dụng báo lỗi, bỏ bản ghi, hay dừng nạp? Quyết định phải phù hợp `data_format.md` và không âm thầm làm sai dữ liệu.

Xem thêm [bản rà soát quan hệ trước đây](../development/QuanHeModels.md). Tài liệu cũ có các đoạn hướng dẫn trước khi một số con trỏ được thêm vào mã; khi triển khai, ưu tiên đối chiếu tám header hiện tại và bản đề xuất này.
