# RoomRepository – Quản lý dữ liệu phòng chiếu

## 1. Tổng quan

`RoomRepository` là lớp thuộc tầng Data Access, chịu trách nhiệm quản lý dữ liệu phòng chiếu trong hệ thống quản lý rạp phim.

Lớp sử dụng `Repository<Room>` để tái sử dụng các chức năng chung như đọc file, ghi file, thêm, xóa, tìm kiếm và quản lý bộ nhớ.

Ngoài ra, `RoomRepository` bổ sung các quy tắc riêng của phòng chiếu, chẳng hạn kiểm tra mã phòng, tên phòng, số hàng, số cột và điều kiện thay đổi kích thước hoặc xóa phòng.

Các chức năng chính:

- Đọc và ghi dữ liệu phòng chiếu từ file TXT.
- Kiểm tra tính hợp lệ của dữ liệu Room.
- Chuyển đổi giữa chuỗi TXT và đối tượng Room.
- Thêm phòng chiếu mới.
- Cập nhật thông tin phòng chiếu.
- Xóa phòng khi đáp ứng điều kiện.
- Tìm kiếm và truy xuất danh sách phòng chiếu.

**Phân chia trách nhiệm:**

- `Repository<Room>`: Quản lý danh sách đối tượng, đọc/ghi file và giải phóng bộ nhớ.
- `RoomRepository`: Kiểm tra dữ liệu và các ràng buộc trực tiếp của Room.
- `RoomService`: Kiểm tra phân quyền và các quy tắc nghiệp vụ ở tầng Business.

**File liên quan:**

- `src/data_access/RoomRepository.h`
- `src/data_access/RoomRepository.cpp`
- `src/data_access/Repository.h`
- `src/models/Room.h`
- `src/models/Room.cpp`
- `src/utils/Validator.h`

---

## 2. Cấu trúc lớp RoomRepository

### 2.1. Thuộc tính

```cpp
private:
    Repository<Room> repo;
```

`repo` là đối tượng của lớp `Repository<T>` chung, với kiểu dữ liệu cụ thể là `Room`.

Đối tượng này quản lý danh sách `Room*` trong bộ nhớ.

Khi một Room được thêm thành công, `Repository<Room>` sở hữu con trỏ đó và chịu trách nhiệm giải phóng bộ nhớ khi xóa hoặc hủy Repository.

`RoomRepository` không sử dụng kế thừa từ `Repository<Room>` mà sử dụng một đối tượng thành viên để tái sử dụng các chức năng chung.

### 2.2. Các hàm private

```cpp
static string getRoomId(Room* room);
static Room* stringToRoom(const string& line);
static string roomToString(Room* room);
static bool isValidRoom(const Room* room);
```

Các hàm này hỗ trợ lấy khóa định danh, chuyển đổi dữ liệu và kiểm tra tính hợp lệ của Room.

Chúng được khai báo `static` vì không cần truy cập trực tiếp vào thuộc tính `repo` của đối tượng RoomRepository.

---

## 3. Constructor – Khởi tạo RoomRepository

```cpp
RoomRepository::RoomRepository(const string& filePath)
    : repo(filePath, getRoomId, stringToRoom, roomToString)
{}
```

**Chức năng:** Khởi tạo `Repository<Room>` và cung cấp các hàm xử lý dữ liệu Room.

Constructor truyền bốn tham số:

- `filePath`: Đường dẫn file dữ liệu phòng chiếu.
- `getRoomId`: Hàm lấy mã phòng.
- `stringToRoom`: Hàm chuyển một dòng TXT thành đối tượng Room.
- `roomToString`: Hàm chuyển Room thành chuỗi TXT.

Ví dụ:

```cpp
RoomRepository roomRepo("data/rooms.txt");
```

Constructor chỉ khởi tạo Repository, chưa đọc dữ liệu từ file.

Muốn sử dụng dữ liệu cần gọi `load()`.

---

## 4. Hàm getRoomId()

```cpp
string RoomRepository::getRoomId(Room* room)
{
    if (room == nullptr)
    {
        return "";
    }
    return room->getRoomId();
}
```

**Chức năng:** Lấy mã định danh của một phòng chiếu.

Nếu con trỏ Room bằng `nullptr`, hàm trả về chuỗi rỗng.

Ngược lại, hàm gọi `getRoomId()` của Model.

