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

## 4. Constructor

```cpp
MovieService(MovieRepository& movieRepo,
             AuthService& authService);
```

Sử dụng danh sách khởi tạo:

```cpp
MovieService::MovieService(MovieRepository& movieRepo,
                           AuthService& authService)
    : movieRepo(movieRepo),
      authService(authService)
{}
```

Constructor không tự nạp dữ liệu và không tự tạo Repository.

## 5. Quản lý dữ liệu

### 5.1. load()

```cpp
bool load();
```

**Chức năng:** Nạp dữ liệu phim từ file TXT vào Repository.

Gọi:

```cpp
return movieRepo.load();
```

Việc kiểm tra định dạng, chuyển đổi dữ liệu và xử lý lỗi thuộc trách nhiệm Repository.

**Kết quả:**

- `true`: Nạp thành công.
- `false`: Repository trả về thất bại.
- Ngoại lệ: Được truyền lên tầng gọi nếu Repository phát sinh lỗi.

Không gọi `load()` tùy tiện trong lúc các đối tượng khác đang sử dụng con trỏ Movie, vì việc nạp lại có thể làm mất hiệu lực các con trỏ cũ.

### 5.2. save()

```cpp
bool save();
```

**Chức năng:** Lưu dữ liệu phim hiện tại xuống file TXT.

```cpp
return movieRepo.save();
```

MovieService không tự xử lý định dạng TXT.

## 6. Quản lý phim

### 6.1. addMovie()

```cpp
bool addMovie(Movie* movie);
```

**Chức năng:** Thêm phim mới.

Quy trình:

1. Kiểm tra permission `MOVIE_ADD`.
2. Gọi `movieRepo.add(movie)`.
3. Repository kiểm tra trạng thái đã nạp, con trỏ, mã phim, dữ liệu và mã trùng.
4. Trả về kết quả.

```cpp
authService.requirePermission("MOVIE_ADD");
return movieRepo.add(movie);
```

**Quy tắc:**

- Mã phim phải hợp lệ và duy nhất.
- Tên và thể loại phải hợp lệ.
- Thời lượng phải lớn hơn 0.
- Độ tuổi tối thiểu không âm.
- Ngày phát hành không lớn hơn ngày kết thúc.

Repository chịu trách nhiệm kiểm tra các điều kiện dữ liệu trên.

Sau khi thêm thành công, Repository sở hữu đối tượng Movie. Nếu thất bại, bên gọi vẫn chịu trách nhiệm quản lý con trỏ.

### 6.2. updateMovie()

```cpp
bool updateMovie(const Movie& newValues);
```

**Chức năng:** Cập nhật thông tin phim mà không làm ảnh hưởng các suất chiếu đã tạo.

#### Quy trình

**Bước 1: Kiểm tra quyền**

```cpp
authService.requirePermission("MOVIE_UPDATE");
```

**Bước 2: Kiểm tra Repository**

```cpp
if (!movieRepo.isLoaded())
{
    return false;
}
```

**Bước 3: Tìm phim hiện tại**

```cpp
Movie* movie = movieRepo.findById(newValues.getMovieId());

if (movie == nullptr)
{
    return false;
}
```

**Bước 4: Không cho thay đổi trạng thái active**

```cpp
if (movie->isActive() != newValues.isActive())
{
    return false;
}
```

Trạng thái hoạt động chỉ được thay đổi thông qua `setMovieActive()`.

**Bước 5: Kiểm tra thay đổi ngày**

```cpp
bool dateChanged =
    movie->getReleaseDate() != newValues.getReleaseDate() ||
    movie->getEndDate() != newValues.getEndDate();
```

**Bước 6: Kiểm tra thay đổi thời lượng**

```cpp
bool durationChanged =
    movie->getDurationMinutes() != newValues.getDurationMinutes();
```

**Bước 7: Kiểm tra các suất chiếu**

```cpp
if (movie->getShowtimeCount() > 0 &&
    (dateChanged || durationChanged))
{
    return false;
}
```

