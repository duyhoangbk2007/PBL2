# MovieService

## 1. Tổng quan

`MovieService` thuộc tầng **Business Logic**, chịu trách nhiệm xử lý các nghiệp vụ quản lý phim, kiểm tra quyền truy cập và bảo vệ tính nhất quán của dữ liệu.

Lớp sử dụng:

- `MovieRepository`: Thêm, sửa, xóa và truy xuất dữ liệu phim.
- `AuthService`: Kiểm tra tài khoản đăng nhập và quyền thực hiện nghiệp vụ.
- `DateTime`: Lấy ngày hiện tại để xác định phim đang chiếu hoặc sắp chiếu.

`MovieService` không trực tiếp đọc hoặc ghi file TXT.

### 1.1. File liên quan

- `src/business/MovieService.h`
- `src/business/MovieService.cpp`
- `src/business/AuthService.h`
- `src/data_access/MovieRepository.h`
- `src/data_access/MovieRepository.cpp`
- `src/models/Movie.h`
- `src/utils/DateTime.h`

### 1.2. Phạm vi chức năng

MovieService hỗ trợ:

1. Nạp và lưu dữ liệu phim.
2. Thêm phim.
3. Cập nhật thông tin phim.
4. Xóa phim chưa được tham chiếu.
5. Bật hoặc ngừng hoạt động phim.
6. Tra cứu phim theo mã.
7. Lấy phim theo vị trí và đếm tổng số phim.
8. Lọc phim đang chiếu.
9. Lọc phim sắp chiếu.
10. Tìm kiếm phim theo tên.

## 2. Thuộc tính

```cpp
private:
    MovieRepository& movieRepo;
    AuthService& authService;
```

### 2.1. movieRepo

Tham chiếu đến Repository quản lý các đối tượng `Movie`.

MovieService không sở hữu Repository và không tự giải phóng đối tượng này.

### 2.2. authService

Tham chiếu đến `AuthService` để xác định quyền thực hiện thao tác.

Cả hai đối tượng được truyền vào phải tồn tại lâu hơn `MovieService`.

## 3. Phân quyền

MovieService sử dụng cơ chế RBAC thông qua `AuthService::requirePermission()`.

| Chức năng | Permission |
|---|---|
| `addMovie()` | `MOVIE_ADD` |
| `updateMovie()` | `MOVIE_UPDATE` |
| `removeMovie()` | `MOVIE_DELETE` |
| `setMovieActive()` | `MOVIE_UPDATE` |
| `findMovieById()` | `MOVIE_VIEW` |
| `getMovieAt()` | `MOVIE_VIEW` |
| `getMovieCount()` | `MOVIE_VIEW` |
| `getNowShowingMovies()` | `MOVIE_VIEW` |
| `getComingSoonMovies()` | `MOVIE_VIEW` |
| `searchMoviesByTitle()` | `MOVIE_VIEW` |

`load()` và `save()` phục vụ quá trình quản lý dữ liệu của hệ thống, không kiểm tra quyền người dùng.

Theo bảng quyền mặc định:

- Admin được quản lý và tra cứu phim.
- Staff chỉ được tra cứu phim.
- Người chưa đăng nhập không được thực hiện nghiệp vụ có yêu cầu permission.

Quyền thực tế phụ thuộc vào `RolePermissionRepository`. Admin không tự động có quyền nếu bảng quyền không cấp permission tương ứng.

## Chi tiết triển khai từng hàm (MovieService.cpp)

### MovieService()

#### A. Tổng quan

- **Mục đích:** Khởi tạo Service với các Repository và AuthService.
- **Tham số:** `MovieRepository& movieRepo, AuthService& authService) : movieRepo(movieRepo), authService(authService`.
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
MovieService::MovieService(MovieRepository& movieRepo, AuthService& authService)
    : movieRepo(movieRepo), authService(authService)
{}
```

Các câu lệnh được thực thi theo đúng thứ tự trong `.cpp`; điều kiện và nhánh trả về nằm ngay trong code. Giá trị được khai báo ở đây được dùng trong các nhánh tiếp theo; hãy theo dõi nơi nó được trả về hoặc chuyển vào Repository để hiểu tác động lên trạng thái.

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
bool MovieService::load()
{
    return movieRepo.load();
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
bool MovieService::save()
{
    return movieRepo.save();
}
```

