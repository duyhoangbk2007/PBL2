# Quy tắc nghiệp vụ

Đây là thiết kế mục tiêu của bản PBL2. Giá và tỷ lệ dưới đây là mặc định mô phỏng được chọn cho dự án, có thể chỉnh trong cấu hình.

## 1. Quyền và phạm vi

- Admin quản lý danh mục, nhân viên, toàn bộ lịch sử, báo cáo, bán và hủy vé.
- Staff bán vé, quản lý thông tin liên hệ thành viên, xem hóa đơn của mình trong ngày và tra cứu hóa đơn cũ của mình theo mã.
- Mọi tài khoản dùng chung mô hình User với role ADMIN/STAFF; không cần lớp con Admin/Staff. Service kiểm tra quyền mỗi nghiệp vụ. Tài khoản vô hiệu hóa không được đăng nhập. Chức năng sửa Staff không được đổi vai trò.
- Theo thiết kế RBAC, quyền thao tác được tra từ `role_permissions.txt` bằng cặp `(role, permissionCode)`. Backend kiểm tra quyền qua `AuthService` mỗi lần thực hiện nghiệp vụ; GUI có thể dùng quyền để điều chỉnh hiển thị nhưng không thay thế kiểm tra ở Backend. Vai trò `ADMIN` không tự động có mọi quyền nếu file thiếu bản ghi. Các giới hạn nghiệp vụ và phạm vi dữ liệu ở các mục dưới vẫn bắt buộc, kể cả khi có mã quyền tương ứng.
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

Bảng hiện gồm 31 mã quyền; `role_permissions.txt` mặc định có 31 cặp cho Admin và 10 cặp cho Staff. `PRICING_VIEW` cho xem bảng giá/giá gốc, `PRICING_UPDATE` cho sửa giá ghế và tỷ lệ ưu đãi. Bảng mô tả quyền được thống nhất, không xác nhận mọi nghiệp vụ tương ứng đã có code. Quyền thao tác được quyết định bởi các cặp hợp lệ đang nạp, không tự thêm cặp còn thiếu. `INVOICE_VIEW_OWN` vẫn phải giới hạn hóa đơn theo người lập và điều kiện ngày/mã ở mục lịch sử; `TICKET_CANCEL` vẫn phải tuân thủ quy tắc chỉ Admin ở mục hủy vé.

Chưa đăng nhập hoặc tài khoản đã bị vô hiệu hóa thì từ chối nghiệp vụ được bảo vệ; `AuthService` cần tra lại trạng thái tài khoản khi kiểm tra quyền. Nếu thiếu file quyền, file sai định dạng, chứa cặp trùng hoặc mã quyền không tồn tại, phải báo lỗi và từ chối truy cập, không dùng bảng quyền nạp dở hay cấp quyền dự phòng. File quyền rỗng không cấp quyền nào. Phiên bản này chỉ đọc bảng quyền; **chưa triển khai Admin cấp/thu hồi quyền qua GUI hoặc API**, và thay đổi file ngoài ứng dụng chỉ có hiệu lực sau khi nạp lại/khởi động lại. Chữ ký API và loại ngoại lệ cụ thể được chốt khi triển khai code.

## 2. Phim, phòng và suất chiếu

- Phim có tên, thể loại, thời lượng phút dương, độ tuổi tối thiểu không âm, ngày bắt đầu/kết thúc chiếu và cờ hoạt động.
- Ngày bắt đầu không lớn hơn ngày kết thúc. Trước khoảng ngày là sắp chiếu; trong khoảng bao gồm hai đầu là đang chiếu; sau khoảng hoặc bị vô hiệu hóa là ngừng chiếu.
- Nhân viên xác nhận khách đủ tuổi tại quầy trước thanh toán; không lưu ngày sinh khách trong bản PBL2.
- Phòng dùng mảng hai chiều, ô không có ghế là ô rỗng. Ghế có mã duy nhất trong phòng, hàng, cột và loại STANDARD/VIP/COUPLE; không lưu trạng thái đã bán.
- Ghế đôi là một vị trí cho hai người, một vé nguyên cặp. Số vị trí bán vé khác số chỗ ngồi thực tế.
- Trong dữ liệu, ghế đôi có đúng một seatId và một vị trí logic `(row, column)`; không tách thành hai mã ghế. UI hiển thị ô ghế đôi rộng hơn. Một vé ghế đôi được tính là một vé nhưng hai người khi thống kê số người.
- Suất có trạng thái SCHEDULED/CANCELLED; trạng thái đã kết thúc suy ra từ thời gian.
- Khi tạo/sửa suất: phim/phòng hoạt động, bắt đầu ở tương lai, ngày chiếu thuộc khoảng phát hành phim.
- Kết thúc = bắt đầu + thời lượng phim khi tạo/sửa suất. Lưu thời điểm kết thúc riêng; đổi thời lượng phim không tự đổi lịch cũ.
- Khoảng nghỉ tối thiểu 15 phút; code hiện tại khai báo `DateTime::BREAK_MINUTES = 15`. Hai suất cùng phòng trùng nếu các khoảng `[bắt đầu, kết thúc + khoảng nghỉ)` giao nhau. Bỏ qua suất hủy và chính suất đang sửa.
- Không sửa phim/phòng/thời gian của suất đã từng có vé, kể cả vé hủy; không sửa suất đã bắt đầu.
- Chỉ hủy suất chưa bắt đầu và không còn vé SOLD. Admin phải xử lý vé trước, không ngầm hủy hàng loạt.
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
- `PricingService` hiện tính tiền giảm, tiền trả và điểm nhưng chưa cộng/thu hồi điểm trên `Member`, chưa tạo vé hoặc ghi snapshot giá vào `Ticket`; các thao tác này thuộc luồng bán/hủy vé chưa hoàn thiện.
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
6. Tạo một hóa đơn không rỗng với các vé cùng suất; ghi lần lượt hóa đơn, vé, điểm và bộ đếm mã vào các file TXT liên quan.
7. Kiểm tra từng lần ghi; chỉ thông báo thành công sau khi tất cả lần ghi thành công. Nếu có lỗi, báo rõ file bị lỗi và dữ liệu có thể không nhất quán để người dùng xử lý.

