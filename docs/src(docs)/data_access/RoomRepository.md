# RoomRepository

## 1. Tổng quan

`RoomRepository` thuộc tầng Data Access, quản lý dữ liệu phòng chiếu trong PBL2. Lớp triển khai hợp đồng `IRepository<Room>` và chứa `Repository<Room>` theo composition để quản lý danh sách `Room*`, load/save TXT và các thao tác CRUD chung.

Trách nhiệm riêng: kiểm tra Room hợp lệ, chuyển đổi Room ↔ TXT, chặn thu nhỏ phòng làm ghế vượt giới hạn, và chặn xóa phòng còn ghế hoặc suất chiếu. Repository **không kiểm tra Permission**; việc đó thuộc tầng Business. Đợt này **không sửa `RoomService`**.

**File liên quan:** `src/data_access/IRepository.h`, `Repository.h`, `RoomRepository.h`, `RoomRepository.cpp`, `src/models/Room.h`, `src/models/Seat.h`, `src/utils/Validator.h`.

**Trạng thái:** Thiết kế dự kiến — chờ **CHỐT MARKDOWN**; chưa sửa mã nguồn hoặc kiểm thử.

## 2. Kiến trúc

### 2.1. Quan hệ giữa các lớp

```text
RoomService (hiện tại vẫn dùng RoomRepository&)
    |
    v
RoomRepository ---------> IRepository<Room>
    |                       (public inheritance)
    | composition
    v
Repository<Room>
    |
    v
DoublyLinkedList<Room*>
```

### 2.2. Quan hệ với IRepository

`RoomRepository : public IRepository<Room>` triển khai đủ 9 hàm thuần ảo: `load`, `save`, `add`, `update`, `removeById`, `findById`, `getAt`, `count`, `isLoaded`. Từ khóa `override` giúp trình biên dịch kiểm tra chữ ký. **Không thay `RoomService` sang interface trong phạm vi này**; chỉ chuẩn bị khả năng đó cho giai đoạn sau.

### 2.3. Quan hệ với Repository

`Repository<Room> repo` chịu trách nhiệm lưu `Room*`, tra ID, thêm/xóa, đọc/ghi TXT, quản lý trạng thái loaded và quyền sở hữu đối tượng. Các callback `getRoomId`, `stringToRoom`, `roomToString` cung cấp hành vi đặc thù cho generic Repository.

## 3. Định dạng dữ liệu

Mỗi dòng `rooms.txt` có đúng **5 trường**, phân cách bằng `|`, không có header:

```text
roomId|name|rowCount|columnCount|active
ROM0001|Phong 1|8|12|1
```

| STT | Trường | Kiểu | Quy tắc |
|---|---|---|---|
| 1 | roomId | string | ID hợp lệ với tiền tố `ROM` |
| 2 | name | string | `Validator::isValidText()` |
| 3 | rowCount | int | Lớn hơn 0 |
| 4 | columnCount | int | Lớn hơn 0 |
| 5 | active | bool | Chỉ nhận `0` hoặc `1` trong TXT |

Không thêm trường, đổi tiền tố ID hay thay schema. Ghế nằm ở `seats.txt`, suất chiếu ở `showtimes.txt`, không ghi lặp vào `rooms.txt`.

## 4. Thiết kế lớp

### 4.1. Thuộc tính

| Thuộc tính | Kiểu | Phạm vi | Chức năng |
|---|---|---|---|
| `repo` | `Repository<Room>` | private | Quản lý danh sách Room và lưu trữ |

### 4.2. Các hàm private

| Hàm | Trả về | Chức năng |
|---|---|---|
| `getRoomId(Room*)` | `string` | Callback lấy ID |
| `stringToRoom(const string&)` | `Room*` | Parse một dòng TXT |
| `roomToString(Room*)` | `string` | Serialize một Room |
| `isValidRoom(const Room*)` | `bool` | Kiểm tra dữ liệu Room |

Tất cả là `static`, vì không cần truy cập thành viên `repo` của một instance.

### 4.3. Các hàm public

| Hàm | Trả về | Override |
|---|---|---|
| `RoomRepository(const string& filePath)` | Constructor | Không |
| `load()` / `save()` | `bool` | Có |
| `add(Room*)` / `update(const Room&)` / `removeById(const string&)` | `bool` | Có |
| `findById(const string&)` / `getAt(int)` | `Room*` | Có |
| `count()` | `int` | Có |
| `isLoaded()` | `bool` | Có |

