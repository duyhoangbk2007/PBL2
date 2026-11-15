# SeatRepository – Quản lý dữ liệu ghế ngồi

## 1. Tổng quan

`SeatRepository` là lớp thuộc tầng Data Access, chịu trách nhiệm quản lý dữ liệu ghế ngồi trong các phòng chiếu.

Lớp sử dụng `Repository<Seat>` để tái sử dụng các chức năng quản lý danh sách đối tượng, đọc/ghi file TXT, tìm kiếm và giải phóng bộ nhớ.

Khác với `MovieRepository` và `RoomRepository`, `SeatRepository` cần sử dụng thêm `RoomRepository` để xác định phòng sở hữu mỗi ghế và thiết lập liên kết giữa Seat và Room.

Các chức năng chính:

- Đọc và ghi dữ liệu ghế từ file TXT.
- Kiểm tra tính hợp lệ của Seat.
- Xác định ghế bằng khóa ghép `roomId|seatId`.
- Thêm ghế và liên kết với phòng.
- Cập nhật loại ghế.
- Xóa ghế và tháo liên kết với phòng.
- Kiểm tra trùng vị trí ghế.
- Khôi phục trạng thái khi liên kết thất bại.

**File liên quan:**

- `src/data_access/SeatRepository.h`
- `src/data_access/SeatRepository.cpp`
- `src/data_access/RoomRepository.h`
- `src/data_access/Repository.h`
- `src/models/Seat.h`
- `src/models/Room.h`

---

## 2. Cấu trúc lớp

### 2.1. Thuộc tính

```cpp
private:
    Repository<Seat> repo;
    RoomRepository& roomRepo;
```

**`Repository<Seat> repo`**

Quản lý danh sách đối tượng `Seat*`, thực hiện đọc/ghi file và quản lý bộ nhớ của các Seat đã thêm thành công.

**`RoomRepository& roomRepo`**

Tham chiếu đến Repository quản lý phòng chiếu.

Được sử dụng để:

- Kiểm tra phòng có tồn tại hay không.
- Tìm Room tương ứng với mỗi Seat.
- Thiết lập liên kết giữa Seat và Room.

`SeatRepository` không sở hữu `RoomRepository`.

Vì vậy, `RoomRepository` phải tồn tại lâu hơn `SeatRepository`.

### 2.2. Các hàm private

```cpp
static string getSeatKey(Seat* seat);
static Seat* stringToSeat(const string& line);
static string seatToString(Seat* seat);
static bool isValidSeat(const Seat* seat);
```

Các hàm hỗ trợ Repository chung lấy khóa, chuyển đổi dữ liệu và kiểm tra tính hợp lệ.

---

## 3. Constructor

```cpp
SeatRepository::SeatRepository(
    const string& filePath,
    RoomRepository& roomRepo
)
    : repo(filePath, getSeatKey, stringToSeat, seatToString),
      roomRepo(roomRepo)
{}
```

### Chức năng

Khởi tạo `Repository<Seat>` và nhận tham chiếu đến `RoomRepository`.

Ví dụ:

```cpp
RoomRepository roomRepo("data/rooms.txt");
SeatRepository seatRepo("data/seats.txt", roomRepo);
```

Thứ tự khởi tạo quan trọng vì Seat phụ thuộc vào Room.

Khi nạp dữ liệu, cần load Room trước Seat:

```cpp
roomRepo.load();
seatRepo.load();
```

Constructor chưa tự động load dữ liệu hoặc liên kết các ghế.

---

## 4. Hàm getSeatKey()

```cpp
string SeatRepository::getSeatKey(Seat* seat)
{
    if (seat == nullptr)
    {
        return "";
    }

    return seat->getRoomId() + "|" + seat->getSeatId();
}
```

### Chức năng

Tạo khóa định danh duy nhất cho một Seat.

Khác với Movie và Room, `seatId` chỉ cần duy nhất trong phạm vi một phòng.

