# MovieRepository – Quản lý dữ liệu phim

## 1. Tổng quan

`MovieRepository` là lớp thuộc tầng Data Access, chịu trách nhiệm quản lý dữ liệu phim trong hệ thống quản lý rạp chiếu phim.

Lớp này sử dụng `Repository<Movie>` để tái sử dụng các chức năng chung như đọc file, ghi file, thêm, xóa, tìm kiếm và quản lý bộ nhớ.

Ngoài ra, `MovieRepository` bổ sung những quy tắc riêng của dữ liệu phim, chẳng hạn kiểm tra thời lượng, độ tuổi, ngày phát hành và điều kiện xóa phim.

Các chức năng chính:

- Đọc và ghi dữ liệu phim từ file TXT.
- Kiểm tra tính hợp lệ của dữ liệu Movie.
- Chuyển đổi giữa chuỗi TXT và đối tượng Movie.
- Thêm phim mới.
- Cập nhật thông tin phim.
- Xóa phim khi đáp ứng điều kiện.
- Tìm kiếm và truy xuất danh sách phim.

**Phân chia trách nhiệm:**

- `Repository<Movie>`: Quản lý dữ liệu và bộ nhớ.
- `MovieRepository`: Kiểm tra các quy tắc dữ liệu riêng của Movie.
- `MovieService`: Xử lý nghiệp vụ quản lý phim ở tầng Business.

---

## 2. Cấu trúc lớp MovieRepository

### 2.1. Thuộc tính

```cpp
private:
    Repository<Movie> repo;
```

`repo` là đối tượng của lớp `Repository<T>` chung, với kiểu dữ liệu cụ thể là `Movie`.

Đối tượng này chịu trách nhiệm quản lý danh sách `Movie*`, đọc và ghi dữ liệu, đồng thời giải phóng các đối tượng mà nó sở hữu.

Khác với `UserRepository`, `MovieRepository` hiện tại không sử dụng `unordered_map` để lập chỉ mục.

Vì vậy, các thao tác tìm kiếm phim theo ID vẫn được thực hiện bằng cách duyệt danh sách trong `Repository<Movie>`.

### 2.2. Các hàm private

```cpp
static string getMovieId(Movie* movie);
static Movie* stringToMovie(const string& line);
static string movieToString(Movie* movie);
static bool isValidMovie(const Movie* movie);
```

Các hàm này hỗ trợ Repository chung trong việc lấy ID, chuyển đổi dữ liệu và kiểm tra tính hợp lệ của Movie.

Chúng được khai báo `static` vì không cần truy cập trực tiếp đến thuộc tính `repo` của đối tượng MovieRepository.

---

## 3. Constructor – Khởi tạo MovieRepository

```cpp
MovieRepository::MovieRepository(const string& filePath)
    : repo(filePath, getMovieId, stringToMovie, movieToString)
{
}
```

**Chức năng:** Khởi tạo `Repository<Movie>` và cung cấp các hàm xử lý dữ liệu Movie.

Constructor truyền bốn tham số cho Repository chung:

- `filePath`: Đường dẫn file lưu dữ liệu phim.
- `getMovieId`: Hàm lấy ID.
- `stringToMovie`: Hàm chuyển chuỗi thành Movie.
- `movieToString`: Hàm chuyển Movie thành chuỗi.

Ví dụ:

```cpp
MovieRepository movieRepo("data/movies.txt");
```

Constructor chỉ khởi tạo Repository, chưa đọc dữ liệu từ file. Cần gọi `load()` để nạp dữ liệu.

---

## 4. Hàm getMovieId()

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

**Chức năng:** Trả về mã định danh của Movie.

Nếu con trỏ bằng `nullptr`, hàm trả về chuỗi rỗng. Ngược lại, hàm gọi `getMovieId()` của Model.

Hàm này được truyền cho `Repository<Movie>` để Repository chung có thể lấy ID mà không cần biết cấu trúc cụ thể của lớp Movie.

---

## 5. Hàm isValidMovieId()

```cpp
static bool isValidMovieId(const string& id)
```

### Chức năng

Kiểm tra mã phim có đúng định dạng quy định hay không.

