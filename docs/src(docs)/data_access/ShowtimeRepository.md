# ShowtimeRepository

## 1. Tổng quan

`ShowtimeRepository` quản lý dữ liệu suất chiếu ở tầng Data Access. Bản thiết kế này kế thừa `IRepository<Showtime>`, giữ `Repository<Showtime>` bằng composition và nhận `IRepository<Movie>&`, `IRepository<Room>&` qua constructor. Lớp kiểm tra dữ liệu Showtime, tham chiếu Movie/Room, lịch cùng phòng và liên kết hai chiều; không phân quyền hoặc tự ghi TXT sau mỗi thao tác.

**File liên quan:** `src/data_access/ShowtimeRepository.h/.cpp`, `src/data_access/IRepository.h`, `src/data_access/Repository.h`, `src/models/Showtime.h`, `src/models/Movie.h`, `src/models/Room.h`, `src/utils/Validator.h`, `src/utils/DateTime.h`.

**Trạng thái:** **Đã CHỐT MARKDOWN** và triển khai mã nguồn `.h/.cpp` đúng theo hai khối mã ở mục 5. `load()` dùng một vòng; `update()` cho phép đổi Movie/Room với rollback liên kết. ShowtimeService sẽ kiểm tra thời điểm và lịch sử Ticket/Invoice trước khi gọi `update()` khi refactor Business. Chưa kiểm thử mã triển khai.

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

### 2.2. Trách nhiệm validation

- `Repository<Showtime>` kiểm tra loaded, null/trùng con trỏ khi thêm, ID rỗng/trùng, tồn tại khi xóa và lỗi file.
- ShowtimeRepository kiểm tra thuộc tính Showtime, tham chiếu Movie/Room, trạng thái hoạt động của chúng đối với suất `SCHEDULED`, trùng lịch, liên kết hai chiều và rollback trước/sau khi gọi lớp lưu trữ chung.
- ShowtimeService kiểm tra Permission, suất đã bắt đầu, trạng thái, lịch sử Ticket/Invoice, lịch chiếu thuộc khoảng phát hành và thời lượng phim trước khi gọi `update()`. Repository không tự kiểm tra lại các điều kiện nghiệp vụ này.

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
| Constructor, destructor | Khởi tạo dependency; gỡ liên kết trước khi `repo` bị hủy. | Không |
| `load()`, `save()` | Nạp và lưu dữ liệu. | Có |
| `add()`, `update()`, `removeById()` | Thêm, cập nhật cả Movie/Room khi hợp lệ, xóa. | Có |
| `findById()`, `getAt()`, `count()`, `isLoaded()` | Truy vấn. | Có |

## 5. Mã nguồn đề xuất

### 5.1. ShowtimeRepository.h

**Đường dẫn dự kiến:** `src/data_access/ShowtimeRepository.h`

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

    static std::string getShowtimeId(Showtime* showtime);
    static Showtime* stringToShowtime(const std::string& line);
    static std::string showtimeToString(Showtime* showtime);
    static bool isValidShowtime(const Showtime* showtime);

    bool isValidReference(const Showtime* showtime);
    bool hasScheduleConflict(const Showtime* showtime, const std::string& excludeId = "");
    void unlinkReferences(Showtime* showtime);
    void unlinkAll();
    void restoreShowtime(Showtime* showtime, const Showtime& oldValues, Movie* oldMovie, Room* oldRoom);

public:
    ShowtimeRepository(const std::string& filePath, IRepository<Movie>& movieRepo, IRepository<Room>& roomRepo);
    ~ShowtimeRepository();

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

**Đường dẫn dự kiến:** `src/data_access/ShowtimeRepository.cpp`

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
      movieRepo(movieRepo), roomRepo(roomRepo)
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