## 5. Mã nguồn dự kiến

### 5.1. RoomRepository.h

**Đường dẫn:** `src/data_access/RoomRepository.h`.

```cpp
#ifndef ROOM_REPOSITORY_H
#define ROOM_REPOSITORY_H

#include <string>
#include "IRepository.h"
#include "Repository.h"
#include "../models/Room.h"

class RoomRepository : public IRepository<Room>
{
private:
    Repository<Room> repo;

    static std::string getRoomId(Room* room);
    static Room* stringToRoom(const std::string& line);
    static std::string roomToString(Room* room);
    static bool isValidRoom(const Room* room);

public:
    RoomRepository(const std::string& filePath);

    bool load() override;
    bool save() override;

    bool add(Room* room) override;
    bool update(const Room& newValues) override;
    bool removeById(const std::string& roomId) override;

    Room* findById(const std::string& roomId) override;
    Room* getAt(int index) override;
    int count() override;
    bool isLoaded() override;
};

#endif
```

### 5.2. RoomRepository.cpp

**Đường dẫn:** `src/data_access/RoomRepository.cpp`.

Logic giữ nguyên; mã dưới đây chuẩn hóa khoảng trắng/comment khi trình bày. Theo quy trình Markdown-first, nếu CHỐT bản này thì nội dung code triển khai phải khớp bản chốt 100%; phương án thay đổi tối thiểu là giữ nguyên file `.cpp` hiện hành.

```cpp
#include "RoomRepository.h"
#include "../utils/Validator.h"
#include "../models/Seat.h"
#include <stdexcept>
#include <vector>

using namespace std;

RoomRepository::RoomRepository(const string& filePath)
    : repo(filePath, getRoomId, stringToRoom, roomToString)
{}

string RoomRepository::getRoomId(Room* room)
{
    if (room == nullptr)
    {
        return "";
    }
    return room->getRoomId();
}

bool RoomRepository::isValidRoom(const Room* room)
{
    if (room == nullptr)
    {
        return false;
    }
    if (!Validator::isValidId(room->getRoomId(), "ROM"))
    {
        return false;
    }
    if (!Validator::isValidText(room->getName()))
    {
        return false;
    }
    if (room->getRowCount() <= 0)
    {
        return false;
    }
    if (room->getColumnCount() <= 0)
    {
        return false;
    }
    return true;
}

Room* RoomRepository::stringToRoom(const string& line)
{
    vector<string> fields;
    size_t start = 0;
    while (true)
    {
        size_t pos = line.find('|', start);
        if (pos == string::npos)
        {
            fields.push_back(line.substr(start));
            break;
        }
        fields.push_back(line.substr(start, pos - start));
        start = pos + 1;
    }
    if (fields.size() != 5)
    {
        throw runtime_error("Sai so luong truong Room");
    }
    int rowCount;
    int columnCount;
    try
    {
        size_t pos1 = 0;
        size_t pos2 = 0;

        rowCount = stoi(fields[2], &pos1);
        columnCount = stoi(fields[3], &pos2);

        if (pos1 != fields[2].length() || pos2 != fields[3].length())
        {
            throw runtime_error("Sai dinh dang so");
        }
    }
    catch (const exception&)
    {
        throw runtime_error("So hang hoac so cot khong hop le");
    }
    if (fields[4] != "0" && fields[4] != "1")
    {
        throw runtime_error("Trang thai Room khong hop le");
    }
    bool active = (fields[4] == "1");

    Room* room = new Room(
        fields[0],
        fields[1],
        rowCount,
        columnCount,
        active
    );

    if (!isValidRoom(room))
    {
        delete room;
        throw runtime_error("Du lieu Room khong hop le");
    }
    return room;
}

string RoomRepository::roomToString(Room* room)
{
    if (!isValidRoom(room))
    {
        throw runtime_error("Room khong hop le khi ghi file");
    }
    return room->getRoomId() + "|" +
           room->getName() + "|" +
           to_string(room->getRowCount()) + "|" +
           to_string(room->getColumnCount()) + "|" +
           (room->isActive() ? "1" : "0");
}

bool RoomRepository::save()
{
    return repo.save();
}

bool RoomRepository::load()
{
    if (repo.isLoaded())
    {
        return false;
    }
    return repo.load();
}

bool RoomRepository::add(Room* room)
{
    if (!repo.isLoaded() || !isValidRoom(room))
    {
        return false;
    }
    for (int i = 0; i < repo.count(); i++)
    {
        if (repo.getAt(i) == room)
        {
            return false;
        }
    }
    return repo.add(room);
}

bool RoomRepository::update(const Room& newValues)
{
    if (!repo.isLoaded() || !isValidRoom(&newValues))
    {
        return false;
    }
    Room* room = repo.findById(newValues.getRoomId());

    if (room == nullptr)
    {
        return false;
    }
    for (int i = 0; i < room->getSeatCount(); i++)
    {
        Seat* seat = room->getSeatAt(i);
        if (seat == nullptr)
        {
            return false;
        }
        if (seat->getRow() > newValues.getRowCount() || seat->getColumn() > newValues.getColumnCount())
        {
            return false;
        }
    }
    room->setName(newValues.getName());
    room->setRowCount(newValues.getRowCount());
    room->setColumnCount(newValues.getColumnCount());
    room->setActive(newValues.isActive());
    return true;
}

bool RoomRepository::removeById(const string& roomId)
{
    if (!repo.isLoaded())
    {
        return false;
    }
    Room* room = repo.findById(roomId);

    if (room == nullptr)
    {
        return false;
    }
    if (room->getSeatCount() > 0 ||
        room->getShowtimeCount() > 0)
    {
        return false;
    }
    return repo.removeById(roomId);
}

Room* RoomRepository::findById(const string& roomId)
{
    return repo.findById(roomId);
}
Room* RoomRepository::getAt(int index)
{
    return repo.getAt(index);
}
int RoomRepository::count()
{
    return repo.count();
}
bool RoomRepository::isLoaded()
{
    return repo.isLoaded();
}
```

