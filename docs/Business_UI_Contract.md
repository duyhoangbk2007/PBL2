# Business – UI Contract

**Phiên bản:** 0.1 (tạm thời)  
**Nhánh tham chiếu:** `feature/b08-hoang-member-management`  
**Phạm vi:** Các API đã tồn tại tại thời điểm lập contract.

## 1. Nguyên tắc tích hợp

- UI chỉ gọi tầng `Business Service` để thực hiện nghiệp vụ.
- UI không trực tiếp gọi Repository để thêm, sửa, xóa hoặc tra cứu dữ liệu nghiệp vụ.
- UI không tự đọc/ghi các file TXT.
- Các Service và Repository được khởi tạo, nạp dữ liệu trước khi UI thực hiện nghiệp vụ.
- UI sử dụng các đối tượng Service được truyền vào, không tự tạo lại Repository hoặc AuthService cho từng màn hình.
- Khi backend thay đổi, hai bên cập nhật contract trước khi tích hợp thay đổi.

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
| `hasPermission(permission)` | `bool` |
| `requireLogin()` | `const User&` |
| `requirePermission(permission)` | `const User&` |

UI sử dụng `hasPermission()` để quyết định hiển thị hoặc vô hiệu hóa các chức năng theo quyền.

Backend vẫn phải kiểm tra quyền bằng `requirePermission()` khi thực hiện nghiệp vụ. Việc ẩn nút trên UI không thay thế kiểm tra quyền.

`login()` trả về `false` khi thông tin đăng nhập không hợp lệ và có thể ném ngoại lệ khi dữ liệu tài khoản chưa được nạp.

## 3. MovieService – Quản lý phim

Constructor:

```cpp
MovieService(MovieRepository& movieRepo,
             AuthService& authService);
```

| API | Kiểu trả về | Permission |
|---|---|---|
| `addMovie(Movie*)` | `bool` | `MOVIE_ADD` |
| `updateMovie(const Movie&)` | `bool` | `MOVIE_UPDATE` |
| `removeMovie(movieId)` | `bool` | `MOVIE_DELETE` |
| `findMovieById(movieId)` | `const Movie*` | `MOVIE_VIEW` |
| `getMovieAt(index)` | `const Movie*` | `MOVIE_VIEW` |
| `getMovieCount()` | `int` | `MOVIE_VIEW` |

Thông tin phim để UI hiển thị: `movieId`, `title`, `genre`, `durationMinutes`, `minAge`, `releaseDate`, `endDate`, `active`.

## 4. RoomService – Quản lý phòng

Constructor:

```cpp
RoomService(RoomRepository& roomRepo,
            AuthService& authService);
```

| API | Kiểu trả về | Permission |
|---|---|---|
| `addRoom(Room*)` | `bool` | `ROOM_ADD` |
| `updateRoom(const Room&)` | `bool` | `ROOM_UPDATE` |
| `removeRoom(roomId)` | `bool` | `ROOM_DELETE` |
| `findRoomById(roomId)` | `const Room*` | `ROOM_VIEW` |
| `getRoomAt(index)` | `const Room*` | `ROOM_VIEW` |
| `getRoomCount()` | `int` | `ROOM_VIEW` |

Thông tin phòng: `roomId`, `name`, `rowCount`, `columnCount`, `active`.

## 5. SeatService – Quản lý ghế

Constructor:

```cpp
SeatService(SeatRepository& seatRepo,
            AuthService& authService);
```

| API | Kiểu trả về | Permission |
|---|---|---|
| `addSeat(Seat*)` | `bool` | `SEAT_ADD` |
| `updateSeat(const Seat&)` | `bool` | `SEAT_UPDATE` |
| `removeSeat(roomId, seatId)` | `bool` | `SEAT_DELETE` |
| `findSeatById(roomId, seatId)` | `const Seat*` | `SEAT_VIEW` |
| `getSeatAt(index)` | `const Seat*` | `SEAT_VIEW` |
| `getSeatCount()` | `int` | `SEAT_VIEW` |

Thông tin ghế: `roomId`, `seatId`, `row`, `column`, `seatType`.

`SeatType` có ba giá trị: `STANDARD`, `VIP`, `COUPLE`.

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
| `addShowtime(Showtime*)` | `bool` | `SHOWTIME_ADD` |
| `updateShowtime(const Showtime&)` | `bool` | `SHOWTIME_UPDATE` |
| `cancelShowtime(showtimeId)` | `bool` | `SHOWTIME_CANCEL` |
| `removeShowtime(showtimeId)` | `bool` | `SHOWTIME_DELETE` |
| `findShowtimeById(showtimeId)` | `const Showtime*` | `SHOWTIME_VIEW` |
| `getShowtimeAt(index)` | `const Showtime*` | `SHOWTIME_VIEW` |
| `getShowtimeCount()` | `int` | `SHOWTIME_VIEW` |

Thông tin suất chiếu: `showtimeId`, `movieId`, `roomId`, `startAt`, `endAt`, `status`.

`ShowtimeStatus`: `SCHEDULED`, `CANCELLED`.

