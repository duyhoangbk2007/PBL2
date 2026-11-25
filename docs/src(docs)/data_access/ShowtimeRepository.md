# ShowtimeRepository

## 1. Tổng quan

`ShowtimeRepository` quản lý dữ liệu suất chiếu ở tầng Data Access. Bản thiết kế này kế thừa `IRepository<Showtime>`, giữ `Repository<Showtime>` bằng composition và nhận `IRepository<Movie>&`, `IRepository<Room>&` qua constructor. Lớp kiểm tra dữ liệu Showtime, tham chiếu Movie/Room, lịch cùng phòng và liên kết hai chiều; không phân quyền hoặc tự ghi TXT sau mỗi thao tác.

**File liên quan:** `src/data_access/ShowtimeRepository.h/.cpp`, `src/data_access/IRepository.h`, `src/data_access/Repository.h`, `src/models/Showtime.h`, `src/models/Movie.h`, `src/models/Room.h`, `src/utils/Validator.h`, `src/utils/DateTime.h`.

**Trạng thái:** Bản chuẩn hóa Markdown **đang chờ review và CHỐT MARKDOWN**. Mã nguồn `.h/.cpp` ở mục 5 là mã **dự kiến**, chưa triển khai các chỉnh sửa định dạng và `override` của bản này. Logic hiện tại của `load()`, `update()`, `removeById()` và rollback được giữ nguyên; chưa xác nhận kết quả kiểm thử cho bản chuẩn hóa.

## 2. Kiến trúc

### 2.1. Quan hệ giữa các lớp

```text
Business Service → IRepository<Showtime> ← ShowtimeRepository
                                             │ chứa
                                             ▼
                                      Repository<Showtime>
                                             │ tham chiếu qua interface
                                             ├── IRepository<Movie> ← MovieRepository
                                             └── IRepository<Room>  ← RoomRepository
```

`ShowtimeRepository` triển khai chín hàm của `IRepository<Showtime>`. `Repository<Showtime>` quản lý danh sách con trỏ, trạng thái loaded, ID chính và đọc/ghi TXT; nó không kế thừa interface. Hai Repository tham chiếu phải sống lâu hơn ShowtimeRepository để destructor có thể gỡ liên kết.

### 2.2. Quan hệ với IRepository và Repository

`IRepository<Showtime>` là hợp đồng chín thao tác chung, cho phép Service nhận dependency qua interface. `ShowtimeRepository` triển khai hợp đồng và sở hữu `Repository<Showtime>` bằng composition. Hai tham chiếu `IRepository<Movie>&` và `IRepository<Room>&` là dependency mượn, không chuyển quyền sở hữu. `Repository<Showtime>` quản lý danh sách, ID và TXT; ShowtimeRepository giữ các ràng buộc riêng của suất chiếu.

### 2.3. Trách nhiệm validation

- `Repository<Showtime>` kiểm tra loaded, null/trùng con trỏ khi thêm, ID rỗng/trùng, tồn tại khi xóa và lỗi file.
- ShowtimeRepository kiểm tra thuộc tính Showtime, tham chiếu Movie/Room, trạng thái hoạt động của chúng đối với suất `SCHEDULED`, trùng lịch, liên kết hai chiều và rollback trước/sau khi gọi lớp lưu trữ chung.
- ShowtimeService kiểm tra Permission, suất đã bắt đầu, trạng thái, lịch sử Ticket/Invoice, lịch chiếu thuộc khoảng phát hành và thời lượng phim trước khi gọi `update()`. Repository không tự kiểm tra lại các điều kiện nghiệp vụ này.
- Trước `removeById()`, ShowtimeService phải xác nhận suất chưa từng có Ticket, kể cả vé đã hủy, và không có Invoice tham chiếu. Phần kiểm tra này chờ refactor TicketRepository và InvoiceRepository; ShowtimeRepository không nhận thêm hai dependency đó.

## 3. Định dạng dữ liệu

Mỗi dòng TXT gồm đúng sáu trường, không có dòng tiêu đề:

```text
showtimeId|movieId|roomId|startAt|endAt|status
```

| Trường | Yêu cầu |
|---|---|
| `showtimeId` | ID tiền tố `SHW`; duy nhất trong Repository. |
| `movieId`, `roomId` | ID tiền tố `MOV`, `ROM`; tham chiếu phải tồn tại khi load/add. |
| `startAt`, `endAt` | DateTime hợp lệ; `startAt < endAt`. |
| `status` | Chỉ `SCHEDULED` hoặc `CANCELLED`. |

`DateTime::isOverlapping()` tính cả khoảng nghỉ 15 phút. Chỉ hai suất `SCHEDULED` cùng phòng mới bị coi là trùng lịch. Movie và Room phải load trước Showtime. Parser không thay đổi schema, ID hoặc cơ chế lưu TXT.

## 4. Thiết kế lớp

### 4.1. Thuộc tính

| Thuộc tính | Kiểu | Vai trò |
|---|---|---|
| `repo` | `Repository<Showtime>` | Sở hữu Showtime sau khi thêm/load thành công. |
| `movieRepo` | `IRepository<Movie>&` | Tra Movie và kiểm tra trạng thái nạp. |
| `roomRepo` | `IRepository<Room>&` | Tra Room và kiểm tra trạng thái nạp. |

### 4.2. Hàm private

| Hàm | Vai trò |
|---|---|
| `getShowtimeId()` | Callback lấy ID. |
| `stringToShowtime()` | Parse một dòng TXT và validate thuộc tính. |
| `showtimeToString()` | Validate và serialize một Showtime. |
| `isValidShowtime()` | Kiểm tra thuộc tính của Model. |
| `isValidReference()` | Kiểm tra tham chiếu; caller đã gọi `isValidShowtime()`. |
| `hasScheduleConflict()` | Dò trùng lịch; dùng trong load/add/update. |
| `unlinkReferences()` | Gỡ cả hai liên kết hiện có của một Showtime; báo lỗi sau khi đã thử cả hai cạnh. |
| `unlinkAll()` | Gỡ liên kết Movie/Room cho mọi Showtime đang trong `repo`. |
| `restoreShowtime()` | Khôi phục thuộc tính và hai liên kết khi update thất bại. |

Ba callback và `isValidShowtime()` là `static` vì không dùng thuộc tính của ShowtimeRepository. Các helper còn lại cần truy cập Repository hoặc đối tượng đang lưu nên là hàm thành viên.

### 4.3. Hàm public

| Hàm | Vai trò | Override |
|---|---|---|
| Constructor, destructor | Khởi tạo dependency; gỡ liên kết trước khi `repo` bị hủy. | Chỉ destructor |
| `load()`, `save()` | Nạp và lưu dữ liệu. | Có |
| `add()`, `update()`, `removeById()` | Thêm, cập nhật cả Movie/Room khi hợp lệ, xóa. | Có |
| `findById()`, `getAt()`, `count()`, `isLoaded()` | Truy vấn. | Có |

## 5. Mã nguồn dự kiến sau chuẩn hóa

Hai khối dưới đây là toàn bộ mã nguồn **dự kiến**. Mã nguồn hiện tại trong `src/` chưa được sửa theo bản này; chỉ sao chép sau khi nhóm CHỐT MARKDOWN. Các thay đổi ở đây chỉ là `override` cho destructor, nhóm hàm private, căn chỉnh và trình bày; logic được giữ nguyên.

