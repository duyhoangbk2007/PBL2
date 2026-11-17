# `src/business/` — Tầng xử lý nghiệp vụ

## 1. Tổng quan

Thư mục `src/business/` chứa các lớp Service, chịu trách nhiệm kiểm tra quyền truy cập và xử lý các quy tắc nghiệp vụ của hệ thống quản lý rạp chiếu phim.

Service đóng vai trò trung gian giữa giao diện hoặc chương trình điều khiển và tầng Data Access.

Luồng xử lý:

```text
GUI / Main
    |
    v
  Service
    |
    +----> AuthService
    |
    v
 Repository
    |
    v
   Model
    |
    v
 File TXT
```

Service không trực tiếp đọc hoặc ghi file TXT.

## 2. Nguyên tắc thiết kế

Các Service được xây dựng theo những nguyên tắc:

- Sử dụng Repository thông qua tham chiếu.
- Không tự quản lý danh sách đối tượng.
- Không trực tiếp thao tác với file TXT.
- Kiểm tra quyền trước khi thực hiện thao tác quản lý.
- Không lặp lại những kiểm tra dữ liệu đã được Repository xử lý.
- Chỉ bổ sung các quy tắc nghiệp vụ cần thiết.

## 3. Phân quyền

Các Service quản lý Movie, Room, Seat và Showtime sử dụng AuthService.

Quy tắc chung:

| Thao tác | Admin | Staff | Chưa đăng nhập |
|---|---|---|---|
| Thêm | Có | Không | Không |
| Cập nhật | Có | Không | Không |
| Xóa | Có | Không | Không |
| Tìm kiếm | Có | Có | Không |
| Xem danh sách | Có | Có | Không |

Các hàm `load()` và `save()` hiện được sử dụng như thao tác quản lý dữ liệu hệ thống và không kiểm tra quyền Admin.

### Kiểm tra quyền

```cpp
authService.requireAdmin();
```

Yêu cầu quyền Admin.

```cpp
authService.requireLogin();
```

Yêu cầu người dùng đã đăng nhập.

Khi không đáp ứng điều kiện, AuthService ném ngoại lệ.

## 4. Các Service đã triển khai

| Service | Chức năng |
|---|---|
| AuthService | Đăng nhập, trạng thái xác thực và phân quyền |
| MovieService | Quản lý phim |
| RoomService | Quản lý phòng chiếu |
| SeatService | Quản lý ghế ngồi |
| ShowtimeService | Tạo/sửa/tra cứu suất chiếu; hủy/xóa chưa hoàn thiện |

## 5. MovieService

Quản lý các nghiệp vụ liên quan đến phim.

Các chức năng:

- Thêm phim.
- Cập nhật phim.
- Xóa phim.
- Tìm kiếm và truy xuất phim.
- Kiểm tra quyền Admin/Staff.
- Không cho đổi active qua update.
- Không cho thay đổi khoảng ngày phát hành khi đã có Showtime.

Tài liệu chi tiết: `MovieService.md`.

## 6. RoomService

Quản lý các nghiệp vụ liên quan đến phòng chiếu.

Các chức năng:

- Thêm phòng.
- Cập nhật phòng.
- Xóa phòng.
- Tìm kiếm và truy xuất phòng.
- Kiểm tra quyền Admin/Staff.
- Không cho đổi active qua update.
- Không cho thay đổi kích thước phòng khi đã có Showtime.

RoomRepository tiếp tục kiểm tra các ràng buộc về Seat và dữ liệu Room.

Tài liệu chi tiết: `RoomService.md`.

## 7. SeatService

Quản lý các nghiệp vụ liên quan đến ghế ngồi.

Các chức năng:

- Thêm ghế.
- Cập nhật loại ghế.
- Xóa ghế.
- Tìm kiếm và truy xuất ghế.
- Kiểm tra quyền Admin/Staff.

SeatRepository chịu trách nhiệm kiểm tra vị trí, loại ghế và liên kết với Room.

Tài liệu chi tiết: `SeatService.md`.

## 8. ShowtimeService

Service kiểm tra quyền, thời điểm bắt đầu trong tương lai, ngày chiếu nằm trong khoảng phát hành phim và thời lượng suất khớp phim. Repository kiểm tra tham chiếu, trùng lịch cùng phòng và khoảng nghỉ 15 phút. `test_showtime_service.cpp` đạt **89/89 PASS, 0 FAIL** ngày 2026-11-17 cho phạm vi hiện có. `cancelShowtime()` và `removeShowtime()` đang trả `false` để chờ `TicketRepository`; xem [tài liệu ShowtimeService](ShowtimeService.md).

## 9. Phân chia trách nhiệm Service và Repository

Ví dụ với Room:

```text
RoomService
    |
    | Kiểm tra Admin
    | Kiểm tra active
    | Kiểm tra Showtime
    v
RoomRepository
    |
    | Kiểm tra ID
    | Kiểm tra tên
    | Kiểm tra kích thước
    | Kiểm tra Seat
    v
Repository<Room>
```

Ví dụ với Seat:

```text
SeatService
    |
    | Kiểm tra Admin
    v
SeatRepository
    |
    | Kiểm tra dữ liệu Seat
    | Tìm Room
    | Liên kết Room–Seat
    | Kiểm tra vị trí
    v
Repository<Seat>
```

## 10. Quy tắc trả kết quả

Các thao tác quản lý thường trả về `bool`.

- `true`: Thực hiện thành công.
- `false`: Không đáp ứng điều kiện dữ liệu hoặc nghiệp vụ.

Các thao tác tra cứu trả về `const Model*`.

- Con trỏ hợp lệ: Tìm thấy đối tượng.
- `nullptr`: Không tìm thấy.

Các thao tác yêu cầu quyền có thể ném `runtime_error` nếu người dùng không đáp ứng điều kiện xác thực hoặc phân quyền.

## 11. Các nghiệp vụ cần hoàn thiện khi tích hợp

Các module hiện tại mới xử lý những ràng buộc đã có đủ dữ liệu.

Khi tích hợp Showtime và Ticket, cần tiếp tục xem xét:

- Giới hạn chỉnh sửa phòng khi đã có lịch chiếu và vé.
- Không xóa ghế đang được vé tham chiếu.
- Không thay đổi loại ghế nếu ảnh hưởng đến vé đã bán.
- Kiểm tra vé trước khi hủy hoặc xóa suất chiếu.
- Kiểm tra trạng thái ghế khi bán và hủy vé.

Những chức năng này chưa được coi là đã triển khai.

## 12. Kết luận

Tầng Business hiện có các Service quản lý xác thực, phim, phòng chiếu, ghế ngồi và suất chiếu.

Các Service sử dụng cùng một phong cách thiết kế: nhận Repository và AuthService thông qua tham chiếu, kiểm tra quyền, thực hiện nghiệp vụ rồi gọi Repository.

Cách tổ chức này giúp các thành viên triển khai những Service tiếp theo một cách đồng bộ và giảm việc chỉnh sửa lại code khi tích hợp.