Ví dụ:

| Room ID | Seat ID | Khóa |
|---|---|---|
| ROM0001 | A1 | ROM0001\|A1 |
| ROM0001 | A2 | ROM0001\|A2 |
| ROM0002 | A1 | ROM0002\|A1 |

Hai phòng khác nhau được phép có cùng `seatId`.

Tuy nhiên, hai ghế trong cùng phòng không được trùng `seatId`.

`Repository<Seat>` sử dụng khóa ghép này để kiểm tra trùng và tìm kiếm.

---

## 5. Hàm isValidSeat()

```cpp
bool SeatRepository::isValidSeat(const Seat* seat)
```

### Chức năng

Kiểm tra dữ liệu cơ bản của một Seat.

### Bước 1: Kiểm tra con trỏ

```cpp
if (seat == nullptr)
{
    return false;
}
```

### Bước 2: Kiểm tra Room ID

```cpp
if (!Validator::isValidId(seat->getRoomId(), "ROM"))
{
    return false;
}
```

Mã phòng phải đúng định dạng `ROMxxxx`.

### Bước 3: Kiểm tra Seat ID

```cpp
if (seat->getSeatId().empty() ||
    seat->getSeatId().find('|') != string::npos)
{
    return false;
}
```

Seat ID không được rỗng hoặc chứa ký tự `|`.

Lý do không cho chứa `|` là vì ký tự này được dùng làm dấu phân cách trong file TXT và khóa ghép.

### Bước 4: Kiểm tra vị trí

```cpp
if (seat->getRow() <= 0 || seat->getColumn() <= 0)
{
    return false;
}
```

Số hàng và số cột phải lớn hơn 0.

### Bước 5: Kiểm tra loại ghế

```cpp
SeatType type = seat->getSeatType();

if (type != SeatType::STANDARD &&
    type != SeatType::VIP &&
    type != SeatType::COUPLE)
{
    return false;
}
```

Chỉ chấp nhận ba loại ghế:

- `STANDARD`: Ghế thường.
- `VIP`: Ghế VIP.
- `COUPLE`: Ghế đôi.

### Kết quả

Trả về `true` nếu dữ liệu hợp lệ.

**Lưu ý:** `isValidSeat()` chưa kiểm tra phòng có tồn tại, vị trí có nằm trong kích thước phòng hoặc có trùng với ghế khác hay không.

Những điều kiện đó được kiểm tra khi liên kết Seat với Room.

---

## 6. Hàm stringToSeat()

```cpp
Seat* SeatRepository::stringToSeat(const string& line)
```

### Chức năng

Chuyển một dòng TXT thành đối tượng Seat.

Định dạng:

```text
roomId|seatId|row|column|seatType
```

Ví dụ:

```text
ROM0001|A1|1|1|STANDARD
ROM0001|A2|1|2|VIP
ROM0002|B1|2|1|COUPLE
```

### Bước 1: Tách dữ liệu

Sử dụng `vector<string>` và dấu `|` để tách các trường.

Dữ liệu phải có đúng 5 trường.

```cpp
if (fields.size() != 5)
{
    throw runtime_error("Sai so luong truong Seat");
}
```

### Bước 2: Chuyển đổi hàng và cột

```cpp
row = stoi(fields[2], &pos1);
column = stoi(fields[3], &pos2);
```

Kiểm tra toàn bộ chuỗi đã được chuyển thành số nguyên.

Nếu dữ liệu như `"1abc"` hoặc `"abc"`, hàm ném ngoại lệ.

### Bước 3: Chuyển đổi loại ghế

```cpp
if (fields[4] == "STANDARD")
{
    seatType = SeatType::STANDARD;
}
else if (fields[4] == "VIP")
{
    seatType = SeatType::VIP;
}
else if (fields[4] == "COUPLE")
{
    seatType = SeatType::COUPLE;
}
else
{
    throw runtime_error("Loai ghe khong hop le");
}
```

