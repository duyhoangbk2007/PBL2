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

## Chi tiết triển khai từng hàm (RoomService.cpp)

### RoomService()

#### A. Tổng quan

- **Mục đích:** Khởi tạo Service với các Repository và AuthService.
- **Tham số:** `RoomRepository& roomRepo, AuthService& authService) : roomRepo(roomRepo), authService(authService`.
- **Giá trị trả về:** `constructor` (không trả giá trị).
- **Điều kiện quan trọng:** điều kiện và lỗi thể hiện trực tiếp trong mã nguồn.

#### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Chuẩn bị và xử lý dữ liệu
    |
    v
Kết thúc
```

#### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1 — Chuẩn bị và xử lý dữ liệu.**

```cpp
RoomService::RoomService(RoomRepository& roomRepo, AuthService& authService)
    : roomRepo(roomRepo), authService(authService)
{}
```

Các câu lệnh được thực thi theo đúng thứ tự trong `.cpp`; điều kiện và nhánh trả về nằm ngay trong code. Giá trị được khai báo ở đây được dùng trong các nhánh tiếp theo; hãy theo dõi nơi nó được trả về hoặc chuyển vào Repository để hiểu tác động lên trạng thái.

### load()

#### A. Tổng quan

- **Mục đích:** Kiểm tra dữ liệu đã nạp. Xem các nhánh cụ thể trong code.
- **Tham số:** `không có`.
- **Giá trị trả về:** `bool`; các nhánh `return` được ghi đầy đủ bên dưới.
- **Điều kiện quan trọng:** điều kiện và lỗi thể hiện trực tiếp trong mã nguồn.

#### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Kiểm tra dữ liệu đã nạp
    |
    v
Kết thúc
```

#### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1 — Kiểm tra dữ liệu đã nạp.**

```cpp
bool RoomService::load()
{
    return roomRepo.load();
}
```

Các câu lệnh được thực thi theo đúng thứ tự trong `.cpp`; điều kiện và nhánh trả về nằm ngay trong code. Giá trị được khai báo ở đây được dùng trong các nhánh tiếp theo; hãy theo dõi nơi nó được trả về hoặc chuyển vào Repository để hiểu tác động lên trạng thái.

### save()

#### A. Tổng quan

- **Mục đích:** Cập nhật qua Repository. Xem các nhánh cụ thể trong code.
- **Tham số:** `không có`.
- **Giá trị trả về:** `bool`; các nhánh `return` được ghi đầy đủ bên dưới.
- **Điều kiện quan trọng:** điều kiện và lỗi thể hiện trực tiếp trong mã nguồn.

#### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Cập nhật qua Repository
    |
    v
Kết thúc
```

#### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1 — Cập nhật qua Repository.**

```cpp
bool RoomService::save()
{
    return roomRepo.save();
}
```

Repository thực hiện thao tác dữ liệu. `save()` ghi trạng thái Repository ra TXT tại thời điểm lời gọi; thay đổi RAM trước đó không tự đồng nghĩa với đã lưu thành công.

### addRoom()

#### A. Tổng quan

- **Mục đích:** Kiểm tra quyền. Xem các nhánh cụ thể trong code.
- **Tham số:** `Room* room`.
- **Giá trị trả về:** `bool`; các nhánh `return` được ghi đầy đủ bên dưới.
- **Điều kiện quan trọng:** Kiểm tra quyền qua `AuthService`; kiểm tra trạng thái nạp và con trỏ trước khi dùng.

#### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Kiểm tra quyền
    |
    v
Tra cứu dữ liệu
    |
    v
Chuẩn bị và xử lý dữ liệu
    |
    v
Cập nhật qua Repository
    |
    v
Trả kết quả
    |
    v
Kết thúc
```

#### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1 — Kiểm tra quyền.**

```cpp
bool RoomService::addRoom(Room* room)
{
    authService.requirePermission("ROOM_ADD");

    if (!roomRepo.isLoaded() || room == nullptr)
    {
        return false;
    }

    vector<string> existingIds;
```

`AuthService` kiểm tra quyền và có thể ném ngoại lệ. `nullptr` cho biết không có đối tượng để truy cập. `false` báo thao tác không thành công. Quyền `ROOM_ADD` được kiểm tra trước bước xử lý kế tiếp; thiếu quyền sẽ ném ngoại lệ, vì vậy caller không nên xem đó là kết quả `false`. Kiểm tra `isLoaded()` tránh dùng dữ liệu Repository chưa được nạp; nhánh không đạt kết thúc ngay theo giá trị `return` trong code. Nhánh `false` dừng hàm tại đây; những câu lệnh phía sau không chạy, nên không có cập nhật nào từ các bước sau.

**Bước 2 — Tra cứu dữ liệu.**

```cpp

    for (int i = 0; i < roomRepo.count(); i++)
    {
        Room* current = roomRepo.getAt(i);
        if (current == nullptr)
        {
            return false;
        }
        existingIds.push_back(current->getRoomId());
    }
```

