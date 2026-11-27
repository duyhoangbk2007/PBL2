# RoomService — Thiết kế refactor

**Trạng thái:** CHỐT MARKDOWN, đã triển khai và kiểm thử thành công trong phạm vi `tests/test_room_service.cpp` ngày 27/11/2026; RoomService hoàn thành refactor DIP, DI và Permission-only trong phạm vi này. Giữ 8 hàm public cũ, thêm `setRoomActive()` thành 9 hàm; `.h/.cpp` khớp Markdown 100%.

**Mốc:** [5d663e3](https://github.com/duyhoangbk2007/PBL2/commit/5d663e38110b305524338a21a85ebed1a61f42f6) (Hoàng); mẫu tài liệu [e8c26bd](https://github.com/duyhoangbk2007/PBL2/commit/e8c26bd00f427ffb612ec0f9b4078b5c406e766a), nhánh `refactor/Kiet`.

## 1. Tổng quan

RoomService thuộc Business Logic, kiểm tra quyền, sinh ID, xử lý nghiệp vụ phòng và điều phối Repository. Không trực tiếp đọc ghi TXT. Giữ 8 hàm public hiện có, bổ sung setRoomActive; tổng 9 hàm public và constructor. Bổ sung getShowtimeAt vào Room Model để duyệt suất chiếu liên kết.

Nguồn đối chiếu: AGENTS.md, docs/architecture/validation_rules.md, docs/overview/business_rules.md, src/business/RoomService.h/.cpp, AuthService.h, src/data_access/IRepository.h, RoomRepository.h/.cpp, src/models/Room.h/.cpp và Movie.cpp tại mốc Hoàng; MovieService.md tại mốc Kiệt.

## 2. Kiến trúc

```text
GUI -> RoomService -> IRepository<Room> <- RoomRepository
           |                                    |
           v                                    v
       AuthService                       Repository<Room>
```

Service nhận tham chiếu qua constructor (DI), phụ thuộc interface (DIP). Repository quản lý Room, validation và toàn vẹn liên kết; Service kiểm tra Permission và nghiệp vụ. Không sở hữu hai dependency. Không thay đổi AuthService.

## 3. Thuộc tính và API

| Thuộc tính | Kiểu | Vai trò |
|---|---|---|
| roomRepo | IRepository<Room>& | CRUD, truy vấn, load/save qua interface |
| authService | AuthService& | Kiểm tra Permission |

| Hàm | Permission |
|---|---|
| Constructor, load, save | Không kiểm tra trực tiếp; giữ hợp đồng điều phối cũ |
| addRoom | ROOM_ADD |
| updateRoom, setRoomActive | ROOM_UPDATE |
| removeRoom | ROOM_DELETE |
| findRoomById, getRoomAt, getRoomCount | ROOM_VIEW |

Thiếu quyền: truyền ngoại lệ của requirePermission, không chuyển thành false. Lỗi nghiệp vụ: false; truy vấn không có kết quả: nullptr theo API. Không tự save sau CRUD. Con trỏ truy vấn là mượn, không delete và không dùng sau khi đối tượng bị xóa hoặc reload làm mất hiệu lực. getShowtimeAt là API Model, không tự kiểm tra quyền.

## 4. Mã nguồn đã chốt

### 4.1. Bổ sung Room.h

Thêm ngay sau getShowtimeCount(), giữ nguyên phần còn lại:

```cpp
const Showtime* getShowtimeAt(int index);
```

### 4.2. Bổ sung Room.cpp

Thêm định nghĩa sau getShowtimeCount(), giữ nguyên phần còn lại:

```cpp
const Showtime* Room::getShowtimeAt(int index)
{
    if (index < 0 || index >= showtimes.nodeCount())
    {
        return nullptr;
    }
    return showtimes.getAt(index)->data;
}
```

### 4.3. RoomService.h

```cpp
#ifndef ROOM_SERVICE_H
#define ROOM_SERVICE_H

#include <string>
#include "../data_access/IRepository.h"
#include "../models/Room.h"
#include "AuthService.h"

class RoomService
{
private:
    IRepository<Room>& roomRepo;
    AuthService& authService;

public:
    RoomService(IRepository<Room>& roomRepo,
                AuthService& authService);

    // Quan ly du lieu
    bool load();
    bool save();

    // Quan ly phong
    bool addRoom(Room* room);
    bool updateRoom(const Room& newValues);
    bool removeRoom(const std::string& roomId);
    bool setRoomActive(const std::string& roomId, bool active);

    // Tra cuu phong
    const Room* findRoomById(const std::string& roomId);
    const Room* getRoomAt(int index);
    int getRoomCount();
};

#endif
```

### 4.4. RoomService.cpp

```cpp
#include "RoomService.h"
#include "../models/Showtime.h"
#include "../utils/DateTime.h"
#include "../utils/IdGenerator.h"
#include <vector>
using namespace std;

RoomService::RoomService(IRepository<Room>& roomRepo,
                         AuthService& authService)
    : roomRepo(roomRepo), authService(authService)
{
}

bool RoomService::load()
{
    return roomRepo.load();
}

bool RoomService::save()
{
    return roomRepo.save();
}

bool RoomService::addRoom(Room* room)
{
    authService.requirePermission("ROOM_ADD");

    if (!roomRepo.isLoaded() || room == nullptr)
    {
        return false;
    }

    vector<string> existingIds;
    for (int i = 0; i < roomRepo.count(); i++)
    {
        const Room* current = roomRepo.getAt(i);
        if (current == nullptr)
        {
            return false;
        }
        existingIds.push_back(current->getRoomId());
    }

    string roomId = IdGenerator::generateNextId("ROM", existingIds);
    Room* newRoom = new Room(roomId,
                             room->getName(),
                             room->getRowCount(),
                             room->getColumnCount(),
                             room->isActive());
    try
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
        throw;
    }
    return true;
}

bool RoomService::updateRoom(const Room& newValues)
{
    authService.requirePermission("ROOM_UPDATE");

    if (!roomRepo.isLoaded())
    {
        return false;
    }
    const Room* room = roomRepo.findById(newValues.getRoomId());
    if (room == nullptr)
    {
        return false;
    }
    if (room->isActive() != newValues.isActive())
    {
        return false;
    }
    bool sizeChanged =
        room->getRowCount() != newValues.getRowCount() ||
        room->getColumnCount() != newValues.getColumnCount();

    if (sizeChanged && room->getShowtimeCount() > 0)
    {
        return false;
    }
    return roomRepo.update(newValues);
}

bool RoomService::removeRoom(const string& roomId)
{
    authService.requirePermission("ROOM_DELETE");
    return roomRepo.removeById(roomId);
}

bool RoomService::setRoomActive(const string& roomId, bool active)
{
    authService.requirePermission("ROOM_UPDATE");

    if (!roomRepo.isLoaded())
    {
        return false;
    }
    Room* room = roomRepo.findById(roomId);
    if (room == nullptr)
    {
        return false;
    }
    if (room->isActive() == active)
    {
        return true;
    }
    if (!active)
    {
        string now = DateTime::getCurrentDateTime();
        for (int i = 0; i < room->getShowtimeCount(); i++)
        {
            const Showtime* showtime = room->getShowtimeAt(i);
            if (showtime == nullptr)
            {
                return false;
            }
            // Khong ngung hoat dong neu con suat chieu tuong lai
            if (showtime->getStatus() == ShowtimeStatus::SCHEDULED &&
                DateTime::isBefore(now, showtime->getStartAt()))
            {
                return false;
            }
        }
    }

    Room updated(room->getRoomId(),
                 room->getName(),
                 room->getRowCount(),
                 room->getColumnCount(),
                 active);
    return roomRepo.update(updated);
}

const Room* RoomService::findRoomById(const string& roomId)
{
    authService.requirePermission("ROOM_VIEW");
    return roomRepo.findById(roomId);
}

const Room* RoomService::getRoomAt(int index)
{
    authService.requirePermission("ROOM_VIEW");
    return roomRepo.getAt(index);
}

int RoomService::getRoomCount()
{
    authService.requirePermission("ROOM_VIEW");
    return roomRepo.count();
}
```

Header guard chống include lặp. Header include trực tiếp string, IRepository, Room và AuthService; không using namespace std. CPP include Showtime để đọc trạng thái suất, DateTime cho ngày giờ, IdGenerator sinh mã và vector chứa ID; using namespace std chỉ đặt trong CPP.

## 5. Giải thích từng hàm

### 5.1. getShowtimeAt() — Room Model

**Tổng quan:** Truy vấn suất chiếu liên kết theo index bắt đầu từ 0, trả nullptr nếu ngoài phạm vi. Không chuyển quyền sở hữu.

**Luồng hoạt động:**

```text
Index hợp lệ? -- không --> nullptr
  | có -> lấy node -> trả data
```

**Giải thích code:** nodeCount trả số node; điều kiện OR loại index âm hoặc index >= số node. getAt trả node, data là Showtime* chuyển thành const Showtime*. Giả định danh sách hợp lệ, không thay đổi link/unlink. Hàm thành viên chưa có hậu tố const, giống Movie Model.

### 5.2. Constructor

**Tổng quan:** Nhận hai tham chiếu bắt buộc, không trả giá trị, không load dữ liệu.

**Luồng hoạt động:**

```text
Nhận repo và auth -> khởi tạo hai tham chiếu -> kết thúc.
```

**Giải thích code:** IRepository<Room>& chấp nhận implementation tương thích. Danh sách khởi tạo sau dấu `:` gắn roomRepo và authService với đối tượng bên ngoài. Thân rỗng vì không có nghiệp vụ khởi tạo khác.

### 5.3. load()

**Tổng quan:** Không tham số, trả kết quả nạp dữ liệu của repo.

**Luồng hoạt động:**

```text
Gọi repo.load -> trả kết quả hoặc truyền exception.
```

**Giải thích code:** return roomRepo.load() gọi đa hình qua interface; không tự đọc TXT, không tự load dependency, không kiểm tra quyền theo contract hiện tại.

### 5.4. save()

**Tổng quan:** Không tham số, trả kết quả lưu của repo.

**Luồng hoạt động:**

```text
Gọi repo.save -> trả kết quả hoặc truyền exception.
```

**Giải thích code:** Repository chịu trách nhiệm kiểm tra và serialize dữ liệu. Service chỉ chuyển tiếp lời gọi.

### 5.5. addRoom(Room* room)

**Tổng quan:** Thêm phòng với ID sinh mới; true nếu thành công, false khi bị từ chối; lỗi quyền và exception được truyền ra. Input vẫn thuộc caller.

**Luồng hoạt động:**

```text
Quyền -> loaded và input hợp lệ? -> Không: false; Có: duyệt ID -> node null? Có: false; Không: sinh ID -> tạo Room -> add -> thành công: true; false: delete rồi false; exception: delete rồi throw.
```

**Giải thích code:** requirePermission kiểm tra ROOM_ADD trước mọi thao tác. `!isLoaded() || room == nullptr` dùng OR ngắn mạch để từ chối trạng thái chưa load hoặc input null. vector<string> chứa ID hiện có. Vòng for chạy từ 0 đến count-1; getAt trả con trỏ mượn, const Room* chỉ đọc; null thì dừng. push_back sao chép ID vào vector. generateNextId nhận tiền tố ROM và danh sách ID; không sửa input. new Room cấp phát đối tượng độc lập với các giá trị getter của input, không sao chép liên kết. try gọi add; nếu false thì delete đối tượng chưa chuyển sở hữu. catch (...) bắt mọi exception từ add, giải phóng rồi throw lại lỗi gốc. Khi add thành công, repo sở hữu đối tượng, Service không delete. Không tự tạo ghế hay lưu TXT.

### 5.6. updateRoom(const Room& newValues)

**Tổng quan:** Cập nhật phòng theo ID trong newValues, không đổi active; không đổi kích thước khi có suất chiếu. true/false theo kết quả repo, exception truyền ra.

**Luồng hoạt động:**

```text
Quyền -> loaded? Không: false -> tìm ID -> null? Có: false -> active khác? Có: false -> kích thước khác và có Showtime? Có: false -> repo.update -> trả kết quả.
```

**Giải thích code:** Tham chiếu const tránh sao chép và sửa input. requirePermission ROOM_UPDATE. Kiểm tra loaded trước khi đọc. findById tìm phòng hiện tại; const Room* hợp lệ vì mọi getter dùng ở đây đều const. So sánh active để bắt buộc dùng setRoomActive. sizeChanged là OR của hai phép so sánh hàng/cột. Nếu có thay đổi và getShowtimeCount > 0 thì từ chối, kể cả suất chiếu quá khứ (giữ nghiệp vụ cũ). Cuối cùng repo.update kiểm tra dữ liệu, giới hạn ghế khi thu nhỏ và cập nhật đối tượng gốc; Service không sửa trực tiếp.

### 5.7. removeRoom(const string& roomId)

**Tổng quan:** Xóa theo ID nếu đủ quyền và không còn liên kết. Trả kết quả repo.

**Luồng hoạt động:**

```text
Quyền ROOM_DELETE -> repo.removeById -> trả kết quả.
```

**Giải thích code:** const string& tránh sao chép ID. requirePermission chặn người thiếu quyền. Repository kiểm tra tồn tại, liên kết Seat/Showtime và thực hiện xóa; không lặp kiểm tra tại Service.

### 5.8. setRoomActive(const string& roomId, bool active)

**Tổng quan:** Bật/tắt phòng theo ID. Yêu cầu ROOM_UPDATE; không tắt nếu có suất chiếu SCHEDULED với startAt lớn hơn thời điểm hiện tại. true nếu trạng thái đã đúng hoặc cập nhật thành công; false nếu chưa load, không tìm thấy, liên kết null hoặc vi phạm điều kiện. Exception truyền ra.

**Luồng hoạt động:**

```text
Start
  -> requirePermission(ROOM_UPDATE)
  -> loaded? -- No --> false
  -> findById -> null? -- Yes --> false
  -> same active? -- Yes --> true
  -> active == false?
       Yes -> lay now -> duyet Showtime
              -> null? -- Yes --> false
              -> SCHEDULED va now < startAt? -- Yes --> false
              -> het danh sach
       No  -> bo qua duyet
  -> tao Room updated
  -> roomRepo.update(updated)
  -> return ket qua
```

**Giải thích code:** requirePermission đặt trước cả nhánh không thay đổi trạng thái. Kiểm tra loaded, tìm phòng, từ chối null. Dùng Room* thay vì const Room* vì getShowtimeAt chưa là hàm thành viên const, nhưng không sửa trực tiếp Room. Nếu active đã đúng, trả true không gọi update. `if (!active)` chỉ kiểm tra khi tắt phòng. getCurrentDateTime lấy một mốc thời gian chung cho cả vòng lặp. getShowtimeCount trả số liên kết; getShowtimeAt(i) trả con trỏ const Showtime*, không cho sửa suất chiếu qua con trỏ. Null được xử lý an toàn bằng false. Điều kiện AND chỉ chặn suất SCHEDULED chưa bắt đầu; suất CANCELLED hoặc đã đến/qua startAt không bị chặn bởi điều kiện này, đồng bộ MovieService. Tạo Room updated trên stack với ID, tên, kích thước giữ nguyên, chỉ thay active. Không sao chép danh sách liên kết. Repository cập nhật các trường trên đối tượng gốc, giữ liên kết. return truyền kết quả update; đối tượng tạm tự hủy khi ra khỏi scope. Không tự lưu file.

### 5.9. findRoomById(const string& roomId)

**Tổng quan:** Truy vấn phòng theo ID, trả const Room* hoặc nullptr theo repo.

**Luồng hoạt động:**

```text
ROOM_VIEW -> findById -> trả con trỏ.
```

**Giải thích code:** Tham chiếu const cho ID; requirePermission bảo vệ truy vấn. Kết quả Room* của interface được chuyển thành const Room*, không chuyển quyền sở hữu.

### 5.10. getRoomAt(int index)

**Tổng quan:** Truy vấn theo index bắt đầu từ 0, trả const Room* hoặc nullptr theo repo.

**Luồng hoạt động:**

```text
ROOM_VIEW -> getAt -> trả kết quả.
```

**Giải thích code:** index truyền theo giá trị; Repository kiểm tra phạm vi. Service không truy cập danh sách nội bộ. Con trỏ trả về chỉ đọc và thuộc repo.

### 5.11. getRoomCount()

**Tổng quan:** Không tham số, trả số phòng theo repo.

**Luồng hoạt động:**

```text
ROOM_VIEW -> count -> trả số lượng.
```

**Giải thích code:** requirePermission kiểm tra quyền xem, count gọi đa hình qua interface; không lọc theo active, giữ hành vi cũ.

## 6. Nội dung thay đổi và điểm cần chốt

- Chỉ bổ sung getShowtimeAt vào Room.h/cpp; cập nhật models.md tương ứng và phối hợp Hoàng tránh xung đột nhánh.
- Không sửa RoomRepository, AuthService, schema TXT hoặc Service khác.
- setRoomActive là API mới được người dùng đồng ý; không thêm permission mới.
- Không tự tạo/xóa Seat khi đổi kích thước; giữ hành vi hiện tại.
- Quy tắc thời gian dùng so sánh nghiêm ngặt now < startAt giống MovieService, không mở rộng sang suất đang diễn ra.
- ID có thể được tái sử dụng theo IdGenerator hiện hành; chưa thay đổi chính sách ID.
- Thay RoomRepository& bằng IRepository<Room>&; dùng const Room* ở các vị trí chỉ đọc, riêng setRoomActive dùng Room* vì getShowtimeAt chưa const. Đồng bộ thứ tự hàm, comment và khoảng trắng theo MovieService.
- Giữ updateRoom chặn đổi kích thước nếu có bất kỳ Showtime nào; Repository kiểm tra vị trí Seat khi thu nhỏ.
- Phải nạp và liên kết đầy đủ Showtime trước khi tắt phòng; Service không tự xác minh việc nạp dependency này.
- addRoom giữ input thuộc caller; giả định add không ném ngoại lệ sau khi nhận quyền sở hữu bản mới.
- load/save giữ API điều phối không kiểm tra quyền trực tiếp.

## 7. Kiểm thử

`tests/test_room_service.cpp` đã biên dịch C++17 cùng toàn bộ `.cpp` phù hợp trong `src/` trừ `main.cpp` và chạy ngày 27/11/2026: **Passed: 43, Failed: 0, Total: 43**. Dữ liệu TXT test đặt trong thư mục tạm riêng và được dọn sau khi chạy.

- [x] Constructor qua `IRepository<Room>&`; gọi đủ 9 hàm public trong bài test.
- [x] Permission-only: chưa đăng nhập, thiếu quyền, ADMIN thiếu quyền, STAFF được cấp quyền, tài khoản inactive.
- [x] Load/save: TXT riêng, file rỗng, hợp lệ, thiếu, sai định dạng/boolean, load lặp, save trước load, save rồi reload.
- [x] Add: ID ROM đầu/tiếp theo/hết dải, input không đổi, null, chưa load, dữ liệu sai; Repository giả lập trả false/ném exception và caller giữ input.
- [x] Update: đổi tên/kích thước hợp lệ, giữ ID và địa chỉ, dữ liệu sai, đổi active bị từ chối, có Showtime không đổi kích thước, thu nhỏ vượt vị trí Seat.
- [x] Remove: ID không tồn tại, còn Seat/Showtime bị chặn, không liên kết xóa được.
- [x] Model `getShowtimeAt()`: danh sách rỗng, index âm/bằng count/hợp lệ, sau link/unlink.
- [x] `setRoomActive()`: chưa load, ID thiếu, thiếu quyền, cùng trạng thái, bật lại, tắt khi không có suất, SCHEDULED tương lai bị chặn, CANCELLED tương lai và SCHEDULED quá khứ được phép, giữ trường và liên kết; Repository giả lập `update()` trả false.
- [x] Query: ID/index hợp lệ và sai, count, quyền xem.
- [ ] Chưa kiểm thử hồi quy riêng RoomRepository, MovieService và GUI/caller trong lần chạy này.

- [x] Đã kiểm tra trạng thái đã đúng không gọi `update()` bằng Repository giả lập và giữ liên kết Seat/Showtime trong ca bật/tắt.
- [ ] Chưa kiểm thử `startAt` bằng đúng thời điểm hiện tại (cần kiểm soát đồng hồ), lỗi ghi file, tích hợp nạp Showtime thực tế và hồi quy GUI/caller.

PowerShell (loại trừ thêm GUI nếu không tương thích môi trường test):

```powershell
$src = Get-ChildItem src -Recurse -Filter *.cpp | Where-Object { $_.Name -ne 'main.cpp' } | ForEach-Object { $_.FullName }
g++ -std=c++17 -I src tests/test_room_service.cpp $src -o test_room_service.exe
if ($LASTEXITCODE -eq 0) { .\test_room_service.exe }
```

## 8. Bàn giao

Đã CHỐT MARKDOWN và bổ sung đúng hai đoạn `getShowtimeAt()` vào `Room.h/.cpp`; `RoomService.h/.cpp` khớp 100% với hai khối code đã chốt. Biên dịch C++17 cùng các `.cpp` phù hợp trong `src/` thành công; `tests/test_room_service.cpp` chạy ngày 27/11/2026 đạt **Passed: 43, Failed: 0, Total: 43**. Hoàn thành RoomService trong phạm vi đã triển khai và kiểm thử; các giới hạn còn lại ghi ở mục 7. Không sửa Repository, schema TXT hoặc Service khác. Không tự commit/push/merge.

Đã xác minh biên dịch và kết quả test thực tế; chưa chỉnh sửa GitHub. Mẫu trình bày lấy từ MovieService.md tại e8c26bd; logic RoomService dựa trên mốc 5d663e3 và các bổ sung đã thống nhất.
