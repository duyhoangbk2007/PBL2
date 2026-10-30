# Thiết kế quan hệ giữa 8 model — Bước 2.3

Tài liệu này đối chiếu **mã hiện có** với cách biểu diễn quan hệ giữa các object khi chương trình chạy. Đây là thiết kế để học và xem lại trước khi sửa code; các con trỏ dưới đây **chưa được thêm vào model**. File `.txt` vẫn lưu mã (`movieId`, `roomId`, ...), vì địa chỉ con trỏ không thể lưu để dùng lại ở lần chạy sau.

## 1. Ba khái niệm cần phân biệt

- **Association (liên kết):** một object biết hoặc dùng object khác. Ví dụ một `Showtime` dùng một `Movie`. Hai object có thể sống độc lập.
- **Aggregation (kết tập):** quan hệ toàn thể–thành phần nhưng thành phần được quản lý độc lập. Ví dụ `Room` tập hợp nhiều `Seat`; trong thiết kế hiện tại ghế là bản ghi riêng trong `seats.txt`.
- **Composition (hợp thành):** thành phần thuộc về đúng một toàn thể và vòng đời gắn chặt với toàn thể. Về **nghiệp vụ**, một `Ticket` phải thuộc một `Invoice`. Nhưng gọi đó là composition trong C++ còn đòi hỏi xác định rõ ai sở hữu và hủy object; chỉ lưu `Ticket*` không tự tạo quyền sở hữu.

**Số lượng (1–N) và quyền sở hữu là hai câu hỏi khác nhau.** `Room` có nhiều `Seat` không có nghĩa `Room` phải `delete` các ghế. Con trỏ thô `T*` có thể chỉ là tham chiếu không sở hữu; chủ sở hữu thật phải được xác định ở nơi tạo/lưu các object.

## 2. Bảng quan hệ từ dữ liệu hiện có

| Quan hệ | Số lượng | Cách liên kết đang có | Phân loại đề xuất | Hướng con trỏ có thể dùng khi dựng object |
| --- | --- | --- | --- | --- |
| `Room` – `Seat` | Một phòng có nhiều ghế; một ghế thuộc một phòng | `Seat.roomId` → `Room.roomId` | Aggregation để bắt đầu học quan hệ | `Room` giữ danh sách `Seat*` **không sở hữu**; nơi quản lý dữ liệu giữ các `Seat` thật. |
| `Movie` – `Showtime` | Một phim có nhiều suất; một suất chiếu một phim | `Showtime.movieId` → `Movie.movieId` | Association | `Showtime` giữ `Movie*` không sở hữu; không cần `Movie` giữ danh sách suất ngay. |
| `Room` – `Showtime` | Một phòng có nhiều suất theo thời gian; một suất dùng một phòng | `Showtime.roomId` → `Room.roomId` | Association | `Showtime` giữ `Room*` không sở hữu; không nhầm với danh sách ghế của phòng. |
| `Showtime` – `Invoice` | Một suất có thể có nhiều hóa đơn; một hóa đơn dành cho một suất | `Invoice.showtimeId` → `Showtime.showtimeId` | Association | `Invoice` có thể giữ `Showtime*` không sở hữu. |
| `User` – `Invoice` | Một Admin/Staff có thể bán nhiều hóa đơn; một hóa đơn ghi một người bán | `Invoice.staffId` → `User.userId` | Association | `Invoice` có thể giữ `User*` không sở hữu. |
| `Member` – `Invoice` | Một thành viên có thể gắn nhiều hóa đơn; hóa đơn có 0 hoặc 1 thành viên | `Invoice.memberId` → `Member.memberId`, có thể rỗng | Association tùy chọn | `Invoice` có thể giữ `Member*` hoặc `nullptr` khi không gắn thành viên. |
| `Invoice` – `Ticket` | Một hóa đơn có ít nhất một vé; một vé thuộc một hóa đơn | `Ticket.invoiceId` → `Invoice.invoiceId` | Composition **ở mức nghiệp vụ**; quyền sở hữu C++ chưa chốt | Nếu `Invoice` giữ danh sách `Ticket*` không sở hữu thì trong code là liên kết/kết tập, chưa phải composition quản lý bộ nhớ. |
| `Showtime` – `Ticket` | Một suất có nhiều vé; một vé dành cho một suất | `Ticket.showtimeId` → `Showtime.showtimeId` | Association | `Ticket` có thể giữ `Showtime*` không sở hữu; vẫn kiểm tra suất trùng với hóa đơn. |
| `Seat` – `Ticket` | Một vị trí ghế có thể xuất hiện ở nhiều suất/vé lịch sử; mỗi vé chỉ một vị trí | `Ticket.seatId` + `Showtime.roomId` xác định `Seat` | Association | `Ticket` có thể giữ `Seat*` không sở hữu. Ghế bán trùng được kiểm tra theo `(showtimeId, seatId)`, không chỉ theo `seatId`. |

