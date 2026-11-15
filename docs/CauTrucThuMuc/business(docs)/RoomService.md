# RoomService – Nghiệp vụ quản lý phòng chiếu

## 1. Tổng quan

`RoomService` là lớp thuộc tầng Business Logic, chịu trách nhiệm xử lý các nghiệp vụ quản lý phòng chiếu và kiểm tra quyền truy cập.

Lớp sử dụng:

- `RoomRepository` để thêm, sửa, xóa và truy xuất dữ liệu Room.
- `AuthService` để kiểm tra trạng thái đăng nhập và quyền của người thực hiện.

RoomService không trực tiếp đọc hoặc ghi file TXT.

**Quy tắc phân quyền:**

- **Admin:** Được thêm, sửa, xóa và tra cứu phòng.
- **Staff:** Chỉ được tra cứu thông tin phòng.
- **Chưa đăng nhập:** Không được thực hiện các thao tác quản lý hoặc tra cứu phòng.

Các thao tác `load()` và `save()` được xem là thao tác quản lý dữ liệu của hệ thống, không yêu cầu quyền Admin.

**File liên quan:**

- `src/business/RoomService.h`
- `src/business/RoomService.cpp`
- `src/business/AuthService.h`
- `src/data_access/RoomRepository.h`

---

## 2. Thuộc tính

```cpp
private:
    RoomRepository& roomRepo;
    AuthService& authService;
```

### `RoomRepository& roomRepo`

Tham chiếu đến Repository quản lý dữ liệu phòng chiếu.

Được sử dụng để thêm, cập nhật, xóa và truy xuất Room.

### `AuthService& authService`

Tham chiếu đến đối tượng kiểm tra đăng nhập và phân quyền.

RoomService không sở hữu hai đối tượng này.

Vì vậy, RoomRepository và AuthService phải tồn tại lâu hơn RoomService.

---

## 3. Constructor

```cpp
RoomService::RoomService(
    RoomRepository& roomRepo,
    AuthService& authService
)
    : roomRepo(roomRepo),
      authService(authService)
{}
```

### Chức năng

Nhận các đối tượng Repository và AuthService đã được khởi tạo.

RoomService không tự tạo Repository hoặc tự đăng nhập.

Cách thiết kế này giúp tách biệt trách nhiệm giữa tầng Business và Data Access.

---

## 4. Hàm load()

```cpp
bool RoomService::load()
{
    return roomRepo.load();
}
```

### Chức năng

Nạp dữ liệu phòng chiếu từ file TXT thông qua RoomRepository.

RoomService không tự đọc file hoặc chuyển đổi dữ liệu.

Đây là thao tác khởi tạo dữ liệu của hệ thống, không yêu cầu quyền Admin.

---

## 5. Hàm save()

```cpp
bool RoomService::save()
{
    return roomRepo.save();
}
```

### Chức năng

Lưu dữ liệu Room hiện tại xuống file TXT thông qua Repository.

Đây là thao tác lưu trữ dữ liệu của hệ thống, không yêu cầu quyền Admin.

---

## 6. Hàm addRoom()

```cpp
bool RoomService::addRoom(Room* room)
{
    authService.requireAdmin();
    return roomRepo.add(room);
}
```

### Chức năng

Thêm một phòng chiếu mới vào hệ thống.

### Bước 1: Kiểm tra quyền

```cpp
authService.requireAdmin();
```

Chỉ tài khoản Admin được phép thêm phòng.

Nếu chưa đăng nhập hoặc không có quyền Admin, hàm ném ngoại lệ.

### Bước 2: Thêm phòng

```cpp
return roomRepo.add(room);
```

RoomRepository kiểm tra:

- Repository đã load.
- Dữ liệu Room hợp lệ.
- Không trùng con trỏ.
- Không trùng ID.

### Kết quả

- `true`: Thêm phòng thành công.
- `false`: Repository từ chối thêm.
- Ném `runtime_error`: Không có quyền Admin.

Khi thêm thành công, Repository quản lý vòng đời đối tượng Room.

---

## 7. Hàm updateRoom()

```cpp
bool RoomService::updateRoom(const Room& newValues)
```

### Chức năng

Cập nhật thông tin phòng chiếu, đồng thời kiểm tra phân quyền và các ràng buộc nghiệp vụ.

Đây là hàm có nhiều quy tắc hơn các thao tác CRUD thông thường.

### Bước 1: Kiểm tra quyền Admin

```cpp
authService.requireAdmin();
```

Nếu không có quyền Admin, hàm ném ngoại lệ và không thực hiện cập nhật.

### Bước 2: Kiểm tra Repository

```cpp
if (!roomRepo.isLoaded())
{
    return false;
}
```

Không cho phép cập nhật khi dữ liệu chưa được load.

### Bước 3: Tìm phòng hiện tại

```cpp
Room* room = roomRepo.findById(newValues.getRoomId());

if (room == nullptr)
{
    return false;
}
```

Hàm tìm phòng theo ID và thao tác trên đối tượng đang được quản lý.

### Bước 4: Kiểm tra trạng thái active

```cpp
if (room->isActive() != newValues.isActive())
{
    return false;
}
```

Không cho phép thay đổi trạng thái hoạt động thông qua `updateRoom()`.

Ví dụ:

- Phòng đang hoạt động, dữ liệu cập nhật vẫn active: Có thể tiếp tục.
- Phòng đang hoạt động, dữ liệu cập nhật chuyển inactive: Bị từ chối.

Việc thay đổi trạng thái phòng sẽ được xử lý bằng nghiệp vụ riêng nếu cần bổ sung sau này.

### Bước 5: Kiểm tra thay đổi kích thước

