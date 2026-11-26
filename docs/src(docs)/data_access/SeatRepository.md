# SeatRepository

## 1. Tổng quan

`SeatRepository` quản lý dữ liệu ghế ở tầng Data Access.

Lớp kế thừa `IRepository<Seat>`, giữ `Repository<Seat>` bằng composition và nhận `IRepository<Room>&` thông qua constructor.

SeatRepository chịu trách nhiệm:

- Đọc và ghi dữ liệu ghế.
- Kiểm tra tính hợp lệ của dữ liệu Seat.
- Quản lý ghế bằng khóa kết hợp.
- Kiểm tra tham chiếu Room.
- Quản lý liên kết hai chiều giữa Seat và Room.
- Thêm, cập nhật, xóa và truy vấn Seat.
- Khôi phục liên kết khi thao tác thất bại.

SeatRepository không thực hiện phân quyền, không xử lý nghiệp vụ bán vé và không tự động ghi file sau mỗi thao tác.

**File liên quan:**

- `src/data_access/SeatRepository.h`
- `src/data_access/SeatRepository.cpp`
- `src/data_access/IRepository.h`
- `src/data_access/Repository.h`
- `src/models/Seat.h`
- `src/models/Room.h`
- `src/utils/Validator.h`

**Trạng thái:** CHỐT MARKDOWN; hai khối mã ở mục 5 đã triển khai, comment đã chuẩn hóa. `tests/test_seat_repository.cpp` đạt Passed 81, Failed 0, Total 81 (26/11/2026) trong phạm vi bài test; các nhánh lỗi nội bộ và SeatService còn chờ kiểm thử.

## 2. Kiến trúc

### 2.1. Quan hệ giữa các lớp

```text
Business Service → IRepository<Seat> ← SeatRepository
                                            │
                                            │ composition
                                            ▼
                                      Repository<Seat>

SeatRepository
       │
       │ dependency injection
       ▼
IRepository<Room> ← RoomRepository
```

SeatRepository triển khai chín hàm của `IRepository<Seat>`.

Ngoài ra, lớp bổ sung hai hàm overload để hỗ trợ tìm kiếm và xóa ghế bằng mã phòng và mã ghế.

RoomRepository phải được khởi tạo và load trước SeatRepository, đồng thời phải tồn tại lâu hơn SeatRepository để đảm bảo việc tháo liên kết khi hủy đối tượng.

### 2.2. Quan hệ với IRepository và Repository

`IRepository<Seat>` định nghĩa hợp đồng chung cho các thao tác quản lý dữ liệu Seat.

`Repository<Seat>` chịu trách nhiệm:

- Quản lý danh sách Seat.
- Quản lý bộ nhớ.
- Kiểm tra khóa chính.
- Đọc và ghi file TXT.
- Cung cấp các thao tác lưu trữ chung.

SeatRepository sử dụng Repository bằng composition và bổ sung các ràng buộc riêng của Seat.

`IRepository<Room>&` là dependency được truyền từ bên ngoài, không thuộc quyền sở hữu của SeatRepository.

### 2.3. Trách nhiệm validation

#### Repository chung

`Repository<Seat>` chịu trách nhiệm:

- Kiểm tra trạng thái loaded.
- Kiểm tra con trỏ null khi thêm.
- Kiểm tra trùng con trỏ.
- Kiểm tra khóa rỗng hoặc trùng khóa.
- Kiểm tra đối tượng tồn tại khi xóa.
- Kiểm tra vị trí truy cập.
- Xử lý lỗi đọc và ghi file.

#### SeatRepository

SeatRepository chịu trách nhiệm:

- Kiểm tra thuộc tính Seat.
- Kiểm tra định dạng mã phòng và mã ghế.
- Kiểm tra Room tồn tại.
- Kiểm tra liên kết hiện tại trước khi thêm.
- Quản lý liên kết Seat – Room.
- Không cho phép thay đổi vị trí ghế thông qua update().
- Rollback khi thao tác thất bại.

Các kiểm tra cần thiết trước khi tạo hoặc tháo liên kết vẫn được giữ lại, kể cả khi Repository chung có kiểm tra tương ứng.

#### Model

`Seat::linkRoom()` và `Room::linkSeat()` chịu trách nhiệm:

- Kiểm tra mã phòng tương ứng.
- Kiểm tra liên kết hiện tại.
- Kiểm tra giới hạn hàng và cột.
- Kiểm tra trùng mã ghế trong phòng.
- Kiểm tra trùng vị trí ghế trong phòng.
- Thiết lập liên kết hai chiều.

SeatRepository tận dụng các hàm này, không viết lại toàn bộ logic kiểm tra liên kết.

#### Service

SeatService chịu trách nhiệm:

- Kiểm tra Permission.
- Kiểm tra các ràng buộc nghiệp vụ liên quan đến phòng và ghế.
- Kiểm tra lịch sử Ticket trước khi cập nhật hoặc xóa ghế theo business_rules.md.

Đặc biệt, trước khi gọi `removeById()`, Service phải đảm bảo ghế chưa từng được Ticket tham chiếu, kể cả vé đã hủy.

SeatRepository không nhận thêm TicketRepository làm dependency.

## 3. Định dạng dữ liệu

Mỗi dòng trong file TXT gồm đúng năm trường:

```text
roomId|seatId|row|column|seatType
```

Ví dụ:

```text
ROM0001|A01|1|1|STANDARD
ROM0001|A02|1|2|VIP
ROM0002|A01|1|1|COUPLE
```

| Trường | Kiểu dữ liệu | Yêu cầu |
|---|---|---|
| roomId | string | ID hợp lệ với tiền tố ROM |
| seatId | string | Chuỗi hợp lệ theo Validator |
| row | int | Lớn hơn 0 |
| column | int | Lớn hơn 0 |
| seatType | SeatType | STANDARD, VIP hoặc COUPLE |

### 3.1. Khóa kết hợp

Mã ghế có thể trùng nhau giữa các phòng khác nhau.

Ví dụ:

```text
ROM0001|A01
ROM0002|A01
```

Hai ghế trên là hai đối tượng khác nhau.

Vì vậy, SeatRepository sử dụng khóa kết hợp:

```text
roomId|seatId
```

Khóa này được sử dụng trong Repository chung để xác định duy nhất một Seat.

Không thay đổi cấu trúc Model hoặc bổ sung trường khóa mới vào file TXT.

### 3.2. Ràng buộc vị trí

Hàng và cột phải lớn hơn 0 và không được vượt quá kích thước Room tương ứng.

Trong cùng một phòng:

- Không được trùng seatId.
- Không được trùng vị trí row và column.

Các ràng buộc này được kiểm tra khi thiết lập liên kết thông qua `Room::linkSeat()`.

## 4. Thiết kế lớp

### 4.1. Thuộc tính

| Thuộc tính | Kiểu | Vai trò |
|---|---|---|
| repo | Repository<Seat> | Quản lý dữ liệu và bộ nhớ Seat |
| roomRepo | IRepository<Room>& | Truy xuất Room và kiểm tra trạng thái load |

SeatRepository không có danh sách Seat hoặc cờ loaded riêng.

### 4.2. Hàm private

| Hàm | Vai trò |
|---|---|
| getSeatKey() | Lấy khóa kết hợp của Seat |
| stringToSeat() | Chuyển một dòng TXT thành Seat |
| seatToString() | Chuyển Seat thành một dòng TXT |
| isValidSeat() | Kiểm tra thuộc tính Seat |
| unlinkReferences() | Tháo liên kết của một Seat |
| unlinkAll() | Tháo liên kết của toàn bộ Seat |

Bốn hàm đầu tiên được khai báo static vì không sử dụng thuộc tính của SeatRepository.

Ba hàm `getSeatKey()`, `stringToSeat()` và `seatToString()` được truyền vào constructor của Repository chung dưới dạng callback.

