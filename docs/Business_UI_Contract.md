# Business – UI Contract

**Phiên bản:** 0.4 (Draft)
**Nhánh tham chiếu:** `feature/b11-hoang-cancellation-report-management` (mã nguồn đang làm việc)
**Phạm vi:** API đối chiếu với mã nguồn đang làm việc trong `src/business`; các mục chưa triển khai được ghi rõ.

## 1. Nguyên tắc tích hợp

- UI chỉ gọi tầng `Business Service` để thực hiện nghiệp vụ.
- UI không trực tiếp gọi Repository để thêm, sửa, xóa hoặc tra cứu dữ liệu nghiệp vụ.
- UI không tự đọc/ghi các file TXT.
- Các Service và Repository được khởi tạo, nạp dữ liệu trước khi UI thực hiện nghiệp vụ.
- UI sử dụng các đối tượng Service được truyền vào, không tự tạo lại Repository hoặc AuthService cho từng màn hình.
- Khi backend thay đổi, hai bên cập nhật contract trước khi tích hợp thay đổi.
- Trên nhánh hiện tại, quyền nghiệp vụ do `RolePermissionRepository` quyết định theo mã permission; kể cả Admin cũng cần có permission trong bảng quyền. `load()`/`save()` không kiểm tra quyền; ứng dụng gọi trong luồng khởi tạo/lưu, không gọi sau từng thao tác UI.
- Nạp `UserRepository` và `RolePermissionRepository` trước khi sử dụng phân quyền; nạp Movie và Room trước Showtime, Room trước Seat. Các Repository và Service được truyền bằng tham chiếu phải sống lâu hơn Service/UI đang sử dụng.

## 2. AuthService – Đăng nhập và phân quyền

Constructor:

```cpp
AuthService(UserRepository& userRepo,
            RolePermissionRepository& permissionRepo);
```

| API | Kết quả |
|---|---|
| `login(username, password)` | `bool` |
| `logout()` | `void` |
| `getCurrentUser()` | `const User*` |
| `isLoggedIn()` | `bool` |
| `isAdmin()` / `isStaff()` | `bool` |
| `hasPermission(const string& permission)` | `bool` |
| `requireLogin()` | `const User&` |
| `requireAdmin()` | `const User&` |
| `requirePermission(const string& permission)` | `const User&` |

UI dùng `hasPermission()` để hiển thị hoặc vô hiệu hóa chức năng theo mã quyền; hàm trả `false` khi chưa đăng nhập hoặc chưa nạp bảng quyền.

Các Service bên dưới dùng `requirePermission()` cho nghiệp vụ; hàm có thể ném `runtime_error` khi chưa đăng nhập hoặc thiếu quyền. `requireAdmin()` và `requireLogin()` vẫn có trong AuthService. Việc ẩn nút trên UI không thay thế kiểm tra quyền.

`login()` xóa phiên cũ trước khi thử đăng nhập; chỉ nhận Admin/Staff còn hoạt động. Hàm trả `false` khi thông tin không hợp lệ và ném `runtime_error` nếu dữ liệu User chưa nạp. `getCurrentUser()` trả `nullptr` nếu phiên không còn hợp lệ.

## 3. MovieService – Quản lý phim

Constructor:

```cpp
MovieService(MovieRepository& movieRepo,
             AuthService& authService);
```

| API | Kiểu trả về | Permission |
|---|---|---|
| `load()` / `save()` | `bool` | Không kiểm tra quyền |
| `addMovie(Movie*)` | `bool` | `MOVIE_ADD` |
| `updateMovie(const Movie&)` | `bool` | `MOVIE_UPDATE` |
| `removeMovie(const string& movieId)` | `bool` | `MOVIE_DELETE` |
| `findMovieById(const string& movieId)` | `const Movie*` | `MOVIE_VIEW` |
| `getMovieAt(int index)` | `const Movie*` | `MOVIE_VIEW` |
| `getMovieCount()` | `int` | `MOVIE_VIEW` |

Thông tin phim để UI hiển thị: `movieId`, `title`, `genre`, `durationMinutes`, `minAge`, `releaseDate`, `endDate`, `active`.
`updateMovie()` không đổi `active` hoặc khoảng ngày khi phim đã có suất chiếu. Repository chặn xóa phim còn liên kết suất chiếu.

## 4. RoomService – Quản lý phòng

Constructor:

```cpp
RoomService(RoomRepository& roomRepo,
            AuthService& authService);
```