| Thành phần | Vai trò trong mã đề xuất |
|---|---|
| `<string>` | `std::string`, đường dẫn, ID và dòng TXT. |
| `<vector>` | Tách dòng TXT thành sáu trường. |
| `<exception>` | `terminate()` khi destructor không thể gỡ liên kết an toàn. |
| `<iostream>` | `cerr` báo lỗi trước khi dừng trong destructor. |
| `<stdexcept>` | `runtime_error` cho dữ liệu/quan hệ/lịch lỗi khi load hoặc rollback không thể khôi phục. |
| `IRepository.h` | Hợp đồng chung cho Showtime, Movie và Room. |
| `Repository.h` | Danh sách sở hữu `Showtime*`, trạng thái nạp và FileIO. |
| `Showtime.h`, `Movie.h`, `Room.h` | Model, enum, getter, link/unlink. Header chỉ cần định nghĩa Showtime; `.cpp` cần đầy đủ Movie và Room. |
| `Validator.h`, `DateTime.h` | Kiểm ID, DateTime và lịch có khoảng nghỉ. |
| `override` | Compiler xác nhận chín phương thức khớp chữ ký interface. |
| `static` và callback | Hàm chuyển đổi/lấy ID không cần `this`, truyền được vào `Repository<Showtime>`. |
| `IRepository<T>&` | Dependency Injection không sở hữu repository được tham chiếu; đối tượng tham chiếu phải sống đủ lâu. |

## 7. Giải thích các hàm private

### 7.1. `isValidShowtime()`

**Tổng quan:** Nhận `const Showtime*`, trả `bool`; từ chối null, ID sai tiền tố, DateTime sai hoặc không tăng, status ngoài hai giá trị.

```text
null? → false → ID SHW/MOV/ROM hợp lệ? → DateTime hợp lệ?
→ startAt < endAt? → status thuộc SCHEDULED/CANCELLED? → true
      mỗi nhánh sai → false
```

**Giải thích code:** Kiểm `showtime == nullptr` trước mọi `->`. `Validator::isValidId` kiểm ba ID; `||` trả sớm nếu một ID sai. `Validator::isValidDateTime` kiểm chuỗi trước khi `DateTime::isBefore` so thứ tự. Enum được so tường minh để không serialize giá trị lạ. Hàm không tra Repository hoặc kiểm lịch.

### 7.2. `getShowtimeId()`

**Tổng quan:** Nhận `Showtime*`, trả ID dạng `string`; null trả chuỗi rỗng.

```text
showtime null? → ""; không → getShowtimeId()
```

**Giải thích code:** Null guard bảo vệ dereference trong callback. `Repository<T>` dùng chuỗi trả về để tìm, thêm và kiểm ID trùng.

### 7.3. `stringToShowtime()`

**Tổng quan:** Nhận một dòng TXT, trả `Showtime*` mới nếu hợp lệ; dữ liệu sai ném `runtime_error`.

```text
tách theo | → đúng 6 trường? ─ không → throw
→ status SCHEDULED/CANCELLED? ─ không → throw
→ new Showtime → isValidShowtime? ─ không → delete, throw
→ trả con trỏ cho Repository<Showtime>
```

**Giải thích code:** `vector<string> fields` giữ trường; `start`, `pos`, `find('|', start)` và `substr` giữ cả trường rỗng cuối. `string::npos` đánh dấu hết dấu phân cách. Sau kiểm số trường, hai nhánh status ánh xạ văn bản sang enum, nhánh `else` ném lỗi. `new Showtime(...)` tạo Model; nếu validation sai, `delete` trước khi ném để không rò bộ nhớ. Sau khi trả con trỏ, `Repository<Showtime>::load()` nhận quyền sở hữu tạm và thêm thông tin file/dòng khi có exception.

### 7.4. `showtimeToString()`

**Tổng quan:** Nhận `Showtime*`, trả dòng TXT; dữ liệu không hợp lệ ném `runtime_error`.

```text
isValidShowtime? ─ không → throw
→ chọn status → nối 6 trường bằng | → trả string
```

**Giải thích code:** Validation trước dereference chặn null và enum sai. Biến `status` nhận đúng `SCHEDULED` hoặc `CANCELLED`; biểu thức `+` nối các getter theo schema. FileIO do `Repository<Showtime>::save()` gọi, không có thao tác ghi ở đây.

### 7.5. `isValidReference()`

**Tổng quan:** Nhận `const Showtime*`, trả `bool`; caller đã kiểm thuộc tính bằng `isValidShowtime()`. Kiểm null để truy cập an toàn, trạng thái Movie/Room Repository, tồn tại tham chiếu và hoạt động đối với `SCHEDULED`.

```text
null? → false → Movie/Room đã load? ─ không → false
→ tìm Movie và Room → thiếu? → false
→ SCHEDULED và Movie/Room inactive? → false → true
```

