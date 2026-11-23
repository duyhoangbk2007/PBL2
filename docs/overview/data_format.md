# Định dạng dữ liệu TXT

Tài liệu này ghi định dạng đang được Model/Repository/parser sử dụng và tách rõ quy tắc mục tiêu chưa được code bảo đảm. Trong checkout hiện tại, `data/role_permissions.txt` là file TXT có sẵn; các file nghiệp vụ khác có schema trong code nhưng không có sẵn tại `data/`.

## 1. Quy ước

- Dữ liệu lưu bằng UTF-8, mỗi bản ghi nằm trên một dòng, các trường cách nhau bằng ký tự |.
- Parser hiện đọc bản ghi từ dòng đầu tiên; không đọc dòng schema hoặc tiêu đề cột.
- Thứ tự và số lượng trường của mỗi loại file tuân theo định dạng quy định trong tài liệu này.
- Không cho phép `|`, xuống dòng hoặc ký tự điều khiển trong trường văn bản; bản đầu không dùng escape.
- Trường tùy chọn để rỗng, giữ dấu phân cách và cả trường rỗng cuối dòng khi parse; không dùng chữ null.
- Ngày `YYYY-MM-DD`; thời điểm `YYYY-MM-DDTHH:mm:ss`, thống nhất giờ Việt Nam UTC+07:00.
- Tiền dùng `double` theo đơn vị VND, cho phép phần thập phân; khi ghi file dùng dấu chấm `.` làm dấu thập phân, không có dấu phân cách hàng nghìn và không ghi `NaN`/`Infinity`. Boolean dùng 0/1; enum chữ hoa.
- Mã do `IdGenerator` sinh gồm tiền tố ba chữ cái và đúng bốn chữ số: `USR0001`, `MOV0001`, `ROM0001`, `SHW0001`, `TKT0001`, `INV0001`, `MEM0001`.
- Số thứ tự hợp lệ từ 1 đến 9999. Khi hết dải hoặc danh sách ID đầu vào sai định dạng, hàm sinh ID ném lỗi. Hàm lấy số lớn nhất trong **các ID đang nạp** rồi cộng một; không có bộ đếm bền vững. Vì thế mã lớn nhất đã xóa có thể được cấp lại. Quy tắc nghiệp vụ không tái sử dụng mã cần nhóm xác nhận cách bảo đảm trước khi sửa code.
- seatId duy nhất trong phòng, ví dụ A01; hàng/cột bắt đầu từ 1.
- Username phân biệt hoa/thường, không có khoảng trắng đầu/cuối. Điện thoại là chuỗi 10 chữ số bắt đầu bằng 0, duy nhất trong danh sách thành viên.
- Trong phiên bản môn học hiện tại, `password` được lưu dạng văn bản trong `users.txt` để đơn giản hóa phần đăng nhập. Trường này vẫn phải tuân thủ quy tắc ký tự của file (không chứa `|`, xuống dòng hoặc ký tự điều khiển). Nếu nâng cấp sang lưu hash, cần đổi đồng bộ model, định dạng file và dữ liệu tài khoản đã có; không dùng hash table hoặc `std::hash` như hàm bảo vệ mật khẩu.

## 2. Cấu trúc các file dữ liệu (thứ tự trường)

Các dòng tên trường trong khối ví dụ dưới đây chỉ mô tả thứ tự cột; không ghi chúng thành dòng tiêu đề trong file TXT.

### users.txt

```text
userId|username|password|fullName|role|active
```

Trong bộ nhớ, `UserRepository` dùng hai `std::unordered_map<std::string, User*>` theo `userId` và `username`; `Repository<User>` giữ danh sách đối tượng. `InvoiceRepository` hiện không có hash table theo `invoiceId`, mà tra qua `Repository<Invoice>`. Username phải duy nhất và không thay thế `userId` làm mã bất biến của tài khoản.

role = ADMIN/STAFF. Mô hình dùng một kiểu `User` có trường role, không cần lớp con Admin/Staff. `AuthService` (Bước 05) và `StaffService` (Bước 07) cùng dùng `UserRepository` để truy cập file này; sửa nhân viên phải giữ các tài khoản Admin.

