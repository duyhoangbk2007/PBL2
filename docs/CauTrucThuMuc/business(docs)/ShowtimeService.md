# ShowtimeService

## 1. Tổng quan

`ShowtimeService` là lớp thuộc tầng **Business Logic**, chịu trách nhiệm xử lý nghiệp vụ quản lý suất chiếu trong hệ thống rạp chiếu phim.

Lớp sử dụng `ShowtimeRepository` để lưu trữ và truy xuất suất chiếu, `MovieRepository` để đối chiếu thông tin phim, `RoomRepository` làm phụ thuộc phục vụ nghiệp vụ liên quan đến phòng và `AuthService` để kiểm tra quyền người dùng.

`ShowtimeService` **không trực tiếp đọc/ghi file TXT**. Các kiểm tra về dữ liệu, tham chiếu phim/phòng và trùng lịch ở tầng Repository vẫn phải được thực hiện; Service bổ sung kiểm tra quyền và các điều kiện thời gian nghiệp vụ.

**File liên quan:**
- `src/business/ShowtimeService.h`
- `src/business/ShowtimeService.cpp`
- `src/business/AuthService.h`, `src/business/AuthService.cpp`
- `src/data_access/ShowtimeRepository.h`, `src/data_access/ShowtimeRepository.cpp`
- `src/data_access/MovieRepository.h`, `src/data_access/RoomRepository.h`
- `src/models/Showtime.h`, `src/models/Showtime.cpp`
- `src/utils/DateTime.h`, `src/utils/Validator.h`
- `tests/test_showtime_service.cpp`

## 2. Trách nhiệm và phạm vi

- Admin được tạo và cập nhật suất chiếu.
- Admin được gọi các thao tác hủy/xóa suất, nhưng **hai hàm này hiện chưa hoàn thiện và luôn trả `false` sau khi kiểm tra điều kiện ban đầu**.
- Người dùng đã đăng nhập hợp lệ (Admin hoặc Staff) được tra cứu suất chiếu.
- Khi tạo/sửa, kiểm tra suất bắt đầu trong tương lai, ngày chiếu thuộc khoảng phát hành phim và thời gian kết thúc khớp thời lượng phim.
- Chỉ cho phép tạo suất ở trạng thái `SCHEDULED`; không sửa suất đã hủy hoặc đã bắt đầu.
- Giao việc kiểm tra tham chiếu, phòng/phim hợp lệ, xung đột lịch và thao tác dữ liệu cho `ShowtimeRepository`.
- Không tự động hủy vé, không xóa vé hoặc hóa đơn khi thay đổi suất chiếu.

## 3. Thuộc tính và khởi tạo

```cpp
private:
    ShowtimeRepository& showtimeRepo;
    MovieRepository& movieRepo;
    RoomRepository& roomRepo;
    AuthService& authService;

    bool isValidSchedule(const Showtime& showtime);
```

Constructor:

```cpp
ShowtimeService(
    ShowtimeRepository& showtimeRepo,
    MovieRepository& movieRepo,
    RoomRepository& roomRepo,
    AuthService& authService
);
```

Các thuộc tính đều là **tham chiếu**, giúp Service dùng chung các Repository và phiên đăng nhập đã được khởi tạo bên ngoài, không sao chép các đối tượng quản lý dữ liệu.

`roomRepo` được nhận và lưu trong Service nhưng **chưa được gọi trực tiếp trong các hàm của `ShowtimeService.cpp` hiện tại**. Việc kiểm tra phòng diễn ra qua `ShowtimeRepository`.

**Lưu ý thứ tự nạp dữ liệu:** cần nạp `MovieRepository` và `RoomRepository` trước `ShowtimeRepository`, vì suất chiếu tham chiếu đến phim và phòng. `UserRepository` phải được nạp trước khi đăng nhập bằng `AuthService`. Khi dùng chung Repository, chỉ nạp một lần; không gọi `load()` lặp lại ở nhiều Service.

## 4. Phân quyền