**Giải thích code:** Không gọi lại `isValidShowtime()`. `findById` lấy con trỏ mượn, được kiểm `nullptr` trước `isActive()`. `CANCELLED` không đòi Movie/Room còn active để có thể đọc lịch sử. Hàm không kiểm giờ tương lai; việc đó thuộc Service.

### 7.6. `hasScheduleConflict()`

**Tổng quan:** Nhận Showtime cần kiểm và `excludeId` tùy chọn; trả `true` khi suất `SCHEDULED` cùng phòng chồng lịch, kể cả 15 phút nghỉ.

```text
null hoặc CANCELLED? → false
duyệt repo → bỏ null / chính ID exclude / khác phòng / CANCELLED
→ DateTime::isOverlapping? ─ có → true; không → tiếp tục
hết danh sách → false
```

**Giải thích code:** `excludeId` loại chính suất đang update hoặc đang được duyệt trong load; nó không loại suất khác vì ID chính đã duy nhất sau `repo.load()`. `current` null được bỏ qua trước khi đọc getter. `DateTime::isOverlapping` áp dụng khoảng nghỉ hiện hành. Helper được gọi ở load/add/update để tránh ba cách kiểm lịch khác nhau; chỉ gọi sau khi dữ liệu thời gian đã qua validation.

### 7.7. `unlinkReferences()`

**Tổng quan:** Nhận `Showtime*` đã được caller kiểm tra, trả `void`; gỡ các liên kết Movie và Room đang tồn tại. Nếu một thao tác unlink trả `false`, vẫn thử thao tác còn lại rồi ném `runtime_error`. Dùng khi rollback `add()` và khi gỡ từng suất trong `unlinkAll()`.

```text
lấy Movie/Room hiện liên kết từ Showtime
→ Movie có? → thử unlink, lưu movieUnlinked
→ Room có? → thử unlink, lưu roomUnlinked
→ một kết quả false? ─ có → throw; không → kết thúc
```

**Giải thích code:** `movie` và `room` là con trỏ mượn; hai biến `movieUnlinked` và `roomUnlinked` mặc định `true` cho cạnh không tồn tại. Mỗi cạnh có con trỏ mới gọi `unlinkShowtime()` và lưu kết quả riêng. Hai khối `if` độc lập bảo đảm thao tác Room vẫn được thử khi Movie trả `false`. Sau cả hai thao tác, một kết quả `false` gây exception, báo rollback không hoàn tất. Helper không xóa Showtime; nếu chính `unlinkShowtime()` ném exception thì hàm chuyển tiếp exception đó và không thể bảo đảm đã thử cạnh còn lại.

### 7.8. `unlinkAll()`

**Tổng quan:** Không tham số, trả `void`; gỡ các liên kết Movie/Room của mọi Showtime đang được `repo` giữ. Nếu unlink thất bại, ném `runtime_error` để không tiếp tục xóa Showtime khi còn liên kết. Dùng trong destructor và trước `repo.clear()` khi load rollback.

```text
duyệt repo → lấy Showtime → unlinkReferences(showtime)
→ một cạnh unlink trả false? → helper thử cả hai rồi throw
hết danh sách → kết thúc
```

**Giải thích code:** Mỗi lần lặp truyền `Showtime*` còn sống cho `unlinkReferences()`; helper xử lý cả Movie và Room, `unlinkAll()` không lặp lại logic này. Khi helper ném, vòng lặp dừng trước khi xóa Showtime. Trong rollback của `load()`, exception này ngăn `repo.clear()` và báo lỗi liên kết chưa được dọn, thay cho lỗi load ban đầu. Trong destructor, lỗi được báo qua `cerr` rồi dừng chương trình vì destructor của interface không cho ném exception ra ngoài. Thiết kế vẫn không tự phục hồi được trạng thái Model đã hỏng sẵn.

### 7.9. `restoreShowtime()`

**Tổng quan:** Nhận `Showtime* showtime`, bản sao thuộc tính cũ `const Showtime& oldValues`, cùng `Movie* oldMovie` và `Room* oldRoom`; trả `void`. Khôi phục các cạnh đã đổi và ba thuộc tính thời gian/status. Nếu khôi phục thất bại, ném `runtime_error` có thông điệp rõ ràng.

```text
Movie hiện khác oldMovie? → unlink Movie mới nếu có → setMovieId cũ → link Movie cũ
Room hiện khác oldRoom? → unlink Room mới nếu có → setRoomId cũ → link Room cũ
setStartAt, setEndAt, setStatus từ oldValues → kết thúc
unlink/set ID/link thất bại → runtime_error nêu cạnh không khôi phục được
```