Repository thực hiện thao tác dữ liệu. `save()` ghi trạng thái Repository ra TXT tại thời điểm lời gọi; thay đổi RAM trước đó không tự đồng nghĩa với đã lưu thành công.

### addMovie()

#### A. Tổng quan

- **Mục đích:** Kiểm tra quyền. Xem các nhánh cụ thể trong code.
- **Tham số:** `Movie* movie`.
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
Tra cứu dữ liệu
    |
    v
Chuẩn bị và xử lý dữ liệu
    |
    v
Cập nhật qua Repository
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
bool MovieService::addMovie(Movie* movie)
{
    authService.requirePermission("MOVIE_ADD");

    if (!movieRepo.isLoaded() || movie == nullptr)
    {
        return false;
    }
    // Lay danh sach ID hien co
    vector<string> existingIds;
```

`AuthService` kiểm tra quyền và có thể ném ngoại lệ. `nullptr` cho biết không có đối tượng để truy cập. `false` báo thao tác không thành công. Quyền `MOVIE_ADD` được kiểm tra trước bước xử lý kế tiếp; thiếu quyền sẽ ném ngoại lệ, vì vậy caller không nên xem đó là kết quả `false`. Kiểm tra `isLoaded()` tránh dùng dữ liệu Repository chưa được nạp; nhánh không đạt kết thúc ngay theo giá trị `return` trong code. Nhánh `false` dừng hàm tại đây; những câu lệnh phía sau không chạy, nên không có cập nhật nào từ các bước sau.

**Bước 2 — Tra cứu dữ liệu.**

```cpp
    for (int i = 0; i < movieRepo.count(); i++)
    {
        Movie* current = movieRepo.getAt(i);
        if (current == nullptr)
        {
            return false;
        }
        existingIds.push_back(current->getMovieId());
    }
    // Sinh ID moi
```

`nullptr` cho biết không có đối tượng để truy cập. `false` báo thao tác không thành công. Vòng `for` duyệt lần lượt các phần tử. Nhánh `false` dừng hàm tại đây; những câu lệnh phía sau không chạy, nên không có cập nhật nào từ các bước sau. Vòng lặp xử lý từng phần tử theo thứ tự; điều kiện bên trong quyết định phần tử được giữ, bỏ qua hay đưa vào kết quả.

**Bước 3 — Chuẩn bị và xử lý dữ liệu.**

```cpp
    string movieId = IdGenerator::generateNextId("MOV", existingIds);
    // Tao Movie moi voi ID vua sinh
    Movie* newMovie = new Movie(movieId,
                               movie->getTitle(),
                               movie->getGenre(),
                               movie->getDurationMinutes(),
                               movie->getMinAge(),
                               movie->getReleaseDate(),
                               movie->getEndDate(),
                               movie->isActive());
```

Các câu lệnh được thực thi theo đúng thứ tự trong `.cpp`; điều kiện và nhánh trả về nằm ngay trong code. `new` cấp phát đối tượng trên heap; cần theo dõi thời điểm quyền sở hữu được chuyển sang Repository để tránh rò rỉ bộ nhớ.

**Bước 4 — Cập nhật qua Repository.**

```cpp
    // Them vao Repository
    try
    {
        if (!movieRepo.add(newMovie))
        {
            delete newMovie;
            return false;
        }
    }
    catch (...)
```

`false` báo thao tác không thành công. Repository thực hiện thao tác dữ liệu. Khi `add()` thành công, Repository nhận đối tượng và quản lý vòng đời của nó; nếu thất bại, bên gọi vẫn phải xét trách nhiệm giải phóng con trỏ. Nhánh `false` dừng hàm tại đây; những câu lệnh phía sau không chạy, nên không có cập nhật nào từ các bước sau. Khối `try/catch` giữ lỗi phát sinh trong phạm vi xử lý này và chuyển sang nhánh khôi phục hoặc báo lỗi đã viết trong code. `delete` giải phóng vùng nhớ của đối tượng, sau đó không được dùng lại con trỏ trỏ đến vùng nhớ đó.

**Bước 5 — Trả kết quả.**

```cpp
    {
        delete newMovie;
        throw;
    }
    return true;
}
```

Các câu lệnh được thực thi theo đúng thứ tự trong `.cpp`; điều kiện và nhánh trả về nằm ngay trong code. `delete` giải phóng vùng nhớ của đối tượng, sau đó không được dùng lại con trỏ trỏ đến vùng nhớ đó.

### updateMovie()

#### A. Tổng quan

- **Mục đích:** Kiểm tra quyền. Xem các nhánh cụ thể trong code.
- **Tham số:** `const Movie& newValues`.
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
Kiểm tra điều kiện
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
bool MovieService::updateMovie(const Movie& newValues)
{
    authService.requirePermission("MOVIE_UPDATE");

    if (!movieRepo.isLoaded())
    {
        return false;
    }
    Movie* movie = movieRepo.findById(newValues.getMovieId());
    if (movie == nullptr)
```

