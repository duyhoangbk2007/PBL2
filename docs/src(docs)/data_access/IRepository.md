# IRepository

## 1. Tổng quan

`IRepository<T>` là lớp trừu tượng thuộc tầng Data Access, định nghĩa hợp đồng chung cho các Concrete Repository trong hệ thống PBL2.

Lớp được xây dựng nhằm áp dụng nguyên lý Dependency Inversion Principle (DIP), giúp các Business Service và các Repository phụ thuộc vào interface thay vì phụ thuộc trực tiếp vào lớp triển khai cụ thể.

`IRepository<T>` chỉ khai báo các hàm chung, không trực tiếp quản lý dữ liệu, đọc ghi file TXT hoặc xử lý nghiệp vụ.

**File liên quan:**

- `src/data_access/IRepository.h`
- `src/data_access/Repository.h`
- Các Concrete Repository trong `src/data_access/`.

**Trạng thái:** Thiết kế dự kiến – chờ CHỐT MARKDOWN.

## 2. Mục tiêu

- Áp dụng Dependency Inversion Principle (DIP).
- Chuẩn hóa các hàm chung của Repository.
- Giảm sự phụ thuộc trực tiếp giữa các lớp.
- Hỗ trợ Dependency Injection thông qua constructor.
- Hỗ trợ mở rộng và kiểm thử hệ thống.
- Tận dụng lớp `Repository<T>` hiện tại.
- Giữ nguyên cấu trúc dữ liệu và định dạng file TXT.

Hệ thống chỉ sử dụng một interface Repository chung là `IRepository<T>`.

## 3. Kiến trúc

### 3.1. Quan hệ giữa các lớp

Ví dụ đối với module Movie:

```text
MovieService
     |
     | Phu thuoc
     v
IRepository<Movie>
     ^
     | Ke thua
     |
MovieRepository
     |
     | Composition
     v
Repository<Movie>
     |
     v
DoublyLinkedList<Movie*> + TXT
```

`MovieService` phụ thuộc vào `IRepository<Movie>` thay vì phụ thuộc trực tiếp vào `MovieRepository`.

`MovieRepository` kế thừa và triển khai các hàm được khai báo trong `IRepository<Movie>`.

Bên trong `MovieRepository` vẫn sử dụng `Repository<Movie>` để quản lý dữ liệu.

### 3.2. Trách nhiệm của từng lớp

| Lớp | Trách nhiệm |
|---|---|
| `IRepository<T>` | Định nghĩa hợp đồng chung cho các Repository. |
| `Repository<T>` | Quản lý danh sách, bộ nhớ và đọc ghi dữ liệu. |
| Concrete Repository | Triển khai interface, kiểm tra tính hợp lệ và toàn vẹn dữ liệu. |
| Business Service | Xử lý nghiệp vụ và kiểm tra quyền. |
| GUI | Hiển thị dữ liệu và tương tác với người dùng. |

### 3.3. Nguyên tắc thiết kế

- Concrete Repository kế thừa `IRepository<T>`.
- Concrete Repository chứa đối tượng `Repository<T>`.
- `Repository<T>` không cần kế thừa `IRepository<T>`.
- Business Service sử dụng tham chiếu `IRepository<T>&` tại các vị trí phù hợp.
- Các dependency được truyền vào thông qua constructor.
- Interface không sở hữu hoặc quản lý vòng đời của các đối tượng Model.
- Các hàm đặc thù của từng Repository được giữ lại và rà soát khi refactor từng module.

## 4. Thiết kế lớp

### 4.1. Template

```cpp
template <typename T>
class IRepository
```

`T` đại diện cho kiểu Model mà Repository quản lý.

Ví dụ:

```cpp
IRepository<Movie>
IRepository<Room>
IRepository<Seat>
IRepository<Showtime>
IRepository<User>
IRepository<Member>
IRepository<Ticket>
IRepository<Invoice>
```

