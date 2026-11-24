# MovieRepository

## 1. Tổng quan

`MovieRepository` là lớp thuộc tầng Data Access, chịu trách nhiệm quản lý dữ liệu phim trong hệ thống PBL2.

Lớp kế thừa `IRepository<Movie>` để triển khai hợp đồng chung của Repository, đồng thời sử dụng `Repository<Movie>` thông qua composition để quản lý danh sách đối tượng và đọc ghi file TXT.

MovieRepository chịu trách nhiệm:

- Kiểm tra tính hợp lệ của dữ liệu phim.
- Chuyển đổi dữ liệu giữa đối tượng Movie và chuỗi TXT.
- Thêm, cập nhật, xóa và truy vấn phim.
- Bảo đảm tính toàn vẹn dữ liệu giữa Movie và Showtime.
- Triển khai đầy đủ các hàm của `IRepository<Movie>`.

MovieRepository không trực tiếp xử lý phân quyền hoặc các nghiệp vụ thuộc tầng Business Logic.

**File liên quan:**

- `src/data_access/IRepository.h`
- `src/data_access/Repository.h`
- `src/data_access/MovieRepository.h`
- `src/data_access/MovieRepository.cpp`
- `src/models/Movie.h`
- `src/utils/Validator.h`

**Trạng thái:** CHỐT MARKDOWN; đã triển khai đồng bộ quy tắc validation trong `MovieRepository.cpp`. Khối `.cpp` ở mục 5 khớp mã nguồn hiện tại; `tests/test_irepository.cpp` đã biên dịch và chạy lại thành công (81 PASS, 0 FAIL).

## 2. Kiến trúc

### 2.1. Quan hệ giữa các lớp

```text
MovieService
     |
     | Dependency Injection
     v
IRepository<Movie>
     ^
     | Public Inheritance
     |
MovieRepository
     |
     | Composition
     v
Repository<Movie>
     |
     v
DoublyLinkedList<Movie*>
```

### 2.2. Quan hệ với IRepository

MovieRepository kế thừa công khai từ `IRepository<Movie>`.

```cpp
class MovieRepository : public IRepository<Movie>
```

MovieRepository phải triển khai đầy đủ 9 hàm thuần ảo được khai báo trong interface.

Các hàm ghi đè được khai báo bằng từ khóa `override`.

Việc kế thừa interface giúp MovieService có thể phụ thuộc vào `IRepository<Movie>&` thay vì phụ thuộc trực tiếp vào `MovieRepository&`.

### 2.3. Quan hệ với Repository

MovieRepository chứa một đối tượng `Repository<Movie>`.

```cpp
Repository<Movie> repo;
```

Đây là quan hệ composition.

`Repository<Movie>` chịu trách nhiệm:

- Quản lý danh sách phim.
- Quản lý bộ nhớ của các đối tượng Movie.
- Đọc và ghi file TXT.
- Thêm, xóa và tìm kiếm đối tượng.
- Quản lý trạng thái nạp dữ liệu.

MovieRepository bổ sung các logic riêng của Movie mà Repository chung không thể xử lý.

Theo `docs/architecture/validation_rules.md`, `Repository<Movie>` kiểm tra trạng thái loaded và điều kiện lưu trữ chung; MovieRepository kiểm tra thuộc tính Movie và liên kết Showtime khi cần. Không loại bỏ kiểm tra tại thời điểm khác nếu cần bảo vệ dữ liệu hoặc bộ nhớ.

## 3. Định dạng dữ liệu

Dữ liệu phim được lưu dưới dạng TXT.

Mỗi dòng chứa một phim với 8 trường, phân cách bằng ký tự `|`.

```text
movieId|title|genre|durationMinutes|minAge|releaseDate|endDate|active
```

| STT | Trường | Kiểu dữ liệu | Ý nghĩa |
|---|---|---|---|
| 1 | movieId | string | Mã phim |
| 2 | title | string | Tên phim |
| 3 | genre | string | Thể loại |
| 4 | durationMinutes | int | Thời lượng phim |
| 5 | minAge | int | Độ tuổi tối thiểu |
| 6 | releaseDate | string | Ngày bắt đầu chiếu |
| 7 | endDate | string | Ngày kết thúc chiếu |
| 8 | active | bool | Trạng thái hoạt động |

Ví dụ:

```text
MOV0001|Interstellar|Science Fiction|169|13|2026-11-01|2026-12-01|1
```

Quy định:

- Mã phim sử dụng tiền tố `MOV`.
- Thời lượng phim phải lớn hơn 0.
- Độ tuổi tối thiểu không được âm.
- Ngày bắt đầu và ngày kết thúc phải hợp lệ.
- Ngày bắt đầu không được lớn hơn ngày kết thúc.
- Trạng thái `active` được lưu dưới dạng `0` hoặc `1`.
- Các trường văn bản phải đáp ứng quy định của Validator.

Quá trình refactor không thay đổi định dạng dữ liệu hiện tại.

## 4. Thiết kế lớp

### 4.1. Thuộc tính

| Thuộc tính | Kiểu dữ liệu | Phạm vi | Chức năng |
|---|---|---|---|
| repo | Repository<Movie> | private | Quản lý dữ liệu phim |

MovieRepository không khai báo lại danh sách phim, đường dẫn file hoặc trạng thái nạp dữ liệu vì các thuộc tính này đã được quản lý bởi `Repository<Movie>`.

### 4.2. Các hàm private

| Hàm | Kiểu trả về | Chức năng |
|---|---|---|
| getMovieId(Movie* movie) | string | Lấy mã phim |
| stringToMovie(const string& line) | Movie* | Chuyển chuỗi TXT thành Movie |
| movieToString(Movie* movie) | string | Chuyển Movie thành chuỗi TXT |
| isValidMovie(const Movie* movie) | bool | Kiểm tra tính hợp lệ của phim |

Các hàm trên được khai báo `static` vì không cần truy cập trực tiếp vào thuộc tính của MovieRepository.

### 4.3. Các hàm public