| Thao tác | Admin | Staff | Chưa đăng nhập |
| --- | --- | --- | --- |
| `load()`, `save()` | Không kiểm tra quyền trong hàm | Không kiểm tra quyền trong hàm | Không kiểm tra quyền trong hàm |
| `addShowtime()` | Cho phép | Từ chối bằng ngoại lệ | Từ chối bằng ngoại lệ |
| `updateShowtime()` | Cho phép | Từ chối bằng ngoại lệ | Từ chối bằng ngoại lệ |
| `cancelShowtime()` | Được gọi, nhưng hiện trả `false` | Từ chối bằng ngoại lệ | Từ chối bằng ngoại lệ |
| `removeShowtime()` | Được gọi, nhưng hiện trả `false` | Từ chối bằng ngoại lệ | Từ chối bằng ngoại lệ |
| `findShowtimeById()` | Cho phép | Cho phép | Từ chối bằng ngoại lệ |
| `getShowtimeAt()` | Cho phép | Cho phép | Từ chối bằng ngoại lệ |
| `getShowtimeCount()` | Cho phép | Cho phép | Từ chối bằng ngoại lệ |

`authService.requireAdmin()` và `authService.requireLogin()` **ném `runtime_error`** khi không đủ quyền; chúng không trả về `false`. `load()` và `save()` chỉ chuyển tiếp đến Repository, không tự kiểm tra quyền.

## 5. Danh sách hàm

| Hàm | Kiểu trả về | Chức năng |
| --- | --- | --- |
| `ShowtimeService(ShowtimeRepository&, MovieRepository&, RoomRepository&, AuthService&)` | Constructor | Nhận các phụ thuộc dùng chung |
| `isValidSchedule(const Showtime&)` | `bool` (private) | Kiểm tra thời gian chiếu theo nghiệp vụ |
| `load()` | `bool` | Nạp dữ liệu suất chiếu |
| `save()` | `bool` | Lưu dữ liệu suất chiếu |
| `addShowtime(Showtime*)` | `bool` | Tạo suất chiếu mới |
| `updateShowtime(const Showtime&)` | `bool` | Cập nhật suất chiếu chưa bắt đầu |
| `cancelShowtime(const string&)` | `bool` | Dự kiến hủy suất; **chưa triển khai hoàn chỉnh** |
| `removeShowtime(const string&)` | `bool` | Dự kiến xóa suất; **chưa triển khai hoàn chỉnh** |
| `findShowtimeById(const string&)` | `const Showtime*` | Tìm suất theo ID |
| `getShowtimeAt(int)` | `const Showtime*` | Lấy suất tại vị trí trong Repository |
| `getShowtimeCount()` | `int` | Đếm tổng số suất |

## Chi tiết triển khai từng hàm (ShowtimeService.cpp)

### ShowtimeService()

#### A. Tổng quan

- **Mục đích:** Khởi tạo Service với các Repository và AuthService.
- **Tham số:** `ShowtimeRepository& showtimeRepo, MovieRepository& movieRepo, RoomRepository& roomRepo, AuthService& authService) : showtimeRepo(showtimeRepo), movieRepo(movieRepo), roomRepo(roomRepo), authService(authService`.
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
ShowtimeService::ShowtimeService(ShowtimeRepository& showtimeRepo, MovieRepository& movieRepo, RoomRepository& roomRepo, AuthService& authService)
    : showtimeRepo(showtimeRepo),
      movieRepo(movieRepo),
      roomRepo(roomRepo),
      authService(authService)
{}
```

Các câu lệnh được thực thi theo đúng thứ tự trong `.cpp`; điều kiện và nhánh trả về nằm ngay trong code. Giá trị được khai báo ở đây được dùng trong các nhánh tiếp theo; hãy theo dõi nơi nó được trả về hoặc chuyển vào Repository để hiểu tác động lên trạng thái.

### isValidSchedule()

#### A. Tổng quan

- **Mục đích:** Tra cứu dữ liệu. Xem các nhánh cụ thể trong code.
- **Tham số:** `const Showtime& showtime`.
- **Giá trị trả về:** `bool`; các nhánh `return` được ghi đầy đủ bên dưới.
- **Điều kiện quan trọng:** kiểm tra trạng thái nạp và con trỏ trước khi dùng.

#### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Tra cứu dữ liệu
    |
    v
Từ chối dữ liệu không hợp lệ
    |
    v
Từ chối dữ liệu không hợp lệ
    |
    v
Kết thúc
```

#### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1 — Tra cứu dữ liệu.**

```cpp
bool ShowtimeService::isValidSchedule(const Showtime& showtime)
{
    Movie* movie = movieRepo.findById(showtime.getMovieId());
    if (movie == nullptr)
    {
        return false;
    }
    const string& startAt = showtime.getStartAt();
    if (!Validator::isValidDateTime(startAt))
    {
```