**Giải thích code:** `oldValues` là Model độc lập giữ ID và thuộc tính trước khi `update()` sửa; nó không được link vào Repository. Helper xử lý Movie trước Room, đúng thứ tự hiện có trong khối `.cpp`. Khi con trỏ hiện tại khác con trỏ cũ, helper lấy con trỏ mới vào biến `movie`/`room`, kiểm null, gọi `unlinkShowtime()` và kiểm kết quả; sau đó `setMovieId()`/`setRoomId()` khôi phục ID trước khi link Model cũ. Con trỏ không đổi thì không thực hiện lại link. Ba setter cuối trả thời gian/status cũ. Hàm không kiểm null đầu vào hoặc bắt exception; `update()` chỉ gọi nó sau khi xác nhận `current`, `oldMovie`, `oldRoom` khác null. Nếu một bước khôi phục thất bại, `runtime_error` của helper được ném ra, có thể thay lỗi update gốc; không khẳng định rollback đã hoàn tất. Helper không giải phóng `Showtime*` hoặc lưu TXT.

## 8. Giải thích các hàm public

### 8.1. Constructor

**Tổng quan:** Nhận đường dẫn TXT, hai tham chiếu interface; không trả giá trị. Movie/Room Repository phải sống lâu hơn ShowtimeRepository.

```text
nhận filePath, movieRepo, roomRepo → khởi tạo repo với 3 callback
→ giữ hai tham chiếu → kết thúc, chưa load file
```

**Giải thích code:** Danh sách sau dấu `:` khởi tạo `repo` rồi hai tham chiếu theo thứ tự thuộc tính. Không cấp quyền sở hữu Movie/Room Repository; thân `{}` rỗng. Callback static có chữ ký phù hợp `Repository<Showtime>`.

### 8.2. Destructor

**Tổng quan:** Không tham số/giá trị trả về; tháo liên kết trước khi thành viên `repo` tự hủy. Nếu unlink thất bại, báo lỗi rồi `terminate()` vì destructor không được ném exception ra ngoài.

```text
~ShowtimeRepository → try unlinkAll()
→ thành công: hủy repo → delete Showtime*
→ exception: cerr báo lỗi → terminate()
```

**Giải thích code:** `try` gọi `unlinkAll()`. `catch (const exception& e)` in `e.what()` ra `cerr`; `catch (...)` báo lỗi không xác định. Cả hai nhánh lỗi gọi `terminate()` trước khi `repo` bị hủy để không xóa Showtime khi Movie/Room còn giữ liên kết. Khi thành công, C++ hủy `repo` sau thân destructor, dẫn tới `Repository<T>::clear()`. Cần hủy ShowtimeRepository trước MovieRepository và RoomRepository. Dừng chương trình là giới hạn của destructor `noexcept` từ `IRepository<Showtime>`; không thể trả lỗi khôi phục cho caller qua destructor.

### 8.3. `load()`

**Tổng quan:** Không tham số, trả `bool` nếu điều kiện nạp/file thành công hoặc thất bại; ném exception nếu dữ liệu, tham chiếu hay lịch lỗi. Rollback mọi liên kết đã tạo khi lỗi sau `repo.load()`.

```text
Movie/Room chưa load? → false → repo.load thất bại? → false
→ mỗi Showtime: tìm Movie/Room → tham chiếu tồn tại? → lịch không trùng?
→ linkMovie, linkRoom ─ lỗi tại bước nào → catch
→ true
catch → unlinkAll thành công? ─ có → repo.clear → throw lỗi gốc
                          └ không → ném lỗi unlink, không clear
```

