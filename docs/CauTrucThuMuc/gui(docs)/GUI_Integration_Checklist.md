
# GUI Integration Checklist

**Dự án:** PBL2 Cinema
**Nhánh:** feature/b13-hoang-kiet-gui-backend-integration
**Backend:** B11
**GUI hiện tại:** B13
**Trạng thái:** Draft

## 1. Mục tiêu

Tích hợp các màn hình GUI hiện có với Business Service,
không viết lại giao diện khi không cần thiết.

Mỗi chức năng chỉ được đánh dấu hoàn thành khi:
- Gọi đúng Business Service.
- Không truy cập Repository trực tiếp từ GUI.
- Không tự xử lý nghiệp vụ trong GUI.
- Xử lý đúng kết quả và ngoại lệ.
- Không sử dụng dữ liệu DEMO cho giao dịch thật.
- Kiểm thử thành công.

## 2. Quy ước

- [ ] Chưa thực hiện.
- [x] Đã thực hiện và xác minh.

Trạng thái:
- READY: Có thể bắt đầu tích hợp.
- IN_PROGRESS: Đang thực hiện.
- BLOCKED: Phụ thuộc Backend chưa hoàn thiện.
- NEEDS_VERIFICATION: Cần kiểm tra thêm.
- DONE: Hoàn thành và đã kiểm thử.

Không đánh dấu DONE chỉ vì đã viết code.

---

## 3. Cấu hình và khởi tạo

### CMakeLists.txt

- [ ] Bổ sung BookingService.cpp.
- [ ] Bổ sung InvoiceService.cpp.
- [ ] Bổ sung ReportService.cpp.
- [ ] Bổ sung InvoiceRepository.cpp.
- [ ] Bổ sung TicketRepository.cpp.
- [ ] Build thành công.

### ApplicationContext.h/.cpp

- [ ] Khởi tạo InvoiceRepository.
- [ ] Khởi tạo TicketRepository.
- [ ] Khởi tạo BookingService.
- [ ] Khởi tạo InvoiceService.
- [ ] Khởi tạo ReportService.
- [ ] Nạp Invoice trước Ticket.
- [ ] Xử lý lỗi dữ liệu giao dịch.
- [ ] Kiểm tra vòng đời Repository/Service.
- [ ] Kiểm thử khởi động lại.

### Dữ liệu

- [ ] Kiểm tra PBL2_DATA_DIR.
- [ ] Chuẩn bị invoices.txt.
- [ ] Chuẩn bị tickets.txt.
- [ ] Bổ sung fixture giao dịch cho test GUI.
- [ ] Kiểm tra dữ liệu rỗng.
- [ ] Kiểm tra dữ liệu sai định dạng.
- [ ] Kiểm tra tham chiếu không hợp lệ.

---

## 4. Quản lý phim

File: MovieManagementView.cpp
Service: MovieService
Trạng thái: NEEDS_VERIFICATION (lưu/khôi phục file)

- [x] Hiển thị danh sách phim từ Service; tìm theo mã/tên và xem chi tiết.
- [x] Tra cứu phim theo ID qua findMovieById().
- [x] Thêm phim bằng addMovie(); Service tự cấp mã MOV.
- [x] Cập nhật bằng updateMovie(); bảng hiển thị giá trị mới.
- [x] Xóa bằng removeMovie(); không xóa phim còn suất chiếu liên kết.
- [x] Bật/ngừng hoạt động bằng setMovieActive() theo ràng buộc Service.
- [ ] Lưu dữ liệu bằng save().
- [x] Kiểm tra quyền MOVIE_VIEW, MOVIE_ADD, MOVIE_UPDATE, MOVIE_DELETE (Backend không dùng MOVIE_CREATE).
- [x] Dữ liệu không hợp lệ bị Service từ chối; GUI không báo thành công giả.
- [x] Làm mới bảng sau thêm/sửa/xóa/đổi trạng thái; test kiểm tra tên mới.
- [ ] Kiểm tra dữ liệu sau khi khởi động lại.

NEEDS_VERIFICATION: Các thay đổi danh mục hiện ở bộ nhớ; GUI chưa có luồng gọi save() và chưa kiểm thử lỗi ghi file/khôi phục sau khởi động lại.