### 5.1. ShowtimeRepository.h

**Đường dẫn:** `src/data_access/ShowtimeRepository.h`

```cpp
#ifndef SHOWTIME_REPOSITORY_H
#define SHOWTIME_REPOSITORY_H

#include <string>
#include "IRepository.h"
#include "Repository.h"
#include "../models/Showtime.h"

class Movie;
class Room;

class ShowtimeRepository : public IRepository<Showtime>
{
private:
    Repository<Showtime> repo;
    IRepository<Movie>& movieRepo;
    IRepository<Room>& roomRepo;

    // Chuyen doi du lieu

    static std::string getShowtimeId(Showtime* showtime);
    static Showtime* stringToShowtime(const std::string& line);
    static std::string showtimeToString(Showtime* showtime);
    // Kiem tra du lieu va rang buoc

    static bool isValidShowtime(const Showtime* showtime);

    bool isValidReference(const Showtime* showtime);
    bool hasScheduleConflict(const Showtime* showtime, const std::string& excludeId = "");
    // Quan ly lien ket va khoi phuc

    void unlinkReferences(Showtime* showtime);
    void unlinkAll();
    void restoreShowtime(Showtime* showtime,
                         const Showtime& oldValues,
                         Movie* oldMovie,
                         Room* oldRoom);

public:
    ShowtimeRepository(const std::string& filePath,
                       IRepository<Movie>& movieRepo,
                       IRepository<Room>& roomRepo);
    ~ShowtimeRepository() override;

    // Doc va ghi du lieu
    bool load() override;
    bool save() override;

    // Quan ly du lieu
    bool add(Showtime* showtime) override;
    bool update(const Showtime& newValues) override;
    bool removeById(const std::string& showtimeId) override;

    // Truy van du lieu
    Showtime* findById(const std::string& showtimeId) override;
    Showtime* getAt(int index) override;
    int count() override;
    bool isLoaded() override;
};

#endif
```

### 5.2. ShowtimeRepository.cpp

**Đường dẫn:** `src/data_access/ShowtimeRepository.cpp`

