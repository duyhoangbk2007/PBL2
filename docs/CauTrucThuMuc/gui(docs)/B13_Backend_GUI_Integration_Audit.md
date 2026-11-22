# B13 — Rà soát tích hợp Backend mới vào GUI (đối chiếu contract v0.4)

**Ngày rà soát:** 2026-11-22  
**Repository:** `duyhoangbk2007/PBL2`  
**Nhánh mã được đọc:** `feature/b13-hoang-kiet-gui-backend-integration`  
**Contract:** `docs/CauTrucThuMuc/gui(docs)/Business_UI_Contract.md` (v0.4 Draft)  
**Mục tiêu:** liệt kê các kết nối Backend còn thiếu trong GUI B13 đang mang cơ chế bán vé/tra cứu/báo cáo dạng bản mẫu. Đây là **báo cáo đọc tĩnh mã**, chưa build hay chạy Qt test; không đánh đồng trạng thái API sẵn có với hành vi đã tích hợp.

## 1. Kết luận nhanh

**Ưu tiên P0 — chưa có luồng nghiệp vụ thật:** `BookingService` (bán vé), `InvoiceService` (lịch sử), `BookingService::cancelTicket` (hủy vé), `ReportService` (báo cáo). Các phần này có giao diện hoặc demo nhưng chưa gọi Service tương ứng.

**Ưu tiên P0 — nền tảng:** bổ sung `InvoiceRepository`, `TicketRepository`, `BookingService`, `InvoiceService`, `ReportService` vào `ApplicationContext`, nạp đúng thứ tự, bổ sung file `.cpp` vào `CMakeLists.txt` và bộ fixture `invoices.txt`, `tickets.txt` phù hợp.

**Ưu tiên P1 — các form quản lý cũ:** những đường **Thêm** Member/Staff/Showtime đang gọi nhầm `update...`; Thêm Movie/Room chỉ là nút `pending`. Chưa có quy trình `save()` nhất quán cho các danh mục sau khi thay đổi.

**Đã có kết nối thật một phần:** đăng nhập/phân quyền, tra cứu/sửa/xóa Movie/Room, Seat, Showtime; tra cứu/cập nhật Pricing; Member/Staff tra cứu/cập nhật/vô hiệu hóa. Không nên viết lại phần đã có nếu chỉ thiếu điều phối lưu hoặc chức năng Thêm.

## 2. Ma trận chênh lệch: contract ↔ GUI ↔ yêu cầu

