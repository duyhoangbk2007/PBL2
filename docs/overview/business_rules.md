# Quy tắc nghiệp vụ

Đây là thiết kế mục tiêu của bản PBL2. Giá và tỷ lệ dưới đây là mặc định mô phỏng được chọn cho dự án, có thể chỉnh trong cấu hình.

## 1. Quyền và phạm vi

- Mọi tài khoản dùng chung Model `User` với role `ADMIN`/`STAFF`; không cần lớp con. Role là khóa tra bảng quyền và thuộc tính dữ liệu, **không tự cấp quyền nghiệp vụ**. Tài khoản vô hiệu hóa không được đăng nhập; sửa tài khoản nhân viên không được đổi role.
- Quyền thao tác được tra từ `role_permissions.txt` bằng cặp `(role, permissionCode)`. Service dùng `AuthService::requirePermission()` trước mỗi nghiệp vụ được bảo vệ; GUI có thể dùng `hasPermission()` để điều chỉnh hiển thị nhưng không thay thế kiểm tra ở Service. Không dùng `requireAdmin()` hoặc so role cứng để cấp quyền. Không có cặp quyền thì từ chối, kể cả với tài khoản `ADMIN`.
- `INVOICE_VIEW_OWN` chỉ cho xem hóa đơn do chính tài khoản lập và vẫn áp dụng giới hạn ngày/mã. `INVOICE_VIEW_ALL` cho phạm vi rộng khi bảng quyền cấp. Quyền `TICKET_CANCEL` không miễn các điều kiện về vé, suất, hoàn tiền và điểm.
- Service kiểm tra Permission, quy tắc nghiệp vụ và điều phối nhiều nguồn dữ liệu. Repository xử lý CRUD, parse/serialize, lưu trữ, liên kết đối tượng và tính toàn vẹn dữ liệu; không phân quyền người dùng. Model giữ trạng thái và quan hệ; GUI không tự quyết định hợp lệ nghiệp vụ.
- Không quản lý ca, online, thanh toán điện tử, hạng thẻ hoặc đổi điểm.
- Bản PBL2 chạy như ứng dụng tại quầy trên một bộ dữ liệu TXT. Nếu sau này cho phép nhiều tiến trình cùng ghi, cần thiết kế cơ chế đồng bộ riêng trước khi sử dụng.

### Bảng quyền RBAC mặc định theo thiết kế

| Permission | ADMIN | STAFF |
| --- | --- | --- |
| `MOVIE_VIEW` | Có | Có |
| `MOVIE_ADD` | Có | Không |
| `MOVIE_UPDATE` | Có | Không |
| `MOVIE_DELETE` | Có | Không |
| `ROOM_VIEW` | Có | Có |
| `ROOM_ADD` | Có | Không |
| `ROOM_UPDATE` | Có | Không |
| `ROOM_DELETE` | Có | Không |
| `SEAT_VIEW` | Có | Có |
| `SEAT_ADD` | Có | Không |
| `SEAT_UPDATE` | Có | Không |
| `SEAT_DELETE` | Có | Không |
| `SHOWTIME_VIEW` | Có | Có |
| `SHOWTIME_ADD` | Có | Không |
| `SHOWTIME_UPDATE` | Có | Không |
| `SHOWTIME_CANCEL` | Có | Không |
| `SHOWTIME_DELETE` | Có | Không |
| `STAFF_VIEW` | Có | Không |
| `STAFF_ADD` | Có | Không |
| `STAFF_UPDATE` | Có | Không |
| `STAFF_DEACTIVATE` | Có | Không |
| `MEMBER_VIEW` | Có | Có |
| `MEMBER_ADD` | Có | Có |
| `MEMBER_UPDATE` | Có | Có |
| `TICKET_SELL` | Có | Có |
| `TICKET_CANCEL` | Có | Không |
| `INVOICE_VIEW_OWN` | Có | Có |
| `INVOICE_VIEW_ALL` | Có | Không |
| `REPORT_VIEW` | Có | Không |
| `PRICING_VIEW` | Có | Có |
| `PRICING_UPDATE` | Có | Không |