**Giải thích code:** Kiểm dependency trước khi đọc file để không tạo dữ liệu chưa nối. `repo.load()` tự xử lý loaded, TXT, ID rỗng/trùng và giải phóng khi parse lỗi; `false` được trả nguyên. Một vòng `for` lấy `showtime`, tra Movie/Room đúng một lần, kiểm null, dùng `hasScheduleConflict(showtime, showtime->getShowtimeId())` để bỏ chính suất, rồi link hai phía. Vì helper kiểm toàn bộ danh sách đã nạp, không phụ thuộc vào thứ tự đã link. Nếu `linkMovie()` thành công nhưng `linkRoom()` thất bại hoặc ném lỗi, `unlinkAll()` thấy con trỏ Movie của suất vừa xử lý. Chỉ khi mọi unlink thành công, `repo.clear()` xóa Showtime và `throw` chuyển lỗi gốc. Nếu unlink thất bại, exception mới được báo và không clear để tránh con trỏ treo. Không gọi `isValidReference()` ở đây vì load cũ chỉ yêu cầu tham chiếu tồn tại; phim/phòng lịch sử có thể đã inactive. Không ghi file hoặc liên kết Ticket/Invoice trong hàm này.

### 8.4. `save()`

**Tổng quan:** Không tham số, trả kết quả `bool` của `repo.save()`.

```text
save → Repository<Showtime>::save → serialize từng suất → FileIO ghi TXT → bool
```

**Giải thích code:** `return repo.save();` chuyển lời gọi; Repository chung chặn khi chưa load và trả kết quả ghi. Callback `showtimeToString()` validate mỗi bản ghi, có thể ném exception; hàm không nuốt lỗi.

### 8.5. `add()`

**Tổng quan:** Nhận `Showtime*`, trả `bool`. Khi thất bại trước `repo.add()` hoặc `repo.add()` trả false, caller vẫn sở hữu con trỏ mới. Nếu rollback unlink thất bại, ném `runtime_error` thay vì báo `false` giả.

```text
chưa load / Model sai / đã link / ID trùng / tham chiếu sai / trùng lịch? → false
→ lấy Movie/Room để link
→ linkMovie rồi linkRoom; nếu cả hai thành công thì repo.add
→ added=false: unlinkReferences(showtime) → false (unlink lỗi → throw)
→ added=true: Repository sở hữu Showtime → true
exception khi link/add → unlinkReferences(showtime) → throw (unlink lỗi → báo lỗi rollback)
```

**Giải thích code:** Kiểm `repo.isLoaded()` sớm vì phải tạo liên kết **trước** `repo.add()`; tránh link rồi mới phát hiện chưa load. `isValidShowtime()` bảo vệ null/dữ liệu. Hai getter con trỏ chặn đối tượng đã liên kết; tìm ID trước khi link tránh thay đổi Model cho khóa trùng. `isValidReference()` không lặp validation thuộc tính; `hasScheduleConflict()` dùng chung. `movie`/`room` là con trỏ mượn để link. Trong `try`, toán tử `&&` chỉ gọi link Room sau khi link Movie thành công; `repo.add()` chỉ chạy khi cả hai link thành công. `added` ghi kết quả nhận quyền sở hữu. Khi exception xảy ra, `catch (...)` gọi `unlinkReferences(showtime)` rồi ném tiếp lỗi gốc nếu gỡ thành công. Khi `added` false, gọi cùng helper bên ngoài `try` rồi trả false. Helper thử gỡ cả hai cạnh hiện có và ném lỗi rõ ràng nếu một unlink trả false. Không `delete` con trỏ của caller; `Repository<T>` vẫn kiểm trùng con trỏ và khóa tại điểm nhận đối tượng.

### 8.6. `update()`

**Tổng quan:** Nhận `const Showtime& newValues`, trả `bool`; giữ nguyên `showtimeId` và địa chỉ `Showtime*`, cho phép đổi Movie, Room, thời gian và status khi dữ liệu, tham chiếu và lịch hợp lệ. Validation thất bại trước khi sửa trả `false`; lỗi trong lúc đổi liên kết được rollback rồi ném exception. ShowtimeService phải kiểm tra điều kiện đã bắt đầu và lịch sử Ticket/Invoice trước khi gọi.

```text
chưa load / dữ liệu mới sai / không tìm thấy? → false
tham chiếu mới sai / lịch trùng (bỏ current)? → false
lấy oldMovie, oldRoom, newMovie, newRoom; một con trỏ null? → false
movieChanged = oldMovie != newMovie; roomChanged = oldRoom != newRoom
tạo Showtime oldValues
movieChanged? → unlink oldMovie → setMovieId(newValues.getMovieId()) → link newMovie
roomChanged? → unlink oldRoom → setRoomId(newValues.getRoomId()) → link newRoom
một bước trả false → ném runtime_error vào catch
một bước ném exception → catch
catch → restoreShowtime(current, oldValues, oldMovie, oldRoom) → throw
không lỗi → setStartAt/EndAt/Status bằng getter của newValues → true
```