Việc sử dụng template giúp định nghĩa interface một lần và tái sử dụng cho nhiều kiểu Model khác nhau.

### 4.2. Thuộc tính

`IRepository<T>` không có thuộc tính.

Các thuộc tính quản lý dữ liệu như `filePath`, `items`, `loaded`, `getId`, `parse` và `serialize` vẫn thuộc về `Repository<T>`.

Interface chỉ quy định các thao tác mà Repository phải cung cấp, không quy định cách triển khai bên trong.

### 4.3. Danh sách các hàm

| STT | Hàm | Kiểu trả về | Chức năng |
|---|---|---|---|
| 1 | `~IRepository()` | Không có | Destructor ảo. |
| 2 | `load()` | `bool` | Nạp dữ liệu. |
| 3 | `save()` | `bool` | Lưu dữ liệu. |
| 4 | `add(T* item)` | `bool` | Thêm đối tượng. |
| 5 | `update(const T& newValues)` | `bool` | Cập nhật đối tượng. |
| 6 | `removeById(const string& key)` | `bool` | Xóa đối tượng theo khóa. |
| 7 | `findById(const string& key)` | `T*` | Tìm đối tượng theo khóa. |
| 8 | `getAt(int index)` | `T*` | Lấy đối tượng theo vị trí. |
| 9 | `count()` | `int` | Đếm số lượng đối tượng. |
| 10 | `isLoaded()` | `bool` | Kiểm tra trạng thái nạp dữ liệu. |

Tất cả 9 hàm chức năng đều là hàm thuần ảo (pure virtual function).

Các Concrete Repository bắt buộc phải định nghĩa lại đầy đủ các hàm này.

## 5. Code dự kiến

### 5.1. IRepository.h

**Đường dẫn:** `src/data_access/IRepository.h`

```cpp
#ifndef IREPOSITORY_H
#define IREPOSITORY_H

#include <string>

using namespace std;

template <typename T>
class IRepository
{
public:
    virtual ~IRepository() = default;

    // Doc va ghi du lieu
    virtual bool load() = 0;
    virtual bool save() = 0;

    // Quan ly du lieu
    virtual bool add(T* item) = 0;
    virtual bool update(const T& newValues) = 0;
    virtual bool removeById(const string& key) = 0;

    // Truy van du lieu
    virtual T* findById(const string& key) = 0;
    virtual T* getAt(int index) = 0;
    virtual int count() = 0;
    virtual bool isLoaded() = 0;
};

#endif
```

### 5.2. IRepository.cpp

Không cần xây dựng file `IRepository.cpp`.

`IRepository<T>` là lớp template và chỉ chứa các khai báo hàm thuần ảo cùng destructor được định nghĩa mặc định.

Toàn bộ nội dung của lớp được đặt trong file `IRepository.h`.

## 6. Giải thích code

### 6.1. Header Guard

```cpp
#ifndef IREPOSITORY_H
#define IREPOSITORY_H
```

Header Guard giúp tránh việc một file header bị định nghĩa nhiều lần trong cùng một đơn vị biên dịch.

```cpp
#endif
```

Kết thúc phạm vi Header Guard.

### 6.2. Thư viện string

```cpp
#include <string>

using namespace std;
```

Thư viện `<string>` được sử dụng để khai báo kiểu dữ liệu `string` trong các hàm tìm kiếm và xóa đối tượng theo khóa.

`using namespace std;` cho phép sử dụng `string` thay vì phải viết `std::string`.

### 6.3. Template

```cpp
template <typename T>
class IRepository
```

Khai báo một lớp template với tham số kiểu dữ liệu `T`.

Khi sử dụng `IRepository<Movie>`, các vị trí sử dụng `T` sẽ tương ứng với kiểu `Movie`.

Khi sử dụng `IRepository<User>`, các vị trí sử dụng `T` sẽ tương ứng với kiểu `User`.

Nhờ đó, không cần xây dựng một interface riêng cho từng Model.

### 6.4. Từ khóa virtual