## 6. Giải thích các thư viện và từ khóa

### 6.1. `string`

`#include <string>` cung cấp `std::string`. `find('|', start)` tìm dấu phân cách; `substr()` cắt trường; `length()` kiểm tra số ký tự đã được `stoi` tiêu thụ; `to_string()` chuyển số nguyên khi serialize.

### 6.2. `vector`

`#include <vector>` dùng `vector<string> fields` để giữ tạm các trường của **một dòng** TXT. `push_back` thêm trường; `size` xác nhận đúng 5 trường. Danh sách phòng chính vẫn là `DoublyLinkedList<Room*>` trong generic Repository.

### 6.3. `stdexcept`

`#include <stdexcept>` cung cấp `runtime_error` để báo lỗi parse/serialize. `try/catch (const exception&)` chuyển lỗi `stoi` hoặc hậu tố không hợp lệ thành thông báo phù hợp. Lỗi từ generic `Repository<T>::load()` có thể kèm tên file và dòng.

### 6.4. `static`

Các helper `static` không có `this`; có thể truyền trực tiếp dưới dạng callback cho constructor `Repository<Room>`.

### 6.5. `override`

`override` đánh dấu hàm triển khai phương thức ảo trong `IRepository<Room>`; trình biên dịch phát hiện chữ ký sai. Không đổi hành vi runtime của các hàm hiện tại.

### 6.6. Public inheritance

`class RoomRepository : public IRepository<Room>` tạo quan hệ is-a: có thể tham chiếu một `RoomRepository` qua `IRepository<Room>&`. `IRepository` có destructor ảo; `Repository<Room>` vẫn được chứa theo composition, không kế thừa.

## 7. Giải thích các hàm private

### 7.1. `isValidRoom(const Room* room)`

- **Mục đích:** Kiểm tra con trỏ, ID `ROM`, tên và kích thước dương; trả `bool`.
- **Luồng hoạt động chi tiết (ASCII):**

  ```text
  `null? -> false; ID sai? -> false; tên sai? -> false; hàng <=0? -> false; cột <=0? -> false; -> true`.
  ```

- **Giải thích:** `const Room*` không cho thay đổi đối tượng qua tham số; `Validator::isValidId` xác minh định dạng; `Validator::isValidText` xác minh văn bản; các getter kiểm tra giới hạn. Không kiểm tra quan hệ Seat/Showtime ở helper này.

**Giải thích bổ sung theo đoạn code:** Đoạn `if (room == nullptr)` bảo vệ mọi lần gọi getter sau đó. `Validator::isValidId(..., "ROM")` kiểm tra mã; `isValidText` kiểm tra tên. Hai điều kiện `<= 0` loại kích thước không dương. `return true` chỉ đến sau khi toàn bộ kiểm tra thành công.