Hàm được truyền cho `Repository<Room>` để Repository chung có thể tìm kiếm và kiểm tra trùng ID mà không cần biết cấu trúc cụ thể của Room.

---

## 5. Hàm isValidRoom()

```cpp
bool RoomRepository::isValidRoom(const Room* room)
```

### Chức năng

Kiểm tra tính hợp lệ của một đối tượng Room trước khi thêm, cập nhật hoặc ghi dữ liệu.

### Bước 1: Kiểm tra con trỏ

```cpp
if (room == nullptr)
{
    return false;
}
```

Không thể kiểm tra dữ liệu nếu đối tượng không tồn tại.

### Bước 2: Kiểm tra mã phòng

```cpp
if (!Validator::isValidId(room->getRoomId(), "ROM"))
{
    return false;
}
```

Mã phòng phải đúng định dạng ID của loại Room.

Ví dụ:

- `ROM0001`: Hợp lệ.
- `ROM0123`: Hợp lệ.
- `MOV0001`: Không hợp lệ.
- `ROM0000`: Không hợp lệ.

Việc kiểm tra định dạng được sử dụng lại từ lớp `Validator`.

### Bước 3: Kiểm tra tên phòng

```cpp
if (!Validator::isValidText(room->getName()))
{
    return false;
}
```

Tên phòng phải là văn bản hợp lệ theo quy tắc của `Validator`.

### Bước 4: Kiểm tra số hàng

```cpp
if (room->getRowCount() <= 0)
{
    return false;
}
```

Số hàng phải lớn hơn 0.

### Bước 5: Kiểm tra số cột

```cpp
if (room->getColumnCount() <= 0)
{
    return false;
}
```

Số cột phải lớn hơn 0.

### Kết quả

Trả về `true` nếu tất cả điều kiện hợp lệ.

Ngược lại trả về `false`.

**Lưu ý:** Hàm không kiểm tra trùng ID với những Room khác. Việc kiểm tra trùng ID do `Repository<Room>` thực hiện.

---

## 6. Hàm stringToRoom()

```cpp
Room* RoomRepository::stringToRoom(const string& line)
```

### Chức năng

Chuyển một dòng dữ liệu TXT thành đối tượng Room.

Định dạng:

```text
roomId|name|rowCount|columnCount|active
```

Ví dụ:

```text
ROM0001|Phong 1|8|12|1
```

### Bước 1: Tách chuỗi

Hàm sử dụng `vector<string>` để lưu các trường dữ liệu được phân tách bằng dấu `|`.

```cpp
vector<string> fields;
size_t start = 0;
```

Sau khi tách, dữ liệu phải có đúng 5 trường.

```cpp
if (fields.size() != 5)
{
    throw runtime_error("Sai so luong truong Room");
}
```

### Bước 2: Chuyển đổi số hàng và số cột

```cpp
rowCount = stoi(fields[2], &pos1);
columnCount = stoi(fields[3], &pos2);
```

`stoi()` chuyển chuỗi thành số nguyên.

Hai biến `pos1` và `pos2` được sử dụng để kiểm tra toàn bộ chuỗi đã được chuyển đổi hay chưa.

Ví dụ:

- `"8"`: Hợp lệ.
- `"12"`: Hợp lệ.
- `"8abc"`: Không hợp lệ.
- `"abc"`: Không hợp lệ.

Nếu chuyển đổi thất bại, hàm ném `runtime_error`.

### Bước 3: Kiểm tra trạng thái

```cpp
if (fields[4] != "0" && fields[4] != "1")
{
    throw runtime_error("Trang thai Room khong hop le");
}
```

Quy ước:

- `1`: Phòng đang hoạt động.
- `0`: Phòng ngừng hoạt động.

### Bước 4: Tạo đối tượng Room

```cpp
Room* room = new Room(
    fields[0],
    fields[1],
    rowCount,
    columnCount,
    active
);
```

### Bước 5: Kiểm tra dữ liệu

```cpp
if (!isValidRoom(room))
{
    delete room;
    throw runtime_error("Du lieu Room khong hop le");
}
```

Nếu dữ liệu không hợp lệ, đối tượng tạm được giải phóng trước khi ném ngoại lệ.

### Kết quả

Trả về `Room*` nếu chuyển đổi thành công.

Ném `runtime_error` nếu dòng dữ liệu không hợp lệ.

---

## 7. Hàm roomToString()

```cpp
string RoomRepository::roomToString(Room* room)
```

### Chức năng

