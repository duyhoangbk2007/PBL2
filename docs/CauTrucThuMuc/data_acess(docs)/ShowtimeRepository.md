# ShowtimeRepository — mã nguồn và quy tắc dữ liệu

## 1. Tổng quan

`ShowtimeRepository` chứa `Repository<Showtime>` sở hữu các suất, giữ tham chiếu Movie/Room để kiểm tra và nối quan hệ. TXT lưu `showtimeId|movieId|roomId|startAt|endAt|status`; Movie và Room phải nạp trước, và Showtime nên được hủy trước chúng để tháo liên kết.

Suất `SCHEDULED` cùng phòng không được chồng lịch; `DateTime::isOverlapping()` tính cả khoảng nghỉ 15 phút. `load()` kiểm tra dữ liệu và tham chiếu trước khi hoàn tất liên kết; các nhánh lỗi trong code gỡ liên kết hoặc xóa dữ liệu vừa nạp. Repository không phân quyền, không lưu sau từng thay đổi và không có transaction đa file.

Mã nguồn: `src/data_access/ShowtimeRepository.cpp` và `src/data_access/ShowtimeRepository.h`. Các đoạn code dưới đây giữ nguyên nội dung hàm, gồm dòng trống và comment trong hàm. Khi ghép các đoạn của một hàm theo thứ tự, ta thu được đúng định nghĩa gốc.

## 2. Chi tiết từng hàm

### ShowtimeRepository::ShowtimeRepository()

#### A. Tổng quan

- **Mục đích:** Khởi tạo Repository và các tham chiếu/callback cần dùng.
- **Tham số:** `const string& filePath, MovieRepository& movieRepo, RoomRepository& roomRepo`.
- **Giá trị trả về:** không có (constructor/destructor).
- **Điều kiện quan trọng:** Các tham chiếu và callback truyền vào phải còn hợp lệ khi Repository sử dụng.

#### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Chuẩn bị giá trị cho bước kế tiếp
    |
    v
Kết thúc
```

#### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1 — Chuẩn bị giá trị cho bước kế tiếp.**

```cpp
ShowtimeRepository::ShowtimeRepository(const string& filePath, MovieRepository& movieRepo, RoomRepository& roomRepo)
    : repo(filePath, getShowtimeId, stringToShowtime, showtimeToString), movieRepo(movieRepo), roomRepo(roomRepo)
{
}
```

Danh sách sau dấu `:` khởi tạo thành viên trước thân constructor; tham chiếu được giữ để dùng về sau, còn thân hàm không nạp file.

### ShowtimeRepository::~ShowtimeRepository()

#### A. Tổng quan

- **Mục đích:** Tháo liên kết hoặc giải phóng tài nguyên khi hủy Repository.
- **Tham số:** không có.
- **Giá trị trả về:** không có (constructor/destructor).
- **Điều kiện quan trọng:** Các Repository được tham chiếu cần còn sống đến khi destructor tháo liên kết.

#### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Tháo liên kết đối tượng
    |
    v
Tháo liên kết đối tượng
    |
    v
Kết thúc
```

#### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1 — Tháo liên kết đối tượng.**

```cpp
ShowtimeRepository::~ShowtimeRepository()
{
    for (int i = 0; i < repo.count(); i++)
    {
        Showtime* showtime = repo.getAt(i);
        if (showtime->getMovie() != nullptr)
        {
            showtime->getMovie()->unlinkShowtime(showtime);
```

Liên kết được đổi theo thứ tự trong code; nếu bước sau thất bại, phần đã nối phải được tháo trước khi xóa đối tượng để tránh con trỏ treo. Vòng lặp xét từng bản ghi; điều kiện bên trong quyết định tiếp tục, bỏ qua hay dừng khi gặp dữ liệu sai.

**Bước 2 — Tháo liên kết đối tượng.**

```cpp
        }
        if (showtime->getRoom() != nullptr)
        {
            showtime->getRoom()->unlinkShowtime(showtime);
        }
    }
}
```

Liên kết được đổi theo thứ tự trong code; nếu bước sau thất bại, phần đã nối phải được tháo trước khi xóa đối tượng để tránh con trỏ treo.

### ShowtimeRepository::getShowtimeId()

#### A. Tổng quan

- **Mục đích:** Lấy ID của đối tượng làm khóa.
- **Tham số:** `Showtime* showtime`.
- **Giá trị trả về:** `string`; xem các nhánh `return` bên dưới.
- **Điều kiện quan trọng:** con trỏ rỗng.

#### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Trả kết quả cho bên gọi
    |
    v
Kết thúc
```

#### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1 — Trả kết quả cho bên gọi.**

```cpp
string ShowtimeRepository::getShowtimeId(Showtime* showtime)
{
    if (showtime == nullptr)
    {
        return "";
    }
    return showtime->getShowtimeId();
}
```

Đoạn này chuẩn bị hoặc trả giá trị theo code. Nó chỉ đổi RAM hoặc TXT nếu có lời gọi cập nhật hay ghi dữ liệu tương ứng.

### ShowtimeRepository::isValidShowtime()

#### A. Tổng quan

- **Mục đích:** Kiểm tra tính hợp lệ của dữ liệu đầu vào.
- **Tham số:** `const Showtime* showtime`.
- **Giá trị trả về:** `bool`; xem các nhánh `return` bên dưới.
- **Điều kiện quan trọng:** con trỏ rỗng, dữ liệu hợp lệ, nhánh trả `false`.

#### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Kiểm tra dữ liệu hợp lệ (không đạt: false)
    |
    v
Kiểm tra dữ liệu hợp lệ (không đạt: false)
    |
    v
Từ chối điều kiện không đạt (không đạt: false)
    |
    v
Xác nhận xử lý thành công
    |
    v
Kết thúc
```

#### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1 — Kiểm tra dữ liệu hợp lệ.**

```cpp
bool ShowtimeRepository::isValidShowtime(const Showtime* showtime)
{
    if (showtime == nullptr)
    {
        return false;
    }
    if (!Validator::isValidId(showtime->getShowtimeId(), "SHW"))
    {
        return false;
```

Điều kiện hợp lệ ngăn dữ liệu sai định dạng hoặc ngoài giới hạn đi vào danh sách đang quản lý. `return false` kết thúc ngay nhánh không đạt; các thao tác cập nhật phía sau không chạy.

**Bước 2 — Kiểm tra dữ liệu hợp lệ.**

```cpp
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
```

Điều kiện hợp lệ ngăn dữ liệu sai định dạng hoặc ngoài giới hạn đi vào danh sách đang quản lý. Cặp `roomId` và `seatId` xác định đúng ghế trong phòng; chỉ dùng `seatId` sẽ không đủ để phân biệt ghế ở các phòng khác nhau. `return false` kết thúc ngay nhánh không đạt; các thao tác cập nhật phía sau không chạy.

**Bước 3 — Từ chối điều kiện không đạt.**

```cpp
    }
    if (!DateTime::isBefore(showtime->getStartAt(), showtime->getEndAt()))
    {
        return false;
    }
    if (showtime->getStatus() != ShowtimeStatus::SCHEDULED &&
        showtime->getStatus() != ShowtimeStatus::CANCELLED)
    {
        return false;
```

`return false` kết thúc ngay nhánh không đạt; các thao tác cập nhật phía sau không chạy.

**Bước 4 — Xác nhận xử lý thành công.**

```cpp
    }
    return true;
}
```

Đoạn này chuẩn bị hoặc trả giá trị theo code. Nó chỉ đổi RAM hoặc TXT nếu có lời gọi cập nhật hay ghi dữ liệu tương ứng.

### ShowtimeRepository::isValidReference()

#### A. Tổng quan

- **Mục đích:** Kiểm tra các ID tham chiếu còn tồn tại.
- **Tham số:** `const Showtime* showtime`.
- **Giá trị trả về:** `bool`; xem các nhánh `return` bên dưới.
- **Điều kiện quan trọng:** trạng thái nạp, con trỏ rỗng, dữ liệu hợp lệ, nhánh trả `false`.

#### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Kiểm tra dữ liệu hợp lệ (không đạt: false)
    |
    v
Tra cứu đối tượng theo khóa (không đạt: false)
    |
    v
Xác nhận xử lý thành công
    |
    v
Kết thúc
```

#### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1 — Kiểm tra dữ liệu hợp lệ.**

```cpp
bool ShowtimeRepository::isValidReference(const Showtime* showtime)
{
    if (!isValidShowtime(showtime))
    {
        return false;
    }
    if (!movieRepo.isLoaded() || !roomRepo.isLoaded())
    {
        return false;
```

Trạng thái `loaded` quyết định Repository đã sẵn sàng chưa; nhánh chưa nạp dừng trước khi thao tác dựa trên dữ liệu đó. Điều kiện hợp lệ ngăn dữ liệu sai định dạng hoặc ngoài giới hạn đi vào danh sách đang quản lý. `return false` kết thúc ngay nhánh không đạt; các thao tác cập nhật phía sau không chạy.

**Bước 2 — Tra cứu đối tượng theo khóa.**

```cpp
    }
    Movie* movie = movieRepo.findById(showtime->getMovieId());
    Room* room = roomRepo.findById(showtime->getRoomId());
    if (movie == nullptr || room == nullptr)
    {
        return false;
    }
    if (showtime->getStatus() == ShowtimeStatus::SCHEDULED)
    {
        if (!movie->isActive() || !room->isActive())
        {
            return false;
```

Tra Room bằng `roomId` để bảo đảm đối tượng liên kết còn tồn tại; `nullptr` nghĩa là tham chiếu sai và không thể nối Seat/Showtime vào phòng. Tra Movie theo ID trước khi nối suất chiếu; con trỏ rỗng là tham chiếu không hợp lệ. Cặp `roomId` và `seatId` xác định đúng ghế trong phòng; chỉ dùng `seatId` sẽ không đủ để phân biệt ghế ở các phòng khác nhau.

**Bước 3 — Xác nhận xử lý thành công.**

```cpp
        }
    }
    return true;
}
```

Đoạn này chuẩn bị hoặc trả giá trị theo code. Nó chỉ đổi RAM hoặc TXT nếu có lời gọi cập nhật hay ghi dữ liệu tương ứng.

### ShowtimeRepository::hasScheduleConflict()

#### A. Tổng quan

- **Mục đích:** Kiểm tra lịch chiếu chồng nhau.
- **Tham số:** `const Showtime* showtime, const string& excludeId`.
- **Giá trị trả về:** `bool`; xem các nhánh `return` bên dưới.
- **Điều kiện quan trọng:** con trỏ rỗng, nhánh trả `false`.

#### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Từ chối điều kiện không đạt (không đạt: false)
    |
    v
Duyệt và đối chiếu từng phần tử
    |
    v
Kiểm tra điều kiện trước khi tiếp tục
    |
    v
Xác nhận xử lý thành công
    |
    v
Từ chối điều kiện không đạt (không đạt: false)
    |
    v
Kết thúc
```

#### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1 — Từ chối điều kiện không đạt.**

```cpp
bool ShowtimeRepository::hasScheduleConflict(const Showtime* showtime, const string& excludeId)
{ 
    if (showtime == nullptr)
    {
        return false;
    }
    if (showtime->getStatus() == ShowtimeStatus::CANCELLED)
    {
        return false;
```

Kiểm tra trùng lịch giữ khoảng thời gian của các suất cùng phòng không chồng nhau theo quy tắc nghỉ giữa suất. `return false` kết thúc ngay nhánh không đạt; các thao tác cập nhật phía sau không chạy.

**Bước 2 — Duyệt và đối chiếu từng phần tử.**

```cpp
    }
    for (int i = 0; i < repo.count(); i++)
    {
        Showtime* current = repo.getAt(i);
        if (current == nullptr)
        {
            continue;
        }
```

Vòng lặp xét từng bản ghi; điều kiện bên trong quyết định tiếp tục, bỏ qua hay dừng khi gặp dữ liệu sai.

**Bước 3 — Kiểm tra điều kiện trước khi tiếp tục.**

```cpp
        // Bo qua chinh suat dang cap nhat
        if (current->getShowtimeId() == excludeId) // excludeId là Id của suất chiếu đang cập nhật, cần bỏ qua khi update suất chiếu
        {
            continue;
        }
        // Chi kiem tra cac suat cung phong
        if (current->getRoomId() != showtime->getRoomId())
        {
            continue;
```

Cặp `roomId` và `seatId` xác định đúng ghế trong phòng; chỉ dùng `seatId` sẽ không đủ để phân biệt ghế ở các phòng khác nhau.

**Bước 4 — Xác nhận xử lý thành công.**

```cpp
        }
        // Bo qua suat da huy
        if (current->getStatus() == ShowtimeStatus::CANCELLED)
        {
            continue;
        }

        if (DateTime::isOverlapping(
                current->getStartAt(),
                current->getEndAt(),
                showtime->getStartAt(),
                showtime->getEndAt()))
        {
            return true;
```

Kiểm tra trùng lịch giữ khoảng thời gian của các suất cùng phòng không chồng nhau theo quy tắc nghỉ giữa suất.

**Bước 5 — Từ chối điều kiện không đạt.**

```cpp
        }
    }
    return false;
}
```

`return false` kết thúc ngay nhánh không đạt; các thao tác cập nhật phía sau không chạy.

### ShowtimeRepository::stringToShowtime()

#### A. Tổng quan

- **Mục đích:** Chuyển một dòng TXT thành đối tượng Model.
- **Tham số:** `const string& line`.
- **Giá trị trả về:** `Showtime*`; xem các nhánh `return` bên dưới.
- **Điều kiện quan trọng:** dữ liệu hợp lệ, ngoại lệ.

#### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Duyệt và đối chiếu từng phần tử
    |
    v
Kiểm tra điều kiện trước khi tiếp tục (lỗi: throw)
    |
    v
Kiểm tra điều kiện trước khi tiếp tục
    |
    v
Chuẩn bị giá trị cho bước kế tiếp (lỗi: throw)
    |
    v
Kiểm tra dữ liệu hợp lệ (lỗi: throw)
    |
    v