`Ticket` không cần trỏ trực tiếp tới `Room`: từ suất chiếu của vé đã tìm được phòng; từ phòng mới kiểm tra đúng ghế. Danh sách trên mô tả những quan hệ cần hiểu, **không yêu cầu thêm tất cả con trỏ cùng lúc**.

## 3. Bắt đầu bằng Room – Seat ở Bước 2.4

Ví dụ dự kiến:

```text
Room ROM000001
  ├── Seat A01
  ├── Seat A02
  └── Seat B01
```

`Room` sẽ giữ danh sách các `Seat*` trỏ đến những object ghế đã được tạo và quản lý ở nơi khác. `Room` có thể duyệt ghế để dựng sơ đồ, nhưng **không tự `delete` các con trỏ không sở hữu**. `Seat.roomId` vẫn cần cho `seats.txt` và để kiểm tra ghế thật sự thuộc phòng. Khi tải file, đọc/tạo `Room` và `Seat` trước, rồi nối những ghế có `seat.roomId == room.roomId` vào phòng.

Nếu dùng `DoublyLinkedList<Seat*>` hiện có, phải xử lý việc sao chép của danh sách trước: lớp đang sở hữu node bằng con trỏ thô và chưa tự định nghĩa/vô hiệu hóa copy constructor, copy assignment. Sao chép mặc định sẽ khiến hai danh sách cùng trỏ tới một chuỗi node và có nguy cơ `delete` lặp. Cũng cần giữ địa chỉ các `Seat` ổn định; nếu object bị xóa/di chuyển trong khi `Room` còn giữ `Seat*`, con trỏ sẽ mất hiệu lực. Những việc này là **điều kiện trước khi cài**, chưa phải mã đã hoạt động.

## 4. Những quyết định cần chốt trước khi sửa các model còn lại

1. **Ai sở hữu object?** Repository hoặc một nơi quản lý dữ liệu tạo/hủy `Movie`, `Room`, `Seat`, ...; con trỏ trong model chỉ tham chiếu hay thật sự sở hữu? Không để hai nơi cùng `delete` một object.
2. **Khi nào nối con trỏ?** Sau khi đọc hết file, xác minh mã tham chiếu và dựng object liên quan. Bản ghi thiếu `Room`, `Movie`, `Invoice`... phải báo lỗi dữ liệu, không âm thầm để con trỏ rỗng (ngoại trừ `Member*` của hóa đơn không gắn thành viên).
3. **Giữ gì trong file?** Mã vẫn là dữ liệu bền vững; con trỏ chỉ có ý nghĩa trong lần chạy hiện tại. Nếu model giữ cả mã lẫn con trỏ thì phải đồng bộ chúng; không tự đổi một bên mà quên bên kia.
4. **Có thật sự cần con trỏ ở quan hệ đó không?** Chỉ thêm nơi giúp truy cập object trong luồng cần thiết. Chẳng hạn ở Bước 2.4 làm `Room`–`Seat` trước; các quan hệ `Showtime`, `Invoice`, `Ticket` có thể tiếp tục dùng mã đến khi có Repository và chiến lược vòng đời rõ ràng.
5. **Composition của Invoice–Ticket sẽ cài thế nào?** Nếu `Invoice` sở hữu vé, cần thay đổi cách repository lưu/hủy object. Nếu mỗi vé vẫn được quản lý độc lập và `Invoice` chỉ có `Ticket*`, hãy mô tả đúng đó là tham chiếu không sở hữu, dù vé phụ thuộc hóa đơn về nghiệp vụ.