`AuthService` kiểm tra quyền và có thể ném ngoại lệ. `nullptr` cho biết không có đối tượng để truy cập. `false` báo thao tác không thành công. Quyền `MOVIE_UPDATE` được kiểm tra trước bước xử lý kế tiếp; thiếu quyền sẽ ném ngoại lệ, vì vậy caller không nên xem đó là kết quả `false`. Kiểm tra `isLoaded()` tránh dùng dữ liệu Repository chưa được nạp; nhánh không đạt kết thúc ngay theo giá trị `return` trong code. Lệnh tra cứu trả con trỏ đến đối tượng đang được Repository quản lý. Con trỏ rỗng có nghĩa là không tìm thấy, nên bước sau chỉ được dùng đối tượng khi kiểm tra thành công. Nhánh `false` dừng hàm tại đây; những câu lệnh phía sau không chạy, nên không có cập nhật nào từ các bước sau.

**Bước 2 — Từ chối dữ liệu không hợp lệ.**

```cpp
    {
        return false;
    }
    // Khong cho thay doi trang thai qua updateMovie
    if (movie->isActive() != newValues.isActive())
    {
        return false;
    }
    // Kiem tra thay doi ngay phat hanh
    bool dateChanged =
```

`false` báo thao tác không thành công. Nhánh `false` dừng hàm tại đây; những câu lệnh phía sau không chạy, nên không có cập nhật nào từ các bước sau.

**Bước 3 — Kiểm tra điều kiện.**

```cpp
        movie->getReleaseDate() != newValues.getReleaseDate() ||
        movie->getEndDate() != newValues.getEndDate();

    // Kiem tra thay doi thoi luong phim
    bool durationChanged = movie->getDurationMinutes() != newValues.getDurationMinutes();

    // Khong cho doi ngay hoac thoi luong khi da co suat chieu
    if (movie->getShowtimeCount() > 0 &&
        (dateChanged || durationChanged))
    {
```

Các câu lệnh được thực thi theo đúng thứ tự trong `.cpp`; điều kiện và nhánh trả về nằm ngay trong code. Giá trị được khai báo ở đây được dùng trong các nhánh tiếp theo; hãy theo dõi nơi nó được trả về hoặc chuyển vào Repository để hiểu tác động lên trạng thái.

**Bước 4 — Cập nhật qua Repository.**

```cpp
        return false;
    }

    return movieRepo.update(newValues);
}
```

`false` báo thao tác không thành công. Repository thực hiện thao tác dữ liệu. `update()` chuyển việc thay đổi dữ liệu cho Repository; kết quả trả về phản ánh kiểm tra và thao tác cập nhật ở tầng dữ liệu. Nhánh `false` dừng hàm tại đây; những câu lệnh phía sau không chạy, nên không có cập nhật nào từ các bước sau.

### removeMovie()

#### A. Tổng quan