Kết thúc
```

#### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1 — Duyệt và đối chiếu từng phần tử.**

```cpp
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
```

Vòng lặp xét từng bản ghi; điều kiện bên trong quyết định tiếp tục, bỏ qua hay dừng khi gặp dữ liệu sai.

**Bước 2 — Kiểm tra điều kiện trước khi tiếp tục.**

```cpp
            break;
        }
        fields.push_back(line.substr(start, pos - start));
        start = pos + 1;
    }
    if (fields.size() != 6)
    {
        throw runtime_error("Sai so luong truong Showtime");
```

`throw` báo lỗi bằng ngoại lệ cho bên gọi, khác với kết quả `false` thông thường.

**Bước 3 — Kiểm tra điều kiện trước khi tiếp tục.**

```cpp
    }
    ShowtimeStatus status;
    if (fields[5] == "SCHEDULED")
    {
        status = ShowtimeStatus::SCHEDULED;
    }
    else if (fields[5] == "CANCELLED")
    {
        status = ShowtimeStatus::CANCELLED;
```

Đoạn này chuẩn bị hoặc trả giá trị theo code. Nó chỉ đổi RAM hoặc TXT nếu có lời gọi cập nhật hay ghi dữ liệu tương ứng.

**Bước 4 — Chuẩn bị giá trị cho bước kế tiếp.**

```cpp
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
```

`new` cấp phát Model trên heap. Cho đến khi Repository nhận thành công, nhánh lỗi hoặc caller vẫn phải chịu trách nhiệm giải phóng. `throw` báo lỗi bằng ngoại lệ cho bên gọi, khác với kết quả `false` thông thường.

**Bước 5 — Kiểm tra dữ liệu hợp lệ.**

```cpp

    if (!isValidShowtime(showtime))
    {
        delete showtime;
        throw runtime_error("Du lieu Showtime khong hop le");
    }
    return showtime;
}
```

Điều kiện hợp lệ ngăn dữ liệu sai định dạng hoặc ngoài giới hạn đi vào danh sách đang quản lý. `delete` giải phóng đối tượng; không được dùng lại con trỏ sau lệnh này. `throw` báo lỗi bằng ngoại lệ cho bên gọi, khác với kết quả `false` thông thường.

### ShowtimeRepository::showtimeToString()

#### A. Tổng quan

- **Mục đích:** Chuyển đối tượng Model thành một dòng TXT.
- **Tham số:** `Showtime* showtime`.
- **Giá trị trả về:** `string`; xem các nhánh `return` bên dưới.
- **Điều kiện quan trọng:** dữ liệu hợp lệ, ngoại lệ.

#### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Kiểm tra dữ liệu hợp lệ (lỗi: throw)
    |
    v
Trả kết quả cho bên gọi
    |
    v
Chuẩn bị giá trị cho bước kế tiếp
    |
    v
Kết thúc
```

#### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1 — Kiểm tra dữ liệu hợp lệ.**

```cpp
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
```

Điều kiện hợp lệ ngăn dữ liệu sai định dạng hoặc ngoài giới hạn đi vào danh sách đang quản lý. `throw` báo lỗi bằng ngoại lệ cho bên gọi, khác với kết quả `false` thông thường.

**Bước 2 — Trả kết quả cho bên gọi.**

```cpp
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
```

Cặp `roomId` và `seatId` xác định đúng ghế trong phòng; chỉ dùng `seatId` sẽ không đủ để phân biệt ghế ở các phòng khác nhau.

**Bước 3 — Chuẩn bị giá trị cho bước kế tiếp.**

```cpp
}
```

Đoạn này chuẩn bị hoặc trả giá trị theo code. Nó chỉ đổi RAM hoặc TXT nếu có lời gọi cập nhật hay ghi dữ liệu tương ứng.

### ShowtimeRepository::load()

#### A. Tổng quan

- **Mục đích:** Nạp và kiểm tra dữ liệu từ TXT vào RAM.
- **Tham số:** không có.
- **Giá trị trả về:** `bool`; xem các nhánh `return` bên dưới.
- **Điều kiện quan trọng:** trạng thái nạp, con trỏ rỗng, ngoại lệ, nhánh trả `false`.

#### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Từ chối điều kiện không đạt (không đạt: false)
    |
    v
Đọc dữ liệu TXT (không đạt: false)
    |
    v
Tra cứu đối tượng theo khóa (lỗi: throw)
    |
    v
Duyệt và đối chiếu từng phần tử
    |
    v
Nối quan hệ giữa các Model (lỗi: throw)
    |
    v
Bắt lỗi và khôi phục trạng thái
    |
    v
Xóa dữ liệu nạp dở khi kiểm tra thất bại
    |
    v
Xác nhận xử lý thành công
    |
    v