| API | Kiểu trả về | Permission |
|---|---|---|
| `load()` / `save()` | `bool` | Không kiểm tra quyền |
| `addRoom(Room*)` | `bool` | `ROOM_ADD` |
| `updateRoom(const Room&)` | `bool` | `ROOM_UPDATE` |
| `removeRoom(const string& roomId)` | `bool` | `ROOM_DELETE` |
| `findRoomById(const string& roomId)` | `const Room*` | `ROOM_VIEW` |
| `getRoomAt(int index)` | `const Room*` | `ROOM_VIEW` |
| `getRoomCount()` | `int` | `ROOM_VIEW` |

Thông tin phòng: `roomId`, `name`, `rowCount`, `columnCount`, `active`.
`updateRoom()` không đổi `active` hoặc kích thước khi có suất chiếu. Repository chặn thu nhỏ vượt vị trí ghế và xóa phòng còn ghế/suất chiếu.

## 5. SeatService – Quản lý ghế

Constructor:

```cpp
SeatService(SeatRepository& seatRepo,
            AuthService& authService);
```

| API | Kiểu trả về | Permission |
|---|---|---|
| `load()` / `save()` | `bool` | Không kiểm tra quyền |
| `addSeat(Seat*)` | `bool` | `SEAT_ADD` |
| `updateSeat(const Seat&)` | `bool` | `SEAT_UPDATE` |
| `removeSeat(const string& roomId, const string& seatId)` | `bool` | `SEAT_DELETE` |
| `findSeatById(const string& roomId, const string& seatId)` | `const Seat*` | `SEAT_VIEW` |
| `getSeatAt(int index)` | `const Seat*` | `SEAT_VIEW` |
| `getSeatCount()` | `int` | `SEAT_VIEW` |

Thông tin ghế: `roomId`, `seatId`, `row`, `column`, `seatType`.

`SeatType` có ba giá trị: `STANDARD`, `VIP`, `COUPLE`.
Khóa ghép là `(roomId, seatId)`; vị trí trong phòng phải duy nhất. `updateSeat()` chỉ đổi loại ghế, không đổi khóa/vị trí. Chưa có kiểm tra vé đã tham chiếu ghế khi sửa/xóa sơ đồ.

## 6. ShowtimeService – Quản lý suất chiếu

Constructor:

```cpp
ShowtimeService(ShowtimeRepository& showtimeRepo,
                MovieRepository& movieRepo,
                RoomRepository& roomRepo,
                AuthService& authService);
```

| API | Kiểu trả về | Permission |
|---|---|---|
| `load()` / `save()` | `bool` | Không kiểm tra quyền |
| `addShowtime(Showtime*)` | `bool` | `SHOWTIME_ADD` |
| `updateShowtime(const Showtime&)` | `bool` | `SHOWTIME_UPDATE` |
| `cancelShowtime(const string& showtimeId)` | `bool` | `SHOWTIME_CANCEL` |
| `removeShowtime(const string& showtimeId)` | `bool` | `SHOWTIME_DELETE` |
| `findShowtimeById(const string& showtimeId)` | `const Showtime*` | `SHOWTIME_VIEW` |
| `getShowtimeAt(int index)` | `const Showtime*` | `SHOWTIME_VIEW` |
| `getShowtimeCount()` | `int` | `SHOWTIME_VIEW` |

Thông tin suất chiếu: `showtimeId`, `movieId`, `roomId`, `startAt`, `endAt`, `status`.

`ShowtimeStatus`: `SCHEDULED`, `CANCELLED`.

**Chưa hoàn thiện:** `cancelShowtime()` và `removeShowtime()` hiện chưa thực hiện thành công việc hủy/xóa vì còn chờ kiểm tra dữ liệu vé. UI chưa được coi hai chức năng này là đã sẵn sàng.
Hai hàm có chữ ký nhưng sau kiểm tra quyền/điều kiện đều trả `false`. Thêm suất chỉ nhận `SCHEDULED`, giờ bắt đầu trong tương lai, ngày thuộc khoảng phát hành phim, giờ kết thúc khớp thời lượng; sửa chỉ nhận suất `SCHEDULED` chưa bắt đầu. Repository chặn đổi phim/phòng và trùng lịch.

## 7. StaffService – Quản lý nhân viên

Constructor: `StaffService(UserRepository& staffRepo, AuthService& authService)`.