### Bước 4: Tạo Seat

```cpp
Seat* seat = new Seat(
    fields[0],
    fields[1],
    row,
    column,
    seatType
);
```

### Bước 5: Kiểm tra đối tượng

```cpp
if (!isValidSeat(seat))
{
    delete seat;
    throw runtime_error("Du lieu Seat khong hop le");
}
```

Nếu dữ liệu sai, đối tượng được giải phóng trước khi ném ngoại lệ.

### Kết quả

Trả về `Seat*` nếu chuyển đổi thành công.

Hàm này chưa liên kết Seat với Room. Việc liên kết được thực hiện trong `load()`.

---

## 7. Hàm seatToString()

```cpp
string SeatRepository::seatToString(Seat* seat)
```

### Chức năng

Chuyển đối tượng Seat thành một dòng dữ liệu TXT.

Đầu tiên, kiểm tra `isValidSeat()`.

Sau đó chuyển `SeatType` thành chuỗi tương ứng.

Ví dụ:

```text
ROM0001|A1|1|1|STANDARD
```

Hàm không ghi file trực tiếp.

---

## 8. Hàm load()

```cpp
bool SeatRepository::load()
```

### Chức năng

Nạp danh sách Seat từ file TXT, sau đó liên kết từng ghế với Room tương ứng.

Đây là một trong những hàm quan trọng nhất của SeatRepository.

### Bước 1: Kiểm tra điều kiện load

```cpp
if (repo.isLoaded() || !roomRepo.isLoaded())
{
    return false;
}
```

Không cho phép load nếu:

- SeatRepository đã load.
- RoomRepository chưa load.

### Bước 2: Nạp dữ liệu Seat

```cpp
if (!repo.load())
{
    return false;
}
```

Repository chung đọc dữ liệu và tạo các đối tượng Seat.

### Bước 3: Duyệt từng ghế

```cpp
for (int i = 0; i < repo.count(); i++)
{
    Seat* seat = repo.getAt(i);
    Room* room = roomRepo.findById(seat->getRoomId());
```

Mỗi Seat được xác định phòng tương ứng thông qua `roomId`.

### Bước 4: Liên kết Seat với Room

```cpp
if (room == nullptr || !seat->linkRoom(room))
```

Việc liên kết thất bại nếu:

- Không tìm thấy phòng.
- Ghế nằm ngoài kích thước phòng.
- Ghế trùng Seat ID trong phòng.
- Ghế trùng vị trí hàng/cột.
- Ghế không đáp ứng điều kiện liên kết của Model.

### Bước 5: Rollback khi thất bại

```cpp
for (int j = 0; j < i; j++)
{
    Seat* linkedSeat = repo.getAt(j);

    if (linkedSeat->getRoom() != nullptr)
    {
        linkedSeat->getRoom()->unlinkSeat(linkedSeat);
    }
}

repo.clear();
return false;
```

### Vì sao phải rollback?

Giả sử file có ba ghế:

```text
ROM0001|A1|1|1|STANDARD
ROM0001|A2|1|2|VIP
ROM9999|A3|1|3|STANDARD
```

Hai ghế đầu liên kết thành công.

Ghế thứ ba thất bại vì phòng `ROM9999` không tồn tại.

Nếu chỉ trả về `false`, hai ghế đầu vẫn nằm trong danh sách liên kết của Room, khiến trạng thái dữ liệu không nhất quán.

Vì vậy, hàm:

1. Tháo liên kết của những ghế đã liên kết thành công.
2. Xóa danh sách Seat vừa load.
3. Đưa SeatRepository về trạng thái chưa load.
4. Trả về `false`.

Cách xử lý này bảo đảm không giữ lại một phần dữ liệu sau khi liên kết thất bại.

### Kết quả

- `true`: Tất cả Seat được load và liên kết thành công.
- `false`: Không thể load hoặc liên kết thất bại.
- Có thể ném ngoại lệ khi file TXT sai định dạng.