Chưa sửa code ở Bước 2.3. Việc tiếp theo là thống nhất quyền sở hữu và cách giữ địa chỉ ổn định cho `Room`–`Seat`, sau đó mới thử cài một quan hệ này ở Bước 2.4.

## 5. Cụ thể từng model cần thêm gì?

Bảng dưới là **đề xuất về mặt thiết kế**, dựa trên tám header hiện tại. “Chưa cần” nghĩa là quan hệ vẫn được biểu diễn bằng mã và được Repository/Service tra cứu khi cần; không phải thiếu chức năng ngay trong model. Tất cả con trỏ đề xuất đều **không sở hữu** object được trỏ tới, trừ khi sau này có một quyết định riêng về Composition.

| Model | Thêm ở Bước 2.4? | Có thể thêm sau khi đã có nơi quản lý object ổn định | Giữ nguyên |
| --- | --- | --- | --- |
| `Movie` | Chưa cần thêm gì. | Không cần giữ danh sách `Showtime*` chỉ để chứng minh quan hệ 1–N; có thể tra các suất theo `movieId`. | `movieId` và thông tin phim. |
| `Room` | Một tập `Seat*` không sở hữu, dự kiến `DoublyLinkedList<Seat*> seats`; thao tác thêm ghế sau khi kiểm tra `seat.roomId` trùng `roomId`, và cách duyệt/tìm ghế mà không sao chép cả danh sách. | Chỉ thêm danh sách suất nếu một chức năng thực tế cần duyệt từ phòng sang các suất. | `roomId`, kích thước hàng/cột và trạng thái; các trường này vẫn ghi vào `rooms.txt`. |
| `Seat` | Chưa cần thêm `Room*`: quan hệ ngược đã có qua `Seat.roomId`, còn `Room` sẽ giữ tham chiếu tới ghế. | Có thể thêm `Room*` không sở hữu nếu thực sự cần truy cập phòng từ ghế; lúc đó phải đồng bộ với `roomId`. | `roomId`, `seatId`, hàng/cột và loại ghế để ghi `seats.txt`. |
| `Showtime` | Chưa cần trong lúc chỉ làm `Room`–`Seat`. | `Movie* movie` và `Room* room` không sở hữu, kèm thao tác nối sau khi tìm đúng `movieId`/`roomId`. | Các mã và thời điểm vẫn cần cho `showtimes.txt`. |
| `User` | Chưa cần thêm gì. | Không cần giữ mọi hóa đơn từng bán; tìm theo `Invoice.staffId` là đủ khi xem lịch sử. | `userId`, vai trò và thông tin tài khoản. |
| `Member` | Chưa cần thêm con trỏ quan hệ. | Không cần giữ mọi hóa đơn; tìm theo `Invoice.memberId`. Phần `setPoint`/`setPoints` của mục 2.2 là việc riêng, không phải quan hệ object. | `memberId`, liên hệ và điểm. |
| `Invoice` | Chưa cần trong lúc chỉ làm `Room`–`Seat`. | Có thể nối `Showtime*`, `User*` người bán, `Member*` tùy chọn (`nullptr` khi không có thành viên). Nếu cần duyệt các vé từ hóa đơn, có thể giữ danh sách `Ticket*` không sở hữu và thêm vé theo `invoiceId`. | `showtimeId`, `staffId`, `memberId` và tổng tiền vẫn ghi vào `invoices.txt`. |
| `Ticket` | Chưa cần trong lúc chỉ làm `Room`–`Seat`. | Có thể nối `Showtime*` và `Seat*` không sở hữu để truy cập suất/ghế; chỉ nối ghế thuộc phòng của suất. Không bắt buộc thêm `Room*` hoặc `Invoice*` nếu `Invoice` đã giữ danh sách vé. | `invoiceId`, `showtimeId`, `seatId` và các thông tin giá/hủy vẫn ghi vào `tickets.txt`. |