```cpp
#include "ShowtimeRepository.h"
#include "../models/Movie.h"
#include "../models/Room.h"
#include "../utils/Validator.h"
#include "../utils/DateTime.h"
#include <exception>
#include <iostream>
#include <stdexcept>
#include <vector>
using namespace std;

// Kiem tra toan bo du lieu Showtime
bool ShowtimeRepository::isValidShowtime(const Showtime* showtime)
{
    if (showtime == nullptr)
    {
        return false;
    }
    if (!Validator::isValidId(showtime->getShowtimeId(), "SHW"))
    {
        return false;
    }
    if (!Validator::isValidId(showtime->getMovieId(), "MOV") ||
        !Validator::isValidId(showtime->getRoomId(), "ROM"))
    {
        return false;
    }
    if (!Validator::isValidDateTime(showtime->getStartAt()) ||
        !Validator::isValidDateTime(showtime->getEndAt()))
    {
        return false;
    }
    if (!DateTime::isBefore(showtime->getStartAt(), showtime->getEndAt()))
    {
        return false;
    }
    if (showtime->getStatus() != ShowtimeStatus::SCHEDULED &&
        showtime->getStatus() != ShowtimeStatus::CANCELLED)
    {
        return false;
    }
    return true;
}

// Lay ID Showtime
string ShowtimeRepository::getShowtimeId(Showtime* showtime)
{
    if (showtime == nullptr)
    {
        return "";
    }
    return showtime->getShowtimeId();
}

// Chuyen mot dong TXT thanh Showtime
Showtime* ShowtimeRepository::stringToShowtime(const string& line)
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
    if (fields.size() != 6)
    {
        throw runtime_error("Sai so luong truong Showtime");
    }

    ShowtimeStatus status;
    if (fields[5] == "SCHEDULED")
    {
        status = ShowtimeStatus::SCHEDULED;
    }
    else if (fields[5] == "CANCELLED")
    {
        status = ShowtimeStatus::CANCELLED;
    }
    else
    {
        throw runtime_error("Trang thai Showtime khong hop le");
    }

    Showtime* showtime = new Showtime(
        fields[0],
        fields[1],
        fields[2],
        fields[3],
        fields[4],
        status
    );
    if (!isValidShowtime(showtime))
    {
        delete showtime;
        throw runtime_error("Du lieu Showtime khong hop le");
    }
    return showtime;
}

// Chuyen Showtime thanh mot dong TXT
string ShowtimeRepository::showtimeToString(Showtime* showtime)
{
    if (!isValidShowtime(showtime))
    {
        throw runtime_error("Showtime khong hop le khi ghi file");
    }
    string status;
    if (showtime->getStatus() == ShowtimeStatus::SCHEDULED)
    {
        status = "SCHEDULED";
    }
    else
    {
        status = "CANCELLED";
    }
    return showtime->getShowtimeId() + "|" +
           showtime->getMovieId() + "|" +
           showtime->getRoomId() + "|" +
           showtime->getStartAt() + "|" +
           showtime->getEndAt() + "|" +
           status;
}

ShowtimeRepository::ShowtimeRepository(const string& filePath,
                                       IRepository<Movie>& movieRepo,
                                       IRepository<Room>& roomRepo)
    : repo(filePath, getShowtimeId, stringToShowtime, showtimeToString),
      movieRepo(movieRepo),
      roomRepo(roomRepo)
{
}

ShowtimeRepository::~ShowtimeRepository()
{
    try
    {
        unlinkAll();
    }
    catch (const exception& e)
    {
        cerr << "Loi huy ShowtimeRepository: " << e.what() << endl;
        terminate();
    }
    catch (...)
    {
        cerr << "Loi huy ShowtimeRepository khong xac dinh" << endl;
        terminate();
    }
}

// Kiem tra tham chieu Movie va Room
bool ShowtimeRepository::isValidReference(const Showtime* showtime)
{
    if (showtime == nullptr)
    {
        return false;
    }
    if (!movieRepo.isLoaded() || !roomRepo.isLoaded())
    {
        return false;
    }
    Movie* movie = movieRepo.findById(showtime->getMovieId());
    Room* room = roomRepo.findById(showtime->getRoomId());
    if (movie == nullptr || room == nullptr)
    {
        return false;
    }
    if (showtime->getStatus() == ShowtimeStatus::SCHEDULED &&
        (!movie->isActive() || !room->isActive()))
    {
        return false;
    }
    return true;
}

// Kiem tra trung lich trong cung phong
bool ShowtimeRepository::hasScheduleConflict(const Showtime* showtime, const string& excludeId)
{
    if (showtime == nullptr || showtime->getStatus() == ShowtimeStatus::CANCELLED)
    {
        return false;
    }
    for (int i = 0; i < repo.count(); i++)
    {
        Showtime* current = repo.getAt(i);
        if (current == nullptr || current->getShowtimeId() == excludeId ||
            current->getRoomId() != showtime->getRoomId() ||
            current->getStatus() == ShowtimeStatus::CANCELLED)
        {
            continue;
        }
        if (DateTime::isOverlapping(
                current->getStartAt(), current->getEndAt(),
                showtime->getStartAt(), showtime->getEndAt()))
        {
            return true;
        }
    }
    return false;
}

// Go lien ket Movie va Room cua mot Showtime
void ShowtimeRepository::unlinkReferences(Showtime* showtime)
{
    Movie* movie = showtime->getMovie();
    Room* room = showtime->getRoom();
    bool movieUnlinked = true;
    bool roomUnlinked = true;
    if (movie != nullptr)
    {
        movieUnlinked = movie->unlinkShowtime(showtime);
    }
    if (room != nullptr)
    {
        roomUnlinked = room->unlinkShowtime(showtime);
    }
    if (!movieUnlinked || !roomUnlinked)
    {
        throw runtime_error("Khong the go lien ket Movie/Room cua Showtime");
    }
}

// Go lien ket cua cac Showtime dang duoc quan ly
void ShowtimeRepository::unlinkAll()
{
    for (int i = 0; i < repo.count(); i++)
    {
        unlinkReferences(repo.getAt(i));
    }
}

// Khoi phuc Showtime sau khi cap nhat that bai
void ShowtimeRepository::restoreShowtime(Showtime* showtime,
                                         const Showtime& oldValues,
                                         Movie* oldMovie,
                                         Room* oldRoom)
{
    // Khoi phuc lien ket Movie
    if (showtime->getMovie() != oldMovie)
    {
        Movie* movie = showtime->getMovie();

        if (movie != nullptr && !movie->unlinkShowtime(showtime))
        {
            throw runtime_error("Khong the huy lien ket Movie moi");
        }

        if (!showtime->setMovieId(oldValues.getMovieId()) ||
            !showtime->linkMovie(oldMovie))
        {
            throw runtime_error("Khong the khoi phuc lien ket Movie");
        }
    }

    // Khoi phuc lien ket Room
    if (showtime->getRoom() != oldRoom)
    {
        Room* room = showtime->getRoom();

        if (room != nullptr && !room->unlinkShowtime(showtime))
        {
            throw runtime_error("Khong the huy lien ket Room moi");
        }

        if (!showtime->setRoomId(oldValues.getRoomId()) ||
            !showtime->linkRoom(oldRoom))
        {
            throw runtime_error("Khong the khoi phuc lien ket Room");
        }
    }

    // Khoi phuc cac thuoc tinh cu
    showtime->setStartAt(oldValues.getStartAt());
    showtime->setEndAt(oldValues.getEndAt());
    showtime->setStatus(oldValues.getStatus());
}

// Doc du lieu Showtime tu file
bool ShowtimeRepository::load()
{
    if (!movieRepo.isLoaded() || !roomRepo.isLoaded())
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
            Showtime* showtime = repo.getAt(i);
            Movie* movie = movieRepo.findById(showtime->getMovieId());
            Room* room = roomRepo.findById(showtime->getRoomId());
            if (movie == nullptr || room == nullptr)
            {
                throw runtime_error("Showtime tham chieu Movie hoac Room khong ton tai");
            }
            if (hasScheduleConflict(showtime, showtime->getShowtimeId()))
            {
                throw runtime_error("Trung lich Showtime trong file");
            }
            if (!showtime->linkMovie(movie) || !showtime->linkRoom(room))
            {
                throw runtime_error("Khong the lien ket Showtime voi Movie va Room");
            }
        }
    }
    catch (...)
    {
        unlinkAll();
        repo.clear();
        throw;
    }
    return true;
}

// Luu du lieu Showtime xuong file
bool ShowtimeRepository::save()
{
    return repo.save();
}

// Them suat chieu moi
bool ShowtimeRepository::add(Showtime* showtime)
{
    if (!repo.isLoaded() || !isValidShowtime(showtime))
    {
        return false;
    }
    // Khong them Showtime da duoc lien ket
    if (showtime->getMovie() != nullptr || showtime->getRoom() != nullptr)
    {
        return false;
    }
    // Can kiem tra truoc khi tao lien ket
    if (repo.findById(showtime->getShowtimeId()) != nullptr)
    {
        return false;
    }
    if (!isValidReference(showtime) || hasScheduleConflict(showtime))
    {
        return false;
    }

    Movie* movie = movieRepo.findById(showtime->getMovieId());
    Room* room = roomRepo.findById(showtime->getRoomId());

    bool added = false;
    try
    {
        if (showtime->linkMovie(movie) && showtime->linkRoom(room))
        {
            added = repo.add(showtime);
        }
    }
    catch (...)
    {
        unlinkReferences(showtime);
        throw;
    }
    if (!added)
    {
        unlinkReferences(showtime);
        return false;
    }
    return true;
}

// Cap nhat suat chieu
bool ShowtimeRepository::update(const Showtime& newValues)
{
    if (!repo.isLoaded() || !isValidShowtime(&newValues))
    {
        return false;
    }
    Showtime* current = repo.findById(newValues.getShowtimeId());
    if (current == nullptr)
    {
        return false;
    }
    if (!isValidReference(&newValues) ||
        hasScheduleConflict(&newValues, current->getShowtimeId()))
    {
        return false;
    }

    Movie* oldMovie = current->getMovie();
    Room* oldRoom = current->getRoom();

    Movie* newMovie = movieRepo.findById(newValues.getMovieId());
    Room* newRoom = roomRepo.findById(newValues.getRoomId());

    if (oldMovie == nullptr || oldRoom == nullptr ||
        newMovie == nullptr || newRoom == nullptr)
    {
        return false;
    }
    const bool movieChanged = oldMovie != newMovie;
    const bool roomChanged = oldRoom != newRoom;

    Showtime oldValues(current->getShowtimeId(),
                       current->getMovieId(),
                       current->getRoomId(),
                       current->getStartAt(),
                       current->getEndAt(),
                       current->getStatus());
    try
    {
        if (movieChanged)
        {
            if (!oldMovie->unlinkShowtime(current) ||
                !current->setMovieId(newValues.getMovieId()) ||
                !current->linkMovie(newMovie))
            {
                throw runtime_error("Khong the doi Movie cua Showtime");
            }
        }
        if (roomChanged)
        {
            if (!oldRoom->unlinkShowtime(current) ||
                !current->setRoomId(newValues.getRoomId()) ||
                !current->linkRoom(newRoom))
            {
                throw runtime_error("Khong the doi Room cua Showtime");
            }
        }
        current->setStartAt(newValues.getStartAt());
        current->setEndAt(newValues.getEndAt());
        current->setStatus(newValues.getStatus());
    }
    catch (...)
    {
        restoreShowtime(current, oldValues, oldMovie, oldRoom);
        throw;
    }
    return true;
}

// Xoa suat chieu theo ID
bool ShowtimeRepository::removeById(const string& showtimeId)
{
    if (!repo.isLoaded())
    {
        return false;
    }
    Showtime* showtime = repo.findById(showtimeId);
    if (showtime == nullptr)
    {
        return false;
    }
    Movie* movie = showtime->getMovie();
    Room* room = showtime->getRoom();
    if (movie == nullptr || room == nullptr)
    {
        return false;
    }
    if (!movie->unlinkShowtime(showtime))
    {
        return false;
    }
    if (!room->unlinkShowtime(showtime))
    {
        if (!movie->linkShowtime(showtime))
        {
            throw runtime_error("Khong the khoi phuc lien ket Movie");
        }
        return false;
    }
    if (!repo.removeById(showtimeId))
    {
        bool movieRestored = movie->linkShowtime(showtime);
        bool roomRestored = room->linkShowtime(showtime);
        if (!movieRestored || !roomRestored)
        {
            throw runtime_error("Khong the khoi phuc lien ket Showtime");
        }
        return false;
    }
    return true;
}

// Tim suat chieu theo ID
Showtime* ShowtimeRepository::findById(const string& showtimeId)
{
    return repo.findById(showtimeId);
}

// Lay suat chieu theo vi tri
Showtime* ShowtimeRepository::getAt(int index)
{
    return repo.getAt(index);
}

// Dem so luong suat chieu
int ShowtimeRepository::count()
{
    return repo.count();
}

// Kiem tra trang thai load
bool ShowtimeRepository::isLoaded()
{
    return repo.isLoaded();
}
```

