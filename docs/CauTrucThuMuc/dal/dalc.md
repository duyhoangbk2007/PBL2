# DALC — Data Access Logic Components

DALC nằm trong `core/dal/dalc/`, thuộc tầng DAL của luồng **GUI → BLL → DAL → DB**. DALC đọc/ghi TXT, chuyển đổi dòng dữ liệu ↔ Model và giữ danh sách Model trong RAM. BLL duyệt danh sách để tìm kiếm, quyết định CRUD theo quy tắc nghiệp vụ rồi yêu cầu DALC thay đổi dữ liệu. `FileHandler` đọc/ghi dòng thô; thao tác nhiều file dùng `TransactionManager`.

## Hợp đồng chung cho DALC quản lý Model

Mỗi DALC quản lý một Model có `load()` và `save()`; `count()` và `getAt()` cho BLL đọc danh sách; `add()` và các thao tác thay đổi cần thiết cập nhật danh sách sau khi BLL đã kiểm tra nghiệp vụ. `getAt()` trả **con trỏ không sở hữu tới `const Model`**: BLL không `delete` hoặc gọi setter qua con trỏ này. `add(Model*)` chuyển quyền sở hữu cho DALC **chỉ khi trả `true`**; nếu thất bại, bên gọi vẫn sở hữu con trỏ. DALC chỉ được xóa object khi các quan hệ tham chiếu đã được xử lý an toàn.

Ví dụ sườn công khai đề xuất cho `MovieDALC` (không khẳng định header hiện tại đã theo sườn này):

```cpp
void load();
void save();
int count();
const Movie* getAt(int index);
bool add(Movie* movie);
bool update(const Movie& newValues);
bool removeById(const std::string& movieId);
```

BLL có thể tìm phim bằng cách duyệt `count()`/`getAt()`; vì vậy `findById()` không bắt buộc trong API công khai tối giản. `update()` ở DALC chỉ áp dụng dữ liệu đã được BLL chấp nhận, không quyết định quyền hay điều kiện sửa phim. Các hàm chuyển đổi dòng và dọn bộ nhớ vẫn là `private`.

## Sườn cho 9 DALC còn lại

`MovieDALC.h/.cpp` hiện là mẫu tham khảo. Mỗi tên dưới đây là **một cặp `.h/.cpp` dự kiến**, không phải code đã tạo. Các đoạn chỉ là sườn khai báo; khi viết header thật phải thêm `#include` và thống nhất cách lưu an toàn khi `save()` thất bại. Không chép máy móc `removeById()` sang vé hoặc hóa đơn vì phải giữ lịch sử.

### Hoàng: phòng, ghế, suất chiếu

```cpp
// RoomDALC.h: rooms.txt
class RoomDALC {
    std::string filePath;
    DoublyLinkedList<Room*> rooms;
    bool loaded = false;
    Room* StringToRoom(const std::string& line);
    std::string RoomToString(const Room* room);
    void clearRooms();
public:
    explicit RoomDALC(const std::string& filePath);
    ~RoomDALC();
    void load();
    void save();
    int count();
    const Room* getAt(int index);
    bool add(Room* room);
    bool update(const Room& newValues);
    bool removeById(const std::string& roomId);
};

// SeatDALC.h: seats.txt
class SeatDALC {
    std::string filePath;
    DoublyLinkedList<Seat*> seats;
    bool loaded = false;
    Seat* StringToSeat(const std::string& line);
    std::string SeatToString(const Seat* seat);
    void clearSeats();
public:
    explicit SeatDALC(const std::string& filePath);
    ~SeatDALC();
    void load();
    void save();
    int count();
    const Seat* getAt(int index);
    bool add(Seat* seat);
    bool update(const Seat& newValues);
    bool removeById(const std::string& roomId, const std::string& seatId);
};

// ShowtimeDALC.h: showtimes.txt
class ShowtimeDALC {
    std::string filePath;
    DoublyLinkedList<Showtime*> showtimes;
    bool loaded = false;
    Showtime* StringToShowtime(const std::string& line);
    std::string ShowtimeToString(const Showtime* showtime);
    void clearShowtimes();
public:
    explicit ShowtimeDALC(const std::string& filePath);
    ~ShowtimeDALC();
    void load();
    void save();
    int count();
    const Showtime* getAt(int index);
    bool add(Showtime* showtime);
    bool update(const Showtime& newValues);
    bool removeById(const std::string& showtimeId);
};
```

### Kiệt: tài khoản, thành viên, vé, hóa đơn

