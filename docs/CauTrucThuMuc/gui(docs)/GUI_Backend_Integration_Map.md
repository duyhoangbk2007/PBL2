
# GUI – Backend Integration Map

**Dự án:** PBL2 Cinema
**Nhánh tích hợp:** feature/b13-hoang-kiet-gui-backend-integration
**Contract tham chiếu:** Business_UI_Contract.md v0.4
**Trạng thái:** Draft

## 1. Nguyên tắc bắt buộc

1. GUI chỉ gọi Business Service để thực hiện nghiệp vụ.
2. GUI không truy cập Repository để xử lý dữ liệu nghiệp vụ.
3. GUI không đọc/ghi trực tiếp file TXT.
4. GUI không tự thực hiện tính giá, phân quyền nghiệp vụ,
   kiểm tra điều kiện bán/hủy vé hoặc tính doanh thu.
5. GUI chỉ nhận dữ liệu, gọi Service, hiển thị kết quả
   và xử lý trạng thái giao diện.
6. Khi thiếu API, ghi vào BACKEND_API_GAPS.md.
7. Không tự ý sửa Backend Service hoặc thay đổi chữ ký API.
8. Các chức năng chưa được hỗ trợ phải giữ trạng thái PENDING.

## 2. Khởi tạo ứng dụng

### ApplicationContext

File:
- src/gui/ApplicationContext.h
- src/gui/ApplicationContext.cpp

Trách nhiệm:
- Khởi tạo Repository và Service.
- Quản lý vòng đời các đối tượng.
- Nạp dữ liệu theo đúng thứ tự phụ thuộc.
- Cung cấp Service cho các màn hình.
- Ghi nhận lỗi khởi tạo/nạp dữ liệu.

Không thực hiện nghiệp vụ bán vé, hủy vé hoặc báo cáo.

### Các Service cần cung cấp

- AuthService
- MovieService
- RoomService
- SeatService
- ShowtimeService
- StaffService
- MemberService
- PricingService
- BookingService
- InvoiceService
- ReportService

## 3. Ánh xạ màn hình – Service

### 3.1. LoginView

Service: AuthService

API:
- login()
- logout()
- getCurrentUser()
- isLoggedIn()

GUI:
- Nhập username/password.
- Gọi login().
- Hiển thị kết quả.

### 3.2. MovieManagementView

Service: MovieService; quyền thực tế MOVIE_VIEW, MOVIE_ADD, MOVIE_UPDATE, MOVIE_DELETE (không có MOVIE_CREATE).

API:
- getMovieCount()
- getMovieAt()
- findMovieById()
- addMovie()
- updateMovie()
- removeMovie()
- setMovieActive()
- save() (có API, GUI chưa gọi)

GUI:
- Hiển thị danh sách, tìm theo mã/tên và mở chi tiết chỉ đọc.
- Thêm, sửa, xóa, bật/ngừng hoạt động qua Service; Service cấp mã MOV và kiểm tra ràng buộc suất chiếu.
- Làm mới bảng sau thao tác; test kiểm tra tên vừa sửa và phim vừa thêm.

NEEDS_VERIFICATION: Chưa kiểm thử luồng save(), lỗi ghi file và nạp lại sau khởi động.

### 3.3. RoomManagementView

Service: RoomService; quyền thực tế ROOM_VIEW, ROOM_ADD, ROOM_UPDATE, ROOM_DELETE.

API:
- getRoomCount()
- getRoomAt()
- findRoomById()
- addRoom()
- updateRoom()
- removeRoom()
- save() (có API, GUI chưa gọi)

GUI:
- Hiển thị danh sách, tìm theo mã/tên và mở chi tiết chỉ đọc.
- Thêm/sửa/xóa qua Service; mã ROM do Service cấp.
- Service chặn thu nhỏ làm ghế vượt kích thước và xóa phòng còn ghế/suất chiếu; GUI cập nhật bảng sau thao tác.

Giới hạn:
- BLOCKED: RoomService không có API đổi active; updateRoom() từ chối đổi active.
- NEEDS_VERIFICATION: Chưa kiểm thử save(), lỗi ghi file và nạp lại sau khởi động.

### 3.4. SeatMapView

Service: SeatService, RoomService; quyền thực tế SEAT_VIEW, SEAT_ADD, SEAT_UPDATE, SEAT_DELETE.

API:
- getSeatCount()
- getSeatAt()
- findSeatById()
- addSeat()
- updateSeat()
- removeSeat()
- save() (có API, GUI chưa gọi)