- **Mục đích:** Kiểm tra quyền. Xem các nhánh cụ thể trong code.
- **Tham số:** `const string& movieId`.
- **Giá trị trả về:** `bool`; các nhánh `return` được ghi đầy đủ bên dưới.
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
bool MovieService::removeMovie(const string& movieId)
{
    authService.requirePermission("MOVIE_DELETE");
    return movieRepo.removeById(movieId);
}
```

`AuthService` kiểm tra quyền và có thể ném ngoại lệ. Repository thực hiện thao tác dữ liệu. Quyền `MOVIE_DELETE` được kiểm tra trước bước xử lý kế tiếp; thiếu quyền sẽ ném ngoại lệ, vì vậy caller không nên xem đó là kết quả `false`. Thao tác xóa có thể làm mất đối tượng do Repository sở hữu; các liên kết liên quan cần được Repository xử lý theo đúng ràng buộc của nó.

### findMovieById()

#### A. Tổng quan

- **Mục đích:** Kiểm tra quyền. Xem các nhánh cụ thể trong code.
- **Tham số:** `const string& movieId`.
- **Giá trị trả về:** `const Movie*`; các nhánh `return` được ghi đầy đủ bên dưới.
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
const Movie* MovieService::findMovieById(const string& movieId)
{
    authService.requirePermission("MOVIE_VIEW");
    return movieRepo.findById(movieId);
}
```

`AuthService` kiểm tra quyền và có thể ném ngoại lệ. Quyền `MOVIE_VIEW` được kiểm tra trước bước xử lý kế tiếp; thiếu quyền sẽ ném ngoại lệ, vì vậy caller không nên xem đó là kết quả `false`. Lệnh tra cứu trả con trỏ đến đối tượng đang được Repository quản lý. Con trỏ rỗng có nghĩa là không tìm thấy, nên bước sau chỉ được dùng đối tượng khi kiểm tra thành công.

### getMovieAt()

#### A. Tổng quan

- **Mục đích:** Kiểm tra quyền. Xem các nhánh cụ thể trong code.
- **Tham số:** `int index`.
- **Giá trị trả về:** `const Movie*`; các nhánh `return` được ghi đầy đủ bên dưới.
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
const Movie* MovieService::getMovieAt(int index)
{
    authService.requirePermission("MOVIE_VIEW");
    return movieRepo.getAt(index);
}
```

`AuthService` kiểm tra quyền và có thể ném ngoại lệ. Quyền `MOVIE_VIEW` được kiểm tra trước bước xử lý kế tiếp; thiếu quyền sẽ ném ngoại lệ, vì vậy caller không nên xem đó là kết quả `false`.

### getMovieCount()

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
int MovieService::getMovieCount()
{
    authService.requirePermission("MOVIE_VIEW");
    return movieRepo.count();
}
```

`AuthService` kiểm tra quyền và có thể ném ngoại lệ. Quyền `MOVIE_VIEW` được kiểm tra trước bước xử lý kế tiếp; thiếu quyền sẽ ném ngoại lệ, vì vậy caller không nên xem đó là kết quả `false`.

### setMovieActive()

#### A. Tổng quan

- **Mục đích:** Kiểm tra quyền. Xem các nhánh cụ thể trong code.
- **Tham số:** `const string& movieId, bool active`.
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
Duyệt từng phần tử
    |
    v
Từ chối dữ liệu không hợp lệ
    |
    v
Cập nhật qua Repository
    |
    v
Chuẩn bị và xử lý dữ liệu
    |
    v