---

## 9. Hàm save()

```cpp
bool SeatRepository::save()
{
    return repo.save();
}
```

Lưu toàn bộ dữ liệu Seat xuống file TXT thông qua Repository chung.

Không lưu trực tiếp danh sách liên kết của Room vì thông tin `roomId` đã được lưu trong từng Seat.

Khi load lại, liên kết được khôi phục từ `roomId`.

---

## 10. Hàm add()

```cpp
bool SeatRepository::add(Seat* seat)
```

### Chức năng

Thêm Seat mới vào Repository và liên kết với Room.

### Bước 1: Kiểm tra điều kiện ban đầu

```cpp
if (!repo.isLoaded() ||
    !roomRepo.isLoaded() ||
    !isValidSeat(seat))
{
    return false;
}
```

### Bước 2: Tìm phòng

```cpp
Room* room = roomRepo.findById(seat->getRoomId());

if (room == nullptr)
{
    return false;
}
```

### Bước 3: Kiểm tra trùng con trỏ

Duyệt danh sách để bảo đảm cùng một con trỏ Seat chưa được thêm trước đó.

### Bước 4: Kiểm tra ghế đã liên kết hay chưa

```cpp
if (seat->getRoom() != nullptr)
{
    return false;
}
```

Không thêm Seat đã liên kết với Room.

### Bước 5: Liên kết ghế

```cpp
if (!seat->linkRoom(room))
{
    return false;
}
```

Model Room kiểm tra vị trí, ID và các điều kiện liên kết.

### Bước 6: Thêm vào Repository

```cpp
if (!repo.add(seat))
{
    room->unlinkSeat(seat);
    return false;
}
```

Nếu thêm vào Repository thất bại sau khi liên kết thành công, hàm tháo liên kết để khôi phục trạng thái ban đầu.

### Kết quả

- `true`: Ghế được thêm và liên kết thành công.
- `false`: Không đáp ứng điều kiện.

Nếu thêm thành công, Repository sở hữu con trỏ Seat.

Nếu thêm thất bại, bên gọi vẫn sở hữu con trỏ.

---

## 11. Hàm update()

```cpp
bool SeatRepository::update(const Seat& newValues)
```

### Chức năng

Cập nhật loại ghế của một Seat đã tồn tại.

### Bước 1: Kiểm tra Repository và dữ liệu

```cpp
if (!repo.isLoaded() || !isValidSeat(&newValues))
{
    return false;
}
```

### Bước 2: Tìm Seat

```cpp
Seat* seat = findById(
    newValues.getRoomId(),
    newValues.getSeatId()
);
```

### Bước 3: Không cho thay đổi vị trí

```cpp
if (seat->getRow() != newValues.getRow() ||
    seat->getColumn() != newValues.getColumn())
{
    return false;
}
```

Không cho phép thay đổi hàng hoặc cột qua hàm update.

Mã phòng và mã ghế cũng không được thay đổi vì chúng được dùng làm khóa tìm kiếm.

### Bước 4: Cập nhật loại ghế

```cpp
seat->setSeatType(newValues.getSeatType());
return true;
```

### Kết quả

Hàm hiện tại chỉ cập nhật `SeatType`.

Không tạo Seat mới hoặc thay đổi vị trí ghế.

---

## 12. Hàm removeById()

```cpp
bool SeatRepository::removeById(
    const string& roomId,
    const string& seatId
)
```

### Chức năng

Xóa một Seat theo khóa ghép `roomId|seatId`.

### Bước 1: Kiểm tra Repository

```cpp
if (!repo.isLoaded())
{
    return false;
}
```

### Bước 2: Tìm Seat

```cpp
Seat* seat = findById(roomId, seatId);

if (seat == nullptr)
{
    return false;
}
```

### Bước 3: Lấy Room liên kết