`nullptr` cho biết không có đối tượng để truy cập. `false` báo thao tác không thành công. Lệnh tra cứu trả con trỏ đến đối tượng đang được Repository quản lý. Con trỏ rỗng có nghĩa là không tìm thấy, nên bước sau chỉ được dùng đối tượng khi kiểm tra thành công. Nhánh `false` dừng hàm tại đây; những câu lệnh phía sau không chạy, nên không có cập nhật nào từ các bước sau.

**Bước 2 — Từ chối dữ liệu không hợp lệ.**

```cpp
        return false;
    }
    // Suat chieu phai bat dau trong tuong lai
    if (!DateTime::isBefore(DateTime::getCurrentDateTime(), startAt))
    {
        return false;
    }
    // Ngay chieu nam trong khoang phat hanh phim
    string showDate = startAt.substr(0, 10);
    if (showDate < movie->getReleaseDate() ||
```

`false` báo thao tác không thành công. `DateTime` cung cấp thời gian hoặc phép so sánh thời gian; điều kiện này quyết định dữ liệu có còn hợp lệ ở thời điểm gọi hàm hay không. Nhánh `false` dừng hàm tại đây; những câu lệnh phía sau không chạy, nên không có cập nhật nào từ các bước sau.

**Bước 3 — Từ chối dữ liệu không hợp lệ.**

```cpp
        showDate > movie->getEndDate())
    {
        return false;
    }
    // Thoi gian ket thuc phai khop thoi luong phim
    string expectedEnd = DateTime::addMinutes(startAt, movie->getDurationMinutes());
    return showtime.getEndAt() == expectedEnd;
}
```

