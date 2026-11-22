
# Backend API Gaps – GUI Integration

**Dự án:** PBL2 Cinema
**Nhánh GUI:** feature/b13-hoang-kiet-gui-backend-integration
**Contract:** Business_UI_Contract.md v0.4
**Trạng thái:** Draft – chờ xác nhận từ Backend

## 1. Mục đích

Theo dõi các chức năng GUI chưa thể tích hợp hoàn chỉnh
vì Backend thiếu API, thiếu kiểm tra nghiệp vụ hoặc còn
vấn đề cần xác minh.

Không sửa Backend trong quá trình tích hợp GUI khi chưa
được phê duyệt.

Không sử dụng Repository trực tiếp từ GUI để thay thế
API Service còn thiếu.

## 2. Quy ước trạng thái

- BLOCKED: Chưa có API hoặc nghiệp vụ Backend cần thiết.
- NEEDS_VERIFICATION: Có API nhưng còn lỗi hoặc rủi ro.
- READY: API phù hợp đã được xác nhận.
- RESOLVED: Vấn đề đã được xử lý và kiểm thử.

Mức độ ưu tiên:
- P0: Ảnh hưởng chức năng chính hoặc an toàn dữ liệu.
- P1: Ảnh hưởng một chức năng quản lý.
- P2: Cải thiện trải nghiệm hoặc khả năng tra cứu.

---

## GAP-01: Tra cứu vé trực tiếp theo mã vé

**Ưu tiên:** P0
**Trạng thái:** BLOCKED

### Màn hình ảnh hưởng
- TicketCancellationView
- InvoiceHistoryView (chi tiết vé)

### Hiện trạng
BookingService có cancelTicket(ticketId, reason,
refundConfirmed), nhưng chưa có API Service để tra cứu
vé trực tiếp theo ticketId.

TicketRepository có findById(), nhưng GUI không được
gọi Repository trực tiếp.

InvoiceService có thể tìm hóa đơn theo invoiceId,
không có API tìm trực tiếp theo ticketId.

### API đề xuất

Trong InvoiceService hoặc Service tra cứu vé phù hợp:

    const Ticket* findTicketById(
        const std::string& ticketId);

Hoặc API trả DTO chỉ đọc có thông tin vé, hóa đơn,
suất chiếu và quyền xem tương ứng.

### Yêu cầu Backend
- Kiểm tra quyền trước khi trả dữ liệu.
- Không để Staff xem vé ngoài phạm vi được phép.
- Trả nullptr khi mã vé không tồn tại.
- Phân biệt lỗi thiếu quyền và không tìm thấy.
- Không trả dữ liệu có thể bị GUI sửa trực tiếp.

### Điều kiện mở khóa
Backend cung cấp API và test quyền truy cập thành công.

---

## GAP-02: Kiểm tra khả năng hủy và số tiền hoàn

**Ưu tiên:** P0
**Trạng thái:** BLOCKED

### Màn hình ảnh hưởng
- TicketCancellationView

### Hiện trạng
BookingService có:

    bool cancelTicket(
        const std::string& ticketId,
        const std::string& reason,
        bool refundConfirmed);

Nhưng chưa có API báo trước vé có đủ điều kiện hủy
và số tiền cần hoàn.

GUI không được tự quyết định:
- Vé còn SOLD hay đã CANCELLED.
- Suất chiếu đã bắt đầu hay chưa.
- Thành viên có đủ điểm để thu hồi hay không.
- Số tiền phải hoàn.

### API đề xuất

    bool getCancellationQuote(
        const std::string& ticketId,
        CancellationQuote& result);

DTO đề xuất:

    struct CancellationQuote {
        std::string ticketId;
        bool eligible = false;
        double refundAmount = 0;
        std::string reason;
    };

Đây là đề xuất mới, chưa tồn tại trong Backend.

### Yêu cầu Backend
- Kiểm tra quyền TICKET_CANCEL.
- Tính điều kiện và tiền hoàn trong Service.
- Không thay đổi dữ liệu khi chỉ tra cứu.
- cancelTicket() phải kiểm tra lại khi thực hiện.

