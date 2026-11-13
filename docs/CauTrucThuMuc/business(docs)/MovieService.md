# MovieService

## 1. Tổng quan

`MovieService` là lớp thuộc tầng Business Logic, chịu trách nhiệm xử lý các nghiệp vụ quản lý phim và kiểm tra quyền truy cập.

Lớp sử dụng:
- `MovieRepository` để thêm, sửa, xóa và truy xuất dữ liệu phim.
- `AuthService` để kiểm tra trạng thái đăng nhập và quyền của người thực hiện.

MovieService không trực tiếp đọc hoặc ghi file TXT.

**Quy tắc phân quyền:**
- **Admin:** Được thêm, sửa, xóa và tra cứu phim.
- **Staff:** Chỉ được tra cứu thông tin phim.
- **Chưa đăng nhập:** Không được thực hiện các thao tác quản lý hoặc tra cứu phim.

**File liên quan:**
- `src/business/MovieService.h`
- `src/business/MovieService.cpp`
- `src/business/AuthService.h`
- `src/data_access/MovieRepository.h`

## 2. Thuộc tính

### `MovieRepository& movieRepo`

Tham chiếu đến `MovieRepository` để thực hiện các thao tác dữ liệu phim.

### `AuthService& authService`

Tham chiếu đến `AuthService` để kiểm tra quyền của tài khoản đang đăng nhập.

MovieService không sở hữu hai đối tượng này, vì vậy `MovieRepository` và `AuthService` phải tồn tại lâu hơn `MovieService`.

## 3. Các hàm

### 3.1. Constructor

```cpp
MovieService(MovieRepository& movieRepo, AuthService& authService);
```

Nhận tham chiếu đến `MovieRepository` và `AuthService` thông qua danh sách khởi tạo.

Không tự tạo Repository, AuthService hoặc tự đăng nhập.

### 3.2. `load()`

```cpp
bool load();
```

Gọi `movieRepo.load()` để nạp dữ liệu phim từ file TXT vào bộ nhớ.

Việc đọc file, chuyển đổi dữ liệu và kiểm tra định dạng được xử lý tại Repository.

Đây là thao tác khởi tạo dữ liệu của hệ thống, không yêu cầu quyền Admin.

### 3.3. `save()`

```cpp
bool save();
```

Gọi `movieRepo.save()` để lưu dữ liệu phim hiện tại xuống file TXT.

Đây là thao tác lưu trữ dữ liệu của hệ thống, không yêu cầu quyền Admin.

### 3.4. `addMovie()`

```cpp
bool addMovie(Movie* movie);
```

**Chức năng:** Thêm phim mới vào hệ thống.

**Quy trình:**

1. Gọi `authService.requireAdmin()` để kiểm tra quyền Admin.
2. Nếu hợp lệ, gọi `movieRepo.add(movie)` để thêm phim.
3. Repository kiểm tra dữ liệu phim và ID trùng lặp.

```cpp
authService.requireAdmin();

return movieRepo.add(movie);
```

**Kết quả:**
- `true`: Thêm phim thành công.
- `false`: Repository từ chối thêm phim.
- Ném `runtime_error`: Chưa đăng nhập hoặc không có quyền Admin.

**Lưu ý:** Khi thêm thành công, Repository quản lý vòng đời đối tượng Movie được truyền vào.

### 3.5. `updateMovie()`

```cpp
bool updateMovie(const Movie& newValues);
```

**Chức năng:** Cập nhật thông tin phim, đồng thời kiểm tra quyền Admin và các ràng buộc nghiệp vụ.

**Quy trình:**

**Bước 1: Kiểm tra quyền Admin**

```cpp
authService.requireAdmin();
```

Nếu chưa đăng nhập hoặc không có quyền Admin, hàm ném ngoại lệ và không tiếp tục cập nhật.

**Bước 2: Kiểm tra Repository**

```cpp
if (!movieRepo.isLoaded())
{
    return false;
}
```

Không cho phép cập nhật khi dữ liệu chưa được load.

**Bước 3: Tìm phim hiện tại**

```cpp
Movie* movie = movieRepo.findById(newValues.getMovieId());

if (movie == nullptr)
{
    return false;
}
```

Nếu không tìm thấy phim theo ID thì không thể cập nhật.

**Bước 4: Kiểm tra trạng thái active**

```cpp
if (movie->isActive() != newValues.isActive())
{
    return false;
}
```

Không cho phép thay đổi trạng thái hoạt động thông qua `updateMovie()`.

Việc thay đổi trạng thái phim sẽ được xử lý bằng nghiệp vụ riêng nếu cần bổ sung sau này.

**Bước 5: Kiểm tra thay đổi ngày**

```cpp
bool dateChanged =
    movie->getReleaseDate() != newValues.getReleaseDate() ||
    movie->getEndDate() != newValues.getEndDate();
```

Biến `dateChanged` xác định ngày phát hành hoặc ngày kết thúc có thay đổi hay không.

**Bước 6: Kiểm tra Showtime**

```cpp
if (dateChanged && movie->getShowtimeCount() > 0)
{
    return false;
}
```

Nếu phim đã có suất chiếu thì không cho phép thay đổi khoảng ngày phát hành và kết thúc.

Quy tắc này giúp tránh ảnh hưởng đến các suất chiếu đã được tạo.