### 4.3. Hàm public

| Hàm | Vai trò | Override |
|---|---|---|
| Constructor | Khởi tạo Repository và dependency | Không |
| Destructor | Tháo liên kết trước khi giải phóng dữ liệu | Có |
| load() | Đọc dữ liệu và thiết lập liên kết | Có |
| save() | Ghi dữ liệu xuống file | Có |
| add() | Thêm Seat | Có |
| update() | Cập nhật loại ghế | Có |
| removeById(key) | Xóa Seat theo khóa kết hợp | Có |
| removeById(roomId, seatId) | Xóa Seat theo hai mã | Không |
| findById(key) | Tìm Seat theo khóa kết hợp | Có |
| findById(roomId, seatId) | Tìm Seat theo hai mã | Không |
| getAt() | Lấy Seat theo vị trí | Có |
| count() | Đếm số lượng Seat | Có |
| isLoaded() | Kiểm tra trạng thái load | Có |

SeatRepository có:

- 2 thuộc tính.
- 6 hàm private.
- 9 hàm override từ hợp đồng IRepository.
- 2 hàm overload bổ sung.
- 1 constructor.
- 1 destructor override.

Tổng cộng 19 hàm, tính cả constructor và destructor.

## 5. Mã nguồn đã chốt và triển khai

Hai khối mã nguồn dưới đây là bản thiết kế đã CHỐT MARKDOWN qua quá trình rà soát từng hàm và được triển khai nguyên văn.

Mã nguồn được chuẩn hóa theo ShowtimeRepository, điều chỉnh phù hợp với đặc điểm của SeatRepository.

### 5.1. SeatRepository.h

**Đường dẫn:** `src/data_access/SeatRepository.h`

```cpp
#ifndef SEAT_REPOSITORY_H
#define SEAT_REPOSITORY_H

#include <string>
#include "IRepository.h"
#include "Repository.h"
#include "../models/Seat.h"

class Room;

class SeatRepository : public IRepository<Seat>
{
private:
    Repository<Seat> repo;
    IRepository<Room>& roomRepo;

    // Chuyen doi du lieu
    static std::string getSeatKey(Seat* seat);
    static Seat* stringToSeat(const std::string& line);
    static std::string seatToString(Seat* seat);

    // Kiem tra du lieu
    static bool isValidSeat(const Seat* seat);

    // Quan ly lien ket
    void unlinkReferences(Seat* seat);
    void unlinkAll();

public:
    SeatRepository(const std::string& filePath,
                   IRepository<Room>& roomRepo);
    ~SeatRepository() override;

    // Doc va ghi du lieu
    bool load() override;
    bool save() override;

    // Quan ly du lieu
    bool add(Seat* seat) override;
    bool update(const Seat& newValues) override;

    bool removeById(const std::string& key) override;
    bool removeById(const std::string& roomId,
                    const std::string& seatId);

    // Truy van du lieu
    Seat* findById(const std::string& key) override;
    Seat* findById(const std::string& roomId,
                   const std::string& seatId);

    Seat* getAt(int index) override;
    int count() override;
    bool isLoaded() override;
};

#endif
```

### 5.2. SeatRepository.cpp

**Đường dẫn:** `src/data_access/SeatRepository.cpp`

```cpp
#include "SeatRepository.h"
#include "../models/Room.h"
#include "../utils/Validator.h"
#include <exception>
#include <iostream>
#include <stdexcept>
#include <vector>
using namespace std;

bool SeatRepository::isValidSeat(const Seat* seat)
{
    if (seat == nullptr)
    {
        return false;
    }
    if (!Validator::isValidId(seat->getRoomId(), "ROM"))
    {
        return false;
    }
    if (!Validator::isValidText(seat->getSeatId()))
    {
        return false;
    }
    if (seat->getRow() <= 0 || seat->getColumn() <= 0)
    {
        return false;
    }

    SeatType type = seat->getSeatType();
    if (type != SeatType::STANDARD &&
        type != SeatType::VIP &&
        type != SeatType::COUPLE)
    {
        return false;
    }
    return true;
}

string SeatRepository::getSeatKey(Seat* seat)
{
    if (seat == nullptr)
    {
        return "";
    }
    // Khoa ghe gom ma phong va ma ghe.
    return seat->getRoomId() + "|" + seat->getSeatId();
}

Seat* SeatRepository::stringToSeat(const string& line)
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
        throw runtime_error("Sai so luong truong Seat");
    }

    int row;
    int column;
    try
    {
        size_t pos1 = 0;
        size_t pos2 = 0;

        row = stoi(fields[2], &pos1);
        column = stoi(fields[3], &pos2);

        if (pos1 != fields[2].length() ||
            pos2 != fields[3].length())
        {
            throw runtime_error("Sai dinh dang so");
        }
    }
    catch (const exception&)
    {
        throw runtime_error("Hang hoac cot Seat khong hop le");
    }

    SeatType seatType;
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

    Seat* seat = new Seat(
        fields[0],
        fields[1],
        row,
        column,
        seatType
    );
    if (!isValidSeat(seat))
    {
        delete seat;
        throw runtime_error("Du lieu Seat khong hop le");
    }
    return seat;
}

string SeatRepository::seatToString(Seat* seat)
{
    if (!isValidSeat(seat))
    {
        throw runtime_error("Seat khong hop le khi ghi file");
    }

    string type;
    switch (seat->getSeatType())
    {
        case SeatType::STANDARD:
            type = "STANDARD";
            break;

        case SeatType::VIP:
            type = "VIP";
            break;

        case SeatType::COUPLE:
            type = "COUPLE";
            break;

        default:
            throw runtime_error("Loai ghe khong hop le");
    }

    return seat->getRoomId() + "|" +
           seat->getSeatId() + "|" +
           to_string(seat->getRow()) + "|" +
           to_string(seat->getColumn()) + "|" +
           type;
}

SeatRepository::SeatRepository(const string& filePath,
                               IRepository<Room>& roomRepo)
    : repo(filePath, getSeatKey, stringToSeat, seatToString),
      roomRepo(roomRepo)
{
}

SeatRepository::~SeatRepository()
{
    // Go lien ket truoc khi repo huy cac Seat dang so huu.
    try
    {
        unlinkAll();
    }
    catch (const exception& e)
    {
        cerr << "Loi huy SeatRepository: " << e.what() << endl;
        terminate();
    }
    catch (...)
    {
        cerr << "Loi huy SeatRepository khong xac dinh" << endl;
        terminate();
    }
}

void SeatRepository::unlinkReferences(Seat* seat)
{
    if (seat == nullptr)
    {
        return;
    }

    Room* room = seat->getRoom();
    if (room != nullptr && !room->unlinkSeat(seat))
    {
        throw runtime_error("Khong the thao lien ket Seat va Room");
    }
}

void SeatRepository::unlinkAll()
{
    for (int i = 0; i < repo.count(); i++)
    {
        unlinkReferences(repo.getAt(i));
    }
}

bool SeatRepository::load()
{
    if (!roomRepo.isLoaded())
    {
        return false;
    }
    if (!repo.load())
    {
        return false;
    }
    try
    {
        for (int i = 0; i < repo.count(); i++)
        {
            Seat* seat = repo.getAt(i);
            Room* room = roomRepo.findById(seat->getRoomId());

            if (room == nullptr)
            {
                throw runtime_error("Seat tham chieu Room khong ton tai");
            }
            if (!seat->linkRoom(room))
            {
                throw runtime_error("Khong the lien ket Seat voi Room");
            }
        }
    }
    catch (...)
    {
        // Go lien ket truoc khi xoa Seat da nap.
        unlinkAll();
        repo.clear();
        throw;
    }
    return true;
}

bool SeatRepository::save()
{
    return repo.save();
}

bool SeatRepository::add(Seat* seat)
{
    if (!repo.isLoaded() || !roomRepo.isLoaded() ||
        !isValidSeat(seat))
    {
        return false;
    }
    // Tu choi ghe da lien ket hoac trung khoa truoc khi thay doi Room.
    if (seat->getRoom() != nullptr)
    {
        return false;
    }
    if (findById(getSeatKey(seat)) != nullptr)
    {
        return false;
    }

    Room* room = roomRepo.findById(seat->getRoomId());
    if (room == nullptr)
    {
        return false;
    }

    // Lien ket Room truoc; repo chi nhan quyen so huu khi them thanh cong.
    try
    {
        if (seat->linkRoom(room) && repo.add(seat))
        {
            return true;
        }
    }
    catch (...)
    {
        // Thu hoi lien ket neu thao tac them nem loi.
        unlinkReferences(seat);
        throw;
    }

    // Caller van so huu Seat neu them that bai.
    unlinkReferences(seat);
    return false;
}

bool SeatRepository::update(const Seat& newValues)
{
    if (!repo.isLoaded() || !isValidSeat(&newValues))
    {
        return false;
    }

    Seat* seat = findById(newValues.getRoomId(),
                          newValues.getSeatId());

    if (seat == nullptr)
    {
        return false;
    }
    // Vi tri ghe bat bien; chi cho phep doi loai ghe.
    if (seat->getRow() != newValues.getRow() ||
        seat->getColumn() != newValues.getColumn())
    {
        return false;
    }

    seat->setSeatType(newValues.getSeatType());
    return true;
}

bool SeatRepository::removeById(const string& key)
{
    if (!repo.isLoaded())
    {
        return false;
    }

    Seat* seat = repo.findById(key);
    if (seat == nullptr)
    {
        return false;
    }

    Room* room = seat->getRoom();
    if (room == nullptr)
    {
        return false;
    }

    // Go khoi Room truoc khi repo giai phong Seat.
    if (!room->unlinkSeat(seat))
    {
        return false;
    }

    if (!repo.removeById(key))
    {
        // Khoi phuc lien ket neu Seat van con trong repo.
        if (!room->linkSeat(seat))
        {
            throw runtime_error("Khong the khoi phuc lien ket Seat");
        }
        return false;
    }
    return true;
}

bool SeatRepository::removeById(const string& roomId,
                                const string& seatId)
{
    return removeById(roomId + "|" + seatId);
}

Seat* SeatRepository::findById(const string& key)
{
    return repo.findById(key);
}

Seat* SeatRepository::findById(const string& roomId,
                               const string& seatId)
{
    return findById(roomId + "|" + seatId);
}

Seat* SeatRepository::getAt(int index)
{
    return repo.getAt(index);
}

int SeatRepository::count()
{
    return repo.count();
}

bool SeatRepository::isLoaded()
{
    return repo.isLoaded();
}
```

