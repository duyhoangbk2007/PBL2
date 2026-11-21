# `src/data_access/` — Kiến trúc và quy chuẩn Repository

> **Dự án:** PBL2 — Hệ thống quản lý rạp chiếu phim  
> **Phạm vi:** `src/data_access/`, các quan hệ Model và kiểm thử Repository  
> **Mã nguồn tham chiếu:** `feature/b10-hoang-booking-management`  
> **Mục đích:** Thống nhất cách thiết kế, triển khai, kiểm thử và bảo trì Repository giữa các thành viên.
>
> **Lưu ý:** Tài liệu gồm kiến trúc hiện tại và các quy chuẩn đề xuất. Một số Repository chưa tuân thủ đầy đủ và cần được đồng bộ sau khi thống nhất thiết kế.

---

## 1. Tổng quan

Thư mục `src/data_access/` chứa các lớp Repository, chịu trách nhiệm quản lý dữ liệu Model trong RAM, đọc và ghi dữ liệu từ file TXT.

Dự án sử dụng file TXT thay vì hệ quản trị cơ sở dữ liệu SQL.

Repository là tầng trung gian giữa dữ liệu lưu trữ và tầng Business Logic.

### 1.1. Kiến trúc tổng thể

```text
             GUI (Qt Widgets)
                    |
                    v
              Business Service
                    |
                    v
           Repository cụ thể
                    |
          +---------+---------+
          |                   |
          v                   v
     Repository<T>       Model liên quan
          |                   |
          v                   v
   DoublyLinkedList      Quan hệ đối tượng
          |
          v
        FileIO
          |
          v
       File TXT
```

Chiều gọi thông thường:

```text
GUI -> Service -> Repository -> Model / FileIO
```

Repository không được phụ thuộc ngược vào GUI hoặc Service.

### 1.2. Mục tiêu thiết kế

1. Tái sử dụng những thao tác chung bằng `Repository<T>`.
2. Thống nhất cấu trúc `.h` và `.cpp`.
3. Phân biệt kiểm tra dữ liệu, tham chiếu và nghiệp vụ.
4. Bảo đảm dữ liệu không bị thay đổi một phần khi thao tác thất bại.
5. Quản lý bộ nhớ và các liên kết giữa Model an toàn.
6. Duy trì cách viết C++ đơn giản, dễ đọc và dễ giải thích.
7. Giúp các thành viên phát triển Repository theo cùng một quy chuẩn.

**Nguyên tắc quan trọng:** Đồng bộ cách tổ chức và luồng xử lý, không bắt buộc mọi Repository phải có cùng số lượng hàm.

---

## 2. Phân chia trách nhiệm

### 2.1. `Repository<T>`

Đây là lớp template quản lý dữ liệu chung.

Các hàm hiện có:

| Hàm | Trách nhiệm |
|---|---|
| `load()` | Đọc dữ liệu từ file |
| `save()` | Ghi dữ liệu ra file |
| `add()` | Thêm đối tượng |
| `removeById()` | Xóa đối tượng theo ID |
| `findById()` | Tìm đối tượng theo ID |
| `getAt()` | Lấy đối tượng theo vị trí |
| `count()` | Đếm đối tượng |
| `isLoaded()` | Kiểm tra trạng thái tải |
| `clear()` | Giải phóng dữ liệu đang quản lý |
| `getFilePath()` | Lấy đường dẫn file |

`Repository<T>` sử dụng `DoublyLinkedList<T*>` để lưu các đối tượng.

Lớp này không biết các quy tắc riêng của Movie, Room, Seat, Invoice hoặc Ticket.

### 2.2. Repository cụ thể

Ví dụ:

```cpp
class MovieRepository
{
private:
    Repository<Movie> repo;
};
```

Repository cụ thể chịu trách nhiệm:

- Kiểm tra dữ liệu của Model.
- Kiểm tra đối tượng được tham chiếu.
- Kiểm tra các ràng buộc giữa nhiều bản ghi.
- Thiết lập và tháo liên kết giữa Model.
- Thực hiện CRUD theo quy tắc của đối tượng.
- Bảo đảm trạng thái dữ liệu nhất quán khi thao tác thất bại.

Repository cụ thể **không tự tạo một cơ chế quản lý danh sách khác** nếu `Repository<T>` đã đáp ứng yêu cầu.

### 2.3. Model

Model quản lý thuộc tính và các quan hệ đối tượng.

Ví dụ:

```cpp
showtime->linkMovie(movie);
showtime->linkRoom(room);
```

Model không tự đọc file hoặc tìm dữ liệu trong Repository.

### 2.4. Service

Service chịu trách nhiệm:

- Kiểm tra quyền người thực hiện.
- Điều phối nghiệp vụ.
- Xử lý quy trình bán và hủy vé.
- Tính giá, giảm giá, điểm thưởng.
- Phối hợp nhiều Repository.
- Xử lý giao dịch liên quan nhiều file.

