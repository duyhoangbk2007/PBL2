# Quy tắc nghiệp vụ

Đây là thiết kế mục tiêu của bản PBL2. Giá và tỷ lệ dưới đây là mặc định mô phỏng được chọn cho dự án, có thể chỉnh trong cấu hình.

## 1. Quyền và phạm vi

- Admin quản lý danh mục, nhân viên, toàn bộ lịch sử, báo cáo, bán và hủy vé.
- Staff bán vé, quản lý thông tin liên hệ thành viên, xem hóa đơn của mình trong ngày và tra cứu hóa đơn cũ của mình theo mã.
- Service kiểm tra quyền mỗi nghiệp vụ. Tài khoản vô hiệu hóa không được đăng nhập. Chức năng sửa Staff không được đổi vai trò.
- Không quản lý ca, online, thanh toán điện tử, hạng thẻ hoặc đổi điểm.
- Giữ khóa độc quyền bộ dữ liệu suốt phiên ứng dụng, từ chối phiên ghi thứ hai.

## 2. Phim, phòng và suất chiếu

- Phim có tên, thể loại, thời lượng phút dương, độ tuổi tối thiểu không âm, ngày bắt đầu/kết thúc chiếu và cờ hoạt động.
- Ngày bắt đầu không lớn hơn ngày kết thúc. Trước khoảng ngày là sắp chiếu; trong khoảng bao gồm hai đầu là đang chiếu; sau khoảng hoặc bị vô hiệu hóa là ngừng chiếu.
- Nhân viên xác nhận khách đủ tuổi tại quầy trước thanh toán; không lưu ngày sinh khách trong bản PBL2.
- Phòng dùng mảng hai chiều, ô không có ghế là ô rỗng. Ghế có mã duy nhất trong phòng, hàng, cột và loại STANDARD/VIP/COUPLE; không lưu trạng thái đã bán.
- Ghế đôi là một vị trí cho hai người, một vé nguyên cặp. Số vị trí bán vé khác số chỗ ngồi thực tế.
- Suất có trạng thái SCHEDULED/CANCELLED; trạng thái đã kết thúc suy ra từ thời gian.
- Khi tạo/sửa suất: phim/phòng hoạt động, bắt đầu ở tương lai, ngày chiếu thuộc khoảng phát hành phim.
- Kết thúc = bắt đầu + thời lượng phim khi tạo/sửa suất. Lưu thời điểm kết thúc riêng; đổi thời lượng phim không tự đổi lịch cũ.
- Khoảng nghỉ mặc định 15 phút, đặt trong SchedulingConfig.h. Hai suất cùng phòng trùng nếu các khoảng `[bắt đầu, kết thúc + khoảng nghỉ)` giao nhau. Bỏ qua suất hủy và chính suất đang sửa.
- Không sửa phim/phòng/thời gian của suất đã từng có vé, kể cả vé hủy; không sửa suất đã bắt đầu.
- Chỉ hủy suất chưa bắt đầu và không còn vé SOLD. Admin phải xử lý vé trước, không ngầm hủy hàng loạt.
- Chặn đổi sơ đồ phòng nếu có suất tương lai chưa hủy đã từng phát sinh vé. Không đổi/xóa mã ghế từng xuất hiện trong vé; sơ đồ mới không tương thích cần tạo phòng mới.

## 3. Giá, ưu đãi và thành viên

| Giá trị mặc định trong PricingConfig.h | Mức áp dụng |
| --- | --- |
| Ghế thường | 70.000 VND/vé |
| Ghế VIP | 100.000 VND/vé |
| Ghế đôi | 160.000 VND/vé cho hai người |
| Giảm học sinh | 10% |
| Giảm thành viên | 5% |
| Tích điểm | 1 điểm/10.000 VND thực trả trên từng vé |

- Tiền dùng số nguyên 64-bit VND, không dùng số thực.
- Loại vé REGULAR/STUDENT độc lập với loại ghế và thành viên.
- Nhân viên kiểm tra giấy tờ học sinh; ghế đôi loại học sinh yêu cầu cả hai người đủ điều kiện.
- Lấy tỷ lệ giảm cao nhất đủ điều kiện, không cộng dồn. Tiền giảm = phần nguyên `(giá gốc × tỷ lệ) / 100`; thực trả = giá gốc − tiền giảm.
- Điểm mỗi vé = phần nguyên `thực trả / 10000` nếu có thành viên, ngược lại bằng 0. Điểm hóa đơn là tổng điểm từng vé.
- Thành viên có mã, họ tên, số điện thoại duy nhất và điểm không âm; khởi tạo 0 điểm. Không chỉnh điểm trực tiếp.
- Chỉ cộng điểm khi giao dịch lưu thành công. Không xóa thành viên đã có hóa đơn.
- Lưu giá gốc, tiền giảm, thực trả và điểm trên vé tại thời điểm bán; đổi cấu hình không làm thay đổi lịch sử.