**Giải thích code:** `Repository<T>` không có `update()` nên kiểm loaded tại đây. `isValidShowtime(&newValues)` kiểm thuộc tính; `&` lấy địa chỉ tham số. `repo.findById(newValues.getShowtimeId())` định vị đúng đối tượng; không có setter đổi `showtimeId`, nên ID bất biến. `isValidReference()` tra Movie/Room mới và trạng thái active cho `SCHEDULED`; `hasScheduleConflict()` bỏ chính ID hiện tại. Bản `update()` đang làm chuẩn lấy `oldMovie`, `oldRoom`, `newMovie`, `newRoom` rồi trả false nếu một con trỏ null. Nó không có bước đối chiếu ID của Model cũ với ID lưu trong Showtime; các nhánh unlink/link của Model sẽ từ chối khi quan hệ không nhất quán.

**Giải thích code tiếp:** `newMovie`/`newRoom` được tra bằng getter của `newValues` và là con trỏ mượn; kiểm null sau khi tra là điểm bảo vệ trước link. `movieChanged`/`roomChanged` so sánh con trỏ cũ với mới để quyết định cạnh nào cần đổi. `Showtime oldValues(...)` tạo bản sao độc lập của ID, Movie ID, Room ID, thời gian và status cũ; không sao chép liên kết. Các getter của `newValues` được dùng trực tiếp khi đặt ID, thời gian và status; cạnh không đổi không bị unlink rồi link lại. Nếu `newValues` chính là `*current`, hai cặp con trỏ bằng nhau và không có thao tác đổi liên kết.

**Giải thích rollback:** Trong `try`, ba phép nối bằng `||` dùng short circuit: nếu unlink cũ thất bại thì không đổi ID; nếu đổi ID thất bại thì không link mới. Mỗi nhánh `false` được chuyển thành `runtime_error` để đi chung đường `catch (...)`. Movie được đổi trước Room, sau đó ba setter cập nhật thời gian/status. `catch (...)` gọi `restoreShowtime()` đúng một nơi rồi `throw;` ném lại lỗi gốc khi khôi phục thành công. Nếu helper ném, lỗi khôi phục được báo thay cho lỗi gốc. Không thay con trỏ `Showtime*`, không xóa Model và không tự save. Các trường hợp validation trước `try` vẫn trả `false` và không cần rollback.

**Điều kiện gọi từ Service:** Với thao tác đổi phim, phòng hoặc thời gian, Service phải từ chối suất đã bắt đầu hoặc từng phát sinh **bất kỳ** Ticket, kể cả vé `CANCELLED`; kiểm Invoice liên quan và tính nhất quán tham chiếu trước khi gọi Repository. Service cũng phải kiểm ngày chiếu thuộc khoảng phát hành, `endAt` theo thời lượng Movie, Permission và quy tắc trạng thái. Bản `.cpp` đề xuất không thể tự xác minh lịch sử Ticket/Invoice vì không có dependency hai Repository này; gọi trực tiếp `update()` từ nơi khác mà bỏ qua Service sẽ không bảo đảm các ràng buộc nghiệp vụ trên.

### 8.7. `removeById()`

**Tổng quan:** Nhận ID, trả `bool`; gỡ liên kết Movie/Room rồi xóa đối tượng nếu cả hai bước thành công. Chưa kiểm tra Ticket/Invoice; xem mục 9.

```text
chưa load / không tìm thấy / thiếu Movie hoặc Room? → false
→ unlink Movie thất bại? → false
→ unlink Room thất bại? → link lại Movie; lỗi khôi phục → throw; còn lại false
→ repo.removeById thất bại? → link lại Movie/Room; lỗi khôi phục → throw; còn lại false
→ true
```

**Giải thích code:** Loaded được kiểm trước khi unlink vì thao tác có tác dụng phụ. `showtime` được kiểm null trước khi đọc hai con trỏ Model; thiếu một cạnh thì từ chối xóa. Sau khi `movie->unlinkShowtime`, nếu Room không unlink được, `movie->linkShowtime` khôi phục cạnh đầu. Nếu xóa trong `repo` thất bại, hai biến `movieRestored`/`roomRestored` buộc **cả hai** lệnh link được thử trước khi kiểm kết quả; cách này tránh short circuit để lại một cạnh chưa được thử khôi phục. Lỗi khôi phục ném `runtime_error` thay vì báo thành công. Chỉ `Repository<Showtime>::removeById()` giải phóng Showtime; không `delete` trực tiếp. Luồng này giữ hành vi hiện tại, không tự thêm chính sách giao dịch.