## 5. Quản lý phòng

File: RoomManagementView.cpp
Service: RoomService
Trạng thái: BLOCKED một phần; NEEDS_VERIFICATION (lưu/khôi phục file)

- [x] Hiển thị danh sách, tìm theo mã/tên và xem chi tiết phòng.
- [x] Thêm phòng bằng addRoom(); Service tự cấp mã ROM, hàng/cột từ 1.
- [x] Sửa tên/kích thước bằng updateRoom(); bảng hiển thị giá trị mới.
- [x] Xóa phòng bằng removeRoom(); Service từ chối khi còn ghế/suất chiếu.
- [ ] Lưu dữ liệu bằng save().
- [x] Kiểm tra liên kết ghế/suất chiếu: không thu nhỏ làm ghế vượt kích thước, không xóa phòng còn liên kết.
- [x] Kiểm tra quyền ROOM_VIEW, ROOM_ADD, ROOM_UPDATE, ROOM_DELETE; Staff chỉ xem.
- [ ] Kiểm tra dữ liệu sau khởi động lại.

BLOCKED:
- [ ] Đổi trạng thái active theo quy tắc Backend.

Tham chiếu: GAP-06.

NEEDS_VERIFICATION: GUI chưa gọi save() và chưa kiểm thử lỗi ghi file/khôi phục sau khởi động lại.

## 6. Quản lý ghế

File: SeatMapView.cpp
Service: SeatService, RoomService
Trạng thái: BLOCKED một phần; NEEDS_VERIFICATION (lưu/khôi phục file)

- [x] Hiển thị sơ đồ, lọc theo phòng và xem chi tiết ghế.
- [x] Thêm ghế bằng addSeat() cho phòng chưa có suất chiếu.
- [x] Đổi loại ghế bằng updateSeat() khi được phép.
- [x] Xóa ghế bằng removeSeat() khi được phép.
- [ ] Lưu dữ liệu bằng save().
- [x] Kiểm tra quyền SEAT_VIEW, SEAT_ADD, SEAT_UPDATE, SEAT_DELETE; Staff chỉ xem.
- [x] Service từ chối phòng/vị trí/loại ghế không hợp lệ và vị trí trùng.
- [x] Sơ đồ được tải lại sau thêm/sửa/xóa; kiểm tra lại phòng trước khi ghi nếu suất chiếu xuất hiện lúc dialog mở.
- [x] Khi dữ liệu giao dịch lỗi, vẫn xem sơ đồ nhưng khóa thao tác ghi.
- [ ] Kiểm tra dữ liệu sau khởi động lại.

BLOCKED: Không sửa/xóa ghế của phòng đã có suất chiếu vì SeatService chưa xác minh lịch sử Ticket cho từng ghế (GAP-05).

NEEDS_VERIFICATION: GUI chưa gọi save() và chưa kiểm thử lỗi ghi file/khôi phục sau khởi động lại.

Tham chiếu: GAP-05.

## 7. Quản lý suất chiếu

File: ShowtimeManagementView.cpp
Service: ShowtimeService
Trạng thái: BLOCKED một phần; NEEDS_VERIFICATION (lưu/khôi phục file)

- [x] Hiển thị danh sách suất chiếu.
- [x] Lọc theo phim/phòng/ngày.
- [x] Thêm bằng addShowtime().
- [ ] Sửa bằng updateShowtime().
- [ ] Lưu bằng save().
- [x] Kiểm tra trùng lịch, kể cả khi dữ liệu đổi lúc dialog mở.
- [x] Kiểm tra quyền SHOWTIME_VIEW, SHOWTIME_ADD, SHOWTIME_UPDATE, SHOWTIME_CANCEL, SHOWTIME_DELETE.
- [ ] Kiểm tra dữ liệu sau khởi động lại.

BLOCKED:
- [ ] Hủy suất chiếu.
- [ ] Xóa suất chiếu.

Tham chiếu: GAP-03, GAP-04.

BLOCKED: GUI khóa sửa/hủy/xóa vì Backend chưa xác minh an toàn với vé đã bán. Danh sách vẫn xem được khi dữ liệu giao dịch lỗi; thao tác ghi bị khóa. NEEDS_VERIFICATION: Chưa kiểm thử save()/khôi phục file.

