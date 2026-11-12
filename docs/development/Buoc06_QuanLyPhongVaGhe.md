# Nhật ký Bước 06 — Quản lý phòng và ghế

**Phụ trách:** Hoàng. **Trạng thái:** chuẩn bị triển khai (2026-11-12). Bước 04 đang tạm dừng; việc chuyển sang Bước 06 không đồng nghĩa Bước 04 đã hoàn thành. Giao diện Qt thuộc Bước 13.

## Hiện trạng và nguồn đối chiếu

- Đã có `src/models/Room.h/.cpp` và `src/models/Seat.h/.cpp`. `Room::linkSeat` kiểm tra `roomId`, vị trí trong kích thước phòng, trùng `seatId` và trùng hàng/cột; `Seat::linkRoom` gọi lại hàm này. `Room` cũng giữ danh sách `Showtime*`.
- Đã có `src/data_access/Repository.h` dùng `DoublyLinkedList`, `FileIO` để nạp/lưu bản ghi và quản lý vòng đời object. `MovieRepository` là ví dụ cách nối Model với Repository; không mặc định sao chép nguyên API hoặc hành vi của phim.
- Chưa có `RoomRepository`, `SeatRepository`, `RoomService` hoặc kiểm thử chức năng phòng/ghế. Chưa có bằng chứng build hay chạy test Bước 06.
- Định dạng chuẩn ở [data_format.md](../TongQuan/data_format.md): `rooms.txt` gồm `roomId|name|rowCount|columnCount|active`; `seats.txt` gồm `roomId|seatId|row|column|seatType`. Không có dòng tiêu đề. `roomId` dạng `ROM0001`; hàng/cột bắt đầu từ 1; `seatType` là `STANDARD`, `VIP` hoặc `COUPLE`.

## Công việc

- [ ] Đối chiếu API `Room`/`Seat`, quy tắc cấp mã, quyền sở hữu object và thứ tự nạp/liên kết. Xác định cách xóa hoặc thay thế phòng/ghế khi còn `Showtime` hoặc vé tham chiếu.
- [ ] Cài `RoomRepository`: nạp/lưu `rooms.txt`, thêm/sửa/tra cứu/xóa khi hợp lệ; kiểm tra mã, kích thước, tên, trạng thái, trùng ID, dữ liệu lỗi và lỗi đọc/ghi.
- [ ] Cài `SeatRepository`: nạp/lưu `seats.txt`, tra cứu theo khóa `(roomId, seatId)`, kiểm tra loại ghế, vị trí, phòng tồn tại, vị trí trong kích thước phòng, trùng ID và trùng vị trí trong cùng phòng. Không dùng riêng `seatId` làm khóa toàn cục.
- [ ] Liên kết ghế với phòng sau khi nạp và kiểm tra tham chiếu trước khi công bố dữ liệu; không để con trỏ treo khi nạp lại hoặc xóa. Xác định cách báo lỗi có file và dòng khi bản ghi sai.
- [ ] Cài `RoomService` cho nghiệp vụ phòng và sơ đồ ghế: thêm/sửa/tìm phòng, thay đổi loại ghế và trạng thái, quản lý vị trí ghế, kiểm tra quyền Admin khi API xác thực có sẵn. GUI chỉ gọi Service.
- [ ] Kiểm thử C++: dữ liệu hợp lệ/sai, khóa và vị trí trùng, ghế ngoài sơ đồ, phòng thiếu, loại ghế sai, nạp/lưu lại, lỗi ghi, quyền và tham chiếu. Ghi lệnh, kết quả thực chạy và giới hạn kiểm thử vào nhật ký này.

## Quyết định cần giữ khi triển khai

Ghế `COUPLE` là một `seatId` và một vị trí logic; ô không có bản ghi là ô trống. Không lưu trạng thái đã bán trong `seats.txt`. Phần ghế đã bán theo suất thuộc nghiệp vụ vé ở Bước 11. `Room` có danh sách `Showtime*`, vì vậy thao tác sửa kích thước, xóa hoặc nạp lại cần xét quan hệ với suất chiếu; không coi việc `Room::linkSeat` kiểm tra trùng là đủ để xác thực toàn bộ file.

## Điều kiện hoàn thành

Repository và Service phòng/ghế hoạt động theo định dạng dữ liệu và quy tắc quan hệ, có kiểm thử C++ chạy đạt và tài liệu ghi kết quả thực tế. Chỉ đánh dấu hoàn thành sau khi kiểm tra các thao tác lưu/nạp lại và trường hợp lỗi quan trọng.