## 6. Giải thích thư viện và từ khóa

### 6.1. Các thư viện và file header

| Thư viện/File | Vai trò |
|---|---|
| string | Sử dụng string, stoi(), to_string() |
| vector | Lưu các trường dữ liệu sau khi tách chuỗi |
| exception | Sử dụng exception và terminate() |
| iostream | Sử dụng cerr và endl để thông báo lỗi |
| stdexcept | Sử dụng runtime_error |
| IRepository.h | Interface chung cho các Repository |
| Repository.h | Lớp lưu trữ dữ liệu chung |
| Seat.h | Định nghĩa Model Seat và SeatType |
| Room.h | Truy xuất và quản lý liên kết Room |
| Validator.h | Kiểm tra định dạng dữ liệu |

Trong file header sử dụng `std::` trực tiếp.

Trong file cpp sử dụng `using namespace std;` để giảm việc lặp lại tiền tố namespace.

### 6.2. Forward declaration

```cpp
class Room;
```

Thông báo cho trình biên dịch biết Room là một lớp.

SeatRepository.h chỉ cần khai báo kiểu Room để sử dụng trong:

```cpp
IRepository<Room>& roomRepo;
```

File cpp include Room.h để có đầy đủ định nghĩa của Room khi gọi các hàm thành viên.

Cách này giảm sự phụ thuộc trực tiếp giữa các file header.

### 6.3. static

Các hàm static thuộc về lớp và không cần một đối tượng SeatRepository cụ thể để gọi.

Ví dụ:

```cpp
static std::string getSeatKey(Seat* seat);
```

Hàm chỉ sử dụng đối tượng Seat được truyền vào, không truy cập repo hoặc roomRepo.

Do đó, hàm có thể được truyền vào Repository chung dưới dạng callback.

### 6.4. override

```cpp
bool load() override;
```

Từ khóa override xác nhận hàm đang ghi đè một hàm virtual của lớp cha.

Trình biên dịch sẽ kiểm tra chữ ký hàm có khớp với IRepository hay không.

Destructor cũng sử dụng override vì IRepository có virtual destructor.

### 6.5. Overload

Overload cho phép nhiều hàm có cùng tên nhưng khác danh sách tham số.

Ví dụ:

```cpp
Seat* findById(const std::string& key) override;

Seat* findById(const std::string& roomId,
               const std::string& seatId);
```

Hàm thứ nhất vừa thuộc nhóm overload, vừa override hàm của IRepository.

Hàm thứ hai chỉ là overload bổ sung của SeatRepository.

Hai hàm không chứa hai logic tìm kiếm độc lập.

Hàm nhận hai tham số chỉ ghép khóa rồi gọi hàm nhận một tham số.

### 6.6. const và tham chiếu

```cpp
bool update(const Seat& newValues);
```

Tham chiếu giúp truyền đối tượng mà không cần tạo bản sao.

const đảm bảo hàm không thay đổi đối tượng newValues.

```cpp
static bool isValidSeat(const Seat* seat);
```

Hàm được phép đọc thuộc tính Seat nhưng không được thay đổi Seat thông qua con trỏ này.

### 6.7. try, catch và throw

```cpp
try
{
    // Thuc hien thao tac
}
catch (...)
{
    // Rollback
    throw;
}
```

- try chứa các thao tác có thể phát sinh ngoại lệ.
- catch (...) bắt mọi loại ngoại lệ C++.
- throw; ném lại ngoại lệ đang được xử lý.

Rollback được thực hiện trước khi chuyển lỗi lên tầng gọi.

### 6.8. runtime_error

```cpp
throw runtime_error("Du lieu Seat khong hop le");
```

runtime_error biểu diễn lỗi phát sinh trong quá trình chạy chương trình.

SeatRepository sử dụng runtime_error khi:

- Dữ liệu TXT sai định dạng.
- Không tìm thấy Room khi load.
- Không thể thiết lập liên kết khi load.
- Không thể tháo hoặc khôi phục liên kết.

### 6.9. cerr, what() và terminate()

```cpp
cerr << e.what() << endl;
terminate();
```

- cerr là luồng xuất thông báo lỗi.
- e.what() trả về nội dung mô tả ngoại lệ.
- endl xuống dòng và đẩy dữ liệu ra luồng xuất.
- terminate() kết thúc chương trình.

Destructor không để ngoại lệ thoát ra ngoài. Nếu tháo liên kết thất bại, chương trình báo lỗi và dừng thay vì tiếp tục giải phóng Seat trong trạng thái liên kết không an toàn.

## 7. Giải thích các hàm private

### 7.1. Hàm isValidSeat()

**Chữ ký:**

```cpp
static bool isValidSeat(const Seat* seat);
```

**Mục đích:** Kiểm tra tính hợp lệ của các thuộc tính Seat.

**Giá trị trả về:**

- true: Dữ liệu hợp lệ.
- false: Có ít nhất một thuộc tính không hợp lệ.

**Quy trình xử lý:**

```text
Seat khác nullptr?
    ↓
roomId hợp lệ?
    ↓
seatId hợp lệ?
    ↓
row và column lớn hơn 0?
    ↓
SeatType hợp lệ?
    ↓
return true
```