GUI:
- Lọc phòng bằng RoomService, hiển thị sơ đồ và chi tiết ghế; thao tác ghi qua SeatService.
- Chỉ cho thêm/sửa/xóa khi phòng chưa có suất chiếu và dữ liệu giao dịch đã nạp; kiểm tra lại phòng trước khi ghi nếu dữ liệu đổi lúc dialog mở.
- Tải lại sơ đồ sau thao tác; Staff chỉ xem.

Giới hạn:
- BLOCKED: SeatService chưa kiểm tra đầy đủ vé tham chiếu khi chỉnh sửa/xóa ghế của phòng có suất chiếu (GAP-05).
- NEEDS_VERIFICATION: Chưa kiểm thử save(), lỗi ghi file và nạp lại sau khởi động.

### 3.5. ShowtimeManagementView

Service: ShowtimeService, MovieService, RoomService; quyền thực tế SHOWTIME_VIEW, SHOWTIME_ADD, SHOWTIME_UPDATE, SHOWTIME_CANCEL, SHOWTIME_DELETE.

API:
- getShowtimeCount()
- getShowtimeAt()
- findShowtimeById()
- addShowtime()
- updateShowtime()
- save() (có API, GUI chưa gọi)

PENDING:
- updateShowtime(), cancelShowtime(), removeShowtime() trên GUI

Lý do:
- Backend có chữ ký API nhưng chưa xác minh an toàn với vé đã bán. GUI khóa sửa/hủy/xóa; thêm suất chiếu đi qua Service và kiểm tra lại xung đột lịch lúc gửi.
- NEEDS_VERIFICATION: suất chiếu thêm qua GUI chưa được lưu/khôi phục.

### 3.6. StaffManagementView

Service: StaffService; quyền thực tế STAFF_VIEW, STAFF_ADD, STAFF_UPDATE, STAFF_DEACTIVATE (chỉ Admin trong role_permissions).

API:
- getStaffCount()
- getStaffAt()
- findStaffById()
- findStaffByUsername()
- addStaff()
- updateStaff()
- deactivateStaff()
- save() (có API, GUI chưa gọi)

GUI hiển thị danh sách, tìm kiếm, xem thông tin, thêm/sửa và vô hiệu hóa qua Service; không hiển thị mật khẩu đã lưu. Service kiểm tra lại quyền khi gửi form, nên phiên bị thu hồi giữa lúc dialog mở không ghi dữ liệu.

BLOCKED: Chưa có API mở khóa/xóa Staff. NEEDS_VERIFICATION: Chưa lưu/khôi phục thay đổi GUI sau khởi động lại.

### 3.7. MemberManagementView

Service: MemberService; quyền thực tế MEMBER_VIEW, MEMBER_ADD, MEMBER_UPDATE (Admin và Staff).

API:
- getMemberCount()
- getMemberAt()
- findMemberById()
- findMemberByPhone()
- addMember()
- updateMember()
- save() (có API, GUI chưa gọi)

GUI hiển thị danh sách, tìm theo ID/tên/số điện thoại, xem thông tin và thêm/sửa qua Service. Khi đóng dialog sửa, GUI tra lại Member để lấy điểm hiện tại; không tự thay đổi điểm. BookingService xử lý cộng/trừ điểm khi bán/hủy vé.

BLOCKED: Chưa có API xóa/ngừng hoạt động Member hoặc trường email. NEEDS_VERIFICATION: Chưa lưu/khôi phục thay đổi GUI sau khởi động lại.

### 3.8. PricingManagementView

Service: PricingService

GUI:
- Hiển thị cấu hình giá.
- Gọi Service khi cập nhật.
- Không tự tính giá vé cuối cùng để bán.

### 3.9. BookingView

Service: BookingService, MovieService, ShowtimeService, MemberService; quyền giao dịch TICKET_SELL, tra cứu thành viên MEMBER_VIEW.

API:
- getAvailableSeats()
- calculateBooking()
- createBooking()

GUI:
- Chọn suất chiếu, ghế, loại vé và thành viên.
- Hiển thị BookingSummary từ Service.
- Xác nhận tuổi và thanh toán.
- Gọi createBooking().
- Hiển thị invoiceId khi thành công.
- Tải lại trạng thái ghế Sold/Available từ Service; khóa nút khi giao dịch đang xử lý hoặc lỗi lưu không rõ kết quả.

Không được:
- Tự xác định ghế đã bán bằng dữ liệu giả.
- Tự tính giá cuối cùng.
- Tự tạo Ticket hoặc Invoice.
- Tự ghi dữ liệu giao dịch.

### 3.10. InvoiceHistoryView

Service: InvoiceService; quyền thực tế INVOICE_VIEW_ALL cho Admin, INVOICE_VIEW_OWN cho Staff.

API:
- findInvoiceById()
- getInvoiceHistory()
- getInvoiceStatus()
- getRefundAmount()
- getRemainingAmount()