Đây là hàm hỗ trợ được định nghĩa trong `MovieRepository.cpp`, không phải hàm thành viên của lớp MovieRepository.

### Các bước xử lý

**Bước 1: Kiểm tra độ dài và tiền tố**

```cpp
if (id.length() != 7 || id.substr(0, 3) != "MOV")
{
    return false;
}
```

ID phải có đúng 7 ký tự và bắt đầu bằng `MOV`.

**Bước 2: Kiểm tra phần số**

```cpp
for (int i = 3; i < 7; i++)
{
    if (id[i] < '0' || id[i] > '9')
    {
        return false;
    }
}
```

Bốn ký tự còn lại phải là chữ số từ `0` đến `9`.

**Bước 3: Kiểm tra ID khác 0000**

```cpp
return id.substr(3) != "0000";
```

Không chấp nhận mã `MOV0000`.

Ví dụ:

| ID | Kết quả |
|---|---|
| `MOV0001` | Hợp lệ |
| `MOV0123` | Hợp lệ |
| `MOV9999` | Hợp lệ |
| `MOV0000` | Không hợp lệ |
| `USR0001` | Không hợp lệ |
| `MOV12A4` | Không hợp lệ |

---

## 6. Hàm isValidMovie()

```cpp
bool MovieRepository::isValidMovie(const Movie* movie)
```

### Chức năng

Kiểm tra toàn bộ thông tin của một đối tượng Movie trước khi thêm, cập nhật hoặc ghi dữ liệu.

### Các bước xử lý

**Bước 1: Kiểm tra con trỏ**

```cpp
if (movie == nullptr)
{
    return false;
}
```

Nếu đối tượng không tồn tại, không thể kiểm tra dữ liệu.

**Bước 2: Kiểm tra mã phim**

```cpp
if (!isValidMovieId(movie->getMovieId()))
{
    return false;
}
```

Sử dụng hàm `isValidMovieId()` để xác nhận ID đúng định dạng `MOVxxxx`.

**Bước 3: Kiểm tra tên và thể loại**

```cpp
if (!Validator::isValidText(movie->getTitle()) ||
    !Validator::isValidText(movie->getGenre()))
{
    return false;
}
```

Sử dụng `Validator` chung để kiểm tra các trường văn bản.

**Bước 4: Kiểm tra thời lượng**

```cpp
if (movie->getDurationMinutes() <= 0)
{
    return false;
}
```

Thời lượng phim phải lớn hơn 0 phút.

**Bước 5: Kiểm tra độ tuổi tối thiểu**

```cpp
if (movie->getMinAge() < 0)
{
    return false;
}
```

Độ tuổi tối thiểu không được âm.

**Bước 6: Kiểm tra ngày tháng**

```cpp
if (!Validator::isValidDate(movie->getReleaseDate()) ||
    !Validator::isValidDate(movie->getEndDate()))
{
    return false;
}
```

Cả ngày phát hành và ngày kết thúc phải có định dạng và giá trị ngày hợp lệ theo `Validator`.

**Bước 7: Kiểm tra thứ tự ngày**

```cpp
if (movie->getReleaseDate() > movie->getEndDate())
{
    return false;
}
```

Ngày phát hành không được lớn hơn ngày kết thúc.

Việc so sánh chuỗi này phù hợp khi ngày được chuẩn hóa theo định dạng `YYYY-MM-DD`.

### Kết quả

Hàm trả về `true` nếu tất cả điều kiện hợp lệ, ngược lại trả về `false`.

### Lưu ý

`isValidMovie()` kiểm tra dữ liệu của một Movie độc lập, không kiểm tra trùng ID với các Movie khác.

Việc kiểm tra ID trùng được thực hiện trong `Repository<Movie>`.

---

## 7. Hàm stringToMovie()

```cpp
Movie* MovieRepository::stringToMovie(const string& line)
```

### Chức năng

Chuyển một dòng dữ liệu trong file TXT thành đối tượng Movie.

Ví dụ:

```text
MOV0001|Avengers|Action|180|13|2026-01-01|2026-12-31|1
```

### Các bước xử lý

**Bước 1: Khởi tạo danh sách lưu trường dữ liệu**

