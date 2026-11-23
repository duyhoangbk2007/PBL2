# Hợp đồng GUI – Business Service

**Trạng thái:** API trong bảng là **hiện tại theo header**; GUI và thay đổi DIP/RBAC là **dự kiến**. Chưa sửa code hoặc GUI. Đọc cùng [DIP](dependency_inversion.md), [Permission](PermissionDesign.md), [quy tắc nghiệp vụ](../overview/business_rules.md) và [định dạng dữ liệu](../overview/data_format.md).

## 1. Ranh giới bắt buộc

GUI chỉ thu dữ liệu, hiển thị kết quả và gọi public API của Service. GUI có thể hỏi `AuthService::hasPermission(code)` để ẩn hoặc tắt chức năng, nhưng **Service luôn gọi `requirePermission(code)`** và kiểm tra điều kiện nghiệp vụ tại thời điểm thực hiện. GUI không gọi Repository để sửa/tra dữ liệu nghiệp vụ, không ghi TXT, không tự kết luận thao tác hợp lệ theo tuổi, trạng thái suất, ghế trống, điểm hay phạm vi hóa đơn. Repository xử lý CRUD, liên kết và tính toàn vẹn; Service điều phối, Permission và quy tắc phối hợp. Model trả qua GUI là con trỏ mượn, không `delete` hay sửa trực tiếp.

```text
Người dùng -> GUI -> Service -> interface Repository [dự kiến]
                         -> AuthService::requirePermission()
Service -> bool / con trỏ / vector / số / exception -> GUI -> thông báo & làm mới
```

**Hiện tại:** các Service nhận concrete repository qua constructor; chưa có `src/gui/`. Refactor DIP dự kiến đổi kiểu dependency trong constructor và include nội bộ, **không tự đổi** tên hàm, tham số, kiểu trả về của API GUI. Interface và bản code đầy đủ phải được nhóm **CHỐT MARKDOWN** trước triển khai.

## 2. Hợp đồng kết quả và lỗi

| Kết quả hiện có | GUI xử lý | Điều cần giữ |
| --- | --- | --- |
| `bool true` | Thông báo thành công, làm mới dữ liệu. | Với Service danh mục, kiểm tra hợp đồng `save()` riêng; không giả định thao tác đã bền vững nếu chưa save. |
| `bool false` | Thông báo thao tác không thực hiện; giữ input để sửa. | Không suy mọi `false` là lỗi nhập; có thể chưa load, đối tượng không có hoặc quy tắc không đạt. |
| `T*`/`const T*`, `nullptr` | Có dữ liệu / không tìm thấy theo hợp đồng từng hàm. | Con trỏ mượn; không sửa/delete/giữ sau xóa hoặc reload. |
| `vector<T*>`, số, enum | Hiển thị theo ý nghĩa từng hàm. | Vector cũng chứa con trỏ mượn. `0`/rỗng có thể là kết quả hợp lệ, không phải mã lỗi chung. |
| `invalid_argument`, `runtime_error` hoặc exception khác | Bắt tại biên GUI, hiện thông điệp phù hợp, không báo thành công. | Permission, chưa load, dữ liệu sai và lỗi ghi có thể ném; không nuốt ngoại lệ hay tự thử lại giao dịch lưu thất bại. |

Các hàm `addXxx(T*)` chuyển quyền sở hữu cho repository **khi thêm thành công**; nếu thất bại/ném trước khi nhận, caller giải phóng theo hợp đồng đã chốt. GUI nên điều phối qua Service và tránh tự quản lý raw pointer dài hạn. Cách dùng smart pointer hoặc sửa chữ ký là **chờ xác nhận**, không tự đổi trong bước tài liệu.

## 3. API hiện tại cho xác thực và danh mục