| API | Kiểu trả về | Permission |
|---|---|---|
| `load()` / `save()` | `bool` | Không kiểm tra quyền |
| `addStaff(User*)` | `bool` | `STAFF_ADD` |
| `updateStaff(const User&)` | `bool` | `STAFF_UPDATE` |
| `deactivateStaff(const string& staffId)` | `bool` | `STAFF_DEACTIVATE` |
| `findStaffById(const string& staffId)` | `const User*` | `STAFF_VIEW` |
| `findStaffByUsername(const string& username)` | `const User*` | `STAFF_VIEW` |
| `getStaffAt(int index)` | `const User*` | `STAFF_VIEW` |
| `getStaffCount()` | `int` | `STAFF_VIEW` |

Chỉ thêm/sửa/trả về User có role `STAFF`. `updateStaff()` không đổi trạng thái `active`; `deactivateStaff()` chỉ áp dụng cho Staff đang hoạt động. `getStaffAt()` và `getStaffCount()` lọc Staff trong `UserRepository`. Model `User` có trường `password`; UI không hiển thị mật khẩu.

## 8. MemberService – Quản lý thành viên

Constructor: `MemberService(MemberRepository& memberRepo, AuthService& authService)`.

| API | Kiểu trả về | Permission |
|---|---|---|
| `load()` / `save()` | `bool` | Không kiểm tra quyền |
| `addMember(Member*)` | `bool` | `MEMBER_ADD` |
| `updateMember(const Member&)` | `bool` | `MEMBER_UPDATE` |
| `findMemberById(const string& memberId)` | `const Member*` | `MEMBER_VIEW` |
| `findMemberByPhone(const string& phone)` | `const Member*` | `MEMBER_VIEW` |
| `getMemberAt(int index)` | `const Member*` | `MEMBER_VIEW` |
| `getMemberCount()` | `int` | `MEMBER_VIEW` |

Model `Member` có `memberId`, `fullName`, `phone`, `points`. Repository yêu cầu thành viên mới có `points = 0`, số điện thoại hợp lệ và duy nhất; `updateMember()` không dùng để đổi điểm. Chưa có API xóa thành viên, chỉnh điểm trực tiếp hoặc xem lịch sử hóa đơn.

## 9. PricingService và BookingService trên nhánh Bước 11

`PricingService` đã có API xem và cập nhật bảng giá, tính giá gốc, ưu đãi, tiền trả và điểm. Quyền xem giá là `PRICING_VIEW`; quyền sửa giá là `PRICING_UPDATE`. Cập nhật giá ghi TXT trong `PricingRepository::update()`.

`BookingService` nhận `ShowtimeRepository`, `SeatRepository`, `TicketRepository`, `InvoiceRepository`, `MemberRepository`, `PricingService` và `AuthService` bằng tham chiếu. Các phụ thuộc cần được tạo và nạp trước khi GUI gọi Service.

| API | Đầu vào/kết quả | Điều kiện chính |
| --- | --- | --- |
| `getAvailableSeats(showtimeId)` | `vector<Seat*>` | Suất `SCHEDULED`, chưa bắt đầu, phòng hoạt động; danh sách rỗng có nhiều nguyên nhân |
| `calculateBooking(showtimeId, items, memberId, summary)` | `bool`; `summary` gồm giá từng vé, tổng tiền, tổng điểm | Ghế hợp lệ/chưa bán, không trùng, loại vé hợp lệ; điểm tính trên từng Ticket |
| `createBooking(showtimeId, items, memberId, ageConfirmed, paymentConfirmed, invoiceId)` | `bool`; `invoiceId` chỉ có giá trị khi thành công | `TICKET_SELL`, xác nhận tuổi và thu tiền; người bán lấy từ phiên đăng nhập |
| `cancelTicket(ticketId, reason, refundConfirmed)` | `bool` | Chỉ Admin có `TICKET_CANCEL`; vé `SOLD`, suất chưa bắt đầu, lý do hợp lệ, xác nhận đã hoàn tiền, Member đủ điểm |

`BookingItem` gồm `seatId` và `ticketType` (`REGULAR`/`STUDENT`). `getAvailableSeats()` trả con trỏ mượn từ Repository: GUI không sửa hoặc xóa Model. Danh sách ghế trống chỉ để hiển thị; `createBooking()` kiểm tra lại lúc bán.

### Hủy vé và trách nhiệm GUI