**Bước 1:** Kiểm tra con trỏ.

```cpp
if (seat == nullptr)
{
    return false;
}
```

Tránh truy cập thuộc tính thông qua con trỏ null.

**Bước 2:** Kiểm tra mã phòng.

```cpp
Validator::isValidId(seat->getRoomId(), "ROM")
```

Đảm bảo roomId đúng định dạng ID phòng.

**Bước 3:** Kiểm tra mã ghế.

```cpp
Validator::isValidText(seat->getSeatId())
```

Validator loại bỏ chuỗi rỗng, chuỗi toàn dấu cách, ký tự điều khiển và ký tự phân cách `|`.

Không sử dụng isValidId() cho seatId vì mã ghế không theo định dạng ID bảy ký tự của các Entity khác.

**Bước 4:** Kiểm tra vị trí.

Hàng và cột phải lớn hơn 0.

Giới hạn trên phụ thuộc kích thước Room nên được kiểm tra tại Model khi liên kết.

**Bước 5:** Kiểm tra SeatType.

Chỉ chấp nhận STANDARD, VIP và COUPLE.

Hàm không kiểm tra Room tồn tại, không truy cập Repository và không thay đổi liên kết.

### 7.2. Hàm getSeatKey()

**Chữ ký:**

```cpp
static std::string getSeatKey(Seat* seat);
```

**Mục đích:** Trả về khóa định danh duy nhất của Seat.

```cpp
return seat->getRoomId() + "|" + seat->getSeatId();
```

Ví dụ:

```text
roomId = ROM0001
seatId = A01

Kết quả = ROM0001|A01
```

Nếu Seat là nullptr, hàm trả về chuỗi rỗng.

Repository chung sử dụng callback này để:

- Kiểm tra khóa trùng.
- Tìm Seat.
- Xóa Seat.
- Xác định khóa của đối tượng khi load.

Khóa kết hợp không được lưu thành thuộc tính riêng trong Model.

### 7.3. Hàm stringToSeat()

**Chữ ký:**

```cpp
static Seat* stringToSeat(const std::string& line);
```

**Mục đích:** Chuyển một dòng TXT thành đối tượng Seat.

```text
Dòng TXT
    ↓
Tách thành 5 trường
    ↓
Chuyển row và column thành int
    ↓
Chuyển chuỗi thành SeatType
    ↓
Khởi tạo Seat
    ↓
isValidSeat()
    ↓
Trả về Seat*
```

**Bước 1: Tách chuỗi.**

```cpp
size_t pos = line.find('|', start);
```

find() tìm vị trí ký tự `|` bắt đầu từ start.

Nếu không tìm thấy, hàm trả về string::npos.

```cpp
fields.push_back(line.substr(start, pos - start));
```

substr() lấy một phần của chuỗi.

push_back() thêm trường vừa tách vào vector.

```cpp
start = pos + 1;
```

Chuyển vị trí bắt đầu sang trường tiếp theo.

Khi không còn dấu phân cách, lấy trường cuối cùng rồi break khỏi vòng lặp.

**Bước 2: Kiểm tra số trường.**

```cpp
if (fields.size() != 5)
```

Mỗi dòng Seat phải có đúng năm trường.

**Bước 3: Chuyển đổi hàng và cột.**

```cpp
row = stoi(fields[2], &pos1);
column = stoi(fields[3], &pos2);
```

stoi() chuyển chuỗi thành số nguyên.

pos1 và pos2 lưu số ký tự đã được xử lý.

Ví dụ:

```text
"12"  → 12, pos = 2
"12a" → 12, pos = 2
```

Trường hợp thứ hai phải bị từ chối vì chuỗi có ba ký tự nhưng stoi() chỉ xử lý hai ký tự đầu.

```cpp
if (pos1 != fields[2].length() ||
    pos2 != fields[3].length())
```

Điều kiện này đảm bảo không có ký tự thừa phía sau số.

Nếu stoi() không thể chuyển đổi hoặc giá trị vượt phạm vi int, ngoại lệ sẽ được chuyển thành runtime_error với thông báo phù hợp.

Lưu ý: stoi() vẫn có thể chấp nhận dấu cộng và khoảng trắng đầu chuỗi. Hàm hiện tại không áp dụng quy tắc chỉ gồm chữ số.

**Bước 4: Chuyển SeatType.**

So sánh trường thứ năm với:

- STANDARD.
- VIP.
- COUPLE.

Nếu không khớp, ném runtime_error.

**Bước 5: Khởi tạo đối tượng.**

```cpp
Seat* seat = new Seat(
    fields[0],
    fields[1],
    row,
    column,
    seatType
);
```

new cấp phát bộ nhớ và gọi constructor của Seat.

Seat vừa được tạo chưa liên kết với Room.

**Bước 6: Kiểm tra và trả kết quả.**

```cpp
if (!isValidSeat(seat))
{
    delete seat;
    throw runtime_error("Du lieu Seat khong hop le");
}
```

Nếu dữ liệu không hợp lệ, giải phóng Seat trước khi ném ngoại lệ.

Nếu hợp lệ, trả con trỏ cho Repository chung tiếp nhận và quản lý.

Hàm không thực hiện liên kết Room.

### 7.4. Hàm seatToString()

**Chữ ký:**

```cpp
static std::string seatToString(Seat* seat);
```

**Mục đích:** Chuyển đối tượng Seat thành một dòng TXT.

**Bước 1:** Kiểm tra Seat bằng isValidSeat().

Không ghi dữ liệu Model không hợp lệ xuống file.

**Bước 2:** Chuyển SeatType thành chuỗi.

```cpp
switch (seat->getSeatType())
```

switch lựa chọn nhánh xử lý dựa trên giá trị SeatType.

break kết thúc nhánh hiện tại.

default xử lý trường hợp không thuộc các loại ghế hợp lệ.

**Bước 3:** Ghép các trường dữ liệu.

```cpp
return seat->getRoomId() + "|" +
       seat->getSeatId() + "|" +
       to_string(seat->getRow()) + "|" +
       to_string(seat->getColumn()) + "|" +
       type;
```

to_string() chuyển số nguyên thành chuỗi.

Kết quả tuân theo đúng định dạng file TXT.

Hàm không trực tiếp ghi file. Repository chung sử dụng chuỗi trả về để thực hiện save().

### 7.5. Hàm unlinkReferences()

**Chữ ký:**

```cpp
void unlinkReferences(Seat* seat);
```

**Mục đích:** Tháo liên kết giữa một Seat và Room tương ứng.

```text
Seat là nullptr?
├─ Có → return
└─ Không
     ↓
   Lấy Room đang liên kết
     ↓
   Room là nullptr?
   ├─ Có → kết thúc
   └─ Không → room->unlinkSeat(seat)
                 ├─ true → kết thúc
                 └─ false → throw
```

```cpp
Room* room = seat->getRoom();
```

Lấy Room hiện đang liên kết với Seat.

```cpp
if (room != nullptr && !room->unlinkSeat(seat))
```

Chỉ gọi unlinkSeat() khi Room tồn tại.

Room::unlinkSeat() chịu trách nhiệm:

- Xóa con trỏ Seat khỏi danh sách ghế của Room.
- Đặt con trỏ room của Seat về nullptr.

Nếu tháo liên kết thất bại, ném runtime_error để thông báo trạng thái không nhất quán.

Hàm được sử dụng trong:

- unlinkAll().
- Rollback add().

Không giải phóng Seat.

### 7.6. Hàm unlinkAll()

**Chữ ký:**

```cpp
void unlinkAll();
```

**Mục đích:** Tháo liên kết của tất cả Seat đang được Repository quản lý.

```cpp
for (int i = 0; i < repo.count(); i++)
{
    unlinkReferences(repo.getAt(i));
}
```