Ví dụ:

`TicketRepository` kiểm tra vé tham chiếu đến Invoice và Showtime hợp lệ.

Service kiểm tra người đang đăng nhập có quyền hủy vé hay không.

---

## 3. Các Repository trong dự án

| Repository | Model | Dependency |
|---|---|---|
| MovieRepository | Movie | Không |
| RoomRepository | Room | Không |
| UserRepository | User | Không |
| MemberRepository | Member | Không |
| SeatRepository | Seat | RoomRepository |
| ShowtimeRepository | Showtime | MovieRepository, RoomRepository |
| InvoiceRepository | Invoice | ShowtimeRepository, UserRepository, MemberRepository |
| TicketRepository | Ticket | ShowtimeRepository, SeatRepository, InvoiceRepository |

Ngoài ra, `PricingRepository` quản lý cấu hình giá vé.

**Ngoại lệ:** `PricingRepository` không sử dụng `Repository<T>` vì quản lý một cấu hình gồm các khóa cố định, thay vì danh sách đối tượng có ID.

Vì vậy, không ép `PricingRepository` phải có CRUD hoặc các callback như Repository thông thường.

---

## 4. Quy chuẩn cấu trúc Repository

### 4.1. Cấu trúc file `.h`

Thứ tự đề xuất:

1. Include guard.
2. Include các thư viện và kiểu cần thiết.
3. Forward declaration.
4. Khai báo class.
5. Thuộc tính `private`.
6. Callback dùng cho `Repository<T>`.
7. Các hàm kiểm tra.
8. Các helper liên kết và rollback.
9. Constructor, destructor.
10. `load()`, `save()`.
11. CRUD.
12. Các hàm truy vấn.

Ví dụ:

```cpp
class XRepository
{
private:
    Repository<X> repo;
    YRepository& yRepo;

    static string getXId(X* item);
    static X* stringToX(const string& line);
    static string xToString(X* item);

    static bool isValidX(const X* item);
    bool isValidReference(const X* item);

    void unlinkAll();

public:
    XRepository(const string& filePath,
                YRepository& yRepo);

    ~XRepository();

    bool load();
    bool save();

    bool add(X* item);
    bool update(const X& newValues);
    bool removeById(const string& id);

    X* findById(const string& id);
    X* getAt(int index);
    int count();
    bool isLoaded();
};
```

Đây là ví dụ cấu trúc, **không phải yêu cầu mọi Repository phải khai báo toàn bộ các hàm**.

Nếu Model không có dependency thì bỏ `isValidReference()`.

Nếu không cần tháo liên kết khi hủy thì bỏ `unlinkAll()` và destructor riêng.

Nếu không được phép sửa hoặc xóa thì không khai báo `update()` hoặc `removeById()`.

### 4.2. Quy chuẩn định dạng code

- Dùng tên hàm rõ nghĩa.
- Giữ cùng chữ ký giữa `.h` và `.cpp`.
- Căn các tham số nhiều dòng thẳng hàng.
- Sử dụng dấu `{}` nhất quán.
- Không viết logic phức tạp trong một dòng.
- Không thêm thư viện hoặc kỹ thuật C++ phức tạp khi không cần thiết.
- Ưu tiên các vòng lặp và cấu trúc điều kiện dễ hiểu.

---

## 5. Quy chuẩn kiểm tra dữ liệu

Đây là một trong những phần quan trọng nhất.

Cần phân biệt ba loại kiểm tra:

| Loại | Hàm | Mục đích |
|---|---|---|
| Dữ liệu nội tại | `isValidX()` | Kiểm tra bản thân đối tượng |
| Tham chiếu | `isValidReference()` | Kiểm tra quan hệ với đối tượng khác |
| Ràng buộc giữa bản ghi | Helper chuyên biệt | Kiểm tra trùng lịch, trùng ghế, trùng số điện thoại... |

### 5.1. `isValidX()`

Ví dụ:

```cpp
bool MovieRepository::isValidMovie(
    const Movie* movie
);
```

Hàm này chỉ kiểm tra dữ liệu của chính đối tượng.

Quy tắc:

- `nullptr` trả về `false`.
- Không truy cập Repository khác.
- Không thay đổi dữ liệu.
- Không thiết lập liên kết.
- Không phụ thuộc trạng thái `loaded`.

Ví dụ với Ticket:

```cpp
bool TicketRepository::isValidTicket(
    const Ticket* ticket
);
```

Kiểm tra:

- ID đúng định dạng.
- Loại vé hợp lệ.
- Trạng thái hợp lệ.
- Giá không âm và hữu hạn.
- Tiền giảm không vượt giá gốc.
- Tiền thanh toán phù hợp giá gốc và giảm giá.
- Dữ liệu hủy vé phù hợp trạng thái.