Kết thúc
```

#### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1 — Từ chối điều kiện không đạt.**

```cpp
bool ShowtimeRepository::load()
{
    if (repo.isLoaded())
    {
        return false;
    }
    if (!movieRepo.isLoaded() || !roomRepo.isLoaded())
    {
        return false;
```

Trạng thái `loaded` quyết định Repository đã sẵn sàng chưa; nhánh chưa nạp dừng trước khi thao tác dựa trên dữ liệu đó. `return false` kết thúc ngay nhánh không đạt; các thao tác cập nhật phía sau không chạy.

**Bước 2 — Đọc dữ liệu TXT.**

```cpp
    }
    // 1. Doc du lieu Showtime tu file
    if (!repo.load())
    {
        return false;
    }
    // 2. Kiem tra tham chieu va trung lich
    try
    {
        for (int i = 0; i < repo.count(); i++)
        {
            Showtime* showtime = repo.getAt(i);
```

Lời gọi nạp đọc TXT vào bộ nhớ. Nếu trả `false`, hàm dừng ở nhánh lỗi; dữ liệu chỉ được xem là dùng được sau khi các bước kiểm tra tiếp theo hoàn tất. `try/catch` xử lý ngoại lệ trong phạm vi hiện tại; code dọn các thay đổi trước khi trả lỗi hoặc ném lại, nên thứ tự dọn có ý nghĩa. `return false` kết thúc ngay nhánh không đạt; các thao tác cập nhật phía sau không chạy.

**Bước 3 — Tra cứu đối tượng theo khóa.**

```cpp
            Movie* movie = movieRepo.findById(showtime->getMovieId());
            Room* room = roomRepo.findById(showtime->getRoomId());

            if (movie == nullptr || room == nullptr)
            {
                throw runtime_error("Showtime tham chieu Movie hoac Room khong ton tai");
            }
            // Kiem tra trung lich voi cac suat truoc do
            for (int j = 0; j < i; j++)
            {
                Showtime* other = repo.getAt(j);
                if (showtime->getStatus() == ShowtimeStatus::CANCELLED ||
                    other->getStatus() == ShowtimeStatus::CANCELLED)
                {
                    continue;
                }
                if (showtime->getRoomId() == other->getRoomId() &&
                    DateTime::isOverlapping(
                        showtime->getStartAt(),
                        showtime->getEndAt(),
                        other->getStartAt(),
                        other->getEndAt()))
                {
                    throw runtime_error("Trung lich Showtime trong file");
```

Tra Room bằng `roomId` để bảo đảm đối tượng liên kết còn tồn tại; `nullptr` nghĩa là tham chiếu sai và không thể nối Seat/Showtime vào phòng. Tra Movie theo ID trước khi nối suất chiếu; con trỏ rỗng là tham chiếu không hợp lệ. Cặp `roomId` và `seatId` xác định đúng ghế trong phòng; chỉ dùng `seatId` sẽ không đủ để phân biệt ghế ở các phòng khác nhau.

**Bước 4 — Duyệt và đối chiếu từng phần tử.**

```cpp
                }
            }
        }

        // Lien ket Showtime voi Movie va Room
        for (int i = 0; i < repo.count(); i++)
        {
            Showtime* showtime = repo.getAt(i);
```

Vòng lặp xét từng bản ghi; điều kiện bên trong quyết định tiếp tục, bỏ qua hay dừng khi gặp dữ liệu sai.

**Bước 5 — Nối quan hệ giữa các Model.**

```cpp
            Movie* movie = movieRepo.findById(showtime->getMovieId());
            Room* room = roomRepo.findById(showtime->getRoomId());
            if (!showtime->linkMovie(movie) || !showtime->linkRoom(room))
            {
                throw runtime_error("Khong the lien ket Showtime voi Movie va Room");
            }
        }
    }
```

Tra Room bằng `roomId` để bảo đảm đối tượng liên kết còn tồn tại; `nullptr` nghĩa là tham chiếu sai và không thể nối Seat/Showtime vào phòng. Tra Movie theo ID trước khi nối suất chiếu; con trỏ rỗng là tham chiếu không hợp lệ. Cặp `roomId` và `seatId` xác định đúng ghế trong phòng; chỉ dùng `seatId` sẽ không đủ để phân biệt ghế ở các phòng khác nhau.

**Bước 6 — Bắt lỗi và khôi phục trạng thái.**

```cpp
    catch (...)
    {
        // Hoan tac cac lien ket da tao
        for (int i = 0; i < repo.count(); i++)
        {
            Showtime* showtime = repo.getAt(i);
            if (showtime->getMovie() != nullptr)
            {
                showtime->getMovie()->unlinkShowtime(showtime);
            }
```

Liên kết được đổi theo thứ tự trong code; nếu bước sau thất bại, phần đã nối phải được tháo trước khi xóa đối tượng để tránh con trỏ treo. `try/catch` xử lý ngoại lệ trong phạm vi hiện tại; code dọn các thay đổi trước khi trả lỗi hoặc ném lại, nên thứ tự dọn có ý nghĩa. Vòng lặp xét từng bản ghi; điều kiện bên trong quyết định tiếp tục, bỏ qua hay dừng khi gặp dữ liệu sai.

**Bước 7 — Xóa dữ liệu nạp dở khi kiểm tra thất bại.**

```cpp
            if (showtime->getRoom() != nullptr)
            {
                showtime->getRoom()->unlinkShowtime(showtime);
            }
        }

        repo.clear();
        throw;
```

`repo.clear()` giải phóng các đối tượng vừa nạp và đặt lại trạng thái; vì dữ liệu chưa qua hết kiểm tra, không được để chúng tiếp tục xuất hiện như bản ghi hợp lệ. Liên kết được đổi theo thứ tự trong code; nếu bước sau thất bại, phần đã nối phải được tháo trước khi xóa đối tượng để tránh con trỏ treo.

**Bước 8 — Xác nhận xử lý thành công.**

```cpp
    }

    return true;
}
```

Đoạn này chuẩn bị hoặc trả giá trị theo code. Nó chỉ đổi RAM hoặc TXT nếu có lời gọi cập nhật hay ghi dữ liệu tương ứng.

### ShowtimeRepository::save()

#### A. Tổng quan

- **Mục đích:** Ghi trạng thái hiện tại ra TXT.
- **Tham số:** không có.
- **Giá trị trả về:** `bool`; xem các nhánh `return` bên dưới.
- **Điều kiện quan trọng:** không có điều kiện đặc biệt ngoài các câu lệnh bên dưới.

#### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Ghi dữ liệu TXT
    |
    v
Kết thúc
```

#### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1 — Ghi dữ liệu TXT.**

```cpp
bool ShowtimeRepository::save()
{
    return repo.save();
}
```

