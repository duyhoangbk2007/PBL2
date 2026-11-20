# `src/data_access/` — Tầng truy xuất dữ liệu

## 1. Tổng quan

Thư mục `src/data_access/` chứa các lớp Repository, chịu trách nhiệm quản lý dữ liệu Model trong RAM và thực hiện đọc/ghi dữ liệu từ file TXT.

Trong dự án PBL2, dữ liệu được lưu bằng file TXT thay vì cơ sở dữ liệu SQL.

Repository đóng vai trò trung gian giữa dữ liệu trong file và các đối tượng Model mà chương trình sử dụng.

Luồng xử lý:

```text
File TXT
   |
   v
FileIO
   |
   v
Repository<T>
   |
   v
Repository cụ thể
   |
   v
Model trong RAM
   |
   v
Service
```

## 2. Phân chia trách nhiệm

### Repository chung

`Repository<T>` cung cấp các thao tác dùng chung:

- `load()`
- `save()`
- `add()`
- `removeById()`
- `findById()`
- `getAt()`
- `count()`
- `isLoaded()`
- `clear()`

Repository chung quản lý danh sách đối tượng và bộ nhớ.

### Repository cụ thể

Các Repository cụ thể sử dụng `Repository<T>` thông qua thuộc tính thành viên.

Chúng bổ sung các quy tắc kiểm tra dữ liệu riêng của từng Model.

### Model

Chứa dữ liệu và các phương thức liên kết giữa các đối tượng.

### Service

Kiểm tra quyền và các quy tắc nghiệp vụ trước khi gọi Repository.

## 3. Các Repository đã triển khai

| Repository | Model | Vai trò |
|---|---|---|
| UserRepository | User | Quản lý tài khoản |
| MovieRepository | Movie | Quản lý phim |
| RoomRepository | Room | Quản lý phòng chiếu |
| SeatRepository | Seat | Quản lý ghế và liên kết phòng |
| ShowtimeRepository | Showtime | Quản lý suất chiếu, liên kết phim/phòng và lịch cùng phòng |
| MemberRepository | Member | Quản lý thành viên, tra ID/phone và kiểm tra số điện thoại duy nhất; test riêng 63/63 |
| [PricingRepository](PricingRepository.md) | Pricing | Giữ một cấu hình trong RAM, đọc/ghi/cập nhật năm khóa của `pricing.txt` qua `FileIO`; bộ test Pricing chung 63/63, không dùng `Repository<T>` |

## 4. RoomRepository

Sử dụng `Repository<Room>` để quản lý dữ liệu phòng.

Các chức năng:

- Load/save phòng.
- Thêm, sửa, xóa phòng.
- Kiểm tra ID và dữ liệu Room.
- Không thu nhỏ phòng khiến Seat nằm ngoài kích thước.
- Không xóa phòng còn Seat hoặc Showtime.

Tài liệu chi tiết: `RoomRepository.md`.

## 5. SeatRepository

Sử dụng `Repository<Seat>` và tham chiếu `RoomRepository`.

Các chức năng:

- Load/save ghế.
- Thêm, sửa, xóa ghế.
- Tìm ghế theo khóa ghép `roomId|seatId`.
- Liên kết Seat với Room.
- Không cho trùng vị trí ghế trong cùng phòng.
- Rollback nếu liên kết thất bại.
- Tháo liên kết trước khi xóa Seat.

Tài liệu chi tiết: `SeatRepository.md`.

## 6. Quy tắc quản lý bộ nhớ

Repository chung sở hữu các con trỏ đối tượng được thêm thành công.

Ví dụ:

```cpp
Repository<Room>
Repository<Seat>
```

Khi xóa đối tượng, Repository giải phóng bộ nhớ tương ứng.

Các liên kết giữa Model là con trỏ không sở hữu.

Ví dụ:

- Room lưu con trỏ Seat.
- Seat lưu con trỏ Room.

Vì vậy, cần tháo liên kết trước khi giải phóng đối tượng đang được tham chiếu.

## 7. Thứ tự khởi tạo và load dữ liệu

Với Room và Seat:

```cpp
RoomRepository roomRepo(roomPath);
SeatRepository seatRepo(seatPath, roomRepo);

roomRepo.load();
seatRepo.load();
```

Room phải được load trước Seat.

Lý do: SeatRepository cần tìm Room để thiết lập liên kết khi load.

RoomRepository cũng cần tồn tại lâu hơn SeatRepository.

## 8. Quy tắc xử lý lỗi

Các Repository có thể trả về `false` khi:

- Dữ liệu chưa được load.
- Đối tượng không tồn tại.
- Dữ liệu không hợp lệ.
- Không đáp ứng ràng buộc liên kết.

Khi đọc file sai định dạng, quá trình chuyển đổi có thể ném `runtime_error`.

Repository chung chịu trách nhiệm dọn dẹp dữ liệu tạm khi load thất bại.

SeatRepository bổ sung rollback khi liên kết Room–Seat thất bại.

## 9. Ranh giới với tầng Business

Repository không kiểm tra quyền Admin hoặc Staff.

Ví dụ:

- RoomRepository kiểm tra phòng có còn Seat hay không.
- RoomService kiểm tra người thực hiện có quyền Admin hay không.
- SeatRepository kiểm tra ghế có nằm trong Room hay không.
- SeatService kiểm tra quyền khi thêm, sửa, xóa ghế.

Phân chia này giúp tránh lặp lại logic và giữ trách nhiệm của từng tầng rõ ràng.

## 10. Tài liệu chi tiết

- `MovieRepository.md`
- `RoomRepository.md`
- `SeatRepository.md`
- [`ShowtimeRepository.md`](ShowtimeRepository.md)

Các Repository khác được tham khảo theo tài liệu tương ứng trong thư mục.

## 11. Kết luận

Tầng Data Access hiện sử dụng `Repository<T>` để tái sử dụng các thao tác quản lý dữ liệu chung.

Các Repository cụ thể bổ sung quy tắc riêng của Model và phối hợp với các Model liên quan.

Các ràng buộc phức tạp hơn giữa Ticket, Showtime, Seat và Room sẽ được tiếp tục hoàn thiện trong những bước tích hợp sau.