Từ khóa `virtual` cho phép sử dụng tính đa hình trong C++.

Khi gọi một hàm thông qua tham chiếu hoặc con trỏ của lớp cha, chương trình sẽ thực hiện hàm tương ứng của lớp con.

Ví dụ:

```cpp
IRepository<Movie>& repository = movieRepo;

repository.load();
```

Mặc dù `repository` có kiểu `IRepository<Movie>&`, hàm được thực hiện là `MovieRepository::load()` nếu đối tượng được truyền vào là `MovieRepository`.

### 6.5. Hàm thuần ảo

```cpp
virtual bool load() = 0;
```

Cú pháp `= 0` dùng để khai báo hàm thuần ảo.

Hàm thuần ảo định nghĩa một hợp đồng mà lớp con phải triển khai để trở thành lớp cụ thể có thể khởi tạo đối tượng.

Do chứa các hàm thuần ảo, `IRepository<T>` là lớp trừu tượng và không thể khởi tạo trực tiếp.

Ví dụ không hợp lệ:

```cpp
IRepository<Movie> repository;
```

Ví dụ hợp lệ:

```cpp
MovieRepository movieRepo("data/movies.txt");

IRepository<Movie>& repository = movieRepo;
```

Ví dụ trên giả định `MovieRepository` đã được refactor để triển khai đầy đủ interface.

### 6.6. Từ khóa override

Khi triển khai các hàm của interface, Concrete Repository sử dụng từ khóa `override`.

Ví dụ:

```cpp
class MovieRepository : public IRepository<Movie>
{
public:
    bool load() override;
    bool save() override;
};
```

Từ khóa `override` giúp trình biên dịch kiểm tra xem hàm của lớp con có ghi đè đúng hàm ảo của lớp cha hay không.

Nếu viết sai tên hàm, kiểu tham số hoặc chữ ký không tương thích, trình biên dịch sẽ báo lỗi.

Đoạn code trên chỉ minh họa cú pháp `override`, chưa triển khai đầy đủ interface.

## 7. Giải thích chi tiết các hàm

### 7.1. Destructor

```cpp
virtual ~IRepository() = default;
```

**Chức năng:**

Đảm bảo đối tượng của lớp con được hủy đúng cách khi bị xóa thông qua con trỏ của lớp cha.

**Giải thích:**

- `virtual`: Cho phép gọi đúng destructor của lớp con.
- `~IRepository()`: Destructor của interface.
- `= default`: Yêu cầu trình biên dịch tạo destructor mặc định.

Interface không có thuộc tính và không trực tiếp sở hữu dữ liệu nên không cần viết thêm logic giải phóng bộ nhớ.

Việc giải phóng dữ liệu và hủy liên kết giữa các đối tượng vẫn do Concrete Repository và `Repository<T>` thực hiện.

### 7.2. Hàm load()

```cpp
virtual bool load() = 0;
```

**Chức năng:**

Nạp dữ liệu từ nguồn lưu trữ vào bộ nhớ.

**Tham số:** Không có.

**Giá trị trả về:**

- `true`: Nạp dữ liệu thành công.
- `false`: Không thể nạp dữ liệu hoặc Repository không cho phép nạp lại.

**Nguyên tắc triển khai:**

- Kiểm tra trạng thái nạp dữ liệu.
- Đọc và chuyển đổi dữ liệu thành các đối tượng Model.
- Kiểm tra tính hợp lệ của dữ liệu.
- Kiểm tra và thiết lập các liên kết cần thiết.
- Cập nhật trạng thái nạp dữ liệu khi thành công.
- Giải phóng dữ liệu tạm và hoàn tác liên kết khi xảy ra lỗi.

Concrete Repository có thể ném exception khi phát hiện dữ liệu không hợp lệ, theo cơ chế xử lý lỗi hiện tại của dự án.

### 7.3. Hàm save()

```cpp
virtual bool save() = 0;
```

**Chức năng:**

