# MovieRepository — mã nguồn và quy tắc dữ liệu

## 1. Tổng quan

`MovieRepository` chứa `Repository<Movie>` để quản lý danh sách, TXT và bộ nhớ. Nó kiểm tra dữ liệu riêng của Movie, chuyển đổi một dòng TXT và chuyển các thao tác tìm/thêm/xóa cho Repository chung. Các con trỏ trả về là con trỏ mượn; caller không được `delete`.

Repository không phân quyền và không thay thế quy tắc nghiệp vụ ở `MovieService`. ID phim không đổi khi cập nhật; các điều kiện về thời lượng, độ tuổi, ngày phát hành và trạng thái được kiểm tra trong code từng hàm.

Mã nguồn: `src/data_access/MovieRepository.cpp` và `src/data_access/MovieRepository.h`. Các đoạn code dưới đây giữ nguyên nội dung hàm, gồm dòng trống và comment trong hàm. Khi ghép các đoạn của một hàm theo thứ tự, ta thu được đúng định nghĩa gốc.

## 2. Chi tiết từng hàm

### MovieRepository::isValidMovie()

#### A. Tổng quan

- **Mục đích:** Kiểm tra tính hợp lệ của dữ liệu đầu vào.
- **Tham số:** `const Movie* movie`.
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
Kiểm tra dữ liệu hợp lệ (không đạt: false)
    |
    v
Từ chối điều kiện không đạt (không đạt: false)
    |
    v
Kết thúc
```

#### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1 — Kiểm tra dữ liệu hợp lệ.**

```cpp
bool MovieRepository::isValidMovie(const Movie* movie)
{
    if (movie == nullptr)
    {
        return false;
    }
    if (!Validator::isValidId(movie->getMovieId(), "MOV"))
    {
        return false;
```

Điều kiện hợp lệ ngăn dữ liệu sai định dạng hoặc ngoài giới hạn đi vào danh sách đang quản lý. `return false` kết thúc ngay nhánh không đạt; các thao tác cập nhật phía sau không chạy.

**Bước 2 — Kiểm tra dữ liệu hợp lệ.**

```cpp
    }
    if (!Validator::isValidText(movie->getTitle()) || !Validator::isValidText(movie->getGenre()))
    {
        return false;
    }
    if (movie->getDurationMinutes() <= 0)
    {
        return false;
```

Điều kiện hợp lệ ngăn dữ liệu sai định dạng hoặc ngoài giới hạn đi vào danh sách đang quản lý. `return false` kết thúc ngay nhánh không đạt; các thao tác cập nhật phía sau không chạy.

**Bước 3 — Kiểm tra dữ liệu hợp lệ.**

```cpp
    }
    if (movie->getMinAge() < 0)
    {
        return false;
    }
    if (!Validator::isValidDate(movie->getReleaseDate()) || !Validator::isValidDate(movie->getEndDate()))
    {
        return false;
```

Điều kiện hợp lệ ngăn dữ liệu sai định dạng hoặc ngoài giới hạn đi vào danh sách đang quản lý. `return false` kết thúc ngay nhánh không đạt; các thao tác cập nhật phía sau không chạy.

**Bước 4 — Từ chối điều kiện không đạt.**

```cpp
    }
    if (movie->getReleaseDate() > movie->getEndDate())
    {
        return false;
    }
    return true;
}
```

`return false` kết thúc ngay nhánh không đạt; các thao tác cập nhật phía sau không chạy.

### MovieRepository::MovieRepository()

#### A. Tổng quan

- **Mục đích:** Khởi tạo Repository và các tham chiếu/callback cần dùng.
- **Tham số:** `const string& filePath`.
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
MovieRepository::MovieRepository(const string& filePath)
    : repo(filePath, getMovieId, stringToMovie, movieToString)
{
}
```

Danh sách sau dấu `:` khởi tạo thành viên trước thân constructor; tham chiếu được giữ để dùng về sau, còn thân hàm không nạp file.

### MovieRepository::getMovieId()

#### A. Tổng quan