## 6. Giải thích thư viện và từ khóa

### 6.1. Thư viện chuẩn

| Include | Cách dùng |
|---|---|
| `<string>` | `std::string` trong header cho đường dẫn, ID và dòng TXT; `.cpp` dùng `string` sau `using namespace std;`. |
| `<vector>` | `vector<string>` giữ sáu trường sau khi tách một dòng. |
| `<stdexcept>` | `runtime_error` báo dữ liệu hoặc liên kết không hợp lệ. |
| `<exception>` | `exception` trong catch và `terminate()` khi destructor không thể gỡ liên kết. |
| `<iostream>` | `cerr` và `endl` báo lỗi ngay trước `terminate()`. |

### 6.2. Header dự án

`IRepository.h` khai báo hợp đồng chung; `Repository.h` cung cấp lưu trữ và FileIO. `Showtime.h` cho Model và enum. Header chỉ cần forward declaration `class Movie; class Room;` cho tham chiếu và con trỏ; `.cpp` include đầy đủ `Movie.h` và `Room.h` để gọi getter, `isActive()` và link/unlink. `Validator.h` kiểm ID/DateTime; `DateTime.h` so thứ tự và xung đột lịch có khoảng nghỉ.

### 6.3. `static`, callback và `override`

Các hàm `static` không có `this`, nên ba callback lấy ID, parse và serialize có thể truyền trực tiếp cho `Repository<Showtime>`. `override` xác nhận chín phương thức interface và destructor khớp hàm ảo của lớp cha; sai chữ ký sẽ bị compiler phát hiện.

### 6.4. Tham chiếu, con trỏ và exception

`IRepository<Movie>&` và `IRepository<Room>&` là dependency mượn, không thể null và không do ShowtimeRepository hủy. `Showtime*` do `repo` sở hữu sau load/add thành công; con trỏ Movie/Room là liên kết mượn. `try/catch (...)` chỉ xuất hiện khi cần gỡ liên kết hoặc khôi phục trạng thái; `throw;` ném lại lỗi đang xử lý sau rollback thành công.

## 7. Giải thích các hàm private

Các chữ ký dưới đây dùng cách viết trong `.cpp`; khai báo trong header dùng `std::string`. Các callback `static` không dùng trạng thái của đối tượng Repository.

### 7.1. Hàm isValidShowtime()

**Chữ ký:** `static bool isValidShowtime(const Showtime* showtime)`. Nhận con trỏ chỉ đọc, trả `true` nếu các thuộc tính hợp lệ; `false` nếu không.

**Luồng xử lý:**

```text
showtime null? ─có→ false
↓ không
ID SHW, MOV, ROM hợp lệ? ─không→ false
↓ có
startAt, endAt là DateTime hợp lệ? ─không→ false
↓ có
startAt trước endAt? ─không→ false
↓ có
status là SCHEDULED/CANCELLED? ─không→ false
↓ có
true
```

**Giải thích code:** Kiểm `nullptr` trước các getter. Ba lời gọi `Validator::isValidId()` kiểm định dạng từng khóa, không kiểm đối tượng tham chiếu có tồn tại. `Validator::isValidDateTime()` bảo đảm chuỗi giờ hợp lệ trước khi `DateTime::isBefore()` so thứ tự. So enum tường minh chặn giá trị ngoài hai trạng thái. Hàm chỉ kiểm thuộc tính; `isValidReference()` và `hasScheduleConflict()` xử lý tham chiếu, lịch.

### 7.2. Hàm getShowtimeId()

**Chữ ký:** `static string getShowtimeId(Showtime* showtime)`. Nhận con trỏ, trả ID hoặc chuỗi rỗng nếu null.

```text
showtime null? ─có→ return ""
↓ không
return showtime->getShowtimeId()
```

`Repository<Showtime>` gọi callback này để kiểm ID trùng, tìm kiếm và xóa. Guard null ngăn truy cập con trỏ rỗng.

### 7.3. Hàm stringToShowtime()

**Chữ ký:** `static Showtime* stringToShowtime(const string& line)`. Nhận dòng TXT, trả đối tượng mới; dữ liệu sai ném `runtime_error`.

```text
Tách line theo '|'
↓
đúng 6 trường? ─không→ throw
↓ có
status hợp lệ? ─không→ throw
↓ có
new Showtime → isValidShowtime?
├─ không → delete showtime → throw
↓ có
return showtime
```

**Bước 1 — tách và kiểm số trường.** `vector<string> fields`, `find('|', start)` và `substr()` giữ cả trường rỗng cuối. `string::npos` là dấu hiệu không còn dấu phân cách. Chỉ đọc `fields[5]` sau khi `fields.size() == 6`.

**Bước 2 — giải mã trạng thái.** Hai nhánh `if/else if` ánh xạ `SCHEDULED` và `CANCELLED` sang enum; nhánh khác ném lỗi thay vì mặc định một trạng thái.

**Bước 3 — tạo và kiểm Model.** `new Showtime(fields[0], ..., status)` cấp phát; `isValidShowtime()` sai thì `delete showtime` trước khi ném. Con trỏ trả về được `Repository<Showtime>::load()` quản lý. Phương thức này chưa liên kết Movie/Room; `load()` làm sau khi toàn bộ bản ghi được parse.

### 7.4. Hàm showtimeToString()

**Chữ ký:** `static string showtimeToString(Showtime* showtime)`. Nhận con trỏ, trả dòng sáu trường; dữ liệu sai ném `runtime_error`.

```text
isValidShowtime? ─không→ throw
↓ có
chọn chuỗi status → nối 6 trường → return string
```

`isValidShowtime()` chặn null và enum sai trước khi gọi getter. `status` nhận `SCHEDULED` hoặc `CANCELLED`; phép `+` nối các trường theo đúng schema. Callback chỉ tạo chuỗi; `repo.save()` chịu trách nhiệm ghi file.

### 7.5. Hàm isValidReference()

**Chữ ký:** `bool isValidReference(const Showtime* showtime)`. Nhận con trỏ chỉ đọc; trả `true` khi tham chiếu dùng được.

```text
showtime null? ─có→ false
↓ không
movieRepo và roomRepo loaded? ─không→ false
↓ có
tìm Movie, Room theo ID → thiếu một bên? ─có→ false
↓ không
SCHEDULED và Movie/Room inactive? ─có→ false
↓ không
true
```

Caller đã gọi `isValidShowtime()`, nên helper không kiểm lại ID và thời gian. Hai con trỏ tìm được phải khác null trước `isActive()`. Suất `CANCELLED` được phép giữ tham chiếu đến Movie/Room đã ngừng hoạt động. Hàm không áp đặt thời điểm tương lai hoặc thời lượng phim.