**Cách nối một con trỏ về sau:** file được đọc thành object với mã trước; sau khi tất cả object liên quan đã tồn tại, nơi quản lý dữ liệu tìm theo mã và gắn con trỏ. Thao tác gắn phải kiểm tra mã khớp, không gắn `Seat` của phòng khác hay `Ticket` của hóa đơn khác. Getter có thể trả con trỏ để đọc quan hệ; không trả `DoublyLinkedList` **theo giá trị** khi lớp này chưa hỗ trợ sao chép an toàn. Nếu object bị xóa hoặc thay địa chỉ, phải gỡ/nối lại tham chiếu trước khi dùng.

**Riêng `Invoice`–`Ticket`:** nếu thầy yêu cầu Composition cả trong cách quản lý bộ nhớ, không dùng bảng trên như phương án cuối. Phải quyết định `Invoice` thực sự sở hữu `Ticket` bằng cơ chế sở hữu rõ ràng, rồi thiết kế lại nơi tạo/hủy vé và cách Repository giữ vé. Bản PBL2 hiện lưu vé trong `tickets.txt` riêng và giữ lịch sử khi hủy, nên chưa nên thêm `delete` vào `Invoice` chỉ vì thấy quan hệ 1–N.

## 6. Checklist thực hiện riêng Bước 2.3

Mục tiêu của 2.3 là có **bản thiết kế quan hệ đủ rõ để cài thử `Room`–`Seat` ở 2.4**. Những ô dưới đây là việc cần tự kiểm tra và chốt; chưa đánh dấu hoàn thành chỉ vì bảng đề xuất ở trên đã được viết. Ở bước này chưa thêm thuộc tính con trỏ vào `.h` hoặc viết hàm trong `.cpp`.

### 2.3.1. Tìm mọi quan hệ từ dữ liệu hiện có

- [ ] Đọc 8 header trong `core/dal/models/` và các mục `rooms.txt`, `seats.txt`, `showtimes.txt`, `invoices.txt`, `tickets.txt` của `data_format.md`.
- [ ] Với mỗi trường mã tham chiếu (`roomId`, `movieId`, `showtimeId`, `staffId`, `memberId`, `invoiceId`, `seatId`), trả lời: nó đang trỏ đến loại object nào? Ghi thành một cặp vào bảng Mục 2.
- [ ] Kiểm tra các liên kết gián tiếp: `Ticket.seatId` phải được hiểu cùng `Showtime.roomId`; `Invoice.memberId` có thể rỗng.

**Kết quả cần có:** không bỏ sót quan hệ có trong schema; không coi hai chuỗi cùng tên là hai object tự động nối với nhau. Ví dụ `Seat.roomId` chỉ là mã để tìm `Room`, chưa phải `Room*`.

### 2.3.2. Xác định số lượng ở hai đầu

- [ ] Đọc mỗi dòng của bảng Mục 2 theo cả hai chiều. Ví dụ: một `Room` có **0 đến nhiều** `Seat` trong bộ nhớ; mỗi `Seat` có **đúng một** `Room` hợp lệ trong dữ liệu.
- [ ] Phân biệt bắt buộc và tùy chọn: một `Invoice` có ít nhất một `Ticket`; `Invoice` có thể không có `Member`, nhưng luôn có người bán `User` và một `Showtime`.
- [ ] Ghi rõ thời gian có ảnh hưởng không: một `Seat` vật lý có thể gắn với nhiều vé lịch sử ở các suất khác nhau; không kết luận mỗi ghế chỉ có một vé.