Hàm duyệt toàn bộ danh sách và gọi unlinkReferences() cho từng Seat.

Việc tháo liên kết không làm thay đổi số lượng Seat trong Repository.

Hàm được sử dụng trong:

- Destructor.
- Rollback load().

unlinkAll() không gọi delete và không gọi repo.clear().

Nếu một lần tháo liên kết ném ngoại lệ, vòng lặp dừng và ngoại lệ được truyền ra ngoài.

## 8. Giải thích các hàm public

### 8.1. Constructor

**Chữ ký:**

```cpp
SeatRepository(const std::string& filePath,
               IRepository<Room>& roomRepo);
```

Constructor sử dụng initializer list:

```cpp
: repo(filePath, getSeatKey, stringToSeat, seatToString),
  roomRepo(roomRepo)
```

Repository chung được khởi tạo với:

- Đường dẫn file.
- Callback lấy khóa.
- Callback chuyển TXT thành Seat.
- Callback chuyển Seat thành TXT.

roomRepo được truyền từ bên ngoài thông qua interface.

Constructor không đọc file và không tạo liên kết.

### 8.2. Destructor

**Chữ ký:**

```cpp
~SeatRepository() override;
```

Destructor thực hiện:

```cpp
unlinkAll();
```

Tháo liên kết của toàn bộ Seat trước khi thuộc tính repo bị hủy.

Nếu không tháo liên kết, Room có thể giữ con trỏ đến Seat đã được giải phóng.

```text
SeatRepository destructor
    ↓
unlinkAll()
    ↓
Tháo liên kết Seat – Room
    ↓
Repository<Seat> destructor
    ↓
Giải phóng các Seat
```

Nếu unlinkAll() ném ngoại lệ:

- In thông báo lỗi.
- Gọi terminate().

Cách xử lý này đồng bộ với ShowtimeRepository.

RoomRepository phải còn tồn tại trong suốt quá trình hủy SeatRepository.

### 8.3. Hàm load()

**Chữ ký:**

```cpp
bool load() override;
```

**Mục đích:** Đọc dữ liệu Seat từ file và thiết lập liên kết với Room.

```text
roomRepo đã load?
├─ Không → false
└─ Có
     ↓
   repo.load()
     ├─ false → false
     └─ true
          ↓
        Duyệt từng Seat
          ↓
        Tìm Room
          ↓
        seat->linkRoom(room)
          ↓
        Hoàn thành → true

Lỗi trong quá trình liên kết:
    unlinkAll()
        ↓
    repo.clear()
        ↓
    throw
```

**Bước 1: Kiểm tra dependency.**

```cpp
if (!roomRepo.isLoaded())
{
    return false;
}
```

Room phải được load trước Seat.

**Bước 2: Đọc dữ liệu.**

```cpp
if (!repo.load())
{
    return false;
}
```

Repository chung đọc file, gọi stringToSeat(), kiểm tra khóa và đưa các Seat vào danh sách.

Sau khi repo.load() thành công, các Seat đã tồn tại trong bộ nhớ nhưng chưa liên kết với Room.

Phải thực hiện bước này trước vòng lặp liên kết vì nếu chưa load thì Repository chưa có các đối tượng Seat để xử lý.

Không kiểm tra lại repo.isLoaded() vì Repository chung đã thực hiện.

**Bước 3: Tìm Room tương ứng.**

```cpp
Seat* seat = repo.getAt(i);
Room* room = roomRepo.findById(seat->getRoomId());
```

Nếu Room không tồn tại, dữ liệu Seat không đảm bảo tính toàn vẹn tham chiếu.

Hàm ném runtime_error.

**Bước 4: Thiết lập liên kết.**

```cpp
if (!seat->linkRoom(room))
{
    throw runtime_error("Khong the lien ket Seat voi Room");
}
```

Luồng gọi:

```text
SeatRepository::load()
    ↓
Seat::linkRoom()
    ↓
Room::linkSeat()
```

Room::linkSeat() kiểm tra:

- Mã phòng tương ứng.
- Liên kết hiện tại.
- Giới hạn hàng và cột.
- Trùng mã ghế.
- Trùng vị trí ghế.

SeatRepository không cần lặp lại các kiểm tra này.

Việc liên kết được thực hiện lần lượt nên các Seat đã liên kết trước đó cũng được dùng để kiểm tra trùng vị trí với Seat tiếp theo.

**Bước 5: Rollback.**

```cpp
catch (...)
{
    unlinkAll();
    repo.clear();
    throw;
}
```

Thứ tự thực hiện:

1. Tháo toàn bộ liên kết đã tạo.
2. Giải phóng dữ liệu Seat.
3. Đặt loaded về false thông qua repo.clear().
4. Ném lại ngoại lệ.

Phải tháo liên kết trước khi giải phóng Seat để tránh con trỏ treo trong Room.

repo.load() được đặt ngoài try vì Repository chung đã có cơ chế rollback riêng khi đọc file thất bại.

SeatRepository chỉ chịu trách nhiệm rollback phần liên kết do nó thực hiện.

**Giới hạn rollback:** Nếu unlinkAll() ném ngoại lệ, repo.clear() sẽ không được gọi. Ngoại lệ rollback có thể thay thế ngoại lệ ban đầu và trạng thái liên kết có thể chưa được khôi phục đầy đủ.

### 8.4. Hàm save()

**Chữ ký:**

```cpp
bool save() override;
```

```cpp
return repo.save();
```

SeatRepository gọi trực tiếp Repository chung để ghi dữ liệu xuống file.

Repository chung sử dụng callback seatToString() để chuyển từng Seat thành chuỗi.

Không cần kiểm tra loaded hoặc viết lại logic ghi file.

### 8.5. Hàm add()

**Chữ ký:**

```cpp
bool add(Seat* seat) override;
```

**Mục đích:** Thêm Seat mới và thiết lập liên kết với Room.

```text
Hai Repository đã load?
    ↓
Seat hợp lệ?
    ↓
Seat chưa liên kết?
    ↓
Khóa chưa tồn tại?
    ↓
Room tồn tại?
    ↓
try: linkRoom() && repo.add()
    ├─ true → return true
    ├─ false → unlinkReferences() → false
    └─ exception → unlinkReferences() → throw
```

**Bước 1: Kiểm tra điều kiện trước khi thêm.**

```cpp
if (!repo.isLoaded() || !roomRepo.isLoaded() ||
    !isValidSeat(seat))
```

Cần kiểm tra loaded trước vì hàm thực hiện thay đổi liên kết trước khi gọi repo.add().

isValidSeat() đồng thời kiểm tra con trỏ null.

**Bước 2: Kiểm tra liên kết hiện tại.**

```cpp
if (seat->getRoom() != nullptr)
{
    return false;
}
```

Không thêm Seat đã liên kết với Room.

Điều này tránh việc tạo lại liên kết hoặc tháo nhầm liên kết đang tồn tại khi rollback.

**Bước 3: Kiểm tra trùng khóa.**

```cpp
if (findById(getSeatKey(seat)) != nullptr)
{
    return false;
}
```

Kiểm tra trước khi tạo liên kết.

Không cần duyệt toàn bộ Repository bằng một vòng lặp riêng để kiểm tra trùng con trỏ.

**Bước 4: Tìm Room.**

```cpp
Room* room = roomRepo.findById(seat->getRoomId());
```

Nếu Room không tồn tại, trả về false.

**Bước 5: Liên kết và thêm.**

```cpp
try
{
    if (seat->linkRoom(room) && repo.add(seat))
    {
        return true;
    }
}
```

Toán tử && sử dụng cơ chế đánh giá ngắn mạch.

repo.add() chỉ được gọi khi seat->linkRoom(room) trả về true.

Nếu cả hai thao tác thành công, Seat được Repository quản lý và hàm trả về true.

**Bước 6: Rollback.**