### 7.2. `getRoomId(Room* room)`

- **Mục đích:** Trích ID cho `Repository<Room>`; trả chuỗi rỗng nếu null.
- **Luồng hoạt động chi tiết (ASCII):**

  ```text
  `room == nullptr? --có--> return ""; --không--> return room->getRoomId()`.
  ```

- **Giải thích:** `nullptr` ngăn dereference sai; `->` truy cập getter qua con trỏ; trả về bản sao `string`.

**Giải thích bổ sung theo đoạn code:** `return ""` là giá trị bảo vệ nếu con trỏ null; trường hợp hợp lệ trả về bản sao mã từ getter, không thay đổi Room.

### 7.3. `stringToRoom(const string& line)`

- **Mục đích:** Parse đúng 5 trường `roomId|name|rowCount|columnCount|active`, trả `Room*` mới hoặc ném `runtime_error`.
- **Luồng hoạt động chi tiết (ASCII):**

  ```text
  `line -> tách bằng '|' -> đủ 5? --không--> throw -> stoi cả 2 số + kiểm tra đọc hết? --không--> throw -> active 0/1? --không--> throw -> new Room -> isValidRoom? --không--> delete + throw; --có--> return room`.
  ```

- **Giải thích:** `vector<string> fields` giữ các trường; `start`, `find`, `substr` duyệt từ trái qua phải và giữ trường rỗng cuối. `stoi` dùng `pos1/pos2` để chặn chuỗi số có hậu tố; `try/catch` chuyển lỗi thành thông báo Room. Chỉ chấp nhận `"0"` hoặc `"1"` và chuyển thành `bool`. `new Room` cấp phát heap; nếu kiểm tra thất bại phải `delete` trước khi throw; khi thành công chuyển con trỏ cho caller (`Repository<T>::load`).

**Giải thích bổ sung theo đoạn code:** `start = pos + 1` dịch vị trí sau dấu `|`; khi `find` trả `npos`, `substr(start)` giữ cả trường cuối. `stoi(fields[2], &pos1)` và `stoi(fields[3], &pos2)` cần tiêu thụ hết chuỗi, nên các hậu tố như `8abc` bị loại. Sau `new Room`, `delete room` ở nhánh invalid tránh rò rỉ. Nếu hợp lệ, trả con trỏ cho bên gọi quản lý.

### 7.4. `roomToString(Room* room)`

- **Mục đích:** Serialize Room hợp lệ thành một dòng TXT; ném lỗi nếu không hợp lệ.
- **Luồng hoạt động chi tiết (ASCII):**

  ```text
  `isValidRoom? --không--> throw; --có--> nối ID|tên|hàng|cột|active -> return string`.
  ```

- **Giải thích:** `to_string` chuyển số nguyên; toán tử `+` nối trường với `|`; toán tử ba ngôi chuyển bool thành `"1"`/`"0"`. Không ghi file trực tiếp.

**Giải thích bổ sung theo đoạn code:** Trước tiên `isValidRoom` chặn serialize đối tượng lỗi. Biểu thức nối dùng đúng thứ tự năm trường; `to_string` đổi số hàng/cột; `(room->isActive() ? "1" : "0")` chuẩn hóa boolean.

## 8. Giải thích các hàm public

### 8.1. Constructor `RoomRepository(filePath)`

- **Mục đích:** Khởi tạo `repo` với đường dẫn TXT và ba callback static; không tự load.
- **Tham số:** `filePath` tham chiếu chuỗi; không trả về.
- **Luồng hoạt động chi tiết (ASCII):**

  ```text
  `filePath -> repo(filePath, getRoomId, stringToRoom, roomToString) -> kết thúc`.
  ```

- **Giải thích:** Danh sách khởi tạo sau dấu `:` xây dựng thành viên `repo` trước thân constructor `{}`. Callback dùng khi tra ID, parse và serialize; chưa cấp phát `Room`.

**Giải thích bổ sung theo đoạn code:** Danh sách khởi tạo `: repo(...)` chạy trước thân hàm rỗng. Không gọi `load`, không tạo Room. Các tên hàm static được truyền như con trỏ hàm, để generic Repository dùng khi cần.

### 8.2. `load()`