- **Mục đích:** Lấy ID của đối tượng làm khóa.
- **Tham số:** `Movie* movie`.
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
string MovieRepository::getMovieId(Movie* movie)
{
    if (movie == nullptr)
    {
        return "";
    }
    return movie->getMovieId();
}
```

Đoạn này chuẩn bị hoặc trả giá trị theo code. Nó chỉ đổi RAM hoặc TXT nếu có lời gọi cập nhật hay ghi dữ liệu tương ứng.

### MovieRepository::stringToMovie()

#### A. Tổng quan

- **Mục đích:** Chuyển một dòng TXT thành đối tượng Model.
- **Tham số:** `const string& line`.
- **Giá trị trả về:** `Movie*`; xem các nhánh `return` bên dưới.
- **Điều kiện quan trọng:** dữ liệu hợp lệ, ngoại lệ.

#### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Duyệt và đối chiếu từng phần tử
    |
    v
Kiểm tra điều kiện trước khi tiếp tục
    |
    v
Kiểm tra điều kiện trước khi tiếp tục (lỗi: throw)
    |
    v
Kiểm tra điều kiện trước khi tiếp tục (lỗi: throw)
    |
    v
Bắt lỗi và khôi phục trạng thái (lỗi: throw)
    |
    v
Chuẩn bị giá trị cho bước kế tiếp
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
Movie* MovieRepository::stringToMovie(const string& line)
{
    vector<string> fields; //lưu dữ liệu sau khi tách chuỗi
    size_t start = 0;
    // Tach cac truong du lieu theo dau |
    while (true)
    {
        size_t pos = line.find('|', start);
```

Vòng lặp xét từng bản ghi; điều kiện bên trong quyết định tiếp tục, bỏ qua hay dừng khi gặp dữ liệu sai.

**Bước 2 — Kiểm tra điều kiện trước khi tiếp tục.**

```cpp
        if (pos == string::npos) // nếu tới chuỗi ở cuối line sẽ k tìm thấy | sẽ trả về npos (-1)
        {
            fields.push_back(line.substr(start));
            break;
        }
        fields.push_back(line.substr(start, pos - start));
        start = pos + 1;
    }
```

Đoạn này chuẩn bị hoặc trả giá trị theo code. Nó chỉ đổi RAM hoặc TXT nếu có lời gọi cập nhật hay ghi dữ liệu tương ứng.

**Bước 3 — Kiểm tra điều kiện trước khi tiếp tục.**

```cpp
    // Movie phai co dung 8 truong
    if (fields.size() != 8)
    {
        throw runtime_error("Sai so luong truong Movie");
    }
    int durationMinutes;
    int minAge;
    // Chuyen du lieu so
    try
    {
        size_t pos1 = 0;
```

`try/catch` xử lý ngoại lệ trong phạm vi hiện tại; code dọn các thay đổi trước khi trả lỗi hoặc ném lại, nên thứ tự dọn có ý nghĩa. `throw` báo lỗi bằng ngoại lệ cho bên gọi, khác với kết quả `false` thông thường.

**Bước 4 — Kiểm tra điều kiện trước khi tiếp tục.**

```cpp
        size_t pos2 = 0;
        durationMinutes = stoi(fields[3], &pos1);
        minAge = stoi(fields[4], &pos2);
        if (pos1 != fields[3].length() || pos2 != fields[4].length())
        {
            throw runtime_error("Sai dinh dang so");
        }
    }
```

`throw` báo lỗi bằng ngoại lệ cho bên gọi, khác với kết quả `false` thông thường. Phép parse đổi chuỗi TXT sang trường có kiểu; dữ liệu không chuyển được phải bị từ chối trước khi tạo đối tượng hợp lệ.

**Bước 5 — Bắt lỗi và khôi phục trạng thái.**

```cpp
    catch (const exception&)
    {
        throw runtime_error("Thoi luong hoac do tuoi khong hop le");
    }

    // Kiem tra trang thai (active)
    if (fields[7] != "0" && fields[7] != "1")
    {
        throw runtime_error("Trang thai Movie khong hop le");
```

`try/catch` xử lý ngoại lệ trong phạm vi hiện tại; code dọn các thay đổi trước khi trả lỗi hoặc ném lại, nên thứ tự dọn có ý nghĩa. `throw` báo lỗi bằng ngoại lệ cho bên gọi, khác với kết quả `false` thông thường.

**Bước 6 — Chuẩn bị giá trị cho bước kế tiếp.**

```cpp
    }
    bool active = (fields[7] == "1");

    // Tao doi tuong Movie
    Movie* movie = new Movie(
        fields[0],
        fields[1],
        fields[2],
        durationMinutes,
        minAge,
        fields[5],
        fields[6],
        active
    );
```

`new` cấp phát Model trên heap. Cho đến khi Repository nhận thành công, nhánh lỗi hoặc caller vẫn phải chịu trách nhiệm giải phóng.

**Bước 7 — Kiểm tra dữ liệu hợp lệ.**

```cpp
    // Kiem tra du lieu Movie
    if (!isValidMovie(movie))
    {
        delete movie;
        throw runtime_error("Du lieu Movie khong hop le");
    }
    return movie;
}
```