Nếu thao tác phát sinh ngoại lệ:

```cpp
catch (...)
{
    unlinkReferences(seat);
    throw;
}
```

Nếu thao tác trả về false:

```cpp
unlinkReferences(seat);
return false;
```

Đặt rollback của trường hợp false bên ngoài try để tránh gọi rollback lần thứ hai nếu chính unlinkReferences() phát sinh ngoại lệ.

SeatRepository không delete Seat khi add thất bại.

**Giới hạn rollback:** Nếu unlinkReferences() thất bại, hàm ném ngoại lệ và không đảm bảo liên kết đã được khôi phục hoàn toàn.

### 8.6. Hàm update()

**Chữ ký:**

```cpp
bool update(const Seat& newValues) override;
```

**Mục đích:** Cập nhật loại ghế của Seat đang được quản lý.

```text
Repository đã load?
    ↓
newValues hợp lệ?
    ↓
Tìm Seat theo khóa
    ↓
Vị trí không thay đổi?
    ↓
setSeatType()
    ↓
return true
```

**Bước 1:** Kiểm tra trạng thái loaded và dữ liệu mới.

```cpp
if (!repo.isLoaded() || !isValidSeat(&newValues))
```

Repository chung không cung cấp logic update nên SeatRepository phải tự kiểm tra.

**Bước 2:** Tìm Seat cần cập nhật.

```cpp
Seat* seat = findById(newValues.getRoomId(),
                      newValues.getSeatId());
```

Tận dụng hàm overload.

Nếu không tìm thấy Seat, trả về false.

**Bước 3:** Kiểm tra vị trí.

```cpp
if (seat->getRow() != newValues.getRow() ||
    seat->getColumn() != newValues.getColumn())
```

Không cho phép thay đổi hàng hoặc cột của Seat.

**Bước 4:** Cập nhật loại ghế.

```cpp
seat->setSeatType(newValues.getSeatType());
```

Chỉ thay đổi SeatType trên đối tượng hiện tại.

Không thay thế con trỏ Seat, không thay đổi khóa và không tạo lại liên kết Room.

Setter hiện tại chỉ gán giá trị enum nên không cần snapshot hoặc hàm restoreSeat() như ShowtimeRepository.

Các ràng buộc nghiệp vụ liên quan đến lịch sử Ticket được xử lý tại Service trước khi gọi update().

### 8.7. Hàm removeById()

#### Hàm override

**Chữ ký:**

```cpp
bool removeById(const std::string& key) override;
```

**Mục đích:** Xóa Seat theo khóa kết hợp.

```text
Repository đã load?
    ↓
Tìm Seat theo khóa
    ↓
Room liên kết tồn tại?
    ↓
room->unlinkSeat(seat)
    ├─ false → return false
    └─ true
         ↓
       repo.removeById(key)
         ├─ true → return true
         └─ false
              ↓
            room->linkSeat(seat)
              ├─ true → return false
              └─ false → throw
```

**Bước 1:** Kiểm tra loaded và tìm Seat.

Nếu chưa load hoặc không tìm thấy Seat, trả về false.

**Bước 2:** Lấy Room đang liên kết.

```cpp
Room* room = seat->getRoom();
```

Nếu Room là nullptr, trả về false vì Seat không có liên kết cần thiết.

**Bước 3:** Tháo liên kết.

```cpp
if (!room->unlinkSeat(seat))
{
    return false;
}
```

Tháo liên kết trước khi giải phóng Seat.

Không sử dụng unlinkReferences() ở đây vì hàm removeById() cần trả về false khi thao tác tháo liên kết bị từ chối.

**Bước 4:** Xóa Seat khỏi Repository.

```cpp
if (!repo.removeById(key))
```

Repository chung chịu trách nhiệm xóa Seat khỏi danh sách và giải phóng bộ nhớ.

**Bước 5:** Khôi phục liên kết nếu xóa thất bại.

```cpp
if (!room->linkSeat(seat))
{
    throw runtime_error("Khong the khoi phuc lien ket Seat");
}
```

Nếu Repository chung trả về false, Seat vẫn tồn tại nên cần liên kết lại với Room.

Nếu khôi phục thành công, trả về false.

Nếu khôi phục thất bại, ném runtime_error.

**Giới hạn rollback:** Hàm xử lý trường hợp repo.removeById() trả về false. Chưa có cơ chế rollback tổng quát cho ngoại lệ phát sinh trong quá trình xóa hoặc khôi phục.

**Điều kiện nghiệp vụ trước khi gọi:**

SeatService phải đảm bảo ghế chưa từng được Ticket tham chiếu, kể cả vé đã hủy.

SeatRepository không tự kiểm tra lịch sử Ticket.

Nếu gọi trực tiếp removeById() khi Ticket vẫn giữ con trỏ Seat, việc xóa có thể tạo con trỏ treo.

#### Hàm overload

**Chữ ký:**

```cpp
bool removeById(const std::string& roomId,
                const std::string& seatId);
```

```cpp
return removeById(roomId + "|" + seatId);
```

Hàm chỉ ghép khóa và gọi hàm override.

Không viết lại logic tháo liên kết, xóa hoặc rollback.

### 8.8. Hàm findById()

#### Hàm override

**Chữ ký:**

```cpp
Seat* findById(const std::string& key) override;
```

```cpp
return repo.findById(key);
```

Tìm Seat theo khóa kết hợp.

Ví dụ:

```cpp
seatRepo.findById("ROM0001|A01");
```

Nếu không tìm thấy, trả về nullptr.

#### Hàm overload

**Chữ ký:**

```cpp
Seat* findById(const std::string& roomId,
               const std::string& seatId);
```

```cpp
return findById(roomId + "|" + seatId);
```

Cho phép sử dụng:

```cpp
seatRepo.findById("ROM0001", "A01");
```

Hai phiên bản trả về cùng một Seat nếu khóa tương ứng giống nhau.

Con trỏ trả về là con trỏ mượn, không chuyển quyền sở hữu cho bên gọi.

### 8.9. Hàm getAt()

**Chữ ký:**

```cpp
Seat* getAt(int index) override;
```

```cpp
return repo.getAt(index);
```

Trả về Seat tại vị trí index.

Repository chung kiểm tra giới hạn vị trí.

Nếu index không hợp lệ, trả về nullptr.

Không tạo bản sao Seat.

### 8.10. Hàm count()

**Chữ ký:**

```cpp
int count() override;
```

```cpp
return repo.count();
```

Trả về số lượng Seat đang được Repository quản lý.

Không đọc lại file hoặc duyệt danh sách ghế của Room.

### 8.11. Hàm isLoaded()

**Chữ ký:**

```cpp
bool isLoaded() override;
```

```cpp
return repo.isLoaded();
```

Trả về trạng thái loaded của Repository chung.

SeatRepository không có cờ loaded riêng.

File Seat rỗng vẫn có thể load thành công với count() bằng 0.

## 9. Quản lý bộ nhớ và ràng buộc Ticket

### 9.1. Quyền sở hữu Seat

| Trường hợp | Thành phần quản lý bộ nhớ |
|---|---|
| Seat vừa được tạo bên ngoài | Bên gọi |
| add() thành công | Repository<Seat> |
| add() trả về false | Bên gọi đối với Seat mới |
| load() thành công | Repository<Seat> |
| findById() hoặc getAt() | Không chuyển quyền sở hữu |
| removeById() thành công | Repository giải phóng Seat |
| Repository bị hủy | Repository giải phóng các Seat còn lại |

Nếu add() được gọi với một con trỏ đã thuộc Repository, việc trả về false không chuyển quyền sở hữu con trỏ đó cho bên gọi.

Không được delete một Seat đang được Repository quản lý.

### 9.2. Liên kết hai chiều

```text
Room
 └── Danh sách Seat*
          ↕
         Seat
          └── Room*
```