Hàm này không kiểm tra Showtime hoặc Invoice có tồn tại.

### 5.2. `isValidReference()`

Chỉ khai báo khi đối tượng có dependency.

Ví dụ:

```cpp
bool TicketRepository::isValidReference(
    const Ticket* ticket
);
```

Kiểm tra:

1. Dữ liệu Ticket hợp lệ.
2. Các Repository phụ thuộc đã load.
3. Showtime tồn tại.
4. Invoice tồn tại.
5. Seat tồn tại trong phòng tương ứng.
6. Invoice và Ticket thuộc cùng Showtime.
7. Các điều kiện tham chiếu bổ sung được quy định.

**Quy chuẩn thống nhất:**

`isValidReference()` được phép gọi `isValidX()` ở đầu hàm để tự bảo vệ khi được gọi độc lập.

Hàm không được thiết lập hoặc tháo liên kết.

Không kiểm tra quyền của người đang thực hiện thao tác trong hàm này.

### 5.3. Kiểm tra ràng buộc giữa nhiều bản ghi

Những quy tắc không chỉ phụ thuộc một đối tượng nên được đặt trong helper riêng nếu logic đủ phức tạp.

Ví dụ:

```cpp
bool hasScheduleConflict(
    const Showtime* showtime,
    const string& excludeId
);
```

Hoặc:

```cpp
bool isSeatSold(
    const string& showtimeId,
    const string& seatId
);
```

Không nên đưa toàn bộ logic kiểm tra trùng lịch hoặc trùng ghế vào `isValidReference()`.

Lý do: kiểm tra tham chiếu và kiểm tra xung đột là hai trách nhiệm khác nhau.

### 5.4. Phân biệt dữ liệu lịch sử và điều kiện tạo mới

Đây là quy tắc quan trọng đối với hệ thống rạp chiếu phim.

Ví dụ, một nhân viên từng bán vé nhưng sau đó bị vô hiệu hóa tài khoản.

Hóa đơn cũ vẫn phải có khả năng được tải lại.

Vì vậy, cần phân biệt:

- Tham chiếu lịch sử có tồn tại và hợp lệ hay không.
- Đối tượng có đủ điều kiện thực hiện nghiệp vụ mới hay không.

Không tự động áp dụng mọi trạng thái hiện tại vào dữ liệu lịch sử.

---

## 6. Quy chuẩn `load()`

`load()` là một trong những hàm cần được thống nhất chặt chẽ nhất.

### 6.1. Nguyên tắc

Một lần load thành công phải bảo đảm:

- Dữ liệu đọc đúng định dạng.
- Không trùng khóa.
- Các tham chiếu hợp lệ.
- Không vi phạm ràng buộc liên bản ghi.
- Các liên kết được thiết lập đầy đủ.
- Không tồn tại con trỏ treo.

Nếu thất bại, Repository phải trở về trạng thái xác định, không giữ dữ liệu liên kết dở.

### 6.2. Luồng xử lý chuẩn

```text
BẮT ĐẦU
   |
   v
Kiểm tra repo đã load?
   |
   v
Kiểm tra dependency đã load?
   |
   v
repo.load()
   |
   v
Kiểm tra toàn bộ tham chiếu
   |
   v
Kiểm tra xung đột giữa các bản ghi
   |
   v
Liên kết các đối tượng
   |
   +---- Lỗi ----> Rollback
   |                 |
   |                 v
   |             repo.clear()
   |
   v
return true
```

### 6.3. Giai đoạn 1 — Kiểm tra tiền điều kiện

```cpp
if (repo.isLoaded())
{
    return false;
}

if (!yRepo.isLoaded())
{
    return false;
}
```

Không load lại một Repository đang được sử dụng.

Không load Repository phụ thuộc khi dependency chưa sẵn sàng.

### 6.4. Giai đoạn 2 — Đọc dữ liệu

```cpp
if (!repo.load())
{
    return false;
}
```

`Repository<T>::load()` chịu trách nhiệm:

- Đọc các dòng qua `FileIO`.
- Parse dữ liệu.
- Kiểm tra ID trùng.
- Quản lý các đối tượng đã tạo.
- Dọn dữ liệu tạm khi parse thất bại.

Lưu ý: Trong code hiện tại, lỗi parse có thể ném `runtime_error`, không nhất thiết trả về `false`.

### 6.5. Giai đoạn 3 — Kiểm tra toàn bộ

Duyệt các đối tượng và kiểm tra:

```cpp
for (int i = 0; i < repo.count(); i++)
{
    X* item = repo.getAt(i);

    if (!isValidReference(item))
    {
        // Xu ly loi
    }
}
```

Nếu có quy tắc trùng lịch, trùng ghế hoặc xung đột khác thì kiểm tra ở giai đoạn này.

**Không thiết lập liên kết trong lúc đang kiểm tra toàn bộ dữ liệu**, nếu có thể tách riêng.

