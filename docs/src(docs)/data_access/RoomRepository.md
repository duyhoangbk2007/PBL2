# RoomRepository

## 1. Tổng quan

`RoomRepository` thuộc tầng Data Access, quản lý dữ liệu phòng chiếu trong PBL2. Lớp triển khai hợp đồng `IRepository<Room>` và chứa `Repository<Room>` theo composition để quản lý danh sách `Room*`, load/save TXT và các thao tác CRUD chung.

Trách nhiệm riêng: kiểm tra Room hợp lệ, chuyển đổi Room ↔ TXT, chặn thu nhỏ phòng làm ghế vượt giới hạn, và chặn xóa phòng còn ghế hoặc suất chiếu. Repository **không kiểm tra Permission**; việc đó thuộc tầng Business. Đợt này **không sửa `RoomService`**.

**File liên quan:** `src/data_access/IRepository.h`, `Repository.h`, `RoomRepository.h`, `RoomRepository.cpp`, `src/models/Room.h`, `src/models/Seat.h`, `src/utils/Validator.h`.

**Trạng thái:** Đã **CHỐT MARKDOWN**, triển khai và kiểm thử trong phạm vi `tests/test_room_repository.cpp` ngày 25/11/2026: **72 PASS, 0 FAIL**. Hai khối mã `.h/.cpp` ở mục 5 đã đối chiếu và khớp mã nguồn thực tế.

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

`RoomRepository : public IRepository<Room>` hiện đã có trong mã nguồn và triển khai đủ 9 hàm thuần ảo: `load`, `save`, `add`, `update`, `removeById`, `findById`, `getAt`, `count`, `isLoaded`. Từ khóa `override` giúp trình biên dịch kiểm tra chữ ký. **Không thay `RoomService` sang interface trong phạm vi này**.

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

    // Doc va ghi du lieu
    bool load() override;
    bool save() override;

    // Quan ly du lieu
    bool add(Room* room) override;
    bool update(const Room& newValues) override;
    bool removeById(const std::string& roomId) override;

    // Truy van du lieu
    Room* findById(const std::string& roomId) override;
    Room* getAt(int index) override;
    int count() override;
    bool isLoaded() override;
};

#endif
```

### 5.2. RoomRepository.cpp

**Đường dẫn:** `src/data_access/RoomRepository.cpp`.

Mã dưới đây đã **CHỐT MARKDOWN** và khớp mã nguồn đã triển khai: giữ logic parse/serialize, truy vấn và liên kết hiện có; chuẩn hóa thứ tự, định dạng và bỏ validation lưu trữ trùng với `Repository<Room>`.

```cpp
#include "RoomRepository.h"
#include "../utils/Validator.h"
#include "../models/Seat.h"
#include <stdexcept>
#include <vector>
using namespace std;


// Kiem tra toan bo du lieu Room
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
    if (room->getRowCount() <= 0 ||
        room->getColumnCount() <= 0)
    {
        return false;
    }
    return true;
}

RoomRepository::RoomRepository(const string& filePath)
    : repo(filePath, getRoomId, stringToRoom, roomToString)
{
}

string RoomRepository::getRoomId(Room* room)
{
    if (room == nullptr)
    {
        return "";
    }
    return room->getRoomId();
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

// Chuyen Room thanh mot dong TXT
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

// Doc du lieu phong tu file
bool RoomRepository::load()
{
    return repo.load();
}

// Luu du lieu phong xuong file
bool RoomRepository::save()
{
    return repo.save();
}

// Them phong moi
bool RoomRepository::add(Room* room)
{
    if (!isValidRoom(room))
    {
        return false;
    }

    return repo.add(room);
}

// Cap nhat thong tin phong
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

    // Chi kiem tra ghe khi thu nho phong
    if (newValues.getRowCount() < room->getRowCount() ||
        newValues.getColumnCount() < room->getColumnCount())
    {
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
    }

    room->setName(newValues.getName());
    room->setRowCount(newValues.getRowCount());
    room->setColumnCount(newValues.getColumnCount());
    room->setActive(newValues.isActive());
    return true;
}