| Hàm | Kiểu trả về | Chức năng | Override |
|---|---|---|---|
| MovieRepository(const string& filePath) | Không có | Constructor | Không |
| load() | bool | Nạp dữ liệu | Có |
| save() | bool | Lưu dữ liệu | Có |
| add(Movie* movie) | bool | Thêm phim | Có |
| update(const Movie& newValues) | bool | Cập nhật phim | Có |
| removeById(const string& movieId) | bool | Xóa phim | Có |
| findById(const string& movieId) | Movie* | Tìm phim theo mã | Có |
| getAt(int index) | Movie* | Lấy phim theo vị trí | Có |
| count() | int | Đếm số lượng phim | Có |
| isLoaded() | bool | Kiểm tra trạng thái nạp | Có |

## 5. Mã nguồn

### 5.1. MovieRepository.h

**Đường dẫn:** `src/data_access/MovieRepository.h`

```cpp
#ifndef MOVIE_REPOSITORY_H
#define MOVIE_REPOSITORY_H

#include <string>
#include "IRepository.h"
#include "Repository.h"
#include "../models/Movie.h"

using namespace std;

class MovieRepository : public IRepository<Movie>
{
private:
    Repository<Movie> repo;

    // Cac ham cung cap cho Repository<Movie>
    static string getMovieId(Movie* movie);
    static Movie* stringToMovie(const string& line);
    static string movieToString(Movie* movie);

    // Kiem tra du lieu Movie
    static bool isValidMovie(const Movie* movie);

public:
    MovieRepository(const string& filePath);

    // Doc va ghi du lieu
    bool load() override;
    bool save() override;

    // Quan ly phim
    bool add(Movie* movie) override;
    bool update(const Movie& newValues) override;
    bool removeById(const string& movieId) override;

    // Truy van phim
    Movie* findById(const string& movieId) override;
    Movie* getAt(int index) override;
    int count() override;
    bool isLoaded() override;
};

#endif
```

### 5.2. MovieRepository.cpp

**Đường dẫn:** `src/data_access/MovieRepository.cpp`

Giữ nguyên logic triển khai hiện tại vì chữ ký của các hàm đã tương thích với `IRepository<Movie>`.

```cpp
#include "MovieRepository.h"
#include "../utils/Validator.h"
#include <stdexcept>
#include <vector>

using namespace std;

// Kiem tra toan bo du lieu Movie

bool MovieRepository::isValidMovie(const Movie* movie)
{
    if (movie == nullptr)
    {
        return false;
    }
    if (!Validator::isValidId(movie->getMovieId(), "MOV"))
    {
        return false;
    }
    if (!Validator::isValidText(movie->getTitle()) ||
        !Validator::isValidText(movie->getGenre()))
    {
        return false;
    }
    if (movie->getDurationMinutes() <= 0)
    {
        return false;
    }
    if (movie->getMinAge() < 0)
    {
        return false;
    }
    if (!Validator::isValidDate(movie->getReleaseDate()) ||
        !Validator::isValidDate(movie->getEndDate()))
    {
        return false;
    }
    if (movie->getReleaseDate() > movie->getEndDate())
    {
        return false;
    }
    return true;
}

MovieRepository::MovieRepository(const string& filePath)
    : repo(filePath, getMovieId, stringToMovie, movieToString)
{
}

string MovieRepository::getMovieId(Movie* movie)
{
    if (movie == nullptr)
    {
        return "";
    }
    return movie->getMovieId();
}

Movie* MovieRepository::stringToMovie(const string& line)
{
    vector<string> fields;
    size_t start = 0;

    // Tach cac truong du lieu theo dau |
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
        size_t pos2 = 0;

        durationMinutes = stoi(fields[3], &pos1);
        minAge = stoi(fields[4], &pos2);

        if (pos1 != fields[3].length() ||
            pos2 != fields[4].length())
        {
            throw runtime_error("Sai dinh dang so");
        }
    }
    catch (const exception&)
    {
        throw runtime_error(
            "Thoi luong hoac do tuoi khong hop le"
        );
    }

    // Kiem tra trang thai active
    if (fields[7] != "0" && fields[7] != "1")
    {
        throw runtime_error("Trang thai Movie khong hop le");
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

    // Kiem tra du lieu Movie
    if (!isValidMovie(movie))
    {
        delete movie;
        throw runtime_error("Du lieu Movie khong hop le");
    }

    return movie;
}

// Chuyen Movie thanh mot dong TXT
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
}

// Doc du lieu phim tu file
bool MovieRepository::load()
{
    return repo.load();
}

// Luu du lieu phim xuong file
bool MovieRepository::save()
{
    return repo.save();
}

// Them phim moi
bool MovieRepository::add(Movie* movie)
{
    if (!isValidMovie(movie))
    {
        return false;
    }

    return repo.add(movie);
}

// Cap nhat thong tin phim
bool MovieRepository::update(const Movie& newValues)
{
    if (!repo.isLoaded() || !isValidMovie(&newValues))
    {
        return false;
    }

    Movie* movie = repo.findById(newValues.getMovieId());

    if (movie == nullptr)
    {
        return false;
    }

    movie->setTitle(newValues.getTitle());
    movie->setGenre(newValues.getGenre());
    movie->setDurationMinutes(newValues.getDurationMinutes());
    movie->setMinAge(newValues.getMinAge());
    movie->setReleaseDate(newValues.getReleaseDate());
    movie->setEndDate(newValues.getEndDate());
    movie->setActive(newValues.isActive());

    return true;
}

bool MovieRepository::removeById(const string& movieId)
{
    Movie* movie = repo.findById(movieId);

    if (movie == nullptr)
    {
        return false;
    }

    // Khong xoa neu phim con lien ket Showtime
    if (movie->getShowtimeCount() > 0)
    {
        return false;
    }

    return repo.removeById(movieId);
}

Movie* MovieRepository::findById(const string& movieId)
{
    return repo.findById(movieId);
}

Movie* MovieRepository::getAt(int index)
{
    return repo.getAt(index);
}

int MovieRepository::count()
{
    return repo.count();
}

bool MovieRepository::isLoaded()
{
    return repo.isLoaded();
}
```