Lưu dữ liệu hiện tại trong bộ nhớ xuống nguồn lưu trữ.

**Tham số:** Không có.

**Giá trị trả về:**

- `true`: Lưu dữ liệu thành công.
- `false`: Lưu dữ liệu thất bại hoặc Repository chưa được nạp.

**Nguyên tắc triển khai:**

- Kiểm tra trạng thái Repository.
- Chuyển các đối tượng Model thành định dạng lưu trữ.
- Ghi dữ liệu xuống file.
- Giữ nguyên cơ chế xử lý lỗi của từng Repository.

Đối với TicketRepository và InvoiceRepository, cần giữ nguyên cơ chế quản lý `pendingIds` khi lưu dữ liệu.

### 7.4. Hàm add()

```cpp
virtual bool add(T* item) = 0;
```

**Chức năng:**

Thêm một đối tượng mới vào Repository.

**Tham số:**

- `item`: Con trỏ đến đối tượng cần thêm.

**Giá trị trả về:**

- `true`: Thêm đối tượng thành công.
- `false`: Thêm đối tượng thất bại.

**Nguyên tắc triển khai:**

- Repository phải được nạp dữ liệu.
- Đối tượng không được là `nullptr`.
- Dữ liệu của đối tượng phải hợp lệ.
- Khóa của đối tượng không được trùng.
- Các liên kết với đối tượng khác phải hợp lệ.
- Thiết lập hoặc hoàn tác các liên kết khi cần thiết.

**Quy tắc quản lý bộ nhớ:**

- Khi thêm thành công, Repository tiếp nhận quyền sở hữu đối tượng.
- Khi thêm thất bại, quyền sở hữu đối tượng không được chuyển sang Repository.
- Bên gọi chịu trách nhiệm giải phóng đối tượng mới nếu việc thêm thất bại và đối tượng chưa thuộc quyền sở hữu của Repository.
- Không được giải phóng một đối tượng đã thuộc quyền sở hữu của Repository.

Hàm `add()` chỉ thay đổi dữ liệu trong bộ nhớ. Việc lưu xuống file được thực hiện thông qua `save()`.

### 7.5. Hàm update()

```cpp
virtual bool update(const T& newValues) = 0;
```

**Chức năng:**

Cập nhật thông tin của một đối tượng đã tồn tại trong Repository.

**Tham số:**

- `newValues`: Đối tượng chứa thông tin mới cần cập nhật.

**Giá trị trả về:**

- `true`: Cập nhật thành công.
- `false`: Cập nhật thất bại hoặc Repository không hỗ trợ thao tác cập nhật.

**Giải thích tham số:**

- `T`: Kiểu Model tương ứng.
- `&`: Truyền tham chiếu để tránh sao chép đối tượng không cần thiết.
- `const`: Không cho phép thay đổi trực tiếp đối tượng `newValues` thông qua tham số.

**Nguyên tắc triển khai:**

- Kiểm tra trạng thái Repository.
- Xác định đối tượng cần cập nhật thông qua khóa.
- Kiểm tra tính hợp lệ của dữ liệu mới.
- Kiểm tra các ràng buộc liên quan.
- Chỉ cập nhật khi dữ liệu đáp ứng đầy đủ điều kiện.
- Không tự ý thay đổi khóa của đối tượng.
- Ưu tiên cập nhật trực tiếp đối tượng hiện tại để bảo toàn các liên kết và con trỏ đang sử dụng.

Mỗi Concrete Repository tự định nghĩa cách cập nhật phù hợp với đặc điểm của Model.

Đối với Repository chưa hỗ trợ cập nhật, hàm được định nghĩa trả về `false` và không làm thay đổi dữ liệu.

### 7.6. Hàm removeById()

```cpp
virtual bool removeById(const string& key) = 0;
```

**Chức năng:**

Xóa một đối tượng khỏi Repository dựa trên khóa.

**Tham số:**

- `key`: Khóa xác định đối tượng cần xóa.

