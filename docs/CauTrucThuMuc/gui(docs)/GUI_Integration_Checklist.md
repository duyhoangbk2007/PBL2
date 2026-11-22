
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
Trạng thái: READY

- [ ] Hiển thị danh sách phim từ Service.
- [ ] Tra cứu phim theo ID.
- [ ] Thêm phim bằng addMovie().
- [ ] Cập nhật bằng updateMovie().
- [ ] Xóa bằng removeMovie().
- [ ] Lưu dữ liệu bằng save().
- [ ] Xử lý thiếu quyền.
- [ ] Xử lý dữ liệu không hợp lệ.
- [ ] Làm mới bảng sau thao tác.
- [ ] Kiểm tra dữ liệu sau khi khởi động lại.

## 5. Quản lý phòng

File: RoomManagementView.cpp
Service: RoomService
Trạng thái: READY một phần

- [ ] Hiển thị danh sách phòng.
- [ ] Thêm phòng bằng addRoom().
- [ ] Sửa phòng bằng updateRoom().
- [ ] Xóa phòng bằng removeRoom().
- [ ] Lưu dữ liệu bằng save().
- [ ] Kiểm tra phòng có liên kết suất chiếu.
- [ ] Kiểm tra dữ liệu sau khởi động lại.

BLOCKED:
- [ ] Đổi trạng thái active theo quy tắc Backend.

Tham chiếu: GAP-06.

## 6. Quản lý ghế

File: SeatMapView.cpp
Service: SeatService
Trạng thái: NEEDS_VERIFICATION

- [ ] Hiển thị sơ đồ ghế theo phòng.
- [ ] Thêm ghế bằng addSeat().
- [ ] Đổi loại ghế bằng updateSeat().
- [ ] Xóa ghế bằng removeSeat().
- [ ] Lưu dữ liệu bằng save().
- [ ] Kiểm tra quyền truy cập.
- [ ] Kiểm tra vị trí ghế trùng.
- [ ] Kiểm tra dữ liệu sau khởi động lại.

Cần xác minh:
- [ ] Không làm hỏng liên kết Ticket khi sửa/xóa ghế.

Tham chiếu: GAP-05.

## 7. Quản lý suất chiếu

File: ShowtimeManagementView.cpp
Service: ShowtimeService
Trạng thái: READY một phần

- [ ] Hiển thị danh sách suất chiếu.
- [ ] Lọc theo phim/phòng/ngày.
- [ ] Thêm bằng addShowtime().
- [ ] Sửa bằng updateShowtime().
- [ ] Lưu bằng save().
- [ ] Kiểm tra trùng lịch.
- [ ] Kiểm tra quyền.
- [ ] Kiểm tra dữ liệu sau khởi động lại.

BLOCKED:
- [ ] Hủy suất chiếu.
- [ ] Xóa suất chiếu.

Tham chiếu: GAP-03, GAP-04.

## 8. Quản lý nhân viên

File: StaffManagementView.cpp
Service: StaffService
Trạng thái: READY

- [ ] Hiển thị danh sách Staff.
- [ ] Thêm bằng addStaff().
- [ ] Sửa bằng updateStaff().
- [ ] Vô hiệu hóa bằng deactivateStaff().
- [ ] Lưu bằng save().
- [ ] Kiểm tra quyền Admin.
- [ ] Không hiển thị mật khẩu đã lưu.
- [ ] Kiểm tra username trùng.
- [ ] Kiểm tra dữ liệu sau khởi động lại.

## 9. Quản lý thành viên

File: MemberManagementView.cpp
Service: MemberService
Trạng thái: READY

- [ ] Hiển thị danh sách thành viên.
- [ ] Thêm bằng addMember().
- [ ] Sửa bằng updateMember().
- [ ] Tra cứu theo ID hoặc số điện thoại.
- [ ] Lưu bằng save().
- [ ] Kiểm tra số điện thoại trùng.
- [ ] Không tự chỉnh điểm thành viên.
- [ ] Kiểm tra dữ liệu sau khởi động lại.

## 10. Cấu hình giá

File: PricingManagementView.cpp
Service: PricingService
Trạng thái: READY

- [ ] Hiển thị giá từ Service.
- [ ] Cập nhật giá qua Service.
- [ ] Không tự tính giá bán vé cuối cùng.
- [ ] Kiểm tra quyền PRICING_UPDATE.
- [ ] Kiểm tra dữ liệu không hợp lệ.
- [ ] Kiểm tra lưu và khởi động lại.

## 11. Bán vé

Files:
- BookingView.cpp
- BookingAdapter.h

Service: BookingService
Trạng thái: READY để tích hợp