Kết thúc
```

#### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1 — Kiểm tra quyền.**

```cpp
bool MovieService::setMovieActive(const string& movieId, bool active)
{
    authService.requirePermission("MOVIE_UPDATE");

    if (!movieRepo.isLoaded())
    {
        return false;
    }
    Movie* movie = movieRepo.findById(movieId);
    if (movie == nullptr)
```

`AuthService` kiểm tra quyền và có thể ném ngoại lệ. `nullptr` cho biết không có đối tượng để truy cập. `false` báo thao tác không thành công. Quyền `MOVIE_UPDATE` được kiểm tra trước bước xử lý kế tiếp; thiếu quyền sẽ ném ngoại lệ, vì vậy caller không nên xem đó là kết quả `false`. Kiểm tra `isLoaded()` tránh dùng dữ liệu Repository chưa được nạp; nhánh không đạt kết thúc ngay theo giá trị `return` trong code. Lệnh tra cứu trả con trỏ đến đối tượng đang được Repository quản lý. Con trỏ rỗng có nghĩa là không tìm thấy, nên bước sau chỉ được dùng đối tượng khi kiểm tra thành công. Nhánh `false` dừng hàm tại đây; những câu lệnh phía sau không chạy, nên không có cập nhật nào từ các bước sau.

**Bước 2 — Từ chối dữ liệu không hợp lệ.**

```cpp
    {
        return false;
    }
    // Trang thai da dung voi yeu cau
    if (movie->isActive() == active)
    {
        return true;
    }
    // Kiem tra cac suat chieu khi ngung hoat dong phim
    if (!active)
```

`false` báo thao tác không thành công. Nhánh `false` dừng hàm tại đây; những câu lệnh phía sau không chạy, nên không có cập nhật nào từ các bước sau.

**Bước 3 — Duyệt từng phần tử.**

```cpp
    {
        string now = DateTime::getCurrentDateTime();
        for (int i = 0; i < movie->getShowtimeCount(); i++)
        {
            const Showtime* showtime = movie->getShowtimeAt(i);
            if (showtime == nullptr)
            {
                return false;
            }
            // suất chiếu đã lên lịch và chưa bắt đầu thì không cho hủy phim
```

`nullptr` cho biết không có đối tượng để truy cập. `false` báo thao tác không thành công. Vòng `for` duyệt lần lượt các phần tử. `DateTime` cung cấp thời gian hoặc phép so sánh thời gian; điều kiện này quyết định dữ liệu có còn hợp lệ ở thời điểm gọi hàm hay không. Nhánh `false` dừng hàm tại đây; những câu lệnh phía sau không chạy, nên không có cập nhật nào từ các bước sau. Vòng lặp xử lý từng phần tử theo thứ tự; điều kiện bên trong quyết định phần tử được giữ, bỏ qua hay đưa vào kết quả.

**Bước 4 — Từ chối dữ liệu không hợp lệ.**

```cpp
            if (showtime->getStatus() == ShowtimeStatus::SCHEDULED &&
                DateTime::isBefore(now, showtime->getStartAt()))
            {
                return false;
            }
        }
    }
    // Tao doi tuong tam voi trang thai moi
    Movie updated(
        movie->getMovieId(),
```

`false` báo thao tác không thành công. `DateTime` cung cấp thời gian hoặc phép so sánh thời gian; điều kiện này quyết định dữ liệu có còn hợp lệ ở thời điểm gọi hàm hay không. Nhánh `false` dừng hàm tại đây; những câu lệnh phía sau không chạy, nên không có cập nhật nào từ các bước sau.

**Bước 5 — Cập nhật qua Repository.**

```cpp
        movie->getTitle(),
        movie->getGenre(),
        movie->getDurationMinutes(),
        movie->getMinAge(),
        movie->getReleaseDate(),
        movie->getEndDate(),
        active
    );

    return movieRepo.update(updated);
```

Repository thực hiện thao tác dữ liệu. `update()` chuyển việc thay đổi dữ liệu cho Repository; kết quả trả về phản ánh kiểm tra và thao tác cập nhật ở tầng dữ liệu.

**Bước 6 — Chuẩn bị và xử lý dữ liệu.**

```cpp
}
```

Các câu lệnh được thực thi theo đúng thứ tự trong `.cpp`; điều kiện và nhánh trả về nằm ngay trong code. Giá trị được khai báo ở đây được dùng trong các nhánh tiếp theo; hãy theo dõi nơi nó được trả về hoặc chuyển vào Repository để hiểu tác động lên trạng thái.

### getNowShowingMovies()

#### A. Tổng quan

- **Mục đích:** Kiểm tra quyền. Xem các nhánh cụ thể trong code.
- **Tham số:** `không có`.
- **Giá trị trả về:** `vector<const Movie*>`; các nhánh `return` được ghi đầy đủ bên dưới.
- **Điều kiện quan trọng:** Kiểm tra quyền qua `AuthService`; kiểm tra trạng thái nạp và con trỏ trước khi dùng.

#### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Kiểm tra quyền
    |
    v
Tra cứu dữ liệu
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
vector<const Movie*> MovieService::getNowShowingMovies()
{
    authService.requirePermission("MOVIE_VIEW");

    vector<const Movie*> result;
    if (!movieRepo.isLoaded())
    {
        return result;
    }
    string today = DateTime::getCurrentDateTime().substr(0, 10);
```