**Giá trị trả về:**

- `true`: Xóa thành công.
- `false`: Xóa thất bại hoặc Repository không hỗ trợ thao tác xóa.

**Nguyên tắc triển khai:**

- Kiểm tra trạng thái Repository.
- Tìm đối tượng theo khóa.
- Kiểm tra các ràng buộc trước khi xóa.
- Không xóa đối tượng nếu việc xóa làm mất tính toàn vẹn dữ liệu.
- Hủy các liên kết cần thiết trước khi giải phóng đối tượng.
- Cập nhật các cấu trúc dữ liệu liên quan.

Đối với các Repository không cho phép xóa dữ liệu, hàm được định nghĩa trả về `false`.

Không sử dụng `removeById()` để thay thế `rollbackPending()` trong TicketRepository và InvoiceRepository.

### 7.7. Hàm findById()

```cpp
virtual T* findById(const string& key) = 0;
```

**Chức năng:**

Tìm kiếm một đối tượng trong Repository dựa trên khóa.

**Tham số:**

- `key`: Khóa của đối tượng cần tìm.

**Giá trị trả về:**

- Con trỏ `T*` nếu tìm thấy đối tượng.
- `nullptr` nếu không tìm thấy đối tượng.

Sử dụng tên tham số `key` thay cho `id` để hỗ trợ cả khóa đơn và khóa ghép.

Ví dụ khóa đơn:

```cpp
Movie* movie = movieRepo.findById("MOV0001");
```

Ví dụ khóa ghép của Seat:

```cpp
Seat* seat = seatRepo.findById("ROM0001|A01");
```

Ví dụ Seat sử dụng chữ ký dự kiến sau refactor.

**Quy tắc quản lý bộ nhớ:**

Con trỏ trả về là con trỏ mượn đến đối tượng đang được Repository quản lý.

Bên gọi không được tự ý `delete` đối tượng.

Cần kiểm tra `nullptr` trước khi truy cập đối tượng.

### 7.8. Hàm getAt()

```cpp
virtual T* getAt(int index) = 0;
```

**Chức năng:**

Lấy đối tượng tại một vị trí trong danh sách.

**Tham số:**

- `index`: Vị trí của đối tượng, bắt đầu từ 0.

**Giá trị trả về:**

- Con trỏ `T*` nếu vị trí hợp lệ.
- `nullptr` nếu vị trí không hợp lệ.

Hàm thường được sử dụng kết hợp với `count()` để duyệt toàn bộ dữ liệu.

Ví dụ:

```cpp
for (int i = 0; i < movieRepo.count(); i++)
{
    Movie* movie = movieRepo.getAt(i);

    if (movie != nullptr)
    {
        // Xu ly doi tuong Movie
    }
}
```

Con trỏ trả về không chuyển quyền sở hữu cho bên gọi.

### 7.9. Hàm count()

```cpp
virtual int count() = 0;
```

**Chức năng:**

Trả về số lượng đối tượng hiện có trong Repository.

**Tham số:** Không có.

**Giá trị trả về:**

Số lượng đối tượng đang được Repository quản lý.

Nếu Repository không chứa đối tượng nào, hàm trả về `0`.

Hàm được sử dụng để:

- Duyệt danh sách đối tượng.
- Kiểm tra số lượng dữ liệu.
- Hỗ trợ tìm kiếm và thống kê.
- Hỗ trợ tạo ID trong các module liên quan.

### 7.10. Hàm isLoaded()

```cpp
virtual bool isLoaded() = 0;
```

**Chức năng:**

Kiểm tra trạng thái nạp dữ liệu của Repository.

**Tham số:** Không có.

**Giá trị trả về:**

- `true`: Repository đã được nạp dữ liệu thành công.
- `false`: Repository chưa được nạp dữ liệu thành công.

Hàm giúp các Repository và Service kiểm tra trạng thái dữ liệu trước khi thực hiện các thao tác liên quan.