### 7.6. Hàm hasScheduleConflict()

**Chữ ký:** `bool hasScheduleConflict(const Showtime* showtime, const string& excludeId = "")`. Trả `true` nếu có suất cùng phòng chồng lịch; `excludeId` bỏ qua chính bản ghi đang xét.

```text
showtime null hoặc CANCELLED? ─có→ false
↓ không
duyệt repo
├─ current null / cùng excludeId / khác phòng / CANCELLED → tiếp tục
↓ suất cần so
DateTime::isOverlapping()? ─có→ true
↓ không, tiếp tục
hết danh sách → false
```

Vòng `for` dùng `repo.count()` và `repo.getAt(i)`. `continue` loại các trường hợp không thể xung đột trước khi gọi `DateTime::isOverlapping()`. Hàm DateTime áp dụng khoảng nghỉ 15 phút hiện hành. `load()` truyền ID của chính suất; `update()` truyền ID hiện có; `add()` dùng chuỗi mặc định rỗng. Chỉ gọi sau khi thời gian đầu vào đã hợp lệ.

### 7.7. Hàm unlinkReferences()

**Chữ ký:** `void unlinkReferences(Showtime* showtime)`. Nhận con trỏ hợp lệ do caller cung cấp; không có giá trị trả về, ném `runtime_error` nếu một cạnh không gỡ được.

```text
lấy Movie*, Room* hiện đang liên kết
↓
Movie khác null? ─có→ thử movie->unlinkShowtime()
↓
Room khác null? ─có→ thử room->unlinkShowtime()
↓
một kết quả false? ─có→ throw
↓ không
kết thúc
```

Hai biến `movieUnlinked` và `roomUnlinked` khởi tạo `true` để cạnh vắng mặt được coi là không cần gỡ. Phương thức cố thử cả hai cạnh kể cả cạnh đầu trả false, rồi mới báo lỗi. Vì vậy lỗi có thể xảy ra sau khi một cạnh đã được gỡ; helper không tự khôi phục cạnh đó. Caller dùng hàm trong rollback `add()`, `load()` và destructor phải xét giới hạn này.

```cpp
if (movie != nullptr)
{
    movieUnlinked = movie->unlinkShowtime(showtime);
}
if (room != nullptr)
{
    roomUnlinked = room->unlinkShowtime(showtime);
}
if (!movieUnlinked || !roomUnlinked)
{
    throw runtime_error("Khong the go lien ket Movie/Room cua Showtime");
}
```

### 7.8. Hàm unlinkAll()

**Chữ ký:** `void unlinkAll()`. Không có tham số hoặc kết quả; duyệt mọi Showtime trong `repo` và gỡ Movie/Room.

```text
i = 0
↓
i < repo.count()? ─không→ kết thúc
↓ có
unlinkReferences(repo.getAt(i)) ─exception→ truyền ra
↓ thành công
i++ → lặp
```

Hàm không xóa `repo`; nó chỉ gỡ các liên kết trước khi `repo.clear()` hoặc destructor sở hữu dữ liệu chạy. Nếu một lần gỡ ném, vòng lặp dừng và các Showtime phía sau chưa được xử lý.

### 7.9. Hàm restoreShowtime()

**Chữ ký:** `void restoreShowtime(Showtime* showtime, const Showtime& oldValues, Movie* oldMovie, Room* oldRoom)`. Nhận đối tượng đang sửa, bản sao thuộc tính cũ và hai con trỏ cũ; không trả về, ném khi không thể khôi phục.

```text
Movie hiện tại khác oldMovie?
├─ có → gỡ Movie mới nếu có ─lỗi→ throw
│       → setMovieId cũ → linkMovie cũ ─lỗi→ throw
↓
Room hiện tại khác oldRoom?
├─ có → gỡ Room mới nếu có ─lỗi→ throw
│       → setRoomId cũ → linkRoom cũ ─lỗi→ throw
↓
setStartAt, setEndAt, setStatus về giá trị cũ
↓
kết thúc
```

**Khôi phục Movie:** So con trỏ hiện tại với `oldMovie`; nếu khác, gỡ liên kết mới trước. `setMovieId()` chỉ nhận ID khi con trỏ Movie đã null; tiếp đó `linkMovie(oldMovie)` tái lập liên kết hai chiều.

```cpp
if (showtime->getMovie() != oldMovie)
{
    Movie* movie = showtime->getMovie();
    if (movie != nullptr && !movie->unlinkShowtime(showtime))
    {
        throw runtime_error("Khong the huy lien ket Movie moi");
    }
    if (!showtime->setMovieId(oldValues.getMovieId()) ||
        !showtime->linkMovie(oldMovie))
    {
        throw runtime_error("Khong the khoi phuc lien ket Movie");
    }
}
```

**Khôi phục Room:** Làm tương tự với `oldRoom` và `setRoomId()`. Mỗi lần unlink/set/link thất bại đều ném lỗi ngay, không tiếp tục các bước sau.

```cpp
if (showtime->getRoom() != oldRoom)
{
    Room* room = showtime->getRoom();
    if (room != nullptr && !room->unlinkShowtime(showtime))
    {
        throw runtime_error("Khong the huy lien ket Room moi");
    }
    if (!showtime->setRoomId(oldValues.getRoomId()) ||
        !showtime->linkRoom(oldRoom))
    {
        throw runtime_error("Khong the khoi phuc lien ket Room");
    }
}
```

**Khôi phục thuộc tính:** Ba setter cho thời gian và trạng thái chạy sau hai liên kết. Nếu chính quá trình khôi phục ném, trạng thái có thể chỉ được phục hồi một phần và exception gốc từ `update()` bị thay thế; mã hiện tại không có rollback cấp hai.

## 8. Giải thích các hàm public

### 8.1. Constructor

**Chữ ký:** `ShowtimeRepository(const string& filePath, IRepository<Movie>& movieRepo, IRepository<Room>& roomRepo)`. Nhận đường dẫn và hai dependency qua tham chiếu; không trả về, không tự load.

```text
nhận filePath, Movie repo, Room repo
↓
khởi tạo repo với ba callback
↓
lưu hai tham chiếu → kết thúc
```

Danh sách khởi tạo gọi `Repository<Showtime>(filePath, getShowtimeId, stringToShowtime, showtimeToString)`, rồi gắn hai tham chiếu. `MovieRepository` và `RoomRepository` phải tồn tại lâu hơn đối tượng này vì destructor còn dùng chúng để gỡ liên kết.

### 8.2. Destructor

**Chữ ký:** `~ShowtimeRepository() override`. Không có tham số hay kết quả; gỡ các liên kết trước khi `repo` tự hủy.

```text
try unlinkAll()
├─ thành công → hủy các thuộc tính; repo giải phóng Showtime
├─ std::exception → ghi lỗi → terminate()
└─ ngoại lệ khác → ghi lỗi → terminate()
```

`unlinkAll()` gỡ các con trỏ Movie/Room trong lúc Showtime còn tồn tại. Destructor không ném ra ngoài; hai `catch` ghi `cerr` và gọi `terminate()` nếu gỡ liên kết thất bại. `override` ở header yêu cầu destructor của interface là virtual. Đây là hành vi hiện tại, không phải cơ chế phục hồi khi destructor lỗi.

