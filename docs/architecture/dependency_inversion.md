# Dependency Inversion trong PBL2

> **Trạng thái: tài liệu giải thích và ví dụ minh họa, chưa CHỐT MARKDOWN.** Code dưới đây không phải mã nguồn chuẩn để chép vào `src/`. Việc thống nhất chữ ký interface và triển khai phải qua quy trình review trong [AGENTS.md](../../AGENTS.md).

## 1. Tổng quan

**Dependency Inversion Principle (DIP)** yêu cầu phần xử lý nghiệp vụ phụ thuộc vào một hợp đồng trừu tượng, thay vì phụ thuộc trực tiếp lớp lưu dữ liệu cụ thể. Trong PBL2, Service cần biết *có thể yêu cầu đọc hoặc lưu đối tượng bằng những hàm nào*; Service không cần biết repository dùng `DoublyLinkedList`, callback parse/serialize hay file TXT ra sao.

Ví dụ hiện tại: `src/business/MovieService.h` include `MovieRepository.h`, giữ `MovieRepository& movieRepo` và constructor nhận `MovieRepository&`. `src/data_access/MovieRepository.h` lại chứa `Repository<Movie> repo`. Nếu Service chuyển sang nhận `IRepository<Movie>&`, Service chỉ dựa vào những hàm công khai của interface. `MovieRepository` thực hiện interface và tiếp tục dùng `Repository<Movie>` bên trong.

Tài liệu này **chỉ dùng một loại interface chung `IRepository<T>`** để Hoàng và Kiệt thấy rõ cơ chế DIP mà không phải học thêm một cây interface theo từng lớp. `T` là kiểu Model: `IRepository<Movie>` và `IRepository<Room>` là hai cách dùng của cùng một template. Cách này giúp các thao tác thật sự giống nhau có một hợp đồng thống nhất; nó không tự làm mọi API hiện tại giống nhau. **CHỜ XÁC NHẬN:** một số repository hiện có chữ ký hoặc khả năng khác với CRUD mẫu, nên nhóm phải quyết định phạm vi hàm chung trước khi triển khai. Ví dụ bên dưới chỉ dùng Movie và không khẳng định bản mẫu áp dụng nguyên xi cho toàn bộ code hiện tại.

### Phân biệt hai lớp có tên gần giống

| Thành phần | Vai trò | Hiện trạng |
| --- | --- | --- |
| `IRepository<T>` | Hợp đồng trừu tượng: khai báo hàm Service được phép gọi; không giữ dữ liệu, đường dẫn hay quyền sở hữu `T*`. | **Chưa có trong `src/`**; code bên dưới chỉ minh họa. |
| `Repository<T>` | Lớp lưu trữ hiện có trong `src/data_access/Repository.h`: giữ `filePath`, danh sách `T*`, cờ `loaded` và callback ID/parse/serialize; thực hiện load/save/CRUD, xóa đối tượng khi clear/hủy. | **Đã có**; không đổi thành interface trong ví dụ. |
| `MovieRepository` | Lớp cụ thể kiểm tra dữ liệu Movie và chuyển lời gọi tới `Repository<Movie>`. | **Đã có**; hiện chưa kế thừa interface. |

## 2. Cơ chế hoạt động

### Sơ đồ phụ thuộc

```text
Trước DIP — đúng với code hiện tại

MovieService ----include/gọi----> MovieRepository
                                      |
                                      v
                                Repository<Movie> ----> TXT
```

```text
Sau DIP — mô hình minh họa, chưa triển khai

MovieService ----include/gọi----> IRepository<Movie> <----kế thừa---- MovieRepository
                                                                  |
                                                                  v
                                                            Repository<Movie> ----> TXT

Nơi khởi tạo: tạo MovieRepository rồi truyền nó vào MovieService dưới dạng
IRepository<Movie>&. MovieService không tự tạo MovieRepository.
```

**Service** giữ quy tắc nghiệp vụ và gọi các thao tác dữ liệu qua interface. **`IRepository<T>`** chỉ công bố chữ ký hàm. **Concrete Repository** như `MovieRepository` cung cấp phần thân các hàm, kiểm tra dữ liệu và bảo đảm tính toàn vẹn. **`Repository<T>` hiện có** giữ danh sách đối tượng và cơ chế TXT. Interface không thay thế lớp lưu trữ đó.

### `virtual`, `= 0`, `override` và kế thừa

Trong ví dụ C++17 dưới đây, `virtual bool load() = 0;` là **pure virtual function**: interface nói rằng phải có hàm `load()` trả `bool`, nhưng không tự định nghĩa cách đọc file. Vì còn hàm thuần ảo, không thể tạo trực tiếp `IRepository<Movie>`. `MovieRepository : public IRepository<Movie>` nghĩa là MovieRepository kế thừa công khai hợp đồng; nó phải cung cấp các hàm thuần ảo để có thể được tạo thành object.