Điều kiện hợp lệ ngăn dữ liệu sai định dạng hoặc ngoài giới hạn đi vào danh sách đang quản lý. `delete` giải phóng đối tượng; không được dùng lại con trỏ sau lệnh này. `throw` báo lỗi bằng ngoại lệ cho bên gọi, khác với kết quả `false` thông thường.

### MovieRepository::movieToString()

#### A. Tổng quan

- **Mục đích:** Chuyển đối tượng Model thành một dòng TXT.
- **Tham số:** `Movie* movie`.
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
Chuẩn bị giá trị cho bước kế tiếp
    |
    v
Kết thúc
```

#### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1 — Kiểm tra dữ liệu hợp lệ.**

```cpp
string MovieRepository::movieToString(Movie* movie)
{
    if (!isValidMovie(movie))
    {
        throw runtime_error("Movie khong hop le khi ghi file");
    }
    return movie->getMovieId() + "|" +
           movie->getTitle() + "|" +
           movie->getGenre() + "|" +
           to_string(movie->getDurationMinutes()) + "|" +
           to_string(movie->getMinAge()) + "|" +
           movie->getReleaseDate() + "|" +
           movie->getEndDate() + "|" +
           (movie->isActive() ? "1" : "0");
```

Điều kiện hợp lệ ngăn dữ liệu sai định dạng hoặc ngoài giới hạn đi vào danh sách đang quản lý. `throw` báo lỗi bằng ngoại lệ cho bên gọi, khác với kết quả `false` thông thường.

**Bước 2 — Chuẩn bị giá trị cho bước kế tiếp.**

```cpp
}
```

Đoạn này chuẩn bị hoặc trả giá trị theo code. Nó chỉ đổi RAM hoặc TXT nếu có lời gọi cập nhật hay ghi dữ liệu tương ứng.

### MovieRepository::load()

#### A. Tổng quan

- **Mục đích:** Nạp và kiểm tra dữ liệu từ TXT vào RAM.
- **Tham số:** không có.
- **Giá trị trả về:** `bool`; xem các nhánh `return` bên dưới.
- **Điều kiện quan trọng:** không có điều kiện đặc biệt ngoài các câu lệnh bên dưới.

#### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Đọc dữ liệu TXT
    |
    v
Kết thúc
```

#### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1 — Đọc dữ liệu TXT.**

```cpp
bool MovieRepository::load()
{
    return repo.load();
}
```

Lời gọi nạp đọc TXT vào bộ nhớ. Nếu trả `false`, hàm dừng ở nhánh lỗi; dữ liệu chỉ được xem là dùng được sau khi các bước kiểm tra tiếp theo hoàn tất.

### MovieRepository::save()

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
bool MovieRepository::save()
{
    return repo.save();
}
```

Lời gọi ghi chuyển trạng thái hiện tại ra TXT. Kết quả ghi quyết định giá trị trả về; một lời gọi này không bảo đảm các file Repository khác cùng được cập nhật.

### MovieRepository::add()

#### A. Tổng quan

- **Mục đích:** Thêm đối tượng hợp lệ vào Repository.
- **Tham số:** `Movie* movie`.
- **Giá trị trả về:** `bool`; xem các nhánh `return` bên dưới.
- **Điều kiện quan trọng:** trạng thái nạp, dữ liệu hợp lệ, nhánh trả `false`.

#### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Kiểm tra dữ liệu hợp lệ (không đạt: false)
    |
    v
Trả kết quả cho bên gọi
    |
    v
Kết thúc
```

#### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1 — Kiểm tra dữ liệu hợp lệ.**

```cpp
bool MovieRepository::add(Movie* movie)
{
    if (!repo.isLoaded() || !isValidMovie(movie))
    {
        return false;
    }
    // Khong them lai con tro da duoc quan ly
    for (int i = 0; i < repo.count(); i++)
    {
        if (repo.getAt(i) == movie)
        {
            return false;
```

Trạng thái `loaded` quyết định Repository đã sẵn sàng chưa; nhánh chưa nạp dừng trước khi thao tác dựa trên dữ liệu đó. Điều kiện hợp lệ ngăn dữ liệu sai định dạng hoặc ngoài giới hạn đi vào danh sách đang quản lý. `return false` kết thúc ngay nhánh không đạt; các thao tác cập nhật phía sau không chạy.

**Bước 2 — Trả kết quả cho bên gọi.**

```cpp
        }
    }
    return repo.add(movie);
}
```

Đoạn này chuẩn bị hoặc trả giá trị theo code. Nó chỉ đổi RAM hoặc TXT nếu có lời gọi cập nhật hay ghi dữ liệu tương ứng.

### MovieRepository::update()

#### A. Tổng quan