`AuthService` kiểm tra quyền và có thể ném ngoại lệ. Quyền `MOVIE_VIEW` được kiểm tra trước bước xử lý kế tiếp; thiếu quyền sẽ ném ngoại lệ, vì vậy caller không nên xem đó là kết quả `false`. Kiểm tra `isLoaded()` tránh dùng dữ liệu Repository chưa được nạp; nhánh không đạt kết thúc ngay theo giá trị `return` trong code. `DateTime` cung cấp thời gian hoặc phép so sánh thời gian; điều kiện này quyết định dữ liệu có còn hợp lệ ở thời điểm gọi hàm hay không.

**Bước 2 — Tra cứu dữ liệu.**

```cpp
    for (int i = 0; i < movieRepo.count(); i++)
    {
        const Movie* movie = movieRepo.getAt(i);
        if (movie == nullptr)
        {
            continue;
        }
        if (movie->isActive() &&
            movie->getReleaseDate() <= today &&
            today <= movie->getEndDate())
```

`nullptr` cho biết không có đối tượng để truy cập. Vòng `for` duyệt lần lượt các phần tử. Vòng lặp xử lý từng phần tử theo thứ tự; điều kiện bên trong quyết định phần tử được giữ, bỏ qua hay đưa vào kết quả.

**Bước 3 — Trả kết quả.**

```cpp
        {
            result.push_back(movie);
        }
    }
    return result;
}
```

Các câu lệnh được thực thi theo đúng thứ tự trong `.cpp`; điều kiện và nhánh trả về nằm ngay trong code. Giá trị được khai báo ở đây được dùng trong các nhánh tiếp theo; hãy theo dõi nơi nó được trả về hoặc chuyển vào Repository để hiểu tác động lên trạng thái.

### getComingSoonMovies()

#### A. Tổng quan

- **Mục đích:** Kiểm tra quyền. Xem các nhánh cụ thể trong code.
- **Tham số:** `không có`.
- **Giá trị trả về:** `vector<const Movie*>`; các nhánh `return` được ghi đầy đủ bên dưới.
- **Điều kiện quan trọng:** Kiểm tra quyền qua `AuthService`; kiểm tra trạng thái nạp và con trỏ trước khi dùng.

#### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Kiểm tra quyền
    |
    v
Tra cứu dữ liệu
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
vector<const Movie*> MovieService::getComingSoonMovies()
{
    authService.requirePermission("MOVIE_VIEW");

    vector<const Movie*> result;
    if (!movieRepo.isLoaded())
    {
        return result;
    }
    string today = DateTime::getCurrentDateTime().substr(0, 10);
```

`AuthService` kiểm tra quyền và có thể ném ngoại lệ. Quyền `MOVIE_VIEW` được kiểm tra trước bước xử lý kế tiếp; thiếu quyền sẽ ném ngoại lệ, vì vậy caller không nên xem đó là kết quả `false`. Kiểm tra `isLoaded()` tránh dùng dữ liệu Repository chưa được nạp; nhánh không đạt kết thúc ngay theo giá trị `return` trong code. `DateTime` cung cấp thời gian hoặc phép so sánh thời gian; điều kiện này quyết định dữ liệu có còn hợp lệ ở thời điểm gọi hàm hay không.

**Bước 2 — Tra cứu dữ liệu.**

```cpp
    for (int i = 0; i < movieRepo.count(); i++)
    {
        const Movie* movie = movieRepo.getAt(i);
        if (movie == nullptr)
        {
            continue;
        }
        if (movie->isActive() &&
            today < movie->getReleaseDate())
        {
```

`nullptr` cho biết không có đối tượng để truy cập. Vòng `for` duyệt lần lượt các phần tử. Vòng lặp xử lý từng phần tử theo thứ tự; điều kiện bên trong quyết định phần tử được giữ, bỏ qua hay đưa vào kết quả.

**Bước 3 — Trả kết quả.**

```cpp
            result.push_back(movie);
        }
    }
    return result;
}
```

Các câu lệnh được thực thi theo đúng thứ tự trong `.cpp`; điều kiện và nhánh trả về nằm ngay trong code. Giá trị được khai báo ở đây được dùng trong các nhánh tiếp theo; hãy theo dõi nơi nó được trả về hoặc chuyển vào Repository để hiểu tác động lên trạng thái.

### searchMoviesByTitle()

#### A. Tổng quan

- **Mục đích:** Kiểm tra quyền. Xem các nhánh cụ thể trong code.
- **Tham số:** `const string& keyword`.
- **Giá trị trả về:** `vector<const Movie*>`; các nhánh `return` được ghi đầy đủ bên dưới.
- **Điều kiện quan trọng:** Kiểm tra quyền qua `AuthService`; kiểm tra trạng thái nạp và con trỏ trước khi dùng.

#### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Kiểm tra quyền
    |
    v
Duyệt từng phần tử
    |
    v
Tra cứu dữ liệu
    |
    v
Trả kết quả
    |
    v
Chuẩn bị và xử lý dữ liệu
    |
    v
Kết thúc
```

#### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1 — Kiểm tra quyền.**

```cpp
vector<const Movie*> MovieService::searchMoviesByTitle(const string& keyword)
{
    authService.requirePermission("MOVIE_VIEW");

    vector<const Movie*> result;
    if (!movieRepo.isLoaded())
    {
        return result;
    }
    // Chuyen tu khoa thanh chu thuong
```

`AuthService` kiểm tra quyền và có thể ném ngoại lệ. Quyền `MOVIE_VIEW` được kiểm tra trước bước xử lý kế tiếp; thiếu quyền sẽ ném ngoại lệ, vì vậy caller không nên xem đó là kết quả `false`. Kiểm tra `isLoaded()` tránh dùng dữ liệu Repository chưa được nạp; nhánh không đạt kết thúc ngay theo giá trị `return` trong code.

**Bước 2 — Duyệt từng phần tử.**

```cpp
    string searchKey = keyword;
    for (char& c : searchKey)
    {
        c = static_cast<char>(
            tolower(static_cast<unsigned char>(c))
        );
    }
    // Duyet danh sach phim
    for (int i = 0; i < movieRepo.count(); i++)
    {
```

Vòng `for` duyệt lần lượt các phần tử. `static_cast` chuyển kiểu tường minh. Vòng lặp xử lý từng phần tử theo thứ tự; điều kiện bên trong quyết định phần tử được giữ, bỏ qua hay đưa vào kết quả. `static_cast` nêu rõ kiểu đích khi chuyển giá trị, giúp tránh chuyển kiểu ngầm khó nhận ra.

**Bước 3 — Tra cứu dữ liệu.**

```cpp
        const Movie* movie = movieRepo.getAt(i);
        if (movie == nullptr)
        {
            continue;
        }
        string title = movie->getTitle();
        // Chuyen ten phim thanh chu thuong
        for (char& c : title)
        {
            c = static_cast<char>(
```

`nullptr` cho biết không có đối tượng để truy cập. Vòng `for` duyệt lần lượt các phần tử. `static_cast` chuyển kiểu tường minh. Vòng lặp xử lý từng phần tử theo thứ tự; điều kiện bên trong quyết định phần tử được giữ, bỏ qua hay đưa vào kết quả. `static_cast` nêu rõ kiểu đích khi chuyển giá trị, giúp tránh chuyển kiểu ngầm khó nhận ra.

**Bước 4 — Trả kết quả.**

```cpp
                tolower(static_cast<unsigned char>(c))
            );
        }
        // Tim tu khoa trong ten phim
        if (title.find(searchKey) != string::npos)
        {
            result.push_back(movie);
        }
    }
    return result;
```

`static_cast` chuyển kiểu tường minh. `static_cast` nêu rõ kiểu đích khi chuyển giá trị, giúp tránh chuyển kiểu ngầm khó nhận ra.

**Bước 5 — Chuẩn bị và xử lý dữ liệu.**

```cpp
}
```