Room giữ danh sách con trỏ Seat.

Seat giữ con trỏ đến Room tương ứng.

Hai phía không sở hữu bộ nhớ của nhau thông qua các liên kết này.

Khi xóa Seat:

```text
Tháo liên kết
    ↓
Xóa khỏi Repository
    ↓
Giải phóng bộ nhớ
```

Khi hủy SeatRepository:

```text
unlinkAll()
    ↓
Repository destructor
    ↓
Giải phóng toàn bộ Seat
```

### 9.3. Thứ tự vòng đời Repository

RoomRepository phải tồn tại lâu hơn SeatRepository.

Ví dụ:

```cpp
RoomRepository roomRepo(roomFilePath);
SeatRepository seatRepo(seatFilePath, roomRepo);
```

Khi kết thúc phạm vi:

1. SeatRepository bị hủy trước.
2. Các liên kết Seat – Room được tháo.
3. RoomRepository bị hủy sau.

Các thành phần giữ con trỏ Seat, đặc biệt là TicketRepository, cũng phải được quản lý vòng đời phù hợp và không tiếp tục sử dụng Seat đã bị giải phóng.

### 9.4. Ràng buộc Ticket

Ticket có thể tham chiếu trực tiếp đến Seat.

Seat không có danh sách liên kết ngược đến Ticket.

Do đó, SeatRepository không thể tự xác định đầy đủ lịch sử vé của một ghế.

SeatService phải kiểm tra các ràng buộc trong business_rules.md trước khi gọi update() hoặc removeById().

Đặc biệt, không được xóa Seat nếu ghế đã từng được Ticket tham chiếu, kể cả Ticket đã hủy.

Không bổ sung TicketRepository vào SeatRepository vì TicketRepository phụ thuộc vào SeatRepository để truy xuất ghế.

Việc bổ sung dependency ngược có thể tạo phụ thuộc vòng.

Phần kiểm tra lịch sử Ticket sẽ được hoàn thiện khi refactor Business.

### 9.5. Giới hạn rollback

Các hàm rollback giúp khôi phục liên kết khi thao tác thất bại trong những trường hợp đã được xử lý.

Nếu chính thao tác tháo hoặc khôi phục liên kết thất bại:

- Ngoại lệ được truyền ra ngoài.
- Không khẳng định dữ liệu đã được khôi phục đầy đủ.
- Không báo thao tác thành công.

Destructor báo lỗi và gọi terminate() nếu không thể tháo liên kết an toàn.

## 10. Áp dụng Dependency Inversion Principle

### 10.1. Trước refactor

```cpp
class SeatRepository
{
private:
    Repository<Seat> repo;
    RoomRepository& roomRepo;
};
```

SeatRepository phụ thuộc trực tiếp vào lớp RoomRepository cụ thể.

### 10.2. Sau refactor

```cpp
class SeatRepository : public IRepository<Seat>
{
private:
    Repository<Seat> repo;
    IRepository<Room>& roomRepo;
};
```

SeatRepository phụ thuộc vào abstraction thay vì RoomRepository cụ thể.

Dependency được truyền thông qua constructor.

```cpp
SeatRepository(const std::string& filePath,
               IRepository<Room>& roomRepo);
```

Các thao tác với Room chỉ sử dụng hợp đồng chung:

```cpp
roomRepo.isLoaded();
roomRepo.findById(roomId);
```

Không thay đổi IRepository hoặc Repository chung.

### 10.3. Sử dụng SeatRepository thông qua interface

```cpp
IRepository<Seat>& repo = seatRepo;

repo.findById("ROM0001|A01");
repo.removeById("ROM0001|A01");
```

Interface chỉ cung cấp các hàm nhận một khóa.

Không thể gọi:

```cpp
repo.findById("ROM0001", "A01");
```

Vì hàm nhận hai tham số chỉ được khai báo trong SeatRepository.

### 10.4. Vai trò của overload trong quá trình refactor

Hai hàm overload được giữ lại để tương thích với những lớp chưa chuyển sang sử dụng IRepository<Seat>.

Khi Service hoặc Repository khác chuyển sang interface, sử dụng:

```cpp
seatRepo.findById(roomId + "|" + seatId);
```

Sau khi hoàn thành refactor DIP, có thể rà soát và loại bỏ các overload nếu không còn cần thiết.

Không ép kiểu từ IRepository về SeatRepository để gọi các hàm overload.

## 11. Nội dung chuẩn hóa

### 11.1. Header

- Kế thừa IRepository<Seat>.
- Bổ sung override cho chín hàm chung.
- Bổ sung override cho destructor.
- Thay RoomRepository& bằng IRepository<Room>&.
- Sử dụng forward declaration cho Room.
- Bổ sung hai hàm overload.
- Bổ sung unlinkReferences() và unlinkAll().
- Nhóm các hàm theo trách nhiệm.
- Chuẩn hóa namespace và căn chỉnh tham số.

### 11.2. File triển khai

| Hàm | Nội dung thay đổi |
|---|---|
| Constructor | Áp dụng Dependency Injection qua interface |
| Destructor | Sử dụng unlinkAll(), xử lý ngoại lệ |
| isValidSeat() | Sử dụng Validator::isValidText() |
| getSeatKey() | Giữ nguyên logic |
| stringToSeat() | Giữ nguyên logic |
| seatToString() | Giữ nguyên logic |
| unlinkReferences() | Bổ sung helper tháo liên kết |
| unlinkAll() | Bổ sung helper tháo toàn bộ liên kết |
| load() | Gộp kiểm tra và liên kết vào một vòng lặp |
| save() | Giữ nguyên logic |
| add() | Chuẩn hóa kiểm tra và rollback |
| update() | Giữ nguyên logic |
| removeById() | Bổ sung khôi phục liên kết |
| findById() | Override và overload |
| getAt() | Giữ nguyên logic |
| count() | Giữ nguyên logic |
| isLoaded() | Giữ nguyên logic |

### 11.3. Thành phần giữ nguyên

- Model Seat và Room.
- Định dạng file TXT.
- Khóa kết hợp roomId và seatId.
- Repository chung.
- IRepository chung.
- Quyền sở hữu Seat.
- Logic chỉ cập nhật SeatType.
- Các ràng buộc vị trí ghế trong Room.

### 11.4. Thay đổi cách xử lý lỗi

Phiên bản load() cũ trả về false khi Room không tồn tại hoặc liên kết thất bại.

Phiên bản mới ném runtime_error trong các trường hợp này, đồng bộ với ShowtimeRepository.

Các tầng gọi cần xử lý ngoại lệ phù hợp.

## 12. Kế hoạch kiểm thử

**File đã triển khai và chạy:**

`tests/test_seat_repository.cpp`

Bài test sử dụng file TXT riêng, dọn file sau khi chạy và không thay đổi dữ liệu chính. Đã biên dịch C++17 cùng các .cpp trong src/ trừ main.cpp và chạy ngày 26/11/2026: **Passed: 81, Failed: 0, Total: 81**. Kết quả chỉ xác nhận các trường hợp thực sự có trong bài test.

### 12.1. Load và save

- [x] Load khi RoomRepository chưa load.
- [x] Load file hợp lệ.
- [x] Load file rỗng.
- [x] Load file không tồn tại.
- [x] Load nhiều lần.
- [x] Load dữ liệu thiếu hoặc thừa trường.
- [x] Load roomId sai định dạng.
- [x] Load seatId rỗng hoặc không hợp lệ.
- [x] Load hàng hoặc cột sai định dạng.
- [x] Load hàng hoặc cột không dương.
- [x] Load hàng hoặc cột vượt kích thước Room.
- [x] Load SeatType không hợp lệ.
- [x] Load Seat tham chiếu Room không tồn tại.
- [x] Load trùng khóa kết hợp.
- [x] Load trùng vị trí trong cùng Room.
- [x] Load cùng seatId nhưng khác Room.
- [x] Kiểm tra liên kết hai chiều sau load.
- [x] Kiểm tra rollback khi một Seat lỗi sau khi các Seat trước đã liên kết.
- [x] Kiểm tra loaded và count sau rollback thành công.
- [x] Sửa dữ liệu lỗi và load lại.
- [x] Save trước khi load.
- [x] Save dữ liệu hợp lệ.
- [x] Save file rỗng.
- [x] Kiểm tra đúng định dạng TXT.
- [x] Load lại dữ liệu sau save.