```cpp
Room* room = seat->getRoom();

if (room == nullptr)
{
    return false;
}
```

### Bước 4: Tháo liên kết

```cpp
if (!room->unlinkSeat(seat))
{
    return false;
}
```

Phải tháo liên kết trước khi xóa đối tượng Seat.

Nếu không, Room có thể giữ con trỏ đến vùng nhớ đã được giải phóng.

### Bước 5: Xóa Seat

```cpp
return repo.removeById(roomId + "|" + seatId);
```

Repository chung xóa phần tử và giải phóng đối tượng Seat.

---

## 13. Các hàm truy xuất

### 13.1. findById()

```cpp
Seat* findById(
    const string& roomId,
    const string& seatId
);
```

Tìm ghế bằng khóa ghép.

```cpp
return repo.findById(roomId + "|" + seatId);
```

### 13.2. getAt()

```cpp
Seat* getAt(int index);
```

Trả về Seat theo vị trí trong danh sách.

### 13.3. count()

```cpp
int count();
```

Trả về tổng số ghế của toàn bộ các phòng đang được quản lý.

Không phải số ghế của một Room cụ thể.

### 13.4. isLoaded()

```cpp
bool isLoaded();
```

Trả về trạng thái đã load của SeatRepository.

---

## 14. Quản lý bộ nhớ và vòng đời đối tượng

`Repository<Seat>` sở hữu các đối tượng Seat được thêm thành công.

`Room` chỉ giữ con trỏ liên kết đến Seat, không sở hữu bộ nhớ của Seat.

```text
SeatRepository
      |
      v
Repository<Seat>
      |
      v
     Seat
      ^
      |
     Room
```

Vì vậy, trước khi giải phóng Seat, cần tháo liên kết khỏi Room.

Hàm `removeById()` đã thực hiện quy tắc này.

**Lưu ý về destructor:** Trong phiên bản `SeatRepository.h` trên GitHub được đối chiếu khi viết tài liệu, destructor riêng chưa xuất hiện. Nếu đã bổ sung destructor ở máy cục bộ, cần đồng bộ phần tài liệu này sau khi push.

Destructor riêng được đề xuất có nhiệm vụ tháo các liên kết Room–Seat trước khi `Repository<Seat>` giải phóng các Seat.

---

## 15. Các ràng buộc nghiệp vụ chưa tích hợp

Khi hệ thống có Ticket và Showtime, cần xem xét bổ sung:

- Không xóa ghế đã được vé tham chiếu.
- Không thay đổi loại ghế nếu ảnh hưởng đến dữ liệu vé hoặc giá vé đã bán.
- Không thay đổi sơ đồ ghế khi đã có suất chiếu và vé chịu ảnh hưởng.

Những điều kiện này chưa được triển khai trong `SeatRepository` hiện tại.

Không được coi chúng là những chức năng đã hoàn thành.

---

## 16. Kiểm thử

File:

```text
tests/test_seat_repository.cpp
```

Các nhóm kiểm thử:

- Thêm, sửa, xóa, tìm kiếm Seat.
- Load và save.
- Kiểm tra Room ID và Seat ID.
- Kiểm tra loại ghế.
- Không cho trùng khóa ghép.
- Không cho trùng vị trí trong cùng Room.
- Không cho ghế nằm ngoài kích thước phòng.
- Không load Seat trước Room.
- Rollback khi liên kết thất bại.
- Kiểm tra file TXT không hợp lệ.
- Kiểm tra tháo liên kết khi xóa.

---

## 17. Kết luận

`SeatRepository` quản lý dữ liệu Seat và bảo đảm liên kết với Room trong các thao tác load, add và remove.

Điểm khác biệt quan trọng so với các Repository khác là sử dụng khóa ghép và cần xử lý đồng bộ giữa danh sách Seat và các liên kết trong Room.

Các nghiệp vụ liên quan đến vé và suất chiếu sẽ được bổ sung trong những bước tích hợp tiếp theo.