
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

Service: MovieService

API:
- getMovieCount()
- getMovieAt()
- findMovieById()
- addMovie()
- updateMovie()
- removeMovie()
- save()

GUI:
- Hiển thị danh sách phim.
- Nhập dữ liệu và gọi Service.
- Làm mới danh sách sau thao tác.

### 3.3. RoomManagementView

Service: RoomService

API:
- getRoomCount()
- getRoomAt()
- findRoomById()
- addRoom()
- updateRoom()
- removeRoom()
- save()

Giới hạn:
- Chưa có API riêng để đổi trạng thái active của phòng.

### 3.4. SeatMapView

Service: SeatService

API:
- getSeatCount()
- getSeatAt()
- findSeatById()
- addSeat()
- updateSeat()
- removeSeat()
- save()

Giới hạn:
- Backend chưa kiểm tra đầy đủ vé tham chiếu
  khi chỉnh sửa/xóa sơ đồ ghế.

### 3.5. ShowtimeManagementView

Service: ShowtimeService

API:
- getShowtimeCount()
- getShowtimeAt()
- findShowtimeById()
- addShowtime()
- updateShowtime()
- save()

PENDING:
- cancelShowtime()
- removeShowtime()

Lý do:
- Backend chưa hoàn thiện kiểm tra liên kết vé.

### 3.6. StaffManagementView

Service: StaffService

API:
- getStaffCount()
- getStaffAt()
- findStaffById()
- findStaffByUsername()
- addStaff()
- updateStaff()
- deactivateStaff()
- save()

GUI không hiển thị mật khẩu đã lưu.

### 3.7. MemberManagementView

Service: MemberService

API:
- getMemberCount()
- getMemberAt()
- findMemberById()
- findMemberByPhone()
- addMember()
- updateMember()
- save()

GUI không tự thay đổi điểm thành viên.

### 3.8. PricingManagementView

Service: PricingService

GUI:
- Hiển thị cấu hình giá.
- Gọi Service khi cập nhật.
- Không tự tính giá vé cuối cùng để bán.

### 3.9. BookingView

Service: BookingService

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

Không được:
- Tự xác định ghế đã bán bằng dữ liệu giả.
- Tự tính giá cuối cùng.
- Tự tạo Ticket hoặc Invoice.
- Tự ghi dữ liệu giao dịch.

### 3.10. InvoiceHistoryView

Service: InvoiceService

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

Service: BookingService

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

Service: ReportService

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
- Hoàn thiện hủy/xóa suất chiếu.
- Xác minh lỗi test ReportService.
- Xử lý transactionError trong MainWindow.

Chi tiết theo dõi tại BACKEND_API_GAPS.md.

## 6. Tiêu chí hoàn thành

- Tất cả màn hình gọi đúng Service.
- Không có nghiệp vụ trong GUI.
- Không có truy cập Repository trực tiếp từ màn hình.
- Không sử dụng dữ liệu DEMO cho giao dịch thật.
- API thiếu được ghi nhận.
- Build và kiểm thử tích hợp đạt yêu cầu.