`false` báo thao tác không thành công. `DateTime` cung cấp thời gian hoặc phép so sánh thời gian; điều kiện này quyết định dữ liệu có còn hợp lệ ở thời điểm gọi hàm hay không. Nhánh `false` dừng hàm tại đây; những câu lệnh phía sau không chạy, nên không có cập nhật nào từ các bước sau.

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
bool ShowtimeService::load()
{
    return showtimeRepo.load();
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
bool ShowtimeService::save()
{
    return showtimeRepo.save();
}
```

Repository thực hiện thao tác dữ liệu. `save()` ghi trạng thái Repository ra TXT tại thời điểm lời gọi; thay đổi RAM trước đó không tự đồng nghĩa với đã lưu thành công.

### addShowtime()

#### A. Tổng quan

- **Mục đích:** Kiểm tra quyền. Xem các nhánh cụ thể trong code.
- **Tham số:** `Showtime* showtime`.
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
Tra cứu dữ liệu
    |
    v
Cập nhật qua Repository
    |
    v
Từ chối dữ liệu không hợp lệ
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
bool ShowtimeService::addShowtime(Showtime* showtime)
{
    authService.requirePermission("SHOWTIME_ADD");

    if (!showtimeRepo.isLoaded() ||
        !movieRepo.isLoaded() ||
        !roomRepo.isLoaded() ||
        showtime == nullptr)
    {
        return false;
```

`AuthService` kiểm tra quyền và có thể ném ngoại lệ. `nullptr` cho biết không có đối tượng để truy cập. `false` báo thao tác không thành công. Quyền `SHOWTIME_ADD` được kiểm tra trước bước xử lý kế tiếp; thiếu quyền sẽ ném ngoại lệ, vì vậy caller không nên xem đó là kết quả `false`. Kiểm tra `isLoaded()` tránh dùng dữ liệu Repository chưa được nạp; nhánh không đạt kết thúc ngay theo giá trị `return` trong code. Nhánh `false` dừng hàm tại đây; những câu lệnh phía sau không chạy, nên không có cập nhật nào từ các bước sau.

**Bước 2 — Từ chối dữ liệu không hợp lệ.**

```cpp
    }
    if (showtime->getStatus() != ShowtimeStatus::SCHEDULED)
    {
        return false;
    }
    if (!isValidSchedule(*showtime))
    {
        return false;
    }
    vector<string> existingIds;
```

`false` báo thao tác không thành công. Nhánh `false` dừng hàm tại đây; những câu lệnh phía sau không chạy, nên không có cập nhật nào từ các bước sau.

**Bước 3 — Tra cứu dữ liệu.**

```cpp
    for (int i = 0; i < showtimeRepo.count(); i++)
    {
        Showtime* current = showtimeRepo.getAt(i);
        if (current == nullptr)
        {
            return false;
        }
        existingIds.push_back(current->getShowtimeId());
    }
    string showtimeId = IdGenerator::generateNextId("SHW", existingIds);
```

`nullptr` cho biết không có đối tượng để truy cập. `false` báo thao tác không thành công. Vòng `for` duyệt lần lượt các phần tử. Nhánh `false` dừng hàm tại đây; những câu lệnh phía sau không chạy, nên không có cập nhật nào từ các bước sau. Vòng lặp xử lý từng phần tử theo thứ tự; điều kiện bên trong quyết định phần tử được giữ, bỏ qua hay đưa vào kết quả.

**Bước 4 — Cập nhật qua Repository.**

```cpp
    Showtime* newShowtime = new Showtime(showtimeId,
                                         showtime->getMovieId(),
                                         showtime->getRoomId(),
                                         showtime->getStartAt(),
                                         showtime->getEndAt(),
                                         showtime->getStatus());

    try
    {
        if (!showtimeRepo.add(newShowtime))
```

Repository thực hiện thao tác dữ liệu. Khi `add()` thành công, Repository nhận đối tượng và quản lý vòng đời của nó; nếu thất bại, bên gọi vẫn phải xét trách nhiệm giải phóng con trỏ. Khối `try/catch` giữ lỗi phát sinh trong phạm vi xử lý này và chuyển sang nhánh khôi phục hoặc báo lỗi đã viết trong code. `new` cấp phát đối tượng trên heap; cần theo dõi thời điểm quyền sở hữu được chuyển sang Repository để tránh rò rỉ bộ nhớ.

**Bước 5 — Từ chối dữ liệu không hợp lệ.**

```cpp
        {
            delete newShowtime;
            return false;
        }
    }
    catch (...)
    {
        delete newShowtime;
        throw;
    }
```

`false` báo thao tác không thành công. Nhánh `false` dừng hàm tại đây; những câu lệnh phía sau không chạy, nên không có cập nhật nào từ các bước sau. Khối `try/catch` giữ lỗi phát sinh trong phạm vi xử lý này và chuyển sang nhánh khôi phục hoặc báo lỗi đã viết trong code. `delete` giải phóng vùng nhớ của đối tượng, sau đó không được dùng lại con trỏ trỏ đến vùng nhớ đó.

**Bước 6 — Trả kết quả.**

```cpp
    return true;
}
```

Các câu lệnh được thực thi theo đúng thứ tự trong `.cpp`; điều kiện và nhánh trả về nằm ngay trong code. Giá trị được khai báo ở đây được dùng trong các nhánh tiếp theo; hãy theo dõi nơi nó được trả về hoặc chuyển vào Repository để hiểu tác động lên trạng thái.

### updateShowtime()

#### A. Tổng quan

- **Mục đích:** Kiểm tra quyền. Xem các nhánh cụ thể trong code.
- **Tham số:** `const Showtime& newValues`.
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
bool ShowtimeService::updateShowtime(const Showtime& newValues)
{
    authService.requirePermission("SHOWTIME_UPDATE");
    Showtime* current = showtimeRepo.findById(newValues.getShowtimeId());
    if (current == nullptr)
    {
        return false;
    }
    // Khong sua suat da huy
    if (current->getStatus() != ShowtimeStatus::SCHEDULED || newValues.getStatus() != ShowtimeStatus::SCHEDULED)
```

`AuthService` kiểm tra quyền và có thể ném ngoại lệ. `nullptr` cho biết không có đối tượng để truy cập. `false` báo thao tác không thành công. Quyền `SHOWTIME_UPDATE` được kiểm tra trước bước xử lý kế tiếp; thiếu quyền sẽ ném ngoại lệ, vì vậy caller không nên xem đó là kết quả `false`. Lệnh tra cứu trả con trỏ đến đối tượng đang được Repository quản lý. Con trỏ rỗng có nghĩa là không tìm thấy, nên bước sau chỉ được dùng đối tượng khi kiểm tra thành công. Nhánh `false` dừng hàm tại đây; những câu lệnh phía sau không chạy, nên không có cập nhật nào từ các bước sau.

**Bước 2 — Từ chối dữ liệu không hợp lệ.**

```cpp
    {
        return false;
    }
    // Khong sua suat da bat dau
    if (!DateTime::isBefore(DateTime::getCurrentDateTime(), current->getStartAt()))
    {
        return false;
    }
    if (!isValidSchedule(newValues))
    {
```

`false` báo thao tác không thành công. `DateTime` cung cấp thời gian hoặc phép so sánh thời gian; điều kiện này quyết định dữ liệu có còn hợp lệ ở thời điểm gọi hàm hay không. Nhánh `false` dừng hàm tại đây; những câu lệnh phía sau không chạy, nên không có cập nhật nào từ các bước sau.

**Bước 3 — Cập nhật qua Repository.**

```cpp
        return false;
    }
    return showtimeRepo.update(newValues);
}
```

`false` báo thao tác không thành công. Repository thực hiện thao tác dữ liệu. `update()` chuyển việc thay đổi dữ liệu cho Repository; kết quả trả về phản ánh kiểm tra và thao tác cập nhật ở tầng dữ liệu. Nhánh `false` dừng hàm tại đây; những câu lệnh phía sau không chạy, nên không có cập nhật nào từ các bước sau.

### cancelShowtime()

#### A. Tổng quan

- **Mục đích:** Kiểm tra quyền. Xem các nhánh cụ thể trong code.
- **Tham số:** `const string& showtimeId`.
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
Từ chối dữ liệu không hợp lệ
    |
    v
Kết thúc
```

#### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1 — Kiểm tra quyền.**

```cpp
bool ShowtimeService::cancelShowtime(const string& showtimeId)
{
    authService.requirePermission("SHOWTIME_CANCEL");

    if (!showtimeRepo.isLoaded())
    {
        return false;
    }
    Showtime* current = showtimeRepo.findById(showtimeId);
    if (current == nullptr)
```

`AuthService` kiểm tra quyền và có thể ném ngoại lệ. `nullptr` cho biết không có đối tượng để truy cập. `false` báo thao tác không thành công. Quyền `SHOWTIME_CANCEL` được kiểm tra trước bước xử lý kế tiếp; thiếu quyền sẽ ném ngoại lệ, vì vậy caller không nên xem đó là kết quả `false`. Kiểm tra `isLoaded()` tránh dùng dữ liệu Repository chưa được nạp; nhánh không đạt kết thúc ngay theo giá trị `return` trong code. Lệnh tra cứu trả con trỏ đến đối tượng đang được Repository quản lý. Con trỏ rỗng có nghĩa là không tìm thấy, nên bước sau chỉ được dùng đối tượng khi kiểm tra thành công. Nhánh `false` dừng hàm tại đây; những câu lệnh phía sau không chạy, nên không có cập nhật nào từ các bước sau.

**Bước 2 — Từ chối dữ liệu không hợp lệ.**

```cpp
    {
        return false;
    }
    // Khong huy lai suat da huy
    if (current->getStatus() != ShowtimeStatus::SCHEDULED)
    {
        return false;
    }
    // Chi huy suat chua bat dau
    if (!DateTime::isBefore(DateTime::getCurrentDateTime(), current->getStartAt()))
```

`false` báo thao tác không thành công. `DateTime` cung cấp thời gian hoặc phép so sánh thời gian; điều kiện này quyết định dữ liệu có còn hợp lệ ở thời điểm gọi hàm hay không. Nhánh `false` dừng hàm tại đây; những câu lệnh phía sau không chạy, nên không có cập nhật nào từ các bước sau.

**Bước 3 — Từ chối dữ liệu không hợp lệ.**

```cpp
    {
        return false;
    }

    // TODO: Kiem tra khong con ve SOLD khi co TicketRepository.
    // Hien tai chua du thong tin de huy suat an toan.
    return false;
}
```

`false` báo thao tác không thành công. Nhánh `false` dừng hàm tại đây; những câu lệnh phía sau không chạy, nên không có cập nhật nào từ các bước sau.

### removeShowtime()

#### A. Tổng quan

- **Mục đích:** Kiểm tra quyền. Xem các nhánh cụ thể trong code.
- **Tham số:** `const string& showtimeId`.
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
Kết thúc
```

#### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1 — Kiểm tra quyền.**

```cpp
bool ShowtimeService::removeShowtime(const string& showtimeId)
{
    authService.requirePermission("SHOWTIME_DELETE");

    if (!showtimeRepo.isLoaded())
    {
        return false;
    }

    Showtime* current = showtimeRepo.findById(showtimeId);
```

`AuthService` kiểm tra quyền và có thể ném ngoại lệ. `false` báo thao tác không thành công. Quyền `SHOWTIME_DELETE` được kiểm tra trước bước xử lý kế tiếp; thiếu quyền sẽ ném ngoại lệ, vì vậy caller không nên xem đó là kết quả `false`. Kiểm tra `isLoaded()` tránh dùng dữ liệu Repository chưa được nạp; nhánh không đạt kết thúc ngay theo giá trị `return` trong code. Lệnh tra cứu trả con trỏ đến đối tượng đang được Repository quản lý. Con trỏ rỗng có nghĩa là không tìm thấy, nên bước sau chỉ được dùng đối tượng khi kiểm tra thành công. Nhánh `false` dừng hàm tại đây; những câu lệnh phía sau không chạy, nên không có cập nhật nào từ các bước sau.

**Bước 2 — Từ chối dữ liệu không hợp lệ.**

```cpp

    if (current == nullptr)
    {
        return false;
    }

    // TODO: Chi duoc xoa khi suat chua tung co ve.
    // Chua co TicketRepository de xac minh dieu kien nay.
    return false;
}
```

`nullptr` cho biết không có đối tượng để truy cập. `false` báo thao tác không thành công. Nhánh `false` dừng hàm tại đây; những câu lệnh phía sau không chạy, nên không có cập nhật nào từ các bước sau.

### findShowtimeById()

#### A. Tổng quan

- **Mục đích:** Kiểm tra quyền. Xem các nhánh cụ thể trong code.
- **Tham số:** `const string& showtimeId`.
- **Giá trị trả về:** `const Showtime*`; các nhánh `return` được ghi đầy đủ bên dưới.
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
const Showtime* ShowtimeService::findShowtimeById(const string& showtimeId)
{
    authService.requirePermission("SHOWTIME_VIEW");
    return showtimeRepo.findById(showtimeId);
}
```

`AuthService` kiểm tra quyền và có thể ném ngoại lệ. Quyền `SHOWTIME_VIEW` được kiểm tra trước bước xử lý kế tiếp; thiếu quyền sẽ ném ngoại lệ, vì vậy caller không nên xem đó là kết quả `false`. Lệnh tra cứu trả con trỏ đến đối tượng đang được Repository quản lý. Con trỏ rỗng có nghĩa là không tìm thấy, nên bước sau chỉ được dùng đối tượng khi kiểm tra thành công.

### getShowtimeAt()

#### A. Tổng quan

- **Mục đích:** Kiểm tra quyền. Xem các nhánh cụ thể trong code.
- **Tham số:** `int index`.
- **Giá trị trả về:** `const Showtime*`; các nhánh `return` được ghi đầy đủ bên dưới.
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
const Showtime* ShowtimeService::getShowtimeAt(int index)
{
    authService.requirePermission("SHOWTIME_VIEW");
    return showtimeRepo.getAt(index);
}
```

`AuthService` kiểm tra quyền và có thể ném ngoại lệ. Quyền `SHOWTIME_VIEW` được kiểm tra trước bước xử lý kế tiếp; thiếu quyền sẽ ném ngoại lệ, vì vậy caller không nên xem đó là kết quả `false`.

### getShowtimeCount()

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
int ShowtimeService::getShowtimeCount()
{
    authService.requirePermission("SHOWTIME_VIEW");
    return showtimeRepo.count();
}
```

`AuthService` kiểm tra quyền và có thể ném ngoại lệ. Quyền `SHOWTIME_VIEW` được kiểm tra trước bước xử lý kế tiếp; thiếu quyền sẽ ném ngoại lệ, vì vậy caller không nên xem đó là kết quả `false`.

## 7. Quy tắc dữ liệu và lưu trữ

- Model `Showtime` gồm `showtimeId`, `movieId`, `roomId`, `startAt`, `endAt`, `status`; trạng thái gồm `SCHEDULED` và `CANCELLED`.
- Trạng thái đã kết thúc được suy ra từ thời gian, không có giá trị `FINISHED` trong enum.
- Thời gian kết thúc phải bằng thời gian bắt đầu cộng thời lượng phim tại lúc tạo/sửa; thay đổi thời lượng phim sau đó không tự cập nhật lịch cũ.
- Khoảng nghỉ tối thiểu giữa hai suất cùng phòng là **15 phút** (`DateTime::BREAK_MINUTES`); `ShowtimeRepository` chịu trách nhiệm phát hiện xung đột lịch.
- Khi so lịch, bỏ qua suất `CANCELLED` và suất đang được sửa theo ID.
- Không cho sửa suất đã bắt đầu hoặc đã hủy.
- Các giới hạn liên quan đến lịch sử vé chưa được tích hợp đầy đủ; `cancelShowtime()` và `removeShowtime()` đang chủ động từ chối để tránh thao tác thiếu an toàn.
- `load()`/`save()` là các thao tác ở mức Repository; dữ liệu liên kết phim/phòng phải được nạp đúng thứ tự.

## 8. Ví dụ sử dụng

```cpp
// Gia su userRepo, movieRepo, roomRepo da duoc khoi tao va load.
AuthService authService(userRepo);
ShowtimeRepository showtimeRepo("data/showtimes.txt", movieRepo, roomRepo);
ShowtimeService showtimeService(showtimeRepo, movieRepo, roomRepo, authService);

showtimeService.load();

// Can dang nhap Admin truoc khi them/sua suat.
if (authService.login("admin01", "123456"))
{
    int total = showtimeService.getShowtimeCount();
    // Su dung total de hien thi danh sach suat chieu.
}
```

Đây là **ví dụ minh họa**; đường dẫn file và thông tin đăng nhập phải thay bằng dữ liệu thực tế. Cần kiểm tra kết quả `load()` và bảo đảm Repository phụ thuộc đã được nạp trước.

## 9. Kiểm thử

**File:** `tests/test_showtime_service.cpp`.

Theo kết quả kiểm thử đã ghi nhận trong quá trình phát triển dự án:

| Chỉ số | Kết quả |
| --- | ---: |
| Passed | 89 |
| Failed | 0 |
| Total | 89 |

Các nhóm cần kiểm tra gồm phân quyền, tạo/sửa suất hợp lệ và không hợp lệ, thời gian chiếu, trùng lịch, tra cứu, lưu/nạp và hành vi từ chối hủy/xóa khi chưa có dữ liệu vé.

**Lưu ý:** 89/89 PASS là kết quả đã báo cáo trước đó, không phải lần chạy test mới khi biên soạn tài liệu. Test PASS không có nghĩa hai nghiệp vụ hủy/xóa đã hoàn thiện.

Biên dịch toàn bộ mã nguồn (trừ `main.cpp`) trên PowerShell:

```powershell
$src = Get-ChildItem -Path src -Recurse -Filter *.cpp |
    Where-Object { $_.Name -ne "main.cpp" } |
    ForEach-Object { $_.FullName }

g++ -std=c++17 -Wall -Wextra -I . -I src $src tests/test_showtime_service.cpp -o test_showtime_service.exe

if ($LASTEXITCODE -eq 0) {
    .\test_showtime_service.exe
}
```

## 10. Lưu ý và hướng mở rộng

1. **Tích hợp `TicketRepository`** để kiểm tra lịch sử vé trước khi sửa, hủy hoặc xóa suất.
2. **Hoàn thiện `cancelShowtime()`**: chỉ hủy suất chưa bắt đầu và không còn vé `SOLD`.
3. **Hoàn thiện `removeShowtime()`**: chỉ xóa suất chưa từng phát sinh vé.
4. **Giữ thống nhất phân quyền**: Admin quản lý, Admin/Staff đã đăng nhập được tra cứu; thao tác `load()`/`save()` do luồng khởi động/lưu dữ liệu điều phối.
5. **Đảm bảo thứ tự load**: Movie/Room trước Showtime; User trước đăng nhập; không load lặp một Repository đã nạp.
6. **Không coi `getShowtimeCount()` là số suất còn bán vé**; khi cần danh sách suất khả dụng, phải lọc theo trạng thái và thời gian.

---

*Tài liệu phản ánh triển khai `ShowtimeService.h/.cpp` trên nhánh `main` được đối chiếu khi biên soạn; các TODO liên quan đến vé được ghi rõ thay vì coi là chức năng đã hoàn thành.*