### role_permissions.txt

```text
ROLE|PERMISSION_CODE
```

Dòng trên chỉ mô tả hai trường, **không phải dòng tiêu đề**. Mỗi dòng thực tế chứa đúng một cặp, ví dụ `STAFF|MOVIE_VIEW`. File hiện có tại `data/role_permissions.txt`; đường dẫn lúc chạy do constructor nhận.

- `ROLE` chỉ nhận `ADMIN` hoặc `STAFF`. Danh mục Permission mục tiêu gồm 31 mã: `MOVIE_VIEW`, `MOVIE_ADD`, `MOVIE_UPDATE`, `MOVIE_DELETE`, `ROOM_VIEW`, `ROOM_ADD`, `ROOM_UPDATE`, `ROOM_DELETE`, `SEAT_VIEW`, `SEAT_ADD`, `SEAT_UPDATE`, `SEAT_DELETE`, `SHOWTIME_VIEW`, `SHOWTIME_ADD`, `SHOWTIME_UPDATE`, `SHOWTIME_CANCEL`, `SHOWTIME_DELETE`, `STAFF_VIEW`, `STAFF_ADD`, `STAFF_UPDATE`, `STAFF_DEACTIVATE`, `MEMBER_VIEW`, `MEMBER_ADD`, `MEMBER_UPDATE`, `TICKET_SELL`, `TICKET_CANCEL`, `INVOICE_VIEW_OWN`, `INVOICE_VIEW_ALL`, `REPORT_VIEW`, `PRICING_VIEW`, `PRICING_UPDATE`. Mã phân biệt chữ hoa/chữ thường; xem [thiết kế phân quyền](../architecture/PermissionDesign.md).
- Khóa duy nhất là `(ROLE, PERMISSION_CODE)`. Parser hiện từ chối dòng thiếu/thừa trường, role sai hoặc cặp trùng và trả `false`, nhưng **chưa kiểm tra Permission có thuộc danh mục 31 mã** và chưa báo số dòng lỗi. Đó là ràng buộc mục tiêu cần bổ sung sau khi nhóm chốt, không được coi đã triển khai.
- Cặp không có trong file nghĩa là không được cấp quyền, kể cả với Admin. File rỗng hợp lệ về cú pháp nhưng không cấp quyền nào. File thiếu hoặc nạp lỗi phải chặn nghiệp vụ được bảo vệ, không tạo quyền mặc định thay thế.
- Bảng quyền chỉ được đọc trong phiên bản này; sửa file ngoài ứng dụng chỉ có hiệu lực sau khi nạp lại/khởi động lại. Không có dòng quản lý cấp/thu hồi quyền trong định dạng này.

### movies.txt

```text
movieId|title|genre|durationMinutes|minAge|releaseDate|endDate|active
```

Thời lượng dương, tuổi tối thiểu không âm; releaseDate không lớn hơn endDate.

### rooms.txt

```text
roomId|name|rowCount|columnCount|active
```

Số hàng/cột dương. Số vị trí và số chỗ ngồi được tính từ seats.txt, không lưu lặp.

### seats.txt

```text
roomId|seatId|row|column|seatType
```

seatType = STANDARD/VIP/COUPLE. Khóa `(roomId, seatId)` và vị trí `(roomId, row, column)` không trùng. Vị trí nằm trong kích thước phòng; ô không có bản ghi là ô rỗng. Ghế COUPLE có một seatId và một vị trí logic, không tách thành hai bản ghi. Không lưu trạng thái bán.

### showtimes.txt

```text
showtimeId|movieId|roomId|startAt|endAt|status
```

status = SCHEDULED/CANCELLED. endAt > startAt, chốt theo thời lượng phim lúc tạo/sửa suất. `ShowtimeRepository` hiện kiểm tra lịch cùng phòng với khoảng nghỉ tối thiểu 15 phút; nạp `MovieRepository` và `RoomRepository` trước `ShowtimeRepository` để nối các tham chiếu.