```cpp
vector<string> fields;
size_t start = 0;
```

`fields` lưu các chuỗi sau khi tách.

`start` đánh dấu vị trí bắt đầu của trường đang xử lý.

**Bước 2: Tách chuỗi theo dấu `|`**

```cpp
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
```

Giải thích:

- `line.find('|', start)` tìm dấu `|` tiếp theo từ vị trí `start`.
- `line.substr(start, pos - start)` lấy nội dung nằm trước dấu phân cách.
- `fields.push_back()` đưa nội dung vừa tách vào vector.
- `start = pos + 1` chuyển sang trường tiếp theo.
- `string::npos` cho biết không còn tìm thấy dấu `|`.

Khi không còn dấu phân cách, hàm lấy phần chuỗi cuối cùng rồi kết thúc vòng lặp.

Sau khi tách:

```cpp
fields[0] = "MOV0001";
fields[1] = "Avengers";
fields[2] = "Action";
fields[3] = "180";
fields[4] = "13";
fields[5] = "2026-01-01";
fields[6] = "2026-12-31";
fields[7] = "1";
```

**Bước 3: Kiểm tra số lượng trường**

```cpp
if (fields.size() != 8)
{
    throw runtime_error("Sai so luong truong Movie");
}
```

Một dòng Movie phải có đúng 8 trường.

Nếu thiếu hoặc thừa trường, hàm ném ngoại lệ để báo dữ liệu sai định dạng.

**Bước 4: Chuyển dữ liệu số**

```cpp
int durationMinutes;
int minAge;

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
    throw runtime_error("Thoi luong hoac do tuoi khong hop le");
}
```

`stoi()` chuyển chuỗi thành số nguyên.

Ví dụ:

```cpp
stoi("180"); // 180
stoi("13");  // 13
```

Tuy nhiên, `stoi()` có thể chuyển được phần đầu của một chuỗi không hoàn toàn là số.

Ví dụ, chuỗi `"180abc"` có thể được chuyển thành số `180`.

Vì vậy, chương trình sử dụng `pos1` và `pos2` để kiểm tra số ký tự đã được chuyển đổi.

```cpp
pos1 != fields[3].length()
```

Nếu số ký tự đã xử lý không bằng độ dài chuỗi, nghĩa là vẫn còn ký tự không hợp lệ.

Khối `try...catch` xử lý các lỗi chuyển đổi số và ném lại thông báo phù hợp với dữ liệu Movie.

**Bước 5: Kiểm tra trạng thái active**

```cpp
if (fields[7] != "0" && fields[7] != "1")
{
    throw runtime_error("Trang thai Movie khong hop le");
}

bool active = (fields[7] == "1");
```

Quy ước:

- `"1"`: Movie đang được kích hoạt.
- `"0"`: Movie không được kích hoạt.

Những giá trị khác đều không hợp lệ.

**Bước 6: Tạo đối tượng Movie**

```cpp
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

Sau khi chuyển đổi dữ liệu, chương trình sử dụng `new` để cấp phát đối tượng Movie.

**Bước 7: Kiểm tra dữ liệu Movie**

```cpp
if (!isValidMovie(movie))
{
    delete movie;
    throw runtime_error("Du lieu Movie khong hop le");
}
```

Nếu Movie không hợp lệ, phải giải phóng đối tượng vừa tạo bằng `delete` trước khi ném ngoại lệ.

Điều này tránh rò rỉ bộ nhớ.

**Bước 8: Trả về đối tượng**

```cpp
return movie;
```

Nếu mọi bước thành công, hàm trả về con trỏ `Movie*`.

Khi được gọi từ `Repository<Movie>::load()`, đối tượng này sẽ được Repository chung tiếp nhận và quản lý.

### Tổng kết

`stringToMovie()` thực hiện ba nhiệm vụ chính:

1. Phân tích cấu trúc dòng TXT.
2. Chuyển đổi các trường dữ liệu sang kiểu tương ứng.
3. Tạo và kiểm tra đối tượng Movie.

Nếu dữ liệu sai, hàm ném ngoại lệ thay vì trả về một Movie không hợp lệ.

---

## 8. Hàm movieToString()

```cpp
string MovieRepository::movieToString(Movie* movie)
```

### Chức năng

Chuyển đối tượng Movie thành một dòng TXT để ghi xuống file.

Đây là thao tác ngược lại của `stringToMovie()`.

### Các bước xử lý

**Bước 1: Kiểm tra dữ liệu**

```cpp
if (!isValidMovie(movie))
{
    throw runtime_error("Movie khong hop le khi ghi file");
}
```

Không cho phép ghi Movie có dữ liệu không hợp lệ.

**Bước 2: Chuyển các trường thành chuỗi**

```cpp
return movie->getMovieId() + "|" +
       movie->getTitle() + "|" +
       movie->getGenre() + "|" +
       to_string(movie->getDurationMinutes()) + "|" +
       to_string(movie->getMinAge()) + "|" +
       movie->getReleaseDate() + "|" +
       movie->getEndDate() + "|" +
       (movie->isActive() ? "1" : "0");