## 6. Giải thích các thư viện và từ khóa

### 6.1. Thư viện string

```cpp
#include <string>
```

Cung cấp kiểu dữ liệu `string` và các hàm xử lý chuỗi.

Các hàm được sử dụng:

- `find()`: Tìm vị trí xuất hiện của ký tự.
- `substr()`: Lấy một phần của chuỗi.
- `length()`: Lấy độ dài chuỗi.
- `stoi()`: Chuyển chuỗi thành số nguyên.
- `to_string()`: Chuyển số thành chuỗi.

### 6.2. Thư viện vector

```cpp
#include <vector>
```

`vector` là cấu trúc dữ liệu có khả năng tự thay đổi kích thước.

Trong MovieRepository, vector được sử dụng để lưu các trường dữ liệu sau khi tách một dòng TXT.

```cpp
vector<string> fields;
```

Các hàm được sử dụng:

- `push_back()`: Thêm phần tử vào cuối vector.
- `size()`: Lấy số lượng phần tử.
- `fields[index]`: Truy cập phần tử theo vị trí.

Vector chỉ được sử dụng tạm thời trong quá trình chuyển đổi dữ liệu.

Danh sách phim chính vẫn được quản lý bằng DoublyLinkedList thông qua Repository.

### 6.3. Thư viện stdexcept

```cpp
#include <stdexcept>
```

Cung cấp các lớp exception tiêu chuẩn của C++.

MovieRepository sử dụng `runtime_error` để thông báo lỗi khi dữ liệu TXT không hợp lệ.

Ví dụ:

```cpp
throw runtime_error("Sai so luong truong Movie");
```

Exception được truyền về lớp gọi để xử lý.

### 6.4. Từ khóa static

```cpp
static string getMovieId(Movie* movie);
```

Hàm thành viên static thuộc về lớp thay vì thuộc về một đối tượng cụ thể.

Hàm static không có con trỏ `this` và không thể trực tiếp truy cập các thuộc tính không static của lớp.

Các hàm `getMovieId()`, `stringToMovie()` và `movieToString()` được khai báo static để có thể truyền vào Repository dưới dạng con trỏ hàm thông thường.

Hàm `isValidMovie()` cũng được khai báo static vì chỉ kiểm tra đối tượng Movie được truyền vào.

### 6.5. Từ khóa override

```cpp
bool load() override;
```

Từ khóa `override` xác nhận rằng hàm của MovieRepository đang ghi đè một hàm ảo trong `IRepository<Movie>`.

Nếu chữ ký không tương thích, trình biên dịch sẽ báo lỗi.

Từ khóa `override` chỉ được sử dụng tại phần khai báo hàm trong lớp, không sử dụng tại phần định nghĩa hàm trong file `.cpp`.

### 6.6. Public inheritance

```cpp
class MovieRepository : public IRepository<Movie>
```

MovieRepository kế thừa công khai từ interface.

Nhờ đó, có thể sử dụng đối tượng MovieRepository thông qua tham chiếu hoặc con trỏ `IRepository<Movie>`.

Ví dụ:

```cpp
MovieRepository movieRepo("data/movies.txt");

IRepository<Movie>& repo = movieRepo;

repo.load();
```

Hàm được thực hiện là `MovieRepository::load()`.

## 7. Giải thích các hàm private

### 7.1. Hàm isValidMovie()

```cpp
static bool isValidMovie(const Movie* movie);
```

**Chức năng:**

Kiểm tra toàn bộ dữ liệu của một đối tượng Movie.

**Tham số:**

- `movie`: Con trỏ đến đối tượng Movie cần kiểm tra.

**Giá trị trả về:**

- `true`: Dữ liệu hợp lệ.
- `false`: Dữ liệu không hợp lệ.

**Quy trình xử lý:**

1. Kiểm tra con trỏ khác `nullptr`.
2. Kiểm tra mã phim bằng `Validator::isValidId()`.
3. Kiểm tra tên phim và thể loại bằng `Validator::isValidText()`.
4. Kiểm tra thời lượng phim lớn hơn 0.
5. Kiểm tra độ tuổi tối thiểu không âm.
6. Kiểm tra ngày bắt đầu và ngày kết thúc bằng `Validator::isValidDate()`.
7. Kiểm tra ngày bắt đầu không lớn hơn ngày kết thúc.
8. Trả về `true` nếu tất cả điều kiện đều hợp lệ.

Hàm nhận `const Movie*` vì chỉ đọc dữ liệu, không thay đổi thông tin phim.

Hàm được sử dụng trong:

- `stringToMovie()`.
- `movieToString()`.
- `add()`.
- `update()`.

Việc sử dụng chung một hàm kiểm tra giúp tránh lặp lại logic validation.

### 7.2. Hàm getMovieId()

```cpp
static string getMovieId(Movie* movie);
```

**Chức năng:**

Lấy khóa định danh của đối tượng Movie.

**Giá trị trả về:**

- Mã phim nếu con trỏ hợp lệ.
- Chuỗi rỗng nếu con trỏ là `nullptr`.

Repository chung không biết mỗi Model sử dụng hàm getter nào để lấy ID.

Ví dụ:

- Movie sử dụng `getMovieId()`.
- Room sử dụng `getRoomId()`.
- User sử dụng `getUserId()`.

Vì vậy, MovieRepository cung cấp hàm `getMovieId()` cho `Repository<Movie>`.

Repository sử dụng hàm này để xác định khóa, kiểm tra trùng khóa, tìm kiếm và xóa đối tượng.

### 7.3. Hàm stringToMovie()

```cpp
static Movie* stringToMovie(const string& line);
```

**Chức năng:**

Chuyển một dòng dữ liệu TXT thành đối tượng Movie.

Hàm này còn được gọi là hàm parse.

**Tham số:**

- `line`: Dòng dữ liệu phim đọc từ file TXT.

**Giá trị trả về:**

Con trỏ đến đối tượng Movie mới được tạo.

Nếu dữ liệu không hợp lệ, hàm ném `runtime_error`.

**Bước 1: Tách các trường dữ liệu**