- **Mục đích:** Cập nhật đối tượng đang quản lý.
- **Tham số:** `const Movie& newValues`.
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
Từ chối điều kiện không đạt (không đạt: false)
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
bool MovieRepository::update(const Movie& newValues)
{
    if (!repo.isLoaded() || !isValidMovie(&newValues))
    {
        return false;
    }
    Movie* movie = repo.findById(newValues.getMovieId());

```

Trạng thái `loaded` quyết định Repository đã sẵn sàng chưa; nhánh chưa nạp dừng trước khi thao tác dựa trên dữ liệu đó. Điều kiện hợp lệ ngăn dữ liệu sai định dạng hoặc ngoài giới hạn đi vào danh sách đang quản lý. `return false` kết thúc ngay nhánh không đạt; các thao tác cập nhật phía sau không chạy.

**Bước 2 — Từ chối điều kiện không đạt.**

```cpp
    if (movie == nullptr)
    {
        return false;
    }
    movie->setTitle(newValues.getTitle());
    movie->setGenre(newValues.getGenre());
    movie->setDurationMinutes(newValues.getDurationMinutes());
    movie->setMinAge(newValues.getMinAge());
```

`return false` kết thúc ngay nhánh không đạt; các thao tác cập nhật phía sau không chạy.

**Bước 3 — Xác nhận xử lý thành công.**

```cpp
    movie->setReleaseDate(newValues.getReleaseDate());
    movie->setEndDate(newValues.getEndDate());
    movie->setActive(newValues.isActive());
    return true;
}
```

Đoạn này chuẩn bị hoặc trả giá trị theo code. Nó chỉ đổi RAM hoặc TXT nếu có lời gọi cập nhật hay ghi dữ liệu tương ứng.

### MovieRepository::removeById()

#### A. Tổng quan

- **Mục đích:** Xóa đối tượng theo khóa sau khi kiểm tra ràng buộc.
- **Tham số:** `const string& movieId`.
- **Giá trị trả về:** `bool`; xem các nhánh `return` bên dưới.
- **Điều kiện quan trọng:** trạng thái nạp, con trỏ rỗng, nhánh trả `false`.

#### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Tra cứu đối tượng theo khóa (không đạt: false)
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
bool MovieRepository::removeById(const string& movieId)
{
    if (!repo.isLoaded())
    {
        return false;
    }
    Movie* movie = repo.findById(movieId);
    if (movie == nullptr)
    {
        return false;
```

Trạng thái `loaded` quyết định Repository đã sẵn sàng chưa; nhánh chưa nạp dừng trước khi thao tác dựa trên dữ liệu đó. `return false` kết thúc ngay nhánh không đạt; các thao tác cập nhật phía sau không chạy.

**Bước 2 — Từ chối điều kiện không đạt.**

```cpp
    }
    // Khong xoa neu phim con lien ket Showtime
    if (movie->getShowtimeCount() > 0)
    {
        return false;
    }
    return repo.removeById(movieId);
}
```

`return false` kết thúc ngay nhánh không đạt; các thao tác cập nhật phía sau không chạy.

### MovieRepository::findById()

#### A. Tổng quan

- **Mục đích:** Tra đối tượng theo ID.
- **Tham số:** `const string& movieId`.
- **Giá trị trả về:** `Movie*`; xem các nhánh `return` bên dưới.
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
Movie* MovieRepository::findById(const string& movieId)
{
    return repo.findById(movieId);
}
```

Đoạn này chuẩn bị hoặc trả giá trị theo code. Nó chỉ đổi RAM hoặc TXT nếu có lời gọi cập nhật hay ghi dữ liệu tương ứng.

### MovieRepository::getAt()

#### A. Tổng quan

- **Mục đích:** Lấy phần tử theo chỉ số.
- **Tham số:** `int index`.
- **Giá trị trả về:** `Movie*`; xem các nhánh `return` bên dưới.
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
Movie* MovieRepository::getAt(int index)
{
    return repo.getAt(index);
}
```

Đoạn này chuẩn bị hoặc trả giá trị theo code. Nó chỉ đổi RAM hoặc TXT nếu có lời gọi cập nhật hay ghi dữ liệu tương ứng.

### MovieRepository::count()

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
int MovieRepository::count()
{
    return repo.count();
}
```

Đoạn này chuẩn bị hoặc trả giá trị theo code. Nó chỉ đổi RAM hoặc TXT nếu có lời gọi cập nhật hay ghi dữ liệu tương ứng.

### MovieRepository::isLoaded()

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
bool MovieRepository::isLoaded()
{
    return repo.isLoaded();
}
```

Trạng thái `loaded` quyết định Repository đã sẵn sàng chưa; nhánh chưa nạp dừng trước khi thao tác dựa trên dữ liệu đó.