Lời gọi ghi chuyển trạng thái hiện tại ra TXT. Kết quả ghi quyết định giá trị trả về; một lời gọi này không bảo đảm các file Repository khác cùng được cập nhật.

### ShowtimeRepository::findById()

#### A. Tổng quan

- **Mục đích:** Tra đối tượng theo ID.
- **Tham số:** `const string& showtimeId`.
- **Giá trị trả về:** `Showtime*`; xem các nhánh `return` bên dưới.
- **Điều kiện quan trọng:** không có điều kiện đặc biệt ngoài các câu lệnh bên dưới.

#### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Tra cứu đối tượng theo khóa
    |
    v
Kết thúc
```

#### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1 — Tra cứu đối tượng theo khóa.**

```cpp
Showtime* ShowtimeRepository::findById(const string& showtimeId)
{
    return repo.findById(showtimeId);
}
```

Đoạn này chuẩn bị hoặc trả giá trị theo code. Nó chỉ đổi RAM hoặc TXT nếu có lời gọi cập nhật hay ghi dữ liệu tương ứng.

### ShowtimeRepository::getAt()

#### A. Tổng quan

- **Mục đích:** Lấy phần tử theo chỉ số.
- **Tham số:** `int index`.
- **Giá trị trả về:** `Showtime*`; xem các nhánh `return` bên dưới.
- **Điều kiện quan trọng:** không có điều kiện đặc biệt ngoài các câu lệnh bên dưới.

#### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Trả kết quả cho bên gọi
    |
    v
Kết thúc
```

#### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1 — Trả kết quả cho bên gọi.**

```cpp
Showtime* ShowtimeRepository::getAt(int index)
{
    return repo.getAt(index);
}
```

Đoạn này chuẩn bị hoặc trả giá trị theo code. Nó chỉ đổi RAM hoặc TXT nếu có lời gọi cập nhật hay ghi dữ liệu tương ứng.

### ShowtimeRepository::count()

#### A. Tổng quan

- **Mục đích:** Trả số phần tử đang quản lý.
- **Tham số:** không có.
- **Giá trị trả về:** `int`; xem các nhánh `return` bên dưới.
- **Điều kiện quan trọng:** không có điều kiện đặc biệt ngoài các câu lệnh bên dưới.

#### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Trả kết quả cho bên gọi
    |
    v
Kết thúc
```

#### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1 — Trả kết quả cho bên gọi.**

```cpp
int ShowtimeRepository::count()
{
    return repo.count();
}
```

Đoạn này chuẩn bị hoặc trả giá trị theo code. Nó chỉ đổi RAM hoặc TXT nếu có lời gọi cập nhật hay ghi dữ liệu tương ứng.

### ShowtimeRepository::isLoaded()

#### A. Tổng quan

- **Mục đích:** Trả trạng thái đã nạp.
- **Tham số:** không có.
- **Giá trị trả về:** `bool`; xem các nhánh `return` bên dưới.
- **Điều kiện quan trọng:** trạng thái nạp.

#### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Trả kết quả cho bên gọi
    |
    v
Kết thúc
```

#### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1 — Trả kết quả cho bên gọi.**

```cpp
bool ShowtimeRepository::isLoaded()
{
    return repo.isLoaded();
}
```

Trạng thái `loaded` quyết định Repository đã sẵn sàng chưa; nhánh chưa nạp dừng trước khi thao tác dựa trên dữ liệu đó.

### ShowtimeRepository::add()

#### A. Tổng quan

- **Mục đích:** Thêm đối tượng hợp lệ vào Repository.
- **Tham số:** `Showtime* showtime`.
- **Giá trị trả về:** `bool`; xem các nhánh `return` bên dưới.
- **Điều kiện quan trọng:** trạng thái nạp, con trỏ rỗng, dữ liệu hợp lệ, nhánh trả `false`.

#### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Kiểm tra dữ liệu hợp lệ (không đạt: false)
    |
    v
Tra cứu đối tượng theo khóa (không đạt: false)
    |
    v
Kiểm tra dữ liệu hợp lệ (không đạt: false)
    |
    v
Nối quan hệ giữa các Model (không đạt: false)
    |
    v
Tháo liên kết đối tượng (không đạt: false)
    |
    v
Bắt lỗi và khôi phục trạng thái (không đạt: false)
    |
    v
Tháo liên kết đối tượng
    |
    v
Chuẩn bị giá trị cho bước kế tiếp
    |
    v
