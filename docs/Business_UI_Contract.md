# Business – UI Contract

**Phiên bản:** 0.2 (Draft)
**Nhánh tham chiếu:** `main`
**Phạm vi:** API đối chiếu trực tiếp từ các file `.h`/`.cpp` trong `src/business` trên `main`. Các mục PENDING chưa phải API có thể gọi trên nhánh này.

## 1. Nguyên tắc tích hợp

- UI chỉ gọi tầng `Business Service` để thực hiện nghiệp vụ.
- UI không trực tiếp gọi Repository để thêm, sửa, xóa hoặc tra cứu dữ liệu nghiệp vụ.
- UI không tự đọc/ghi các file TXT.
- Các Service và Repository được khởi tạo, nạp dữ liệu trước khi UI thực hiện nghiệp vụ.
- UI sử dụng các đối tượng Service được truyền vào, không tự tạo lại Repository hoặc AuthService cho từng màn hình.
- Khi backend thay đổi, hai bên cập nhật contract trước khi tích hợp thay đổi.
- Trên `main`, các thao tác ghi yêu cầu Admin, thao tác đọc yêu cầu đăng nhập (Admin hoặc Staff). `load()`/`save()` không kiểm tra quyền; ứng dụng gọi trong luồng khởi tạo/lưu, không gọi sau từng thao tác UI.
- Nạp `UserRepository` trước khi đăng nhập; nạp Movie và Room trước Showtime, Room trước Seat. Các Repository và Service được truyền bằng tham chiếu phải sống lâu hơn Service/UI đang sử dụng.

## 2. AuthService – Đăng nhập và phân quyền

Constructor:

```cpp
AuthService(UserRepository& userRepo);
```

| API | Kết quả |
|---|---|
| `login(username, password)` | `bool` |
| `logout()` | `void` |
| `getCurrentUser()` | `const User*` |
| `isLoggedIn()` | `bool` |
| `isAdmin()` / `isStaff()` | `bool` |
| `requireLogin()` | `const User&` |
| `requireAdmin()` | `const User&` |

UI dùng `isLoggedIn()`, `isAdmin()` và `isStaff()` để quyết định hiển thị hoặc vô hiệu hóa chức năng. `main` chưa có permission theo mã như `MOVIE_ADD` hoặc `ROOM_VIEW`.

Backend kiểm tra bằng `requireAdmin()` cho thao tác ghi và `requireLogin()` cho tra cứu; cả hai có thể ném `runtime_error`. Việc ẩn nút trên UI không thay thế kiểm tra quyền.

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
| `addMovie(Movie*)` | `bool` | Admin |
| `updateMovie(const Movie&)` | `bool` | Admin |
| `removeMovie(const string& movieId)` | `bool` | Admin |
| `findMovieById(const string& movieId)` | `const Movie*` | Đăng nhập |
| `getMovieAt(int index)` | `const Movie*` | Đăng nhập |
| `getMovieCount()` | `int` | Đăng nhập |

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
| `addRoom(Room*)` | `bool` | Admin |
| `updateRoom(const Room&)` | `bool` | Admin |
| `removeRoom(const string& roomId)` | `bool` | Admin |
| `findRoomById(const string& roomId)` | `const Room*` | Đăng nhập |
| `getRoomAt(int index)` | `const Room*` | Đăng nhập |
| `getRoomCount()` | `int` | Đăng nhập |

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
| `addSeat(Seat*)` | `bool` | Admin |
| `updateSeat(const Seat&)` | `bool` | Admin |
| `removeSeat(const string& roomId, const string& seatId)` | `bool` | Admin |
| `findSeatById(const string& roomId, const string& seatId)` | `const Seat*` | Đăng nhập |
| `getSeatAt(int index)` | `const Seat*` | Đăng nhập |
| `getSeatCount()` | `int` | Đăng nhập |

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
| `addShowtime(Showtime*)` | `bool` | Admin |
| `updateShowtime(const Showtime&)` | `bool` | Admin |
| `cancelShowtime(const string& showtimeId)` | `bool` | Admin |
| `removeShowtime(const string& showtimeId)` | `bool` | Admin |
| `findShowtimeById(const string& showtimeId)` | `const Showtime*` | Đăng nhập |
| `getShowtimeAt(int index)` | `const Showtime*` | Đăng nhập |
| `getShowtimeCount()` | `int` | Đăng nhập |