### 12.2. Add và update

- [x] Add trước khi load.
- [x] Add nullptr.
- [x] Add Seat hợp lệ.
- [x] Add Seat có dữ liệu sai.
- [x] Add Seat tham chiếu Room không tồn tại.
- [x] Add Seat vượt kích thước Room.
- [x] Add trùng khóa.
- [x] Add trùng vị trí.
- [x] Add cùng seatId nhưng khác Room.
- [x] Add Seat đã liên kết.
- [x] Add lại con trỏ đang được quản lý.
- [x] Kiểm tra liên kết hai chiều sau add.
- [x] Kiểm tra count và liên kết sau add thất bại.
- [x] Kiểm tra quyền sở hữu Seat khi add thất bại.
- [ ] Giả lập repo.add() thất bại sau khi liên kết thành công.
- [ ] Kiểm tra rollback khi phát sinh ngoại lệ trong add().
- [x] Update trước khi load.
- [x] Update Seat không tồn tại.
- [x] Update dữ liệu không hợp lệ.
- [x] Update SeatType hợp lệ.
- [x] Từ chối thay đổi hàng hoặc cột.
- [x] Kiểm tra địa chỉ Seat không thay đổi sau update.
- [x] Kiểm tra liên kết Room không thay đổi sau update.

### 12.3. Remove, truy vấn và vòng đời

- [x] Remove trước khi load.
- [x] Remove Seat không tồn tại.
- [x] Remove Seat theo khóa kết hợp.
- [x] Remove Seat bằng hàm overload.
- [x] Kiểm tra count sau remove.
- [x] Kiểm tra Room không còn giữ Seat đã xóa.
- [x] Kiểm tra các Seat khác không bị ảnh hưởng.
- [ ] Giả lập repo.removeById() trả về false.
- [ ] Kiểm tra khôi phục liên kết khi xóa thất bại.
- [ ] Giả lập khôi phục liên kết thất bại.
- [x] FindById theo khóa kết hợp.
- [x] FindById bằng hai tham số.
- [x] Kiểm tra hai hàm findById trả cùng con trỏ.
- [x] FindById không tồn tại.
- [x] GetAt với index hợp lệ.
- [x] GetAt với index âm hoặc vượt giới hạn.
- [x] Count và isLoaded.
- [x] Sử dụng chín hàm chung qua IRepository<Seat>.
- [x] Destructor tháo toàn bộ liên kết Seat – Room.
- [ ] Giả lập lỗi tháo liên kết trong destructor.
- [ ] Kiểm tra hồi quy các Repository liên quan.

Rollback load đã kiểm chứng khi Seat đầu tiên liên kết thành công, Seat tiếp theo thiếu Room hoặc trùng/vượt vị trí; loaded=false, count=0, Room không giữ Seat vừa nạp và không tháo liên kết có sẵn. Các trường hợp giả lập lỗi nội bộ cần cơ chế phù hợp; chưa đánh dấu PASS cho nhánh không thể kích hoạt.

### 12.4. Kiểm thử nghiệp vụ sau khi refactor Service

- [ ] Kiểm tra Permission.
- [ ] Kiểm tra ràng buộc thay đổi ghế theo lịch sử Ticket.
- [ ] Từ chối xóa Seat từng được Ticket SOLD tham chiếu.
- [ ] Từ chối xóa Seat từng được Ticket CANCELLED tham chiếu.
- [ ] Chỉ gọi removeById() khi thỏa mãn điều kiện nghiệp vụ.

Các kiểm thử này thuộc phạm vi Service, không phải kết quả kiểm thử SeatRepository.

### 12.5. Biên dịch và chạy kiểm thử

Sử dụng C++17 và PowerShell; biên dịch cùng các `.cpp` trong `src/`, trừ `main.cpp`:

```powershell
$sources = @(rg --files src -g '*.cpp' | Where-Object { $_ -notmatch '(^|[\\/])main\.cpp$' })
g++ -std=c++17 -I src tests/test_seat_repository.cpp $sources -o test_seat_repository.exe
```

Chạy:

```powershell
.\test_seat_repository.exe
```

**Kết quả kiểm thử thực tế (26/11/2026):** **Passed: 81, Failed: 0, Total: 81**. Các nhánh rollback do ngoại lệ nội bộ, lỗi unlink/destructor và kiểm hồi quy module liên quan chưa được xác nhận.

## 13. Tiến độ triển khai

- [x] Rà soát SeatRepository hiện tại.
- [x] Rà soát Model Seat và Room.
- [x] Thống nhất kiến trúc DIP.
- [x] Thống nhất khóa kết hợp.
- [x] Thống nhất override và overload.
- [x] Thống nhất các hàm cơ bản.
- [x] Thống nhất các hàm chuyển đổi và validation.
- [x] Thống nhất destructor và helper liên kết.
- [x] Thống nhất load().
- [x] Thống nhất add().
- [x] Thống nhất update().
- [x] Hoàn thiện phương án removeById().
- [x] Viết mã nguồn đề xuất .h/.cpp.
- [x] Giải thích các hàm theo form ShowtimeRepository.
- [x] Ghi nhận giới hạn rollback và ràng buộc Ticket.
- [x] Rà soát toàn bộ Markdown lần cuối.
- [x] CHỐT MARKDOWN.
- [x] Triển khai SeatRepository.h và SeatRepository.cpp.
- [x] Đối chiếu mã nguồn với Markdown.
- [x] Viết test_seat_repository.cpp.
- [x] Biên dịch và chạy kiểm thử trong phạm vi test_seat_repository.cpp: Passed 81, Failed 0, Total 81 (26/11/2026).
- [x] Ghi nhận kết quả kiểm thử thực tế và các nhánh chưa kiểm chứng.
- [x] Cập nhật tài liệu tiến độ và phân công công việc.
- [ ] Kiểm thử các nhánh lỗi nội bộ/rollback do ngoại lệ và kiểm hồi quy liên quan.
- [ ] Hoàn thiện ràng buộc Ticket tại SeatService.

## 14. Kết luận

SeatRepository được thiết kế lại theo Dependency Inversion Principle, kế thừa IRepository<Seat>, sử dụng Repository<Seat> bằng composition và nhận IRepository<Room>& thông qua constructor.

Lớp giữ nguyên khóa kết hợp và định dạng dữ liệu TXT.

Hai hàm overload được bổ sung để duy trì khả năng tương thích với các lớp chưa hoàn thành refactor DIP.

Các hàm load(), add() và removeById() được chuẩn hóa theo ShowtimeRepository, tận dụng validation trong Model và sử dụng các helper quản lý liên kết để giảm code lặp lại.

Hàm update() giữ nguyên logic, chỉ cho phép cập nhật SeatType.

SeatRepository chịu trách nhiệm đảm bảo tính hợp lệ của dữ liệu và liên kết Seat – Room. Các ràng buộc nghiệp vụ liên quan đến lịch sử Ticket thuộc trách nhiệm của Service.

Bản thiết kế đã CHỐT MARKDOWN, triển khai đúng hai khối mã ở mục 5 và chuẩn hóa comment. Bài test Repository đạt Passed 81, Failed 0, Total 81 trong phạm vi đã ghi; các nhánh lỗi nội bộ và điều kiện lịch sử Ticket tại SeatService vẫn chưa hoàn thành.