### 6.6. Giai đoạn 4 — Liên kết

Sau khi toàn bộ dữ liệu hợp lệ, tiến hành liên kết.

Ví dụ:

```cpp
if (!showtime->linkMovie(movie) ||
    !showtime->linkRoom(room))
{
    throw runtime_error(
        "Khong the lien ket Showtime"
    );
}
```

Cần kiểm tra kết quả từng thao tác.

Không được mặc định mọi `link...()` đều thành công.

### 6.7. Giai đoạn 5 — Rollback

Nếu liên kết thất bại:

1. Gỡ các liên kết đã tạo.
2. Gỡ liên kết ở phía Model phụ thuộc.
3. Xóa dữ liệu thuộc Repository đang load.
4. Khôi phục trạng thái chưa load.
5. Trả lỗi theo hợp đồng đã thống nhất.

Ví dụ cấu trúc:

```cpp
try
{
    // Kiem tra va lien ket
}
catch (...)
{
    unlinkAll();
    repo.clear();
    throw;
}
```

Đây là cấu trúc minh họa. `unlinkAll()` phải được cài đặt an toàn trước khi sử dụng.

Nếu hàm có nhánh `return false` sau khi `repo.load()` thành công, nhánh đó cũng phải rollback đầy đủ.

### 6.8. Quy tắc xử lý lỗi

- `false`: tiền điều kiện không đạt hoặc trường hợp thất bại được thiết kế trả `false`.
- `runtime_error`: dữ liệu file sai định dạng, ID trùng trong file hoặc lỗi bất thường.
- `catch (...)`: dùng để bảo đảm dọn dẹp trước khi ném lại ngoại lệ.

Không dùng `catch (...)` để âm thầm bỏ qua lỗi.

Không được gọi `repo.clear()` khi các dependency vẫn giữ con trỏ tới đối tượng sắp bị xóa.

---

## 7. Quy chuẩn `add()`

### 7.1. Luồng xử lý

```text
Kiểm tra loaded
       |
       v
Kiểm tra isValidX
       |
       v
Kiểm tra trùng ID
       |
       v
Kiểm tra đối tượng đã liên kết?
       |
       v
Kiểm tra isValidReference
       |
       v
Kiểm tra xung đột
       |
       v
Thiết lập liên kết
       |
       v
repo.add()
       |
       v
return true
```

### 7.2. Quy tắc

1. Không thêm khi Repository chưa load.
2. Không thêm `nullptr`.
3. Không thêm ID trùng.
4. Không thêm lại cùng con trỏ đã được quản lý.
5. Không thêm đối tượng đã liên kết trái với quy ước.
6. Không thêm đối tượng có tham chiếu sai.
7. Không thêm đối tượng vi phạm ràng buộc nghiệp vụ.
8. Nếu liên kết thất bại, phải rollback.
9. Chỉ chuyển quyền sở hữu đối tượng khi thêm thành công.

### 7.3. Quyền sở hữu bộ nhớ

Trước khi `repo.add(item)` thành công, đối tượng vẫn do bên gọi sở hữu.

Sau khi `repo.add(item)` thành công, `Repository<T>` chịu trách nhiệm giải phóng đối tượng.

Vì vậy:

**Không tự ý `delete` đối tượng đầu vào khi `add()` trả về `false` trước khi chuyển quyền sở hữu.**

Cũng không sử dụng `repo.removeById()` như một thao tác gỡ khỏi danh sách mà vẫn giữ con trỏ, vì hàm này sẽ `delete` đối tượng.

### 7.4. Rollback khi thêm thất bại

Nếu `linkMovie()` thành công nhưng `linkRoom()` thất bại:

- Gỡ liên kết Movie vừa tạo.
- Không thêm đối tượng vào Repository.
- Trả `false` hoặc ném ngoại lệ theo hợp đồng.

Nếu đã thêm vào Repository rồi mới thực hiện các liên kết khác, cần đặc biệt chú ý quyền sở hữu và thao tác rollback.

Ưu tiên thiết kế thứ tự thao tác để giảm số bước cần rollback.

---

## 8. Quy chuẩn `update()`

Không phải Repository nào cũng cần `update()`.

### 8.1. Luồng xử lý

1. Kiểm tra Repository đã load.
2. Tìm đối tượng theo ID.
3. Kiểm tra dữ liệu mới bằng `isValidX()`.
4. Kiểm tra các trường không được thay đổi.
5. Kiểm tra tham chiếu.
6. Kiểm tra xung đột.
7. Chuẩn bị rollback nếu cập nhật có thể thất bại.
8. Thực hiện cập nhật.
9. Trả `true`.

### 8.2. Không thay đổi ID

ID là khóa định danh của đối tượng.

Không cho phép thay đổi ID bằng `update()` thông thường.

### 8.3. Không phá vỡ quan hệ

