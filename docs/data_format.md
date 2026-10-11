# Định dạng dữ liệu và phục hồi giao dịch

Đặc tả thiết kế mục tiêu; việc cập nhật tài liệu chưa tạo file dữ liệu hoặc triển khai cơ chế phục hồi.

## 1. Quy ước

- UTF-8, một bản ghi mỗi dòng, trường cách nhau bằng `|`. Dòng đầu bắt buộc `#schema=1`; không có dòng tên cột trong file thực tế.
- Không cho phép `|`, xuống dòng hoặc ký tự điều khiển trong trường văn bản; bản đầu không dùng escape.
- Trường tùy chọn để rỗng, giữ dấu phân cách và cả trường rỗng cuối dòng khi parse; không dùng chữ null.
- Ngày `YYYY-MM-DD`; thời điểm `YYYY-MM-DDTHH:mm:ss`, thống nhất giờ Việt Nam UTC+07:00.
- Tiền là số nguyên 64-bit VND, không dấu phân cách hàng nghìn; boolean 0/1; enum chữ hoa.
- Mã gồm tiền tố và số tăng dần tối thiểu sáu chữ số: USR000001, MOV000001, ROM000001, SHW000001, TKT000001, INV000001, MEM000001. Không giới hạn ở sáu chữ số và không tái sử dụng mã.
- seatId duy nhất trong phòng, ví dụ A01; hàng/cột bắt đầu từ 1.
- Username phân biệt hoa/thường, không có khoảng trắng đầu/cuối. Điện thoại là chuỗi 10 chữ số bắt đầu bằng 0, duy nhất trong danh sách thành viên.
- Không lưu mật khẩu rõ. passwordHash là chuỗi hash có salt, định danh thuật toán và tham số từ thư viện chuyên dụng, phải tương thích quy tắc ký tự file. Ưu tiên Argon2id; nếu môi trường môn học không hỗ trợ thì chọn bcrypt hoặc PBKDF2-HMAC-SHA-256 và ghi rõ trong tài liệu triển khai. Không tự viết thuật toán và không dùng hash table hoặc std::hash để bảo vệ mật khẩu.

## 2. Schema (thứ tự trường)

### users.txt

```text
userId|username|passwordHash|fullName|role|active
```

Trong bộ nhớ dựng hai chỉ mục tài khoản: một hash table theo `userId` cho quan hệ tham chiếu và một hash table theo `username` cho đăng nhập. Hash table hóa đơn dùng `invoiceId`. Username phải duy nhất và không thay thế `userId` làm mã bất biến của tài khoản.

role = ADMIN/STAFF. Mô hình dùng một kiểu `User` có trường role, không cần lớp con Admin/Staff. AuthRepository và StaffRepository cùng dùng file này; sửa nhân viên phải giữ các tài khoản Admin.

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

status = SCHEDULED/CANCELLED. endAt > startAt, chốt theo thời lượng phim lúc tạo/sửa suất.

### invoices.txt

```text
invoiceId|showtimeId|staffId|memberId|createdAt|totalAmount
```

showtimeId tham chiếu suất của hóa đơn; staffId tham chiếu userId người bán (Staff hoặc Admin); memberId tùy chọn. totalAmount bằng tổng paidAmount ban đầu của mọi vé thuộc hóa đơn, kể cả vé về sau hủy. Mỗi hóa đơn có ít nhất một vé; showtimeId trên mọi vé phải bằng showtimeId của hóa đơn. Không lưu lặp danh sách mã vé.

### tickets.txt

```text
ticketId|invoiceId|showtimeId|seatId|ticketType|basePrice|discountAmount|paidAmount|pointsEarned|status|cancelledAt|cancelledBy|cancelReason|refundAmount
```

- ticketType = REGULAR/STUDENT; status = SOLD/CANCELLED.
- Phòng suy ra từ suất chiếu; ghế phải tồn tại trong phòng đó.
- paidAmount = basePrice − discountAmount; tiền và điểm không âm; giảm không vượt giá gốc.
- Vé SOLD: ba trường thời điểm/người/lý do hủy rỗng, refundAmount = 0.
- Vé CANCELLED: đủ thông tin hủy, cancelledBy tham chiếu Admin, refundAmount = paidAmount; createdAt của hóa đơn <= cancelledAt < startAt.
- Tối đa một vé SOLD cho `(showtimeId, seatId)`; được có nhiều vé CANCELLED lịch sử cùng cặp.
- pointsEarned giữ nguyên sau hủy. Hóa đơn không gắn thành viên thì vé có pointsEarned = 0.