| Service | Public API hiện có | Permission/ghi chú |
| --- | --- | --- |
| `AuthService` | `login(username,password): bool`, `logout(): void`, `getCurrentUser(): const User*`, `isLoggedIn(): bool`, `hasPermission(code): bool`, `requirePermission(code): const User&`; còn `isAdmin/isStaff/requireAdmin/requireLogin` | GUI dùng `hasPermission` để điều khiển hiển thị. `requireAdmin` không dùng để cấp quyền nghiệp vụ theo thiết kế đích. |
| `MovieService` | `load/save`, `addMovie`, `updateMovie`, `removeMovie`, `setMovieActive`, `findMovieById`, `getMovieAt`, `getMovieCount`, `getNowShowingMovies`, `getComingSoonMovies`, `searchMoviesByTitle` | `MOVIE_ADD/UPDATE/DELETE/VIEW`; `setMovieActive` dùng `MOVIE_UPDATE`. Query trả `const Movie*` hoặc `vector<const Movie*>`. |
| `RoomService` | `load/save`, `addRoom`, `updateRoom`, `removeRoom`, `findRoomById`, `getRoomAt`, `getRoomCount` | `ROOM_ADD/UPDATE/DELETE/VIEW`; query trả `const Room*`. |
| `SeatService` | `load/save`, `addSeat`, `updateSeat`, `removeSeat(roomId,seatId)`, `findSeatById(roomId,seatId)`, `getSeatAt`, `getSeatCount` | `SEAT_ADD/UPDATE/DELETE/VIEW`; khóa ghép, không tra chỉ bằng `seatId`. Query trả `const Seat*`. |
| `ShowtimeService` | `load/save`, `addShowtime`, `updateShowtime`, `cancelShowtime`, `removeShowtime`, `findShowtimeById`, `getShowtimeAt`, `getShowtimeCount` | `SHOWTIME_ADD/UPDATE/CANCEL/DELETE/VIEW`. Hủy/xóa có chữ ký nhưng code hiện trả `false` sau kiểm tra; chưa coi là chức năng hoàn thành. |
| `StaffService` | `load/save`, `addStaff`, `updateStaff`, `deactivateStaff`, `findStaffById`, `findStaffByUsername`, `getStaffAt`, `getStaffCount` | `STAFF_ADD/UPDATE/DEACTIVATE/VIEW`; lọc User có role STAFF là điều kiện đối tượng, không cấp quyền. Không hiển thị password. |
| `MemberService` | `load/save`, `addMember`, `updateMember`, `findMemberById`, `findMemberByPhone`, `getMemberAt`, `getMemberCount` | `MEMBER_ADD/UPDATE/VIEW`; không có API xóa hoặc chỉnh điểm trực tiếp. |

`load/save` trong các Service danh mục là API hiện có để quản lý vòng đời dữ liệu, không phải nút quản trị công khai. GUI không tự gọi để bỏ qua Permission; nơi ghép nối nạp và lưu theo hợp đồng đã thống nhất. Các thao tác thêm nhận con trỏ; cập nhật nhận `const T&` theo header. Không tự đổi tên hoặc return type vì DIP.

## 4. Giá, bán vé và hủy vé

| Service/API hiện tại | Dữ liệu vào/ra và ý nghĩa | Quyền/điểm cần review |
| --- | --- | --- |
| `PricingService::getPricing()` | `const Pricing*`; đọc cấu hình. | `PRICING_VIEW`. |
| `updateSeatPrice(type,price)`, `updateDiscountRate(type,rate)` | `bool`; repository cập nhật và ghi cấu hình. | `PRICING_UPDATE`; lỗi lưu phải hiển thị. |
| `getBasePrice(type)` | `double`. | `PRICING_VIEW`. |
| `calculateDiscount(basePrice,ticketType,isMember)`, `calculatePaidAmount(basePrice,discountAmount)`, `calculatePoints(paidAmount,isMember)` | `double/double/int`; helper tính giá công khai. | Hiện chỉ `requireLogin`; **chờ xác nhận** Permission hoặc chuyển thành helper nội bộ, không tự tạo mã mới. |
| `BookingService::getAvailableSeats(showtimeId)` | `vector<Seat*>` mượn; ghế có thể đổi trạng thái sau khi hiển thị. | Rà quyền đọc; GUI phải cho Service kiểm tra lại khi bán. |
| `calculateBooking(showtimeId,items,memberId,summary)` | `bool`, ghi `BookingSummary` khi hợp lệ; không xác nhận bán. | Rà quyền preview/tính giá; không dùng kết quả này thay kiểm tra lúc bán. |
| `createBooking(showtimeId,items,memberId,ageConfirmed,paymentConfirmed,invoiceId)` | `bool`; `invoiceId` có nghĩa khi thành công. | `TICKET_SELL`; Service kiểm tra tuổi, tiền, ghế, điểm và ghi nhiều TXT. |
| `cancelTicket(ticketId,reason,refundConfirmed)` | `bool`; vé giữ lịch sử, hoàn tiền/điểm theo snapshot. | Hiện `requireAdmin()` rồi `TICKET_CANCEL`; mục tiêu Permission-only. Xung đột “chỉ Admin” trong nghiệp vụ cần nhóm chốt. |