`nullptr` cho biết không có đối tượng để truy cập. `false` báo thao tác không thành công. Vòng `for` duyệt lần lượt các phần tử. Nhánh `false` dừng hàm tại đây; những câu lệnh phía sau không chạy, nên không có cập nhật nào từ các bước sau. Vòng lặp xử lý từng phần tử theo thứ tự; điều kiện bên trong quyết định phần tử được giữ, bỏ qua hay đưa vào kết quả.

**Bước 3 — Chuẩn bị và xử lý dữ liệu.**

```cpp

    string roomId = IdGenerator::generateNextId("ROM", existingIds);

    Room* newRoom = new Room(roomId,
                             room->getName(),
                             room->getRowCount(),
                             room->getColumnCount(),
                             room->isActive());

    try
```

Các câu lệnh được thực thi theo đúng thứ tự trong `.cpp`; điều kiện và nhánh trả về nằm ngay trong code. `new` cấp phát đối tượng trên heap; cần theo dõi thời điểm quyền sở hữu được chuyển sang Repository để tránh rò rỉ bộ nhớ.

**Bước 4 — Cập nhật qua Repository.**

```cpp
    {
        if (!roomRepo.add(newRoom))
        {
            delete newRoom;
            return false;
        }
    }
    catch (...)
    {
        delete newRoom;
```

`false` báo thao tác không thành công. Repository thực hiện thao tác dữ liệu. Khi `add()` thành công, Repository nhận đối tượng và quản lý vòng đời của nó; nếu thất bại, bên gọi vẫn phải xét trách nhiệm giải phóng con trỏ. Nhánh `false` dừng hàm tại đây; những câu lệnh phía sau không chạy, nên không có cập nhật nào từ các bước sau. Khối `try/catch` giữ lỗi phát sinh trong phạm vi xử lý này và chuyển sang nhánh khôi phục hoặc báo lỗi đã viết trong code. `delete` giải phóng vùng nhớ của đối tượng, sau đó không được dùng lại con trỏ trỏ đến vùng nhớ đó.

**Bước 5 — Trả kết quả.**

```cpp
        throw;
    }

    return true;
}
```

Các câu lệnh được thực thi theo đúng thứ tự trong `.cpp`; điều kiện và nhánh trả về nằm ngay trong code. Giá trị được khai báo ở đây được dùng trong các nhánh tiếp theo; hãy theo dõi nơi nó được trả về hoặc chuyển vào Repository để hiểu tác động lên trạng thái.

### updateRoom()

#### A. Tổng quan

- **Mục đích:** Kiểm tra quyền. Xem các nhánh cụ thể trong code.
- **Tham số:** `const Room& newValues`.
- **Giá trị trả về:** `bool`; các nhánh `return` được ghi đầy đủ bên dưới.
- **Điều kiện quan trọng:** Kiểm tra quyền qua `AuthService`; kiểm tra trạng thái nạp và con trỏ trước khi dùng.

#### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Kiểm tra quyền
    |
    v
Từ chối dữ liệu không hợp lệ
    |
    v
Cập nhật qua Repository
    |
    v
Kết thúc
```

#### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1 — Kiểm tra quyền.**

```cpp
bool RoomService::updateRoom(const Room& newValues)
{
    authService.requirePermission("ROOM_UPDATE");
    if (!roomRepo.isLoaded())
    {
        return false;
    }
    Room* room = roomRepo.findById(newValues.getRoomId());
    if (room == nullptr)
    {
```

`AuthService` kiểm tra quyền và có thể ném ngoại lệ. `nullptr` cho biết không có đối tượng để truy cập. `false` báo thao tác không thành công. Quyền `ROOM_UPDATE` được kiểm tra trước bước xử lý kế tiếp; thiếu quyền sẽ ném ngoại lệ, vì vậy caller không nên xem đó là kết quả `false`. Kiểm tra `isLoaded()` tránh dùng dữ liệu Repository chưa được nạp; nhánh không đạt kết thúc ngay theo giá trị `return` trong code. Lệnh tra cứu trả con trỏ đến đối tượng đang được Repository quản lý. Con trỏ rỗng có nghĩa là không tìm thấy, nên bước sau chỉ được dùng đối tượng khi kiểm tra thành công. Nhánh `false` dừng hàm tại đây; những câu lệnh phía sau không chạy, nên không có cập nhật nào từ các bước sau.

**Bước 2 — Từ chối dữ liệu không hợp lệ.**

```cpp
        return false;
    }
    // Khong cho doi trang thai qua ham update
    if (room->isActive() != newValues.isActive())
    {
        return false;
    }
    bool sizeChanged = room->getRowCount() != newValues.getRowCount() || room->getColumnCount() != newValues.getColumnCount();
    // Khong cho doi kich thuoc neu da co suat chieu
    if (sizeChanged && room->getShowtimeCount() > 0)
```

`false` báo thao tác không thành công. Nhánh `false` dừng hàm tại đây; những câu lệnh phía sau không chạy, nên không có cập nhật nào từ các bước sau.

**Bước 3 — Cập nhật qua Repository.**

```cpp
    {
        return false;
    }
    return roomRepo.update(newValues);
}
```

`false` báo thao tác không thành công. Repository thực hiện thao tác dữ liệu. `update()` chuyển việc thay đổi dữ liệu cho Repository; kết quả trả về phản ánh kiểm tra và thao tác cập nhật ở tầng dữ liệu. Nhánh `false` dừng hàm tại đây; những câu lệnh phía sau không chạy, nên không có cập nhật nào từ các bước sau.

### removeRoom()

#### A. Tổng quan

- **Mục đích:** Kiểm tra quyền. Xem các nhánh cụ thể trong code.
- **Tham số:** `const string& roomId`.
- **Giá trị trả về:** `bool`; các nhánh `return` được ghi đầy đủ bên dưới.
- **Điều kiện quan trọng:** Kiểm tra quyền qua `AuthService`; điều kiện và lỗi thể hiện trực tiếp trong mã nguồn.

#### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Kiểm tra quyền
    |
    v
Kết thúc
```

#### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1 — Kiểm tra quyền.**

```cpp
bool RoomService::removeRoom(const string& roomId)
{
    authService.requirePermission("ROOM_DELETE");
    return roomRepo.removeById(roomId);
}
```

`AuthService` kiểm tra quyền và có thể ném ngoại lệ. Repository thực hiện thao tác dữ liệu. Quyền `ROOM_DELETE` được kiểm tra trước bước xử lý kế tiếp; thiếu quyền sẽ ném ngoại lệ, vì vậy caller không nên xem đó là kết quả `false`. Thao tác xóa có thể làm mất đối tượng do Repository sở hữu; các liên kết liên quan cần được Repository xử lý theo đúng ràng buộc của nó.

### findRoomById()

#### A. Tổng quan

- **Mục đích:** Kiểm tra quyền. Xem các nhánh cụ thể trong code.
- **Tham số:** `const string& roomId`.
- **Giá trị trả về:** `const Room*`; các nhánh `return` được ghi đầy đủ bên dưới.
- **Điều kiện quan trọng:** Kiểm tra quyền qua `AuthService`; điều kiện và lỗi thể hiện trực tiếp trong mã nguồn.

#### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Kiểm tra quyền
    |
    v
Kết thúc
```

#### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1 — Kiểm tra quyền.**

```cpp
const Room* RoomService::findRoomById(const string& roomId)
{
    authService.requirePermission("ROOM_VIEW");
    return roomRepo.findById(roomId);
}
```

`AuthService` kiểm tra quyền và có thể ném ngoại lệ. Quyền `ROOM_VIEW` được kiểm tra trước bước xử lý kế tiếp; thiếu quyền sẽ ném ngoại lệ, vì vậy caller không nên xem đó là kết quả `false`. Lệnh tra cứu trả con trỏ đến đối tượng đang được Repository quản lý. Con trỏ rỗng có nghĩa là không tìm thấy, nên bước sau chỉ được dùng đối tượng khi kiểm tra thành công.

### getRoomAt()

#### A. Tổng quan

- **Mục đích:** Kiểm tra quyền. Xem các nhánh cụ thể trong code.
- **Tham số:** `int index`.
- **Giá trị trả về:** `const Room*`; các nhánh `return` được ghi đầy đủ bên dưới.
- **Điều kiện quan trọng:** Kiểm tra quyền qua `AuthService`; điều kiện và lỗi thể hiện trực tiếp trong mã nguồn.

#### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Kiểm tra quyền
    |
    v
Kết thúc
```

#### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1 — Kiểm tra quyền.**

```cpp
const Room* RoomService::getRoomAt(int index)
{
    authService.requirePermission("ROOM_VIEW");
    return roomRepo.getAt(index);
}
```

`AuthService` kiểm tra quyền và có thể ném ngoại lệ. Quyền `ROOM_VIEW` được kiểm tra trước bước xử lý kế tiếp; thiếu quyền sẽ ném ngoại lệ, vì vậy caller không nên xem đó là kết quả `false`.

### getRoomCount()

#### A. Tổng quan

- **Mục đích:** Kiểm tra quyền. Xem các nhánh cụ thể trong code.
- **Tham số:** `không có`.
- **Giá trị trả về:** `int`; các nhánh `return` được ghi đầy đủ bên dưới.
- **Điều kiện quan trọng:** Kiểm tra quyền qua `AuthService`; điều kiện và lỗi thể hiện trực tiếp trong mã nguồn.

#### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Kiểm tra quyền
    |
    v
Kết thúc
```

#### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1 — Kiểm tra quyền.**

```cpp
int RoomService::getRoomCount()
{
    authService.requirePermission("ROOM_VIEW");
    return roomRepo.count();
}
```

`AuthService` kiểm tra quyền và có thể ném ngoại lệ. Quyền `ROOM_VIEW` được kiểm tra trước bước xử lý kế tiếp; thiếu quyền sẽ ném ngoại lệ, vì vậy caller không nên xem đó là kết quả `false`.

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