`bool load() override` đặt ở lớp con. Từ khóa `override` yêu cầu compiler xác nhận hàm đó thực sự ghi đè một hàm `virtual` cùng chữ ký; nếu viết nhầm tên, kiểu trả về hoặc `const`, compiler báo lỗi. Khi Service giữ `IRepository<Movie>&` và gọi `load()`, C++ dùng **virtual dispatch** để chạy `MovieRepository::load()`. Destructor của interface nên là `virtual`: nếu về sau có code xóa object qua con trỏ interface, destructor lớp con vẫn được gọi. Ví dụ này truyền **tham chiếu**, nên Service không sở hữu và không xóa repository.

### Dependency Injection qua constructor

```text
Nơi khởi tạo tạo MovieRepository
|
v
Truyền object vào constructor MovieService(IRepository<Movie>&)
|
v
MovieService giữ tham chiếu interface
|
v
Service gọi load()/getAt()/count() trên tham chiếu
|
v
C++ chuyển lời gọi virtual tới MovieRepository tương ứng
|
v
MovieRepository dùng Repository<Movie> để truy xuất/lưu trữ
```

Đây là **constructor injection**: dependency được cung cấp từ bên ngoài lúc tạo Service. Tham chiếu `&` không thể là `nullptr`, nhưng object được tham chiếu phải sống lâu hơn Service. Việc tạo, nạp và hủy repository thuộc nơi khởi tạo ứng dụng hoặc fixture test; Service không biết đường dẫn TXT.

## 3. Ví dụ minh họa với MovieRepository và MovieService

Các đoạn code ở phần này **cố ý rút gọn để giải thích cơ chế**, không phải `.h/.cpp` hoàn chỉnh của PBL2. Chúng dùng các tên hàm thực sự có trong MovieRepository/MovieService hiện tại (`load`, `getAt`, `count`) nhưng bỏ qua các thành viên và quy tắc nghiệp vụ khác. Không dùng các đoạn này để thay file hiện có.

### 3.1 Interface chung, chỉ lấy các thao tác cần cho ví dụ

```cpp
#ifndef IREPOSITORY_H
#define IREPOSITORY_H

template <typename T>
class IRepository
{
public:
    virtual ~IRepository() = default;
    virtual bool load() = 0;
    virtual T* getAt(int index) = 0;
    virtual int count() = 0;
};

#endif
```

**Đoạn 1 — bảo vệ header và kiểu dữ liệu:**

```cpp
#ifndef IREPOSITORY_H
#define IREPOSITORY_H

template <typename T>
class IRepository
{
public:
```

`#ifndef`/`#define` ngăn cùng header bị định nghĩa lặp khi nhiều file include nó. `template <typename T>` cho phép thay `T` bằng `Movie` mà không viết lại lớp. `IRepository<Movie>` là kiểu hợp đồng cho dữ liệu Movie, không phải object Movie. `public` cho phép Service gọi các hàm bên dưới.

**Đoạn 2 — hợp đồng hàm:**

```cpp
    virtual ~IRepository() = default;
    virtual bool load() = 0;
    virtual T* getAt(int index) = 0;
    virtual int count() = 0;
};

#endif
```

`virtual ~IRepository() = default` tạo destructor ảo với phần thân mặc định; interface không sở hữu danh sách `T*`. `load()` nhận không tham số, trả `true/false` theo implementation và có thể ném lỗi nếu implementation quy định. `getAt(int index)` nhận vị trí, trả `T*` **mượn** hoặc `nullptr` nếu không có đối tượng; Service không `delete` con trỏ này. `count()` trả số đối tượng. Dấu `= 0` buộc lớp cụ thể thực hiện ba hàm, không có ý nghĩa “trả về 0”. Không thêm `findById` hay CRUD vào ví dụ vì chưa thống nhất được một chữ ký chung cho mọi repository.

### 3.2 Concrete Repository cung cấp phần thực hiện

```cpp
#include "IRepository.h"
#include "Repository.h"
#include "../models/Movie.h"

class MovieRepository : public IRepository<Movie>
{
private:
    Repository<Movie> repo;

public:
    bool load() override
    {
        return repo.load();
    }

    Movie* getAt(int index) override
    {
        return repo.getAt(index);
    }

    int count() override
    {
        return repo.count();
    }
};
```

**Đoạn 1 — kế thừa và lưu trữ:**

```cpp
#include "IRepository.h"
#include "Repository.h"
#include "../models/Movie.h"

class MovieRepository : public IRepository<Movie>
{
private:
    Repository<Movie> repo;
```