Thông tin suất chiếu: `showtimeId`, `movieId`, `roomId`, `startAt`, `endAt`, `status`.

`ShowtimeStatus`: `SCHEDULED`, `CANCELLED`.

**Chưa hoàn thiện:** `cancelShowtime()` và `removeShowtime()` hiện chưa thực hiện thành công việc hủy/xóa vì còn chờ kiểm tra dữ liệu vé. UI chưa được coi hai chức năng này là đã sẵn sàng.
Hai hàm có chữ ký nhưng sau kiểm tra quyền/điều kiện đều trả `false`. Thêm suất chỉ nhận `SCHEDULED`, giờ bắt đầu trong tương lai, ngày thuộc khoảng phát hành phim, giờ kết thúc khớp thời lượng; sửa chỉ nhận suất `SCHEDULED` chưa bắt đầu. Repository chặn đổi phim/phòng và trùng lịch.

## 7. StaffService – Quản lý nhân viên

**PENDING trên `main`:** Chưa có `StaffService.h/.cpp`, nên chưa có constructor, API hoặc hợp đồng phân quyền để UI gọi. Module trên nhánh tính năng chưa được coi là API trên `main`. Model `User` có trường `password`; UI không hiển thị mật khẩu.

## 8. MemberService – Quản lý thành viên

**PENDING trên `main`:** Chưa có `MemberService.h/.cpp`, nên chưa có constructor, API hoặc hợp đồng phân quyền để UI gọi. Module trên `feature/b08-hoang-member-management` chưa được coi là API trên `main`. Chưa có hợp đồng Business Service trên `main` cho chỉnh điểm, lịch sử hóa đơn hoặc liên kết vé.

## 9. Các nghiệp vụ tiếp theo

- **PENDING:** `PricingService` chỉ được triển khai trên `feature/b09-hoang-pricing-management`; không thuộc danh sách API đã có trên `main`.
- **PENDING:** Chưa có Business Service trên `main` cho bán/hủy vé, hóa đơn, áp dụng giá/ưu đãi và tích điểm. UI không dựa vào API giả định cho các chức năng này.

## 10. Quy ước xử lý kết quả

| Kết quả | Cách UI xử lý |
|---|---|
| `bool == true` | Thao tác thành công |
| `bool == false` | Thông báo thao tác không thành công |
| Con trỏ khác `nullptr` | Có đối tượng hợp lệ |
| `nullptr` | Không tìm thấy hoặc không có đối tượng |
| `runtime_error` | Bắt ngoại lệ và hiển thị thông báo phù hợp |

Không nên giả định mọi trường hợp `false` đều do dữ liệu nhập sai.

Các hàm thêm nhận `Movie*`, `Room*`, `Seat*`, `Showtime*`: Repository sở hữu đối tượng sau khi thêm thành công. Nếu thêm thất bại hoặc ném ngoại lệ trước khi nhận đối tượng, bên gọi phải giải phóng con trỏ. Kiểm tra `nullptr` trước khi truy cập. Các con trỏ trả về từ Service không thuộc quyền sở hữu của UI; UI không được tự `delete` hoặc lưu giữ để sử dụng sau khi dữ liệu bị xóa/nạp lại.

## 11. Quy ước phối hợp

- Hoàng chịu trách nhiệm duy trì Business Logic và cập nhật contract khi API thay đổi.
- Kiệt xây dựng giao diện dựa trên các API đã công bố.
- UI không tự thay đổi chữ ký hàm Service.
- Những chức năng chưa có Service được đánh dấu `PENDING`.
- Mọi thay đổi ảnh hưởng tới UI phải được thông báo trước khi tích hợp.

**Trạng thái:** Draft v0.2 – Có thể dùng để bắt đầu phát triển giao diện, chưa phải API ổn định cuối cùng.