## 4. Bán vé và undo

1. Chọn suất SCHEDULED chưa bắt đầu và các ghế hợp lệ.
2. Ghế trống nếu không có vé SOLD cùng `(showtimeId, seatId)`.
3. Mỗi lần chọn ghế đẩy mã ghế vào stack. Undo bỏ ghế chọn gần nhất. Bản đầu chỉ hỗ trợ bỏ lựa chọn theo thứ tự undo; đổi suất hoặc bỏ giao dịch xóa danh sách và stack.
4. Chọn loại vé từng ghế, gắn tối đa một thành viên và xem lại giá.
5. Xác nhận thu tiền tại quầy; service kiểm tra lại quyền, giờ chiếu, ghế, thành viên và việc xác nhận độ tuổi.
6. Tạo một hóa đơn không rỗng với các vé cùng suất; lưu hóa đơn, vé, điểm, bộ đếm mã như một giao dịch.
7. Sau commit mới cập nhật bộ nhớ và thông báo thành công.

Ghế đang chọn chỉ nằm trong bộ nhớ. Bỏ giao dịch trước thanh toán không lưu vé/hóa đơn/điểm. Không tích hợp cổng thanh toán; xác nhận thu/hoàn tiền là thao tác tại quầy.

## 5. Hủy vé

- Chỉ Admin, chỉ vé SOLD, trước giờ bắt đầu suất.
- Cho phép hủy từng vé trong hóa đơn. Hoàn toàn bộ giá thực trả; lưu thời điểm, người hủy, lý do và tiền hoàn.
- Chuyển vé sang CANCELLED, trả ghế cho suất đó và thu hồi đúng điểm đã cộng; lưu cùng giao dịch.
- Không xóa vé, không đổi tổng tiền ban đầu của hóa đơn, không hủy lần hai.
- Trạng thái hóa đơn suy ra từ vé: còn nguyên/hủy một phần/hủy toàn bộ. Tiền còn lại = tổng tiền ban đầu − tổng tiền hoàn.
- Giữ pointsEarned ban đầu trên vé để đối chiếu dù đã thu hồi điểm. Nếu điểm thành viên không đủ thu hồi, chặn và báo dữ liệu không nhất quán, không ép về 0.

## 6. Lịch sử và báo cáo

- Khoảng thời gian dùng `[từ, đến)`. Ngày cuối người dùng chọn được chuyển thành 00:00 ngày kế tiếp.
- Lịch sử lọc thời điểm lập hóa đơn. BST dùng `(createdAt, invoiceId)` để phân biệt hóa đơn cùng giây.
- Báo cáo dòng tiền: tiền bán theo thời điểm hóa đơn, tiền hoàn theo thời điểm hủy; doanh thu thuần = tiền bán − tiền hoàn. Có thể âm trong ngày có nhiều khoản hoàn giao dịch cũ.
- Đếm vé tạo trong kỳ theo thời điểm hóa đơn, vé hủy trong kỳ theo thời điểm hủy. Một vé đôi tính một vé; thống kê số người thì tính hai chỗ.
- Top phim theo kỳ bán: chỉ lấy vé thuộc hóa đơn lập trong kỳ còn SOLD tại lúc xem; cộng giá thực trả hoặc đếm vé. Chỉ tiêu này có thể khác doanh thu dòng tiền nếu hoàn ở kỳ khác.
- Max heap dựng theo bộ lọc, xếp giảm dần theo chỉ tiêu; bằng nhau thì mã phim tăng dần.

## 7. Sửa/xóa và toàn vẹn

- Mã bất biến, duy nhất, không tái sử dụng. Chỉ xóa hẳn dữ liệu chưa được tham chiếu; vé/hóa đơn không xóa.
- Phim/phòng còn suất tương lai chưa hủy thì chưa được ngừng hoạt động. Sửa ngày phát hành không được làm suất tương lai nằm ngoài khoảng mới.
- Nhân viên đã lập hóa đơn chỉ vô hiệu hóa, giữ mã. Không xóa/vô hiệu hóa Admin cuối cùng hoạt động nếu bổ sung chức năng quản lý Admin.
- Giá, thời gian suất và mã liên kết lịch sử được giữ nguyên. Tên phim/phòng/nhân viên hiển thị theo danh mục hiện tại; bản đầu chưa lưu lịch sử đổi tên.
- Lỗi dữ liệu hoặc tham chiếu mất phải báo file/dòng và chặn ghi, không bỏ qua dòng lỗi rồi ghi đè.
- Phục hồi giao dịch dang dở trước khi tải dữ liệu. Xem [định dạng dữ liệu](data_format.md).