Kết thúc
```

#### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1 — Kiểm tra dữ liệu hợp lệ.**

```cpp
bool ShowtimeRepository::add(Showtime* showtime)
{
    if (!repo.isLoaded())
    {
        return false;
    }
    if (!isValidShowtime(showtime))
    {
        return false;
```

Trạng thái `loaded` quyết định Repository đã sẵn sàng chưa; nhánh chưa nạp dừng trước khi thao tác dựa trên dữ liệu đó. Điều kiện hợp lệ ngăn dữ liệu sai định dạng hoặc ngoài giới hạn đi vào danh sách đang quản lý. `return false` kết thúc ngay nhánh không đạt; các thao tác cập nhật phía sau không chạy.

**Bước 2 — Tra cứu đối tượng theo khóa.**

```cpp
    }
    // không thêm Showtime đã được liên kết từ trước
    if (showtime->getMovie() != nullptr || showtime->getRoom() != nullptr)
    {
        return false;
    }
    // kiểm tra ID trùng
    if (repo.findById(showtime->getShowtimeId()) != nullptr)
    {
        return false;
```

`return false` kết thúc ngay nhánh không đạt; các thao tác cập nhật phía sau không chạy.

**Bước 3 — Kiểm tra dữ liệu hợp lệ.**

```cpp
    }
    // 4. Kiem tra Movie va Room
    if (!isValidReference(showtime))
    {
        return false;
    }
    // 5. Kiem tra trung lich
    if (hasScheduleConflict(showtime))
    {
        return false;
```

Điều kiện hợp lệ ngăn dữ liệu sai định dạng hoặc ngoài giới hạn đi vào danh sách đang quản lý. Kiểm tra trùng lịch giữ khoảng thời gian của các suất cùng phòng không chồng nhau theo quy tắc nghỉ giữa suất. `return false` kết thúc ngay nhánh không đạt; các thao tác cập nhật phía sau không chạy.

**Bước 4 — Nối quan hệ giữa các Model.**

```cpp
    }
    Movie* movie = movieRepo.findById(showtime->getMovieId());
    Room* room = roomRepo.findById(showtime->getRoomId());
    try
    {
        if (!showtime->linkMovie(movie))
        {
            return false;
```

Tra Room bằng `roomId` để bảo đảm đối tượng liên kết còn tồn tại; `nullptr` nghĩa là tham chiếu sai và không thể nối Seat/Showtime vào phòng. Tra Movie theo ID trước khi nối suất chiếu; con trỏ rỗng là tham chiếu không hợp lệ. Cặp `roomId` và `seatId` xác định đúng ghế trong phòng; chỉ dùng `seatId` sẽ không đủ để phân biệt ghế ở các phòng khác nhau.

**Bước 5 — Tháo liên kết đối tượng.**

```cpp
        }
        if (!showtime->linkRoom(room))
        {
            movie->unlinkShowtime(showtime);
            return false;
        }
        if (!repo.add(showtime))
        {
            room->unlinkShowtime(showtime);
```

Liên kết được đổi theo thứ tự trong code; nếu bước sau thất bại, phần đã nối phải được tháo trước khi xóa đối tượng để tránh con trỏ treo. `return false` kết thúc ngay nhánh không đạt; các thao tác cập nhật phía sau không chạy.

**Bước 6 — Bắt lỗi và khôi phục trạng thái.**

```cpp
            movie->unlinkShowtime(showtime);
            return false;
        }
    }
    catch (...)
    {
        if (showtime->getRoom() != nullptr)
        {
            room->unlinkShowtime(showtime);
```

Liên kết được đổi theo thứ tự trong code; nếu bước sau thất bại, phần đã nối phải được tháo trước khi xóa đối tượng để tránh con trỏ treo. `try/catch` xử lý ngoại lệ trong phạm vi hiện tại; code dọn các thay đổi trước khi trả lỗi hoặc ném lại, nên thứ tự dọn có ý nghĩa. `return false` kết thúc ngay nhánh không đạt; các thao tác cập nhật phía sau không chạy.

**Bước 7 — Tháo liên kết đối tượng.**

```cpp
        }
        if (showtime->getMovie() != nullptr)
        {
            movie->unlinkShowtime(showtime);
        }
        throw;
    }
    return true;
```

Liên kết được đổi theo thứ tự trong code; nếu bước sau thất bại, phần đã nối phải được tháo trước khi xóa đối tượng để tránh con trỏ treo.

**Bước 8 — Chuẩn bị giá trị cho bước kế tiếp.**

```cpp

}
```

Đoạn này chuẩn bị hoặc trả giá trị theo code. Nó chỉ đổi RAM hoặc TXT nếu có lời gọi cập nhật hay ghi dữ liệu tương ứng.

### ShowtimeRepository::update()

#### A. Tổng quan

- **Mục đích:** Cập nhật đối tượng đang quản lý.
- **Tham số:** `const Showtime& newValues`.
- **Giá trị trả về:** `bool`; xem các nhánh `return` bên dưới.
- **Điều kiện quan trọng:** trạng thái nạp, con trỏ rỗng, dữ liệu hợp lệ, nhánh trả `false`.

#### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Tra cứu đối tượng theo khóa (không đạt: false)
    |
    v
Kiểm tra dữ liệu hợp lệ (không đạt: false)
    |
    v
Kiểm tra dữ liệu hợp lệ (không đạt: false)
    |
    v
Xác nhận xử lý thành công
    |
    v
Kết thúc
```

#### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1 — Tra cứu đối tượng theo khóa.**

```cpp
bool ShowtimeRepository::update(const Showtime& newValues)
{
    if (!repo.isLoaded())
    {
        return false;
    }
    Showtime* current = repo.findById(newValues.getShowtimeId());
    if (current == nullptr)
    {
        return false;
```

Trạng thái `loaded` quyết định Repository đã sẵn sàng chưa; nhánh chưa nạp dừng trước khi thao tác dựa trên dữ liệu đó. `return false` kết thúc ngay nhánh không đạt; các thao tác cập nhật phía sau không chạy.