`BookingItem` gồm `seatId` và `ticketType`. GUI chỉ xác nhận thu/hoàn tại quầy, không tự sửa Ticket/Invoice/Member. `BookingService` có `storageFailed`: lỗi ghi TXT ném ngoại lệ và chặn thao tác tiếp trên cùng instance; nhiều file không được ghi nguyên tử. GUI phải báo rõ trạng thái cần kiểm tra dữ liệu, không tự retry. `PricingService` ghi khi cập nhật; các Service danh mục thường cần `save()` riêng; Booking tự điều phối lưu liên quan.

## 5. Lịch sử hóa đơn và báo cáo

`InvoiceService` hiện nhận `InvoiceRepository`, `TicketRepository`, `AuthService`. Public API: `findInvoiceById(invoiceId): Invoice*`; `getInvoiceHistory(from,to,staffId="",memberId=""): vector<Invoice*>`; `getInvoiceStatus(invoiceId): InvoiceStatus`; `getRefundAmount(invoiceId): double`; `getRemainingAmount(invoiceId): double`. Khoảng lọc `[from,to)`. Không tìm thấy với `findInvoiceById` trả `nullptr`; các phép tính yêu cầu hóa đơn có thật và có thể ném. Hiện code chọn `INVOICE_VIEW_ALL` hay `INVOICE_VIEW_OWN` theo role; mục tiêu chọn theo Permission và **Service vẫn kiểm tra** owner, ngày và filter. GUI không dùng `staffId` để tự vượt phạm vi. Quy tắc ưu tiên khi có cả hai Permission chờ xác nhận.

`ReportService` hiện nhận `InvoiceRepository`, `TicketRepository`, `MovieRepository`, `ShowtimeRepository`, `AuthService`. Public API: `getTotalSales(from,to): double`, `getTotalRefund(from,to): double`, `getActualRevenue(from,to): double`, `getSoldTicketCount(from,to): int`, `getCancelledTicketCount(from,to): int`, `getMovieRevenue(movieId,from,to): double`, `getTopMovies(from,to,limit): vector<Movie*>`. Thời gian `[from,to)`; tiền bán theo thời điểm hóa đơn, hoàn theo thời điểm hủy; top phim theo vé còn SOLD và doanh thu giảm, hòa theo `movieId`. Hiện còn `requireAdmin()` trước `REPORT_VIEW`; mục tiêu chỉ `REPORT_VIEW`. Dữ liệu lỗi/chưa load, khoảng sai hoặc `limit <= 0` có thể ném; kỳ hợp lệ rỗng trả 0/vector rỗng. Tài liệu overview ghi còn ca ReportService chưa đối chiếu tổng Ticket–Invoice; **không xem backend Report đã nghiệm thu** nếu chưa chạy lại test và sửa theo Markdown-first.

## 6. Tác động DIP và điều cần xác nhận

**Dự kiến đổi bên trong:** constructor Service sẽ nhận interface theo domain thay concrete repository; `AuthService` cũng nhận abstraction User/Permission; tầng ghép nối tạo concrete, nạp theo thứ tự rồi truyền tham chiếu. GUI tiếp tục gọi Service public API đã chốt. Interface đặc thù cho Seat, Pricing, RolePermission, Ticket, Invoice được phân tích tại [DIP](dependency_inversion.md). Chưa có code hoặc test refactor.

**Chờ xác nhận:** tên/chữ ký interface, `const` và quyền sở hữu con trỏ; Permission cho preview/tính giá; ưu tiên `INVOICE_VIEW_ALL/OWN`; ý nghĩa “chỉ Admin” khi hủy vé; chính sách gọi `load/save`; cách GUI trình bày lỗi khi `bool false` không có lý do chi tiết. Không đổi public API hoặc tạo Permission mới khi chưa chốt.

## 7. Phối hợp và nghiệm thu

Hoàng và Kiệt dùng cùng tài liệu contract; mọi đổi public API, Permission, TXT hoặc quy tắc nghiệp vụ phải thông báo và review trước. Khi thiết kế thay đổi, viết đầy đủ `.h/.cpp` trong Markdown, giải thích từng hàm, được **CHỐT MARKDOWN** rồi mới sửa code. GUI chỉ tích hợp API đã có và đã kiểm thử; chức năng chưa xong ghi `PENDING`. Test thực tế phải xác nhận đường quyền, dữ liệu, exception, lưu/rollback và con trỏ mượn; `docs/src/` cập nhật sau khi code tương ứng PASS.