```cpp
vector<string> fields;
size_t start = 0;
```

- `fields`: Lưu các trường sau khi tách.
- `start`: Vị trí bắt đầu của trường hiện tại.

```cpp
size_t pos = line.find('|', start);
```

Tìm ký tự `|` đầu tiên kể từ vị trí `start`.

`size_t` là kiểu số nguyên không dấu thường được sử dụng để biểu diễn kích thước hoặc vị trí trong chuỗi và các container.

```cpp
if (pos == string::npos)
```

`string::npos` là giá trị đặc biệt biểu thị không tìm thấy vị trí phù hợp.

Khi không còn ký tự `|`, phần chuỗi còn lại chính là trường cuối cùng.

```cpp
fields.push_back(line.substr(start));
```

Lấy toàn bộ phần chuỗi từ `start` đến cuối dòng và thêm vào vector.

Nếu vẫn tìm thấy ký tự phân cách:

```cpp
fields.push_back(line.substr(start, pos - start));
start = pos + 1;
```

- `substr(start, pos - start)` lấy nội dung của trường hiện tại.
- `start = pos + 1` chuyển sang trường tiếp theo.

**Bước 2: Kiểm tra số lượng trường**

```cpp
if (fields.size() != 8)
{
    throw runtime_error("Sai so luong truong Movie");
}
```

Mỗi dòng dữ liệu Movie phải có đúng 8 trường.

Nếu thiếu hoặc thừa trường, dữ liệu được xem là không hợp lệ.

**Bước 3: Chuyển đổi dữ liệu số**

```cpp
durationMinutes = stoi(fields[3], &pos1);
minAge = stoi(fields[4], &pos2);
```

Hàm `stoi()` chuyển chuỗi thành số nguyên.

Tham số thứ hai nhận địa chỉ của biến dùng để lưu số ký tự đã được xử lý.

Ví dụ:

```cpp
size_t pos = 0;
int value = stoi("120abc", &pos);
```

Kết quả:

- `value = 120`.
- `pos = 3`.

Do `stoi()` có thể chỉ chuyển đổi phần số ở đầu chuỗi, cần kiểm tra:

```cpp
if (pos1 != fields[3].length() ||
    pos2 != fields[4].length())
```

Điều này giúp phát hiện các chuỗi không hợp lệ như `120abc`.

Khối `try-catch` xử lý lỗi khi chuỗi không thể chuyển thành số hoặc giá trị vượt quá phạm vi của kiểu `int`.

**Bước 4: Kiểm tra trạng thái active**

```cpp
if (fields[7] != "0" && fields[7] != "1")
```

Trạng thái phim chỉ được phép lưu dưới dạng `0` hoặc `1`.

```cpp
bool active = (fields[7] == "1");
```

- `"1"` tương ứng với `true`.
- `"0"` tương ứng với `false`.

**Bước 5: Tạo đối tượng Movie**

Sử dụng `new Movie(...)` để cấp phát đối tượng trên vùng nhớ động.

**Bước 6: Kiểm tra đối tượng**

```cpp
if (!isValidMovie(movie))
{
    delete movie;
    throw runtime_error("Du lieu Movie khong hop le");
}
```

Nếu dữ liệu không hợp lệ, phải giải phóng đối tượng vừa tạo trước khi ném exception để tránh rò rỉ bộ nhớ.

Nếu hợp lệ, trả về con trỏ để Repository tiếp nhận và quản lý.

### 7.4. Hàm movieToString()

```cpp
static string movieToString(Movie* movie);
```

**Chức năng:**

Chuyển một đối tượng Movie thành một dòng dữ liệu TXT.

Hàm này còn được gọi là hàm serialize.

**Tham số:**

- `movie`: Con trỏ đến đối tượng cần chuyển đổi.

**Giá trị trả về:**

Chuỗi chứa đầy đủ 8 trường dữ liệu của Movie.

**Quy trình xử lý:**

1. Kiểm tra tính hợp lệ của Movie.
2. Lấy dữ liệu thông qua các getter.
3. Chuyển các trường số thành chuỗi bằng `to_string()`.
4. Chuyển trạng thái active thành `"0"` hoặc `"1"`.
5. Ghép các trường bằng ký tự `|`.
6. Trả về chuỗi kết quả.

Biểu thức:

```cpp
(movie->isActive() ? "1" : "0")
```

Là toán tử ba ngôi.

Nếu `isActive()` trả về `true`, biểu thức trả về `"1"`.

Ngược lại, biểu thức trả về `"0"`.

MovieRepository chỉ tạo chuỗi dữ liệu. Việc ghi chuỗi xuống file do Repository thực hiện.

## 8. Giải thích các hàm public

### 8.1. Constructor

```cpp
MovieRepository::MovieRepository(const string& filePath)
    : repo(filePath, getMovieId, stringToMovie, movieToString)
{
}
```

**Chức năng:**

Khởi tạo MovieRepository và đối tượng `Repository<Movie>` bên trong.

**Tham số:**

- `filePath`: Đường dẫn file lưu dữ liệu phim.

Constructor sử dụng danh sách khởi tạo thành viên (member initializer list) để khởi tạo trực tiếp thuộc tính `repo`.

Repository nhận 4 tham số:

| Tham số | Chức năng |
|---|---|
| filePath | Đường dẫn file dữ liệu |
| getMovieId | Hàm lấy khóa của Movie |
| stringToMovie | Hàm chuyển chuỗi thành Movie |
| movieToString | Hàm chuyển Movie thành chuỗi |

Ba hàm cuối được truyền dưới dạng con trỏ hàm.

Nhờ đó, `Repository<T>` có thể xử lý nhiều Model khác nhau mà không cần biết chi tiết từng Model.

Constructor không trực tiếp nạp dữ liệu. Việc nạp dữ liệu được thực hiện thông qua `load()`.

### 8.2. Hàm load()

```cpp
bool MovieRepository::load()
{
    return repo.load();
}
```

**Chức năng:**

Nạp dữ liệu phim từ file TXT vào bộ nhớ.

**Giá trị trả về:**

