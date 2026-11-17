# ShowtimeService – Nghiệp vụ suất chiếu

## Vị trí và phụ thuộc

`ShowtimeService` nằm ở `src/business/ShowtimeService.h/.cpp`. Constructor nhận tham chiếu đến `ShowtimeRepository`, `MovieRepository`, `RoomRepository` và `AuthService`. Repository phim và phòng phải được nạp trước Repository suất chiếu; các đối tượng liên quan cần sống lâu hơn `ShowtimeRepository`. Service không sở hữu các Repository và không tự lưu sau mỗi thao tác.

## API và phân quyền

| Hàm | Hành vi hiện tại |
| --- | --- |
| `load()`, `save()` | Chuyển tiếp đến `showtimeRepo.load()` và `showtimeRepo.save()`; không gọi kiểm tra quyền trong hai hàm này. |
| `addShowtime(Showtime*)` | Gọi `requireAdmin()` trước; từ chối `nullptr`, trạng thái khác `SCHEDULED` hoặc lịch không hợp lệ, rồi gọi Repository để kiểm tra dữ liệu/tham chiếu/trùng lịch và thêm. Repository chỉ sở hữu con trỏ khi thêm thành công. |
| `updateShowtime(const Showtime&)` | Gọi `requireAdmin()` trước; chỉ sửa suất tồn tại, còn `SCHEDULED`, chưa bắt đầu và giá trị mới vẫn `SCHEDULED`; kiểm tra lịch rồi gọi Repository. Repository từ chối đổi phim/phòng. |
| `cancelShowtime(showtimeId)` | Gọi `requireAdmin()`; kiểm tra đã nạp, suất tồn tại, còn `SCHEDULED` và chưa bắt đầu. Hiện luôn trả `false` sau các kiểm tra vì chưa có `TicketRepository` để xác minh vé đã bán. |
| `removeShowtime(showtimeId)` | Gọi `requireAdmin()`; kiểm tra đã nạp và suất tồn tại. Hiện luôn trả `false` vì chưa thể xác minh suất chưa từng có vé. |
| `findShowtimeById()`, `getShowtimeAt()`, `getShowtimeCount()` | Gọi `requireLogin()` trước; tra cứu/đếm qua Repository. Hai hàm tra cứu trả `const Showtime*` mượn. |

`requireAdmin()` và `requireLogin()` có thể ném exception. Staff và người chưa đăng nhập không được thêm/sửa/hủy/xóa; Admin và Staff đã đăng nhập được tra cứu. `load()`/`save()` không được mô tả là thao tác có phân quyền vì code chỉ chuyển tiếp.

## Kiểm tra lịch và ràng buộc

Helper `isValidSchedule()` yêu cầu phim tồn tại, `startAt` đúng định dạng ngày giờ, bắt đầu sau thời điểm hiện tại, ngày chiếu nằm trong khoảng `releaseDate`–`endDate` của phim và `endAt` bằng `startAt` cộng thời lượng phim. Khi tạo, suất phải là `SCHEDULED`. Khi sửa, suất cũ và giá trị mới đều phải là `SCHEDULED`; suất đã bắt đầu hoặc đã hủy bị từ chối.

`ShowtimeRepository` kiểm tra ID, tham chiếu Movie/Room, trạng thái hoạt động, trùng lịch cùng phòng và khoảng nghỉ tối thiểu **15 phút** giữa hai suất `SCHEDULED`. Repository cũng xử lý liên kết hai chiều Movie/Room–Showtime, đọc/ghi `showtimes.txt` và giới hạn thay đổi Movie/Room khi cập nhật. Xem [ShowtimeRepository](<../data_acess(docs)/ShowtimeRepository.md>).

**Chưa hoàn thiện:** `cancelShowtime()` và `removeShowtime()` không thay đổi dữ liệu; cả hai đang trả `false` để chờ `TicketRepository`. Cần kiểm tra vé còn hiệu lực khi hủy và lịch sử vé khi xóa, rồi bổ sung test cho nghiệp vụ đó. Chưa thể nghiệm thu toàn Bước 06 hoặc khẳng định ràng buộc vé đã được bảo vệ.

## Kiểm thử

Ngày 2026-11-17, biên dịch C++17 và chạy `tests/test_showtime_service.cpp` cùng các `.cpp` trong `src/` trừ `main.cpp`: **89 PASS, 0 FAIL**, thoát mã 0. Bài test kiểm tra phân quyền, thêm/sửa, thời gian và thời lượng, trùng lịch/khoảng nghỉ, liên kết, lưu/nạp lại và hành vi hiện tại của hủy/xóa (`false`). Kết quả này không xác nhận nghiệp vụ hủy/xóa sau khi có vé. Xem [Bước 06](../../development/Buoc06_QuanLyPhongVaGhe.md).