| Ưu tiên | Màn hình / file hiện tại | Trạng thái đọc mã | Backend cần nối / việc phải làm |
|---|---|---|---|
| **P0** | `src/gui/ApplicationContext.h/.cpp` | Chỉ có User/Permission/Movie/Room/Seat/Showtime/Member/Pricing Repo và Service | Thêm `InvoiceRepository`, `TicketRepository`, `BookingService`, `InvoiceService`, `ReportService`; nạp invoice trước ticket, đảm bảo thứ tự và vòng đời con trỏ |
| **P0** | `CMakeLists.txt` | `pbl2_auth` chưa liệt kê `BookingService.cpp`, `InvoiceService.cpp`, `ReportService.cpp`, `InvoiceRepository.cpp`, `TicketRepository.cpp` | Bổ sung các file Backend và cập nhật link/test target; không chỉ thêm header vào GUI |
| **P0** | `BookingAdapter.h`, `BookingView.cpp` | `SeatStatus::Unknown`, `SaleResult::NotImplemented`; tính báo giá riêng bằng `PricingService`, chặn xác nhận bán thật | Dùng `BookingService::getAvailableSeats`, `calculateBooking`, `createBooking`; ánh xạ `BookingItem`/`BookingSummary`; có kết quả invoice ID; làm mới ghế sau bán |
| **P0** | `SalesHistoryAdapter.cpp`, `InvoiceHistoryView.cpp` | `history()` trả `NotImplemented` khi không demo; detail cũng tra demo | Gọi `InvoiceService::getInvoiceHistory` và `findInvoiceById`, `getInvoiceStatus`, `getRefundAmount`, `getRemainingAmount`; tạo presentation mapping từ Invoice/Ticket/Showtime/Member/User |
| **P0** | `SalesHistoryAdapter.cpp`, `TicketCancellationView.cpp` | `cancellationLookup` demo; nút “Xác nhận hủy vé” bị disable | Tra cứu vé theo quyền bằng Service được bổ sung/adapter có ranh giới rõ; gọi `BookingService::cancelTicket(ticketId, reason, refundConfirmed)` cho admin; refresh hóa đơn/ghế/báo cáo |
| **P0** | `SalesHistoryAdapter.cpp`, `ReportView.cpp` | `report()` demo hoặc `NotImplemented`; 5 KPI/biểu đồ chưa dựa trên dữ liệu thật | Gọi 7 API `ReportService`, chuyển khoảng `[from,to)`; tính doanh thu ngày qua từng khoảng ngày; top phim gọi `getTopMovies` + `getMovieRevenue` |
| **P1** | `MemberManagementView.cpp::form(bool creating)` | Dù `creating=true`, cuối hàm vẫn `service.updateMember(Member(id,...))` (ID rỗng khi tạo) | Tách tạo `new Member(id do Service/Repo cấp theo API, ..., points=0)` + `addMember()` khỏi `updateMember()`; sau thành công gọi `members.save()` |
| **P1** | `StaffManagementView.cpp::form(bool creating)` | Nhận password lúc tạo nhưng cuối hàm luôn `updateProfile()` → `updateStaff()`; không `addStaff()` | Tách `addStaff(User*)` khi tạo, `updateStaff()` khi sửa; giữ role STAFF, không hiển thị mật khẩu; gọi `staff.save()` và xử lý con trỏ khi thêm lỗi |
| **P1** | `ShowtimeManagementView.cpp::form(bool creating)` | Dù tạo mới, cuối hàm vẫn `service.updateShowtime(values)`; ID khi tạo rỗng | Khi tạo gọi `addShowtime(Showtime*)`, khi sửa gọi `updateShowtime()`; sinh ID theo Backend; `showtimes.save()`; giữ kiểm tra thời gian/phim/phòng |
| **P1** | `MovieManagementView.cpp` | “Thêm phim” là `pending(...)`; sửa/xóa và `setMovieActive` có gọi Service | Kết nối `addMovie(Movie*)` và `movies.save()` cho thay đổi; xác minh quyền/các trường; `setMovieActive()` có trong header thực tế dù contract chưa liệt kê |
| **P1** | `RoomManagementView.cpp` | “Thêm phòng” pending; bật/tắt pending (header `RoomService` không có API setActive) | Kết nối `addRoom(Room*)`, `rooms.save()`; giữ nút bật/tắt pending đến khi Backend cung cấp API, không đổi `active` bằng `updateRoom()` |
| **P1** | `SeatMapView.cpp` | Đã gọi `addSeat`/`updateSeat`/`removeSeat`; có TODO về lịch sử vé; chưa thấy `seats.save()` trong luồng này | Thêm `seats.save()` sau thao tác; làm rõ ràng buộc seat đã có vé trước cho sửa/xóa; không lách Service để đọc Repo từ GUI |
| **P1** | `ShowtimeManagementView.cpp` | Hủy/xóa `pending`; contract v0.4 nói API còn trả false | Giữ vô hiệu hóa tới khi kiểm chứng implementation hỗ trợ vé và các điều kiện xóa/hủy; không bật chỉ vì có chữ ký API |
| **P2** | `BookingView.cpp` | Đang tự tính từng dòng qua `PricingService` | Khi có `calculateBooking`, Backend là nguồn duy nhất cho tổng tiền và điểm; bỏ phép tính trùng để tránh lệch kết quả |
| **P2** | `InvoiceHistoryView.cpp` | Có bộ lọc mã NV/phim/suất trên UI | `InvoiceService::getInvoiceHistory` chỉ nhận `staffId`, `memberId` và thời gian; lọc phim/suất tại lớp trình bày **trên tập hóa đơn đã được Service kiểm tra quyền**, không đọc thẳng Repo; tra cứu mã hóa đơn qua `findInvoiceById` |

## 3. Bổ sung ApplicationContext và khởi tạo

**Hiện tại:** `ApplicationContext.h` không khai báo Invoice/Ticket repository và 3 Service mới. `ApplicationContext.cpp::load()` nạp users, permissions, pricing, member, movie, room, seat, showtime; chưa nạp invoice/ticket. Vì vậy UI không thể gọi B11 Service thật dù các header Backend đã tồn tại.