- **Mục đích:** Nạp TXT một lần; trả `false` nếu đã load hoặc `repo.load()` thất bại.
- **Luồng hoạt động chi tiết (ASCII):**

  ```text
  `repo.isLoaded()? --có--> false; --không--> repo.load() -> trả kết quả`.
  ```

- **Giải thích:** Điều kiện chặn nạp lặp. `Repository<T>::load()` đọc file, parse vào danh sách tạm, kiểm tra ID trùng, rồi swap; lỗi parse có thể ném exception kèm file/dòng. Không tự tạo file thiếu.

**Giải thích bổ sung theo đoạn code:** `if (repo.isLoaded()) return false` chặn nạp lần hai. `repo.load()` đọc toàn file vào danh sách tạm, parse từng dòng, từ chối ID trùng, và chỉ đổi danh sách chính sau khi thành công; exception parse được truyền ra.

### 8.3. `save()`

- **Mục đích:** Ghi danh sách Room vào TXT; trả kết quả `repo.save()`.
- **Luồng hoạt động chi tiết (ASCII):**

  ```text
  `repo.save() -> return bool`.
  ```

- **Giải thích:** `Repository<T>::save()` gọi callback `roomToString` cho từng đối tượng rồi `FileIO::writeLines`; không phải transaction nguyên tử và có thể ném exception nếu dữ liệu sai.

**Giải thích bổ sung theo đoạn code:** `return repo.save()` chuyển nguyên kết quả ghi file; generic Repository serialize từng Room rồi gọi `FileIO::writeLines`. Không tự gọi save sau add/update/remove.

### 8.4. `add(Room* room)`

- **Mục đích:** Thêm Room mới nếu repo đã load, dữ liệu hợp lệ và không trùng con trỏ/ID.
- **Luồng hoạt động chi tiết (ASCII):**

  ```text
  `loaded + valid? --không--> false; duyệt từng item -> cùng địa chỉ? --có--> false; -> repo.add(room) -> bool`.
  ```

- **Giải thích:** `!`, `||` từ chối khi chưa load hoặc Room không hợp lệ. Vòng `for` so sánh địa chỉ con trỏ, không so nội dung. `repo.add` tiếp tục kiểm tra ID rỗng/trùng, đưa vào danh sách. Thành công thì repo sở hữu `room`; thất bại caller vẫn sở hữu.

**Giải thích bổ sung theo đoạn code:** `!repo.isLoaded() || !isValidRoom(room)` bảo vệ trạng thái và dữ liệu. Vòng `for` so sánh chính địa chỉ con trỏ với từng phần tử đã quản lý; `repo.add(room)` còn kiểm tra ID rỗng/trùng trước khi nhận ownership.

### 8.5. `update(const Room& newValues)`

- **Mục đích:** Cập nhật thuộc tính của phòng có ID tương ứng, không để ghế vượt kích thước mới.
- **Luồng hoạt động chi tiết (ASCII):**

  ```text
  `loaded + valid? --không--> false -> findById -> tồn tại? --không--> false -> duyệt ghế -> null/vượt giới hạn? --có--> false -> setName/setRowCount/setColumnCount/setActive -> true`.
  ```

- **Giải thích:** `&newValues` lấy địa chỉ đối tượng tham chiếu để validate; tìm theo ID bất biến. `getSeatCount` và `getSeatAt` duyệt các liên kết ghế; nếu ghế nằm ngoài hàng/cột mới thì từ chối trước khi gọi setter. Các setter chỉ chạy sau khi toàn bộ kiểm tra qua; hàm không save. Việc Service ngăn đổi `active` hoặc đổi kích thước khi đã có suất chiếu vẫn nằm ở Service hiện tại, không thay đổi trong đợt này.

**Giải thích bổ sung theo đoạn code:** `&newValues` lấy địa chỉ tham chiếu để validate mà không tạo bản sao. `repo.findById` tìm phòng đang quản lý. Vòng lặp đọc từng `Seat*`; null hoặc tọa độ vượt số hàng/cột mới đều khiến hàm trả false **trước** các setter. Các setter chỉ thay thuộc tính, không thay ID hay danh sách liên kết.

### 8.6. `removeById(const string& roomId)`

- **Mục đích:** Xóa phòng chưa được ghế hoặc suất chiếu tham chiếu.
- **Luồng hoạt động chi tiết (ASCII):**

  ```text
  `loaded? --không--> false -> findById -> null? --có--> false -> có ghế/suất? --có--> false -> repo.removeById(roomId)`.
  ```

