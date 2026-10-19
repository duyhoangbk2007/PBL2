# Thiết kế quy trình BookingService

Tài liệu này giải thích **luồng bán vé dự kiến** của dự án PBL2. `BookingService` và các repository liên quan chưa được lập trình; đây không phải mô tả chức năng đã chạy. Khi triển khai, đối chiếu lại [quy tắc nghiệp vụ](TongQuan/business_rules.md), [định dạng dữ liệu](TongQuan/data_format.md) và mã nguồn thực tế.

## 1. BookingService chịu trách nhiệm gì?

`BookingService` ở tầng Service điều phối một lần bán vé tại quầy: giữ lựa chọn tạm, kiểm tra điều kiện, tính giá/điểm, yêu cầu lưu nhiều file trong một giao dịch và trả kết quả cho UI. Admin và Staff đều được bán; hóa đơn ghi `userId` của người thực sự thao tác.

| Thành phần | Trách nhiệm trong lần bán |
| --- | --- |
| UI (`BookingView`, `SeatMapView` dự kiến) | Hiển thị suất, sơ đồ ghế, giá và lỗi; chuyển thao tác của nhân viên cho Service. Không tự ghi file hay quyết định ghế đã bán. |
| `BookingService` | Kiểm tra quyền và quy tắc; quản lý giỏ ghế tạm/Undo; tính giá, điểm; điều phối lưu giao dịch. |
| Repositories | Tìm `Showtime`, `Movie`, `Room`, `Seat`, `Member`, `Ticket`, `Invoice`, `User`; chuẩn bị thay đổi dữ liệu và chỉ mục. Không tự quyết định quyền hay ưu đãi. |
| Models | Giữ một đối tượng dữ liệu như `Seat`, `Ticket`, `Invoice`, `Member`; liên hệ với nhau qua mã. |
| `TransactionManager` dự kiến | Lưu các file bị ảnh hưởng cùng một giao dịch và phục hồi nếu quá trình ghi thất bại/gián đoạn. |
| `data/*.txt` | Nguồn dữ liệu bền vững sau khi ứng dụng đóng. |

Luồng gọi chính: **UI → BookingService → Repositories → tiện ích lưu file**. Core không phụ thuộc Qt.

## 2. Dữ liệu tạm trước khi xác nhận thu tiền

Một lần chọn vé cần nhớ `showtimeId`, danh sách các ghế đang chọn, `ticketType` của từng ghế, `memberId` tùy chọn và kết quả xem trước giá. Những dữ liệu này **chỉ ở bộ nhớ**, chưa phải `Ticket`/`Invoice` đã bán và chưa thay đổi điểm thành viên.

Stack giữ thứ tự **mã ghế được chọn thêm** để Undo. Danh sách ghế đang chọn là dữ liệu riêng để hiển thị, kiểm tra trùng trong giỏ và tính tiền; stack không thay thế danh sách đó. Mỗi lần chọn ghế thành công thì thêm ghế vào danh sách tạm và `push(seatId)` vào stack. Vì `Stack::push()` hiện trả `void` và im lặng khi đầy, Service phải kiểm tra `isFull()` trước khi ghi lựa chọn vào cả hai nơi.

Chỉ thao tác **chọn thêm ghế** mới đẩy vào stack; xem phim, đổi loại vé, gắn thành viên và xem lại giá không đẩy vào stack. Bản đầu chỉ hỗ trợ bỏ lựa chọn theo thứ tự Undo; nếu cho phép bỏ một ghế bất kỳ, cần thiết kế lại cách đồng bộ stack. Đổi suất hoặc bỏ giao dịch xóa toàn bộ lựa chọn tạm và làm rỗng stack.

## 3. Quy trình từng bước

### 3.1. Bắt đầu và chọn suất

1. UI gửi mã suất mà nhân viên chọn.
2. Service kiểm tra tài khoản còn hoạt động và thuộc vai trò Admin/Staff; tìm suất, phim và phòng liên quan. Suất phải `SCHEDULED`, chưa bắt đầu; dữ liệu tham chiếu phải tồn tại và hợp lệ.
3. Lấy danh sách `Seat` của phòng để dựng sơ đồ. Một ghế được xem là đã bán cho suất khi có vé `SOLD` cùng cặp `(showtimeId, seatId)`; trạng thái bán không nằm trong `Seat`.
4. Nếu chuyển từ suất khác, xóa giỏ và stack cũ. Chỉ hiển thị các vị trí và trạng thái sau khi kiểm tra dữ liệu thành công.