Trước mắt giữ nguyên chữ ký không có `const` để tương thích với các Repository dạng danh sách hiện tại.

## 8. Quy định triển khai Concrete Repository

### 8.1. Kế thừa interface

Các Concrete Repository kế thừa công khai từ `IRepository<T>`.

Ví dụ:

```cpp
class MovieRepository : public IRepository<Movie>
{
private:
    Repository<Movie> repo;

public:
    MovieRepository(const string& filePath);

    bool load() override;
    bool save() override;

    bool add(Movie* movie) override;
    bool update(const Movie& newValues) override;
    bool removeById(const string& movieId) override;

    Movie* findById(const string& movieId) override;
    Movie* getAt(int index) override;
    int count() override;
    bool isLoaded() override;
};
```

Đoạn code trên chỉ minh họa cách triển khai interface, không thay thế toàn bộ header hiện tại của MovieRepository.

### 8.2. Phạm vi áp dụng

| Repository | Interface tương ứng |
|---|---|
| MovieRepository | `IRepository<Movie>` |
| RoomRepository | `IRepository<Room>` |
| SeatRepository | `IRepository<Seat>` |
| ShowtimeRepository | `IRepository<Showtime>` |
| UserRepository | `IRepository<User>` |
| MemberRepository | `IRepository<Member>` |
| TicketRepository | `IRepository<Ticket>` |
| InvoiceRepository | `IRepository<Invoice>` |

PricingRepository và RolePermissionRepository có cách tổ chức dữ liệu và API khác với các Repository dạng danh sách.

Trước mắt, hai lớp này giữ nguyên thiết kế hiện tại và không bắt buộc kế thừa `IRepository<T>`.

### 8.3. Quy định đối với update() và removeById()

Tất cả Concrete Repository kế thừa interface đều phải định nghĩa lại hai hàm `update()` và `removeById()`.

Tuy nhiên, cách triển khai phụ thuộc vào quy tắc của từng module.

| Repository | update() | removeById() |
|---|---|---|
| MovieRepository | Hỗ trợ | Hỗ trợ |
| RoomRepository | Hỗ trợ | Hỗ trợ |
| SeatRepository | Hỗ trợ | Hỗ trợ |
| ShowtimeRepository | Hỗ trợ | Hỗ trợ |
| UserRepository | Hỗ trợ | Không hỗ trợ |
| MemberRepository | Hỗ trợ | Không hỗ trợ |
| TicketRepository | Chưa hỗ trợ | Không hỗ trợ |
| InvoiceRepository | Chưa hỗ trợ | Không hỗ trợ |

Ví dụ đối với TicketRepository:

```cpp
bool TicketRepository::update(const Ticket& newValues)
{
    return false;
}

bool TicketRepository::removeById(const string& key)
{
    return false;
}
```

Các hàm không hỗ trợ phải đảm bảo không làm thay đổi dữ liệu, liên kết hoặc trạng thái Repository.

Việc trả về `false` được định nghĩa tại Concrete Repository, không định nghĩa mặc định trong interface.

Khi cần mở rộng chức năng, nhóm có thể thay đổi cách triển khai của từng Concrete Repository mà không cần thay đổi chữ ký interface.

Đây là sự đánh đổi để duy trì một interface chung. Service phải sử dụng đúng các thao tác mà Repository tương ứng hỗ trợ.

### 8.4. Quy định đối với SeatRepository

SeatRepository sử dụng khóa ghép gồm `roomId` và `seatId`.

Khóa ghép có dạng:

```text
roomId|seatId
```

Ví dụ:

```text
ROM0001|A01
```

SeatRepository dự kiến bổ sung các hàm nhận một tham số `key` để tương thích với interface.

```cpp
Seat* findById(const string& key) override;
bool removeById(const string& key) override;
```

Đồng thời giữ lại các hàm hiện tại:

```cpp
Seat* findById(const string& roomId,
               const string& seatId);

bool removeById(const string& roomId,
                const string& seatId);
```