```

Các trường văn bản được lấy trực tiếp bằng getter.

Các trường số nguyên sử dụng `to_string()` để chuyển thành chuỗi.

Trạng thái `active` được chuyển thành `"1"` hoặc `"0"` bằng toán tử ba ngôi.

### Kết quả

Ví dụ:

```text
MOV0001|Avengers|Action|180|13|2026-01-01|2026-12-31|1
```

Chuỗi trả về sẽ được `Repository<Movie>::save()` sử dụng khi ghi file.

### Lưu ý

Thứ tự các trường trong `movieToString()` phải tương ứng với thứ tự đọc của `stringToMovie()`.

Nếu thay đổi thứ tự ở một hàm mà không sửa hàm còn lại, dữ liệu có thể bị đọc sai.

---

## 9. Hàm load()

```cpp
bool MovieRepository::load()
{
    return repo.load();
}
```

### Chức năng

Nạp danh sách Movie từ file TXT vào bộ nhớ.

### Các bước xử lý bên trong Repository chung

Mặc dù `MovieRepository::load()` chỉ có một dòng, quá trình đọc dữ liệu bên trong `Repository<Movie>` gồm nhiều bước:

**Bước 1: Đọc file**

`Repository<Movie>` gọi `FileIO::readLines()` để đọc các dòng dữ liệu.

Nếu không đọc được file, `repo.load()` trả về `false`.

**Bước 2: Chuyển từng dòng thành Movie**

Repository chung gọi con trỏ hàm `parse`, đã được gán với `stringToMovie()` khi khởi tạo.

Mỗi dòng hợp lệ được chuyển thành một đối tượng Movie.

**Bước 3: Kiểm tra ID**

Repository kiểm tra ID của Movie và phát hiện ID trùng trong dữ liệu đang đọc.

Nếu có lỗi dữ liệu, quá trình load bị hủy và ngoại lệ được ném ra.

**Bước 4: Quản lý danh sách tạm**

Các Movie mới được đưa vào danh sách tạm trong quá trình đọc.

Nếu xảy ra lỗi, Repository giải phóng các đối tượng trong danh sách tạm để tránh rò rỉ bộ nhớ.

**Bước 5: Thay thế dữ liệu hiện tại**

Khi toàn bộ dữ liệu được đọc thành công, Repository giải phóng danh sách cũ và chuyển các Movie mới vào danh sách chính.

**Bước 6: Cập nhật trạng thái**

Repository đặt `loaded = true` và trả về `true`.

### Kết quả

- `true`: Đọc dữ liệu thành công.
- `false`: Không đọc được file.
- Ngoại lệ: Dữ liệu trong file không hợp lệ hoặc phát sinh lỗi xử lý.

### Lưu ý

`MovieRepository::load()` không tự kiểm tra trạng thái đã load trước đó.

Theo cách triển khai Repository chung hiện tại, việc gọi load lại thành công có thể thay thế các đối tượng Movie cũ.

Vì vậy, các con trỏ `Movie*` đã lấy trước lần load lại có thể không còn hợp lệ.

---

## 10. Hàm save()

```cpp
bool MovieRepository::save()
{
    return repo.save();
}
```

**Chức năng:** Ghi danh sách Movie hiện tại xuống file TXT.

Hàm gọi trực tiếp `Repository<Movie>::save()`.

Repository chung sẽ duyệt danh sách Movie, gọi `movieToString()` để chuyển từng Movie thành chuỗi và sử dụng `FileIO::writeLines()` để ghi file.

Hàm trả về `true` khi ghi thành công và `false` khi chưa load hoặc ghi file thất bại. Lỗi chuyển đổi dữ liệu vẫn có thể phát sinh ngoại lệ.

---

## 11. Hàm add()

```cpp
bool MovieRepository::add(Movie* movie)
```

### Chức năng

Thêm một Movie mới vào danh sách đang quản lý.

### Các bước xử lý

**Bước 1: Kiểm tra trạng thái và dữ liệu**

```cpp
if (!repo.isLoaded() || !isValidMovie(movie))
{
    return false;
}
```

Không cho phép thêm Movie khi Repository chưa load hoặc dữ liệu Movie không hợp lệ.

**Bước 2: Kiểm tra con trỏ trùng**

```cpp
for (int i = 0; i < repo.count(); i++)
{
    if (repo.getAt(i) == movie)
    {
        return false;
    }
}
```

Duyệt các Movie đang được Repository quản lý và so sánh địa chỉ con trỏ.

Nếu con trỏ `movie` đã tồn tại, hàm trả về `false`.

### Tại sao phải kiểm tra con trỏ trùng?

`Repository<Movie>` chịu trách nhiệm giải phóng các đối tượng Movie mà nó sở hữu.

Nếu cùng một con trỏ được thêm vào danh sách nhiều lần, quá trình giải phóng có thể gọi `delete` nhiều lần trên cùng một đối tượng.

Điều này gây lỗi quản lý bộ nhớ.

**Bước 3: Gọi Repository chung**

```cpp
return repo.add(movie);
```

Repository chung thực hiện thêm Movie và kiểm tra ID trùng.

### Kết quả

- `true`: Thêm Movie thành công.
- `false`: Chưa load, dữ liệu không hợp lệ, con trỏ trùng hoặc ID trùng.

### Quyền sở hữu bộ nhớ

Khi thêm thành công, `Repository<Movie>` nhận quyền sở hữu con trỏ Movie.

Nếu thêm thất bại, bên gọi vẫn chịu trách nhiệm quản lý con trỏ được truyền vào, trừ trường hợp đó vốn là con trỏ đã thuộc Repository.

Hàm `add()` chỉ cập nhật dữ liệu trong bộ nhớ. Cần gọi `save()` để ghi xuống file.

---

## 12. Hàm update()

```cpp
bool MovieRepository::update(const Movie& newValues)
```

### Chức năng

Cập nhật thông tin của một Movie đã tồn tại theo ID.

Hàm không thay thế đối tượng Movie bằng đối tượng mới mà cập nhật trực tiếp các thuộc tính thông qua setter.

### Các bước xử lý

**Bước 1: Kiểm tra trạng thái và dữ liệu mới**

```cpp
if (!repo.isLoaded() || !isValidMovie(&newValues))
{
    return false;
}
```

Chỉ cho phép cập nhật khi Repository đã load và dữ liệu mới hợp lệ.

**Bước 2: Tìm Movie cần cập nhật**

```cpp
Movie* movie = repo.findById(newValues.getMovieId());