Ví dụ, không cho phép đổi `roomId` của Showtime một cách tùy ý khi Showtime đã có liên kết hoặc dữ liệu lịch sử.

Nếu nghiệp vụ thực sự cho phép đổi quan hệ, phải thiết kế thao tác unlink/link và rollback đầy đủ.

### 8.4. Kiểm tra xung đột khi cập nhật

Ví dụ:

```cpp
hasScheduleConflict(
    &newValues,
    current->getShowtimeId()
);
```

Tham số `excludeId` giúp bỏ qua chính đối tượng đang cập nhật.

### 8.5. Quy tắc đối với Invoice và Ticket

Không bổ sung `update()` tổng quát chỉ để giống các Repository khác.

Invoice và Ticket là dữ liệu lịch sử.

Việc hủy vé phải đi qua nghiệp vụ có kiểm soát, giữ lại:

- Trạng thái vé.
- Thời gian hủy.
- Người hủy.
- Lý do hủy.
- Số tiền hoàn.

---

## 9. Quy chuẩn `removeById()`

### 9.1. Luồng xử lý

```text
Kiểm tra loaded
       |
       v
Tìm đối tượng
       |
       v
Kiểm tra ràng buộc xóa
       |
       v
Tháo các liên kết
       |
       v
repo.removeById()
       |
       v
return true
```

### 9.2. Kiểm tra trước khi xóa

Ví dụ:

- Room còn Seat thì không được xóa.
- Room còn Showtime thì không được xóa.
- Movie còn Showtime thì cần tuân thủ quy tắc lịch sử.
- Invoice đã bán vé không được xóa tùy ý.

### 9.3. Tháo liên kết trước khi xóa

`Repository<T>::removeById()` giải phóng bộ nhớ đối tượng.

Vì vậy, phải gỡ các con trỏ tham chiếu trước khi gọi hàm này.

Nếu gỡ một liên kết thành công nhưng liên kết tiếp theo thất bại, phải khôi phục liên kết trước đó.

### 9.4. Không xóa dây chuyền tùy ý

Không tự động xóa Ticket chỉ vì Invoice bị xóa, hoặc xóa Showtime chỉ vì Room bị xóa.

Mọi thao tác xóa phải tuân thủ quy tắc nghiệp vụ và bảo toàn lịch sử.

---

## 10. Quy chuẩn `save()`

Repository cụ thể thông thường chỉ cần:

```cpp
bool XRepository::save()
{
    return repo.save();
}
```

`Repository<T>::save()` thực hiện chuyển đối tượng thành chuỗi rồi ghi qua `FileIO`.

### 10.1. Quy tắc serialize

Hàm `xToString()` phải:

- Kiểm tra dữ liệu nội tại.
- Ghi đúng thứ tự trường.
- Ghi đúng enum và định dạng số.
- Không thay đổi đối tượng.
- Không tạo hoặc tháo liên kết.

### 10.2. Giao dịch nhiều file

Lưu một file thành công không có nghĩa toàn bộ giao dịch thành công.

Ví dụ bán vé có thể ảnh hưởng:

```text
invoices.txt
tickets.txt
members.txt
```

Nếu lưu Invoice thành công nhưng lưu Ticket thất bại, dữ liệu có thể không nhất quán.

Vì vậy, việc điều phối lưu nhiều file phải được xử lý ở tầng nghiệp vụ hoặc cơ chế giao dịch riêng.

Không coi các lời gọi `save()` liên tiếp là một giao dịch nguyên tử.

---

## 11. Destructor và `unlinkAll()`

### 11.1. Khi nào cần destructor riêng?

Không phải Repository nào cũng cần destructor.

`Repository<T>` đã có destructor giải phóng các đối tượng mà nó quản lý.

Repository cụ thể chỉ cần destructor riêng khi phải dọn dẹp các liên kết trước khi đối tượng bị xóa.

Ví dụ:

```cpp
XRepository::~XRepository()
{
    unlinkAll();
}
```

### 11.2. Khi nào cần `unlinkAll()`?

Nên sử dụng khi Repository có nhiều liên kết cần gỡ và thao tác dọn dẹp được dùng ở nhiều nơi, chẳng hạn:

- Rollback trong `load()`.
- Destructor.
- Các thao tác khôi phục trạng thái khác.

Không tạo `unlinkAll()` rỗng chỉ để mọi Repository giống nhau.

### 11.3. Trách nhiệm của `unlinkAll()`

- Gỡ các liên kết do Repository thiết lập.
- Gỡ backlink ở các Model phụ thuộc.
- Không xóa đối tượng thuộc Repository khác.
- Không gọi `save()`.
- An toàn khi liên kết mới được tạo một phần.
- Không để con trỏ treo.

### 11.4. Thứ tự destructor

Trong C++, các thành viên của một class được hủy theo thứ tự ngược với thứ tự khai báo.