Khi phim đã có ít nhất một Showtime, không cho phép thay đổi:

- `releaseDate`
- `endDate`
- `durationMinutes`

Quy tắc áp dụng cả với suất chiếu đã hủy hoặc đã kết thúc.

**Lý do:** Showtime lưu riêng thời điểm bắt đầu và kết thúc. Nếu thay đổi thời lượng phim sau khi đã tạo lịch, thông tin phim có thể không còn phù hợp với dữ liệu lịch sử.

**Bước 8: Cập nhật**

```cpp
return movieRepo.update(newValues);
```

Repository tiếp tục kiểm tra dữ liệu trước khi cập nhật.

#### Các thuộc tính được phép sửa

| Thuộc tính | Chưa có Showtime | Đã có Showtime |
|---|---|---|
| `movieId` | Không | Không |
| `title` | Có | Có |
| `genre` | Có | Có |
| `durationMinutes` | Có | Không |
| `minAge` | Có | Có |
| `releaseDate` | Có | Không |
| `endDate` | Có | Không |
| `active` | Không | Không |

Việc cho phép sửa `minAge` của phim đã có Showtime là quy tắc hiện tại; tuổi khách vẫn được nhân viên xác nhận tại thời điểm bán vé.

### 6.3. removeMovie()

```cpp
bool removeMovie(const string& movieId);
```

**Chức năng:** Xóa phim khỏi hệ thống.

Quy trình:

1. Kiểm tra `MOVIE_DELETE`.
2. Gọi `movieRepo.removeById(movieId)`.
3. Repository từ chối nếu phim còn liên kết Showtime.

```cpp
authService.requirePermission("MOVIE_DELETE");
return movieRepo.removeById(movieId);
```

Không được xóa phim nếu còn bất kỳ suất chiếu nào, kể cả suất đã hủy hoặc đã kết thúc.

Không tự động xóa Showtime khi xóa phim.

### 6.4. setMovieActive()

```cpp
bool setMovieActive(const string& movieId,
                    bool active);
```

**Chức năng:** Kích hoạt hoặc ngừng hoạt động một phim.

#### Quy trình

1. Kiểm tra `MOVIE_UPDATE`.
2. Kiểm tra Repository đã nạp.
3. Tìm phim theo `movieId`.
4. Nếu trạng thái hiện tại bằng trạng thái yêu cầu, trả về `true` mà không thay đổi dữ liệu.
5. Nếu yêu cầu ngừng hoạt động, kiểm tra các Showtime liên kết.
6. Từ chối nếu còn suất `SCHEDULED` có thời điểm bắt đầu trong tương lai.
7. Tạo đối tượng Movie tạm với các thuộc tính hiện tại và trạng thái mới.
8. Gọi `movieRepo.update()` để cập nhật.

#### Quy tắc

- `active = true`: Kích hoạt phim.
- `active = false`: Ngừng hoạt động phim.
- Không cho ngừng hoạt động nếu phim còn suất tương lai chưa hủy.
- Không xóa hoặc thay đổi các Showtime cũ.
- Không thay đổi dữ liệu vé và hóa đơn.

Khi kiểm tra các Showtime, sử dụng liên kết có sẵn trong Model Movie thay vì bắt buộc bổ sung `ShowtimeRepository` vào constructor.

Cần lưu ý `Movie` hiện chỉ cung cấp `findShowtime(showtimeId)` và `getShowtimeCount()`, chưa có API duyệt Showtime theo chỉ số. Vì vậy, trước khi code phải bổ sung một cách duyệt an toàn, chỉ đọc, chẳng hạn:

```cpp
const Showtime* getShowtimeAt(int index) const;
```

Không truy cập trực tiếp danh sách liên kết private từ Service.

#### Kết quả

- `true`: Trạng thái đã đúng hoặc cập nhật thành công.
- `false`: Phim không tồn tại, dữ liệu chưa nạp hoặc không thỏa điều kiện nghiệp vụ.
- Ngoại lệ: Không có quyền hoặc lỗi được Repository truyền lên.