- `true`: Nạp dữ liệu thành công.
- `false`: Nạp dữ liệu không thành công hoặc không được phép nạp lại.

Hàm gọi trực tiếp `Repository<Movie>::load()`.

Trong quá trình nạp, Repository sử dụng `stringToMovie()` để chuyển từng dòng TXT thành đối tượng Movie.

Do `stringToMovie()` đã gọi `isValidMovie()`, MovieRepository không cần duyệt lại toàn bộ danh sách để kiểm tra dữ liệu sau khi nạp.

Movie không cần thiết lập tham chiếu đến Repository khác trong quá trình load.

Các lỗi dữ liệu có thể phát sinh exception theo cơ chế của Repository.

### 8.3. Hàm save()

```cpp
bool MovieRepository::save()
{
    return repo.save();
}
```

**Chức năng:**

Lưu danh sách phim hiện tại xuống file TXT.

**Giá trị trả về:**

- `true`: Lưu thành công.
- `false`: Lưu thất bại.

Repository sử dụng `movieToString()` để chuyển từng đối tượng Movie thành chuỗi trước khi ghi file.

MovieRepository không trực tiếp sử dụng `ofstream` hoặc tự triển khai cơ chế ghi file.

### 8.4. Hàm add()

```cpp
bool add(Movie* movie) override;
```

**Chức năng:**

Thêm một phim mới vào Repository.

**Tham số:**

- `movie`: Con trỏ đến đối tượng Movie cần thêm.

**Giá trị trả về:**

- `true`: Thêm thành công.
- `false`: Thêm thất bại.

**Quy trình xử lý:**

Bước 1: Kiểm tra dữ liệu Movie tại tầng Concrete Repository.

```cpp
if (!isValidMovie(movie))
{
    return false;
}
```

`isValidMovie()` kiểm tra `nullptr` trước khi truy cập thuộc tính, rồi kiểm tra mã `MOV`, văn bản, thời lượng, tuổi và ngày. Đây là validation riêng của Movie; nếu sai, trả `false` và không chuyển quyền sở hữu.

Bước 2: Giao kiểm tra lưu trữ chung và thêm phim cho Repository.

```cpp
return repo.add(movie);
```

`Repository<Movie>::add()` kiểm tra loaded, `nullptr`, trùng địa chỉ con trỏ, ID rỗng và trùng ID trước khi `pushBack()`. MovieRepository không cần lặp lại các kiểm tra này. `return` chuyển nguyên kết quả `bool` của tầng dưới; không tự bắt exception hoặc lưu TXT.

**Luồng hoạt động chi tiết:**

```text
add(movie) → isValidMovie(movie)? ─ không → false (caller giữ con trỏ mới)
                            │ có
                            v
                       repo.add(movie) → điều kiện lưu trữ sai → false
                            │ thành công
                            v
                         true; repo sở hữu movie
```

**Lưu ý:**

- MovieRepository không tự sinh mã phim.
- Mã phim phải được chuẩn bị trước khi gọi `add()`.
- Khi thêm thành công, Repository tiếp nhận quyền sở hữu đối tượng.
- Khi thêm thất bại, đối tượng mới chưa được Repository tiếp nhận vẫn thuộc quyền sở hữu của bên gọi.
- Hàm không tự động lưu dữ liệu xuống file.
- Việc lưu dữ liệu được thực hiện thông qua `save()`.

### 8.5. Hàm update()

```cpp
bool update(const Movie& newValues) override;
```

**Chức năng:**

Cập nhật thông tin của một phim đã tồn tại.

**Tham số:**

- `newValues`: Đối tượng chứa thông tin mới của phim.

Sử dụng `const Movie&` để tránh sao chép đối tượng và không thay đổi dữ liệu đầu vào thông qua tham số.

**Giá trị trả về:**

- `true`: Cập nhật thành công.
- `false`: Cập nhật thất bại.

**Quy trình xử lý:**

Bước 1: Kiểm tra trạng thái Repository và dữ liệu mới.

```cpp
if (!repo.isLoaded() || !isValidMovie(&newValues))
{
    return false;
}
```

Toán tử `&` lấy địa chỉ của đối tượng `newValues` để truyền vào hàm `isValidMovie()`.

Bước 2: Tìm phim cần cập nhật.

```cpp
Movie* movie = repo.findById(newValues.getMovieId());
```

Mã phim trong `newValues` được sử dụng để xác định đối tượng cần cập nhật.

Bước 3: Kiểm tra phim có tồn tại hay không.

```cpp
if (movie == nullptr)
{
    return false;
}
```

Bước 4: Cập nhật các thuộc tính bằng setter.

Các thuộc tính được cập nhật:

- Tên phim.
- Thể loại.
- Thời lượng.
- Độ tuổi tối thiểu.
- Ngày bắt đầu chiếu.
- Ngày kết thúc chiếu.
- Trạng thái hoạt động.

Không cập nhật mã phim.

**Lý do cập nhật trực tiếp đối tượng:**

Movie có thể đang được các đối tượng Showtime tham chiếu.

Nếu xóa Movie cũ và tạo Movie mới, các con trỏ đang tham chiếu đến Movie cũ có thể trở thành con trỏ không hợp lệ.

Cập nhật trực tiếp bằng setter giúp giữ nguyên địa chỉ của đối tượng và các liên kết hiện tại.

**Lưu ý:**

- Không thay đổi mã phim.
- Không thay thế đối tượng Movie hiện tại bằng đối tượng mới.
- Không tự động lưu dữ liệu xuống file.
- Các ràng buộc nghiệp vụ bổ sung liên quan đến suất chiếu cần được kiểm tra tại tầng phù hợp.
- Hàm hiện tại không kiểm tra toàn bộ lịch chiếu khi thay đổi thời lượng hoặc khoảng ngày chiếu của phim.

### 8.6. Hàm removeById()

```cpp
bool removeById(const string& movieId) override;
```

**Chức năng:**

Xóa phim khỏi Repository dựa trên mã phim.

**Tham số:**

- `movieId`: Mã phim cần xóa.

**Giá trị trả về:**