- **Giải thích:** `getSeatCount`/`getShowtimeCount` chặn xóa đối tượng còn liên kết; `Repository<T>::removeById` gỡ node và `delete Room*` khi thành công. Con trỏ mượn đến Room bị xóa sẽ không còn hợp lệ.

**Giải thích bổ sung theo đoạn code:** Kiểm tra loaded và tồn tại trước. Hai bộ đếm ghế/suất bảo vệ quan hệ đang tham chiếu. Chỉ sau khi cả hai bằng 0, `repo.removeById` gỡ node và giải phóng Room; không lưu TXT tự động.

### 8.7. `findById(const string& roomId)`

- **Mục đích:** Trả con trỏ Room theo ID hoặc `nullptr`.
- **Luồng hoạt động chi tiết (ASCII):**

  ```text
  `roomId -> repo.findById(roomId) -> Room*/nullptr`.
  ```

- **Giải thích:** Chỉ chuyển tiếp cho generic repo; không chuyển quyền sở hữu con trỏ.

**Giải thích bổ sung theo đoạn code:** Chuyển tiếp nguyên `roomId` cho `repo.findById`; kết quả là con trỏ mượn hoặc null. Không cấp phát và không chuyển ownership.

### 8.8. `getAt(int index)`

- **Mục đích:** Lấy con trỏ Room theo vị trí hoặc `nullptr` nếu không có node.
- **Luồng hoạt động chi tiết (ASCII):**

  ```text
  `index -> repo.getAt(index) -> Room*/nullptr`.
  ```

- **Giải thích:** Vị trí do danh sách liên kết quản lý; caller không sở hữu kết quả.

**Giải thích bổ sung theo đoạn code:** Chuyển tiếp `index` cho danh sách generic; chỉ số không hợp lệ dẫn đến `nullptr` theo `Repository<T>::getAt`.

### 8.9. `count()`

- **Mục đích:** Đếm số Room hiện trong danh sách.
- **Luồng hoạt động chi tiết (ASCII):**

  ```text
  `repo.count() -> int`.
  ```

- **Giải thích:** `Repository<T>::count()` trả `items.nodeCount()`; không tự load.

**Giải thích bổ sung theo đoạn code:** Trả số node do generic Repository quản lý; không tính số ghế, số suất hay tổng số chỗ.

### 8.10. `isLoaded()`

- **Mục đích:** Trả trạng thái nạp dữ liệu.
- **Luồng hoạt động chi tiết (ASCII):**

  ```text
  `repo.isLoaded() -> bool`.
  ```

- **Giải thích:** Chuyển tiếp cờ `loaded` của generic repo; không đọc file.

**Giải thích bổ sung theo đoạn code:** Trả cờ loaded của generic Repository; không tự tải file hoặc thay đổi trạng thái.

## 9. Quản lý bộ nhớ

### 9.1. Quyền sở hữu đối tượng

`Repository<Room>` sở hữu `Room*` sau khi `add` thành công hoặc sau khi `load` hoàn tất. `Repository<T>::clear()` và `removeById()` giải phóng các đối tượng được sở hữu. Khi `add()` thất bại, caller vẫn chịu trách nhiệm `delete` con trỏ chưa được chuyển giao.

### 9.2. Con trỏ mượn

`findById()` và `getAt()` trả con trỏ mượn; không `delete` từ caller. Con trỏ mất hiệu lực khi Room bị xóa hoặc Repository bị hủy/clear. `Room` có danh sách liên kết Seat/Showtime nhưng không sở hữu các đối tượng liên kết đó.

### 9.3. Destructor

`IRepository<Room>` có `virtual ~IRepository() = default;`. `RoomRepository` không cần khai báo destructor riêng; destructor mặc định hủy thành viên `repo`, từ đó generic Repository giải phóng các Room mà nó sở hữu.

## 10. Áp dụng Dependency Inversion Principle

### 10.1. Trước khi refactor

`RoomRepository` là lớp concrete độc lập, có đủ các hàm CRUD nhưng chưa khai báo triển khai `IRepository<Room>`. `RoomService` nhận `RoomRepository&`.

### 10.2. Sau khi refactor Repository

`RoomRepository` kế thừa `IRepository<Room>` và ghi `override` cho đủ chín phương thức. Có thể gọi các phương thức chung qua `IRepository<Room>&` khi kiểm thử. **`RoomService` vẫn nhận `RoomRepository&`** trong đợt này; DIP ở tầng Service chưa hoàn tất và không được ghi nhận là đã triển khai.