**Phụ thuộc xác minh từ mã header:**

```text
UserRepository / RolePermissionRepository
MovieRepository, RoomRepository
SeatRepository(RoomRepository)
ShowtimeRepository(MovieRepository, RoomRepository)
MemberRepository
PricingRepository
InvoiceRepository(ShowtimeRepository, UserRepository, MemberRepository)
TicketRepository(ShowtimeRepository, SeatRepository, InvoiceRepository)

AuthService(UserRepository, RolePermissionRepository)
PricingService(PricingRepository, AuthService)
BookingService(ShowtimeRepository, SeatRepository, TicketRepository,
               InvoiceRepository, MemberRepository, PricingService, AuthService)
InvoiceService(InvoiceRepository, TicketRepository, AuthService)
ReportService(InvoiceRepository, TicketRepository, MovieRepository,
              ShowtimeRepository, AuthService)
```

**Đề xuất thứ tự `load()`** (suy ra từ các phụ thuộc): users → permissions → movies → rooms → seats → showtimes → members → pricing → invoices → tickets. Việc đổi vị trí members/pricing trong các bước độc lập có thể được, miễn phải nạp xong các phụ thuộc trước invoice/ticket và khởi chạy nghiệp vụ. Khi nạp thất bại, cần phân biệt lỗi làm toàn ứng dụng không hoạt động với lỗi riêng màn hình; bán vé/hủy vé/tra cứu hóa đơn/báo cáo phải bị khóa khi thiếu invoice/ticket, không được dùng dữ liệu bán vé giả để thay thế.

**Cảnh báo vòng đời:** khởi tạo/giải hủy theo chiều phụ thuộc; `TicketRepository` giữ tham chiếu Invoice/Seat/Showtime, `InvoiceRepository` giữ Showtime/User/Member. Không reload/xóa Repository nền khi đối tượng liên kết còn tồn tại.

**CMake:** bổ sung `src/business/{BookingService,InvoiceService,ReportService}.cpp` và `src/data_access/{InvoiceRepository,TicketRepository}.cpp` vào library. Tạo và bổ sung fixtures `tests/GUI/data/invoices.txt`, `tickets.txt`; không tự cho rằng fixture cũ đã đủ cho `load()`.

## 4. Các luồng tích hợp chi tiết

### 4.1. Bán vé — P0

**Cũ:** `BookingView::loadSeats()` duyệt `ctx.seats` và gọi `BookingAdapter::seatStatus()` luôn trả Unknown. `BookingView::quote()` tính giá riêng từ `PricingService`. `BookingView::setStep(3)` nhận `saleSupport() == NotImplemented`, nên không hoàn tất bán.

**Mới:**

1. Chọn suất hợp lệ → `ctx.booking.getAvailableSeats(showtimeId)`; biểu diễn chính xác ghế chưa bán, không hiểu danh sách rỗng là đồng nghĩa hết ghế (có thể do suất không đủ điều kiện).
2. Người dùng chọn ghế, loại vé `REGULAR`/`STUDENT`, member ID (chuỗi rỗng với khách vãng lai). Tạo `std::vector<BookingItem>`.
3. Tính lại hóa đơn bằng `ctx.booking.calculateBooking(showtimeId, items, memberId, BookingSummary&)`. Hiển thị `ticketPrices`, `totalAmount`, `earnedPoints` từ kết quả Service. Không tin vào báo giá hiển thị cũ khi tạo giao dịch.
4. Xác nhận tuổi và xác nhận đã nhận thanh toán; chỉ lúc người dùng chủ động bán mới gọi `createBooking(..., ageConfirmed, paymentConfirmed, invoiceId)`.
5. Thành công: hiển thị `invoiceId`, làm mới ghế/lịch sử; thất bại: bảo toàn trạng thái UI theo kết quả và phân biệt `false` với ngoại lệ. **Không tự gọi `save()` cho transaction bán vé:** Backend điều phối lưu nhiều repo.
6. Backend có cờ `storageFailed` khi ghi TXT lỗi: thông báo dừng bán/hủy cho phiên Service đó, không retry mù.

**Kiểm thử:** ghế đã bán không chọn được; hai lần bán cùng seat/showtime không tạo vé trùng; COUPLE tương ứng một Ticket; điểm thành viên, giảm giá Student, age confirmation, payment confirmation; xác nhận tạo xong có invoice ID; lỗi ghi dữ liệu không hiển thị thành công.