Vì vậy, Repository phụ thuộc phải được hủy trước Repository mà nó tham chiếu.

Ví dụ:

```cpp
struct ApplicationData
{
    MovieRepository movies;
    RoomRepository rooms;
    SeatRepository seats;
    ShowtimeRepository shows;
    UserRepository users;
    MemberRepository members;
    InvoiceRepository invoices;
    TicketRepository tickets;
};
```

Khi `ApplicationData` bị hủy, `tickets` bị hủy trước `invoices`.

Điều này cần thiết vì Ticket có liên kết đến Invoice.

**Quy tắc:** Dependency phải tồn tại lâu hơn Repository sử dụng dependency đó.

### 11.5. Destructor không thay thế rollback

Destructor chỉ xử lý việc dọn dẹp khi đối tượng kết thúc vòng đời.

Nếu `load()` hoặc `add()` thất bại, rollback phải được thực hiện ngay lúc đó.

Không để trạng thái lỗi tồn tại cho đến khi chương trình kết thúc.

---

## 12. Quản lý con trỏ và liên kết Model

### 12.1. Con trỏ sở hữu

`Repository<T>` sở hữu các đối tượng đã thêm hoặc load thành công.

Khi `clear()` hoặc `removeById()` được gọi, đối tượng có thể bị `delete`.

### 12.2. Con trỏ không sở hữu

Các quan hệ giữa Model thường không chuyển quyền sở hữu.

Ví dụ:

```text
Room <----> Seat
Movie <----> Showtime
User <----> Invoice
Member <----> Invoice
Invoice <----> Ticket
```

Danh sách con trỏ ở một Model không có nghĩa Model đó được quyền xóa các đối tượng được tham chiếu.

### 12.3. Liên kết hai chiều

Khi thiết lập quan hệ hai chiều, phải bảo đảm cả hai phía thống nhất.

Ví dụ, Invoice giữ Ticket trong danh sách và Ticket giữ con trỏ Invoice.

Nếu Ticket bị xóa, Invoice không được tiếp tục giữ con trỏ đến Ticket đó.

### 12.4. Quy tắc rollback

Một thao tác gồm nhiều bước chỉ được coi là thành công khi tất cả các bước hoàn tất.

Nếu bước thứ ba thất bại sau khi hai bước trước thành công, cần khôi phục hai bước trước.

Không để đối tượng ở trạng thái liên kết một phần.

---

## 13. Quy chuẩn xử lý lỗi

### 13.1. Khi nào trả về `false`?

Thông thường:

- Repository chưa load.
- Đối tượng không tồn tại.
- ID trùng.
- Dữ liệu không hợp lệ.
- Tham chiếu không tồn tại.
- Không đáp ứng điều kiện thêm, sửa, xóa.
- Thao tác bị từ chối theo quy tắc.

### 13.2. Khi nào ném ngoại lệ?

Ví dụ:

- File có dữ liệu sai định dạng.
- Chuyển đổi chuỗi thất bại.
- Trùng ID trong dữ liệu được load.
- Lỗi bất thường khi liên kết.
- Không thể khôi phục trạng thái sau lỗi.

Cần giữ cách xử lý nhất quán với hợp đồng của `Repository<T>` hiện tại.

### 13.3. Không nuốt ngoại lệ

Không sử dụng:

```cpp
catch (...)
{
    return false;
}
```

nếu điều đó che giấu lỗi nghiêm trọng hoặc bỏ qua rollback.

### 13.4. Không làm mất dữ liệu khi lỗi

Sau khi thao tác thất bại, phải xác định rõ:

- Repository còn loaded hay không.
- Đối tượng đã được thêm hay chưa.
- Quyền sở hữu con trỏ thuộc về ai.
- Các liên kết đã được khôi phục chưa.
- Dữ liệu trên đĩa có bị thay đổi hay không.

---

## 14. Quy chuẩn riêng cho từng Repository

| Repository | `isValidX()` | `isValidReference()` | Destructor riêng |
|---|---|---|---|
| Movie | Có | Không cần | Chỉ khi cần cleanup quan hệ |
| Room | Có | Không cần | Chỉ khi cần cleanup quan hệ |
| User | Có | Không cần | Chỉ khi cần cleanup quan hệ |
| Member | Có | Không cần | Chỉ khi cần cleanup quan hệ |
| Seat | Có | Nên bổ sung để kiểm tra Room | Theo liên kết thực tế |
| Showtime | Có | Có | Cần gỡ liên kết Movie/Room |
| Invoice | Có | Có | Cần gỡ liên kết User/Member |
| Ticket | Có | Có | Cần xử lý liên kết Invoice–Ticket |

### 14.1. MovieRepository

- Kiểm tra dữ liệu Movie.
- Kiểm tra các ràng buộc xóa/sửa liên quan Showtime.
- Không cần `isValidReference()` nếu không tham chiếu Repository khác.

### 14.2. RoomRepository