### 8.8. `findById()`

**Tổng quan:** Nhận ID, trả `Showtime*` mượn hoặc null.

```text
findById → repo.findById → Showtime* / nullptr
```

**Giải thích code:** Chuyển lời gọi; không kiểm lại loaded hoặc quyền truy cập. Caller không được xóa con trỏ mượn.

### 8.9. `getAt()`

**Tổng quan:** Nhận chỉ số, trả `Showtime*` mượn hoặc null nếu ngoài phạm vi.

```text
getAt(index) → repo.getAt(index) → Showtime* / nullptr
```

**Giải thích code:** Repository chung kiểm vị trí qua `DoublyLinkedList`; không chuyển sở hữu.

### 8.10. `count()`

**Tổng quan:** Không tham số; trả số suất trong danh sách.

```text
count → repo.count → int
```

**Giải thích code:** Không đổi trạng thái và không cần lặp vòng đếm trong Concrete Repository.

### 8.11. `isLoaded()`

**Tổng quan:** Không tham số; trả trạng thái nạp dữ liệu.

```text
isLoaded → repo.isLoaded → bool
```

**Giải thích code:** Chuyển trạng thái của Repository chung; ShowtimeRepository không giữ cờ loaded thứ hai.

## 9. Quản lý bộ nhớ và ràng buộc Ticket/Invoice

`Repository<Showtime>` sở hữu `Showtime*` sau add/load thành công; caller sở hữu con trỏ mới khi add thất bại. Movie/Room chỉ giữ liên kết không sở hữu. `unlinkAll()` gỡ hai cạnh trước khi destructor xóa Showtime hoặc `load()` gọi `repo.clear()`; `removeById()` gỡ hai cạnh trước khi xóa một suất. Các Repository nền phải còn sống khi destructor chạy.

**Vấn đề cần nhóm quyết định, không được coi đã giải quyết:** `Ticket` và `Invoice` giữ con trỏ `Showtime*` cùng `showtimeId`, nhưng `Showtime` không có danh sách ngược Ticket/Invoice. `ShowtimeRepository` hiện chỉ có dependency Movie/Room; nó không thể kiểm tra trực tiếp lịch sử vé/hóa đơn trước khi `removeById()` giải phóng Showtime. `ShowtimeService::removeShowtime()` hiện luôn trả `false` sau khi kiểm quyền/tồn tại, với TODO về suất đã từng có vé. Gọi trực tiếp Repository để xóa sau khi Ticket/Invoice đã link có thể để con trỏ treo. Quy tắc bảo vệ lịch sử giao dịch trong `business_rules.md` cần được thiết kế và CHỐT riêng trước khi mở luồng xóa thực tế; bản này **giữ nguyên hành vi xóa hiện tại** và không tự thêm dependency/API/quy tắc xóa.

`unlinkReferences()` kiểm kết quả gỡ cả hai cạnh của một Showtime, dùng lại trong `unlinkAll()` và rollback `add()`. Trong `load()`, lỗi từ helper ngăn `repo.clear()` để tránh xóa Showtime còn được tham chiếu; trong destructor nó được báo ra `cerr` rồi `terminate()` vì không thể ném qua destructor của interface. Đây là xử lý trạng thái lỗi rõ ràng, không khẳng định đã tự sửa được quan hệ Model hỏng sẵn.

**Trách nhiệm đã thống nhất cho `update()`:** `business_rules.md` cấm đổi phim/phòng/thời gian của suất đã bắt đầu hoặc từng có vé, kể cả vé đã hủy. ShowtimeService sẽ dùng TicketRepository và InvoiceRepository khi refactor Business để kiểm lịch sử trước khi gọi `ShowtimeRepository::update()`; không thêm các dependency đó vào ShowtimeRepository vì chúng đang phụ thuộc ngược vào ShowtimeRepository. Service hiện chưa có các dependency này, nên điều kiện gọi chưa được triển khai. Repository chỉ xác thực Movie/Room, lịch và liên kết, rồi rollback thay đổi nội bộ khi lỗi. Không suy diễn "không thấy con trỏ" là "chưa từng có giao dịch".