**Bước 7: Cập nhật dữ liệu**

```cpp
return movieRepo.update(newValues);
```

Sau khi vượt qua các kiểm tra, MovieService chuyển việc cập nhật cho Repository.

Repository tiếp tục kiểm tra tính hợp lệ của dữ liệu và thay đổi các thuộc tính của Movie.

**Kết quả:**
- `true`: Cập nhật thành công.
- `false`: Không thỏa điều kiện cập nhật hoặc Repository từ chối.
- Ném `runtime_error`: Không có quyền thực hiện thao tác.

### 3.6. `removeMovie()`

```cpp
bool removeMovie(const string& movieId);
```

**Chức năng:** Xóa phim khỏi hệ thống.

**Quy trình:**

1. Gọi `authService.requireAdmin()` để kiểm tra quyền Admin.
2. Gọi `movieRepo.removeById(movieId)` để xóa phim.
3. Repository kiểm tra phim có tồn tại và còn liên kết Showtime hay không.

```cpp
authService.requireAdmin();

return movieRepo.removeById(movieId);
```

**Kết quả:**
- `true`: Xóa phim thành công.
- `false`: Không tìm thấy phim hoặc phim còn liên kết Showtime.
- Ném `runtime_error`: Không có quyền Admin.

### 3.7. `findMovieById()`

```cpp
const Movie* findMovieById(const string& movieId);
```

Gọi `authService.requireLogin()` để yêu cầu đăng nhập.

Sau đó gọi `movieRepo.findById(movieId)` để tìm phim theo ID.

- Trả về con trỏ Movie nếu tìm thấy.
- Trả về `nullptr` nếu không tìm thấy.
- Ném ngoại lệ nếu chưa đăng nhập.

Sử dụng `const Movie*` để không cho phép sửa trực tiếp thông tin phim thông qua con trỏ trả về.

### 3.8. `getMovieAt()`

```cpp
const Movie* getMovieAt(int index);
```

Yêu cầu đăng nhập bằng `authService.requireLogin()`.

Sau đó gọi `movieRepo.getAt(index)` để lấy phim theo vị trí trong danh sách.

Trả về `nullptr` nếu vị trí không hợp lệ.

### 3.9. `getMovieCount()`

```cpp
int getMovieCount();
```

Yêu cầu đăng nhập bằng `authService.requireLogin()`.

Sau đó gọi `movieRepo.count()` để lấy tổng số phim đang được quản lý trong bộ nhớ.

Không phân biệt phim đang hoạt động hay ngừng hoạt động.

## 4. Luồng hoạt động

### 4.1. Quan hệ giữa các lớp

```text
             MovieService
              /        \
             v          v
       AuthService   MovieRepository
             |          |
             v          v
      UserRepository  Repository<Movie>
                        |
                        v
                      File TXT
```

- `AuthService`: Kiểm tra đăng nhập và phân quyền.
- `MovieRepository`: Thao tác dữ liệu phim.
- `MovieService`: Kết hợp kiểm tra quyền và xử lý nghiệp vụ.

### 4.2. Luồng cập nhật phim

```text
updateMovie(newValues)
        |
        v
authService.requireAdmin()
        |
        +-- Khong co quyen --> throw runtime_error
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

## 5. Xử lý ngoại lệ và phân quyền

MovieService sử dụng hai hàm của AuthService:

### `requireLogin()`

Yêu cầu tài khoản đã đăng nhập hợp lệ.

Được sử dụng trong các hàm tra cứu phim, cho phép cả Admin và Staff.

### `requireAdmin()`

Yêu cầu tài khoản hiện tại có quyền Admin.

Được sử dụng trong:
- `addMovie()`
- `updateMovie()`
- `removeMovie()`

Nếu không thỏa điều kiện, AuthService ném `runtime_error`. Khi đó, thao tác bị dừng trước khi gọi Repository.

**Phân biệt kết quả:**
- `true`: Thao tác thành công.
- `false`: Thao tác không thỏa điều kiện nghiệp vụ thông thường.
- `nullptr`: Không tìm thấy đối tượng cần tra cứu.
- `throw`: Vi phạm điều kiện bắt buộc về quyền truy cập hoặc phát sinh lỗi bất thường.

Ngoại lệ sẽ được truyền về tầng gọi để xử lý bằng `try-catch`.

## 6. Lưu ý

- MovieService xử lý phân quyền và quy tắc nghiệp vụ, MovieRepository xử lý dữ liệu.
- Chỉ Admin được thêm, sửa và xóa phim.
- Admin và Staff đều được tra cứu thông tin phim sau khi đăng nhập.
- `load()` và `save()` phục vụ hệ thống, không trực tiếp kiểm tra quyền.
- Không cho phép thay đổi `active` thông qua `updateMovie()`.
- Không cho phép thay đổi khoảng ngày của phim đã có Showtime.
- Các hàm tra cứu trả về `const Movie*` để hạn chế sửa dữ liệu trực tiếp.
- `getMovieCount()` trả về tổng số phim, không phải số phim đang chiếu.
- Các thao tác quản lý phim cần đi qua MovieService để áp dụng kiểm tra quyền.
- Các nghiệp vụ như lọc phim đang chiếu, phim sắp chiếu và thay đổi trạng thái phim có thể được bổ sung sau.