- Kiểm tra dữ liệu Room.
- Không thu nhỏ phòng làm ghế nằm ngoài phạm vi.
- Không xóa phòng khi còn dữ liệu phụ thuộc.
- Không cần `isValidReference()` cho bản thân Room.

### 14.3. SeatRepository

- Kiểm tra dữ liệu Seat.
- Kiểm tra Room tồn tại.
- Kiểm tra vị trí ghế không trùng.
- Liên kết Seat với Room.
- Rollback khi liên kết thất bại.

**Đề xuất đồng bộ:** Tách kiểm tra Room sang `isValidReference()`.

### 14.4. ShowtimeRepository

- Kiểm tra dữ liệu Showtime.
- Kiểm tra Movie và Room.
- Kiểm tra trùng lịch.
- Liên kết Showtime với Movie/Room.
- Gỡ liên kết khi xóa hoặc hủy Repository.

### 14.5. UserRepository

- Kiểm tra dữ liệu User.
- Kiểm tra tính duy nhất của những trường được quy định.
- Không cần `isValidReference()` nếu không có dependency.

### 14.6. MemberRepository

- Kiểm tra dữ liệu Member.
- Kiểm tra số điện thoại hợp lệ và duy nhất.
- Quản lý các quan hệ hóa đơn theo thiết kế Model.
- Không cần `isValidReference()` nếu không có dependency.

### 14.7. InvoiceRepository

- Kiểm tra dữ liệu Invoice.
- Kiểm tra Showtime, User và Member.
- Liên kết Invoice với các đối tượng liên quan.
- Rollback khi load hoặc add thất bại.
- Gỡ liên kết User/Member trước khi Invoice bị xóa.

Không cho phép sửa/xóa lịch sử hóa đơn tùy ý.

### 14.8. TicketRepository

- Kiểm tra dữ liệu Ticket.
- Kiểm tra Invoice, Showtime và Seat.
- Không cho phép hai vé `SOLD` cùng ghế trong một suất chiếu.
- Giữ lịch sử vé `CANCELLED`.
- Liên kết Ticket với Invoice.
- Rollback khi liên kết hoặc thêm thất bại.
- Gỡ liên kết Invoice–Ticket trước khi Ticket bị xóa.

Cần kiểm tra thêm tham chiếu `cancelledBy` theo quy tắc nghiệp vụ đã thống nhất.

---

## 15. Thứ tự load toàn hệ thống

Các Repository độc lập có thể được load trước:

```text
MovieRepository
RoomRepository
UserRepository
MemberRepository
```

Sau đó:

```text
RoomRepository
       |
       v
SeatRepository

MovieRepository + RoomRepository
       |
       v
ShowtimeRepository

ShowtimeRepository + UserRepository + MemberRepository
       |
       v
InvoiceRepository

ShowtimeRepository + SeatRepository + InvoiceRepository
       |
       v
TicketRepository
```

Một thứ tự hợp lệ:

```text
1. Movie
2. Room
3. Seat
4. Showtime
5. User
6. Member
7. Invoice
8. Ticket
```

PricingRepository có thể được tải riêng theo dependency của tầng nghiệp vụ.

**Lưu ý:** Thứ tự load không nhất thiết giống thứ tự tên file, mà phải tuân thủ quan hệ phụ thuộc.

---

## 16. Quy chuẩn kiểm thử

Mỗi Repository cần có các nhóm kiểm thử phù hợp với chức năng của mình.

### 16.1. Load

- [ ] Load file hợp lệ.
- [ ] Load file rỗng.
- [ ] Không load hai lần.
- [ ] Không load trước dependency.
- [ ] Từ chối dữ liệu sai định dạng.
- [ ] Từ chối ID trùng.
- [ ] Từ chối tham chiếu không tồn tại.
- [ ] Từ chối xung đột giữa các bản ghi.
- [ ] Rollback khi liên kết thất bại.
- [ ] Không để dangling pointer sau lỗi.

### 16.2. Add

- [ ] Thêm đối tượng hợp lệ.
- [ ] Từ chối `nullptr`.
- [ ] Từ chối ID trùng.
- [ ] Từ chối tham chiếu sai.
- [ ] Từ chối đối tượng đã liên kết không hợp lệ.
- [ ] Kiểm tra rollback khi thêm thất bại.
- [ ] Kiểm tra ownership sau thất bại.

### 16.3. Update

- [ ] Cập nhật hợp lệ.
- [ ] Từ chối ID không tồn tại.
- [ ] Từ chối dữ liệu mới không hợp lệ.
- [ ] Không cho thay đổi ID.
- [ ] Không phá vỡ quan hệ.
- [ ] Kiểm tra xung đột với bản ghi khác.

### 16.4. Remove