### 4.2. Hóa đơn và tra cứu — P0

**Cũ:** `SalesHistory::history()` không lấy dữ liệu thật, trả `NotImplemented` hoặc fixture demo. `InvoiceHistoryView::detail()` tái truy vấn demo.

**Mới:** dùng `InvoiceService::getInvoiceHistory(from,to,staffId,memberId)`, `findInvoiceById(invoiceId)` để enforce phạm vi xem khi mở chi tiết, `getInvoiceStatus`, `getRefundAmount`, `getRemainingAmount`; chuyển các thuộc tính Invoice/Ticket thành DTO hiển thị, không lưu con trỏ qua thao tác reload/xóa. Phạm vi: Admin `INVOICE_VIEW_ALL`, Staff `INVOICE_VIEW_OWN`; Staff chỉ xem danh sách hóa đơn của mình trong ngày hiện tại theo contract, tra chi tiết hóa đơn cũ bằng mã theo quyền.

Ngày UI chọn phải chuyển thành `[from,to)` theo ISO `YYYY-MM-DDTHH:MM:SS`, `to` là 00:00 ngày sau ngày kết thúc. Không gọi history với đầu mốc rỗng nếu Backend không hỗ trợ. Filter phim/suất có thể lọc trên tập hóa đơn đã được kiểm quyền và phải ghi rõ semantics (ví dụ không áp vào kết quả phân trang nếu sau này có).

**Kiểm thử:** Admin/Staff/khách; hóa đơn active/partially cancelled/cancelled; số tiền gốc, hoàn, còn lại; xem chi tiết hóa đơn cũ theo ID; định dạng ngày loại trừ đầu `to`.

### 4.3. Hủy vé — P0

**Cũ:** `TicketCancellationView::confirm()` chỉ mở dialog DEMO, nút xác nhận disable; `SalesHistory::cancellationLookup()` không truy xuất Backend.

**Mới:** cần luồng **đọc theo quyền** bằng `InvoiceService` và dữ liệu vé đã được Backend xác minh trước khi hiển thị. `BookingService` hiện chỉ khai báo `cancelTicket`, không có API riêng `getTicketById`/`quoteRefund`. Vì vậy **điểm cần thống nhất Backend–GUI**: thêm API Service tra cứu vé theo ID hoặc adapter chỉ lấy vé từ `InvoiceService` theo hóa đơn có thể tra được; với tìm kiếm trực tiếp Ticket ID, cần Service được phân quyền, không cho GUI gọi `TicketRepository` trực tiếp. Không tự suy đoán quyền hủy, số tiền hoàn hay điểm từ demo.

Sau khi Admin nhập lý do và đánh dấu đã hoàn tiền, gọi `ctx.booking.cancelTicket(ticketId, reason, refundConfirmed)`. Nếu thành công, làm mới hóa đơn, ticket và sơ đồ ghế. Không viết/xóa TXT từ GUI. Giữ vé hủy làm lịch sử; thiếu quyền phải chặn ở Service. Lỗi lưu nhiều file phải báo rõ và không tự chạy lại.

**Kiểm thử:** Staff không hủy được, vé đã CANCELLED bị từ chối, suất đã bắt đầu bị từ chối, thiếu lý do/xác nhận hoàn tiền, không đủ điểm Member, refresh và lịch sử hoàn tiền.

### 4.4. Thống kê — P0

**Cũ:** `SalesHistory::report()` là dữ liệu ví dụ/NotImplemented; `ReportView` chỉ hiển thị DTO.

**Mới:** gọi `getTotalSales`, `getTotalRefund`, `getActualRevenue`, `getSoldTicketCount`, `getCancelledTicketCount`, `getTopMovies`, và `getMovieRevenue` cho phim được xếp hạng. Tất cả cần `REPORT_VIEW` và Admin. Biểu đồ theo ngày cần gọi các API dòng tiền cho **mỗi ngày** (với `[dayStart,nextDayStart)`) vì header không có API tổng hợp chuỗi theo ngày. Không dùng doanh thu phim để thay thế tổng bán–hoàn.