**Kết quả cần có:** mỗi quan hệ có câu dạng “một A có bao nhiêu B; một B thuộc bao nhiêu A”. Số lượng này là quy tắc dữ liệu, chưa cho biết ai sở hữu bộ nhớ.

### 2.3.3. Phân loại Association, Aggregation, Composition

- [ ] Với từng cặp, hỏi: đây chỉ là liên kết để tra cứu, hay một object đóng vai trò toàn thể chứa các thành phần?
- [ ] Dùng `Room`–`Seat` làm bài tập Aggregation: phòng gom các ghế để hiển thị sơ đồ, nhưng ghế vẫn là bản ghi riêng và được quản lý ngoài `Room` trong đề xuất hiện tại.
- [ ] Dùng `Showtime`–`Movie` làm ví dụ Association: suất chiếu dùng một phim, phim không bị tạo/hủy chỉ vì một suất được tạo/hủy.
- [ ] Dùng `Invoice`–`Ticket` để thảo luận Composition **về nghiệp vụ**: vé luôn thuộc hóa đơn và không xóa lịch sử. Ghi riêng rằng quyền sở hữu object trong C++ còn chưa chốt; không suy ra `Invoice` phải `delete Ticket*`.

**Kết quả cần có:** mỗi nhãn quan hệ có một lý do ngắn dựa trên vòng đời dữ liệu, không dựa chỉ vào việc có dấu `*` trong code.

### 2.3.4. Chọn hướng tham chiếu tối thiểu

- [ ] Xác định chức năng sẽ đi **từ object nào sang object nào**. Ở 2.4 cần duyệt từ `Room` sang các `Seat` để dựng sơ đồ, nên đề xuất `Room` giữ tập `Seat*`.
- [ ] Với các cặp khác, xem bảng Mục 5 và ghi “cần ngay”, “để sau” hoặc “chỉ tra theo mã”. Không thêm cả hai chiều chỉ vì quan hệ là 1–N.
- [ ] Với mỗi con trỏ đề xuất, ghi rõ `nullptr` có hợp lệ không. Ví dụ `Invoice` không gắn thành viên có thể có `Member* == nullptr`; một `Showtime` đã nối quan hệ thì không nên thiếu `Movie` hoặc `Room`.

**Kết quả cần có:** danh sách ngắn các trường sẽ thêm ở 2.4 (trước mắt chỉ tập `Seat*` trong `Room`) và danh sách những con trỏ để sang bước sau.

### 2.3.5. Chốt chủ sở hữu và vòng đời trước khi cài

- [ ] Chỉ định nơi **tạo, giữ và hủy** các object `Seat` thật. Đề xuất là Repository hoặc nơi quản lý dữ liệu giữ chúng; `Room` chỉ tham chiếu, không `delete Seat*`.
- [ ] Chọn cách giữ địa chỉ `Seat` ổn định trong suốt thời gian `Room` đang trỏ tới ghế. Không dùng con trỏ tới object tạm hoặc object có thể bị dời khi một vùng lưu trữ tăng kích thước.
- [ ] Quy định khi xóa/tải lại ghế: gỡ hoặc dựng lại tham chiếu trong `Room` **trước** khi object cũ biến mất; kiểm tra `Seat.roomId` khớp `Room.roomId` và tránh gắn lặp cùng ghế.
- [ ] Với `DoublyLinkedList<Seat*>`, xem lại copy constructor/copy assignment của danh sách và của `Room`. Chốt giải pháp trước khi cài để tránh hai danh sách cùng sở hữu node và giải phóng lặp. Node của danh sách và object `Seat` là hai vùng nhớ khác nhau.

**Kết quả cần có:** viết được một câu rõ ràng: “Ai sở hữu `Seat`; `Room` giữ tham chiếu không sở hữu; khi nào tham chiếu được nối/gỡ; danh sách có được sao chép không”. Nếu chưa trả lời được, dừng ở thiết kế, chưa viết `DoublyLinkedList<Seat*>` vào `Room`.