- [ ] Xóa đối tượng hợp lệ.
- [ ] Từ chối ID không tồn tại.
- [ ] Từ chối xóa khi còn dependency.
- [ ] Gỡ toàn bộ liên kết trước khi xóa.
- [ ] Rollback khi unlink thất bại.
- [ ] Không double-delete hoặc dangling pointer.

### 16.5. Save và vòng đời

- [ ] Save thành công.
- [ ] Save/reload giữ nguyên dữ liệu.
- [ ] Destructor không để con trỏ treo.
- [ ] Thứ tự hủy dependency an toàn.
- [ ] Load thất bại không phá dữ liệu của Repository khác.

---

## 17. Những điểm cần đồng bộ trên nhánh B10

Các điểm dưới đây là kết quả đối chiếu mã nguồn, chưa phải thay đổi đã được áp dụng.

| Thành phần | Vấn đề | Hướng chuẩn hóa |
|---|---|---|
| SeatRepository | Kiểm tra Room trực tiếp | Tách `isValidReference()` |
| ShowtimeRepository | Cleanup nằm trực tiếp trong destructor | Cân nhắc helper `unlinkAll()` |
| InvoiceRepository | Đã có `unlinkAll()` | Giữ và kiểm tra tính an toàn |
| TicketRepository | Chưa có cleanup vòng đời tương ứng liên kết Invoice | Bổ sung cơ chế unlink |
| TicketRepository `load()` | Chưa rollback toàn diện khi có ngoại lệ | Chuẩn hóa theo luồng load |
| TicketRepository `add()` | Có nguy cơ giữ liên kết dở khi thất bại | Chuẩn hóa rollback |
| Invoice/Ticket | Chưa đối chiếu tổng tiền ở mức giao dịch | Kiểm tra sau khi cả hai Repo đã load |
| Các Repository | Thứ tự kiểm tra và cách gọi helper chưa hoàn toàn giống nhau | Thống nhất theo tài liệu |

Không nên chỉnh sửa hàng loạt mà chưa chạy lại test.

---

## 18. Quy trình triển khai Repository mới

Mỗi Repository mới nên thực hiện theo thứ tự:

1. **Thiết kế:** xác định Model, dependency, CRUD được phép, quy tắc dữ liệu và quan hệ.
2. **Viết tài liệu:** mô tả từng hàm, tiền điều kiện, hậu điều kiện, rollback và lỗi.
3. **Viết `.h`:** thống nhất tên hàm, chữ ký và thứ tự khai báo.
4. **Viết `.cpp`:** triển khai đúng luồng xử lý.
5. **Viết test:** kiểm tra trường hợp đúng, sai và lỗi giữa chừng.
6. **Chạy test:** biên dịch với các thành phần liên quan và kiểm tra regression.
7. **Cập nhật docs:** bảo đảm tài liệu phản ánh code thực tế.
8. **Review trước khi merge:** kiểm tra ownership, destructor, dependency và toàn vẹn dữ liệu.

Không đánh dấu test đã pass nếu chưa thực sự chạy.

---

## 19. Tài liệu liên quan

- `Repository.h`
- `MovieRepository.md`
- `RoomRepository.md`
- `SeatRepository.md`
- `ShowtimeRepository.md`
- `InvoiceRepository.md`
- `TicketRepository.md` (nếu đã được tạo)
- `models.md`
- `business_rules.md`
- `data_format.md`
- `AGENTS.md`

Các tài liệu riêng của Repository mô tả chi tiết nghiệp vụ từng lớp.

Tài liệu này là quy chuẩn chung để tránh lặp lại các nguyên tắc kiến trúc và quản lý bộ nhớ trong từng file.

---

## 20. Kết luận

Tầng Data Access của PBL2 sử dụng `Repository<T>` để tái sử dụng các thao tác quản lý dữ liệu chung, trong khi Repository cụ thể xử lý những ràng buộc riêng của Model.

Tất cả Repository phải tuân thủ các nguyên tắc:

1. **Kiểm tra trước khi thay đổi dữ liệu.**
2. **Tách kiểm tra nội tại, tham chiếu và xung đột.**
3. **Không thiết lập liên kết khi dữ liệu chưa được xác nhận hợp lệ.**
4. **Rollback đầy đủ khi thao tác thất bại.**
5. **Không xóa đối tượng khi còn liên kết tham chiếu chưa được xử lý.**
6. **Destructor phải an toàn và không ném ngoại lệ.**
7. **Không thêm CRUD hoặc helper không cần thiết.**
8. **Không đưa logic phân quyền của người thao tác vào Repository.**
9. **Mọi thay đổi phải có test và cập nhật tài liệu.**
10. **Giữ code đơn giản, dễ đọc và nhất quán giữa các thành viên.**

Mục tiêu cuối cùng không phải làm tất cả Repository giống hệt nhau, mà là bảo đảm chúng tuân thủ **cùng một quy trình kiểm tra, liên kết, quản lý bộ nhớ và xử lý lỗi**.