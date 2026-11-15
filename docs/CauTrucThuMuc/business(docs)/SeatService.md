# SeatService – Nghiệp vụ quản lý ghế ngồi

## 1. Tổng quan

`SeatService` là lớp thuộc tầng Business Logic, chịu trách nhiệm xử lý các nghiệp vụ quản lý ghế ngồi và kiểm tra quyền truy cập.

Lớp sử dụng:

- `SeatRepository` để thêm, sửa, xóa và truy xuất dữ liệu ghế.
- `AuthService` để kiểm tra đăng nhập và phân quyền.

SeatService không trực tiếp đọc hoặc ghi file TXT.

**Quy tắc phân quyền:**

- **Admin:** Được thêm, sửa, xóa và tra cứu ghế.
- **Staff:** Chỉ được tra cứu ghế.
- **Chưa đăng nhập:** Không được quản lý hoặc tra cứu ghế.

`load()` và `save()` là các thao tác quản lý dữ liệu của hệ thống, không yêu cầu quyền Admin.

**File liên quan:**

- `src/business/SeatService.h`
- `src/business/SeatService.cpp`
- `src/business/AuthService.h`
- `src/data_access/SeatRepository.h`

---

## 2. Thuộc tính

```cpp
private:
    SeatRepository& seatRepo;
    AuthService& authService;
```

### `SeatRepository& seatRepo`

Tham chiếu đến Repository quản lý dữ liệu ghế.

### `AuthService& authService`

Tham chiếu đến đối tượng quản lý đăng nhập và phân quyền.

SeatService không sở hữu hai đối tượng này.

Chúng phải tồn tại lâu hơn SeatService.

---

## 3. Constructor

```cpp
SeatService::SeatService(
    SeatRepository& seatRepo,
    AuthService& authService
)
    : seatRepo(seatRepo),
      authService(authService)
{}
```

### Chức năng

Nhận các đối tượng SeatRepository và AuthService đã được tạo.

Không tự tạo Repository, không đọc file và không thực hiện đăng nhập.

---

## 4. Hàm load()

```cpp
bool SeatService::load()
{
    return seatRepo.load();
}
```

### Chức năng

Nạp dữ liệu Seat từ file TXT thông qua SeatRepository.

SeatRepository chịu trách nhiệm:

- Đọc dữ liệu.
- Chuyển đổi thành đối tượng Seat.
- Tìm Room tương ứng.
- Liên kết Seat với Room.
- Rollback nếu liên kết thất bại.

RoomRepository phải được load trước khi load SeatRepository.

SeatService không tự thực hiện các thao tác này.

---

## 5. Hàm save()

```cpp
bool SeatService::save()
{
    return seatRepo.save();
}
```

### Chức năng

Lưu dữ liệu ghế hiện tại xuống file TXT.

Việc chuyển đổi và ghi dữ liệu do Repository xử lý.

---

## 6. Hàm addSeat()

```cpp
bool SeatService::addSeat(Seat* seat)
{
    authService.requireAdmin();
    return seatRepo.add(seat);
}
```

### Chức năng

Thêm ghế mới vào hệ thống.

### Bước 1: Kiểm tra quyền Admin

```cpp
authService.requireAdmin();
```

Nếu không có quyền Admin, hàm ném ngoại lệ.

### Bước 2: Gọi SeatRepository

```cpp
return seatRepo.add(seat);
```

Repository kiểm tra:

- Dữ liệu Seat hợp lệ.
- Room tồn tại.
- Seat chưa được thêm trước đó.
- Không trùng khóa ghép.
- Không trùng vị trí.
- Ghế nằm trong phạm vi Room.
- Liên kết Room–Seat thành công.

### Kết quả

- `true`: Thêm thành công.
- `false`: Repository từ chối.
- Ném `runtime_error`: Không có quyền Admin.

Khi thêm thành công, Repository quản lý vòng đời đối tượng Seat.