```cpp
// UserDALC.h: users.txt
class UserDALC {
    std::string filePath;
    DoublyLinkedList<User*> users;
    bool loaded = false;
    User* StringToUser(const std::string& line);
    std::string UserToString(const User* user);
    void clearUsers();
public:
    explicit UserDALC(const std::string& filePath);
    ~UserDALC();
    void load();
    void save();
    int count();
    const User* getAt(int index);
    bool add(User* user);
    bool update(const User& newValues);
    bool removeById(const std::string& userId);
};

// MemberDALC.h: members.txt
class MemberDALC {
    std::string filePath;
    DoublyLinkedList<Member*> members;
    bool loaded = false;
    Member* StringToMember(const std::string& line);
    std::string MemberToString(const Member* member);
    void clearMembers();
public:
    explicit MemberDALC(const std::string& filePath);
    ~MemberDALC();
    void load();
    void save();
    int count();
    const Member* getAt(int index);
    bool add(Member* member);
    bool update(const Member& newValues);
    bool removeById(const std::string& memberId);
};

// TicketDALC.h: tickets.txt
class TicketDALC {
    std::string filePath;
    DoublyLinkedList<Ticket*> tickets;
    bool loaded = false;
    Ticket* StringToTicket(const std::string& line);
    std::string TicketToString(const Ticket* ticket);
    void clearTickets();
public:
    explicit TicketDALC(const std::string& filePath);
    ~TicketDALC();
    void load();
    void save();
    int count();
    const Ticket* getAt(int index);
    bool add(Ticket* ticket);
    bool markCancelled(const std::string& ticketId,
                       const std::string& cancelledAt,
                       const std::string& cancelledBy,
                       const std::string& cancelReason);
};

// InvoiceDALC.h: invoices.txt
class InvoiceDALC {
    std::string filePath;
    DoublyLinkedList<Invoice*> invoices;
    bool loaded = false;
    Invoice* StringToInvoice(const std::string& line);
    std::string InvoiceToString(const Invoice* invoice);
    void clearInvoices();
public:
    explicit InvoiceDALC(const std::string& filePath);
    ~InvoiceDALC();
    void load();
    void save();
    int count();
    const Invoice* getAt(int index);
    bool add(Invoice* invoice);
};
```

### Kiệt: bộ đếm và ghi giao dịch

```cpp
// SequenceDALC.h: sequences.txt; file này không có Model riêng
class SequenceDALC {
    std::string filePath;
    std::vector<std::string> lines;
    bool loaded = false;
    void validateLine(const std::string& line);
public:
    explicit SequenceDALC(const std::string& filePath);
    void load();
    void save();
};

// BookingDALC.h: không có booking.txt riêng
class BookingDALC {
public:
    // Ghi các bản mới đã được chuẩn bị từ hóa đơn, vé,
    // thành viên và bộ đếm trong một giao dịch.
    void save(const std::vector<TransactionFile>& files);
};
```

### Sườn phần `.cpp` dùng cho các DALC theo từng Model

Ví dụ khung xử lý cho `RoomDALC.cpp`; thay `Room`/`rooms` và tên file tương ứng cho sáu lớp còn lại ở trên. Đây là các bước cần cài, chưa phải phần thân hàm hoàn chỉnh:

```cpp
void RoomDALC::load() {
    // FileHandler::readlines(filePath, lines); báo lỗi nếu đọc thất bại.
    // Duyệt lines từ phần tử 0, gọi StringToRoom cho từng dòng.
    // Nếu một dòng sai, dọn các Room tạm; không công bố dữ liệu nạp dở.
    // Chỉ đặt loaded = true khi toàn bộ file hợp lệ.
}

void RoomDALC::save() {
    // Duyệt rooms, gọi RoomToString để tạo toàn bộ dòng mới.
    // Gọi TransactionManager::execute với filePath và các dòng đó.
    // Không thêm dòng schema hoặc tiêu đề.
}
```

`SequenceDALC.cpp` đọc/kiểm các dòng `entity|nextValue` và chuẩn bị nội dung bộ đếm; `BookingDALC.cpp` ghép các `TransactionFile` đã chuẩn bị rồi gọi `TransactionManager::execute`. Hai lớp này không dùng `StringTo<Model>` vì không có Model tương ứng.

Trong các file `.cpp`, `load()` đọc **ngay từ dòng đầu tiên**; `save()` ghi các dòng dữ liệu và không thêm `#schema=1` hoặc tiêu đề. `StringTo...` kiểm số trường/kiểu trước khi tạo Model, `...ToString` xuất đúng thứ tự trong [data_format.md](../../TongQuan/data_format.md). Theo sườn này, DALC sở hữu object sau khi nạp hoặc `add()` thành công; BLL chỉ mượn con trỏ. Các con trỏ liên kết giữa Model không sở hữu object đích. Cần ngăn sao chép nông DALC/DLL và xử lý trường hợp `save()` thất bại để RAM không lệch TXT; chữ ký trên chưa quy định cách commit/rollback cụ thể.

**Lưu ý với mã mẫu:** `MovieDALC::load()` hiện bỏ qua dòng đầu vì giả định có schema. Theo định dạng mới, hành vi đó làm mất bản ghi phim đầu tiên; cần sửa code trong một yêu cầu riêng.