- [ ] Loại bỏ SeatStatus::Unknown giả lập.
- [ ] Loại bỏ SaleResult::NotImplemented giả lập.
- [ ] Hiển thị phim/suất chiếu từ Service.
- [ ] Lấy ghế trống bằng getAvailableSeats().
- [ ] Chọn ghế và loại vé.
- [ ] Chọn thành viên nếu có.
- [ ] Gọi calculateBooking().
- [ ] Hiển thị BookingSummary.
- [ ] Xác nhận tuổi theo giao diện.
- [ ] Xác nhận đã thu tiền.
- [ ] Gọi createBooking().
- [ ] Hiển thị invoiceId được trả về.
- [ ] Làm mới ghế sau bán.
- [ ] Kiểm tra ghế đã bán không thể bán lại.
- [ ] Xử lý thiếu quyền.
- [ ] Xử lý lỗi lưu dữ liệu.
- [ ] Kiểm tra hóa đơn/vé sau khởi động lại.
- [ ] Loại bỏ DEMO khỏi luồng giao dịch thật.

Không tự tạo Invoice/Ticket trong GUI.

## 12. Lịch sử hóa đơn

Files:
- InvoiceHistoryView.cpp
- SalesHistoryAdapter.cpp

Service: InvoiceService
Trạng thái: READY để tích hợp

- [ ] Thay dữ liệu DEMO bằng Service.
- [ ] Gọi getInvoiceHistory().
- [ ] Tra cứu bằng findInvoiceById().
- [ ] Hiển thị trạng thái hóa đơn.
- [ ] Hiển thị tổng tiền gốc.
- [ ] Hiển thị số tiền đã hoàn.
- [ ] Hiển thị số tiền còn lại.
- [ ] Hiển thị thông tin vé từ dữ liệu được phép xem.
- [ ] Kiểm tra khoảng thời gian [from, to).
- [ ] Kiểm tra quyền Admin.
- [ ] Kiểm tra phạm vi hóa đơn của Staff.
- [ ] Không tự tính trạng thái hóa đơn.
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

- [ ] Loại bỏ dữ liệu DEMO.
- [ ] Tra cứu vé qua API Service phù hợp.
- [ ] Hiển thị thông tin vé.
- [ ] Hiển thị điều kiện hủy từ Backend.
- [ ] Hiển thị số tiền hoàn từ Backend.
- [ ] Nhập lý do hủy.
- [ ] Xác nhận đã hoàn tiền.
- [ ] Gọi cancelTicket().
- [ ] Làm mới trạng thái vé.
- [ ] Làm mới ghế sau hủy.
- [ ] Kiểm tra quyền Admin.
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

- [ ] Loại bỏ dữ liệu DEMO.
- [ ] Gọi getTotalSales().
- [ ] Gọi getTotalRefund().
- [ ] Gọi getActualRevenue().
- [ ] Gọi getSoldTicketCount().
- [ ] Gọi getCancelledTicketCount().
- [ ] Gọi getMovieRevenue().
- [ ] Gọi getTopMovies().
- [ ] Hiển thị bảng/biểu đồ.
- [ ] Kiểm tra khoảng thời gian [from, to).
- [ ] Kiểm tra quyền REPORT_VIEW.
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
- [ ] Chạy với dữ liệu hợp lệ.
- [ ] Chạy với dữ liệu rỗng.
- [ ] Kiểm tra lỗi thiếu file.
- [ ] Kiểm tra lỗi dữ liệu sai định dạng.

### Phân quyền

- [ ] Admin đăng nhập.
- [ ] Staff đăng nhập.
- [ ] Staff không mở được chức năng Admin.
- [ ] Service từ chối thao tác thiếu quyền.
- [ ] Đăng xuất và đăng nhập lại.

### Dữ liệu

- [ ] CRUD phim.
- [ ] CRUD phòng.
- [ ] CRUD ghế.
- [ ] CRUD suất chiếu.
- [ ] Thêm/sửa nhân viên.
- [ ] Thêm/sửa thành viên.
- [ ] Cập nhật giá.
- [ ] Bán vé.
- [ ] Chặn bán trùng ghế.
- [ ] Tra cứu hóa đơn.
- [ ] Hủy vé hợp lệ.
- [ ] Báo cáo sau giao dịch.
- [ ] Khởi động lại và kiểm tra dữ liệu.

### Lỗi

- [ ] Dữ liệu tham chiếu không hợp lệ.
- [ ] File không ghi được.
- [ ] Service trả false.
- [ ] Service ném exception.
- [ ] Không hiển thị thành công giả.
- [ ] Không tự thử lại giao dịch lỗi.

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

Các checkbox mặc định chưa được đánh dấu vì
chưa có kết quả build/test xác nhận trên B13.

Checklist được cập nhật sau mỗi đợt tích hợp,
không đánh dấu hoàn thành chỉ dựa vào code đã viết.