## 11. Các thay đổi sau refactor

### 11.1. Thay đổi trong RoomRepository.h

- Thêm `#include "IRepository.h"`.
- Đổi thành `class RoomRepository : public IRepository<Room>`.
- Thêm `override` vào chín hàm public tương ứng.

### 11.2. Thay đổi trong RoomRepository.cpp

Không cần đổi logic. Giữ nguyên các callback, parse/serialize, validation, ràng buộc ghế và suất chiếu. Khối mã ở mục 5 là bản trình bày dự kiến; trước khi triển khai cần chốt chính xác bản code chuẩn để đối chiếu 100%.

### 11.3. Các thành phần giữ nguyên

`RoomService`, `IRepository.h`, `Repository.h`, Model `Room`, `Seat`, `Showtime`, Validator, định dạng TXT, quyền `ROOM_*` và quy tắc cấp ID.

## 12. Kế hoạch kiểm thử

### 12.1. Kiểm thử nạp dữ liệu

- [ ] Load file hợp lệ, rỗng, thiếu, sai số trường, ID sai/trùng, số hàng/cột sai, active khác 0/1.
- [ ] Load hai lần; lỗi parse báo file/dòng; dữ liệu không bị nạp dở.

### 12.2. Kiểm thử thêm phòng

- [ ] Thêm Room hợp lệ, chưa load, null, ID trùng, con trỏ trùng, tên/kích thước không hợp lệ.
- [ ] Xác minh quyền sở hữu con trỏ ở cả nhánh thành công và thất bại.

### 12.3. Kiểm thử cập nhật phòng

- [ ] Update thành công, ID không tồn tại, chưa load, dữ liệu invalid.
- [ ] Thu nhỏ làm ghế vượt giới hạn phải thất bại; thu nhỏ vẫn chứa mọi ghế phải được kiểm tra theo quy tắc hiện hành.
- [ ] Giữ nguyên ID, địa chỉ Room và liên kết Seat/Showtime.

### 12.4. Kiểm thử xóa phòng

- [ ] Xóa phòng không còn liên kết; từ chối nếu còn Seat hoặc Showtime.
- [ ] Xóa ID không tồn tại/chưa load; xác minh số lượng và con trỏ sau xóa.

### 12.5. Kiểm thử truy vấn

- [ ] `findById` tồn tại/không tồn tại; `getAt` chỉ số hợp lệ/âm/vượt phạm vi.
- [ ] `count` và `isLoaded` trước/sau load, add, remove.

### 12.6. Kiểm thử lưu dữ liệu

- [ ] Save trước load; save Room hợp lệ; đối chiếu 5 trường và 0/1; load lại để so sánh.
- [ ] Save khi dữ liệu bị thay đổi thành invalid; lỗi ghi file.

### 12.7. Kiểm thử IRepository

- [ ] Biên dịch `RoomRepository` với `IRepository<Room>&` và `IRepository<Room>*`.
- [ ] Gọi đủ 9 phương thức qua interface; kiểm tra destructor ảo.
- [ ] Chạy lại các test RoomRepository liên quan, không sửa test để tạo PASS.

## 13. Tiến độ triển khai

- [x] Đọc `AGENTS.md`, `IRepository.h`, `MovieRepository.h/.cpp` và `MovieRepository.md`.
- [x] Đọc `RoomRepository.h/.cpp`, `RoomService.h/.cpp`, `Room.h/.cpp` và generic `Repository.h`.
- [x] Xác định phạm vi chỉ refactor Repository.
- [x] Viết bản thiết kế Markdown theo 14 mục của mẫu MovieRepository.
- [ ] Review và **CHỐT MARKDOWN**.
- [ ] Đối chiếu code triển khai khớp Markdown đã chốt.
- [ ] Biên dịch và kiểm thử thực tế.
- [ ] Cập nhật tiến độ sau kiểm thử.

## 14. Kết luận

`RoomRepository` sẽ triển khai `IRepository<Room>` bằng public inheritance, giữ `Repository<Room>` theo composition và bảo toàn toàn bộ nghiệp vụ Room hiện có. Chỉ cần thay đổi phần khai báo header; `.cpp` không cần thay đổi logic. `RoomService` không thuộc phạm vi công việc hiện tại. Bản này **chưa được chốt, chưa triển khai và chưa kiểm thử**.
