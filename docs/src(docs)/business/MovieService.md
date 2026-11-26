# MovieService — Thiết kế refactor

**Trạng thái:** CHỐT MARKDOWN, đã triển khai và kiểm thử thành công trong phạm vi `tests/test_movie_service.cpp` ngày 26/11/2026; MovieService hoàn thành refactor DIP, DI và Permission-only trong phạm vi này. Giữ nguyên 12 hàm public; `.h/.cpp` khớp hai khối code Markdown 100%.

**Mốc:** [5d663e3](https://github.com/duyhoangbk2007/PBL2/commit/5d663e38110b305524338a21a85ebed1a61f42f6), nhánh refactor/hoang-backend, 26/11/2026.

## 1. Tổng quan

MovieService thuộc Business Logic, kiểm tra quyền, sinh ID, xử lý nghiệp vụ phim và điều phối Repository. Không trực tiếp đọc ghi TXT. Giữ 12 hàm public hiện tại và constructor; không thêm Service mới.

Nguồn đối chiếu: AGENTS.md, docs/architecture/validation_rules.md, docs/overview/business_rules.md, src/business/MovieService.h/.cpp, AuthService.h, src/data_access/IRepository.h, Repository.h, MovieRepository.h/.cpp tại mốc trên.

## 2. Kiến trúc

```text
GUI -> MovieService -> IRepository<Movie> <- MovieRepository
            |                                      |
            v                                      v
        AuthService                         Repository<Movie>
```

Service nhận tham chiếu qua constructor (DI), phụ thuộc interface (DIP). AuthService giữ nguyên dependency hiện có. Repository quản lý Movie và kiểm tra dữ liệu; Service kiểm tra Permission và nghiệp vụ. Không sở hữu hai dependency.

## 3. Thuộc tính và API

| Thuộc tính | Kiểu | Vai trò |
|---|---|---|
| movieRepo | IRepository<Movie>& | CRUD, truy vấn, load/save qua interface |
| authService | AuthService& | Kiểm tra Permission |

| Hàm | Permission |
|---|---|
| Constructor, load, save | Không kiểm tra trực tiếp; giữ hợp đồng điều phối cũ |
| addMovie | MOVIE_ADD |
| updateMovie, setMovieActive | MOVIE_UPDATE |
| removeMovie | MOVIE_DELETE |
| findMovieById, getMovieAt, getMovieCount, getNowShowingMovies, getComingSoonMovies, searchMoviesByTitle | MOVIE_VIEW |

Thiếu quyền: truyền ngoại lệ của requirePermission, không chuyển thành false. Lỗi nghiệp vụ: false; truy vấn không có kết quả: nullptr/vector rỗng theo API. Không tự save sau CRUD. Các con trỏ truy vấn là mượn, không delete và không dùng sau khi đối tượng bị xóa.

## 4. Mã nguồn đã chốt

### 4.1. MovieService.h

```cpp
#ifndef MOVIE_SERVICE_H
#define MOVIE_SERVICE_H

#include <string>
#include <vector>
#include "../data_access/IRepository.h"
#include "../models/Movie.h"
#include "AuthService.h"

class MovieService
{
private:
    IRepository<Movie>& movieRepo;
    AuthService& authService;

public:
    MovieService(IRepository<Movie>& movieRepo,
                 AuthService& authService);

    // Quan ly du lieu
    bool load();
    bool save();

    // Quan ly phim
    bool addMovie(Movie* movie);
    bool updateMovie(const Movie& newValues);
    bool removeMovie(const std::string& movieId);
    bool setMovieActive(const std::string& movieId, bool active);

    // Tra cuu phim
    const Movie* findMovieById(const std::string& movieId);
    const Movie* getMovieAt(int index);
    int getMovieCount();
    std::vector<const Movie*> getNowShowingMovies();
    std::vector<const Movie*> getComingSoonMovies();
    std::vector<const Movie*> searchMoviesByTitle(const std::string& keyword);
};

#endif
```

### 4.2. MovieService.cpp

```cpp
#include "MovieService.h"
#include "../models/Showtime.h"
#include "../utils/DateTime.h"
#include "../utils/IdGenerator.h"
#include <cctype>
using namespace std;

MovieService::MovieService(IRepository<Movie>& movieRepo,
                           AuthService& authService)
    : movieRepo(movieRepo), authService(authService)
{
}

bool MovieService::load()
{
    return movieRepo.load();
}

bool MovieService::save()
{
    return movieRepo.save();
}

bool MovieService::addMovie(Movie* movie)
{
    authService.requirePermission("MOVIE_ADD");

    if (!movieRepo.isLoaded() || movie == nullptr)
    {
        return false;
    }

    vector<string> existingIds;
    for (int i = 0; i < movieRepo.count(); i++)
    {
        const Movie* current = movieRepo.getAt(i);
        if (current == nullptr)
        {
            return false;
        }
        existingIds.push_back(current->getMovieId());
    }

    string movieId = IdGenerator::generateNextId("MOV", existingIds);
    Movie* newMovie = new Movie(movieId,
                               movie->getTitle(),
                               movie->getGenre(),
                               movie->getDurationMinutes(),
                               movie->getMinAge(),
                               movie->getReleaseDate(),
                               movie->getEndDate(),
                               movie->isActive());
    try
    {
        if (!movieRepo.add(newMovie))
        {
            delete newMovie;
            return false;
        }
    }
    catch (...)
    {
        delete newMovie;
        throw;
    }
    return true;
}

bool MovieService::updateMovie(const Movie& newValues)
{
    authService.requirePermission("MOVIE_UPDATE");

    if (!movieRepo.isLoaded())
    {
        return false;
    }
    const Movie* movie = movieRepo.findById(newValues.getMovieId());
    if (movie == nullptr)
    {
        return false;
    }
    if (movie->isActive() != newValues.isActive())
    {
        return false;
    }

    bool dateChanged =
        movie->getReleaseDate() != newValues.getReleaseDate() ||
        movie->getEndDate() != newValues.getEndDate();
    bool durationChanged =
        movie->getDurationMinutes() != newValues.getDurationMinutes();

    if (movie->getShowtimeCount() > 0 &&
        (dateChanged || durationChanged))
    {
        return false;
    }
    return movieRepo.update(newValues);
}

bool MovieService::removeMovie(const string& movieId)
{
    authService.requirePermission("MOVIE_DELETE");
    return movieRepo.removeById(movieId);
}

bool MovieService::setMovieActive(const string& movieId, bool active)
{
    authService.requirePermission("MOVIE_UPDATE");

    if (!movieRepo.isLoaded())
    {
        return false;
    }
    Movie* movie = movieRepo.findById(movieId);
    if (movie == nullptr)
    {
        return false;
    }
    if (movie->isActive() == active)
    {
        return true;
    }
    if (!active)
    {
        string now = DateTime::getCurrentDateTime();
        for (int i = 0; i < movie->getShowtimeCount(); i++)
        {
            const Showtime* showtime = movie->getShowtimeAt(i);
            if (showtime == nullptr)
            {
                return false;
            }
            if (showtime->getStatus() == ShowtimeStatus::SCHEDULED &&
                DateTime::isBefore(now, showtime->getStartAt()))
            {
                return false;
            }
        }
    }

    Movie updated(movie->getMovieId(),
                  movie->getTitle(),
                  movie->getGenre(),
                  movie->getDurationMinutes(),
                  movie->getMinAge(),
                  movie->getReleaseDate(),
                  movie->getEndDate(),
                  active);
    return movieRepo.update(updated);
}

const Movie* MovieService::findMovieById(const string& movieId)
{
    authService.requirePermission("MOVIE_VIEW");
    return movieRepo.findById(movieId);
}

const Movie* MovieService::getMovieAt(int index)
{
    authService.requirePermission("MOVIE_VIEW");
    return movieRepo.getAt(index);
}

int MovieService::getMovieCount()
{
    authService.requirePermission("MOVIE_VIEW");
    return movieRepo.count();
}

vector<const Movie*> MovieService::getNowShowingMovies()
{
    authService.requirePermission("MOVIE_VIEW");

    vector<const Movie*> result;
    if (!movieRepo.isLoaded())
    {
        return result;
    }
    string today = DateTime::getCurrentDateTime().substr(0, 10);
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
        {
            result.push_back(movie);
        }
    }
    return result;
}

vector<const Movie*> MovieService::getComingSoonMovies()
{
    authService.requirePermission("MOVIE_VIEW");

    vector<const Movie*> result;
    if (!movieRepo.isLoaded())
    {
        return result;
    }
    string today = DateTime::getCurrentDateTime().substr(0, 10);
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
            result.push_back(movie);
        }
    }
    return result;
}

vector<const Movie*> MovieService::searchMoviesByTitle(const string& keyword)
{
    authService.requirePermission("MOVIE_VIEW");

    vector<const Movie*> result;
    if (!movieRepo.isLoaded())
    {
        return result;
    }
    string searchKey = keyword;
    for (char& c : searchKey)
    {
        c = static_cast<char>(tolower(static_cast<unsigned char>(c)));
    }
    for (int i = 0; i < movieRepo.count(); i++)
    {
        const Movie* movie = movieRepo.getAt(i);
        if (movie == nullptr)
        {
            continue;
        }
        string title = movie->getTitle();
        for (char& c : title)
        {
            c = static_cast<char>(tolower(static_cast<unsigned char>(c)));
        }
        if (title.find(searchKey) != string::npos)
        {
            result.push_back(movie);
        }
    }
    return result;
}
```

Header guard chống include lặp. string và vector được include trực tiếp; Movie.h cung cấp Model, IRepository.h cung cấp hợp đồng, AuthService.h cung cấp kiểm tra quyền. Không using namespace std trong header. CPP dùng Showtime để đọc trạng thái suất, DateTime cho ngày giờ, IdGenerator sinh mã và cctype cho tolower; bỏ algorithm không dùng.

## 5. Giải thích từng hàm

### 5.1. Constructor

**Tổng quan:** Nhận hai dependency bắt buộc, không sở hữu chúng.

**Luồng hoạt động:**

```text
Nhận dependency -> Khởi tạo hai tham chiếu -> Kết thúc
```

**Giải thích code:** Initializer list gắn thuộc tính tham chiếu với đối tượng bên ngoài; không sao chép, không cấp phát. Thân hàm rỗng vì không cần xử lý thêm. Hai dependency phải sống lâu hơn Service.

### 5.2. load()

**Tổng quan:** Nạp dữ liệu, trả bool theo Repository.

**Luồng hoạt động:**

```text
Gọi Repository -> Trả kết quả hoặc truyền ngoại lệ
```

**Giải thích code:** Lời gọi qua interface thực thi hàm override của Repository thực tế. Không kiểm tra loaded lặp, không bắt ngoại lệ. Giữ API điều phối hiện tại, không dùng làm nghiệp vụ sửa dữ liệu của GUI.

### 5.3. save()

**Tổng quan:** Lưu dữ liệu, trả bool theo Repository.

**Luồng hoạt động:**

```text
Gọi Repository -> Trả kết quả hoặc truyền ngoại lệ
```

**Giải thích code:** Lời gọi qua interface thực thi hàm override của Repository thực tế. Không kiểm tra loaded lặp, không bắt ngoại lệ. Giữ API điều phối hiện tại, không dùng làm nghiệp vụ sửa dữ liệu của GUI.

### 5.4. addMovie()

**Tổng quan:** Thêm phim mới từ dữ liệu đầu vào; caller vẫn sở hữu movie. Repository nhận quyền sở hữu bản mới khi add thành công.

**Luồng hoạt động:**

```text
Permission? -- thiếu --> ngoại lệ
  | đủ
Loaded và movie hợp lệ về con trỏ? -- không --> false
  | có
Duyệt ID -- nullptr --> false
  |
Sinh ID -> Tạo bản mới -> repo.add
  | false: delete -> false
  | ngoại lệ: delete -> throw
  | true: chuyển sở hữu -> true
```

**Giải thích code:** requirePermission kiểm tra trước mọi xử lý. loaded và nullptr cần thiết vì Service duyệt dữ liệu và đọc input. vector<string> chứa ID; vòng for duyệt count/getAt, kiểm tra con trỏ trước getter. IdGenerator cấp mã MOV theo dữ liệu đang nạp; ngoại lệ sinh ID truyền lên. new Movie sao chép từng thuộc tính nghiệp vụ, không sao chép liên kết hay ID input. try chỉ bao quanh add; false hoặc ngoại lệ trước khi chuyển sở hữu thì delete bản mới; throw giữ ngoại lệ gốc. Thành công không delete vì Repository sở hữu. Không tự save.

### 5.5. updateMovie()

**Tổng quan:** Cập nhật cùng phim theo ID, giữ quy tắc hiện tại không đổi active qua hàm này.

**Luồng hoạt động:**

```text
Permission -> loaded? -> tìm phim -> tồn tại? -> active không đổi?
  | bất kỳ điều kiện sai: false (thiếu quyền: ngoại lệ)
  v
Có suất và đổi ngày/thời lượng? -- có --> false
  | không
repo.update -> trả kết quả
```

**Giải thích code:** const Movie* chỉ đọc đối tượng đang lưu. So sánh active để buộc dùng setMovieActive. dateChanged dùng || để phát hiện một trong hai ngày thay đổi; durationChanged so sánh thời lượng. && chặn khi đã có suất và có thay đổi liên quan. Repository kiểm tra thuộc tính và cập nhật, Service không gọi setter. Giữ quy tắc cũ nghiêm hơn tài liệu mục 7; cần nhóm xác nhận trước khi nới lỏng.

### 5.6. removeMovie()

**Tổng quan:** Xóa phim theo ID, trả kết quả Repository.

**Luồng hoạt động:**

```text
Permission? -- thiếu --> ngoại lệ
  | đủ -> removeById -> kết quả
```

**Giải thích code:** Không tìm phim trước vì không cần truy cập đối tượng. Repository kiểm tra liên kết Showtime, loaded và tồn tại; không tự xóa suất.

### 5.7. setMovieActive()

**Tổng quan:** Đổi trạng thái hoạt động, không hủy suất tự động.

**Luồng hoạt động:**

```text
Permission -> loaded? -> tìm phim -> tồn tại?
  | sai: false (thiếu quyền: ngoại lệ)
Trạng thái giống yêu cầu? -- có --> true
  | không
Ngừng hoạt động? -- có --> duyệt suất
  |                         | nullptr hoặc SCHEDULED tương lai: false
  | không                   | hết vòng
  +-------------------------+
Tạo Movie tạm -> repo.update -> kết quả
```

**Giải thích code:** Kiểm tra quyền, loaded, con trỏ trước khi đọc. Nếu active không đổi, trả true không ghi. Khi !active, lấy now một lần; duyệt các suất liên kết, từ chối nullptr và suất SCHEDULED có startAt sau now. Suất hủy hoặc đã bắt đầu không chặn theo code cũ. Movie updated là đối tượng stack, giữ ID và thuộc tính cũ, chỉ thay active; update nhận const reference, không chuyển sở hữu. Cần nạp Showtime trước nghiệp vụ để danh sách liên kết đầy đủ.

### 5.8. findMovieById()

**Tổng quan:** Truy vấn theo ID, nullptr nếu không tìm thấy.

**Luồng hoạt động:**

```text
Permission? -- thiếu --> ngoại lệ
  | đủ -> truy vấn Repository -> trả kết quả
```

**Giải thích code:** Kiểm tra MOVIE_VIEW trước khi truy vấn. Không kiểm tra lặp điều kiện lưu trữ. Chuyển Movie* sang const Movie* để caller không sửa trực tiếp; con trỏ mượn, không delete.

### 5.9. getMovieAt()

**Tổng quan:** Truy vấn theo vị trí, nullptr nếu ngoài phạm vi.

**Luồng hoạt động:**

```text
Permission? -- thiếu --> ngoại lệ
  | đủ -> truy vấn Repository -> trả kết quả
```

**Giải thích code:** Kiểm tra MOVIE_VIEW trước khi truy vấn. Không kiểm tra lặp điều kiện lưu trữ. Chuyển Movie* sang const Movie* để caller không sửa trực tiếp; con trỏ mượn, không delete.

### 5.10. getMovieCount()

**Tổng quan:** Truy vấn số phim đang nạp, 0 nếu rỗng.

**Luồng hoạt động:**

```text
Permission? -- thiếu --> ngoại lệ
  | đủ -> truy vấn Repository -> trả kết quả
```

**Giải thích code:** Kiểm tra MOVIE_VIEW trước khi truy vấn. Không kiểm tra lặp điều kiện lưu trữ. Trả số lượng do Repository quản lý, không duyệt lại.

### 5.11. getNowShowingMovies()

**Tổng quan:** Phim active và ngày hiện tại nằm trong khoảng chiếu, gồm hai đầu.

**Luồng hoạt động:**

```text
Permission -> loaded? -- không --> vector rỗng
  | có -> lấy ngày -> duyệt phim
  | nullptr: bỏ qua; thỏa điều kiện: thêm vào result
  v
Trả result
```

**Giải thích code:** vector chứa con trỏ mượn chỉ đọc. loaded kiểm tra trước khi duyệt. substr(0, 10) lấy YYYY-MM-DD; so sánh chuỗi đúng với ngày chuẩn hóa. Vòng for duyệt thứ tự Repository; continue tránh dereference nullptr. Điều kiện lọc: Phim active và ngày hiện tại nằm trong khoảng chiếu, gồm hai đầu. push_back thêm con trỏ, không sao chép Movie. Trả vector theo giá trị.

### 5.12. getComingSoonMovies()

**Tổng quan:** Phim active có ngày bắt đầu sau hôm nay.

**Luồng hoạt động:**

```text
Permission -> loaded? -- không --> vector rỗng
  | có -> lấy ngày -> duyệt phim
  | nullptr: bỏ qua; thỏa điều kiện: thêm vào result
  v
Trả result
```

**Giải thích code:** vector chứa con trỏ mượn chỉ đọc. loaded kiểm tra trước khi duyệt. substr(0, 10) lấy YYYY-MM-DD; so sánh chuỗi đúng với ngày chuẩn hóa. Vòng for duyệt thứ tự Repository; continue tránh dereference nullptr. Điều kiện lọc: Phim active có ngày bắt đầu sau hôm nay. push_back thêm con trỏ, không sao chép Movie. Trả vector theo giá trị.

### 5.13. searchMoviesByTitle()

**Tổng quan:** Tìm chuỗi con trong tên phim, giữ hành vi hiện tại kể cả phim inactive và keyword rỗng.

**Luồng hoạt động:**

```text
Permission -> loaded? -- không --> rỗng
  | có -> chuyển keyword chữ thường -> duyệt phim
  | nullptr: bỏ qua
  | chuyển bản sao title -> tìm chuỗi con -> thêm nếu khớp
  v
Trả result
```

**Giải thích code:** Sao chép keyword và title để không sửa dữ liệu gốc. char& trong range-for sửa từng byte. Ép unsigned char trước tolower tránh undefined behavior với char âm; ép char để lưu lại. find trả vị trí hoặc string::npos; keyword rỗng khớp mọi tên. Không lọc active. Đây không phải Unicode case folding, chưa hỗ trợ đầy đủ chữ hoa/thường tiếng Việt UTF-8.

## 6. Nội dung thay đổi và điểm cần chốt

- Thay dependency MovieRepository& bằng IRepository<Movie>&; bổ sung include trực tiếp, căn chỉnh thứ tự .h/.cpp, dùng const Movie* tại các vị trí chỉ đọc. Không thay đổi chữ ký 12 hàm nghiệp vụ.
- Giữ sinh ID ở Service theo validation_rules.md; Repository hiện yêu cầu ID hợp lệ và không tự sinh. ID lớn nhất bị xóa có thể được cấp lại; business_rules.md ghi rõ còn chờ xác nhận. Không tự sửa IdGenerator.
- Giữ updateMovie chặn mọi thay đổi ngày/thời lượng nếu có suất. Đây là hành vi cũ nghiêm hơn quy tắc chỉ chặn ngày mới làm suất tương lai nằm ngoài khoảng chiếu. Cần chốt nếu muốn thay đổi nghiệp vụ.
- Giữ load/save công khai và không Permission như hiện tại; chỉ dùng điều phối, cần xác nhận với nhóm nếu muốn thay API hoặc bảo vệ save.
- setMovieActive cần dữ liệu Showtime đã nạp đầy đủ; MovieService không có dependency để tự kiểm chứng điều này. Không thêm dependency khi chưa chốt. Cần test tích hợp.
- Tìm kiếm giữ hành vi byte-based, không bảo đảm không phân biệt hoa thường Unicode. Không tự bổ sung thư viện.
- addMovie giữ quyền sở hữu input ở caller; Repository nhận bản sao mới. Cần kiểm tra các caller GUI/test hiện có để tránh rò rỉ. Giả định add không ném ngoại lệ sau khi đã nhận sở hữu, phù hợp hợp đồng hiện tại.
- AuthService chưa refactor DIP; không sửa ngoài phạm vi MovieService.

## 7. Kiểm thử

`tests/test_movie_service.cpp` đã biên dịch C++17 cùng toàn bộ `.cpp` trong `src/` trừ `main.cpp` và chạy ngày 26/11/2026: **Passed: 41, Failed: 0, Total: 41**. Bài test dùng TXT riêng trong thư mục tạm và dọn dẹp sau khi chạy.

- [x] Constructor nhận `IRepository<Movie>&` qua concrete; 12 hàm public được gọi trong bài test.
- [x] Load/save: file rỗng, bản ghi hợp lệ, file thiếu, dữ liệu sai, load lặp, save trước load; lỗi parse được truyền ra.
- [x] Permission-only: chưa đăng nhập, ADMIN không được cấp quyền, STAFF được cấp quyền, tài khoản bị vô hiệu hóa; quyền VIEW/ADD/UPDATE/DELETE trong các ca đã chạy.
- [x] Add/update/remove: sinh ID đầu và kế tiếp, hết dải ID, input vẫn do caller sở hữu, validation thất bại giữ dữ liệu, cập nhật giữ địa chỉ/ID, chặn xóa phim có suất.
- [x] Bật/tắt phim: suất tương lai SCHEDULED chặn ngừng hoạt động, suất CANCELLED không chặn, thao tác lặp và kích hoạt lại.
- [x] Truy vấn, lọc và tìm kiếm: ID/index biên, danh sách rỗng, ngày bắt đầu/kết thúc, phim inactive, chuỗi con ASCII và từ khóa rỗng.
- [ ] Chưa kiểm thử Repository giả lập, ngoại lệ phát sinh bên trong `MovieRepository::add()` sau khi cấp phát bản sao, lỗi ghi file, Unicode case folding, hồi quy GUI/caller và tích hợp load Showtime thực tế. Bài test liên kết Showtime trực tiếp với Movie để kiểm tra quy tắc của Service.

Lệnh PowerShell đã dùng để biên dịch và chạy:

```powershell
$src = Get-ChildItem src -Recurse -Filter *.cpp | Where-Object { $_.Name -ne 'main.cpp' } | ForEach-Object { $_.FullName }
g++ -std=c++17 -I src tests/test_movie_service.cpp $src -o test_movie_service.exe
if ($LASTEXITCODE -eq 0) { .\test_movie_service.exe }
```

Kết quả trên chỉ xác nhận các ca đã chạy; các giới hạn còn lại được ghi trong checklist.

## 8. Bàn giao

Đã đổi `const Movie* movie` thành `Movie* movie` trong `setMovieActive()` để gọi `Movie::getShowtimeAt(int)` theo API hiện tại; giữ nguyên `const Showtime*` và logic nghiệp vụ. Hai khối code đã được đối chiếu với `src/business/MovieService.h/.cpp`: MATCH 100% cho cả hai. Biên dịch và liên kết C++17 với toàn bộ `.cpp` phù hợp trong `src/` thành công; chạy `tests/test_movie_service.cpp` đạt **Passed: 41, Failed: 0, Total: 41** ngày 26/11/2026. Hoàn thành MovieService trong phạm vi đã triển khai và kiểm thử; chưa xác nhận các giới hạn nêu ở mục 7. Dùng MovieService làm mẫu về cấu trúc, DIP/DI, Permission-only và bố cục tài liệu cho các Service tiếp theo, điều chỉnh logic theo từng Service. Không tự commit/push/merge.