### 8.3. Hàm load()

**Chữ ký:** `bool load() override`. Không có tham số; trả false khi dependency chưa load hoặc `repo.load()` từ chối; ném lỗi dữ liệu/liên kết.

```text
Movie/Room repo loaded? ─không→ false
↓ có
repo.load()? ─false→ false
↓ true
try: duyệt Showtime đã parse
├─ thiếu Movie/Room → throw
├─ xung đột lịch → throw
├─ linkMovie/linkRoom false → throw
↓ duyệt xong
true
exception trong try → unlinkAll() → repo.clear() → throw lại
```

**Bước 1:** Kiểm dependency trước `repo.load()` để không tạo Showtime với tham chiếu chưa sẵn sàng. Parser đã kiểm thuộc tính và generic Repository kiểm ID, nên vòng sau không làm lại các kiểm tra đó.

**Bước 2:** Với từng `repo.getAt(i)`, tìm Movie/Room bằng ID, kiểm null, gọi `hasScheduleConflict(showtime, showtime->getShowtimeId())` để bỏ chính suất nhưng so với mọi suất còn lại, rồi tạo hai liên kết qua Model.

**Bước 3:** `catch (...)` gọi `unlinkAll()` trước `repo.clear()` để không để Movie/Room giữ con trỏ treo. Nếu `unlinkAll()` tự ném, `repo.clear()` và `throw;` không chạy; trạng thái có thể còn nạp một phần và ngoại lệ ban đầu bị thay thế. Đây là giới hạn rollback hiện tại. Lỗi từ `repo.load()` xảy ra trước `try`, do generic Repository xử lý theo hợp đồng của nó.

```cpp
catch (...)
{
    unlinkAll();
    repo.clear();
    throw;
}
```

### 8.4. Hàm save()

**Chữ ký:** `bool save() override`. Không có tham số, trả trực tiếp `repo.save()`.

```text
repo.save() → return kết quả hoặc truyền exception
```

`Repository<Showtime>` dùng `showtimeToString()` để tạo sáu trường rồi ghi TXT. Hàm không tự lưu sau `add()` hoặc `update()`.

### 8.5. Hàm add()

**Chữ ký:** `bool add(Showtime* showtime) override`. Nhận con trỏ caller sở hữu trước khi thêm; trả true khi `repo` nhận quyền sở hữu, false khi bị từ chối, hoặc ném khi rollback lỗi.

```text
loaded và thuộc tính hợp lệ? ─không→ false
↓ có
đã liên kết Movie/Room? ─có→ false
↓ không
ID đã có? ─có→ false
↓ không
tham chiếu hợp lệ và không trùng lịch? ─không→ false
↓ có
tìm Movie/Room → try linkMovie rồi linkRoom → repo.add()
├─ true → return true
├─ false → unlinkReferences() → return false
└─ exception → unlinkReferences() → throw lại
```

**Bước 1 — kiểm trước liên kết:** `repo.isLoaded()` cần thiết vì hàm thay đổi liên kết trước khi gọi `repo.add()`. `isValidShowtime()` chặn null. Kiểm con trỏ liên kết hiện có và ID trùng trước khi tạo cạnh mới; `isValidReference()` kiểm Movie/Room và `hasScheduleConflict()` kiểm lịch.

**Bước 2 — tạo cạnh và thêm:** `movieRepo.findById()` cùng `roomRepo.findById()` lấy con trỏ mượn. Trong `try`, phép `&&` gọi `linkRoom()` chỉ khi `linkMovie()` thành công; chỉ khi cả hai true mới gọi `repo.add()`.

**Bước 3 — rollback:** Nếu thao tác trong `try` ném hoặc `repo.add()` trả false, `unlinkReferences(showtime)` gỡ các cạnh đã tạo. Chỉ nhánh `repo.add()` true chuyển quyền sở hữu. Nếu chính việc gỡ ném, ngoại lệ gốc có thể bị thay thế; hàm không bảo đảm khôi phục đầy đủ trong trường hợp đó.

```cpp
catch (...)
{
    unlinkReferences(showtime);
    throw;
}
if (!added)
{
    unlinkReferences(showtime);
    return false;
}
```

### 8.6. Hàm update()

**Chữ ký:** `bool update(const Showtime& newValues) override`. Nhận dữ liệu mới chỉ đọc, sửa tại chỗ và giữ địa chỉ/ID; false nếu điều kiện trước sửa không đạt, ném khi thao tác hoặc khôi phục lỗi.

```text
loaded và newValues hợp lệ? ─không→ false
↓ có
tìm current theo ID ─null→ false
↓ có
tham chiếu hợp lệ, không xung đột lịch? ─không→ false
↓ có
lấy oldMovie/oldRoom và newMovie/newRoom ─có null→ false
↓ đủ
lưu movieChanged, roomChanged, oldValues
↓
try: nếu đổi Movie → gỡ cũ, đổi ID, nối mới
↓
nếu đổi Room → gỡ cũ, đổi ID, nối mới
↓
setStartAt, setEndAt, setStatus → true
exception → restoreShowtime() → throw lại
```

**Bước 1 — validation:** `isValidShowtime(&newValues)` kiểm thuộc tính; `repo.findById()` kiểm suất hiện có; `isValidReference()` và `hasScheduleConflict(..., current->getShowtimeId())` kiểm đối tượng tham chiếu, trạng thái hoạt động và lịch, bỏ chính suất đang sửa. Các phép kiểm liên kết cần chạy trước thay đổi để tránh trạng thái dở dang.

**Bước 2 — snapshot:** Lưu bốn con trỏ Movie/Room cũ/mới, từ chối null; hai cờ quyết định có cần đổi cạnh. `Showtime oldValues(...)` sao chép ID, khóa liên kết, thời gian và status để phục hồi. Không thay thế đối tượng `current`.

**Bước 3 — thay đổi:** Khi Movie/Room đổi, thứ tự là `unlinkShowtime(current)`, `current->setMovieId/RoomId(...)`, `current->linkMovie/Room(new...)`. Bất kỳ kết quả false nào ném lỗi. Sau đó ba setter cập nhật thời gian và status.

**Bước 4 — rollback:** `catch (...)` gọi `restoreShowtime(current, oldValues, oldMovie, oldRoom)` rồi `throw;`. Nếu khôi phục thành công, exception gốc được truyền tiếp. Nếu khôi phục ném, exception khôi phục thay thế exception gốc và có thể còn liên kết hoặc thuộc tính dở dang. Code hiện tại không có cơ chế khôi phục tiếp.

```cpp
catch (...)
{
    restoreShowtime(current, oldValues, oldMovie, oldRoom);
    throw;
}
```

### 8.7. Hàm removeById()

**Chữ ký:** `bool removeById(const string& showtimeId) override`. Nhận ID; false khi chưa load, không tìm thấy, thiếu cạnh hoặc xóa/gỡ bị từ chối; true khi `repo.removeById()` xóa đối tượng.

```text
loaded? ─không→ false → tìm Showtime ─null→ false
↓ có
Movie/Room liên kết? ─thiếu→ false
↓ đủ
gỡ Movie ─false→ false
↓ true
gỡ Room ─false→ khôi phục Movie ─thất bại→ throw; thành công→ false
↓ true
repo.removeById() ─true→ true
↓ false
khôi phục Movie và Room ─thất bại→ throw; thành công→ false
```

