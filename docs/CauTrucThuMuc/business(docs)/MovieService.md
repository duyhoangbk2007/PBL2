# MovieService

## 1. Tổng quan

`MovieService` là lớp thuộc tầng Business Logic, chịu trách nhiệm xử lý các nghiệp vụ quản lý phim.

Lớp sử dụng `MovieRepository` để lưu trữ, cập nhật và truy xuất dữ liệu phim.

MovieService không trực tiếp đọc hoặc ghi file TXT.

**File liên quan:**
- `src/business/MovieService.h`
- `src/business/MovieService.cpp`
- `src/data_access/MovieRepository.h`

## 2. Thuộc tính

### `MovieRepository& movieRepo`

Tham chiếu đến `MovieRepository` để thực hiện các thao tác dữ liệu phim.

MovieService không sở hữu Repository, vì vậy `MovieRepository` phải tồn tại lâu hơn `MovieService`.

## 3. Các hàm

### 3.1. Constructor

```cpp
MovieService(MovieRepository& movieRepo);
```

Nhận tham chiếu đến `MovieRepository` thông qua danh sách khởi tạo.

Không tự tạo Repository hoặc load dữ liệu.

### 3.2. `load()`

```cpp
bool load();
```

Gọi `movieRepo.load()` để nạp dữ liệu phim từ file TXT vào bộ nhớ.

Việc đọc file, chuyển đổi dữ liệu và kiểm tra định dạng được xử lý tại tầng Repository.

### 3.3. `save()`

```cpp
bool save();
```

Gọi `movieRepo.save()` để lưu dữ liệu phim hiện tại xuống file TXT.

### 3.4. `addMovie()`

```cpp
bool addMovie(Movie* movie);
```

Gọi `movieRepo.add(movie)` để thêm phim mới.

Repository chịu trách nhiệm kiểm tra dữ liệu phim và ID trùng lặp.

**Lưu ý:** Khi thêm thành công, Repository quản lý vòng đời đối tượng Movie được truyền vào.

### 3.5. `updateMovie()`

```cpp
bool updateMovie(const Movie& newValues);
```

**Chức năng:** Cập nhật thông tin phim, đồng thời kiểm tra các ràng buộc nghiệp vụ.

Đây là hàm có nhiều xử lý nhất trong MovieService.

**Quy trình:**

**Bước 1: Kiểm tra Repository**

```cpp
if (!movieRepo.isLoaded())
{
    return false;
}
```

Không cho phép cập nhật khi dữ liệu chưa được load.

**Bước 2: Tìm phim hiện tại**

```cpp
Movie* movie = findMovieById(newValues.getMovieId());

if (movie == nullptr)
{
    return false;
}
```

Nếu không tìm thấy phim theo ID thì không thể cập nhật.

**Bước 3: Kiểm tra trạng thái active**

```cpp
if (movie->isActive() != newValues.isActive())
{
    return false;
}
```

Không cho phép thay đổi trạng thái hoạt động thông qua `updateMovie()`.

Việc thay đổi trạng thái phim sẽ được xử lý bằng nghiệp vụ riêng nếu cần bổ sung sau này.

**Bước 4: Kiểm tra thay đổi ngày**

```cpp
bool dateChanged =
    movie->getReleaseDate() != newValues.getReleaseDate() ||
    movie->getEndDate() != newValues.getEndDate();
```

Biến `dateChanged` xác định ngày phát hành hoặc ngày kết thúc có thay đổi hay không.

**Bước 5: Kiểm tra Showtime**

```cpp
if (dateChanged && movie->getShowtimeCount() > 0)
{
    return false;
}
```

Nếu phim đã có suất chiếu thì không cho phép thay đổi khoảng ngày phát hành và kết thúc.

Quy tắc này giúp hạn chế việc cập nhật ngày phim làm ảnh hưởng đến các suất chiếu đã được tạo.

**Bước 6: Cập nhật dữ liệu**

```cpp
return movieRepo.update(newValues);
```

Sau khi vượt qua các kiểm tra nghiệp vụ, MovieService chuyển việc cập nhật cho Repository.

Repository tiếp tục kiểm tra tính hợp lệ của dữ liệu và thay đổi các thuộc tính của Movie.

**Kết quả:**
- `true`: Cập nhật thành công.
- `false`: Không thỏa điều kiện cập nhật hoặc Repository từ chối.

### 3.6. `findMovieById()`

```cpp
Movie* findMovieById(const string& movieId);
```

Gọi `movieRepo.findById(movieId)` để tìm phim theo ID.

Trả về `nullptr` nếu không tìm thấy.

### 3.7. `getMovieAt()`

```cpp
Movie* getMovieAt(int index);
```

Gọi `movieRepo.getAt(index)` để lấy phim theo vị trí trong danh sách.

Trả về `nullptr` nếu vị trí không hợp lệ.

### 3.8. `getMovieCount()`

```cpp
int getMovieCount();
```

Gọi `movieRepo.count()` để lấy tổng số phim đang được quản lý trong bộ nhớ.

Không phân biệt phim đang hoạt động hay ngừng hoạt động.

## 4. Luồng hoạt động

```text
       MovieService
            |
            v
      MovieRepository
            |
            v
      Repository<Movie>
            |
            v
        File TXT
```

Đối với thao tác cập nhật:

```text
updateMovie(newValues)
        |
        v
Kiem tra Repository da load
        |
        v
Tim phim theo ID
        |
        v
Kiem tra trang thai active
        |
        v
Kiem tra thay doi ngay
        |
        v
Kiem tra so luong Showtime
        |
        v
movieRepo.update(newValues)
        |
        v
Tra ve ket qua
```

## 5. Lưu ý

- MovieService xử lý các quy tắc nghiệp vụ, MovieRepository xử lý dữ liệu.
- Không cho phép thay đổi `active` thông qua `updateMovie()`.
- Không cho phép thay đổi khoảng ngày của phim đã có Showtime.
- Các hàm tìm kiếm trả về con trỏ đến Movie đang được Repository quản lý.
- `getMovieCount()` trả về tổng số phim, không phải số phim đang chiếu.
- Các nghiệp vụ như lọc phim đang chiếu, phim sắp chiếu và kiểm tra quyền Admin có thể được bổ sung sau khi các thành phần liên quan hoàn thiện.