### 3.2. Chọn ghế và hoàn tác

1. Với một mã ghế, Service kiểm tra ghế thuộc phòng của suất, chưa có trong giỏ, chưa có vé `SOLD` của suất và stack chưa đầy.
2. Nếu hợp lệ, thêm ghế vào giỏ rồi đẩy `seatId` vào stack. Ghế đôi là **một** `seatId`, **một** vé cho hai người, không tách làm hai mã.
3. Khi bấm **Undo**, nếu stack không rỗng, đọc `top()->data`, bỏ ghế đó khỏi giỏ rồi `pop()`. UI nhận giỏ mới để vẽ lại. Undo không ghi file và không hoàn tiền.
4. Khi bấm **Bỏ giao dịch**, xóa toàn bộ giỏ và pop stack tới khi rỗng. Không tạo vé, hóa đơn hoặc thay điểm. Đây là cách bỏ **cả lần chọn vé trước khi thu tiền**.

Ví dụ: chọn `A01`, `A02`, `B01` thì đỉnh stack là `B01`. Undo một lần bỏ `B01`; Bỏ giao dịch sẽ xóa cả `A01` và `A02` còn lại. Nếu nhân viên đã đổi loại vé của `A02`, Undo vẫn bỏ ghế vừa chọn gần nhất, không hoàn tác riêng việc đổi loại vé.

### 3.3. Loại vé, thành viên và xem trước giá

1. Nhân viên chọn `REGULAR` hoặc `STUDENT` cho từng ghế. Loại vé độc lập với loại ghế `STANDARD`/`VIP`/`COUPLE`. Với vé học sinh, nhân viên phải kiểm tra giấy tờ; ghế đôi loại học sinh yêu cầu cả hai người đủ điều kiện.
2. Có thể gắn tối đa một thành viên cho hóa đơn. Service tra `memberId` nếu có; không cho UI sửa điểm trực tiếp, kể cả khi model `Member` có `setPoints()`.
3. Giá gốc phụ thuộc loại ghế: thường 70.000, VIP 100.000, ghế đôi 160.000 VND/vé theo cấu hình mặc định. Giảm học sinh 10%, thành viên 5%; nếu đủ cả hai, lấy **mức cao hơn**, không cộng dồn.
4. Trên từng vé: `discountAmount = basePrice × tỷ lệ / 100`; `paidAmount = basePrice − discountAmount`; `pointsEarned = phần nguyên(paidAmount / 10000)` khi có thành viên, nếu không thì bằng 0. Giá dùng `double`, giữ phần thập phân. `Invoice.totalAmount` là tổng `paidAmount` ban đầu của từng vé.
5. UI hiển thị giỏ, loại vé, giá từng vé, tổng tiền và điểm dự kiến để nhân viên xem lại. Đây chỉ là **bản xem trước**; chưa lưu hoặc cộng điểm.

## 4. Xác nhận thu tiền và lưu giao dịch

Khi nhân viên xác nhận đã thu tiền tại quầy, Service **kiểm tra lại** ngay trước khi tạo dữ liệu; không dùng kết quả xem trước cũ làm căn cứ duy nhất:

1. Kiểm tra quyền và trạng thái người bán; giỏ phải có ít nhất một ghế và tất cả vé cùng một suất.
2. Kiểm tra suất vẫn `SCHEDULED` và chưa bắt đầu; từng ghế vẫn tồn tại trong phòng, không trùng nhau và chưa có vé `SOLD` cùng suất. Kiểm tra thành viên nếu được chọn và xác nhận độ tuổi/điều kiện vé học sinh.
3. Tính lại giá, tiền giảm, tiền thực trả và điểm từng vé; kiểm tra giá trị tiền/điểm hợp lệ, điểm cộng không tràn kiểu `int`. Lấy thời điểm lập hóa đơn và cấp `invoiceId`, `ticketId` mới từ bộ đếm, không tái sử dụng mã.
4. Chuẩn bị **một** `Invoice` và các `Ticket` trạng thái `SOLD` trong bộ nhớ tạm. Mọi vé dùng cùng `showtimeId`, tham chiếu `invoiceId` vừa cấp; các trường hủy rỗng và `refundAmount = 0`. Hóa đơn ghi `staffId` là `userId` người bán thực tế; `memberId` để rỗng nếu không gắn thành viên.
5. Nếu có thành viên, tính điểm mới từ tổng `pointsEarned` của các vé. `Member::setPoints()` (nếu người học thêm) chỉ được Service gọi sau khi kiểm tra số điểm mới; không cho nhân viên nhập số điểm tùy ý.
6. Gửi bản dữ liệu dự kiến tới cơ chế giao dịch để lưu **`invoices.txt`, `tickets.txt`, `members.txt` khi có thành viên, và `sequences.txt`** như một đơn vị. Không công bố thay đổi lên dữ liệu đang dùng khi chưa commit.
7. **Sau commit**, cập nhật dữ liệu/chỉ mục trong bộ nhớ, xóa giỏ và stack, rồi thông báo bán vé thành công cùng mã hóa đơn. Nếu lỗi trước commit, phục hồi dữ liệu cũ và báo chưa hoàn tất; nếu phục hồi thất bại, chặn ghi tiếp. Quy trình chi tiết của cơ chế lưu nằm trong [data_format.md](TongQuan/data_format.md).

Ứng dụng chỉ ghi nhận **xác nhận thu tiền thủ công** ở quầy, không tích hợp cổng thanh toán. Cách xử lý tình huống tiền mặt đã nhận nhưng ghi file thất bại phải được hướng dẫn ở UI/quy trình vận hành khi triển khai; không được thông báo bán thành công nếu giao dịch dữ liệu chưa commit.

## 5. Trường hợp lỗi cần thông báo rõ

| Tình huống | Cách xử lý dự kiến |
| --- | --- |
| Chưa chọn ghế, ghế không thuộc phòng, ghế trùng trong giỏ hoặc stack đầy | Không nhận lựa chọn/không xác nhận bán; giữ giỏ và stack nhất quán, báo lý do. |
| Ghế đã có vé `SOLD`, suất bị hủy hoặc đã bắt đầu | Chặn bán; tải lại trạng thái để nhân viên chọn lại nếu cần. |
| Không tìm được thành viên hoặc điều kiện vé học sinh/độ tuổi chưa được xác nhận | Chặn xác nhận, yêu cầu kiểm tra lại. |
| Mã sắp cấp tràn, số điểm mới tràn, dữ liệu tham chiếu sai hoặc file dữ liệu lỗi | Chặn ghi và báo lỗi cụ thể; không tự bỏ qua bản ghi lỗi. |
| Lưu nhiều file thất bại trước commit | Phục hồi toàn bộ; không tạo vé/hóa đơn/điểm mới trong dữ liệu đang dùng. |

## 6. Ranh giới với hủy vé

Undo và Bỏ giao dịch ở đây chỉ áp dụng **trước khi xác nhận thu tiền**. Sau khi bán thành công, không dùng stack để xóa lịch sử. Hủy vé là nghiệp vụ riêng của Admin: kiểm tra vé `SOLD` và thời điểm trước suất, lưu lý do/người hủy/tiền hoàn, thu hồi điểm và giữ hóa đơn/vé trong lịch sử. `BookingService` có thể điều phối nghiệp vụ hủy về sau, nhưng phải là một luồng khác với Undo lựa chọn tạm.

## 7. Thứ tự triển khai khi đến bước bán vé

1. Hoàn thiện và kiểm thử cấu trúc dữ liệu, repository, tiện ích thời gian/giá và lưu giao dịch nhiều file.
2. Cài trạng thái chọn ghế tạm, chọn thêm, Undo và Bỏ giao dịch; thử các trường hợp giỏ rỗng, stack đầy và đổi suất.
3. Cài xem trước giá, ưu đãi và điểm trên từng vé; đối chiếu các trường trong `Ticket`/`Invoice`.
4. Cài xác nhận bán với kiểm tra lại điều kiện và commit nhiều file; thử lỗi ghi và phục hồi.
5. Sau khi phần C++ thuần chạy đúng mới nối `BookingView`/`SeatMapView` trong Qt.

Tài liệu này **không bắt đầu Bước 10** và không xác nhận `BookingService` đã có mã; nhật ký bước chỉ được tạo khi người học yêu cầu bắt đầu triển khai bước đó.