Đây là cơ chế đa năng hóa hàm (function overloading).

Các phiên bản nhận khóa ghép phải giữ nguyên các kiểm tra và thao tác hủy liên kết hiện tại, không được bỏ qua logic của Concrete Repository.

Cách triển khai chi tiết sẽ được thiết kế trong tài liệu SeatRepository.

### 8.5. Quy định đối với TicketRepository và InvoiceRepository

TicketRepository và InvoiceRepository có cơ chế quản lý giao dịch đang thực hiện thông qua `pendingIds` và `rollbackPending()`.

Các cơ chế này phải được giữ nguyên trong quá trình refactor.

Không được:

- Sử dụng `removeById()` để thay thế `rollbackPending()`.
- Xóa trực tiếp dữ liệu giao dịch đã lưu.
- Bỏ qua việc hủy liên kết giữa các đối tượng.
- Làm mất tính toàn vẹn của lịch sử giao dịch.

### 8.6. Các hàm đặc thù

Những hàm đặc thù chưa được đưa vào interface trong giai đoạn này, bao gồm:

- `findByUsername()`.
- `findByPhone()`.
- `getByShowtime()`.
- `getByInvoice()`.
- `getByStaff()`.
- `getByMember()`.
- `isSeatSold()`.
- `rollbackPending()`.

Các hàm này tiếp tục được giữ trong Concrete Repository.

Khi refactor từng Service, cần rà soát cách sử dụng các hàm đặc thù trước khi chuyển dependency sang `IRepository<T>`.

Không sử dụng ép kiểu về Concrete Repository chỉ để gọi các hàm không có trong interface.

Thiết kế hiện tại chưa đảm bảo mọi Service có thể loại bỏ hoàn toàn dependency vào Concrete Repository.

## 9. Dependency Injection

Dependency Injection là kỹ thuật truyền các đối tượng phụ thuộc từ bên ngoài vào một lớp thay vì để lớp đó tự khởi tạo dependency.

Trong PBL2, ưu tiên sử dụng Constructor Injection thông qua tham chiếu.

Ví dụ:

```cpp
class MovieService
{
private:
    IRepository<Movie>& movieRepo;
    AuthService& authService;

public:
    MovieService(IRepository<Movie>& movieRepo,
                 AuthService& authService);
};
```

Constructor:

```cpp
MovieService::MovieService(IRepository<Movie>& movieRepo,
                           AuthService& authService)
    : movieRepo(movieRepo),
      authService(authService)
{
}
```

Khởi tạo đối tượng:

```cpp
MovieRepository movieRepo("data/movies.txt");

MovieService movieService(movieRepo, authService);
```

`MovieService` không cần biết cách MovieRepository đọc ghi dữ liệu hoặc sử dụng cấu trúc dữ liệu bên trong.

Service chỉ cần biết các hàm được khai báo trong `IRepository<Movie>`.

Các đoạn code trên minh họa cách áp dụng Dependency Injection sau refactor.

**Quy định:**

- Dependency được truyền thông qua constructor.
- Service không tự tạo Concrete Repository.
- Service không sở hữu Repository được truyền vào.
- Repository phải tồn tại lâu hơn Service sử dụng nó.
- Phải giữ đúng thứ tự khởi tạo, nạp dữ liệu và hủy đối tượng.
- Các Repository phụ thuộc vào Repository khác cũng sử dụng interface tại những vị trí phù hợp.

## 10. Quy tắc quản lý bộ nhớ

- `IRepository<T>` không trực tiếp quản lý bộ nhớ.
- Concrete Repository và `Repository<T>` chịu trách nhiệm quản lý vòng đời của các đối tượng.
- Các con trỏ trả về từ `findById()` và `getAt()` là con trỏ mượn.
- Service không được tự ý giải phóng đối tượng thuộc Repository.
- Không tiếp tục sử dụng con trỏ sau khi đối tượng tương ứng đã bị xóa hoặc Repository đã giải phóng dữ liệu.
- Phải bảo đảm các liên kết giữa các Model được hủy đúng cách trước khi giải phóng đối tượng.
- Không bỏ qua các cơ chế rollback và xử lý exception hiện tại.

