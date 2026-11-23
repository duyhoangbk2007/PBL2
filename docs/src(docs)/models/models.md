# `src/models/` — các đối tượng dữ liệu

**Phạm vi tài liệu:** các Model hiện có trong `src/models/`, gồm tám thực thể ban đầu và `Pricing` của Bước 09. `.h` khai báo dữ liệu và API, `.cpp` cài đặt. Model không tự mở TXT hoặc quyết định quyền và điều kiện bán vé. [Định dạng TXT](../../overview/data_format.md) vẫn là hợp đồng lưu trữ, còn các con trỏ quan hệ chỉ tồn tại trong RAM.

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
| `Pricing.h/.cpp` | Một cấu hình gồm ba giá ghế và hai tỷ lệ giảm; không có ID hoặc quan hệ con trỏ. |

## Pricing — cấu hình giá và ưu đãi

`Pricing` có năm thuộc tính private kiểu `double`: `standardPrice`, `vipPrice`, `couplePrice` (VND), `studentDiscount`, `memberDiscount` (phần trăm). Constructor mặc định đặt lần lượt **70000, 100000, 160000, 10, 5**; constructor thứ hai nhận đủ năm giá trị theo đúng thứ tự này. Model không có setter.

| Getter public | Giá trị trả về |
| --- | --- |
| `getStandardPrice() const` | Giá ghế STANDARD |
| `getVipPrice() const` | Giá ghế VIP |
| `getCouplePrice() const` | Giá ghế COUPLE |
| `getStudentDiscount() const` | Tỷ lệ giảm học sinh |
| `getMemberDiscount() const` | Tỷ lệ giảm thành viên |

`isValid() const` trả `false` nếu bất kỳ thuộc tính nào trong năm giá trị không hữu hạn (`std::isfinite()`), một giá ghế `<= 0` hoặc một tỷ lệ giảm nằm ngoài `[0, 100]`; các trường hợp còn lại trả `true`. Vì vậy kiểm tra này cũng áp dụng khi gọi `PricingRepository::update()` trực tiếp. Bộ `test_pricing.cpp` hiện chưa có ca riêng đưa `NaN`/vô cực vào Model.

## Quan hệ đã khai báo trong mã

| Quan hệ | API/thuộc tính hiện có | Giới hạn cần nhớ |
| --- | --- | --- |
| `Movie` ↔ `Showtime` | `Movie::linkShowtime`, danh sách suất; `Showtime::linkMovie`, con trỏ phim. | Mã phim phải khớp; hiện `ShowtimeRepository` nạp và nối lại từ TXT sau Movie/Room. |
| `Room` ↔ `Seat` | `Room::linkSeat`, danh sách ghế; `Seat::linkRoom`, con trỏ phòng. | Không nhầm ghế trong phòng với ghế đã bán theo suất. |
| `Room` ↔ `Showtime` | `Room::linkShowtime`, danh sách suất; `Showtime::linkRoom`, con trỏ phòng. | `ShowtimeRepository` hiện kiểm tra trùng lịch cùng phòng; các quy tắc nghiệp vụ khác chờ `ShowtimeService`. |
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

Khi đọc một Model: xem thuộc tính và mã tham chiếu trong `.h`, xem kiểm tra/nối quan hệ trong `.cpp`, rồi đối chiếu số trường TXT ở [data_format.md](../../overview/data_format.md). Các bước 01–03 đã có Model và tiện ích; Repository, Service và GUI là phần triển khai chức năng sau đó.