### Điều kiện mở khóa
API được triển khai và kiểm thử với vé SOLD,
CANCELLED, suất đã bắt đầu và thành viên không đủ điểm.

---

## GAP-03: Hủy suất chiếu

**Ưu tiên:** P1
**Trạng thái:** BLOCKED

### Màn hình ảnh hưởng
- ShowtimeManagementView

### Hiện trạng
ShowtimeService đã có:

    bool cancelShowtime(
        const std::string& showtimeId);

Nhưng implementation hiện có TODO kiểm tra Ticket
và trả false sau các kiểm tra ban đầu.

### Đề xuất
Hoàn thiện API hiện có thay vì thêm API mới.

Backend cần:
- Kiểm tra vé liên quan.
- Quy định trường hợp được phép hủy.
- Đảm bảo không làm mất dữ liệu giao dịch.
- Kiểm tra quyền SHOWTIME_CANCEL.

### Điều kiện mở khóa
cancelShowtime() thực hiện được trường hợp hợp lệ
và từ chối an toàn các trường hợp không hợp lệ.

---

## GAP-04: Xóa suất chiếu

**Ưu tiên:** P1
**Trạng thái:** BLOCKED

### Màn hình ảnh hưởng
- ShowtimeManagementView

### Hiện trạng
ShowtimeService có:

    bool removeShowtime(
        const std::string& showtimeId);

Implementation chưa kiểm tra được lịch sử vé,
hiện trả false.

### Đề xuất
Hoàn thiện API hiện có.

Backend cần:
- Kiểm tra lịch sử Ticket và Invoice.
- Không xóa suất có giao dịch liên quan.
- Kiểm tra quyền SHOWTIME_DELETE.

### Điều kiện mở khóa
Có test xóa suất không có giao dịch và từ chối
xóa suất có giao dịch.

---

## GAP-05: An toàn dữ liệu khi chỉnh sơ đồ ghế

**Ưu tiên:** P1
**Trạng thái:** NEEDS_VERIFICATION

### Màn hình ảnh hưởng
- SeatMapView

### Hiện trạng
SeatService có addSeat(), updateSeat(),
removeSeat().

Contract ghi nhận chưa có kiểm tra đầy đủ
Ticket tham chiếu ghế khi sửa/xóa sơ đồ.

### Đề xuất
Backend bổ sung kiểm tra liên kết giao dịch vào
nghiệp vụ chỉnh sửa/xóa ghế.

Giữ API hiện có nếu phù hợp:

    bool updateSeat(const Seat& newValues);

    bool removeSeat(
        const std::string& roomId,
        const std::string& seatId);

### Điều kiện mở khóa
Xác nhận không thể sửa/xóa ghế theo cách làm
hỏng dữ liệu Ticket đã phát hành.

---

## GAP-06: Thay đổi trạng thái hoạt động của phòng

**Ưu tiên:** P1
**Trạng thái:** BLOCKED

### Màn hình ảnh hưởng
- RoomManagementView

### Hiện trạng
RoomService có updateRoom(), nhưng contract ghi nhận
không cho đổi active qua API này khi phòng đã có
suất chiếu.

Chưa có API riêng để kích hoạt/vô hiệu hóa phòng.

### API đề xuất

    bool setRoomActive(
        const std::string& roomId,
        bool active);

### Yêu cầu Backend
- Kiểm tra quyền phù hợp.
- Kiểm tra các suất chiếu liên quan.
- Không cho thay đổi gây bất nhất dữ liệu.
- Xác định quy tắc kích hoạt/vô hiệu hóa.

### Điều kiện mở khóa
Backend xác nhận quy tắc và triển khai API phù hợp.

---

## GAP-07: Tính nhất quán của báo cáo

**Ưu tiên:** P0
**Trạng thái:** NEEDS_VERIFICATION

### Màn hình ảnh hưởng
- ReportView