Các câu lệnh được thực thi theo đúng thứ tự trong `.cpp`; điều kiện và nhánh trả về nằm ngay trong code. Giá trị được khai báo ở đây được dùng trong các nhánh tiếp theo; hãy theo dõi nơi nó được trả về hoặc chuyển vào Repository để hiểu tác động lên trạng thái.

## 9. Nguyên tắc xử lý kết quả

| Kết quả | Ý nghĩa |
|---|---|
| `true` | Nghiệp vụ thành công |
| `false` | Không thỏa điều kiện hoặc Repository từ chối |
| `nullptr` | Không tìm thấy đối tượng |
| Vector rỗng | Không có kết quả phù hợp |
| `runtime_error` | Không có quyền hoặc lỗi hệ thống |
| Ngoại lệ khác | Được truyền lên tầng gọi theo quy ước hiện có |

MovieService không tự bắt rồi bỏ qua ngoại lệ của Repository.

Các con trỏ trả về thuộc quyền quản lý của Repository. GUI không được `delete` hoặc lưu giữ để sử dụng sau khi dữ liệu đã bị xóa hoặc nạp lại.

## 10. Kiểm thử

### 10.1. Phân quyền

- Admin có quyền thêm, sửa, xóa phim.
- Staff có quyền xem phim.
- Staff không có quyền quản lý phim theo bảng quyền mặc định.
- Chưa đăng nhập bị từ chối.
- Thiếu permission bị từ chối kể cả với Admin.

### 10.2. Cập nhật phim

- Sửa tên và thể loại thành công.
- Sửa thời lượng khi chưa có Showtime thành công.
- Sửa thời lượng khi đã có Showtime thất bại.
- Sửa ngày phát hành khi đã có Showtime thất bại.
- Không cho thay đổi `active` qua `updateMovie()`.
- Không cho sửa phim không tồn tại.

### 10.3. Bật/tắt phim

- Ngừng hoạt động phim chưa có Showtime.
- Ngừng hoạt động phim chỉ có suất đã kết thúc.
- Ngừng hoạt động phim chỉ có suất đã hủy.
- Từ chối ngừng hoạt động khi còn suất tương lai `SCHEDULED`.
- Kích hoạt lại phim.
- Gọi lại cùng trạng thái không làm thay đổi dữ liệu.
- Kiểm tra từ chối khi thiếu quyền.

### 10.4. Lọc phim

- Phim đang chiếu.
- Phim sắp chiếu.
- Phim đã hết khoảng phát hành.
- Phim đã ngừng hoạt động.
- Ngày hiện tại trùng ngày bắt đầu.
- Ngày hiện tại trùng ngày kết thúc.
- Danh sách không có kết quả.

### 10.5. Tìm kiếm

- Tìm đúng tên.
- Tìm theo một phần tên.
- Tìm không phân biệt chữ hoa/thường ASCII.
- Tìm từ khóa không tồn tại.
- Từ khóa rỗng.
- Danh sách phim rỗng.
- Tìm phim đã ngừng hoạt động.

## 11. Quy ước triển khai

1. Giữ nguyên các API hiện có để tránh ảnh hưởng GUI.
2. Bổ sung bốn API mới theo thiết kế.
3. Không đưa kiểm tra permission xuống Repository.
4. Không để GUI tự kiểm tra các quy tắc nghiệp vụ thay Service.
5. Không sửa trực tiếp dữ liệu Movie thông qua con trỏ trả về.
6. Không tự ý thay đổi cơ chế load/save hiện tại.
7. Không bổ sung thư viện phức tạp nếu thư viện chuẩn đơn giản đã đáp ứng.
8. Viết kiểm thử trước khi kết luận hoàn thiện.
9. Đồng bộ `Business_UI_Contract.md` sau khi code và test đạt.

## 12. Trạng thái

**Thiết kế:** Đã chốt phương án A.

**Triển khai:** Chưa code các chức năng mới.

**Kiểm thử:** Chưa thực hiện cho phiên bản hoàn thiện.

**Bước tiếp theo:** Cập nhật `Movie.h` nếu cần API duyệt Showtime, sau đó triển khai `MovieService.h`, `MovieService.cpp` và kiểm thử.