if (movie == nullptr)
{
    return false;
}
```

Sử dụng ID trong `newValues` để tìm Movie đang tồn tại.

Nếu không tìm thấy, hàm trả về `false`.

**Bước 3: Cập nhật các thuộc tính**

```cpp
movie->setTitle(newValues.getTitle());
movie->setGenre(newValues.getGenre());
movie->setDurationMinutes(newValues.getDurationMinutes());
movie->setMinAge(newValues.getMinAge());
movie->setReleaseDate(newValues.getReleaseDate());
movie->setEndDate(newValues.getEndDate());
movie->setActive(newValues.isActive());
```

Các setter cập nhật trực tiếp đối tượng Movie đã tìm thấy.

**Bước 4: Trả về kết quả**

```cpp
return true;
```

Nếu thực hiện thành công, hàm trả về `true`.

### Tại sao không tạo Movie mới?

Movie có thể đang được các thành phần khác tham chiếu thông qua con trỏ hoặc các quan hệ dữ liệu.

Nếu xóa đối tượng cũ và thay bằng đối tượng mới, những con trỏ đang tham chiếu có thể không còn hợp lệ.

Việc cập nhật bằng setter giúp giữ nguyên địa chỉ đối tượng.

### Tại sao không cập nhật movieId?

`movieId` được sử dụng để xác định Movie cần cập nhật và là mã định danh của đối tượng.

Do đó, hàm chỉ cập nhật những trường thông tin khác, không thay đổi ID.

### Lưu ý

Hàm `update()` hiện tại không có cơ chế hoàn tác nếu một setter ném ngoại lệ sau khi các setter trước đã thực hiện thành công.

Ngoài ra, hàm không tự gọi `save()`.

---

## 13. Hàm removeById()

```cpp
bool MovieRepository::removeById(const string& movieId)
```

### Chức năng

Xóa một Movie theo ID, với điều kiện Movie đó không còn liên kết với Showtime.

### Các bước xử lý

**Bước 1: Kiểm tra trạng thái**

```cpp
if (!repo.isLoaded())
{
    return false;
}
```

Không cho phép xóa khi Repository chưa load.

**Bước 2: Tìm Movie**

```cpp
Movie* movie = repo.findById(movieId);