## 7. Tra cứu phim

### 7.1. findMovieById()

```cpp
const Movie* findMovieById(const string& movieId);
```

Yêu cầu `MOVIE_VIEW`.

Gọi `movieRepo.findById(movieId)`.

Trả về `nullptr` nếu không tìm thấy.

### 7.2. getMovieAt()

```cpp
const Movie* getMovieAt(int index);
```

Yêu cầu `MOVIE_VIEW`.

Trả về phim tại vị trí `index` trong Repository.

Nếu vị trí không hợp lệ, trả về `nullptr` theo quy ước Repository.

### 7.3. getMovieCount()

```cpp
int getMovieCount();
```

Yêu cầu `MOVIE_VIEW`.

Trả về tổng số phim trong Repository, bao gồm phim đang hoạt động và đã ngừng hoạt động.

## 8. Lọc và tìm kiếm phim

Các hàm mới sử dụng:

```cpp
#include <vector>
```

`vector` là mảng động của thư viện chuẩn C++, cho phép lưu danh sách kết quả có số lượng thay đổi.

Sử dụng `vector<const Movie*>` để tránh sao chép đối tượng Movie và không cho phép sửa trực tiếp thông qua kết quả trả về.

### 8.1. getNowShowingMovies()

```cpp
vector<const Movie*> getNowShowingMovies();
```

**Chức năng:** Lấy các phim đang trong khoảng ngày phát hành.

Điều kiện:

```text
active == true
releaseDate <= today
today <= endDate
```

Trong đó `today` là ngày hiện tại lấy từ `DateTime`.

Quy trình:

1. Kiểm tra `MOVIE_VIEW`.
2. Kiểm tra dữ liệu đã nạp.
3. Lấy ngày hiện tại.
4. Duyệt toàn bộ Movie.
5. Thêm phim thỏa điều kiện vào vector.
6. Trả về danh sách.

Không yêu cầu phim phải có Showtime thực tế.

Không thay đổi thứ tự các phim trong Repository.

Nếu không có phim phù hợp, trả về vector rỗng.

### 8.2. getComingSoonMovies()

```cpp
vector<const Movie*> getComingSoonMovies();
```

**Chức năng:** Lấy các phim sắp chiếu.

Điều kiện:

```text
active == true
today < releaseDate
```

Quy trình tương tự `getNowShowingMovies()`.

Phim đã ngừng hoạt động không được đưa vào kết quả.

Nếu không có phim phù hợp, trả về vector rỗng.

### 8.3. searchMoviesByTitle()

```cpp
vector<const Movie*> searchMoviesByTitle(
    const string& keyword
);
```

**Chức năng:** Tìm kiếm phim có tên chứa từ khóa.

#### Quy tắc

- Yêu cầu `MOVIE_VIEW`.
- Tìm kiếm theo chuỗi con trong `title`.
- Không phân biệt chữ hoa/chữ thường đối với ký tự ASCII.
- Không tự động bỏ dấu tiếng Việt.
- Tìm trên toàn bộ danh sách, bao gồm phim đã ngừng hoạt động.
- Từ khóa rỗng trả về toàn bộ phim.
- Giữ nguyên thứ tự Repository.

Ví dụ:

```text
Tên phim: Avengers Endgame

Từ khóa "avengers" -> Tìm thấy
Từ khóa "ENDGAME"  -> Tìm thấy
Từ khóa "ven"      -> Tìm thấy
Từ khóa "batman"   -> Không tìm thấy
```

Có thể sử dụng `std::transform()` và `std::tolower()` để chuẩn hóa ký tự ASCII trước khi so sánh.

Khi sử dụng `std::tolower()`, cần chuyển ký tự sang `unsigned char` trước để tránh hành vi không xác định với giá trị `char` âm.

Không cần bổ sung chỉ mục tìm kiếm hoặc thay đổi MovieRepository trong phiên bản này.

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