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

## 6. Chi tiết các hàm

### 6.1. `isValidSchedule(const Showtime& showtime)` — private

**Quy trình:**
1. Tìm phim theo `showtime.getMovieId()`; không có phim thì trả `false`.
2. Kiểm tra `startAt` bằng `Validator::isValidDateTime()`.
3. Yêu cầu `DateTime::getCurrentDateTime()` **nhỏ hơn** `startAt` (suất bắt đầu trong tương lai).
4. Lấy ngày chiếu từ 10 ký tự đầu của `startAt`, yêu cầu thuộc đoạn từ `movie.getReleaseDate()` đến `movie.getEndDate()` (bao gồm hai đầu).
5. Tính `expectedEnd = DateTime::addMinutes(startAt, movie.getDurationMinutes())` và yêu cầu `showtime.getEndAt() == expectedEnd`.

Hàm này không trực tiếp kiểm tra trùng lịch phòng, trạng thái hoạt động của phòng/phim hoặc dữ liệu ID; các kiểm tra còn lại do `ShowtimeRepository` xử lý khi thêm/cập nhật.

### 6.2. `load()`

```cpp
bool ShowtimeService::load()
{
    return showtimeRepo.load();
}
```

Nạp dữ liệu suất chiếu thông qua Repository. Nếu Repository đã được nạp, lần `load()` tiếp theo không được coi là một lần nạp mới thành công theo cơ chế Repository hiện tại.

### 6.3. `save()`

```cpp
bool ShowtimeService::save()
{
    return showtimeRepo.save();
}
```

Ghi dữ liệu suất chiếu từ bộ nhớ xuống file TXT. Các thao tác `add`/`update` không đồng nghĩa với việc đã ghi file thành công; luồng ứng dụng cần gọi `save()` và kiểm tra kết quả.

### 6.4. `addShowtime(Showtime* showtime)`

**Quy trình:**
1. `authService.requireAdmin()`.
2. Từ chối `nullptr`.
3. Chỉ chấp nhận `ShowtimeStatus::SCHEDULED`.
4. Gọi `isValidSchedule(*showtime)`.
5. Gọi `showtimeRepo.add(showtime)` để kiểm tra và thêm dữ liệu.

Trả `true` khi thêm thành công, `false` khi điều kiện không đạt. Nếu thêm thất bại, bên gọi vẫn phải quản lý/giải phóng con trỏ được cấp phát, trừ khi đã chuyển quyền sở hữu thành công cho Repository.

### 6.5. `updateShowtime(const Showtime& newValues)`

**Quy trình:**
1. Yêu cầu quyền Admin.
2. Tìm suất hiện tại theo `newValues.getShowtimeId()`; không thấy thì trả `false`.
3. Chỉ sửa khi **suất hiện tại và dữ liệu mới đều có trạng thái `SCHEDULED`**.
4. Suất hiện tại phải **chưa bắt đầu**.
5. Dữ liệu mới phải vượt qua `isValidSchedule()`.
6. Chuyển cho `showtimeRepo.update(newValues)` để kiểm tra/cập nhật.

**Giới hạn hiện tại:** theo quy tắc nghiệp vụ, suất đã từng phát sinh vé (kể cả vé hủy) không được sửa phim/phòng/thời gian. `ShowtimeService` hiện chưa tự kiểm tra lịch sử vé vì chưa tích hợp `TicketRepository`; cần bổ sung trước khi áp dụng nghiệp vụ bán vé đầy đủ.

### 6.6. `cancelShowtime(const string& showtimeId)`

**Trạng thái: Chưa hoàn thiện.**

Hàm hiện kiểm tra quyền Admin, Repository đã load, suất tồn tại, suất còn `SCHEDULED` và chưa bắt đầu. Sau đó hàm **trả `false`**, không thay đổi trạng thái suất.

```cpp
// TODO: Kiem tra khong con ve SOLD khi co TicketRepository.
// Hien tai chua du thong tin de huy suat an toan.
return false;
```

Theo quy tắc nghiệp vụ, chỉ được hủy suất chưa bắt đầu và không còn vé `SOLD`. Không được tự động hủy vé hàng loạt. Sau khi có `TicketRepository`, cần bổ sung kiểm tra vé trước khi cập nhật trạng thái thành `CANCELLED`.

### 6.7. `removeShowtime(const string& showtimeId)`

**Trạng thái: Chưa hoàn thiện.**

Hàm hiện kiểm tra quyền Admin, Repository đã load và suất tồn tại, rồi **trả `false`**; chưa gọi `showtimeRepo.removeById()`.

```cpp
// TODO: Chi duoc xoa khi suat chua tung co ve.
// Chua co TicketRepository de xac minh dieu kien nay.
return false;
```

Chỉ được xóa vật lý suất chưa từng có vé, kể cả vé đã hủy. Cần kiểm tra lịch sử vé trước khi cho phép xóa.

### 6.8. `findShowtimeById(const string& showtimeId)`

```cpp
const Showtime* ShowtimeService::findShowtimeById(const string& showtimeId)
{
    authService.requireLogin();
    return showtimeRepo.findById(showtimeId);
}
```

Người dùng đăng nhập hợp lệ được tìm suất theo ID. Trả về `nullptr` nếu không tìm thấy; kết quả có thể là suất `SCHEDULED` hoặc `CANCELLED`.

### 6.9. `getShowtimeAt(int index)`

```cpp
const Showtime* ShowtimeService::getShowtimeAt(int index)
{
    authService.requireLogin();
    return showtimeRepo.getAt(index);
}
```

Trả về suất tại vị trí `index` của Repository, không lọc trạng thái. Vị trí không hợp lệ được Repository xử lý bằng `nullptr`.

### 6.10. `getShowtimeCount()`

```cpp
int ShowtimeService::getShowtimeCount()
{
    authService.requireLogin();
    return showtimeRepo.count();
}
```

Đếm **toàn bộ suất chiếu** trong Repository, bao gồm cả suất đã hủy; không chỉ đếm suất sắp chiếu.

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
