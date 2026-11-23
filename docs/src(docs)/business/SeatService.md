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

## Chi tiết triển khai từng hàm (SeatService.cpp)

### SeatService()

#### A. Tổng quan

- **Mục đích:** Khởi tạo Service với các Repository và AuthService.
- **Tham số:** `SeatRepository& seatRepo, AuthService& authService) : seatRepo(seatRepo), authService(authService`.
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
SeatService::SeatService(SeatRepository& seatRepo, AuthService& authService)
    : seatRepo(seatRepo), authService(authService)
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
bool SeatService::load()
{
    return seatRepo.load();
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
bool SeatService::save()
{
    return seatRepo.save();
}
```

Repository thực hiện thao tác dữ liệu. `save()` ghi trạng thái Repository ra TXT tại thời điểm lời gọi; thay đổi RAM trước đó không tự đồng nghĩa với đã lưu thành công.

### addSeat()

#### A. Tổng quan

- **Mục đích:** Kiểm tra quyền. Xem các nhánh cụ thể trong code.
- **Tham số:** `Seat* seat`.
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
bool SeatService::addSeat(Seat* seat)
{
    authService.requirePermission("SEAT_ADD");
    return seatRepo.add(seat);
}
```

`AuthService` kiểm tra quyền và có thể ném ngoại lệ. Repository thực hiện thao tác dữ liệu. Quyền `SEAT_ADD` được kiểm tra trước bước xử lý kế tiếp; thiếu quyền sẽ ném ngoại lệ, vì vậy caller không nên xem đó là kết quả `false`. Khi `add()` thành công, Repository nhận đối tượng và quản lý vòng đời của nó; nếu thất bại, bên gọi vẫn phải xét trách nhiệm giải phóng con trỏ.

### updateSeat()

#### A. Tổng quan

- **Mục đích:** Kiểm tra quyền. Xem các nhánh cụ thể trong code.
- **Tham số:** `const Seat& newValues`.
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
Cập nhật qua Repository
    |
    v
Kết thúc
```

#### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1 — Kiểm tra quyền.**

```cpp
bool SeatService::updateSeat(const Seat& newValues)
{
    authService.requirePermission("SEAT_UPDATE");
    if (!seatRepo.isLoaded())
    {
        return false;
    }
    Seat* seat = seatRepo.findById(newValues.getRoomId(), newValues.getSeatId());
    if (seat == nullptr)
    {
```

`AuthService` kiểm tra quyền và có thể ném ngoại lệ. `nullptr` cho biết không có đối tượng để truy cập. `false` báo thao tác không thành công. Quyền `SEAT_UPDATE` được kiểm tra trước bước xử lý kế tiếp; thiếu quyền sẽ ném ngoại lệ, vì vậy caller không nên xem đó là kết quả `false`. Kiểm tra `isLoaded()` tránh dùng dữ liệu Repository chưa được nạp; nhánh không đạt kết thúc ngay theo giá trị `return` trong code. Lệnh tra cứu trả con trỏ đến đối tượng đang được Repository quản lý. Con trỏ rỗng có nghĩa là không tìm thấy, nên bước sau chỉ được dùng đối tượng khi kiểm tra thành công. Nhánh `false` dừng hàm tại đây; những câu lệnh phía sau không chạy, nên không có cập nhật nào từ các bước sau.

**Bước 2 — Cập nhật qua Repository.**

```cpp
        return false;
    }
    return seatRepo.update(newValues);
}
```

`false` báo thao tác không thành công. Repository thực hiện thao tác dữ liệu. `update()` chuyển việc thay đổi dữ liệu cho Repository; kết quả trả về phản ánh kiểm tra và thao tác cập nhật ở tầng dữ liệu. Nhánh `false` dừng hàm tại đây; những câu lệnh phía sau không chạy, nên không có cập nhật nào từ các bước sau.

### removeSeat()

#### A. Tổng quan

- **Mục đích:** Kiểm tra quyền. Xem các nhánh cụ thể trong code.
- **Tham số:** `const string& roomId, const string& seatId`.
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
bool SeatService::removeSeat(const string& roomId, const string& seatId)
{
    authService.requirePermission("SEAT_DELETE");
    return seatRepo.removeById(roomId, seatId);
}
```

`AuthService` kiểm tra quyền và có thể ném ngoại lệ. Repository thực hiện thao tác dữ liệu. Quyền `SEAT_DELETE` được kiểm tra trước bước xử lý kế tiếp; thiếu quyền sẽ ném ngoại lệ, vì vậy caller không nên xem đó là kết quả `false`. Thao tác xóa có thể làm mất đối tượng do Repository sở hữu; các liên kết liên quan cần được Repository xử lý theo đúng ràng buộc của nó.

### findSeatById()

#### A. Tổng quan

- **Mục đích:** Kiểm tra quyền. Xem các nhánh cụ thể trong code.
- **Tham số:** `const string& roomId, const string& seatId`.
- **Giá trị trả về:** `const Seat*`; các nhánh `return` được ghi đầy đủ bên dưới.
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
const Seat* SeatService::findSeatById(const string& roomId, const string& seatId)
{
    authService.requirePermission("SEAT_VIEW");
    return seatRepo.findById(roomId, seatId);
}
```

`AuthService` kiểm tra quyền và có thể ném ngoại lệ. Quyền `SEAT_VIEW` được kiểm tra trước bước xử lý kế tiếp; thiếu quyền sẽ ném ngoại lệ, vì vậy caller không nên xem đó là kết quả `false`. Lệnh tra cứu trả con trỏ đến đối tượng đang được Repository quản lý. Con trỏ rỗng có nghĩa là không tìm thấy, nên bước sau chỉ được dùng đối tượng khi kiểm tra thành công.

### getSeatAt()

#### A. Tổng quan

- **Mục đích:** Kiểm tra quyền. Xem các nhánh cụ thể trong code.
- **Tham số:** `int index`.
- **Giá trị trả về:** `const Seat*`; các nhánh `return` được ghi đầy đủ bên dưới.
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
const Seat* SeatService::getSeatAt(int index)
{
    authService.requirePermission("SEAT_VIEW");
    return seatRepo.getAt(index);
}
```

`AuthService` kiểm tra quyền và có thể ném ngoại lệ. Quyền `SEAT_VIEW` được kiểm tra trước bước xử lý kế tiếp; thiếu quyền sẽ ném ngoại lệ, vì vậy caller không nên xem đó là kết quả `false`.

### getSeatCount()

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
int SeatService::getSeatCount()
{
    authService.requirePermission("SEAT_VIEW");
    return seatRepo.count();
}
```

`AuthService` kiểm tra quyền và có thể ném ngoại lệ. Quyền `SEAT_VIEW` được kiểm tra trước bước xử lý kế tiếp; thiếu quyền sẽ ném ngoại lệ, vì vậy caller không nên xem đó là kết quả `false`.

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