Bảng hiện gồm 31 mã quyền; file `data/role_permissions.txt` hiện có 31 cặp cho `ADMIN` và 10 cặp cho `STAFF`. Hai cột role chỉ mô tả **dữ liệu mặc định**, không phải điều kiện quyền viết cứng. `PRICING_VIEW` cho xem bảng giá/giá gốc, `PRICING_UPDATE` cho sửa giá ghế và tỷ lệ ưu đãi. Có mã quyền không có nghĩa mọi nghiệp vụ tương ứng đã hoàn thiện.

Chưa đăng nhập hoặc tài khoản đã bị vô hiệu hóa thì từ chối nghiệp vụ được bảo vệ; `AuthService` tra lại trạng thái tài khoản khi kiểm tra quyền. Thiếu file quyền, sai định dạng hoặc cặp trùng phải từ chối truy cập, không dùng bảng nạp dở hay cấp quyền dự phòng; file rỗng không cấp quyền nào. **Khoảng trống hiện tại:** `RolePermissionRepository::load()` chưa kiểm tra mã quyền có thuộc danh mục 31 mã; quy tắc từ chối mã lạ cần được triển khai và kiểm thử sau khi nhóm chốt nguồn danh mục. Phiên bản này chỉ đọc bảng quyền; chưa có API/GUI cấp hoặc thu hồi quyền, thay đổi file ngoài ứng dụng chỉ có hiệu lực sau khi nạp lại/khởi động lại.

## 2. Phim, phòng và suất chiếu

- Phim có tên, thể loại, thời lượng phút dương, độ tuổi tối thiểu không âm, ngày bắt đầu/kết thúc chiếu và cờ hoạt động.
- Ngày bắt đầu không lớn hơn ngày kết thúc. Trước khoảng ngày là sắp chiếu; trong khoảng bao gồm hai đầu là đang chiếu; sau khoảng hoặc bị vô hiệu hóa là ngừng chiếu.
- Nhân viên xác nhận khách đủ tuổi tại quầy trước thanh toán; không lưu ngày sinh khách trong bản PBL2.
- Sơ đồ phòng biểu diễn theo hàng/cột; ô không có ghế là ô rỗng về mặt nghiệp vụ. Model `Room` hiện giữ danh sách liên kết ghế, không lưu mảng hai chiều. Ghế có mã duy nhất trong phòng, hàng, cột và loại STANDARD/VIP/COUPLE; không lưu trạng thái đã bán.
- Ghế đôi là một vị trí cho hai người, một vé nguyên cặp. Số vị trí bán vé khác số chỗ ngồi thực tế.
- Trong dữ liệu, ghế đôi có đúng một seatId và một vị trí logic `(row, column)`; không tách thành hai mã ghế. UI hiển thị ô ghế đôi rộng hơn. Một vé ghế đôi được tính là một vé nhưng hai người khi thống kê số người.
- Suất có trạng thái SCHEDULED/CANCELLED; trạng thái đã kết thúc suy ra từ thời gian.
- Khi tạo/sửa suất: phim/phòng hoạt động, bắt đầu ở tương lai, ngày chiếu thuộc khoảng phát hành phim.
- Kết thúc = bắt đầu + thời lượng phim khi tạo/sửa suất. Lưu thời điểm kết thúc riêng; đổi thời lượng phim không tự đổi lịch cũ.
- Khoảng nghỉ tối thiểu 15 phút; code hiện tại khai báo `DateTime::BREAK_MINUTES = 15`. Hai suất cùng phòng trùng nếu các khoảng `[bắt đầu, kết thúc + khoảng nghỉ)` giao nhau. Bỏ qua suất hủy và chính suất đang sửa.
- Không sửa phim/phòng/thời gian của suất đã từng có vé, kể cả vé hủy; không sửa suất đã bắt đầu.
- Chỉ hủy suất chưa bắt đầu và không còn vé SOLD. Người có Permission hủy suất phải xử lý vé trước, không ngầm hủy hàng loạt.
- Chỉ xóa hẳn suất chiếu khi suất **chưa từng phát sinh Ticket**, kể cả Ticket đã hủy, và **không có Invoice tham chiếu**. Trước khi gọi `ShowtimeRepository::removeById()`, `ShowtimeService` phải kiểm tra cả lịch sử Ticket và Invoice theo mã suất; trách nhiệm này sẽ được hoàn thiện sau khi refactor `TicketRepository` và `InvoiceRepository`. Quy tắc xóa này khác thao tác hủy suất ở trên.
- Chặn đổi sơ đồ phòng nếu có suất tương lai chưa hủy đã từng phát sinh vé. Không đổi/xóa mã ghế từng xuất hiện trong vé; sơ đồ mới không tương thích cần tạo phòng mới.