if (movie == nullptr)
{
    return false;
}
```

Nếu không tìm thấy Movie tương ứng, trả về `false`.

**Bước 3: Kiểm tra liên kết Showtime**

```cpp
if (movie->getShowtimeCount() > 0)
{
    return false;
}
```

Nếu Movie còn liên kết với ít nhất một Showtime, không cho phép xóa.

Điều kiện này giúp tránh xóa một Movie vẫn đang được sử dụng trong dữ liệu lịch chiếu.

**Bước 4: Xóa bằng Repository chung**

```cpp
return repo.removeById(movieId);
```

Khi các điều kiện đều thỏa mãn, gọi `Repository<Movie>::removeById()` để xóa Movie khỏi danh sách và giải phóng bộ nhớ.

### Kết quả

- `true`: Xóa thành công.
- `false`: Chưa load, không tìm thấy Movie hoặc Movie còn liên kết Showtime.

### Lưu ý

Sau khi xóa thành công, các con trỏ đang trỏ đến Movie bị xóa sẽ không còn hợp lệ.

Hàm chỉ thay đổi dữ liệu trong bộ nhớ. Cần gọi `save()` để lưu kết quả xuống file.

---

## 14. Hàm findById()

```cpp
Movie* MovieRepository::findById(const string& movieId)
{
    return repo.findById(movieId);
}
```

**Chức năng:** Tìm Movie theo ID.

Hàm gọi trực tiếp `Repository<Movie>::findById()`.

Repository chung duyệt danh sách, so sánh ID và trả về con trỏ đến Movie phù hợp.

Nếu không tìm thấy, trả về `nullptr`.

Độ phức tạp tìm kiếm là O(n).

---

## 15. Hàm getAt()

```cpp
Movie* MovieRepository::getAt(int index)
{
    return repo.getAt(index);
}
```

**Chức năng:** Lấy Movie tại vị trí `index` trong danh sách.

Hàm sử dụng `Repository<Movie>::getAt()`.

Ví dụ:

```cpp
Movie* movie = movieRepo.getAt(0);
```

Nếu vị trí không hợp lệ, trả về `nullptr`.

---

## 16. Hàm count()

```cpp
int MovieRepository::count()
{
    return repo.count();
}
```

**Chức năng:** Trả về số lượng Movie hiện đang được quản lý trong Repository.

Hàm gọi trực tiếp `Repository<Movie>::count()`.

---

## 17. Hàm isLoaded()

```cpp
bool MovieRepository::isLoaded()
{
    return repo.isLoaded();
}
```

**Chức năng:** Kiểm tra trạng thái dữ liệu đã được load hay chưa.

- `true`: Repository đã load thành công.
- `false`: Repository chưa load hoặc đã được đưa về trạng thái chưa load.

---

## 18. Quan hệ giữa MovieRepository và Repository&lt;T&gt;

| Chức năng | Repository&lt;Movie&gt; | MovieRepository |
|---|---|---|
| Đọc file | Thực hiện | Gọi lại |
| Ghi file | Thực hiện | Gọi lại |
| Quản lý bộ nhớ | Thực hiện | Sử dụng |
| Kiểm tra ID trùng | Thực hiện | Tận dụng |
| Kiểm tra dữ liệu Movie | Không biết quy tắc cụ thể | Thực hiện |
| Chuyển chuỗi thành Movie | Gọi hàm được truyền vào | Cung cấp hàm |
| Chuyển Movie thành chuỗi | Gọi hàm được truyền vào | Cung cấp hàm |
| Thêm Movie | Lưu đối tượng | Kiểm tra dữ liệu và con trỏ |
| Cập nhật Movie | Không hỗ trợ trực tiếp | Thực hiện |
| Xóa Movie | Xóa và giải phóng bộ nhớ | Kiểm tra liên kết Showtime |
| Tìm kiếm theo ID | Duyệt danh sách | Gọi lại |

### Nguyên tắc thiết kế

`Repository<T>` quản lý các thao tác dữ liệu tổng quát.

`MovieRepository` bổ sung các quy tắc dữ liệu riêng của Movie.

`MovieService` sử dụng MovieRepository để thực hiện các chức năng ở tầng Business.

Việc phân chia này giúp hạn chế viết lại những đoạn code giống nhau giữa các Repository cụ thể.

---

## 19. Những lưu ý quan trọng

**1. Repository chung sở hữu các Movie**

Khi `add()` thành công hoặc `load()` thành công, Repository chịu trách nhiệm quản lý và giải phóng các đối tượng Movie tương ứng.

Không được tự ý `delete` một Movie vẫn đang được Repository quản lý.

**2. Không tự ý sửa dữ liệu qua con trỏ**

Các hàm `findById()` và `getAt()` trả về `Movie*`, cho phép truy cập trực tiếp đối tượng.

Nếu lớp gọi sử dụng setter mà không thông qua `MovieRepository::update()`, các bước kiểm tra dữ liệu của Repository có thể bị bỏ qua.

Vì vậy, nên cập nhật thông tin phim thông qua phương thức quản lý phù hợp.

**3. Cần lưu dữ liệu sau khi thay đổi**

`add()`, `update()` và `removeById()` chỉ thay đổi dữ liệu trong bộ nhớ.

Cần gọi `save()` để ghi thay đổi xuống file TXT.

**4. Cẩn thận khi load lại**

`Repository<T>::load()` hiện tại có thể thay thế danh sách cũ sau khi đọc file thành công.

Do đó, không nên tiếp tục sử dụng các con trỏ Movie lấy trước lần load lại.

**5. Điều kiện xóa phim**

`removeById()` không cho phép xóa Movie khi `getShowtimeCount() > 0`.

Điều kiện này phụ thuộc vào việc các liên kết Showtime của Movie được duy trì chính xác.

**6. Chưa có cơ chế cập nhật nguyên tử hoàn toàn**

Nếu một setter trong `update()` ném ngoại lệ, các thuộc tính đã cập nhật trước đó có thể không được khôi phục.

Đây là điểm có thể cải thiện khi hoàn thiện xử lý lỗi.

---

## 20. Tổng kết

`MovieRepository` là lớp quản lý dữ liệu phim được xây dựng dựa trên `Repository<Movie>`.

Lớp này tái sử dụng các chức năng chung để quản lý danh sách, đọc ghi file và quản lý bộ nhớ, đồng thời bổ sung những quy tắc đặc thù của Movie.

Các hàm chuyển đổi dữ liệu và kiểm tra tính hợp lệ giúp bảo đảm dữ liệu phim tuân thủ định dạng TXT và các điều kiện đã quy định.

Nhờ đó, MovieRepository có thể cung cấp dữ liệu cho MovieService mà không cần viết lại toàn bộ chức năng quản lý file và danh sách.