Chuyển một đối tượng Room thành chuỗi để ghi xuống file TXT.

Trước tiên, hàm gọi `isValidRoom()`.

```cpp
if (!isValidRoom(room))
{
    throw runtime_error("Room khong hop le khi ghi file");
}
```

Sau đó ghép các trường:

```cpp
return room->getRoomId() + "|" +
       room->getName() + "|" +
       to_string(room->getRowCount()) + "|" +
       to_string(room->getColumnCount()) + "|" +
       (room->isActive() ? "1" : "0");
```

Ví dụ kết quả:

```text
ROM0001|Phong 1|8|12|1
```

Hàm không trực tiếp mở hoặc ghi file.

Việc ghi file được thực hiện bởi `Repository<Room>` thông qua `FileIO`.

---

## 8. Hàm load()

```cpp
bool RoomRepository::load()
{
    if (repo.isLoaded())
    {
        return false;
    }
    return repo.load();
}
```

### Chức năng

Nạp dữ liệu phòng chiếu từ file TXT vào bộ nhớ.

### Quy trình

1. Kiểm tra Repository đã được load hay chưa.
2. Nếu đã load, trả về `false`.
3. Nếu chưa load, gọi `repo.load()`.
4. Repository chung đọc file và sử dụng `stringToRoom()` để tạo đối tượng.

### Kết quả

- `true`: Nạp dữ liệu thành công.
- `false`: Không thể load hoặc Repository đã được load.
- Có thể ném ngoại lệ nếu dữ liệu TXT không hợp lệ.

Việc không cho load nhiều lần giúp tránh thay thế những Room đang được các đối tượng khác liên kết.

---

## 9. Hàm save()

```cpp
bool RoomRepository::save()
{
    return repo.save();
}
```

**Chức năng:** Lưu danh sách phòng chiếu trong bộ nhớ xuống file TXT.

`Repository<Room>` gọi `roomToString()` để chuyển từng đối tượng thành chuỗi, sau đó thực hiện ghi file.

`RoomRepository` không tự thực hiện thao tác I/O.

---

## 10. Hàm add()

```cpp
bool RoomRepository::add(Room* room)
```

### Chức năng

Thêm một phòng chiếu mới vào Repository.

### Bước 1: Kiểm tra trạng thái và dữ liệu

```cpp
if (!repo.isLoaded() || !isValidRoom(room))
{
    return false;
}
```

Không cho phép thêm nếu Repository chưa load hoặc dữ liệu không hợp lệ.

### Bước 2: Kiểm tra trùng con trỏ

```cpp
for (int i = 0; i < repo.count(); i++)
{
    if (repo.getAt(i) == room)
    {
        return false;
    }
}
```

Không cho phép thêm cùng một con trỏ Room nhiều lần.

Quy tắc này giúp tránh một đối tượng bị quản lý và giải phóng nhiều lần.

### Bước 3: Thêm vào Repository chung

```cpp
return repo.add(room);
```

`Repository<Room>` tiếp tục kiểm tra trùng ID và thực hiện thêm.

### Kết quả

- `true`: Thêm thành công.
- `false`: Không đáp ứng điều kiện thêm.

**Quản lý bộ nhớ:** Nếu thêm thành công, Repository sở hữu con trỏ Room. Nếu thêm thất bại, bên gọi vẫn chịu trách nhiệm quản lý con trỏ.

---

## 11. Hàm update()

```cpp
bool RoomRepository::update(const Room& newValues)
```

### Chức năng

Cập nhật thông tin của một phòng chiếu đã tồn tại.

### Bước 1: Kiểm tra dữ liệu

```cpp
if (!repo.isLoaded() || !isValidRoom(&newValues))
{
    return false;
}
```

### Bước 2: Tìm phòng hiện tại

```cpp
Room* room = repo.findById(newValues.getRoomId());

if (room == nullptr)
{
    return false;
}
```

Không tạo Room mới khi cập nhật.

### Bước 3: Kiểm tra các ghế đang liên kết

```cpp
for (int i = 0; i < room->getSeatCount(); i++)
{
    Seat* seat = room->getSeatAt(i);

    if (seat == nullptr)
    {
        return false;
    }

    if (seat->getRow() > newValues.getRowCount() ||
        seat->getColumn() > newValues.getColumnCount())
    {
        return false;
    }
}
```

Nếu kích thước mới khiến bất kỳ ghế hiện tại nào nằm ngoài phạm vi phòng, hàm từ chối cập nhật.