## 3. Giá, ưu đãi và thành viên

| Giá trị mặc định của `Pricing()` | Mức áp dụng |
| --- | --- |
| Ghế thường | 70.000 VND/vé |
| Ghế VIP | 100.000 VND/vé |
| Ghế đôi | 160.000 VND/vé cho hai người |
| Giảm học sinh | 10% |
| Giảm thành viên | 5% |
| Tích điểm | 1 điểm/10.000 VND thực trả trên từng vé |

`Pricing` hiện giữ ba giá và hai tỷ lệ trên; cấu hình vận hành được `PricingRepository` nạp từ `pricing.txt` gồm năm dòng `KEY|VALUE`. `PricingService` cho phép xem bằng `PRICING_VIEW`, sửa từng giá/tỷ lệ bằng `PRICING_UPDATE`; cập nhật thành công được ghi xuống file trước khi thay cấu hình trong RAM. Quy tắc 1 điểm/10.000 VND là hằng số trong hàm tính điểm, không phải trường thứ sáu của `pricing.txt`. Việc cấp quyền thực tế phụ thuộc bảng RBAC đang nạp; không suy ra chỉ từ vai trò mặc định.

- Tiền dùng `double` theo đơn vị VND, có thể có phần thập phân theo quyết định hiện tại của dự án. Cần xử lý sai số số thực khi đối chiếu tổng tiền và khi đọc/ghi dữ liệu.
- Loại vé REGULAR/STUDENT độc lập với loại ghế và thành viên.
- Nhân viên kiểm tra giấy tờ học sinh; ghế đôi loại học sinh yêu cầu cả hai người đủ điều kiện.
- Lấy tỷ lệ giảm cao nhất đủ điều kiện, không cộng dồn. Tiền giảm = `(giá gốc × tỷ lệ) / 100` và giữ phần thập phân; thực trả = giá gốc − tiền giảm.
- Điểm mỗi vé = phần nguyên `thực trả / 10000` nếu có thành viên, ngược lại bằng 0. Điểm hóa đơn là tổng điểm từng vé.
- `PricingService` tính tiền giảm, tiền trả và điểm; `BookingService` hiện dùng các kết quả này để tạo vé, lưu snapshot giá/điểm, cộng điểm khi bán và thu hồi đúng điểm vé khi hủy. Lưu nhiều TXT vẫn không phải transaction đa file.
- Thành viên có mã, họ tên, số điện thoại duy nhất và điểm không âm; khởi tạo 0 điểm. Không chỉnh điểm trực tiếp.
- Chỉ báo cộng điểm thành công khi các lần ghi liên quan đã thành công. Không xóa thành viên đã có hóa đơn.
- Lưu giá gốc, tiền giảm, thực trả và điểm trên vé tại thời điểm bán; đổi cấu hình không làm thay đổi lịch sử.
- Bản PBL2 chưa có đổi điểm nên số điểm luôn đủ để thu hồi khi dữ liệu hợp lệ. Nếu bổ sung đổi điểm sau này, phải thiết kế sổ giao dịch điểm hoặc quy tắc số dư điều chỉnh trước khi giữ cơ chế chặn hủy do thiếu điểm.

## 4. Bán vé và undo

