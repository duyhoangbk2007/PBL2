# Định dạng dữ liệu TXT

Đặc tả thiết kế mục tiêu; việc cập nhật tài liệu chưa tạo file dữ liệu hoặc triển khai chức năng đọc/ghi nghiệp vụ.

## 1. Quy ước

- Dữ liệu lưu bằng UTF-8, mỗi bản ghi nằm trên một dòng, các trường cách nhau bằng ký tự |.
- Mọi file TXT dữ liệu, kể cả `sequences.txt`, không có dòng `#schema=1` hoặc dòng tiêu đề cột; bản ghi đầu tiên nằm ngay ở dòng đầu file.
- Thứ tự và số lượng trường của mỗi loại file tuân theo định dạng quy định trong tài liệu này.
- Không cho phép `|`, xuống dòng hoặc ký tự điều khiển trong trường văn bản; bản đầu không dùng escape.
- Trường tùy chọn để rỗng, giữ dấu phân cách và cả trường rỗng cuối dòng khi parse; không dùng chữ null.
- Ngày `YYYY-MM-DD`; thời điểm `YYYY-MM-DDTHH:mm:ss`, thống nhất giờ Việt Nam UTC+07:00.
- Tiền dùng `double` theo đơn vị VND, cho phép phần thập phân; khi ghi file dùng dấu chấm `.` làm dấu thập phân, không có dấu phân cách hàng nghìn và không ghi `NaN`/`Infinity`. Boolean dùng 0/1; enum chữ hoa.
- Mã gồm tiền tố và đúng 4 chữ số: `USR0001`, `MOV0001`, `ROM0001`, `SHW0001`, `TKT0001`, `INV0001`, `MEM0001`.
- Số thứ tự hợp lệ từ 1 đến 9999. Khi hết số thứ tự phải báo lỗi, không cấp lại mã đã sử dụng.
- seatId duy nhất trong phòng, ví dụ A01; hàng/cột bắt đầu từ 1.
- Username phân biệt hoa/thường, không có khoảng trắng đầu/cuối. Điện thoại là chuỗi 10 chữ số bắt đầu bằng 0, duy nhất trong danh sách thành viên.
- Trong phiên bản môn học hiện tại, `password` được lưu dạng văn bản trong `users.txt` để đơn giản hóa phần đăng nhập. Trường này vẫn phải tuân thủ quy tắc ký tự của file (không chứa `|`, xuống dòng hoặc ký tự điều khiển). Nếu nâng cấp sang lưu hash, cần đổi đồng bộ model, định dạng file và dữ liệu tài khoản đã có; không dùng hash table hoặc `std::hash` như hàm bảo vệ mật khẩu.

## 2. Cấu trúc các file dữ liệu (thứ tự trường)

Các dòng tên trường trong khối ví dụ dưới đây chỉ mô tả thứ tự cột; không ghi chúng thành dòng tiêu đề trong file TXT.

### users.txt

```text
userId|username|password|fullName|role|active
```

Trong bộ nhớ dựng hai chỉ mục tài khoản: một hash table theo `userId` cho quan hệ tham chiếu và một hash table theo `username` cho đăng nhập. Hash table hóa đơn dùng `invoiceId`. Username phải duy nhất và không thay thế `userId` làm mã bất biến của tài khoản.

role = ADMIN/STAFF. Mô hình dùng một kiểu `User` có trường role, không cần lớp con Admin/Staff. `AuthService` (Bước 05) và `StaffService` (Bước 07) cùng dùng `UserRepository` để truy cập file này; sửa nhân viên phải giữ các tài khoản Admin.

### role_permissions.txt

```text
ROLE|PERMISSION_CODE
```

Dòng trên chỉ mô tả hai trường, **không phải dòng tiêu đề**. Mỗi dòng thực tế chứa đúng một cặp, ví dụ `STAFF|MOVIE_VIEW`. File UTF-8 này không có dòng `#schema=1` hoặc tiêu đề cột; đường dẫn dự kiến là `data/role_permissions.txt`, còn đường dẫn thực tế do cấu hình/constructor cung cấp.

- `ROLE` chỉ nhận `ADMIN` hoặc `STAFF`. `PERMISSION_CODE` chỉ nhận một trong 31 mã hiện có: `MOVIE_VIEW`, `MOVIE_ADD`, `MOVIE_UPDATE`, `MOVIE_DELETE`, `ROOM_VIEW`, `ROOM_ADD`, `ROOM_UPDATE`, `ROOM_DELETE`, `SEAT_VIEW`, `SEAT_ADD`, `SEAT_UPDATE`, `SEAT_DELETE`, `SHOWTIME_VIEW`, `SHOWTIME_ADD`, `SHOWTIME_UPDATE`, `SHOWTIME_CANCEL`, `SHOWTIME_DELETE`, `STAFF_VIEW`, `STAFF_ADD`, `STAFF_UPDATE`, `STAFF_DEACTIVATE`, `MEMBER_VIEW`, `MEMBER_ADD`, `MEMBER_UPDATE`, `TICKET_SELL`, `TICKET_CANCEL`, `INVOICE_VIEW_OWN`, `INVOICE_VIEW_ALL`, `REPORT_VIEW`, `PRICING_VIEW`, `PRICING_UPDATE`. Mã phân biệt chữ hoa/chữ thường; xem [thiết kế phân quyền](../CauTrucThuMuc/PermissionDesign.md).
- Khóa duy nhất là `(ROLE, PERMISSION_CODE)`. Dòng trùng, thiếu/thừa trường, role sai hoặc permission không thuộc danh sách đều là dữ liệu lỗi: báo file/dòng và từ chối nạp bảng quyền; không bỏ qua dòng lỗi hay dùng bảng đã nạp dở.
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