**Bước 1:** `repo.isLoaded()` và `repo.findById()` chặn truy cập sai. Kiểm hai con trỏ trước khi gỡ, vì gỡ một cạnh rồi mới phát hiện cạnh kia null sẽ khó khôi phục.

**Bước 2:** Gỡ Movie trước rồi Room. Nếu Room từ chối, gọi `movie->linkShowtime(showtime)` để phục hồi; lỗi phục hồi ném `runtime_error`.

**Bước 3:** Chỉ sau khi gỡ cả hai cạnh mới gọi `repo.removeById()`, thao tác này có thể giải phóng Showtime. Nếu trả false, gọi cả `movie->linkShowtime()` và `room->linkShowtime()`, sau đó kiểm kết quả. Nếu một lời gọi khôi phục ném ngoại lệ, mã hiện tại không có `catch` riêng cho nhánh này.

```cpp
if (!repo.removeById(showtimeId))
{
    bool movieRestored = movie->linkShowtime(showtime);
    bool roomRestored = room->linkShowtime(showtime);
    if (!movieRestored || !roomRestored)
    {
        throw runtime_error("Khong the khoi phuc lien ket Showtime");
    }
    return false;
}
return true;
```

**Điều kiện trước khi gọi:** Showtime phải **chưa từng phát sinh Ticket**, kể cả vé đã hủy, và **không có Invoice tham chiếu**. `ShowtimeService` chịu trách nhiệm kiểm tra lịch sử theo mã suất trước khi gọi `removeById()`; phần này sẽ được hoàn thiện sau khi refactor `TicketRepository` và `InvoiceRepository`. Hàm Repository ở mục 5 giữ nguyên logic, chỉ gỡ Movie/Room và xóa dữ liệu; nó không tự kiểm Ticket/Invoice. Nếu gọi trực tiếp khi vẫn có Ticket hoặc Invoice giữ `Showtime*`, việc xóa có thể tạo con trỏ treo. Model Showtime không bổ sung liên kết ngược đến Ticket/Invoice, và ShowtimeRepository không nhận thêm hai Repository đó làm dependency.

**Giới hạn rollback:** Khi gỡ Room thất bại, hàm chỉ thử nối lại Movie; khi `repo.removeById()` trả false, hàm thử nối lại cả Movie và Room. Nếu thao tác khôi phục trả false hoặc ném ngoại lệ, liên kết có thể còn dở dang. Không coi nhánh rollback thất bại là đã bảo đảm trạng thái nguyên vẹn.

### 8.8. Hàm findById()

**Chữ ký:** `Showtime* findById(const string& showtimeId) override`. Trả con trỏ mượn hoặc null.

```text
repo.findById(showtimeId) → return kết quả
```

Generic Repository tra ID; caller không sở hữu con trỏ trả về.

### 8.9. Hàm getAt()

**Chữ ký:** `Showtime* getAt(int index) override`. Trả con trỏ mượn ở vị trí hợp lệ hoặc null.

```text
repo.getAt(index) → return kết quả
```

Kiểm biên vị trí thuộc `Repository<Showtime>`; hàm không tạo đối tượng mới.

### 8.10. Hàm count()

**Chữ ký:** `int count() override`. Trả số Showtime hiện được `repo` quản lý.

```text
repo.count() → return số lượng
```

Không truy cập file hoặc duyệt liên kết Movie/Room.

### 8.11. Hàm isLoaded()

**Chữ ký:** `bool isLoaded() override`. Trả trạng thái nạp của `repo`.

```text
repo.isLoaded() → return bool
```

Không có cờ loaded riêng trong ShowtimeRepository; file rỗng có thể loaded true với count bằng 0.

## 9. Quản lý bộ nhớ và ràng buộc Ticket/Invoice

`Repository<Showtime>` sở hữu `Showtime*` sau add/load thành công; caller sở hữu con trỏ mới khi add thất bại. Movie/Room chỉ giữ liên kết không sở hữu. `unlinkAll()` gỡ hai cạnh trước khi destructor xóa Showtime hoặc `load()` gọi `repo.clear()`; `removeById()` gỡ hai cạnh trước khi xóa một suất. Các Repository nền phải còn sống khi destructor chạy.

**Điều kiện gọi đã thống nhất:** Chỉ gọi `removeById()` khi Showtime chưa từng phát sinh Ticket, gồm cả vé đã hủy, và không có Invoice tham chiếu. `ShowtimeService` sẽ kiểm tra cả hai điều kiện sau khi refactor `TicketRepository` và `InvoiceRepository`; hiện `ShowtimeService::removeShowtime()` vẫn trả `false` với TODO về lịch sử vé. `Ticket` và `Invoice` giữ `Showtime*` trực tiếp, còn `Showtime` không giữ danh sách ngược. Thiết kế không bổ sung liên kết hai chiều với Ticket/Invoice vào Model, cũng không bổ sung dependency TicketRepository/InvoiceRepository cho ShowtimeRepository. Bởi vậy gọi trực tiếp Repository khi còn tham chiếu có thể gây con trỏ treo; đây là giới hạn của API hiện tại, không phải điều kiện đã được Repository tự bảo đảm.

`unlinkReferences()` kiểm kết quả gỡ cả hai cạnh của một Showtime, dùng lại trong `unlinkAll()` và rollback `add()`. Trong `load()`, lỗi từ helper ngăn `repo.clear()` để tránh xóa Showtime còn được tham chiếu; trong destructor nó được báo ra `cerr` rồi `terminate()` vì không thể ném qua destructor của interface. Đây là xử lý trạng thái lỗi rõ ràng, không khẳng định đã tự sửa được quan hệ Model hỏng sẵn.

**Trách nhiệm đã thống nhất cho `update()`:** `business_rules.md` cấm đổi phim/phòng/thời gian của suất đã bắt đầu hoặc từng có vé, kể cả vé đã hủy. ShowtimeService sẽ dùng TicketRepository và InvoiceRepository khi refactor Business để kiểm lịch sử trước khi gọi `ShowtimeRepository::update()`; không thêm các dependency đó vào ShowtimeRepository vì chúng đang phụ thuộc ngược vào ShowtimeRepository. Service hiện chưa có các dependency này, nên điều kiện gọi chưa được triển khai. Repository chỉ xác thực Movie/Room, lịch và liên kết, rồi rollback thay đổi nội bộ khi lỗi. Không suy diễn "không thấy con trỏ" là "chưa từng có giao dịch".

## 10. Áp dụng Dependency Inversion Principle

Thiết kế DIP hiện có được giữ nguyên: `ShowtimeRepository` kế thừa `IRepository<Showtime>` cho Service sử dụng, nhận `IRepository<Movie>&` và `IRepository<Room>&` từ constructor, và giữ `Repository<Showtime>` bằng composition. Hai dependency được gọi qua `isLoaded()` và `findById()` của interface. Không thêm `update()` vào Repository chung hoặc thay đổi sơ đồ phụ thuộc trong bản chuẩn hóa này.

## 11. Nội dung thay đổi dự kiến

### 11.1. Header

- Bổ sung `override` cho destructor, nhóm rõ callback chuyển đổi, validation và helper liên kết.
- Căn thẳng các tham số constructor và `restoreShowtime()`; giữ nguyên kiểu, thứ tự và public API còn lại.