1. Chọn suất SCHEDULED chưa bắt đầu và các ghế hợp lệ.
2. Ghế trống nếu không có vé SOLD cùng `(showtimeId, seatId)`.
3. Mỗi lần chọn ghế đẩy mã ghế vào stack. Undo bỏ ghế chọn gần nhất. Bản đầu chỉ hỗ trợ bỏ lựa chọn theo thứ tự undo; đổi suất hoặc bỏ giao dịch xóa danh sách và stack.
4. Chọn loại vé từng ghế, gắn tối đa một thành viên và xem lại giá.
5. Xác nhận thu tiền tại quầy; service kiểm tra lại quyền, giờ chiếu, ghế, thành viên và việc xác nhận độ tuổi.
6. Tạo một hóa đơn không rỗng với các vé cùng suất; ghi hóa đơn, vé và điểm vào các file TXT liên quan. ID hiện được cấp từ các mã đang nạp, không ghi bộ đếm riêng.
7. Kiểm tra từng lần ghi; chỉ thông báo thành công sau khi tất cả lần ghi thành công. Nếu có lỗi, báo rõ file bị lỗi và dữ liệu có thể không nhất quán để người dùng xử lý.

Ghế đang chọn chỉ nằm trong bộ nhớ. Bỏ giao dịch trước thanh toán không lưu vé/hóa đơn/điểm. Không tích hợp cổng thanh toán; xác nhận thu/hoàn tiền là thao tác tại quầy.

## 5. Hủy vé

- Chỉ tài khoản có `TICKET_CANCEL`, chỉ vé SOLD, trước giờ bắt đầu suất. Bảng quyền hiện chỉ cấp mã này cho `ADMIN`; role không phải điều kiện cấp quyền viết cứng.
- Cho phép hủy từng vé trong hóa đơn. Hoàn toàn bộ giá thực trả; lưu thời điểm, người hủy, lý do và tiền hoàn.
- Chuyển vé sang CANCELLED, trả ghế cho suất đó và thu hồi đúng điểm đã cộng; kiểm tra từng lần ghi liên quan và báo lỗi nếu lưu chưa hoàn tất.
- Không xóa vé, không đổi tổng tiền ban đầu của hóa đơn, không hủy lần hai.
- Trạng thái hóa đơn không lưu lặp mà suy ra bằng một hàm nghiệp vụ dùng chung: ACTIVE nếu mọi vé còn SOLD, PARTIALLY_CANCELLED nếu có cả SOLD và CANCELLED, CANCELLED nếu mọi vé đã hủy. Tiền còn lại = tổng tiền ban đầu − tổng tiền hoàn.
- Giữ pointsEarned ban đầu trên vé để đối chiếu dù đã thu hồi điểm. Nếu điểm thành viên không đủ thu hồi, chặn và báo dữ liệu không nhất quán, không ép về 0.

**Hiện trạng chưa đạt Permission-only:** `BookingService::cancelTicket()` còn gọi `requireAdmin()` trước `requirePermission("TICKET_CANCEL")`; `TicketRepository` chỉ kiểm tra `cancelledBy` có **định dạng** ID `USR`, không tự cấp quyền hoặc tra User người hủy. Nhóm cần xác nhận việc thay gate role cứng bằng Permission có giữ đúng ý nghĩa nghiệp vụ khi bảng quyền được chỉnh về sau.

## 6. Lịch sử và báo cáo

- Khoảng thời gian dùng `[từ, đến)`. Ngày cuối người dùng chọn được chuyển thành 00:00 ngày kế tiếp.
- Lịch sử lọc theo thời điểm lập hóa đơn; dùng thêm `invoiceId` để phân biệt hóa đơn cùng giây. Có thể lọc/sắp xếp bằng STL trên dữ liệu đã nạp.
- Báo cáo dòng tiền: tiền bán theo thời điểm hóa đơn, tiền hoàn theo thời điểm hủy; doanh thu thuần = tiền bán − tiền hoàn. Có thể âm trong ngày có nhiều khoản hoàn giao dịch cũ.
- Đếm vé tạo trong kỳ theo thời điểm hóa đơn, vé hủy trong kỳ theo thời điểm hủy. Một vé đôi tính một vé; thống kê số người thì tính hai chỗ.
- Top phim theo kỳ bán: chỉ lấy vé thuộc hóa đơn lập trong kỳ còn SOLD tại lúc xem; cộng giá thực trả hoặc đếm vé. Chỉ tiêu này có thể khác doanh thu dòng tiền nếu hoàn ở kỳ khác.
- Xếp hạng phim giảm dần theo chỉ tiêu; bằng nhau thì mã phim tăng dần. Có thể dùng `std::sort` trên kết quả đã lọc.