### members.txt

```text
memberId|fullName|phone|points
```

Điểm không âm, bằng tổng pointsEarned của các vé SOLD gắn thành viên (thành viên khởi tạo 0 điểm, chưa có đổi điểm).

### sequences.txt

```text
entity|nextValue
```

Mỗi loại một dòng; entity = USR/MOV/ROM/SHW/TKT/INV/MEM. nextValue dương, lớn hơn phần số của mọi mã hiện có cùng loại. Tăng và lưu cùng giao dịch tạo dữ liệu; xóa không giảm bộ đếm. Tràn kiểu số phải báo lỗi.

## 3. Tải và kiểm tra

1. Phục hồi giao dịch dang dở trước khi đọc dữ liệu nghiệp vụ.
2. Kiểm tra schema, số trường, kiểu, enum và mã trùng.
3. Tải danh mục rồi kiểm tra tham chiếu suất/hóa đơn/vé; showtimeId của từng vé phải trùng showtimeId trên hóa đơn tương ứng.
4. Kiểm tra hóa đơn không rỗng, ghế bán trùng, tổng tiền, điểm thành viên và bộ đếm mã.
5. Chỉ sau khi hợp lệ mới dựng danh sách, hash table, trie, BST và dữ liệu báo cáo trong bộ nhớ.

Thứ tự khởi động ứng dụng là: lấy khóa độc quyền bộ dữ liệu, phục hồi giao dịch dang dở, đọc và kiểm tra file, dựng chỉ mục rồi mới hiển thị đăng nhập. Khóa phải dựa trên cơ chế khóa của hệ điều hành hoặc có kiểm tra tiến trình sở hữu; không chỉ dựa vào sự tồn tại của một file lock. Khi thoát bình thường, ứng dụng hoàn tất ghi rồi nhả khóa.

File thiếu trong bộ dữ liệu có sẵn không tự coi là rỗng. Báo file/dòng lỗi và chặn ghi. Khởi tạo bộ dữ liệu mới là thao tác riêng, tạo các file có schema, bộ đếm bắt đầu từ 1 và tài khoản Admin đầu tiên; không quy định mật khẩu mặc định trong tài liệu.

## 4. Lưu nhiều file và phục hồi

TransactionManager áp dụng cho bán/hủy vé và các cập nhật nhiều file như phòng/ghế hoặc thực thể/bộ đếm. Thay từng file riêng không bảo đảm nguyên tử cho cả giao dịch.

Mỗi giao dịch có thư mục duy nhất dưới `data/transactions/`, chứa dữ liệu mới, backup và manifest (danh sách file, file gốc có tồn tại không, trạng thái giao dịch).

1. Chuẩn bị và kiểm tra toàn bộ dữ liệu mới trong bộ nhớ, chưa thay trạng thái đang dùng.
2. Ghi file mới và backup, kiểm tra lỗi và flush bền vững theo API hệ điều hành.
3. Ghi bền vững manifest PREPARED trước khi đụng file chính. Cập nhật manifest qua file tạm rồi thay thế nguyên tử trên cùng ổ đĩa.
4. Thay file chính bằng dữ liệu mới; giữ backup và bản mới đến khi hoàn tất.
5. Sau khi mọi file chính được ghi bền vững, ghi bền vững manifest COMMITTED. Đây là mốc commit.
6. Cập nhật bộ nhớ, báo thành công rồi dọn vùng giao dịch. Lỗi dọn sau commit không làm giao dịch thất bại.

Phục hồi trước khi phục vụ người dùng:

- Vùng chuẩn bị chưa có manifest và chưa thay file chính: có thể dọn sau kiểm tra.
- PREPARED: khôi phục toàn bộ file từ backup; file vốn không tồn tại thì loại bỏ bản mới tương ứng. Giữ manifest đến khi rollback xong để phục hồi có thể lặp lại nếu tiếp tục gián đoạn.
- COMMITTED: giữ dữ liệu mới, kiểm tra và hoàn tất dọn; không rollback giao dịch đã commit.
- Manifest hỏng hoặc thiếu backup cần thiết: chặn ghi, báo lỗi; không đoán dữ liệu đúng.

Lỗi trước commit phải rollback và báo chưa hoàn tất; rollback thất bại thì chặn thao tác ghi tiếp. Cần kiểm thử lỗi ghi và gián đoạn ở từng giai đoạn khi triển khai. Cơ chế này không thay thế bản sao lưu độc lập khi ổ đĩa hỏng.