---

## 7. Hàm updateSeat()

```cpp
bool SeatService::updateSeat(const Seat& newValues)
```

### Chức năng

Cập nhật thông tin của một Seat đã tồn tại.

Trong phiên bản hiện tại, thao tác cập nhật chỉ cho phép thay đổi loại ghế.

### Bước 1: Kiểm tra quyền Admin

```cpp
authService.requireAdmin();
```

Chỉ Admin được phép cập nhật ghế.

### Bước 2: Kiểm tra Repository

```cpp
if (!seatRepo.isLoaded())
{
    return false;
}
```

Không cho phép cập nhật khi dữ liệu chưa load.

### Bước 3: Tìm ghế hiện tại

```cpp
Seat* seat = seatRepo.findById(
    newValues.getRoomId(),
    newValues.getSeatId()
);

if (seat == nullptr)
{
    return false;
}
```

Ghế được tìm bằng khóa ghép `roomId|seatId`.

### Bước 4: Gọi Repository cập nhật

```cpp
return seatRepo.update(newValues);
```

SeatRepository tiếp tục kiểm tra:

- Dữ liệu Seat hợp lệ.
- Không thay đổi hàng.
- Không thay đổi cột.
- Chỉ cập nhật loại ghế.

### Kết quả

- `true`: Cập nhật thành công.
- `false`: Ghế không tồn tại hoặc dữ liệu không hợp lệ.
- Ném `runtime_error`: Không có quyền Admin.

### Vì sao Service không tự kiểm tra vị trí ghế?

Vì `SeatRepository::update()` đã thực hiện việc kiểm tra này.

SeatService không cần lặp lại toàn bộ kiểm tra dữ liệu của Repository.

Tầng Business chỉ bổ sung những quy tắc nghiệp vụ riêng khi cần thiết.

---

## 8. Hàm removeSeat()

```cpp
bool SeatService::removeSeat(
    const string& roomId,
    const string& seatId
)
{
    authService.requireAdmin();
    return seatRepo.removeById(roomId, seatId);
}
```

### Chức năng

Xóa một ghế theo Room ID và Seat ID.

### Quy trình

1. Kiểm tra quyền Admin.
2. Gọi SeatRepository tìm Seat.
3. Repository xác định Room đang liên kết.
4. Tháo liên kết Seat khỏi Room.
5. Xóa Seat khỏi Repository.
6. Giải phóng bộ nhớ đối tượng Seat.

### Kết quả

- `true`: Xóa thành công.
- `false`: Không tìm thấy Seat hoặc không thể xóa.
- Ném `runtime_error`: Không có quyền Admin.

**Lưu ý:** Phiên bản hiện tại chưa kiểm tra Seat có được Ticket tham chiếu hay không. Quy tắc này cần được bổ sung khi tích hợp Ticket.

---

## 9. Hàm findSeatById()

```cpp
const Seat* SeatService::findSeatById(
    const string& roomId,
    const string& seatId
)
{
    authService.requireLogin();
    return seatRepo.findById(roomId, seatId);
}
```

### Chức năng

Tìm ghế theo khóa ghép.

Yêu cầu người dùng đã đăng nhập.

Admin và Staff đều có quyền tra cứu.

### Kết quả

- Trả về `const Seat*` nếu tìm thấy.
- Trả về `nullptr` nếu không tìm thấy.
- Ném ngoại lệ nếu chưa đăng nhập.

### Vì sao sử dụng const Seat*?

Để bên gọi không thể sửa trực tiếp dữ liệu Seat thông qua con trỏ trả về.

Mọi thay đổi cần đi qua hàm nghiệp vụ có kiểm tra quyền.

---

## 10. Hàm getSeatAt()

```cpp
const Seat* SeatService::getSeatAt(int index)
{
    authService.requireLogin();
    return seatRepo.getAt(index);
}
```

### Chức năng