- `true`: Xóa thành công.
- `false`: Xóa thất bại.

**Quy trình xử lý:**

Bước 1: Tìm phim theo mã.

```cpp
Movie* movie = repo.findById(movieId);
```

Bước 2: Kiểm tra phim tồn tại trước khi truy cập con trỏ.

```cpp
if (movie == nullptr)
{
    return false;
}
```

Bước 3: Kiểm tra liên kết với Showtime.

```cpp
if (movie->getShowtimeCount() > 0)
{
    return false;
}
```

Không cho phép xóa phim nếu phim vẫn còn liên kết với bất kỳ Showtime nào.

Quy định này giúp tránh việc Showtime tham chiếu đến một đối tượng Movie đã bị giải phóng.

Bước 4: Xóa phim thông qua Repository chung.

```cpp
return repo.removeById(movieId);
```

`repo.findById()` trả `nullptr` khi danh sách chưa có phim; `repo.removeById()` vẫn tự kiểm tra loaded, tìm ID, xóa node và giải phóng bộ nhớ. MovieRepository chỉ giữ `nullptr` guard và ràng buộc Showtime cần thiết trước khi dereference/xóa.

**Luồng hoạt động chi tiết:**

```text
removeById(movieId) → repo.findById(movieId) → nullptr? ─ có → false
                                               │ không
                                               v
                           getShowtimeCount() > 0? ─ có → false
                                               │ không
                                               v
                            repo.removeById(movieId) → trả bool
```

**Lưu ý:**

- Không gọi `delete movie` trực tiếp trong MovieRepository.
- Không tự động xóa các Showtime liên quan.
- Không tiếp tục sử dụng con trỏ Movie sau khi xóa thành công.
- Việc xóa chỉ thay đổi dữ liệu trong bộ nhớ cho đến khi gọi `save()`.

### 8.7. Hàm findById()

```cpp
Movie* MovieRepository::findById(const string& movieId)
{
    return repo.findById(movieId);
}
```

**Chức năng:**

Tìm phim dựa trên mã phim.

**Tham số:**

- `movieId`: Mã phim cần tìm.

**Giá trị trả về:**

- Con trỏ Movie nếu tìm thấy.
- `nullptr` nếu không tìm thấy.

Hàm gọi trực tiếp Repository chung vì không cần xử lý logic riêng.

Con trỏ trả về là con trỏ mượn.

Bên gọi không được tự ý giải phóng đối tượng.

### 8.8. Hàm getAt()

```cpp
Movie* MovieRepository::getAt(int index)
{
    return repo.getAt(index);
}
```

**Chức năng:**

Lấy phim tại một vị trí trong danh sách.

**Tham số:**

- `index`: Vị trí của phim, bắt đầu từ 0.

**Giá trị trả về:**

- Con trỏ Movie nếu vị trí hợp lệ.
- `nullptr` nếu vị trí không hợp lệ.

Hàm thường được sử dụng kết hợp với `count()` để duyệt danh sách.

Ví dụ:

```cpp
for (int i = 0; i < movieRepo.count(); i++)
{
    Movie* movie = movieRepo.getAt(i);

    if (movie != nullptr)
    {
        // Xu ly thong tin phim
    }
}
```

### 8.9. Hàm count()

```cpp
int MovieRepository::count()
{
    return repo.count();
}
```

**Chức năng:**

Trả về số lượng phim hiện có trong Repository.

Nếu danh sách rỗng, hàm trả về `0`.

Hàm không phân biệt phim đang hoạt động hay ngừng hoạt động.

Việc lọc phim theo trạng thái thuộc trách nhiệm của tầng Business Logic.

### 8.10. Hàm isLoaded()

```cpp
bool MovieRepository::isLoaded()
{
    return repo.isLoaded();
}
```

**Chức năng:**

Kiểm tra trạng thái nạp dữ liệu.

**Giá trị trả về:**

- `true`: Repository đã được nạp dữ liệu.
- `false`: Repository chưa được nạp dữ liệu.

MovieRepository không khai báo thuộc tính `loaded` riêng mà sử dụng trạng thái của `Repository<Movie>`.

## 9. Quản lý bộ nhớ

### 9.1. Quyền sở hữu đối tượng

Repository chung chịu trách nhiệm quản lý bộ nhớ của các đối tượng Movie đã được tiếp nhận.

MovieRepository không trực tiếp quản lý danh sách con trỏ.

Quy định:

- Đối tượng Movie mới do bên gọi quản lý trước khi thêm.
- Khi `add()` thành công, quyền sở hữu được chuyển sang Repository.
- Khi `add()` thất bại, bên gọi chịu trách nhiệm giải phóng đối tượng mới chưa được tiếp nhận.
- Không giải phóng lại đối tượng đã thuộc quyền sở hữu của Repository.
- Khi `removeById()` thành công, Repository giải phóng đối tượng tương ứng.

### 9.2. Con trỏ mượn

Các hàm `findById()` và `getAt()` trả về con trỏ mượn.

Ví dụ:

```cpp
Movie* movie = movieRepo.findById("MOV0001");
```

Không được thực hiện:

```cpp
delete movie;
```

Việc giải phóng đối tượng thuộc trách nhiệm của Repository.

Không tiếp tục sử dụng con trỏ sau khi đối tượng đã bị xóa hoặc Repository đã bị hủy.

### 9.3. Destructor

MovieRepository không cần định nghĩa destructor riêng.

`IRepository<T>` đã khai báo destructor ảo:

```cpp
virtual ~IRepository() = default;
```

Destructor được tạo tự động của MovieRepository cũng là destructor ảo.

Khi MovieRepository bị hủy, thuộc tính `Repository<Movie> repo` được hủy theo cơ chế của C++.

Repository chung chịu trách nhiệm giải phóng các đối tượng đang quản lý.

Các Repository và đối tượng đang tham chiếu đến Movie phải được xử lý theo đúng thứ tự vòng đời để tránh con trỏ không hợp lệ.

## 10. Áp dụng Dependency Inversion Principle

### 10.1. Trước khi refactor

MovieService phụ thuộc trực tiếp vào MovieRepository.