## 8. Quản lý nhân viên

File: StaffManagementView.cpp
Service: StaffService
Trạng thái: BLOCKED một phần; NEEDS_VERIFICATION (lưu/khôi phục file)

- [x] Hiển thị danh sách, tìm kiếm và xem thông tin Staff.
- [x] Thêm bằng addStaff().
- [x] Sửa bằng updateStaff().
- [x] Vô hiệu hóa bằng deactivateStaff().
- [ ] Lưu bằng save().
- [x] Kiểm tra quyền STAFF_VIEW, STAFF_ADD, STAFF_UPDATE, STAFF_DEACTIVATE; Staff không được quản lý tài khoản.
- [x] Không hiển thị mật khẩu đã lưu.
- [x] Kiểm tra username trùng, mật khẩu rỗng và phiên đăng nhập bị thu hồi lúc dialog mở.
- [ ] Kiểm tra dữ liệu sau khởi động lại.

BLOCKED: Chưa có API mở khóa hoặc xóa Staff. NEEDS_VERIFICATION: GUI chưa gọi save(); thay đổi tài khoản chưa được lưu bền vững.

## 9. Quản lý thành viên

File: MemberManagementView.cpp
Service: MemberService
Trạng thái: BLOCKED một phần; NEEDS_VERIFICATION (lưu/khôi phục file)

- [x] Hiển thị danh sách, tìm kiếm và xem thông tin thành viên.
- [x] Thêm bằng addMember().
- [x] Sửa bằng updateMember(); lấy lại điểm mới nhất khi dialog đóng.
- [x] Tra cứu theo ID hoặc số điện thoại.
- [ ] Lưu bằng save().
- [x] Kiểm tra số điện thoại trùng hoặc sai định dạng.
- [x] Không tự chỉnh điểm thành viên; BookingService xử lý điểm trong giao dịch.
- [ ] Kiểm tra dữ liệu sau khởi động lại.

Quyền thực tế: MEMBER_VIEW, MEMBER_ADD, MEMBER_UPDATE cho Admin và Staff. BLOCKED: Không có API xóa/ngừng hoạt động hoặc trường email. NEEDS_VERIFICATION: GUI chưa gọi save(); thay đổi danh mục chưa được lưu bền vững.

## 10. Cấu hình giá

File: PricingManagementView.cpp
Service: PricingService
Trạng thái: NEEDS_VERIFICATION (ca lỗi ghi file cấu hình giá)

- [x] Hiển thị giá từ Service.
- [x] Cập nhật giá qua Service.
- [x] Không tự tính giá bán vé cuối cùng.
- [x] Kiểm tra quyền PRICING_VIEW, PRICING_UPDATE.
- [ ] Kiểm tra dữ liệu không hợp lệ.
- [x] Kiểm tra lưu và khởi động lại cấu hình giá bằng test `gui_restart`.

## 11. Bán vé

Files:
- BookingView.cpp
- BookingAdapter.h

Service: BookingService
Trạng thái: NEEDS_VERIFICATION (lỗi lưu giao dịch)

- [x] SeatStatus::Unknown chỉ dùng khi Service không xác định được trạng thái; ghế Sold không được chọn lại.
- [x] Không còn SaleResult::NotImplemented trong luồng bán vé thật.
- [x] Hiển thị phim/suất chiếu từ Service.
- [x] Lấy ghế trống bằng getAvailableSeats().
- [x] Chọn ghế và loại vé.
- [x] Chọn thành viên nếu có.
- [x] Gọi calculateBooking().
- [x] Hiển thị BookingSummary.
- [x] Xác nhận tuổi theo giao diện.
- [x] Xác nhận đã thu tiền.
- [x] Gọi createBooking().
- [x] Hiển thị invoiceId được trả về.
- [x] Làm mới ghế sau bán.
- [x] Kiểm tra ghế đã bán không thể bán lại và không gửi trùng giao dịch.
- [x] Xử lý thiếu quyền TICKET_SELL.
- [ ] Xử lý lỗi lưu dữ liệu.
- [ ] Kiểm tra hóa đơn/vé sau khởi động lại.
- [x] Loại bỏ DEMO khỏi luồng giao dịch thật.