Lấy Seat theo vị trí trong danh sách SeatRepository.

Trả về `nullptr` nếu vị trí không hợp lệ.

Hàm yêu cầu đăng nhập.

---

## 11. Hàm getSeatCount()

```cpp
int SeatService::getSeatCount()
{
    authService.requireLogin();
    return seatRepo.count();
}
```

### Chức năng

Lấy tổng số Seat đang được quản lý trong SeatRepository.

Đây là số ghế của toàn bộ các phòng, không phải số ghế của một phòng cụ thể.

Hàm yêu cầu đăng nhập.

---

## 12. Luồng hoạt động

### 12.1. Quan hệ giữa các lớp

```text
                SeatService
                 /       \
                v         v
         AuthService   SeatRepository
                            |
                     +------+------+
                     |             |
                     v             v
              Repository<Seat> RoomRepository
                     |             |
                     v             v
                    Seat <------> Room
```

### 12.2. Luồng thêm ghế

```text
addSeat(seat)
      |
      v
requireAdmin()
      |
      v
SeatRepository::add()
      |
      v
Kiểm tra dữ liệu Seat
      |
      v
Tìm Room tương ứng
      |
      v
Kiểm tra vị trí và liên kết
      |
      v
Repository<Seat>::add()
      |
      v
Thành công hoặc rollback
```

### 12.3. Luồng xóa ghế

```text
removeSeat(roomId, seatId)
        |
        v
requireAdmin()
        |
        v
SeatRepository::removeById()
        |
        v
Tìm Seat
        |
        v
Tìm Room đang liên kết
        |
        v
Room::unlinkSeat()
        |
        v
Repository<Seat>::removeById()
        |
        v
Giải phóng Seat
```

---

## 13. Phân chia trách nhiệm

| Quy tắc | Lớp xử lý |
|---|---|
| Kiểm tra quyền Admin | SeatService |
| Yêu cầu đăng nhập khi tra cứu | SeatService |
| Kiểm tra Room ID và Seat ID | SeatRepository |
| Kiểm tra loại ghế | SeatRepository |
| Kiểm tra khóa ghép | Repository&lt;Seat&gt; |
| Kiểm tra vị trí ghế trong Room | Room |
| Không cho trùng vị trí | Room |
| Không thay đổi hàng/cột khi update | SeatRepository |
| Liên kết và tháo liên kết Room–Seat | SeatRepository và Model |
| Đọc/ghi file TXT | Repository&lt;Seat&gt; |

---

## 14. Các nghiệp vụ cần bổ sung sau này

Khi tích hợp Ticket và Showtime, SeatService có thể cần thêm các kiểm tra:

- Không xóa ghế đã được Ticket tham chiếu.
- Không thay đổi loại ghế nếu ảnh hưởng đến vé đã bán.
- Không thay đổi sơ đồ ghế khi có suất chiếu hoặc vé liên quan.

Những nghiệp vụ này chưa được triển khai trong SeatService hiện tại.

---

## 15. Kiểm thử

File:

```text
tests/test_seat_service.cpp
```

Các nhóm kiểm thử:

- Admin được thêm, sửa, xóa ghế.
- Staff không được thêm, sửa, xóa ghế.
- Admin và Staff được tra cứu.
- Người chưa đăng nhập không được tra cứu.
- Không cập nhật Seat không tồn tại.
- Không thay đổi vị trí Seat.
- Kiểm tra kết quả thêm, sửa, xóa từ Repository.
- Kiểm tra load và save.

---

## 16. Kết luận

`SeatService` chịu trách nhiệm phân quyền và điều phối các thao tác quản lý ghế.

Các quy tắc dữ liệu và liên kết Room–Seat được xử lý tại SeatRepository và Model.

Thiết kế này đồng bộ với MovieService và RoomService, đồng thời cho phép bổ sung các nghiệp vụ liên quan đến Ticket và Showtime trong những bước tiếp theo.