## 10. Áp dụng Dependency Inversion Principle

Trước refactor, ShowtimeRepository giữ `MovieRepository&` và `RoomRepository&`. Bản đề xuất giữ đúng lời gọi `isLoaded()`/`findById()` nhưng chuyển thành `IRepository<Movie>&` và `IRepository<Room>&`; constructor nhận dependency từ nơi khởi tạo. `ShowtimeRepository` kế thừa `IRepository<Showtime>` cho Service dùng. `Repository<Showtime>` tiếp tục là thành viên composition, không thêm `update()` vào lớp dùng chung.

## 11. Nội dung thay đổi dự kiến

- `.h`: Kế thừa interface, thêm `override`, đổi hai tham chiếu/constructor sang interface, khai báo `unlinkReferences()`, `unlinkAll()` và `restoreShowtime()`; giữ `Repository<Showtime> repo` và chữ ký public hiện có.
- `.cpp`: Dùng `unlinkReferences()` cho rollback `add()` và từng Showtime trong `unlinkAll()`; destructor và catch của `load()` tiếp tục gọi `unlinkAll()`. Bỏ lần gọi `isValidShowtime()` bên trong `isValidReference()`; dùng `hasScheduleConflict()` trong load/add/update. `load()` dùng một vòng: tìm Movie/Room, kiểm tham chiếu, kiểm lịch, link. `removeById()` thử khôi phục cả hai cạnh; `update()` giữ nguyên bản do nhóm vừa chỉnh với `Showtime oldValues` và `restoreShowtime()` trong `catch (...)`.
- Giữ các kiểm tra trước link/unlink và cập nhật cần thiết: loaded ở add/update/remove, dữ liệu Model, trạng thái liên kết sẵn, trùng ID trước link, con trỏ Movie/Room, ràng buộc lịch, rollback khi link, update hoặc xóa thất bại.
- Giữ schema TXT, mã ID, Permission ở Service, quyền sở hữu con trỏ và hành vi xóa hiện tại. Kiểm tra lịch sử Ticket/Invoice trước update là điều kiện gọi thuộc ShowtimeService khi refactor Business; ràng buộc xóa Ticket/Invoice vẫn chưa được giải quyết trong phạm vi thiết kế này.

## 12. Kế hoạch kiểm thử

Chưa chạy test cho mã đã triển khai; mọi mục dưới đây còn chờ kiểm chứng. Dùng file TXT test riêng và không sửa test để tạo PASS.

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
- [ ] Rà soát riêng trường hợp đã có Ticket/Invoice trước khi cho phép xóa thực tế; kiểm không còn con trỏ treo sau khi chính sách xóa được nhóm chốt. **Chưa có thiết kế/test để đánh dấu PASS.**
- [ ] Biên dịch từng test với toàn bộ `.cpp` phù hợp trong `src/` trừ `main.cpp`; chạy `test_showtime_repository.cpp`, `test_showtime_link.cpp` và kiểm hồi quy các module liên quan nếu có.

## 13. Tiến độ triển khai

- [x] Rà soát mã nguồn, Model, quy tắc validation, nghiệp vụ và định dạng TXT.
- [x] Viết mã `.h/.cpp` đề xuất và giải thích theo bố cục MovieRepository.
- [x] Ghi nhận giới hạn Ticket/Invoice và vòng đời liên kết.
- [x] Thống nhất ShowtimeService kiểm lịch sử Ticket/Invoice khi refactor Business; thiết kế `Showtime oldValues` và helper `restoreShowtime()` cho rollback trong Markdown.
- [x] Review và **CHỐT MARKDOWN**.
- [x] Triển khai code đúng 100% theo Markdown sau khi chốt.
- [ ] Biên dịch, chạy test liên quan, đối chiếu code/tài liệu và ghi kết quả thực tế.

## 14. Kết luận

Thiết kế đã chốt áp dụng một interface `IRepository<T>`, giữ `Repository<Showtime>` bằng composition, dùng một vòng trong `load()` và cho `update()` đổi Movie/Room có rollback. Mã `.h/.cpp` đã được triển khai theo Markdown nhưng chưa kiểm thử; ShowtimeService còn phải triển khai kiểm tra lịch sử Ticket/Invoice trước update, còn ràng buộc xóa vẫn cần quyết định riêng.