```cpp
class MovieService
{
private:
    MovieRepository& movieRepo;
};
```

MovieService phải biết lớp triển khai cụ thể là MovieRepository.

### 10.2. Sau khi refactor

MovieService dự kiến phụ thuộc vào interface.

```cpp
class MovieService
{
private:
    IRepository<Movie>& movieRepo;
};
```

Constructor dự kiến:

```cpp
MovieService(IRepository<Movie>& movieRepo,
             AuthService& authService);
```

Khi khởi tạo chương trình:

```cpp
MovieRepository movieRepo("data/movies.txt");

MovieService movieService(movieRepo, authService);
```

MovieRepository được truyền vào MovieService thông qua constructor.

Đây là Constructor Injection.

MovieService chỉ cần sử dụng các hàm được khai báo trong `IRepository<Movie>` mà không cần biết cách MovieRepository quản lý danh sách hoặc đọc ghi dữ liệu.

Phần refactor MovieService sẽ được thực hiện trong bước tiếp theo.

## 11. Các thay đổi sau refactor

Phần 11.1 ghi nhận refactor DIP; phần 11.2 ghi nhận điều chỉnh validation đã triển khai và kiểm thử trong phạm vi `test_irepository.cpp`.

### 11.1. Thay đổi trong MovieRepository.h

Bổ sung thư viện:

```cpp
#include "IRepository.h"
```

Thay đổi khai báo lớp:

```cpp
class MovieRepository : public IRepository<Movie>
```

Bổ sung `override` cho 9 hàm:

- `load()`.
- `save()`.
- `add()`.
- `update()`.
- `removeById()`.
- `findById()`.
- `getAt()`.
- `count()`.
- `isLoaded()`.

### 11.2. Thay đổi trong MovieRepository.cpp

Theo `validation_rules.md`, `add()` giao kiểm tra loaded, `nullptr` và trùng con trỏ/ID cho `Repository<Movie>` sau khi `isValidMovie()` kiểm tra dữ liệu Movie. `removeById()` tìm phim, kiểm tra `nullptr` và liên kết Showtime, rồi giao kiểm tra loaded, tìm kiếm và giải phóng cho `Repository<Movie>`. Hai thay đổi đã có trong `.cpp` hiện tại.

Các hàm hiện tại có chữ ký tương thích với `IRepository<Movie>`.

Tên tham số trong lớp con có thể khác tên tham số trong interface.

Ví dụ:

```cpp
// IRepository
virtual bool removeById(const string& key) = 0;

// MovieRepository
bool removeById(const string& movieId) override;
```

Hai hàm có chữ ký tương thích vì tên tham số không ảnh hưởng đến cơ chế ghi đè.

### 11.3. Các thành phần giữ nguyên

- Constructor.
- Thuộc tính `Repository<Movie> repo`.
- Các hàm static.
- `isValidMovie()`, parse, serialize và các kiểm tra dữ liệu riêng của Movie.
- Logic đọc ghi file.
- API và hành vi trả lỗi của thêm, cập nhật và xóa phim; chỉ phân lại nơi thực hiện hai kiểm tra lưu trữ trùng lặp.
- Cơ chế quản lý bộ nhớ.
- Cơ chế kiểm tra liên kết với Showtime.
- Định dạng file TXT.
- Cấu trúc Model.

### 11.4. Rà soát các phương thức còn lại theo validation_rules.md

- `isValidMovie()` giữ kiểm tra `nullptr` và toàn bộ thuộc tính của Movie; được dùng tại load, add, update và serialize để không phụ thuộc vào Service.
- `getMovieId()` giữ nhánh null trả chuỗi rỗng để callback không dereference con trỏ null. `stringToMovie()` giữ kiểm tra đủ 8 trường, parse số hết chuỗi, active chỉ `0/1`, validation và `delete movie` trước khi ném lỗi; `Repository<Movie>::load()` chịu trách nhiệm dọn danh sách tạm.
- `movieToString()` giữ validation trước khi ghi để không xuất dữ liệu Movie sai. `load()` và `save()` chỉ chuyển lời gọi vì tầng chung đã xử lý loaded, file và ngoại lệ theo hợp đồng hiện tại.
- `update()` giữ kiểm tra loaded vì tự tìm và sửa trực tiếp đối tượng, không có `Repository<T>::update()` bảo vệ thao tác. Giữ `isValidMovie(&newValues)` và kiểm tra `movie == nullptr` trước các setter; mã phim và địa chỉ đối tượng không đổi để bảo vệ liên kết. Các setter hiện tại không có rollback nhiều bước.
- `findById()`, `getAt()`, `count()` và `isLoaded()` chỉ chuyển lời gọi; điều kiện tìm kiếm, chỉ số và trạng thái thuộc tầng chung. Không thêm validation hoặc API mới.

## 12. Kế hoạch kiểm thử

### 12.1. Kiểm thử nạp dữ liệu

- [ ] Nạp file hợp lệ.
- [ ] Nạp file rỗng.
- [ ] Nạp file không tồn tại.
- [ ] Nạp file có dòng thiếu trường.
- [ ] Nạp file có dòng thừa trường.
- [ ] Nạp file có mã phim không hợp lệ.
- [ ] Nạp file có mã phim trùng nhau.
- [ ] Nạp file có thời lượng không hợp lệ.
- [ ] Nạp file có độ tuổi không hợp lệ.
- [ ] Nạp file có ngày không hợp lệ.
- [ ] Nạp file có trạng thái active không hợp lệ.
- [ ] Kiểm tra trường hợp nạp dữ liệu nhiều lần.

### 12.2. Kiểm thử thêm phim

Các trường hợp đã có trong `test_irepository.cpp` được xác nhận sau khi triển khai; các trường hợp còn lại cần test riêng.

- [ ] Thêm phim hợp lệ.
- [ ] Thêm phim khi Repository chưa được nạp.
- [ ] Thêm con trỏ nullptr.
- [ ] Thêm phim có mã trùng.
- [ ] Thêm phim có dữ liệu không hợp lệ.
- [ ] Thêm lại con trỏ đã được quản lý.
- [ ] Kiểm tra số lượng phim sau khi thêm.
- [ ] Kiểm tra dữ liệu không thay đổi khi thêm thất bại.