**Chưa hoàn thiện:** `cancelShowtime()` và `removeShowtime()` hiện chưa thực hiện thành công việc hủy/xóa vì còn chờ kiểm tra dữ liệu vé. UI chưa được coi hai chức năng này là đã sẵn sàng.

## 7. StaffService – Quản lý nhân viên

Constructor:

```cpp
StaffService(UserRepository& staffRepo,
             AuthService& authService);
```

| API | Kiểu trả về | Permission |
|---|---|---|
| `addStaff(User*)` | `bool` | `STAFF_ADD` |
| `updateStaff(const User&)` | `bool` | `STAFF_UPDATE` |
| `deactivateStaff(staffId)` | `bool` | `STAFF_DEACTIVATE` |
| `findStaffById(staffId)` | `const User*` | `STAFF_VIEW` |
| `findStaffByUsername(username)` | `const User*` | `STAFF_VIEW` |
| `getStaffAt(index)` | `const User*` | `STAFF_VIEW` |
| `getStaffCount()` | `int` | `STAFF_VIEW` |

Thông tin nhân viên: `userId`, `username`, `fullName`, `role`, `active`.

Model `User` còn có trường `password`; UI cần hạn chế hiển thị thông tin này.

## 8. MemberService – Quản lý thành viên

Constructor:

```cpp
MemberService(MemberRepository& memberRepo,
              AuthService& authService);
```

| API | Kiểu trả về | Permission |
| --- | --- | --- |
| `load()` | `bool` | Nội bộ; không kiểm tra quyền |
| `save()` | `bool` | Nội bộ; không kiểm tra quyền |
| `addMember(Member* member)` | `bool` | `MEMBER_ADD` |
| `updateMember(const Member& newValues)` | `bool` | `MEMBER_UPDATE` |
| `findMemberById(const string& memberId)` | `const Member*` | `MEMBER_VIEW` |
| `findMemberByPhone(const string& phone)` | `const Member*` | `MEMBER_VIEW` |
| `getMemberAt(int index)` | `const Member*` | `MEMBER_VIEW` |
| `getMemberCount()` | `int` | `MEMBER_VIEW` |

Model `Member` có `memberId`, `fullName`, `phone`, `points`. UI dùng `getMemberCount()`/`getMemberAt()` để duyệt, hai hàm `find` để tra cứu, `addMember()` để thêm và `updateMember()` để sửa họ tên/số điện thoại. Khi tạo `newValues` để sửa, phải giữ nguyên `memberId` và `points` hiện có: Repository từ chối thay đổi điểm qua API này. Thành viên mới phải có `points = 0`; số điện thoại phải duy nhất, gồm 10 chữ số và bắt đầu bằng `0`.

`requirePermission()` có thể ném `runtime_error` khi chưa đăng nhập hoặc thiếu quyền; bảng quyền đang nạp quyết định quyền thực tế, kể cả với Admin. `addMember()` chuyển quyền sở hữu `Member*` cho Repository **chỉ khi trả `true`**; nếu trả `false` hoặc ném ngoại lệ trước khi thêm, bên gọi vẫn phải giải phóng con trỏ. Các con trỏ `const Member*` trả về chỉ để đọc và không thuộc sở hữu UI. `load()`/`save()` chỉ dành cho luồng quản lý dữ liệu, không tự được gọi sau từng thao tác UI; thêm/sửa chỉ thay đổi dữ liệu trong bộ nhớ đến khi lưu.

**Chưa có:** API xóa thành viên, chỉnh điểm trực tiếp hoặc xem lịch sử hóa đơn qua `MemberService`. Các nghiệp vụ điểm và liên kết hóa đơn/vé chưa được kiểm chứng. [Tài liệu MemberService](<CauTrucThuMuc/business(docs)/MemberService.md>) ghi chi tiết; test riêng đạt **69 PASS, 0 FAIL** trong phạm vi hiện có.

## 9. Quy ước xử lý kết quả

| Kết quả | Cách UI xử lý |
|---|---|
| `bool == true` | Thao tác thành công |
| `bool == false` | Thông báo thao tác không thành công |
| Con trỏ khác `nullptr` | Có đối tượng hợp lệ |
| `nullptr` | Không tìm thấy hoặc không có đối tượng |
| `runtime_error` | Bắt ngoại lệ và hiển thị thông báo phù hợp |

Không nên giả định mọi trường hợp `false` đều do dữ liệu nhập sai.

Các con trỏ trả về từ Service không thuộc quyền sở hữu của UI; UI không được tự `delete` hoặc lưu giữ để sử dụng sau khi dữ liệu bị xóa/nạp lại.

## 10. Quy ước phối hợp

- Hoàng chịu trách nhiệm duy trì Business Logic và cập nhật contract khi API thay đổi.
- Kiệt xây dựng giao diện dựa trên các API đã công bố.
- UI không tự thay đổi chữ ký hàm Service.
- Những chức năng chưa có Service được đánh dấu `PENDING`.
- Mọi thay đổi ảnh hưởng tới UI phải được thông báo trước khi tích hợp.

**Trạng thái:** Draft v0.1 – Có thể dùng để bắt đầu phát triển giao diện, chưa phải API ổn định cuối cùng.