Ví dụ:

Phòng hiện tại có 8 hàng, 12 cột và một ghế tại hàng 8, cột 10.

- Đổi thành 8 hàng, 10 cột: Có thể hợp lệ.
- Đổi thành 7 hàng, 12 cột: Không hợp lệ vì ghế ở hàng 8.

### Bước 4: Cập nhật thuộc tính

```cpp
room->setName(newValues.getName());
room->setRowCount(newValues.getRowCount());
room->setColumnCount(newValues.getColumnCount());
room->setActive(newValues.isActive());
```

Repository cập nhật trực tiếp đối tượng đang được quản lý, không thay thế con trỏ Room.

### Kết quả

Trả về `true` nếu cập nhật thành công, ngược lại `false`.

**Lưu ý:** `RoomRepository::update()` cho phép thay đổi trạng thái `active`. Tuy nhiên, `RoomService::updateRoom()` có quy tắc riêng không cho thay đổi trạng thái qua thao tác cập nhật thông thường.

---

## 12. Hàm removeById()

```cpp
bool RoomRepository::removeById(const string& roomId)
```

### Chức năng

Xóa một phòng chiếu theo ID.

### Bước 1: Kiểm tra Repository

```cpp
if (!repo.isLoaded())
{
    return false;
}
```

### Bước 2: Tìm phòng

```cpp
Room* room = repo.findById(roomId);

if (room == nullptr)
{
    return false;
}
```

### Bước 3: Kiểm tra liên kết

```cpp
if (room->getSeatCount() > 0 ||
    room->getShowtimeCount() > 0)
{
    return false;
}
```

Không cho phép xóa Room khi:

- Phòng vẫn chứa Seat.
- Phòng vẫn liên kết với Showtime.

Điều này tránh việc các đối tượng khác giữ con trỏ đến Room đã bị giải phóng.

### Bước 4: Xóa phòng

```cpp
return repo.removeById(roomId);
```

Repository chung xóa phần tử khỏi danh sách và giải phóng đối tượng Room.

---

## 13. Các hàm truy xuất

### 13.1. findById()

```cpp
Room* findById(const string& roomId);
```

Tìm Room theo mã phòng.

Trả về `nullptr` nếu không tìm thấy.

### 13.2. getAt()

```cpp
Room* getAt(int index);
```

Lấy Room theo vị trí trong danh sách.

Trả về `nullptr` nếu chỉ số không hợp lệ.

### 13.3. count()

```cpp
int count();
```

Trả về tổng số Room đang được quản lý trong bộ nhớ.

### 13.4. isLoaded()

```cpp
bool isLoaded();
```

Cho biết dữ liệu Repository đã được load thành công hay chưa.

---

## 14. Quan hệ với Seat và Showtime

Room có thể liên kết với nhiều Seat và Showtime.

Các đối tượng Seat và Showtime không thuộc quyền sở hữu bộ nhớ của Room.

```text
RoomRepository
      |
      v
Repository<Room>
      |
      v
     Room
    /    \
   v      v
 Seat*  Showtime*
```

Room chỉ lưu các con trỏ liên kết.

Vì vậy, khi xóa hoặc giải phóng Seat/Showtime, cần bảo đảm những liên kết tương ứng được tháo bỏ.

---

## 15. Kiểm thử

File:

```text
tests/test_room_repository.cpp
```

Các nhóm kiểm thử cần bao gồm:

- Load và save dữ liệu Room.
- Thêm phòng hợp lệ và không hợp lệ.
- Kiểm tra trùng ID, trùng con trỏ.
- Cập nhật thông tin phòng.
- Không cho thu nhỏ phòng làm ghế nằm ngoài kích thước.
- Không cho xóa phòng còn Seat hoặc Showtime.
- Kiểm tra file TXT sai định dạng.
- Kiểm tra các hàm truy xuất.

---

## 16. Kết luận

`RoomRepository` đã cung cấp các thao tác dữ liệu cơ bản cho Room và kiểm tra các ràng buộc trực tiếp với Seat, Showtime.

Những quy tắc liên quan đến phân quyền, thay đổi trạng thái phòng và giới hạn thay đổi kích thước khi đã có suất chiếu được xử lý ở `RoomService`.

Các ràng buộc sâu hơn liên quan đến lịch chiếu và vé sẽ được tiếp tục hoàn thiện khi tích hợp các module tương ứng.