**Cảnh báo từ contract v0.4:** `test_report_service.cpp` được ghi nhận **31 PASS, 1 FAIL**, trường hợp tổng Ticket paidAmount không khớp Invoice totalAmount chưa bị từ chối. Đây là **kết quả được ghi trong contract**, không phải test được chạy trong lần rà soát này. Gắn nhãn giới hạn và yêu cầu Backend sửa/kiểm thử trước khi nghiệm thu doanh thu.

### 4.5. Các màn hình danh mục — P1

- **Member:** sửa nhánh `creating` thành `addMember(Member*)`; dùng ID Backend sinh theo cách hiện hành, điểm ban đầu 0; không update bằng ID rỗng; save qua `MemberService::save()`; kiểm tra duplicate phone.
- **Staff:** sửa `creating` thành `addStaff(User*)`, dùng password nhập tạo mới; nhánh cập nhật giữ password hiện tại, `deactivateStaff()`; save qua `StaffService::save()`; không log/hiển thị password.
- **Showtime:** `creating` gọi `addShowtime(Showtime*)`, nhánh còn lại `updateShowtime()`; chỉ SCHEDULED, lịch tương lai/đúng thời lượng và khoảng nghỉ; save; nút hủy/xóa còn pending theo contract.
- **Movie/Room:** mở các nút “Thêm” hiện pending, tạo Model với ID chuẩn rồi gọi `addMovie`/`addRoom`; cập nhật/xóa/đổi trạng thái (Movie) gọi `save()` và kiểm tra lỗi lưu. `RoomService` hiện không có `setRoomActive`.
- **Seat:** bổ sung `save()` cho sửa/xóa/thêm; ràng buộc ghế có liên quan vé cần nâng cấp Backend hoặc giữ khóa UI cho những ca chưa bảo đảm.
- **Pricing:** `PricingManagementView` đã gọi `getPricing()`, `updateSeatPrice`, `updateDiscountRate` thật; theo contract `PricingRepository::update()` ghi file ngay. Không gọi `save()` chung một cách mặc định cho thao tác giá.

### 4.6. Phân quyền và ID

- UI hiện dùng `hasPermission()` để ẩn/hạn chế nút; Backend phải tiếp tục kiểm bằng `requirePermission()`/quy tắc role. **Admin cũng cần mã permission** trên bảng quyền.
- Đối với các luồng thêm, contract ghi cách cấp ID hiện nay dựa số lớn nhất đã nạp rồi cộng một, **không** lưu/đọc `sequences.txt` trong GUI. Nên xem trực tiếp cài đặt `Service::add...` tại thời điểm sửa trước khi tạo Model để tránh tự sinh ID giả.
- Dữ liệu Model trả qua con trỏ từ Repo/Service là **mượn**; GUI không `delete`. Đối với con trỏ truyền cho `add...`, quyền sở hữu chuyển sang Repository **chỉ sau khi thêm thành công**; dùng RAII (`unique_ptr` + `release()` sau `true`) để giảm rò rỉ.

## 5. Checklist triển khai có thứ tự

### P0: Khơi thông giao dịch thật
- [ ] Cập nhật `CMakeLists.txt` để build 5 file `.cpp` mới (3 Service + 2 Repository).
- [ ] Bổ sung và khởi tạo 2 Repo + 3 Service trong `ApplicationContext` với thứ tự thành viên đúng vòng đời.
- [ ] Load `invoices.txt`, `tickets.txt` sau tất cả tham chiếu; phân loại và hiển thị lỗi load ở mỗi màn hình.
- [ ] Bổ sung fixtures GUI hợp lệ cho invoices/tickets, kiểm tra load và restart.
- [ ] Chuyển `BookingAdapter` từ hardcoded sang `BookingService`; bật luồng bán thật có xác nhận.
- [ ] Chuyển `SalesHistoryAdapter::history` sang `InvoiceService`, chuyển detail sang `findInvoiceById`.
- [ ] Chốt API tìm vé theo ID/điều kiện hoàn tiền ở tầng Service; chuyển nút hủy từ DEMO sang `cancelTicket`.
- [ ] Chuyển report từ DEMO sang `ReportService`; xử lý kỳ ngày/tháng và empty/error.
- [ ] Regression test phân quyền, lỗi ghi nhiều file, restart và tính nhất quán.