GUI cho Admin chọn `ticketId`, nhập `reason`, xác nhận đã hoàn tiền tại quầy rồi gọi `cancelTicket()`. GUI làm mới vé và sơ đồ ghế khi thành công. Service lấy Admin ID từ `AuthService`, gọi `Ticket::markCancelled()`, thu hồi đúng `pointsEarned` đã lưu trên Ticket và ghi `tickets.txt` rồi `members.txt` nếu hóa đơn có Member. Invoice và Seat không bị sửa; Ticket hủy vẫn được giữ làm lịch sử. Ghế trở lại khả dụng khi không còn Ticket `SOLD` cùng suất/ghế.

Điều kiện nghiệp vụ không đạt trả `false` và không ghi file. Chưa đăng nhập, sai vai trò hoặc thiếu quyền ném ngoại lệ. Lỗi ghi TXT cũng ném ngoại lệ, đặt `storageFailed` và chặn bán/hủy tiếp trên cùng Service; GUI cần hiển thị lỗi, không tự thử lại giao dịch. Nhiều file TXT không được ghi nguyên tử; cần kiểm tra dữ liệu trước khi khởi tạo Service mới. Xem [BookingService](<CauTrucThuMuc/business(docs)/BookingService.md>) để biết luồng xử lý và phạm vi test.

ID MOV/ROM/SHW/USR/MEM/INV/TKT hiện lấy số lớn nhất trong bản ghi đang nạp rồi cộng một, không dùng `sequences.txt`. Các Service danh mục cần gọi `save()` riêng sau thêm/sửa; `PricingService` ghi khi cập nhật giá; `createBooking()` và `cancelTicket()` tự điều phối lưu các Repository liên quan.

## 10. InvoiceService — lịch sử hóa đơn

Constructor: `InvoiceService(InvoiceRepository& invoiceRepo, TicketRepository& ticketRepo, AuthService& authService)`. Service chỉ đọc; GUI không tự truy cập Repository. `Invoice*` và các phần tử của `vector<Invoice*>` là con trỏ mượn, không được GUI sửa hoặc `delete`.

| API | Kết quả và điều kiện |
| --- | --- |
| `findInvoiceById(const string& invoiceId)` | `Invoice*` hoặc `nullptr` nếu mã không có. Admin cần `INVOICE_VIEW_ALL`; Staff cần `INVOICE_VIEW_OWN` và chỉ được xem hóa đơn do mình lập, kể cả hóa đơn cũ. |
| `getInvoiceHistory(const string& from, const string& to, const string& staffId = "", const string& memberId = "")` | `vector<Invoice*>`, lọc theo `createdAt` trong `[from,to)`, thêm bộ lọc nhân viên/thành viên khi chuỗi không rỗng; xếp `createdAt` giảm, nếu trùng thì `invoiceId` tăng. Staff chỉ liệt kê hóa đơn của mình trong ngày hiện tại; không được dùng `staffId` khác để vượt phạm vi. |
| `getInvoiceStatus(const string& invoiceId)` | `InvoiceStatus`: mọi vé `SOLD` → `ACTIVE`; cả `SOLD` và `CANCELLED` → `PARTIALLY_CANCELLED`; mọi vé `CANCELLED` → `CANCELLED`. |
| `getRefundAmount(const string& invoiceId)` | `double` VND; cộng `refundAmount` đã lưu trên các vé `CANCELLED`. |
| `getRemainingAmount(const string& invoiceId)` | `double` VND; tổng gốc của Invoice trừ tổng hoàn, không sửa Invoice. |

GUI đổi ngày cuối được chọn thành 00:00 ngày kế tiếp trước khi gọi bộ lọc. `findInvoiceById()` trả `nullptr` riêng cho mã không tồn tại; thiếu quyền, Repository cần thiết chưa load và dữ liệu hóa đơn/vé bất nhất ném ngoại lệ. Ba hàm trạng thái/tiền cũng ném lỗi khi mã không tồn tại, không dùng `ACTIVE` hay `0` làm mã lỗi. Backend `test_invoice_service.cpp` từng đạt **37 PASS, 0 FAIL**; việc tích hợp GUI chưa được xác nhận.

## 11. ReportService — thống kê Bước 11

Constructor: `ReportService(InvoiceRepository& invoiceRepo, TicketRepository& ticketRepo, MovieRepository& movieRepo, ShowtimeRepository& showtimeRepo, AuthService& authService)`. Mọi API chỉ cho **Admin có `REPORT_VIEW`**; Staff và khách bị từ chối bằng ngoại lệ. GUI gửi hai mốc DateTime đầy đủ theo `[from,to)`: chọn một ngày/tháng thì tự chuyển thành đầu ngày/tháng và đầu kỳ kế tiếp.

