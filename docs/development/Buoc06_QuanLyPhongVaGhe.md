# Nhật ký Bước 06 — Quản lý phòng, ghế và suất chiếu

**Phụ trách Backend:** Hoàng. GUI và tích hợp giao diện thuộc Kiệt từ Bước 12. **Trạng thái:** đang thực hiện; kế hoạch ban đầu lập ngày 2026-11-12. Bước 04 đang tạm dừng; việc chuyển sang Bước 06 không đồng nghĩa Bước 04 đã hoàn thành. Giao diện Qt thuộc Bước 12.

## Hiện trạng và nguồn đối chiếu

- Đã có `src/models/Room.h/.cpp` và `src/models/Seat.h/.cpp`. `Room::linkSeat` kiểm tra `roomId`, vị trí trong kích thước phòng, trùng `seatId` và trùng hàng/cột; `Seat::linkRoom` gọi lại hàm này. `Room` cũng giữ danh sách `Showtime*`.
- Đã có `src/data_access/Repository.h` dùng `DoublyLinkedList`, `FileIO` để nạp/lưu bản ghi và quản lý vòng đời object. `MovieRepository` là ví dụ cách nối Model với Repository; không mặc định sao chép nguyên API hoặc hành vi của phim.
- Nhánh hiện tại đã có `RoomRepository`, `SeatRepository`, `RoomService`, `SeatService` và các file test tương ứng; mô tả chưa có ở mốc lập kế hoạch đã cũ. Lần chạy C++17 ngày 2026-11-17 đã kiểm chứng test Repository/Service phòng và ghế đạt trong phạm vi hiện có. `ShowtimeRepository` và `ShowtimeService` đã có mã; chưa có bằng chứng kiểm thử/nghiệm thu `ShowtimeService`.
- Định dạng chuẩn ở [data_format.md](../TongQuan/data_format.md): `rooms.txt` gồm `roomId|name|rowCount|columnCount|active`; `seats.txt` gồm `roomId|seatId|row|column|seatType`. Không có dòng tiêu đề. `roomId` dạng `ROM0001`; hàng/cột bắt đầu từ 1; `seatType` là `STANDARD`, `VIP` hoặc `COUPLE`.

## Công việc

- [ ] Đối chiếu API `Room`/`Seat`, quy tắc cấp mã, quyền sở hữu object và thứ tự nạp/liên kết. Xác định cách xóa hoặc thay thế phòng/ghế khi còn `Showtime` hoặc vé tham chiếu.
- [x] Đã có mã `RoomRepository` cho nạp/lưu và thao tác dữ liệu phòng; các trường hợp lỗi và nghiệm thu giữ ở mục kiểm thử bên dưới.
- [x] Đã có mã `SeatRepository` cho nạp/lưu và tra cứu theo khóa `(roomId, seatId)`; các ràng buộc vị trí và lỗi dữ liệu tiếp tục kiểm thử. Không dùng riêng `seatId` làm khóa toàn cục.
- [ ] Liên kết ghế với phòng sau khi nạp và kiểm tra tham chiếu trước khi công bố dữ liệu; không để con trỏ treo khi nạp lại hoặc xóa. Xác định cách báo lỗi có file và dòng khi bản ghi sai.
- [ ] Hoàn thiện và nghiệm thu `RoomService` cho nghiệp vụ phòng và sơ đồ ghế: thêm/sửa/tìm phòng, thay đổi loại ghế và trạng thái, quản lý vị trí ghế, kiểm tra quyền Admin khi API xác thực có sẵn. GUI chỉ gọi Service.
- [x] Chạy các test Repository/Service phòng và ghế hiện có bằng C++17: RoomRepository 78/78, RoomService 54/54, SeatRepository 85/85, SeatService 59/59 PASS.
- [ ] Kiểm thử tích hợp sâu với vé, các trường hợp lỗi file/con trỏ và toàn bộ quyền khi các thành phần liên quan sẵn sàng.

## Suất chiếu gộp vào Bước 06

- [x] Triển khai `ShowtimeRepository`: đọc/ghi `showtimes.txt`, kiểm tra dữ liệu và tham chiếu Movie/Room, trùng lịch, liên kết hai chiều và hoàn tác khi nạp lỗi.
- [x] Kiểm thử riêng `ShowtimeRepository`: **89/89 PASS**; Codex đã chạy lại C++17 ngày 2026-11-17. Kết quả không suy ra Service hoặc toàn bước hoàn thành.
- [ ] Hoàn thiện `ShowtimeService` cho quyền và nghiệp vụ tạo/sửa/hủy suất; lớp đã có mã, chưa nghiệm thu.
- [ ] Kiểm thử `ShowtimeService` và tích hợp phòng/ghế/suất với bán vé; `cancelShowtime()` và `removeShowtime()` hiện còn trả `false` để chờ kiểm tra vé.

Khoảng nghỉ giữa hai suất `SCHEDULED` cùng phòng tối thiểu **15 phút**; suất `CANCELLED` không chiếm lịch. Nạp MovieRepository và RoomRepository trước ShowtimeRepository; hủy theo thứ tự ngược lại. Không reload Movie/Room khi Showtime còn liên kết. Xem [tài liệu ShowtimeRepository](<../CauTrucThuMuc/data_acess(docs)/ShowtimeRepository.md>).

## Kết quả kiểm tra trên nhánh hiện tại (2026-11-17)

Codex biên dịch các `.cpp` trong `src/` trừ `src/main.cpp` với C++17, rồi liên kết và chạy từng file test trong thư mục tạm: `test_room_repository.cpp` 78/78, `test_room_service.cpp` 54/54, `test_seat_repository.cpp` 85/85, `test_seat_service.cpp` 59/59, `test_showtime_repository.cpp` 89/89 và `test_showtime_link.cpp` 31/31, đều thoát mã 0. Chưa có `test_showtime_service.cpp`; không đánh dấu hoàn thành Service hoặc toàn Bước 06. Kết quả kiểm tra không thay thế kiểm thử tích hợp với Ticket/Booking.

## Quyết định cần giữ khi triển khai

Ghế `COUPLE` là một `seatId` và một vị trí logic; ô không có bản ghi là ô trống. Không lưu trạng thái đã bán trong `seats.txt`. Phần ghế đã bán theo suất thuộc nghiệp vụ vé ở Bước 10. `Room` có danh sách `Showtime*`, vì vậy thao tác sửa kích thước, xóa hoặc nạp lại cần xét quan hệ với suất chiếu; không coi việc `Room::linkSeat` kiểm tra trùng là đủ để xác thực toàn bộ file.

## Điều kiện hoàn thành

Repository và Service phòng/ghế/suất chiếu hoạt động theo định dạng dữ liệu và quy tắc quan hệ, có kiểm thử C++ chạy đạt và tài liệu ghi kết quả thực tế. Chỉ đánh dấu hoàn thành sau khi kiểm tra các thao tác lưu/nạp lại và trường hợp lỗi quan trọng.