status = SCHEDULED/CANCELLED. endAt > startAt, chốt theo thời lượng phim lúc tạo/sửa suất. `ShowtimeRepository` hiện kiểm tra lịch cùng phòng với khoảng nghỉ tối thiểu 15 phút; nạp MovieRepository và RoomRepository trước ShowtimeRepository để nối các tham chiếu.

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
- Vé CANCELLED: đủ thông tin hủy, cancelledBy tham chiếu Admin, refundAmount = paidAmount; createdAt của hóa đơn <= cancelledAt < startAt.
- Tối đa một vé SOLD cho `(showtimeId, seatId)`; được có nhiều vé CANCELLED lịch sử cùng cặp.
- pointsEarned giữ nguyên sau hủy. Hóa đơn không gắn thành viên thì vé có pointsEarned = 0.

### members.txt

```text
memberId|fullName|phone|points
```

Điểm không âm, bằng tổng pointsEarned của các vé SOLD gắn thành viên (thành viên khởi tạo 0 điểm, chưa có đổi điểm).

### pricing.txt

Mỗi dòng đúng dạng `KEY|VALUE`, không có schema hoặc dòng tiêu đề. File phải có đúng năm khóa, mỗi khóa xuất hiện một lần; thứ tự khi nạp tùy ý, khi ghi theo thứ tự sau:

```text
STANDARD|70000
VIP|100000
COUPLE|160000
STUDENT_DISCOUNT|10
MEMBER_DISCOUNT|5
```

Ba giá ghế là `double` VND và phải dương; hai tỷ lệ là phần trăm trong `[0,100]`. Giá trị TXT phải là số thập phân parse hết, hữu hạn; không nhận khóa lạ, khóa trùng, thiếu khóa, dòng thiếu/thừa dấu `|` hoặc số dòng khác năm. `PricingRepository::load()` trả `false` khi file thiếu/sai và chỉ công bố cấu hình sau khi toàn file hợp lệ; code hiện **không báo file/dòng lỗi** và không tự tạo file mặc định. `save()`/`update()` ghi bằng `std::to_string()` nên có thể tạo sáu chữ số thập phân. Không lưu quy tắc 1 điểm/10.000 VND trong file này.

### sequences.txt

```text
USR|1
MOV|1
ROM|1
SHW|1
TKT|1
INV|1
MEM|1
```

`sequences.txt` cũng tuân theo quy ước không có schema hoặc dòng tiêu đề; mỗi dòng chỉ chứa `entity|nextValue`. Mỗi loại một dòng; entity = USR/MOV/ROM/SHW/TKT/INV/MEM. nextValue dương, lớn hơn phần số của mọi mã hiện có cùng loại. Khi cấp mã, phải lưu bộ đếm mới cùng dữ liệu được tạo trong luồng nghiệp vụ; xóa không giảm bộ đếm. Tràn kiểu số phải báo lỗi.

## 3. Tải và kiểm tra

1. Đọc các file TXT bằng `FileIO`; báo lỗi nếu thiếu file trong bộ dữ liệu đã có. Nạp và kiểm tra `role_permissions.txt` trước khi phục vụ đăng nhập/nghiệp vụ được bảo vệ.
2. Kiểm tra số lượng trường, số trường, kiểu, enum và mã trùng.
3. Tải danh mục rồi kiểm tra tham chiếu suất/hóa đơn/vé; showtimeId của từng vé phải trùng showtimeId trên hóa đơn tương ứng.
4. Kiểm tra hóa đơn không rỗng, ghế bán trùng, tổng tiền, điểm thành viên và bộ đếm mã.
5. Chỉ sau khi hợp lệ mới dựng danh sách, hash table, trie, BST và dữ liệu báo cáo trong bộ nhớ.

Thứ tự khởi động mục tiêu là: đọc và kiểm tra file, dựng dữ liệu trong bộ nhớ rồi mới phục vụ thao tác. Dự án dự kiến chỉ có một phiên ứng dụng thao tác trên bộ dữ liệu tại một thời điểm.

File thiếu trong bộ dữ liệu có sẵn không tự coi là rỗng. Báo file/dòng lỗi và chặn ghi. Khởi tạo bộ dữ liệu mới là thao tác riêng: tạo mọi file TXT không có dòng schema hoặc tiêu đề; bộ đếm trong `sequences.txt` bắt đầu từ 1.

## 4. Đọc và ghi TXT theo thiết kế mục tiêu

Repository dùng `FileIO` để đọc các dòng TXT, kiểm tra định dạng, khóa và tham chiếu rồi chuyển thành Model trong RAM. Khi nghiệp vụ yêu cầu lưu, Repository chuyển dữ liệu thành các dòng và gọi `FileIO` ghi **toàn bộ nội dung** từng file tương ứng; không append từng bản ghi. `FileIO` chỉ đọc/ghi dòng, không hiểu quy tắc nghiệp vụ.

Nếu đọc hoặc ghi thất bại, phải báo lỗi rõ ràng. Không bỏ qua bản ghi sai hoặc âm thầm ghi đè dữ liệu không hợp lệ. Khi một nghiệp vụ ghi nhiều file, các lần ghi diễn ra tuần tự và **không nguyên tử**: lỗi hoặc gián đoạn có thể khiến dữ liệu giữa các file không nhất quán. Luồng xử lý phải kiểm tra kết quả từng lần ghi và không báo thành công khi có lỗi. Thiết kế mục tiêu không dùng `TransactionManager`, backup giao dịch hay phục hồi lúc khởi động.