GUI:
- Chọn khoảng thời gian.
- Nhập bộ lọc.
- Hiển thị hóa đơn và trạng thái từ Service.

Không truy cập InvoiceRepository trực tiếp.

### 3.11. TicketCancellationView

Service: BookingService, InvoiceService; quyền giao dịch TICKET_CANCEL (Admin trong role_permissions).

API:
- cancelTicket()

GUI:
- Nhận mã vé và lý do.
- Xác nhận đã hoàn tiền.
- Gọi cancelTicket().
- Hiển thị kết quả.

API GAP:
- Thiếu Service API tra cứu chi tiết vé theo ticketId
  phục vụ màn hình xác nhận hủy.

Không tự quyết định vé có đủ điều kiện hủy.

### 3.12. ReportView

Service: ReportService; quyền REPORT_VIEW (Admin trong role_permissions).

API:
- getTotalSales()
- getTotalRefund()
- getActualRevenue()
- getSoldTicketCount()
- getCancelledTicketCount()
- getMovieRevenue()
- getTopMovies()

GUI:
- Chọn khoảng thời gian.
- Gọi Service.
- Hiển thị chỉ số và biểu đồ.
- `gui_history` kiểm tra hóa đơn hoàn một phần; tính nhất quán Backend toàn diện vẫn NEEDS_VERIFICATION (GAP-07).

Không tự tổng hợp doanh thu từ Repository.

## 4. Quy ước xử lý kết quả

- true: thao tác thành công.
- false: thao tác không thành công.
- nullptr: không tìm thấy đối tượng khi API cho phép.
- Exception: hiển thị lỗi phù hợp, không tự sửa dữ liệu.
- Con trỏ Model do Service trả về là con trỏ mượn.
- GUI không delete hoặc chỉnh sửa trực tiếp đối tượng mượn.

Các thao tác danh mục cần lưu dữ liệu theo contract.
BookingService tự điều phối lưu giao dịch.

## 5. Các vấn đề đang chờ

- API tra cứu vé trực tiếp theo ticketId.
- Kiểm tra liên kết vé khi sửa/xóa sơ đồ ghế.
- API đổi active của phòng và lưu/khôi phục danh mục phim, phòng, ghế (GAP-06, GAP-09).
- Lưu/khôi phục suất chiếu, Staff, Member cũng thuộc GAP-09; GUI không gọi save() cho các danh mục này.
- Mở khóa/xóa Staff, xóa/ngừng hoạt động Member và trường email còn BLOCKED (GAP-10).
- Hoàn thiện hủy/xóa suất chiếu.
- Xác minh lỗi test ReportService.
- Xác minh khôi phục giao dịch và điểm thành viên trên tiến trình mới sau bán/hủy vé; MainWindow đã xử lý `transactionError` khi mở màn hình.

Chi tiết theo dõi tại BACKEND_API_GAPS.md.

## 6. Tiêu chí hoàn thành

### Đối chiếu liên thông bước 4.7

- Màn hình nghiệp vụ truy cập qua Business Service; Repository trong `ApplicationContext` chỉ được dùng để khởi tạo và nạp dữ liệu theo thứ tự phụ thuộc. Không phát hiện màn hình gọi Repository trực tiếp hoặc tự tính giá/hoàn tiền/doanh thu.
- Test GUI: `gui_navigation` bao phủ danh mục và quyền; `gui_booking` bao phủ báo giá, bán vé, ghế Sold, chặn giao dịch lặp; `gui_history` bao phủ hóa đơn, hủy vé, báo cáo hoàn một phần và phân quyền. Build toàn bộ và 4/4 CTest PASS ở bước 4.7.
- Sau các thao tác GUI với phim, phòng, ghế, suất chiếu, Staff, Member, test mở `ApplicationContext` mới trên cùng file tạm: TXT không đổi và dữ liệu mới không xuất hiện. Các thay đổi này chỉ ở bộ nhớ. BookingService ghi Invoice/Ticket khi bán; khôi phục giao dịch sau khởi động lại và lỗi ghi file còn NEEDS_VERIFICATION. `gui_restart` hiện xác minh cấu hình giá.
- Các thao tác BLOCKED theo GAP-01–06 và GAP-10 chưa được coi là hoàn thành. GAP-07–09 còn NEEDS_VERIFICATION.

- Tất cả màn hình gọi đúng Service.
- Không có nghiệp vụ trong GUI.
- Không có truy cập Repository trực tiếp từ màn hình.
- Không sử dụng dữ liệu DEMO cho giao dịch thật.
- API thiếu được ghi nhận.
- Build và kiểm thử tích hợp đạt yêu cầu.