| API | Kiểu trả về | Dữ liệu được tính |
| --- | --- | --- |
| `getTotalSales(from, to)` | `double` | Tổng `Invoice::totalAmount` theo `createdAt`; không trừ hoàn. |
| `getTotalRefund(from, to)` | `double` | Tổng `Ticket::refundAmount` của vé `CANCELLED` theo `cancelledAt`. |
| `getActualRevenue(from, to)` | `double` | Tổng bán trừ tổng hoàn; có thể âm khi hoàn vé bán kỳ trước. |
| `getSoldTicketCount(from, to)` | `int` | Số Ticket có Invoice lập trong kỳ, kể cả vé hiện đã hủy; ghế đôi là một Ticket. |
| `getCancelledTicketCount(from, to)` | `int` | Số Ticket hủy trong kỳ theo `cancelledAt`. |
| `getMovieRevenue(movieId, from, to)` | `double` | Tổng `paidAmount` của vé còn `SOLD`, thuộc phim và hóa đơn lập trong kỳ; phim hợp lệ không có vé phù hợp trả `0`. |
| `getTopMovies(from, to, limit)` | `vector<Movie*>` | Chỉ phim có vé `SOLD` trong kỳ bán; lấy tối đa `limit`, doanh thu giảm, hòa thì `movieId` tăng. |

`getTopMovies()` gọi helper `calculateMovieRevenue()` cho từng phim, rồi `std::sort()` bằng hàm so sánh riêng. Vector trả về chứa con trỏ Movie mượn; GUI không sửa hoặc `delete`. Khoảng thời gian sai, `limit <= 0`, hoặc `movieId` không tồn tại ném `invalid_argument`; thiếu quyền, Repository chưa load hoặc dữ liệu liên kết/tiền từng bản ghi sai ném ngoại lệ. Kỳ hợp lệ không có dữ liệu trả `0` hoặc vector rỗng. `getTotalSales()` chỉ cần InvoiceRepository đã load; các hàm dựa trên vé cần Invoice, Ticket và Showtime; báo cáo phim cần thêm MovieRepository.

**Trạng thái kiểm thử hiện tại:** `test_report_service.cpp` chạy lại đạt **31 PASS, 1 FAIL**. Ca tổng `paidAmount` của Ticket không khớp `Invoice::totalAmount` chưa bị `ReportService` từ chối. GUI có thể dựa trên chữ ký API để chuẩn bị tích hợp, nhưng chưa coi kiểm thử backend báo cáo đã hoàn tất; không tự sửa số liệu hoặc ghi TXT để xử lý lỗi này.

## 12. Quy ước xử lý kết quả

| Kết quả | Cách UI xử lý |
|---|---|
| `bool == true` | Thao tác thành công |
| `bool == false` | Thông báo thao tác không thành công |
| Con trỏ khác `nullptr` | Có đối tượng hợp lệ |
| `nullptr` | Không tìm thấy hoặc không có đối tượng |
| `runtime_error` | Bắt ngoại lệ và hiển thị thông báo phù hợp |

Không nên giả định mọi trường hợp `false` đều do dữ liệu nhập sai.

Các hàm thêm nhận `Movie*`, `Room*`, `Seat*`, `Showtime*`, `User*`, `Member*`: Repository sở hữu đối tượng sau khi thêm thành công. Nếu thêm thất bại hoặc ném ngoại lệ trước khi nhận đối tượng, bên gọi phải giải phóng con trỏ. Kiểm tra `nullptr` trước khi truy cập. Các con trỏ trả về từ Service không thuộc quyền sở hữu của UI; UI không được tự `delete` hoặc lưu giữ để sử dụng sau khi dữ liệu bị xóa/nạp lại.

## 13. Quy ước phối hợp

- Hoàng chịu trách nhiệm duy trì Business Logic và cập nhật contract khi API thay đổi.
- Kiệt xây dựng giao diện dựa trên các API đã công bố.
- UI không tự thay đổi chữ ký hàm Service.
- Những chức năng chưa có Service được đánh dấu `PENDING`.
- Mọi thay đổi ảnh hưởng tới UI phải được thông báo trước khi tích hợp.

**Trạng thái:** Draft v0.4 – Chữ ký InvoiceService và ReportService đã có trong code; phần kiểm thử ReportService còn một ca FAIL, tích hợp GUI chưa được xác nhận.
