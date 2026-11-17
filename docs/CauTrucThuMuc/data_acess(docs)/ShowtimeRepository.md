# ShowtimeRepository – Quản lý dữ liệu suất chiếu

## Tổng quan

`ShowtimeRepository` nằm ở `src/data_access/ShowtimeRepository.h/.cpp`. Lớp chứa `Repository<Showtime>` để quản lý đối tượng và đọc/ghi `showtimes.txt` qua `FileIO`; đồng thời giữ tham chiếu đến `MovieRepository` và `RoomRepository` để kiểm tra, nối quan hệ. Định dạng mỗi dòng là `showtimeId|movieId|roomId|startAt|endAt|status`, không có schema hoặc tiêu đề.

## API hiện có

| Hàm | Vai trò |
| --- | --- |
| `ShowtimeRepository(filePath, movieRepo, roomRepo)` | Nhận đường dẫn file và hai Repository tồn tại lâu hơn nó. |
| `load()` | Chỉ nạp một lần sau khi Movie/Room đã nạp; kiểm tra tham chiếu, trùng lịch rồi nối hai chiều. Trả `false` nếu chưa đủ điều kiện hoặc không đọc được file; dữ liệu lỗi có thể ném exception. |
| `save()` | Ghi danh sách hiện tại qua `Repository<Showtime>`; không tự lưu sau từng thay đổi. |
| `add(Showtime*)` | Kiểm tra dữ liệu, ID trùng, tham chiếu và lịch; nối Movie/Room rồi thêm vào danh sách. Chỉ chuyển quyền sở hữu con trỏ khi thêm thành công. |
| `update(const Showtime&)` | Sửa thời gian và trạng thái của suất có ID tương ứng; không cho đổi Movie hoặc Room. |
| `removeById(showtimeId)` | Gỡ hai liên kết rồi xóa đối tượng khỏi Repository. |
| `findById()`, `getAt()`, `count()`, `isLoaded()` | Tra cứu hoặc kiểm tra trạng thái danh sách trong RAM. Con trỏ trả về là con trỏ mượn. |

Các helper private gồm `getShowtimeId`, `stringToShowtime`, `showtimeToString`, `isValidShowtime`, `isValidReference` và `hasScheduleConflict`. Parse yêu cầu đúng sáu trường, status chỉ `SCHEDULED` hoặc `CANCELLED`; `Validator` kiểm tra tiền tố ID và ngày giờ, `DateTime::isBefore` yêu cầu `endAt > startAt`. Khi thêm hoặc sửa suất `SCHEDULED`, phim và phòng phải tồn tại, đã nạp và đang hoạt động. Suất `CANCELLED` vẫn cần tham chiếu tồn tại.

## Lịch và liên kết

`hasScheduleConflict()` so các suất `SCHEDULED` cùng phòng, bỏ qua suất đang sửa và suất đã hủy. `DateTime::isOverlapping()` cộng `DateTime::BREAK_MINUTES = 15` vào thời điểm kết thúc: **hai suất cùng phòng cần nghỉ tối thiểu 15 phút**; đúng 15 phút được chấp nhận. Khi `load()`, code cũng kiểm tra trùng lịch giữa các bản ghi trước khi nối quan hệ.

`Showtime::linkMovie()`/`linkRoom()` thiết lập quan hệ hai chiều với `Movie`/`Room`. Nếu nối hoặc thêm thất bại, `add()` gỡ các liên kết vừa tạo. Nếu bước kiểm tra hoặc nối trong `load()` ném lỗi, lớp gỡ các liên kết đã tạo, `repo.clear()` xóa các Showtime vừa nạp rồi ném lại lỗi. Khi xóa, nếu gỡ phía Room thất bại sau khi gỡ phía Movie, lớp cố nối lại Movie. Destructor gỡ liên kết Movie/Room trước khi `Repository<Showtime>` hủy các đối tượng.

`Repository<Showtime>` sở hữu `Showtime*` sau khi `add()` hoặc `load()` thành công; caller giữ trách nhiệm giải phóng con trỏ nếu `add()` thất bại. Con trỏ Movie/Room trong Showtime và danh sách Showtime ở Movie/Room là liên kết **không sở hữu**. Vì vậy, nạp `MovieRepository` và `RoomRepository` trước `ShowtimeRepository`; hủy `ShowtimeRepository` trước Movie/Room. Không reload Movie/Room khi Showtime còn liên kết để tránh con trỏ treo.

## Ranh giới và kiểm thử

Repository kiểm tra định dạng, tham chiếu và trùng lịch. [ShowtimeService](<../business(docs)/ShowtimeService.md>) kiểm tra quyền Admin và điều kiện lịch khi thêm/sửa; test riêng đạt 89/89 trong phạm vi hiện có. `cancelShowtime()` và `removeShowtime()` vẫn trả `false` vì chưa có `TicketRepository`, nên chưa bảo đảm đầy đủ điều kiện tham chiếu vé. Không coi các quy tắc ở Service là bảo đảm của Repository. `save()` ghi một file; không cung cấp giao dịch nhiều file.

`tests/test_showtime_repository.cpp` kiểm tra thêm/sửa/xóa, đọc/ghi, khoảng nghỉ 14/15 phút, liên kết, dữ liệu lỗi và rollback. **89/89 PASS** khi Codex biên dịch C++17 và chạy lại test trong thư mục tạm ngày 2026-11-17. `ShowtimeService` cũng có test riêng 89/89; xem [Bước 06](../../development/Buoc06_QuanLyPhongVaGhe.md) để biết phần còn thiếu.