## 11. Kế hoạch kiểm thử

Do `IRepository<T>` là lớp trừu tượng, không thể khởi tạo trực tiếp đối tượng để kiểm thử.

Việc kiểm thử được thực hiện thông qua các lớp triển khai interface.

### 11.1. Kiểm thử interface

- [ ] Kiểm tra file `IRepository.h` có thể biên dịch độc lập.
- [ ] Kiểm tra `IRepository<T>` là lớp trừu tượng.
- [ ] Kiểm tra Concrete Repository có thể kế thừa interface.
- [ ] Kiểm tra Concrete Repository phải triển khai đầy đủ các hàm thuần ảo.
- [ ] Kiểm tra các hàm sử dụng `override` đúng chữ ký.
- [ ] Kiểm tra cơ chế đa hình thông qua tham chiếu interface.
- [ ] Kiểm tra destructor ảo hoạt động đúng.

### 11.2. Kiểm thử chức năng

Sau khi refactor từng Concrete Repository:

- [ ] Kiểm thử `load()`.
- [ ] Kiểm thử `save()`.
- [ ] Kiểm thử `add()`.
- [ ] Kiểm thử `update()`.
- [ ] Kiểm thử `removeById()`.
- [ ] Kiểm thử `findById()`.
- [ ] Kiểm thử `getAt()`.
- [ ] Kiểm thử `count()`.
- [ ] Kiểm thử `isLoaded()`.
- [ ] Kiểm thử các thao tác không được hỗ trợ.
- [ ] Kiểm thử tính toàn vẹn dữ liệu.
- [ ] Kiểm thử các liên kết giữa các đối tượng.
- [ ] Kiểm thử quản lý bộ nhớ và rollback.

Ưu tiên sử dụng lại các bài kiểm thử hiện có của từng Repository và bổ sung các trường hợp liên quan đến interface.

## 12. Phạm vi triển khai hiện tại

Trong bước xây dựng IRepository:

- [ ] Review và chốt tài liệu `IRepository.md`.
- [ ] Tạo file `src/data_access/IRepository.h`.
- [ ] Triển khai đúng code đã được chốt trong Markdown.
- [ ] Biên dịch và kiểm thử interface.
- [ ] Đối chiếu mã nguồn với tài liệu.
- [ ] Cập nhật tiến độ sau khi kiểm thử thành công.

Chưa thực hiện trong bước này:

- Refactor `Repository<T>`.
- Refactor các Concrete Repository.
- Refactor các Business Service.
- Thay đổi các Model.
- Thay đổi cấu trúc dữ liệu.
- Thay đổi định dạng file TXT.
- Thay đổi Business – UI Contract.
- Thay đổi cơ chế phân quyền.

Các công việc trên sẽ được thực hiện trong những bước tiếp theo.

## 13. Kết luận

`IRepository<T>` là interface chung được sử dụng làm nền tảng cho quá trình refactor tầng Data Access và Business Logic trong PBL2.

Interface gồm:

- Không có thuộc tính.
- Một destructor ảo.
- Chín hàm thuần ảo.
- Không chứa logic lưu trữ dữ liệu.
- Không chứa logic nghiệp vụ.
- Không chứa phần triển khai mặc định của các hàm chức năng.

Các Concrete Repository chịu trách nhiệm định nghĩa đầy đủ các hàm của interface theo đặc điểm và quy tắc của từng module.

Lớp `Repository<T>` hiện tại tiếp tục được sử dụng thông qua composition.

Thiết kế này giúp áp dụng DIP tại các vị trí phù hợp, chuẩn hóa kiến trúc và tạo nền tảng cho quá trình hoàn thiện Backend mà không cần thay đổi toàn bộ cơ chế lưu trữ hiện tại.