### 2.3.6. Đối chiếu lưu file và kết thúc bản thiết kế

- [ ] Đọc lại bảng Mục 5: các mã như `Seat.roomId`, `Showtime.movieId`, `Ticket.invoiceId` vẫn được lưu; không ghi địa chỉ con trỏ vào `.txt`.
- [ ] Mô tả thứ tự khi khởi động: đọc bản ghi → tạo object → kiểm tra mã tham chiếu → nối con trỏ trong bộ nhớ. Nếu mã bắt buộc không tìm thấy object, báo dữ liệu lỗi thay vì âm thầm gắn sai.
- [ ] Viết một ví dụ trên giấy: `Room ROM000001` có `Seat A01` và `A02`; sau khi nối, `Room` duyệt được hai ghế, còn `seats.txt` vẫn ghi `ROM000001` trên mỗi dòng ghế.
- [ ] Đối chiếu yêu cầu cụ thể của giảng viên về Aggregation/Composition. Nếu thầy muốn `Room` **sở hữu** ghế hoặc muốn con trỏ ở chiều ngược lại, chỉnh bản thiết kế trước khi sửa model.

**Điều kiện hoàn thành 2.3:** bảng quan hệ, số lượng, loại quan hệ và hướng tham chiếu đã được rà soát; riêng `Room`–`Seat` đã chốt chủ sở hữu, địa chỉ ổn định và cách xử lý sao chép DLL. Lúc đó mới chuyển sang 2.4 để cài mã. Những quan hệ còn lại có thể giữ ở mức thiết kế và triển khai khi có Repository phù hợp.

## 7. Hướng dẫn viết code theo từng file

Đây là **mẫu hướng dẫn để tự code**, chưa được chép vào các file `.h/.cpp` hoặc biên dịch. Làm từng phần theo thứ tự; đoạn ở Mục 7.3 trở đi là **dự kiến khi có Repository quản lý vòng đời object**, không phải việc bắt buộc phải cài đồng loạt ở 2.4. Giữ các trường mã đang có vì dữ liệu `.txt` không lưu địa chỉ con trỏ.

### 7.1. `core/data_structures/DoublyLinkedList.h` — chuẩn bị trước Room

1. Bọc **toàn bộ** header bằng include guard để sau này nhiều model cùng include không định nghĩa lại `ListNode`/`DoublyLinkedList`:

   ```cpp
   #ifndef PBL2_CORE_DATA_STRUCTURES_DOUBLY_LINKED_LIST_H
   #define PBL2_CORE_DATA_STRUCTURES_DOUBLY_LINKED_LIST_H

   // Giữ nguyên nội dung hiện có của DoublyLinkedList.h

   #endif
   ```

2. Trong phần `public:` của `DoublyLinkedList<T>`, **trước mắt cấm sao chép** vì danh sách sở hữu các node cấp phát bằng `new` nhưng chưa có cách sao chép sâu:

   ```cpp
   DoublyLinkedList(const DoublyLinkedList&) = delete;
   DoublyLinkedList& operator=(const DoublyLinkedList&) = delete;
   ```

3. Khi danh sách chứa `Seat*`, destructor/`clear()` chỉ xóa **node danh sách**. Nó không được `delete node->data`, vì `Seat` thật do nơi khác sở hữu. Sau khi cấm sao chép DLL, `Room` chứa DLL cũng không sao chép được theo mặc định; Repository tương lai phải giữ `Room` ở nơi ổn định hoặc thiết kế lại cách sao chép trước khi dùng theo giá trị.

### 7.2. `core/dal/models/Room.h` và `Room.cpp` — phần cài thử ở 2.4

Trong `Room.h`, thêm include và khai báo trước `Seat`; chỉ cần khai báo trước vì ở đây ta giữ `Seat*`:

```cpp
#include "../data_structures/DoublyLinkedList.h"
class Seat;

class Room {
private:
    // Giữ các trường roomId, name, rowCount, columnCount, active đang có.
    DoublyLinkedList<Seat*> seats; // Room không sở hữu các Seat.

public:
    // Giữ các constructor/getter/setter đang có.
    bool addSeat(Seat* seat);
    Seat* findSeatById(const std::string& seatId);
    int getSeatCount() const;
};
```