Không tự tạo Invoice/Ticket trong GUI.

## 12. Lịch sử hóa đơn

Files:
- InvoiceHistoryView.cpp
- SalesHistoryAdapter.cpp

Service: InvoiceService
Trạng thái: NEEDS_VERIFICATION (kiểm thử phạm vi và dữ liệu lỗi)

- [x] Thay dữ liệu DEMO bằng Service.
- [x] Gọi getInvoiceHistory().
- [x] Tra cứu bằng findInvoiceById().
- [x] Hiển thị trạng thái hóa đơn.
- [x] Hiển thị tổng tiền gốc.
- [x] Hiển thị số tiền đã hoàn.
- [x] Hiển thị số tiền còn lại.
- [ ] Hiển thị thông tin vé từ dữ liệu được phép xem.
- [ ] Kiểm tra khoảng thời gian [from, to).
- [x] Kiểm tra quyền INVOICE_VIEW_ALL của Admin.
- [x] Kiểm tra quyền INVOICE_VIEW_OWN của Staff.
- [x] Không tự tính trạng thái hóa đơn.
- [ ] Xử lý dữ liệu rỗng.
- [ ] Xử lý ngoại lệ.

Lưu ý:
Các bộ lọc phim/suất chiếu và chi tiết vé phải
được đối chiếu với dữ liệu Service trả về.
Không gọi Repository trực tiếp để bổ sung dữ liệu.

## 13. Hủy vé

File: TicketCancellationView.cpp
Service: BookingService
Trạng thái: BLOCKED một phần

- [x] Loại bỏ dữ liệu DEMO khỏi luồng hủy vé thật.
- [ ] Tra cứu vé qua API Service phù hợp.
- [ ] Hiển thị thông tin vé.
- [ ] Hiển thị điều kiện hủy từ Backend.
- [ ] Hiển thị số tiền hoàn từ Backend.
- [x] Nhập lý do hủy.
- [x] Xác nhận đã hoàn tiền.
- [x] Gọi cancelTicket().
- [x] Làm mới trạng thái vé.
- [x] Làm mới ghế sau hủy.
- [x] Kiểm tra quyền TICKET_CANCEL (Admin trong role_permissions).
- [ ] Kiểm tra hủy vé đã hủy.
- [ ] Kiểm tra hủy vé sau giờ chiếu.
- [ ] Kiểm tra điểm thành viên.
- [ ] Kiểm tra dữ liệu sau khởi động lại.

Phụ thuộc:
- GAP-01: Tra cứu vé theo ticketId.
- GAP-02: Điều kiện hủy và số tiền hoàn.

Không tự tính số tiền hoàn trong GUI.

## 14. Báo cáo thống kê

Files:
- ReportView.cpp
- SalesHistoryAdapter.cpp

Service: ReportService
Trạng thái: NEEDS_VERIFICATION

- [x] Loại bỏ dữ liệu DEMO khỏi báo cáo thật.
- [x] Gọi getTotalSales().
- [x] Gọi getTotalRefund().
- [x] Gọi getActualRevenue().
- [x] Gọi getSoldTicketCount().
- [x] Gọi getCancelledTicketCount().
- [x] Gọi getMovieRevenue().
- [x] Gọi getTopMovies().
- [x] Hiển thị bảng/biểu đồ.
- [ ] Kiểm tra khoảng thời gian [from, to).
- [x] Kiểm tra quyền REPORT_VIEW.
- [ ] Kiểm tra kỳ không có dữ liệu.
- [ ] Kiểm tra số liệu sau bán/hủy vé.
- [ ] Xử lý ngoại lệ.

Cần xác minh:
- GAP-07: Tính nhất quán số liệu Backend.

GUI không tự tính lại doanh thu.

---

## 15. Kiểm thử tích hợp

### Khởi động

- [ ] Build Debug.
- [ ] Build Release.
- [x] Chạy với dữ liệu hợp lệ.
- [ ] Chạy với dữ liệu rỗng.
- [x] Kiểm tra lỗi thiếu file.
- [ ] Kiểm tra lỗi dữ liệu sai định dạng.