```cpp
bool sizeChanged =
    room->getRowCount() != newValues.getRowCount() ||
    room->getColumnCount() != newValues.getColumnCount();
```

Biến `sizeChanged` cho biết số hàng hoặc số cột của phòng có thay đổi hay không.

### Bước 6: Kiểm tra Showtime

```cpp
if (sizeChanged && room->getShowtimeCount() > 0)
{
    return false;
}
```

Nếu phòng đã có suất chiếu, không cho phép thay đổi kích thước phòng.

Quy tắc này tránh làm thay đổi sơ đồ phòng khi các suất chiếu đang liên kết với phòng đó.

### Bước 7: Gọi Repository cập nhật

```cpp
return roomRepo.update(newValues);
```

Sau khi vượt qua các kiểm tra nghiệp vụ, RoomService chuyển việc cập nhật cho Repository.

RoomRepository tiếp tục kiểm tra:

- Dữ liệu phòng hợp lệ.
- Không có ghế nằm ngoài kích thước phòng mới.

### Kết quả

- `true`: Cập nhật thành công.
- `false`: Không thỏa điều kiện cập nhật.
- Ném `runtime_error`: Không có quyền Admin.

---

## 8. Hàm removeRoom()

```cpp
bool RoomService::removeRoom(const string& roomId)
{
    authService.requireAdmin();
    return roomRepo.removeById(roomId);
}
```

### Chức năng

Xóa một phòng chiếu theo ID.

### Quy trình

1. Kiểm tra quyền Admin.
2. Gọi RoomRepository để xóa phòng.
3. Repository kiểm tra phòng có tồn tại hay không.
4. Repository từ chối xóa nếu phòng còn Seat hoặc Showtime.

### Kết quả

- `true`: Xóa thành công.
- `false`: Không tìm thấy phòng hoặc phòng còn liên kết.
- Ném `runtime_error`: Không có quyền Admin.

---

## 9. Hàm findRoomById()

```cpp
const Room* RoomService::findRoomById(
    const string& roomId
)
{
    authService.requireLogin();
    return roomRepo.findById(roomId);
}
```

### Chức năng

Tìm phòng chiếu theo ID.

Yêu cầu người dùng đã đăng nhập.

Cả Admin và Staff đều có thể tra cứu.

### Vì sao trả về `const Room*`?

Để bên gọi không thể sửa trực tiếp thông tin Room thông qua con trỏ trả về.

Mọi thay đổi phải đi qua các hàm nghiệp vụ được kiểm soát quyền.

### Kết quả

- Trả về `const Room*` nếu tìm thấy.
- Trả về `nullptr` nếu không tìm thấy.
- Ném ngoại lệ nếu chưa đăng nhập.

---

## 10. Hàm getRoomAt()

```cpp
const Room* RoomService::getRoomAt(int index)
{
    authService.requireLogin();
    return roomRepo.getAt(index);
}
```

### Chức năng

Lấy phòng theo vị trí trong danh sách.

Yêu cầu đăng nhập.

Trả về `nullptr` nếu chỉ số không hợp lệ.

---

## 11. Hàm getRoomCount()

```cpp
int RoomService::getRoomCount()
{
    authService.requireLogin();
    return roomRepo.count();
}
```

### Chức năng

Lấy tổng số phòng chiếu đang được quản lý trong bộ nhớ.

Không phân biệt phòng active hay inactive.

Yêu cầu đăng nhập.

---

## 12. Luồng hoạt động

### 12.1. Quan hệ giữa các lớp

```text
              RoomService
               /       \
              v         v
       AuthService   RoomRepository
                          |
                          v
                    Repository<Room>
                          |
                          v
                        File TXT
```

### 12.2. Luồng cập nhật phòng

```text
updateRoom(newValues)
        |
        v
requireAdmin()
        |
        v
Kiểm tra Repository đã load
        |
        v
Tìm Room theo ID
        |
        v
Kiểm tra trạng thái active
        |
        v
Kiểm tra thay đổi kích thước
        |
        v
Kiểm tra liên kết Showtime
        |
        v
RoomRepository::update()
        |
        v
Kiểm tra dữ liệu và vị trí Seat
        |
        v
Cập nhật Room
```

---

## 13. Phân chia trách nhiệm

| Quy tắc | Lớp xử lý |
|---|---|
| Kiểm tra quyền Admin | RoomService |
| Yêu cầu đăng nhập khi tra cứu | RoomService |
| Không đổi active qua update | RoomService |
| Không đổi kích thước khi có Showtime | RoomService |
| Kiểm tra ID và tên phòng | RoomRepository |
| Kiểm tra số hàng, số cột | RoomRepository |
| Không thu nhỏ làm Seat nằm ngoài phòng | RoomRepository |
| Không xóa Room còn Seat hoặc Showtime | RoomRepository |
| Đọc/ghi TXT | Repository&lt;Room&gt; |

---

## 14. Kiểm thử

File:

```text
tests/test_room_service.cpp
```

Các nhóm kiểm thử:

- Admin được thêm, sửa, xóa.
- Staff không được thêm, sửa, xóa.
- Admin và Staff được tra cứu.
- Người chưa đăng nhập không được tra cứu.
- Không cho thay đổi active qua update.
- Không cho thay đổi kích thước khi có Showtime.
- Không cho xóa Room còn liên kết.
- Kiểm tra kết quả từ Repository.

---

## 15. Kết luận

`RoomService` chịu trách nhiệm kiểm soát quyền truy cập và các nghiệp vụ quản lý phòng chiếu.

Lớp không thay thế RoomRepository mà bổ sung những điều kiện nghiệp vụ trước khi gọi Repository.

Thiết kế này đồng bộ với cách triển khai của `MovieService` trong dự án.