### invoices.txt

```text
invoiceId|showtimeId|staffId|memberId|createdAt|totalAmount
```

showtimeId tham chiếu suất của hóa đơn; staffId tham chiếu userId người bán (Staff hoặc Admin); memberId tùy chọn. totalAmount bằng tổng paidAmount ban đầu của mọi vé thuộc hóa đơn, kể cả vé về sau hủy; khi kiểm tra giá trị `double`, cần tính đến sai số số thực. Mỗi hóa đơn có ít nhất một vé; showtimeId trên mọi vé phải bằng showtimeId của hóa đơn. Không lưu lặp danh sách mã vé.

### tickets.txt

```text
ticketId|invoiceId|showtimeId|seatId|ticketType|basePrice|discountAmount|paidAmount|pointsEarned|status|cancelledAt|cancelledBy|cancelReason|refundAmount
```

- ticketType = REGULAR/STUDENT; status = SOLD/CANCELLED.
- Phòng suy ra từ suất chiếu; ghế phải tồn tại trong phòng đó.
- paidAmount = basePrice − discountAmount; tiền và điểm không âm; giảm không vượt giá gốc. Khi đối chiếu các số `double`, cần tính đến sai số số thực.
- Vé SOLD: ba trường thời điểm/người/lý do hủy rỗng, refundAmount = 0.
- Vé CANCELLED: đủ thông tin hủy, `cancelledBy` là mã User (`USR`); Service phải kiểm tra người hủy có `TICKET_CANCEL`. `refundAmount = paidAmount`; `createdAt` của hóa đơn <= `cancelledAt` < `startAt` là quy tắc nghiệp vụ cần kiểm chứng khi liên kết dữ liệu, không phải riêng kiểm tra chuỗi trong parser.
- Tối đa một vé SOLD cho `(showtimeId, seatId)`; được có nhiều vé CANCELLED lịch sử cùng cặp.
- pointsEarned giữ nguyên sau hủy. Hóa đơn không gắn thành viên thì vé có pointsEarned = 0.

### members.txt

```text
memberId|fullName|phone|points
```

Điểm không âm; theo quy tắc nghiệp vụ, số dư bằng tổng `pointsEarned` của các vé SOLD gắn thành viên (khởi tạo 0 điểm, chưa có đổi điểm). `MemberRepository` hiện kiểm tra điểm không âm nhưng **không tự đối chiếu tổng điểm liên file khi load**.

### pricing.txt

Mỗi dòng đúng dạng `KEY|VALUE`, không có dòng tiêu đề. File phải có đúng năm khóa, mỗi khóa xuất hiện một lần; thứ tự khi nạp tùy ý, khi ghi theo thứ tự sau:

```text
STANDARD|70000
VIP|100000
COUPLE|160000
STUDENT_DISCOUNT|10
MEMBER_DISCOUNT|5
```

Ba giá ghế là `double` VND và phải dương; hai tỷ lệ là phần trăm trong `[0,100]`. Giá trị TXT phải là số thập phân parse hết, hữu hạn; không nhận khóa lạ, khóa trùng, thiếu khóa, dòng thiếu/thừa dấu `|` hoặc số dòng khác năm. `PricingRepository::load()` trả `false` khi file thiếu/sai và chỉ công bố cấu hình sau khi toàn file hợp lệ; code hiện **không báo file/dòng lỗi** và không tự tạo file mặc định. `save()`/`update()` ghi bằng `std::to_string()` nên có thể tạo sáu chữ số thập phân. Không lưu quy tắc 1 điểm/10.000 VND trong file này.

### Cấp ID hiện hành

`IdGenerator::generateNextId(entity, existingIds)` nhận các mã đang nạp, xác minh tiền tố và đúng bốn chữ số, lấy số lớn nhất rồi cộng một (`MOV0001` đến `MOV9999`). Tiền tố hỗ trợ: USR/MOV/ROM/SHW/TKT/INV/MEM. Danh sách rỗng bắt đầu `0001`; mã sai hoặc hết dải ném `runtime_error`. Hàm không đọc/ghi bộ đếm; xóa ID lớn nhất có thể dẫn tới cấp lại ID đó. Không có file cấp số thứ tự riêng trong cơ chế hiện tại.