**Bước 2 — Kiểm tra dữ liệu hợp lệ.**

```cpp
    }
    // 3. Khong cho doi Movie va Room
    if (current->getMovieId() != newValues.getMovieId() ||
        current->getRoomId() != newValues.getRoomId())
    {
        return false;
    }
    // 4. Kiem tra du lieu moi
    if (!isValidShowtime(&newValues))
    {
        return false;
```

Điều kiện hợp lệ ngăn dữ liệu sai định dạng hoặc ngoài giới hạn đi vào danh sách đang quản lý. Cặp `roomId` và `seatId` xác định đúng ghế trong phòng; chỉ dùng `seatId` sẽ không đủ để phân biệt ghế ở các phòng khác nhau. `return false` kết thúc ngay nhánh không đạt; các thao tác cập nhật phía sau không chạy.

**Bước 3 — Kiểm tra dữ liệu hợp lệ.**

```cpp
    }
    if (!isValidReference(&newValues))
    {
        return false;
    }
    // 5. Kiem tra trung lich
    if (hasScheduleConflict(&newValues, current->getShowtimeId()))
    {
        return false;
```

Điều kiện hợp lệ ngăn dữ liệu sai định dạng hoặc ngoài giới hạn đi vào danh sách đang quản lý. Kiểm tra trùng lịch giữ khoảng thời gian của các suất cùng phòng không chồng nhau theo quy tắc nghỉ giữa suất. `return false` kết thúc ngay nhánh không đạt; các thao tác cập nhật phía sau không chạy.

**Bước 4 — Xác nhận xử lý thành công.**

```cpp
    }
    current->setStartAt(newValues.getStartAt());
    current->setEndAt(newValues.getEndAt());
    current->setStatus(newValues.getStatus());
    return true;
}
```

Đoạn này chuẩn bị hoặc trả giá trị theo code. Nó chỉ đổi RAM hoặc TXT nếu có lời gọi cập nhật hay ghi dữ liệu tương ứng.

### ShowtimeRepository::removeById()

#### A. Tổng quan

- **Mục đích:** Xóa đối tượng theo khóa sau khi kiểm tra ràng buộc.
- **Tham số:** `const string& showtimeId`.
- **Giá trị trả về:** `bool`; xem các nhánh `return` bên dưới.
- **Điều kiện quan trọng:** trạng thái nạp, con trỏ rỗng, ngoại lệ, nhánh trả `false`.

#### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Tra cứu đối tượng theo khóa (không đạt: false)
    |
    v
Tháo liên kết đối tượng (không đạt: false)
    |
    v
Tháo liên kết đối tượng (lỗi: throw)
    |
    v
Nối quan hệ giữa các Model (lỗi: throw)
    |
    v
Từ chối điều kiện không đạt (không đạt: false)
    |
    v
Kết thúc
```

#### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1 — Tra cứu đối tượng theo khóa.**

```cpp
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
```

Trạng thái `loaded` quyết định Repository đã sẵn sàng chưa; nhánh chưa nạp dừng trước khi thao tác dựa trên dữ liệu đó. `return false` kết thúc ngay nhánh không đạt; các thao tác cập nhật phía sau không chạy.

**Bước 2 — Tháo liên kết đối tượng.**

```cpp
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
```

Liên kết được đổi theo thứ tự trong code; nếu bước sau thất bại, phần đã nối phải được tháo trước khi xóa đối tượng để tránh con trỏ treo. `return false` kết thúc ngay nhánh không đạt; các thao tác cập nhật phía sau không chạy.

**Bước 3 — Tháo liên kết đối tượng.**

```cpp
    }
    //nếu k hủy liên kết showtime với room được thì phải khôi phục lại liên kết showtime với movie
    if (!room->unlinkShowtime(showtime))
    {
        if (!movie->linkShowtime(showtime))
        {
            throw runtime_error(
                "Khong the khoi phuc lien ket Movie"
            );
```

Liên kết được đổi theo thứ tự trong code; nếu bước sau thất bại, phần đã nối phải được tháo trước khi xóa đối tượng để tránh con trỏ treo. `throw` báo lỗi bằng ngoại lệ cho bên gọi, khác với kết quả `false` thông thường.

**Bước 4 — Nối quan hệ giữa các Model.**

```cpp
        }
        return false;
    }
    if (!repo.removeById(showtimeId))
    {
        if (!movie->linkShowtime(showtime) || !room->linkShowtime(showtime))
        {
            throw runtime_error("Khong the khoi phuc lien ket Showtime");
```

Liên kết được đổi theo thứ tự trong code; nếu bước sau thất bại, phần đã nối phải được tháo trước khi xóa đối tượng để tránh con trỏ treo. `return false` kết thúc ngay nhánh không đạt; các thao tác cập nhật phía sau không chạy. `throw` báo lỗi bằng ngoại lệ cho bên gọi, khác với kết quả `false` thông thường.

**Bước 5 — Từ chối điều kiện không đạt.**

```cpp
        }
        return false;
    }
    return true;
}
```

`return false` kết thúc ngay nhánh không đạt; các thao tác cập nhật phía sau không chạy.