Đây là **các dòng cần ghép vào class hiện có**, không phải thay toàn bộ `Room.h` bằng đoạn trên. Trong `Room.cpp`, thêm `#include "Seat.h"` để gọi các getter của `Seat`. `seats` được dựng mặc định, nên hai constructor hiện có không cần thêm tham số danh sách. Cài lần lượt:

```cpp
bool Room::addSeat(Seat* seat) {
    if (seat == nullptr || seat->getRoomId() != this->roomId ||
        seat->getRow() < 1 || seat->getRow() > this->rowCount ||
        seat->getColumn() < 1 || seat->getColumn() > this->columnCount) {
        return false;
    }

    for (int i = 0; i < this->seats.nodeCount(); ++i) {
        Seat* current = this->seats.getAt(i)->data;
        if (current->getSeatId() == seat->getSeatId() ||
            (current->getRow() == seat->getRow() &&
             current->getColumn() == seat->getColumn())) {
            return false;
        }
    }

    this->seats.pushBack(seat);
    return true;
}

Seat* Room::findSeatById(const std::string& seatId) {
    for (int i = 0; i < this->seats.nodeCount(); ++i) {
        Seat* current = this->seats.getAt(i)->data;
        if (current->getSeatId() == seatId) {
            return current;
        }
    }
    return nullptr;
}

int Room::getSeatCount() const {
    return this->seats.nodeCount();
}
```

`getAt(i)` của DLL hiện chưa có bản `const`, nên ví dụ `findSeatById` tạm là hàm không `const`. Vòng lặp gọi `getAt(i)` nhiều lần sẽ duyệt lại từ đầu (chậm với nhiều ghế); có thể cải tiến DLL bằng hàm duyệt phù hợp sau khi bản đơn giản chạy đúng. `Room` không được `delete seat` trong destructor. Chưa có Repository thì khi thử phải tạo `Seat` có vòng đời dài hơn `Room` hoặc ít nhất dài hơn mọi lần `Room` dùng con trỏ.

### 7.3. `Seat.h/.cpp`, `Movie.h/.cpp`, `User.h/.cpp`, `Member.h/.cpp`

- **`Seat`**: chưa thêm `Room*`; giữ `roomId` và các getter hiện có. `Room::addSeat()` dùng chúng để kiểm tra trước khi nối. Nếu sau này cần `Seat` truy cập `Room`, mới thêm con trỏ ngược và quy tắc đồng bộ.
- **`Movie`**: chưa thêm danh sách suất. `Showtime` có thể trỏ tới phim; tìm mọi suất của một phim sẽ do Repository tra theo `movieId`.
- **`User`**: chưa thêm danh sách hóa đơn; `Invoice.staffId` đủ để tra người bán và lọc lịch sử.
- **`Member`**: chưa thêm danh sách hóa đơn. Việc đổi tên `setPoint(const int&)` thành `setPoints(int)` thuộc 2.2, độc lập với quan hệ con trỏ và chỉ thực hiện sau khi người học quyết định sửa hàm đó.

Với bốn cặp `.h/.cpp` này, **2.3 không yêu cầu sửa code**.

### 7.4. `Showtime.h/.cpp` — ví dụ Association để làm sau

Khi đã có nơi giữ `Movie` và `Room` ở địa chỉ ổn định, trong `Showtime.h` có thể khai báo trước `class Movie; class Room;`, rồi thêm `Movie* movie = nullptr;` và `Room* room = nullptr;` vào `private`. Thêm các hàm `bool bindMovie(Movie* movie);`, `bool bindRoom(Room* room);` và getter trả con trỏ nếu nơi gọi cần đọc object.