## 3. Tải và kiểm tra

1. `FileIO::readLines()` đọc các dòng vào `std::vector<std::string>` và trả `false` nếu không mở/đọc được file; không tự tạo file thiếu. Repository parse từng dòng, kiểm số trường, kiểu, enum, khóa trùng. Một số repository ném exception có tên file/dòng; `PricingRepository` và `RolePermissionRepository` chủ yếu trả `false`, chưa báo dòng lỗi.
2. `Repository<T>` hiện giữ `DoublyLinkedList<T*>`, nạp vào danh sách tạm rồi mới chuyển khi hợp lệ; sở hữu và giải phóng đối tượng được thêm thành công. Model `Room`, `Movie`, `Invoice`, `User`, `Member` giữ các danh sách liên kết bằng `DoublyLinkedList` nhưng không sở hữu đối tượng liên kết.
3. `UserRepository` và `MemberRepository` dùng `std::unordered_map` làm chỉ mục bổ sung cho ID/username và ID/phone. `std::vector` dùng cho dòng file, kết quả truy vấn, danh sách ID và bản ghi pending. Không có trie, BST hay hash table hóa đơn trong code hiện tại.
4. Nạp đối tượng nền trước đối tượng tham chiếu: Movie/Room/User/Member, rồi Seat/Showtime, tiếp theo Invoice, cuối cùng Ticket; bảng Permission phải nạp thành công trước khi cho nghiệp vụ được bảo vệ. Repository liên kết các Model sau khi tham chiếu tồn tại; ID thiếu/lỗi phải báo và không ghi đè dữ liệu lỗi.
5. Các kiểm tra đối chiếu liên file như tổng vé–hóa đơn hoặc điểm thành viên cần được tách giữa phần đã có và phần chưa hoàn chỉnh. Không coi một lần load là phục hồi giao dịch đa file.

Ứng dụng mục tiêu dùng một phiên thao tác trên bộ dữ liệu. Chưa có cơ chế khóa nhiều tiến trình; nếu sau này nhiều tiến trình cùng ghi, nhóm phải thiết kế riêng.

## 4. Đọc, ghi và giới hạn phục hồi hiện tại

Repository dùng `FileIO` để đọc các dòng TXT, kiểm tra định dạng, khóa và tham chiếu rồi chuyển thành Model trong RAM. Khi nghiệp vụ yêu cầu lưu, Repository chuyển dữ liệu thành các dòng và gọi `FileIO` ghi **toàn bộ nội dung** từng file tương ứng; không append từng bản ghi. `FileIO` chỉ đọc/ghi dòng, không hiểu quy tắc nghiệp vụ. `FileIO::writeLines()` mở file bằng `std::ofstream`, nên một lần ghi thất bại có thể đã thay đổi nội dung file; không có cơ chế ghi tạm rồi đổi tên nguyên tử.

Nếu đọc hoặc ghi thất bại, phải báo lỗi rõ ràng. Không bỏ qua bản ghi sai hoặc âm thầm ghi đè dữ liệu không hợp lệ. Khi một nghiệp vụ ghi nhiều file, các lần ghi diễn ra tuần tự và **không nguyên tử**: lỗi hoặc gián đoạn có thể khiến dữ liệu giữa các file không nhất quán. `BookingService` hiện có rollback các bản ghi pending trong RAM trước khi ghi và cờ `storageFailed` chặn giao dịch tiếp sau lỗi ghi; đây **không phải** rollback toàn bộ TXT. Chưa có `TransactionManager` tích hợp, backup/phục hồi giao dịch lúc khởi động hoặc khóa file/đa tiến trình. Luồng xử lý phải kiểm tra từng lần ghi và không báo thành công khi có lỗi.