Ghế đang chọn chỉ nằm trong bộ nhớ. Bỏ giao dịch trước thanh toán không lưu vé/hóa đơn/điểm. Không tích hợp cổng thanh toán; xác nhận thu/hoàn tiền là thao tác tại quầy.

## 5. Hủy vé

- Chỉ Admin, chỉ vé SOLD, trước giờ bắt đầu suất.
- Cho phép hủy từng vé trong hóa đơn. Hoàn toàn bộ giá thực trả; lưu thời điểm, người hủy, lý do và tiền hoàn.
- Chuyển vé sang CANCELLED, trả ghế cho suất đó và thu hồi đúng điểm đã cộng; kiểm tra từng lần ghi liên quan và báo lỗi nếu lưu chưa hoàn tất.
- Không xóa vé, không đổi tổng tiền ban đầu của hóa đơn, không hủy lần hai.
- Trạng thái hóa đơn không lưu lặp mà suy ra bằng một hàm nghiệp vụ dùng chung: ACTIVE nếu mọi vé còn SOLD, PARTIALLY_CANCELLED nếu có cả SOLD và CANCELLED, CANCELLED nếu mọi vé đã hủy. Tiền còn lại = tổng tiền ban đầu − tổng tiền hoàn.
- Giữ pointsEarned ban đầu trên vé để đối chiếu dù đã thu hồi điểm. Nếu điểm thành viên không đủ thu hồi, chặn và báo dữ liệu không nhất quán, không ép về 0.

## 6. Lịch sử và báo cáo

- Khoảng thời gian dùng `[từ, đến)`. Ngày cuối người dùng chọn được chuyển thành 00:00 ngày kế tiếp.
- Lịch sử lọc theo thời điểm lập hóa đơn; dùng thêm `invoiceId` để phân biệt hóa đơn cùng giây. Có thể lọc/sắp xếp bằng STL trên dữ liệu đã nạp.
- Báo cáo dòng tiền: tiền bán theo thời điểm hóa đơn, tiền hoàn theo thời điểm hủy; doanh thu thuần = tiền bán − tiền hoàn. Có thể âm trong ngày có nhiều khoản hoàn giao dịch cũ.
- Đếm vé tạo trong kỳ theo thời điểm hóa đơn, vé hủy trong kỳ theo thời điểm hủy. Một vé đôi tính một vé; thống kê số người thì tính hai chỗ.
- Top phim theo kỳ bán: chỉ lấy vé thuộc hóa đơn lập trong kỳ còn SOLD tại lúc xem; cộng giá thực trả hoặc đếm vé. Chỉ tiêu này có thể khác doanh thu dòng tiền nếu hoàn ở kỳ khác.
- Xếp hạng phim giảm dần theo chỉ tiêu; bằng nhau thì mã phim tăng dần. Có thể dùng `std::sort` trên kết quả đã lọc.

## 7. Sửa/xóa và toàn vẹn

- Mã bất biến, duy nhất, không tái sử dụng. Chỉ xóa hẳn dữ liệu chưa được tham chiếu; vé/hóa đơn không xóa.
- Phim/phòng còn suất tương lai chưa hủy thì chưa được ngừng hoạt động. Sửa ngày phát hành không được làm suất tương lai nằm ngoài khoảng mới.
- Nhân viên đã lập hóa đơn chỉ vô hiệu hóa, giữ mã. Không xóa/vô hiệu hóa Admin cuối cùng hoạt động nếu bổ sung chức năng quản lý Admin.
- Giá, thời gian suất và mã liên kết lịch sử được giữ nguyên. Tên phim/phòng/nhân viên hiển thị theo danh mục hiện tại; bản đầu chưa lưu lịch sử đổi tên.
- movieId là danh tính bất biến của một phim. Chỉ sửa thông tin của cùng phim; không biến bản ghi cũ thành một phim khác để tái sử dụng mã. Phim khác phải được tạo với movieId mới.
- Lỗi dữ liệu hoặc tham chiếu mất phải báo file/dòng và chặn ghi, không bỏ qua dòng lỗi rồi ghi đè.
- Khi đọc dữ liệu lỗi hoặc thiếu tham chiếu, báo file/dòng và chặn thao tác ghi có thể làm mất dữ liệu. Ghi nhiều file TXT tuần tự không nguyên tử; xem [định dạng dữ liệu](data_format.md).