### 11.2. File triển khai

- Chuẩn hóa khoảng trắng và căn tham số constructor; không đổi câu lệnh, nhánh điều kiện hoặc thứ tự rollback.
- Giữ các kiểm tra trước link/unlink: loaded, Model hợp lệ, ID trùng, tham chiếu, lịch và trạng thái liên kết.

### 11.3. Thành phần giữ nguyên và điểm review

`isValidReference()`, `unlinkReferences()`, `unlinkAll()`, `restoreShowtime()`, schema TXT, quyền sở hữu con trỏ và cơ chế rollback hiện tại được giữ nguyên. `removeById()` cũng giữ nguyên; điều kiện không có bất kỳ Ticket nào trong lịch sử và không có Invoice tham chiếu là tiền điều kiện do ShowtimeService kiểm tra sau khi hai Repository liên quan được refactor. Không bổ sung liên kết ngược trong Showtime hoặc dependency mới vào ShowtimeRepository. Giới hạn khi rollback tự phát sinh ngoại lệ được ghi tại mục 7–9.

## 12. Kế hoạch kiểm thử

Chưa có kết quả kiểm thử thực tế được xác nhận cho bản chuẩn hóa Markdown này; mọi mục dưới đây còn chờ kiểm chứng. Dùng file TXT test riêng và không sửa test để tạo PASS.

### 12.1. Load và save

- [ ] File hợp lệ/rỗng/không tồn tại; load lặp; save trước load.
- [ ] Thiếu/thừa trường, ID sai/trùng, DateTime sai, `endAt <= startAt`, status sai.
- [ ] Thiếu Movie/Room; Movie/Room chưa load; suất cùng phòng trùng lịch hoặc cách nhau dưới 15 phút; khác phòng hoặc suất CANCELLED không trùng.
- [ ] Lỗi sau khi đã link các suất trước hoặc chỉ link Movie của suất hiện tại: `unlinkAll()` thành công thì `repo.clear()`, loaded=false, hai phía không giữ con trỏ treo; exception gốc được chuyển tiếp.
- [ ] Giả lập một cạnh unlink trả `false` trong rollback load: `unlinkReferences()` vẫn thử cạnh còn lại, báo lỗi, không gọi `repo.clear()`; xác minh dữ liệu còn sống để không tạo con trỏ treo.
- [ ] Serialize đúng sáu trường, save thất bại và dữ liệu không bị ghi sai.

### 12.2. Add và update

- [ ] Add hợp lệ, null, chưa load, ID trùng, cùng con trỏ, dữ liệu Model sai, tham chiếu thiếu/inactive, đã liên kết trước.
- [ ] Add trùng lịch, sát ranh 15 phút, khác phòng, CANCELLED; `hasScheduleConflict()` cho kết quả nhất quán với load/update.
- [ ] Link Movie thành công nhưng link Room thất bại; `repo.add()` thất bại sau link; kiểm rollback qua `unlinkReferences()`, count và quyền sở hữu con trỏ. Giả lập một unlink rollback trả false: vẫn thử cạnh còn lại, báo exception thay vì trả false giả; kiểm cả trường hợp chỉ có một cạnh được link.
- [ ] Update không đổi liên kết, chỉ đổi Movie, chỉ đổi Room, đổi cả hai; dữ liệu sai, tham chiếu thiếu/inactive, trùng lịch, trước load, ID không tồn tại. Kiểm `movieChanged`/`roomChanged` bằng con trỏ cũ–mới, ID và địa chỉ Showtime không đổi, cạnh không đổi không bị unlink/link lại.
- [ ] Giả lập `unlinkShowtime`, `setMovieId`, `setRoomId`, `linkShowtime` trả false hoặc ném lỗi ở từng bước; `catch (...)` gọi `restoreShowtime()` một lần, thuộc tính và hai danh sách ngược được khôi phục, lỗi gốc được ném tiếp.
- [ ] Giả lập lỗi ngay trong `restoreShowtime()`; xác nhận exception nêu rõ khôi phục thất bại, không báo thành công hoặc coi trạng thái đã an toàn.
- [ ] Khi refactor Service: từ chối đổi phim/phòng/thời gian nếu suất đã bắt đầu, từng có Ticket SOLD/CANCELLED hoặc Invoice; kiểm Permission, ngày phát hành và thời lượng trước khi gọi Repository. Chưa đánh dấu PASS trong bước tài liệu này.

### 12.3. Remove, truy vấn và vòng đời

- [ ] Remove hợp lệ, ID không tồn tại, trước load, thiếu cạnh Movie/Room, lỗi unlink Room và lỗi `repo.removeById()`; kiểm cả hai cạnh đều được thử khôi phục, không xóa Model khi thất bại.
- [ ] Destructor dùng `unlinkAll()`/`unlinkReferences()` để gỡ liên kết Movie/Room trước khi xóa Showtime; nếu unlink lỗi thì báo qua `cerr` và dừng trước khi `repo` hủy. Kiểm `findById()`, `getAt()` biên âm/vượt giới hạn, `count()`, `isLoaded()` qua interface.
- [ ] Sau khi refactor TicketRepository/InvoiceRepository, kiểm thử ShowtimeService từ chối xóa nếu từng có Ticket SOLD hoặc CANCELLED, hoặc còn Invoice tham chiếu; chỉ gọi `removeById()` khi cả hai điều kiện tiền đề đạt. Chưa có kết quả để đánh dấu PASS.
- [ ] Biên dịch từng test với toàn bộ `.cpp` phù hợp trong `src/` trừ `main.cpp`; xây dựng/chạy bài test hiện hành cho ShowtimeRepository và liên kết, rồi kiểm hồi quy module liên quan. Các bài `test_showtime_repository.cpp` và `test_showtime_link.cpp` hiện nằm trong `tests/legacy_test/`, chưa được dùng làm bằng chứng PASS cho bản này.

## 13. Tiến độ triển khai

- [x] Rà soát mã nguồn, Model, quy tắc validation, nghiệp vụ và định dạng TXT.
- [x] Viết mã `.h/.cpp` đề xuất và giải thích theo bố cục MovieRepository.
- [x] Ghi nhận giới hạn Ticket/Invoice và vòng đời liên kết.
- [x] Ghi rõ điều kiện xóa liên quan Ticket/Invoice do ShowtimeService kiểm tra sau refactor; giữ `Showtime oldValues`, `restoreShowtime()` và rollback như code hiện tại.
- [ ] Review và **CHỐT MARKDOWN** cho bản chuẩn hóa này.
- [ ] Triển khai code đúng 100% theo Markdown sau khi chốt.
- [ ] Biên dịch, chạy test liên quan, đối chiếu code/tài liệu và ghi kết quả thực tế.

## 14. Kết luận

Bản Markdown dự kiến giữ DIP, `Repository<Showtime>` bằng composition, luồng `load()` một vòng và rollback hiện tại. Trước khi gọi `removeById()`, ShowtimeService phải xác nhận chưa từng có Ticket, kể cả vé đã hủy, và không có Invoice tham chiếu; việc kiểm tra sẽ hoàn thiện sau refactor hai Repository liên quan. Bản này chờ CHỐT MARKDOWN trước khi đưa các thay đổi trình bày và `override` vào `.h/.cpp`.