Các `#include` cung cấp hợp đồng, lớp lưu trữ và Model. `: public IRepository<Movie>` cho phép một `MovieRepository` được truyền tới nơi cần `IRepository<Movie>&`. Thành viên `repo` là lớp lưu trữ **đã có** trong PBL2; interface không thay nó. Trong code thật, `Repository<Movie>` cần constructor với đường dẫn và callback; ví dụ rút gọn **không tạo được object để biên dịch độc lập** và không biểu diễn đầy đủ validation/parse/serialize.

**Đoạn 2 — chuyển lời gọi:**

```cpp
public:
    bool load() override
    {
        return repo.load();
    }

    Movie* getAt(int index) override
    {
        return repo.getAt(index);
    }

    int count() override
    {
        return repo.count();
    }
};
```

`load()` chuyển yêu cầu đọc tới `Repository<Movie>` và trả lại đúng `bool`; nếu tầng lưu trữ ném exception, ví dụ không bắt nên lỗi đi tiếp cho caller. `getAt(index)` trả cùng con trỏ Movie do repository sở hữu; `index` không hợp lệ dẫn đến `nullptr` theo lớp lưu trữ hiện tại. `count()` trả số Movie trong danh sách. `override` kiểm tra mỗi chữ ký khi biên dịch; phần thân thật của `MovieRepository` hiện còn kiểm tra dữ liệu, nên không được thay bằng bản rút gọn này.

### 3.3 Service nhận interface bằng tham chiếu

```cpp
#include "IRepository.h"
#include "../models/Movie.h"

class MovieService
{
private:
    IRepository<Movie>& movieRepo;

public:
    explicit MovieService(IRepository<Movie>& movieRepo)
        : movieRepo(movieRepo)
    {
    }

    bool load()
    {
        return movieRepo.load();
    }

    const Movie* getMovieAt(int index)
    {
        return movieRepo.getAt(index);
    }
};
```

**Đoạn 1 — dependency của Service:**

```cpp
#include "IRepository.h"
#include "../models/Movie.h"

class MovieService
{
private:
    IRepository<Movie>& movieRepo;
```

Service chỉ biết interface và kiểu Movie. `IRepository<Movie>&` là **tham chiếu** đến repository do nơi khác sở hữu, không phải con trỏ có thể rỗng. Header thật của `MovieService` hiện include `MovieRepository.h`, giữ `MovieRepository&` và còn `AuthService&`; đoạn minh họa bỏ những phần đó để tập trung vào cơ chế DIP, **không phải chữ ký thay thế đã được chốt**.

**Đoạn 2 — constructor và lời gọi:**

```cpp
public:
    explicit MovieService(IRepository<Movie>& movieRepo)
        : movieRepo(movieRepo)
    {
    }

    bool load()
    {
        return movieRepo.load();
    }

    const Movie* getMovieAt(int index)
    {
        return movieRepo.getAt(index);
    }
};
```

`explicit` ngăn chuyển đổi ngầm từ một repository thành Service khi hàm chỉ nhận một tham số. `: movieRepo(movieRepo)` là **initializer list**: thành viên tham chiếu phải được gắn với object ngay khi Service được tạo. `load()` gọi qua interface; C++ chạy `MovieRepository::load()` nhờ `virtual`. `getMovieAt()` trả `const Movie*`: Service không cấp quyền sửa Movie qua kết quả chỉ đọc, dù interface nội bộ trả `Movie*`. Nếu `getAt()` trả `nullptr`, kết quả vẫn là `nullptr`; Service không được dereference trước khi kiểm tra. Đây chỉ là minh họa; `MovieService::getMovieAt()` thật còn kiểm tra điều kiện truy cập theo hợp đồng hiện có.

### 3.4 Luồng gọi và giới hạn của ví dụ

```text
Tạo MovieRepository với đường dẫn và callback thật của dự án
|
v
Tạo MovieService, truyền MovieRepository dưới dạng IRepository<Movie>&
|
v
MovieService::load() gọi movieRepo.load()
|
v
virtual dispatch -> MovieRepository::load() -> Repository<Movie>::load()
|                                             |
v                                             v
true/false theo hợp đồng                         exception nếu đọc/parse lỗi
|
v
MovieService trả kết quả cho caller; MovieRepository vẫn sở hữu các Movie*
```

Ví dụ chứng minh hướng phụ thuộc, **không chứng minh một `IRepository<T>` đã đủ cho mọi Service**. Phạm vi hàm của interface chung, các thao tác không có chữ ký đồng nhất và cách giữ public API thật phải được nhóm xác nhận trước khi viết bản code chuẩn theo [AGENTS.md](../../AGENTS.md). Tài liệu này không triển khai interface hoặc thay đổi code.