### Phân quyền

- [x] Admin đăng nhập.
- [x] Staff đăng nhập.
- [x] Staff không mở được chức năng Admin.
- [x] Service từ chối thao tác thiếu quyền.
- [x] Đăng xuất và đăng nhập lại.

### Dữ liệu

- [ ] CRUD phim.
- [ ] CRUD phòng.
- [ ] CRUD ghế.
- [ ] CRUD suất chiếu.
- [x] Thêm/sửa nhân viên.
- [x] Thêm/sửa thành viên.
- [x] Cập nhật giá.
- [x] Bán vé.
- [x] Chặn bán trùng ghế.
- [x] Tra cứu hóa đơn.
- [x] Hủy vé hợp lệ.
- [x] Báo cáo sau giao dịch, gồm hóa đơn hoàn một phần.
- [ ] Khởi động lại và kiểm tra dữ liệu.

### Lỗi

- [ ] Dữ liệu tham chiếu không hợp lệ.
- [ ] File không ghi được.
- [x] Service trả false trong các ca GUI đã kiểm thử.
- [x] Service ném exception trong các ca GUI đã kiểm thử.
- [x] Không hiển thị thành công giả trong các ca GUI đã kiểm thử.
- [x] Không tự thử lại giao dịch lỗi trong các ca GUI đã kiểm thử.

---

## 16. Tiêu chí nghiệm thu

Một màn hình chỉ được đánh dấu DONE khi:

1. Đã tích hợp Business Service thực tế.
2. Không còn dữ liệu DEMO trong luồng thật.
3. Không chứa nghiệp vụ thuộc Backend.
4. Không gọi Repository trực tiếp.
5. Xử lý đầy đủ quyền và lỗi.
6. Đã kiểm thử trường hợp thành công/thất bại.
7. Dữ liệu vẫn chính xác sau khởi động lại.

Chức năng phụ thuộc GAP chưa giải quyết phải
giữ BLOCKED hoặc NEEDS_VERIFICATION.

## 17. Trạng thái hiện tại

Kiểm tra liên thông bước 4.7: Build toàn bộ thành công, CTest 4 PASS/0 FAIL (`gui_booking`, `gui_restart`, `gui_navigation`, `gui_history`). Test GUI đi qua Service cho danh mục phim/phòng/ghế/suất chiếu, nhân viên/thành viên, bán vé, hóa đơn, hủy vé, báo cáo, quyền Admin/Staff và trường hợp phiên bị thu hồi. `gui_history` kiểm tra hóa đơn hoàn một phần; `gui_booking` kiểm tra ghế đã bán và giao dịch lặp. Các ca lỗi giao dịch có test nhưng chưa bao phủ mọi lỗi ghi file thực tế.

Kiểm thử mới mở lại ApplicationContext trên cùng bộ file tạm sau khi GUI sửa danh mục: `users.txt`, `members.txt`, `movies.txt`, `rooms.txt`, `seats.txt`, `showtimes.txt` không thay đổi; phiên mới nạp dữ liệu cũ. Đây là bằng chứng các thay đổi danh mục hiện chỉ ở bộ nhớ. `gui_booking` xác minh Invoice/Ticket được BookingService ghi file; `gui_restart` hiện chỉ xác minh khôi phục cấu hình giá, chưa kiểm thử khởi động lại sau bán/hủy vé. Không đánh dấu DONE cho các mục lưu/khôi phục chưa kiểm chứng.

Sau các bước 4.1–4.3, quản lý phim/phòng/ghế đã có test GUI cho xem, tìm/lọc, thêm/sửa và các ràng buộc liên quan. Build CMake toàn bộ và 4/4 bài CTest đã PASS ở lượt kiểm tra bước 4.3. Kết quả này xác minh luồng trong bộ nhớ; chưa xác minh save() và khôi phục danh mục sau khởi động lại (GAP-09).

Các checkbox của mục khác vẫn giữ trạng thái theo đợt rà soát riêng; không suy ra DONE chỉ từ kết quả build chung.

Checklist được cập nhật sau mỗi đợt tích hợp,
không đánh dấu hoàn thành chỉ dựa vào code đã viết.
