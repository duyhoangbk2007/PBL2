# `src/models/` — các đối tượng dữ liệu

**Phạm vi tài liệu:** hiện trạng sau Bước 02 và Bước 03. Tám cặp `.h/.cpp` đã có trong `src/models/`; `.h` khai báo dữ liệu và API, `.cpp` cài đặt. Model biểu diễn **một thực thể**, không tự mở TXT, quản lý toàn bộ danh sách hay quyết định quyền, giá và điều kiện bán vé. [Định dạng TXT](../TongQuan/data_format.md) vẫn là hợp đồng lưu trữ, còn các con trỏ quan hệ chỉ tồn tại trong RAM.

## Từng cặp file

| File | Ý nghĩa |
| --- | --- |
| `Movie.h/.cpp` | Phim: mã, tên, thể loại, thời lượng, độ tuổi, ngày chiếu, trạng thái; có danh sách `Showtime*`. |
| `Room.h/.cpp` | Phòng: mã, tên, kích thước, trạng thái; có danh sách `Seat*` và `Showtime*`. |
| `Seat.h/.cpp` | Vị trí và loại ghế (`STANDARD`, `VIP`, `COUPLE`), mã phòng và con trỏ `Room*`. Trạng thái bán ghế phụ thuộc suất chiếu, không phải thuộc tính cố định của ghế. |
| `Showtime.h/.cpp` | Suất chiếu: mã phim/phòng, thời gian, trạng thái, con trỏ `Movie*` và `Room*`. |
| `User.h/.cpp` | Tài khoản Admin/Staff, thông tin đăng nhập, trạng thái và danh sách `Invoice*` liên quan. |
| `Member.h/.cpp` | Thành viên, thông tin liên hệ, điểm và danh sách `Invoice*` liên quan. |
| `Ticket.h/.cpp` | Vé: hóa đơn/suất/ghế, loại vé, giá, điểm, trạng thái hủy và các con trỏ tham chiếu. |
| `Invoice.h/.cpp` | Hóa đơn: một suất, người bán, thành viên tùy chọn, tổng tiền và danh sách nhiều `Ticket*`. |

## Quan hệ đã khai báo trong mã

| Quan hệ | API/thuộc tính hiện có | Giới hạn cần nhớ |
| --- | --- | --- |
| `Movie` ↔ `Showtime` | `Movie::linkShowtime`, danh sách suất; `Showtime::linkMovie`, con trỏ phim. | Mã phim phải khớp; việc nạp và nối lại từ TXT chưa phải chức năng Bước 02–03. |
| `Room` ↔ `Seat` | `Room::linkSeat`, danh sách ghế; `Seat::linkRoom`, con trỏ phòng. | Không nhầm ghế trong phòng với ghế đã bán theo suất. |
| `Room` ↔ `Showtime` | `Room::linkShowtime`, danh sách suất; `Showtime::linkRoom`, con trỏ phòng. | Kiểm tra lịch trùng thuộc nghiệp vụ sau này. |
| `Invoice` ↔ `Ticket` | `Invoice::linkTicket`, danh sách vé; `Ticket::linkInvoice`, con trỏ hóa đơn. | Một hóa đơn có thể có nhiều vé cùng suất; giữ lịch sử vé hủy. |
| `User`/`Member` ↔ `Invoice` | `Invoice::linkSeller`, `linkMember`; `User` và `Member` có danh sách hóa đơn. | Thành viên có thể không có; người bán phải được ghi đúng ID. |
| Tham chiếu khác | `Invoice::linkShowtime`; `Ticket::linkShowtime`, `linkSeat`. | Không suy ra chiều ngược đã có ở mọi lớp. |

Danh sách và con trỏ liên kết là **tham chiếu không sở hữu** đối tượng đích. Cần xác định rõ chủ sở hữu, vòng đời và cách gỡ liên kết khi xây dựng tầng truy xuất dữ liệu; không sao chép nông Model chứa `DoublyLinkedList<T>`.

## Sườn code để đọc một Model

Ví dụ rút gọn từ `Movie.h`; phần `...` biểu thị các trường/hàm đã lược bớt để thấy cấu trúc, **không phải đoạn mã cần chép vào dự án**:

```cpp
class Showtime;

class Movie {
private:
    std::string movieId;
    std::string title;
    DoublyLinkedList<Showtime*> showtimes;
    // ... các thuộc tính phim khác

public:
    const std::string& getMovieId() const;
    bool linkShowtime(Showtime* showtime);
    int getShowtimeCount() const;
    // ... constructor, getter và setter hiện có
};
```

Khi đọc một Model: xem thuộc tính và mã tham chiếu trong `.h`, xem kiểm tra/nối quan hệ trong `.cpp`, rồi đối chiếu số trường TXT ở [data_format.md](../TongQuan/data_format.md). Các bước 01–03 đã có Model và tiện ích; Repository, Service và GUI là phần triển khai chức năng sau đó.