### Hiện trạng
ReportService đã cung cấp các API:
- getTotalSales()
- getTotalRefund()
- getActualRevenue()
- getSoldTicketCount()
- getCancelledTicketCount()
- getMovieRevenue()
- getTopMovies()

Theo Business_UI_Contract.md v0.4,
test_report_service.cpp từng có kết quả
31 PASS, 1 FAIL.

Ca chưa đạt liên quan đến tổng paidAmount
của Ticket không khớp Invoice::totalAmount.

### Đề xuất
Không cần API GUI mới ở thời điểm này.

Backend cần:
- Kiểm tra lại ca test thất bại.
- Xác định quy tắc kiểm tra tính nhất quán.
- Không để GUI tự điều chỉnh số liệu báo cáo.

### Điều kiện mở khóa
Test Backend đạt và số liệu được xác minh.

---

## GAP-08: Trạng thái lỗi lưu giao dịch

**Ưu tiên:** P0
**Trạng thái:** NEEDS_VERIFICATION

### Màn hình ảnh hưởng
- BookingView
- TicketCancellationView

### Hiện trạng
BookingService có trạng thái nội bộ storageFailed.

Khi lỗi ghi dữ liệu giao dịch, Service có thể
chặn các thao tác tiếp theo trên cùng instance.

Chưa có API công khai để GUI tra cứu trạng thái này.

### API đề xuất (nếu Backend xác nhận cần thiết)

    bool isStorageHealthy() const;

Hoặc:

    BookingStorageStatus getStorageStatus() const;

### Yêu cầu Backend
- Xác định cách báo lỗi cho GUI.
- Không để GUI tự thử lại giao dịch nguy hiểm.
- Không để GUI tự sửa dữ liệu TXT.
- Quy định quy trình phục hồi sau lỗi ghi.

### Điều kiện mở khóa
GUI nhận biết được lỗi và có quy trình xử lý an toàn.

---

## 3. Tổng hợp

| ID | Vấn đề | Ưu tiên | Trạng thái |
|---|---|---|---|
| GAP-01 | Tra cứu vé theo ID | P0 | BLOCKED |
| GAP-02 | Điều kiện hủy/tiền hoàn | P0 | BLOCKED |
| GAP-03 | Hủy suất chiếu | P1 | BLOCKED |
| GAP-04 | Xóa suất chiếu | P1 | BLOCKED |
| GAP-05 | An toàn sửa/xóa ghế | P1 | NEEDS_VERIFICATION |
| GAP-06 | Đổi active phòng | P1 | BLOCKED |
| GAP-07 | Tính nhất quán báo cáo | P0 | NEEDS_VERIFICATION |
| GAP-08 | Trạng thái lỗi lưu giao dịch | P0 | NEEDS_VERIFICATION |

## 4. Quy trình xử lý API Gap

1. Kiểm tra API Service hiện có.
2. Kiểm tra API tương đương.
3. Nếu chưa đáp ứng, ghi nhận vào tài liệu này.
4. Đề xuất chữ ký, tham số, kiểu trả về và quy tắc.
5. Chuyển yêu cầu cho người phụ trách Backend.
6. Chỉ cập nhật GUI khi API được xác nhận.
7. Kiểm thử và chuyển trạng thái sang RESOLVED.

Không sửa Backend Service hoặc gọi Repository
trực tiếp từ GUI để vượt qua API Gap.

## 5. Phân công và nghiệm thu

Backend:
- Xác nhận hoặc điều chỉnh API đề xuất.
- Triển khai kiểm tra nghiệp vụ.
- Bổ sung kiểm thử.

GUI:
- Chỉ tích hợp API đã được xác nhận.
- Giữ chức năng BLOCKED ở trạng thái an toàn.
- Hiển thị lỗi rõ ràng.
- Không tự tính toán nghiệp vụ.

Mỗi GAP chỉ được đánh dấu RESOLVED sau khi
có xác nhận API và kết quả kiểm thử phù hợp.