Trong `Showtime.cpp`, include `Movie.h` và `Room.h`. `bindMovie()` trả `false` nếu con trỏ rỗng hoặc `movie->getMovieId() != this->movieId`; nếu hợp lệ mới gán `this->movie = movie`. `bindRoom()` làm tương tự với `roomId`. Không `delete` hai con trỏ trong `Showtime`. Nếu dùng cú pháp khởi tạo thành viên ngay ở header (`= nullptr`), không cần sửa danh sách khởi tạo constructor chỉ để đặt null.

### 7.5. `Invoice.h/.cpp` — các liên kết để làm sau

Khi Repository quản lý object ổn định, `Invoice.h` có thể khai báo trước `Showtime`, `User`, `Member`, `Ticket`; thêm `Showtime* showtime = nullptr;`, `User* seller = nullptr;`, `Member* member = nullptr;`. Những con trỏ này không sở hữu object. `member` được phép `nullptr` khi `memberId` là chuỗi rỗng. Tạo thao tác nối tương ứng và chỉ gán khi `showtimeId`, `staffId`, `memberId` khớp getter của object được truyền vào. Trong `Invoice.cpp`, include header của các model cần gọi getter.

Nếu cần đi từ hóa đơn tới các vé, sau khi giải quyết vấn đề include guard/sao chép DLL, mới thêm `DoublyLinkedList<Ticket*> tickets;` và `bool addTicket(Ticket* ticket);`. `addTicket()` phải từ chối null, kiểm tra `ticket->getInvoiceId() == this->invoiceId`, `ticket->getShowtimeId() == this->showtimeId` và tránh thêm cùng `ticketId` hai lần. `Invoice` không `delete Ticket*` nếu Repository là chủ sở hữu các vé. Nếu thầy yêu cầu **Composition có sở hữu**, phải đổi thiết kế này trước khi code; không vừa để Repository sở hữu vé vừa để `Invoice` xóa vé.

### 7.6. `Ticket.h/.cpp` — các liên kết để làm sau

`Ticket.h` có thể khai báo trước `Showtime` và `Seat`, rồi thêm `Showtime* showtime = nullptr;`, `Seat* seat = nullptr;` khi thật sự cần truy cập object từ vé. Trong `Ticket.cpp`, `linkShowtime(Showtime*)` kiểm tra `getShowtimeId()` khớp; `linkSeat(Seat*)` kiểm tra `getSeatId()` khớp và, **sau khi đã nối suất**, `seat->getRoomId() == this->showtime->getRoomId()`. Nếu suất chưa được nối thì chưa gắn ghế, tránh nhận nhầm `A01` của phòng khác. Các con trỏ này không sở hữu và không được `delete` trong `Ticket`.

Giữ nguyên `invoiceId`, `showtimeId`, `seatId` để đọc/ghi `tickets.txt` và giữ `markCancelled(...)` cho trạng thái hủy. Không thêm `Room*` vì có thể đi qua `Showtime`; chưa cần `Invoice*` nếu `Invoice` đã giữ danh sách vé.

### 7.7. Thứ tự viết và thử khi chuyển sang 2.4

1. Thêm include guard và khóa sao chép cho `DoublyLinkedList.h`; biên dịch thử một file include header hai lần.
2. Chỉ sửa `Room.h/.cpp` theo Mục 7.2; biên dịch cú pháp tám model để bắt lỗi include hoặc chữ ký hàm.
3. Tạo vài `Seat` với địa chỉ ổn định rồi thử thêm vào đúng/sai `Room`: null, sai `roomId`, ngoài kích thước, trùng `seatId`, trùng hàng/cột. Kiểm tra `getSeatCount()` và `findSeatById()`.
4. Chạy thử chương trình tới lúc kết thúc để quan sát hủy `Room` và `Seat` không gây giải phóng lặp. Không sao chép `Room`/DLL khi chưa có quy tắc sao chép sâu.
5. Chỉ sau khi phần này ổn mới cân nhắc Mục 7.4–7.6. Các model khác vẫn chạy bằng mã tham chiếu như hiện tại trong lúc chưa có Repository.