bool RoomRepository::removeById(const string& roomId)
{
    Room* room = repo.findById(roomId);

    if (room == nullptr)
    {
        return false;
    }
    // Khong xoa neu phong con lien ket Seat hoac Showtime
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
  room == nullptr? --có--> false
      | không
      v
  ID sai? --có--> false
      | không
      v
  tên sai? --có--> false
      | không
      v
  hàng <= 0 hoặc cột <= 0? --có--> false
      | không
      v
     true
  ```

- **Giải thích:** `const Room*` không cho thay đổi đối tượng qua tham số; `Validator::isValidId` xác minh định dạng; `Validator::isValidText` xác minh văn bản; một điều kiện `||` kiểm tra cả số hàng và số cột phải dương. Không kiểm tra quan hệ Seat/Showtime ở helper này.


### 7.2. `getRoomId(Room* room)`

- **Mục đích:** Trích ID cho `Repository<Room>`; trả chuỗi rỗng nếu null.
- **Luồng hoạt động chi tiết (ASCII):**

  ```text
  `room == nullptr? --có--> return ""; --không--> return room->getRoomId()`.
  ```

- **Giải thích:** `nullptr` ngăn dereference sai; `->` truy cập getter qua con trỏ; trả về bản sao `string`.


### 7.3. `stringToRoom(const string& line)`

- **Mục đích:** Parse đúng 5 trường `roomId|name|rowCount|columnCount|active`, trả `Room*` mới hoặc ném `runtime_error`.
- **Luồng hoạt động chi tiết (ASCII):**

  ```text
  `line -> tách bằng '|' -> đủ 5? --không--> throw -> stoi cả 2 số + kiểm tra đọc hết? --không--> throw -> active 0/1? --không--> throw -> new Room -> isValidRoom? --không--> delete + throw; --có--> return room`.
  ```

- **Giải thích:** `vector<string> fields` giữ các trường; `start`, `find`, `substr` duyệt từ trái qua phải và giữ trường rỗng cuối. `stoi` dùng `pos1/pos2` để chặn chuỗi số có hậu tố; `try/catch` chuyển lỗi thành thông báo Room. Chỉ chấp nhận `"0"` hoặc `"1"` và chuyển thành `bool`. `new Room` cấp phát heap; nếu kiểm tra thất bại phải `delete` trước khi throw; khi thành công chuyển con trỏ cho caller (`Repository<T>::load`).


### 7.4. `roomToString(Room* room)`

- **Mục đích:** Serialize Room hợp lệ thành một dòng TXT; ném lỗi nếu không hợp lệ.
- **Luồng hoạt động chi tiết (ASCII):**

  ```text
  `isValidRoom? --không--> throw; --có--> nối ID|tên|hàng|cột|active -> return string`.
  ```

- **Giải thích:** `to_string` chuyển số nguyên; toán tử `+` nối trường với `|`; toán tử ba ngôi chuyển bool thành `"1"`/`"0"`. Không ghi file trực tiếp.


## 8. Giải thích các hàm public

### 8.1. Constructor `RoomRepository(filePath)`

- **Mục đích:** Khởi tạo `repo` với đường dẫn TXT và ba callback static; không tự load.
- **Tham số:** `filePath` tham chiếu chuỗi; không trả về.
- **Luồng hoạt động chi tiết (ASCII):**

  ```text
  `filePath -> repo(filePath, getRoomId, stringToRoom, roomToString) -> kết thúc`.
  ```

- **Giải thích:** Danh sách khởi tạo sau dấu `:` xây dựng thành viên `repo` trước thân constructor `{}`. Callback dùng khi tra ID, parse và serialize; chưa cấp phát `Room`.


### 8.2. `load()`

- **Mục đích:** Chuyển yêu cầu nạp TXT cho `Repository<Room>`; trả nguyên kết quả của `repo.load()`.
- **Luồng hoạt động chi tiết (ASCII):**

  ```text
  repo.load() -> đã load hoặc không đọc được file? --có--> false
      | không
      v
  parse từng dòng vào danh sách tạm -> lỗi? --có--> ném exception
      | không
      v
  swap danh sách, đặt loaded = true -> true
  ```

- **Giải thích:** `Repository<T>::load()` tự chặn nạp lặp, đọc file, parse vào danh sách tạm, kiểm tra ID trùng, rồi swap; lỗi parse có thể ném exception kèm file/dòng. Không tự tạo file thiếu.


### 8.3. `save()`

- **Mục đích:** Ghi danh sách Room vào TXT; trả kết quả `repo.save()`.
- **Luồng hoạt động chi tiết (ASCII):**

  ```text
  `repo.save() -> return bool`.
  ```

- **Giải thích:** `Repository<T>::save()` gọi callback `roomToString` cho từng đối tượng rồi `FileIO::writeLines`; không phải transaction nguyên tử và có thể ném exception nếu dữ liệu sai.


### 8.4. `add(Room* room)`

- **Mục đích:** Kiểm tra dữ liệu Room rồi chuyển thao tác thêm cho `Repository<Room>`.
- **Luồng hoạt động chi tiết (ASCII):**

  ```text
  `isValidRoom(room)? --không--> false; --có--> repo.add(room) -> bool`.
  ```

- **Giải thích:** `isValidRoom()` kiểm tra dữ liệu đặc thù; `repo.add()` tự kiểm trạng thái loaded, con trỏ null/trùng và ID rỗng/trùng. Thành công thì repo sở hữu `room`; thất bại caller vẫn sở hữu.


### 8.5. `update(const Room& newValues)`

- **Mục đích:** Cập nhật thuộc tính của phòng có ID tương ứng, không để ghế vượt kích thước mới.
- **Luồng hoạt động chi tiết (ASCII):**

  ```text
  `loaded + valid? --không--> false -> findById -> null? --có--> false -> giảm hàng/cột? --có--> duyệt ghế, null/vượt giới hạn? --có--> false -> cập nhật bốn thuộc tính -> true`.
  ```

- **Giải thích:** `&newValues` lấy địa chỉ đối tượng tham chiếu để validate; tìm theo ID bất biến. Chỉ khi số hàng hoặc số cột giảm mới dùng `getSeatCount` và `getSeatAt` để duyệt liên kết ghế. Ghế null hoặc có tọa độ vượt phạm vi mới khiến hàm trả `false` trước mọi setter. Hàm không save; RoomService hiện kiểm thay đổi trạng thái và kích thước khi có Showtime, còn kiểm tra lịch sử Ticket khi đổi sơ đồ phòng cần hoàn thiện lúc refactor Service.


### 8.6. `removeById(const string& roomId)`

- **Mục đích:** Xóa phòng chưa được ghế hoặc suất chiếu tham chiếu.
- **Luồng hoạt động chi tiết (ASCII):**

  ```text
  `findById -> null? --có--> false -> có ghế/suất? --có--> false -> repo.removeById(roomId)`.
  ```

- **Giải thích:** `getSeatCount`/`getShowtimeCount` chặn xóa đối tượng còn liên kết; `Repository<T>::removeById` gỡ node và `delete Room*` khi thành công. Con trỏ mượn đến Room bị xóa sẽ không còn hợp lệ.


### 8.7. `findById(const string& roomId)`

- **Mục đích:** Trả con trỏ Room theo ID hoặc `nullptr`.
- **Luồng hoạt động chi tiết (ASCII):**

  ```text
  `roomId -> repo.findById(roomId) -> Room*/nullptr`.
  ```

- **Giải thích:** Chỉ chuyển tiếp cho generic repo; không chuyển quyền sở hữu con trỏ.


### 8.8. `getAt(int index)`

- **Mục đích:** Lấy con trỏ Room theo vị trí hoặc `nullptr` nếu không có node.
- **Luồng hoạt động chi tiết (ASCII):**

  ```text
  `index -> repo.getAt(index) -> Room*/nullptr`.
  ```

- **Giải thích:** Vị trí do danh sách liên kết quản lý; caller không sở hữu kết quả.


### 8.9. `count()`

- **Mục đích:** Đếm số Room hiện trong danh sách.
- **Luồng hoạt động chi tiết (ASCII):**

  ```text
  `repo.count() -> int`.
  ```

- **Giải thích:** `Repository<T>::count()` trả `items.nodeCount()`; không tự load.


### 8.10. `isLoaded()`

- **Mục đích:** Trả trạng thái nạp dữ liệu.
- **Luồng hoạt động chi tiết (ASCII):**

  ```text
  `repo.isLoaded() -> bool`.
  ```

- **Giải thích:** Chuyển tiếp cờ `loaded` của generic repo; không đọc file.


## 9. Quản lý bộ nhớ

### 9.1. Quyền sở hữu đối tượng

`Repository<Room>` sở hữu `Room*` sau khi `add` thành công hoặc sau khi `load` hoàn tất. `Repository<T>::clear()` và `removeById()` giải phóng các đối tượng được sở hữu. Khi `add()` thất bại, caller vẫn chịu trách nhiệm `delete` con trỏ chưa được chuyển giao.

### 9.2. Con trỏ mượn

`findById()` và `getAt()` trả con trỏ mượn; không `delete` từ caller. Con trỏ mất hiệu lực khi Room bị xóa hoặc Repository bị hủy/clear. `Room` có danh sách liên kết Seat/Showtime nhưng không sở hữu các đối tượng liên kết đó.

### 9.3. Destructor

`IRepository<Room>` có `virtual ~IRepository() = default;`. `RoomRepository` không cần khai báo destructor riêng; destructor mặc định hủy thành viên `repo`, từ đó generic Repository giải phóng các Room mà nó sở hữu.

## 10. Áp dụng Dependency Inversion Principle

### 10.1. Trước khi refactor

Thiết kế trước khi áp dụng DIP dùng lớp concrete độc lập; mã nguồn hiện tại đã khai báo `RoomRepository : public IRepository<Room>`. `RoomService` vẫn nhận `RoomRepository&`.

### 10.2. Sau khi refactor Repository

Thiết kế tiếp tục kế thừa `IRepository<Room>` và ghi `override` cho đủ chín phương thức. Có thể gọi các phương thức chung qua `IRepository<Room>&` khi kiểm thử. **`RoomService` vẫn nhận `RoomRepository&`** trong đợt này; DIP ở tầng Service chưa hoàn tất và không được ghi nhận là đã triển khai.

## 11. Các thay đổi sau refactor

### 11.1. Thay đổi trong RoomRepository.h

- Giữ `#include "IRepository.h"`, `class RoomRepository : public IRepository<Room>` và chín hàm `override` đã có.
- Header không dùng `using namespace std;`, viết `std::string` trong các chữ ký; sắp xếp callback, validation và nhóm public theo cùng thứ tự, thêm comment nhóm.

### 11.2. Thay đổi trong RoomRepository.cpp

Sắp xếp định nghĩa và comment theo MovieRepository. `isValidRoom()` gộp kiểm tra hàng/cột; `load()` và `add()` bỏ kiểm tra loaded, con trỏ trùng do `Repository<Room>` đã đảm nhiệm. `removeById()` bỏ kiểm tra loaded riêng nhưng giữ kiểm tra Room tồn tại và hai danh sách liên kết. `update()` giữ loaded vì sửa Model trực tiếp, chỉ duyệt ghế khi thu nhỏ và kiểm tra trước các setter. Không thêm rollback, `try/catch` hoặc thay đổi nghiệp vụ.

### 11.3. Các thành phần giữ nguyên

`RoomService`, `IRepository.h`, `Repository.h`, Model `Room`, `Seat`, `Showtime`, Validator, định dạng TXT, quyền `ROOM_*` và quy tắc cấp ID. RoomRepository giữ tính hợp lệ dữ liệu và liên kết hiện có; RoomService kiểm Permission, hiện chặn đổi trạng thái qua `updateRoom()` và đổi kích thước khi có Showtime. Việc kiểm tra đầy đủ nghiệp vụ sơ đồ phòng liên quan Showtime/Ticket sẽ hoàn thiện khi refactor Service, chưa được coi là đã bảo đảm trong mã đề xuất này.

## 12. Kế hoạch kiểm thử

**Kết quả thực tế:** Biên dịch C++17 với `tests/test_room_repository.cpp` và các `.cpp` trong `src/` (trừ `main.cpp`), chạy ngày 25/11/2026: **72 PASS, 0 FAIL**, exit code 0. Kiệt triển khai DIP ở `RoomRepository`; Hoàng rà soát, chuẩn hóa tài liệu và kiểm thử lại. Các ô dưới đây chỉ đánh dấu khi bài test hiện tại có kiểm tra tương ứng.

### 12.1. Kiểm thử nạp dữ liệu

- [x] Load file hợp lệ, rỗng, thiếu, sai số trường, ID sai/trùng, số hàng/cột sai, active khác 0/1.
- [x] Load hai lần; dữ liệu không bị nạp dở khi parse lỗi.
- [ ] Xác minh riêng nội dung exception có tên file/dòng.

### 12.2. Kiểm thử thêm phòng

- [x] Thêm Room hợp lệ, chưa load, null, ID trùng, con trỏ trùng, tên/kích thước không hợp lệ.
- [x] Xác minh con trỏ được giữ sau khi thêm thành công và caller giải phóng con trỏ khi thêm thất bại.

### 12.3. Kiểm thử cập nhật phòng

- [x] Update thành công, ID không tồn tại, chưa load, dữ liệu invalid.
- [ ] Tăng riêng số hàng, tăng riêng số cột và tăng cả hai: thành công, không cần duyệt ghế; giữ danh sách ghế.
- [ ] Giảm riêng số hàng, giảm riêng số cột và giảm cả hai: từ chối khi có ghế ngoài phạm vi mới, chấp nhận khi mọi ghế vẫn nằm trong phạm vi. Kiểm ghế null trong nhánh thu nhỏ.
- [ ] Đổi tên/trạng thái hoặc giữ nguyên kích thước: không duyệt ghế; dữ liệu sai phải bị từ chối trước setter.
- [x] Giữ nguyên ID, địa chỉ Room và liên kết Seat/Showtime trong trường hợp đã kiểm thử.
- [ ] Khi refactor RoomService, kiểm Permission và các ràng buộc Showtime/Ticket trước khi cho thay đổi sơ đồ phòng; chưa đánh dấu PASS ở bước tài liệu.

### 12.4. Kiểm thử xóa phòng

- [x] Xóa phòng không còn liên kết; từ chối nếu còn Seat hoặc Showtime.
- [x] Xóa ID không tồn tại/chưa load; xác minh số lượng và kết quả tra cứu sau xóa.

### 12.5. Kiểm thử truy vấn

- [x] `findById` tồn tại/không tồn tại; `getAt` chỉ số hợp lệ/âm/vượt phạm vi.
- [x] `count` và `isLoaded` trước/sau load, add, remove trong các trường hợp bài test sử dụng.

### 12.6. Kiểm thử lưu dữ liệu

- [x] Save trước load; save Room hợp lệ và load lại để so sánh thuộc tính.
- [ ] Đối chiếu trực tiếp nội dung file TXT theo 5 trường và 0/1.
- [ ] Save khi dữ liệu bị thay đổi thành invalid (exception từ serializer); lỗi mở/ghi file (`false` từ `FileIO`).

### 12.7. Kiểm thử IRepository

- [x] Biên dịch `RoomRepository` với `IRepository<Room>&` và `IRepository<Room>*`.
- [x] Gọi đủ 9 phương thức qua interface; kiểm tra destructor ảo và xóa qua con trỏ interface.
- [x] Chạy `tests/test_room_repository.cpp`: 72 PASS, 0 FAIL.

## 13. Tiến độ triển khai

- [x] Đọc `AGENTS.md`, `IRepository.h`, `MovieRepository.h/.cpp` và `MovieRepository.md`.
- [x] Đọc `RoomRepository.h/.cpp`, `RoomService.h/.cpp`, `Room.h/.cpp` và generic `Repository.h`.
- [x] Xác định phạm vi chỉ refactor Repository.
- [x] Viết bản thiết kế Markdown theo bố cục 14 mục của mẫu MovieRepository.
- [x] Review và **CHỐT MARKDOWN**.
- [x] Đối chiếu hai khối code ở mục 5 khớp `.h/.cpp` thực tế.
- [x] Biên dịch và kiểm thử thực tế ngày 25/11/2026: 72 PASS, 0 FAIL.
- [x] Cập nhật tiến độ sau kiểm thử.

## 14. Kết luận

`RoomRepository` triển khai `IRepository<Room>` bằng public inheritance, giữ `Repository<Room>` theo composition. Mã `.h/.cpp` khớp Markdown đã chốt; bài test RoomRepository đạt **72 PASS, 0 FAIL**. Phần refactor DIP của Repository và việc Hoàng rà soát, chuẩn hóa, kiểm thử lại đã hoàn thành trong phạm vi này. Các trường hợp còn để trống ở mục 12 và DIP của `RoomService` chưa được xác nhận hoàn thành.