**Triển khai hiện tại của Bước 11:** `InvoiceService` hiện chọn `INVOICE_VIEW_ALL` hoặc `INVOICE_VIEW_OWN` theo role cứng: phạm vi OWN chỉ xem hóa đơn do mình lập, tra hóa đơn cũ theo mã nhưng chỉ liệt kê lịch sử trong ngày hiện tại. Đây là chỗ chưa đạt Permission-only; thiết kế đích chọn phạm vi theo Permission đang có và vẫn kiểm tra người lập/ngày trong Service. Trạng thái `ACTIVE`/`PARTIALLY_CANCELLED`/`CANCELLED` suy từ Ticket; tiền hoàn cộng trên vé đã hủy, tiền còn lại bằng tổng gốc trừ tiền hoàn. `ReportService` hiện gọi cả `requireAdmin()` và `REPORT_VIEW`; thiết kế đích chỉ cấp quyền qua `REPORT_VIEW`. Cách tính vẫn giữ: tổng bán theo `Invoice::createdAt`, tổng hoàn theo `Ticket::cancelledAt`, doanh thu thuần là hiệu hai tổng; đếm vé bán theo ngày lập hóa đơn kể cả vé sau đó hủy, đếm vé hủy theo ngày hủy. `getMovieRevenue()` và `getTopMovies()` chỉ cộng `paidAmount` của vé còn `SOLD` có hóa đơn lập trong kỳ. `getTopMovies()` gọi `calculateMovieRevenue()` cho từng phim rồi dùng `std::sort()` theo doanh thu giảm dần, hòa theo `movieId` tăng dần; chỉ đưa phim có vé phù hợp vào kết quả.

Các API báo cáo yêu cầu khoảng DateTime hợp lệ `[from, to)`; mã phim không tồn tại hoặc `limit <= 0` là đầu vào sai. Thiếu quyền, Repository chưa load, con trỏ/liên kết vé–hóa đơn–suất sai, tiền từng bản ghi không hợp lệ hoặc dữ liệu hủy sai phải báo ngoại lệ, không trả số liệu giả. **Giới hạn cần xử lý:** code `ReportService` hiện chưa đối chiếu tổng `paidAmount` của toàn bộ Ticket với `Invoice::totalAmount`; test tương ứng đang FAIL. Không coi kiểm thử ReportService hoặc toàn Bước 11 đã hoàn tất.

## 7. Sửa/xóa và toàn vẹn

- Mã của một bản ghi là bất biến và duy nhất trong dữ liệu đang nạp. **Chờ nhóm xác nhận:** quy tắc nghiệp vụ “không tái sử dụng mã đã xóa” chưa được `IdGenerator` bảo đảm vì nó lấy số lớn nhất của các ID còn tồn tại rồi cộng một; xóa bản ghi có số lớn nhất có thể khiến mã đó được cấp lại. Chỉ xóa hẳn dữ liệu chưa được tham chiếu; vé/hóa đơn không xóa.
- Phim/phòng còn suất tương lai chưa hủy thì chưa được ngừng hoạt động. Sửa ngày phát hành không được làm suất tương lai nằm ngoài khoảng mới.
- Nhân viên đã lập hóa đơn chỉ vô hiệu hóa, giữ mã. Không xóa/vô hiệu hóa Admin cuối cùng hoạt động nếu bổ sung chức năng quản lý Admin.
- Giá, thời gian suất và mã liên kết lịch sử được giữ nguyên. Tên phim/phòng/nhân viên hiển thị theo danh mục hiện tại; bản đầu chưa lưu lịch sử đổi tên.
- movieId là danh tính bất biến của một phim. Chỉ sửa thông tin của cùng phim; không biến bản ghi cũ thành một phim khác để tái sử dụng mã. Phim khác phải được tạo với movieId mới.
- Lỗi dữ liệu hoặc tham chiếu mất phải báo file/dòng và chặn ghi, không bỏ qua dòng lỗi rồi ghi đè.
- Khi đọc dữ liệu lỗi hoặc thiếu tham chiếu, báo file/dòng và chặn thao tác ghi có thể làm mất dữ liệu. Ghi nhiều file TXT tuần tự không nguyên tử; xem [định dạng dữ liệu](data_format.md).