### 12.3. Kiểm thử cập nhật phim

- [ ] Cập nhật phim hợp lệ.
- [ ] Cập nhật phim không tồn tại.
- [ ] Cập nhật bằng dữ liệu không hợp lệ.
- [ ] Cập nhật khi Repository chưa được nạp.
- [ ] Kiểm tra mã phim không thay đổi.
- [ ] Kiểm tra địa chỉ đối tượng không thay đổi.
- [ ] Kiểm tra các liên kết Showtime được giữ nguyên.

### 12.4. Kiểm thử xóa phim

`test_irepository.cpp` đã xác nhận đường đi chưa load và xóa phim thông thường. Trường hợp phim có liên kết Showtime vẫn cần test riêng để xác nhận ràng buộc không giải phóng Movie đang được tham chiếu.

- [ ] Xóa phim hợp lệ.
- [ ] Xóa phim không tồn tại.
- [ ] Xóa phim khi Repository chưa được nạp.
- [ ] Xóa phim đang có liên kết Showtime.
- [ ] Kiểm tra số lượng phim sau khi xóa.
- [ ] Kiểm tra phim không còn được tìm thấy sau khi xóa.
- [ ] Kiểm tra dữ liệu không thay đổi khi xóa thất bại.

### 12.5. Kiểm thử truy vấn

- [ ] Tìm phim theo mã hợp lệ.
- [ ] Tìm phim không tồn tại.
- [ ] Lấy phim tại vị trí hợp lệ.
- [ ] Lấy phim tại vị trí âm.
- [ ] Lấy phim tại vị trí vượt giới hạn.
- [ ] Đếm số lượng phim.
- [ ] Kiểm tra trạng thái isLoaded().

### 12.6. Kiểm thử lưu dữ liệu

- [ ] Lưu danh sách phim hợp lệ.
- [ ] Lưu danh sách phim rỗng.
- [ ] Lưu khi Repository chưa được nạp.
- [ ] Kiểm tra đúng định dạng TXT.
- [ ] Nạp lại dữ liệu sau khi lưu.
- [ ] Kiểm tra dữ liệu trước và sau khi lưu.

### 12.7. Kiểm thử IRepository

Các dấu `[x]` dưới đây đã được xác nhận bằng lần chạy lại `test_irepository.cpp` sau điều chỉnh validation.

- [x] Khởi tạo MovieRepository sau khi kế thừa interface.
- [x] Sử dụng tham chiếu `IRepository<Movie>&`.
- [x] Sử dụng con trỏ `IRepository<Movie>*`.
- [x] Gọi load() thông qua interface.
- [x] Gọi save() thông qua interface.
- [x] Gọi add() thông qua interface.
- [x] Gọi update() thông qua interface.
- [x] Gọi removeById() thông qua interface.
- [x] Gọi findById() thông qua interface.
- [x] Gọi getAt() thông qua interface.
- [x] Gọi count() thông qua interface.
- [x] Gọi isLoaded() thông qua interface.
- [x] Kiểm tra destructor ảo.
- [ ] Chạy lại các bài kiểm thử MovieRepository hiện có.

## 13. Tiến độ triển khai

**Điều chỉnh validation:** Đã triển khai `add()` và `removeById()` theo Markdown đã chốt; đối chiếu toàn bộ khối `.cpp` ở mục 5 với file hiện tại. `test_irepository.cpp` đã chạy lại sau thay đổi.

- [x] Thiết kế MovieRepository kế thừa IRepository.
- [x] Xác định các hàm cần override.
- [x] Rà soát chữ ký các hàm hiện tại.
- [x] Xác định phạm vi thay đổi file MovieRepository.h.
- [x] Rà soát logic file MovieRepository.cpp.
- [x] Xây dựng nội dung tài liệu MovieRepository.md.
- [x] Review và CHỐT MARKDOWN.
- [x] Đối chiếu toàn bộ `MovieRepository.cpp` với khối mã ở mục 5: khớp 100%.
- [x] Biên dịch mã nguồn cùng `test_irepository.cpp`.
- [x] Kiểm thử MovieRepository trong phạm vi `test_irepository.cpp`.
- [x] Kiểm thử IRepository thông qua MovieRepository.
- [x] Cập nhật tiến độ refactor.
- [x] CHỐT MARKDOWN và triển khai điều chỉnh validation ở mục 5, 8 và 11.
- [x] Biên dịch, chạy lại `test_irepository.cpp` sau khi triển khai: 81 PASS, 0 FAIL.
- [ ] Chạy lại bài test MovieRepository cũ và các trường hợp liên kết Showtime chưa được xác nhận.

**Kết quả thực tế (24/11/2026):** `tests/test_irepository.cpp` biên dịch với C++17 và chạy thành công: 81 PASS, 0 FAIL. Bài test bao phủ load/save, CRUD, truy vấn, trạng thái chưa load, đa hình và exception khi load dữ liệu lỗi. Các mục kiểm thử chưa đánh dấu ở mục 12 và bài `test_movie_repository.cpp` cũ chưa được chạy lại trong lần cập nhật này.

## 14. Kết luận

MovieRepository là Concrete Repository triển khai `IRepository<Movie>` và sử dụng `Repository<Movie>` thông qua composition.

Sau refactor, MovieRepository triển khai đầy đủ 9 hàm của interface.

Refactor DIP đã bổ sung quan hệ kế thừa và `override`. Điều chỉnh validation đã rút gọn hai hàm trong `.cpp`, giữ nguyên chữ ký API và ràng buộc Movie.

Thiết kế này giúp MovieRepository tương thích với kiến trúc Dependency Inversion Principle, đồng thời giữ nguyên cơ chế quản lý dữ liệu, bộ nhớ, liên kết đối tượng và định dạng file TXT.

MovieRepository là mẫu chuẩn cho các Concrete Repository còn lại; mỗi Repository vẫn phải áp dụng logic riêng của Model.