### P1: Hoàn chỉnh CRUD
- [ ] Sửa `MemberManagementView` gọi add khi tạo; save.
- [ ] Sửa `StaffManagementView` gọi add khi tạo; save.
- [ ] Sửa `ShowtimeManagementView` gọi add khi tạo; save.
- [ ] Mở Thêm Movie và Room khi có đường cấp ID chuẩn; save.
- [ ] Đồng bộ save sau tất cả thay đổi danh mục Movie/Room/Seat/Showtime/Staff/Member.
- [ ] Rà soát ràng buộc ghế/vé và trạng thái showtime; không bật tính năng chưa bảo đảm.

### P2: Hoàn thiện vận hành
- [ ] Bỏ/cô lập cờ DEMO khỏi luồng production (có thể giữ fixture trong test).
- [ ] Kiểm thử luồng đồng bộ sau bán/hủy: Booking ↔ lịch sử ↔ báo cáo ↔ seat map.
- [ ] Bổ sung UI tests có dữ liệu thật và negative tests; chạy `ctest` thực tế trước khi đánh dấu PASS.
- [ ] Cập nhật contract khi thống nhất API tìm vé và khắc phục vấn đề ReportService được nêu trong v0.4.

## 6. Tiêu chí nghiệm thu tối thiểu

| Nhóm | Điều kiện PASS |
|---|---|
| Khởi động | GUI tải được dữ liệu liên quan và báo lỗi đúng khi thiếu file vé/hóa đơn; restart phục hồi dữ liệu |
| Bán vé | Một giao dịch tạo invoice và tickets thật, tồn tại sau restart; ghế đã bán bị khóa |
| Hủy vé | Admin hủy vé hợp lệ, lưu thông tin hoàn tiền, ghế khả dụng lại, lịch sử vẫn có vé đã hủy |
| Lịch sử | Admin/Staff đúng quyền, tổng ban đầu/hoàn/còn lại đúng, filter thời gian đúng `[from,to)` |
| Báo cáo | KPI khớp Service, ngày hủy tiền hoàn được tính riêng ngày bán; không dùng số liệu demo; xử lý rõ 1 FAIL đã ghi nhận |
| Danh mục | Tạo/sửa/xóa phù hợp API, ghi TXT và tồn tại sau restart; không dùng `update...` để tạo bản ghi |
| Phân quyền | Không nâng quyền bằng chỉnh UI; thiếu permission/role bị Backend từ chối |

## 7. File nguồn đã đọc trực tiếp để lập báo cáo

**Contract:** `docs/CauTrucThuMuc/gui(docs)/Business_UI_Contract.md` v0.4.  
**Backend headers:** `src/business/BookingService.h`, `InvoiceService.h`, `ReportService.h`, `PricingService.h`, `MovieService.h`, `RoomService.h`, `ShowtimeService.h`, `MemberService.h`; `src/data_access/TicketRepository.h`, `InvoiceRepository.h`.  
**GUI:** `ApplicationContext.h/.cpp`, `BookingAdapter.h`, `BookingView.cpp`, `SalesHistoryAdapter.h/.cpp`, `InvoiceHistoryView.h/.cpp`, `TicketCancellationView.h/.cpp`, `ReportView.h/.cpp`, `MainWindow.cpp`, `PricingManagementView.cpp`, `MemberManagementView.cpp`, `StaffManagementView.cpp`, `MovieManagementView.cpp`, `RoomManagementView.cpp`, `ShowtimeManagementView.cpp`, `SeatMapView.cpp`, `src/gui/main.cpp`, `CMakeLists.txt`.

**Giới hạn kiểm chứng:** đây là rà soát mã từ GitHub và contract, **không** biên dịch/cài Qt, không chạy CTest, không xác minh mọi file `.cpp` Backend bên trong. Một số đề xuất như thứ tự load, cách DTO hóa, hay thêm API tra vé là **khuyến nghị tích hợp**, không khẳng định đã có API tương ứng. Cần đối chiếu trực tiếp mã cài đặt khi bắt đầu sửa từng module.

**Liên kết tham chiếu:** [nhánh B13](https://github.com/duyhoangbk2007/PBL2/tree/feature/b13-hoang-kiet-gui-backend-integration) · [commit cập nhật contract](https://github.com/duyhoangbk2007/PBL2/commit/9d43c2ada7ff87ea003f82b8d970a69cc000441e).
