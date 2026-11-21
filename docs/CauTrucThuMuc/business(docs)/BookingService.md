# BookingService

## 1. Tổng quan

`BookingService` là lớp thuộc tầng **Business Logic**, chịu trách nhiệm xử lý nghiệp vụ bán vé tại quầy trong hệ thống quản lý rạp chiếu phim PBL2.

Lớp phối hợp nhiều Repository và Service để:

- Lấy danh sách ghế còn trống của một suất chiếu.
- Kiểm tra danh sách ghế khách hàng lựa chọn.
- Tính giá vé, ưu đãi và điểm tích lũy.
- Xác nhận các điều kiện trước khi bán vé.
- Tạo hóa đơn và các vé tương ứng.
- Cập nhật điểm tích lũy của thành viên.
- Lưu dữ liệu giao dịch xuống các file TXT.
- Hoàn tác những thay đổi trong RAM nếu quá trình tạo giao dịch thất bại.

`BookingService` không trực tiếp đọc hoặc ghi file TXT. Việc lưu trữ được thực hiện thông qua các Repository.

### 1.1. File liên quan

**Business Logic:**

- `src/business/BookingService.h`
- `src/business/BookingService.cpp`
- `src/business/PricingService.h`
- `src/business/AuthService.h`

**Data Access:**

- `src/data_access/ShowtimeRepository.h`
- `src/data_access/SeatRepository.h`
- `src/data_access/TicketRepository.h`
- `src/data_access/InvoiceRepository.h`
- `src/data_access/MemberRepository.h`

**Models:**

- `src/models/Showtime.h`
- `src/models/Room.h`
- `src/models/Seat.h`
- `src/models/Ticket.h`
- `src/models/Invoice.h`
- `src/models/Member.h`
- `src/models/User.h`

**Utils:**

- `src/utils/DateTime.h`
- `src/utils/IdGenerator.h`

**Dữ liệu:**

- `data/showtimes.txt`
- `data/seats.txt`
- `data/tickets.txt`
- `data/invoices.txt`
- `data/members.txt`
- `data/pricing.txt`

### 1.2. Phạm vi chức năng

`BookingService` thuộc Bước 10 – Bán vé tại quầy.

Các nghiệp vụ không thuộc phạm vi của lớp trong bước này:

- Quản lý Stack Undo.
- Quản lý giỏ ghế đang chọn.
- Hiển thị sơ đồ ghế.
- Thanh toán điện tử.
- Hủy hoặc hoàn tiền vé đã bán.
- Thống kê doanh thu.
- Quản lý lịch sử hóa đơn.

**Quyết định thiết kế:** GUI do Kiệt thực hiện chịu trách nhiệm chọn ghế, bỏ chọn ghế và Undo. Backend chỉ nhận danh sách ghế cuối cùng để kiểm tra và xử lý.

Nghiệp vụ hủy vé và hoàn tiền thuộc Bước 11.

---

## 2. Kiến trúc và trách nhiệm

### 2.1. Luồng xử lý

```text
BookingView / SeatMapView
          |
          v
    BookingService
          |
          +---- AuthService
          |
          +---- PricingService
          |
          +---- ShowtimeRepository
          |
          +---- SeatRepository
          |
          +---- TicketRepository
          |
          +---- InvoiceRepository
          |
          +---- MemberRepository
          |
          v
      Du lieu TXT
```

### 2.2. Phân chia trách nhiệm

**GUI:**

- Hiển thị danh sách suất chiếu.
- Hiển thị sơ đồ ghế.
- Quản lý danh sách ghế đang chọn.
- Quản lý Stack Undo.
- Cho phép chọn loại vé từng ghế.
- Nhập hoặc chọn thành viên.
- Hiển thị giá vé dự kiến.
- Xác nhận độ tuổi và điều kiện vé.
- Xác nhận đã thu tiền.
- Hiển thị kết quả bán vé.

**BookingService:**

- Kiểm tra quyền bán vé.
- Kiểm tra trạng thái và thời gian suất chiếu.
- Kiểm tra phòng và ghế.
- Kiểm tra ghế đã bán.
- Kiểm tra loại vé.
- Kiểm tra thành viên.
- Tính giá và điểm tích lũy.
- Tạo Invoice và Ticket.
- Cập nhật điểm thành viên.
- Điều phối lưu giao dịch.
- Xử lý lỗi và rollback trong RAM.

**Repository:**

- Quản lý dữ liệu trong RAM.
- Kiểm tra tính hợp lệ của đối tượng và tham chiếu.
- Thiết lập, tháo các liên kết Model.
- Thêm và tra cứu đối tượng.
- Đọc và ghi file TXT.

---

## 3. Các thuộc tính của BookingService

```cpp
class BookingService
{
private:
    ShowtimeRepository& showtimeRepo;
    SeatRepository& seatRepo;
    TicketRepository& ticketRepo;
    InvoiceRepository& invoiceRepo;
    MemberRepository& memberRepo;

    PricingService& pricingService;
    AuthService& authService;
};
```

### 3.1. ShowtimeRepository

Sử dụng để:

- Tìm suất chiếu theo ID.
- Kiểm tra suất chiếu tồn tại.
- Kiểm tra trạng thái `SCHEDULED`.
- Lấy thời gian bắt đầu.
- Lấy phòng chiếu liên kết.

### 3.2. SeatRepository

Sử dụng để:

- Tìm ghế theo `roomId` và `seatId`.
- Kiểm tra ghế thuộc phòng chiếu.
- Lấy loại ghế để tính giá.

### 3.3. TicketRepository

Sử dụng để:

- Kiểm tra ghế đã bán trong một suất chiếu.
- Lấy danh sách ID vé hiện có.
- Thêm Ticket.
- Lưu danh sách vé.
- Hỗ trợ hoàn tác các Ticket vừa tạo khi giao dịch thất bại.

### 3.4. InvoiceRepository

Sử dụng để:

- Lấy danh sách ID hóa đơn.
- Thêm Invoice.
- Liên kết hóa đơn với các đối tượng liên quan.
- Lưu danh sách hóa đơn.
- Hỗ trợ hoàn tác Invoice vừa tạo khi giao dịch thất bại.

### 3.5. MemberRepository

Sử dụng để:

- Tìm thành viên theo ID.
- Lấy số điểm hiện tại.
- Cập nhật điểm sau khi bán.
- Lưu điểm thành viên.

### 3.6. PricingService

Sử dụng để:

- Lấy giá gốc theo loại ghế.
- Tính mức giảm giá.
- Tính số tiền thực trả.
- Tính điểm tích lũy của từng vé.

### 3.7. AuthService

Sử dụng để:

- Kiểm tra tài khoản đăng nhập.
- Kiểm tra quyền `TICKET_SELL`.
- Lấy thông tin nhân viên thực hiện giao dịch.

Các thuộc tính sử dụng **tham chiếu (`&`)** vì `BookingService` không sở hữu các Repository và Service.

Các đối tượng phụ thuộc phải tồn tại trong suốt thời gian sử dụng `BookingService`.

---

## 4. Phân quyền

### 4.1. Quyền bán vé

Quyền sử dụng:

```cpp
authService.requirePermission("TICKET_SELL");
```

Theo bảng quyền mặc định:

| Vai trò | Bán vé |
| ------- | ------ |
| ADMIN   | Có     |
| STAFF   | Có     |

Quyền thực tế phụ thuộc dữ liệu trong `role_permissions.txt`.

Không tự động cấp quyền cho Admin nếu bảng quyền không chứa quyền tương ứng.

### 4.2. Quy tắc

- Chưa đăng nhập thì không được bán vé.
- Tài khoản không có quyền `TICKET_SELL` thì không được bán.
- Tài khoản bị vô hiệu hóa không được bán.
- `staffId` trong Invoice lấy từ tài khoản đang đăng nhập.
- GUI không được tự truyền `staffId` để thay đổi người bán.

### 4.3. Quyền khi tính giá

`PricingService` hiện có các kiểm tra đăng nhập và phân quyền riêng.

`BookingService` tiếp tục sử dụng các phương thức PricingService hiện có, không bỏ qua cơ chế kiểm tra quyền của chúng.

---

## 5. Các cấu trúc dữ liệu hỗ trợ

### 5.1. BookingItem

```cpp
struct BookingItem
{
    string seatId;
    TicketType ticketType;
};
```

Đại diện cho một ghế được chọn cùng loại vé.

**seatId**

Mã ghế khách hàng lựa chọn.

**ticketType**

Loại vé:

```cpp
TicketType::REGULAR
TicketType::STUDENT
```

Loại vé khác hai giá trị trên không hợp lệ.

Một ghế đôi vẫn chỉ tương ứng một `BookingItem` và một Ticket.

### 5.2. BookingSummary

Giữ cấu trúc hiện tại:

```cpp
struct BookingSummary
{
    vector<double> ticketPrices;
    double totalAmount = 0;
    int earnedPoints = 0;
};
```

**ticketPrices**

Danh sách giá thực trả của từng vé, theo đúng thứ tự `items`.

**totalAmount**

Tổng số tiền phải thanh toán.

**earnedPoints**

Tổng điểm tích lũy của toàn bộ hóa đơn.

Giá trị này được tính bằng tổng điểm của từng vé, không tính trực tiếp từ tổng hóa đơn.

### 5.3. Quy tắc dữ liệu

- `items` không được rỗng.
- Không được chọn trùng `seatId`.
- Mỗi ghế phải thuộc phòng của suất chiếu.
- Mỗi ghế chỉ có một loại vé trong một giao dịch.
- `memberId` rỗng nghĩa là khách không có thành viên.
- Nếu có `memberId`, thành viên phải tồn tại.
- `ticketPrices.size()` bằng `items.size()` khi tính thành công.
- Khi tính thất bại, `summary` được đặt lại trạng thái mặc định.

---

## 6. Constructor

### 6.1. Khai báo

```cpp
BookingService(
    ShowtimeRepository& showtimeRepo,
    SeatRepository& seatRepo,
    TicketRepository& ticketRepo,
    InvoiceRepository& invoiceRepo,
    MemberRepository& memberRepo,
    PricingService& pricingService,
    AuthService& authService
);
```

### 6.2. Chức năng

Nhận các Repository và Service đã được khởi tạo từ bên ngoài.

Constructor không:

- Tự tạo Repository.
- Tự load dữ liệu.
- Tự ghi file.
- Tự tạo Ticket hoặc Invoice.

### 6.3. Điều kiện sử dụng

Các Repository phải được load theo đúng thứ tự phụ thuộc trước khi thực hiện nghiệp vụ.

Đặc biệt, dữ liệu liên quan đến Invoice và Ticket phải được nạp sau khi các đối tượng được tham chiếu đã sẵn sàng.

Thứ tự cụ thể thực hiện theo quy định trong `data_access.md` và phần khởi tạo ứng dụng.

---

## 7. Danh sách hàm public

| Hàm                   | Kiểu trả về     | Chức năng                 |
| --------------------- | --------------- | ------------------------- |
| `getAvailableSeats()` | `vector<Seat*>` | Lấy ghế còn trống         |
| `calculateBooking()`  | `bool`          | Kiểm tra và tính giá      |
| `createBooking()`     | `bool`          | Xác nhận và tạo giao dịch |

Không bổ sung hàm Undo trong Backend.

---

## 8. getAvailableSeats()

### 8.1. Khai báo

```cpp
vector<Seat*> getAvailableSeats(
    const string& showtimeId
);
```

### 8.2. Chức năng

Trả về danh sách ghế chưa bán của một suất chiếu hợp lệ.

Hàm không thay đổi dữ liệu.

### 8.3. Quy trình

**Bước 1: Kiểm tra Repository**

Yêu cầu:

```cpp
showtimeRepo.isLoaded()
seatRepo.isLoaded()
ticketRepo.isLoaded()
```

Nếu có Repository chưa load, trả về danh sách rỗng.

**Bước 2: Tìm suất chiếu**

```cpp
Showtime* showtime =
    showtimeRepo.findById(showtimeId);
```

Nếu không tồn tại, trả về danh sách rỗng.

**Bước 3: Kiểm tra trạng thái**

Suất chiếu phải có trạng thái:

```cpp
ShowtimeStatus::SCHEDULED
```

**Bước 4: Kiểm tra thời gian**

Thời gian hiện tại phải nhỏ hơn thời gian bắt đầu suất.

Sử dụng:

```cpp
DateTime::isBefore(...)
```

**Bước 5: Kiểm tra phòng**

Phòng phải tồn tại và đang hoạt động.

**Bước 6: Duyệt ghế**

Duyệt các ghế của phòng.

Với mỗi ghế, kiểm tra:

```cpp
ticketRepo.isSeatSold(
    showtimeId,
    seat->getSeatId()
);
```

Nếu chưa bán, thêm ghế vào kết quả.

### 8.4. Kết quả

Trả về danh sách `Seat*` còn trống.

Danh sách rỗng có thể có nghĩa:

- Suất không hợp lệ.
- Suất đã bắt đầu.
- Phòng không hoạt động.
- Không còn ghế trống.
- Repository chưa load.

Hàm hiện không phân biệt các nguyên nhân trên bằng mã lỗi.

### 8.5. Lưu ý

- Con trỏ trả về thuộc quyền quản lý của Repository.
- GUI không được `delete` các con trỏ này.
- GUI không được tự thay đổi dữ liệu Model.
- Danh sách ghế trống không có nghĩa ghế đã được giữ chỗ.
- Khi bán vé, `createBooking()` phải kiểm tra lại trạng thái ghế.

---

## 9. calculateBooking()

### 9.1. Khai báo

```cpp
bool calculateBooking(
    const string& showtimeId,
    const vector<BookingItem>& items,
    const string& memberId,
    BookingSummary& summary
);
```

### 9.2. Chức năng

Kiểm tra danh sách vé dự kiến và tính:

- Giá thực trả của từng vé.
- Tổng số tiền.
- Tổng điểm tích lũy.

Hàm không tạo Invoice hoặc Ticket.

Hàm không cộng điểm thành viên.

Hàm không ghi file.

### 9.3. Quy trình

**Bước 1: Khởi tạo kết quả**

```cpp
summary = BookingSummary{};
```

**Bước 2: Kiểm tra Repository**

Kiểm tra các Repository cần thiết đã load.

**Bước 3: Kiểm tra suất chiếu**

- Suất tồn tại.
- Trạng thái SCHEDULED.
- Chưa đến giờ bắt đầu.
- Phòng tồn tại và đang hoạt động.

**Bước 4: Kiểm tra danh sách vé**

```cpp
if (items.empty())
{
    return false;
}
```

**Bước 5: Kiểm tra thành viên**

```cpp
bool isMember = !memberId.empty();
```

Nếu có `memberId`, phải tìm thấy thành viên tương ứng.

**Bước 6: Duyệt từng BookingItem**

Với mỗi phần tử:

1. Tìm ghế theo phòng.
2. Kiểm tra ghế tồn tại.
3. Kiểm tra ghế chưa bán.
4. Kiểm tra không chọn trùng ghế.
5. Kiểm tra loại vé hợp lệ.
6. Lấy giá gốc.
7. Tính tiền giảm.
8. Tính tiền thực trả.
9. Tính điểm tích lũy của vé.
10. Cộng kết quả vào `summary`.

### 9.4. Công thức tính giá

**Giá gốc:**

```cpp
double basePrice =
    pricingService.getBasePrice(
        seat->getSeatType()
    );
```

**Tiền giảm:**

```cpp
double discount =
    pricingService.calculateDiscount(
        basePrice,
        item.ticketType,
        isMember
    );
```

**Tiền thực trả:**

```cpp
double paidAmount =
    pricingService.calculatePaidAmount(
        basePrice,
        discount
    );
```

**Điểm tích lũy:**

```cpp
int points =
    pricingService.calculatePoints(
        paidAmount,
        isMember
    );
```

### 9.5. Quy tắc tích điểm

Điểm được tính riêng cho từng vé:

```text
Diem ve = floor(Tien thuc tra / 10000)
```

Chỉ áp dụng khi có thành viên.

Tổng điểm:

```text
Tong diem = Tong(diem cua tung ve)
```

Không sử dụng:

```cpp
pricingService.calculatePoints(
    summary.totalAmount,
    isMember
);
```

để tính tổng điểm hóa đơn.

### 9.6. Ví dụ

Khách hàng có thành viên mua hai vé.

| Vé   | Tiền thực trả | Điểm |
| ---- | ------------: | ---: |
| Vé 1 |        75.000 |    7 |
| Vé 2 |        75.000 |    7 |
| Tổng |       150.000 |   14 |

Nếu tính điểm trực tiếp trên tổng hóa đơn sẽ nhận 15 điểm, không đúng quy tắc nghiệp vụ.

### 9.7. Kết quả

Trả về `true` khi toàn bộ dữ liệu hợp lệ và tính toán thành công.

Trả về `false` khi điều kiện nghiệp vụ không hợp lệ.

Các lỗi phân quyền, dữ liệu hệ thống hoặc lỗi tính toán nghiêm trọng có thể được xử lý bằng ngoại lệ theo quy ước của các Service liên quan.

### 9.8. Lưu ý

Cần kiểm tra kết quả tính toán hợp lệ và giới hạn số nguyên trước khi cộng điểm.

Nếu phát hiện lỗi sau khi đã thêm một số giá vé vào `summary`, phải bảo đảm kết quả thất bại không bị GUI sử dụng như một báo giá hợp lệ.

Có thể thực hiện tính toán vào biến tạm và chỉ gán cho `summary` sau khi toàn bộ bước kiểm tra thành công.

---

## 10. createBooking()

### 10.1. Khai báo dự kiến

```cpp
bool createBooking(
    const string& showtimeId,
    const vector<BookingItem>& items,
    const string& memberId,
    bool ageConfirmed,
    bool paymentConfirmed,
    string& invoiceId
);
```

So với phiên bản hiện tại, hàm bổ sung:

```cpp
bool ageConfirmed
bool paymentConfirmed
```

Không bổ sung `string& errorMessage`.

### 10.2. Chức năng

Tạo một giao dịch bán vé tại quầy.

Một giao dịch gồm:

- Một Invoice.
- Một hoặc nhiều Ticket.
- Một nhân viên bán.
- Tối đa một thành viên.
- Các vé thuộc cùng một suất chiếu.

### 10.3. Ý nghĩa tham số

**showtimeId**

Mã suất chiếu cần bán.

**items**

Danh sách ghế và loại vé đã chọn.

**memberId**

Mã thành viên, có thể rỗng.

**ageConfirmed**

Nhân viên đã xác nhận khách đáp ứng yêu cầu độ tuổi xem phim và điều kiện loại vé.

Nếu có vé STUDENT, nhân viên phải xác nhận điều kiện học sinh.

Với ghế đôi loại STUDENT, cả hai người phải đủ điều kiện.

**paymentConfirmed**

Nhân viên xác nhận đã thu đủ tiền tại quầy.

Đây là xác nhận nghiệp vụ, không phải tích hợp cổng thanh toán.

**invoiceId**

Tham số đầu ra chứa ID hóa đơn khi giao dịch hoàn tất thành công.

### 10.4. Quy tắc trả kết quả

- Trả `true` khi giao dịch hoàn tất và tất cả file liên quan đã lưu thành công.
- Trả `false` đối với các điều kiện nghiệp vụ không hợp lệ hoặc lỗi tạo giao dịch đã được hoàn tác an toàn.
- Ném ngoại lệ đối với lỗi nghiêm trọng, đặc biệt khi ghi nhiều file thất bại và không thể bảo đảm trạng thái dữ liệu nhất quán.

Không được coi lỗi lưu dở là một thất bại thông thường có thể lập tức thử lại.

---

## 11. Quy trình createBooking()

Quy trình được chia thành ba giai đoạn.

### 11.1. Giai đoạn 1 — Kiểm tra trước giao dịch

**Bước 1: Khởi tạo đầu ra**

```cpp
invoiceId = "";
```

**Bước 2: Kiểm tra quyền**

```cpp
const User& staff =
    authService.requirePermission(
        "TICKET_SELL"
    );
```

**Bước 3: Kiểm tra xác nhận**

```cpp
if (!ageConfirmed || !paymentConfirmed)
{
    return false;
}
```

**Bước 4: Kiểm tra Repository**

Các Repository liên quan phải được load đầy đủ.

**Bước 5: Kiểm tra và tính giá**

Gọi:

```cpp
BookingSummary summary;

if (!calculateBooking(
        showtimeId,
        items,
        memberId,
        summary))
{
    return false;
}
```

**Bước 6: Lấy thông tin giao dịch**

- Lấy `staffId` từ tài khoản đăng nhập.
- Lấy thời gian hiện tại bằng DateTime.
- Lấy Showtime.
- Lấy Member nếu có.

**Bước 7: Sinh ID**

Sinh:

- Một ID Invoice.
- Một ID cho từng Ticket.

Sử dụng:

```cpp
IdGenerator::generateNextId(...)
```

ID được sinh từ danh sách mã hiện có trong Repository.

Phải sinh đủ ID trước khi thêm đối tượng để tránh lỗi cấp mã giữa giao dịch.

**Bước 8: Kiểm tra giới hạn điểm**

Nếu có thành viên, kiểm tra việc cộng điểm không vượt quá giới hạn kiểu `int`.

Tại thời điểm này chưa thay đổi dữ liệu trong RAM.

### 11.2. Giai đoạn 2 — Tạo giao dịch trong RAM

**Bước 9: Tạo Invoice**

Invoice chứa:

- invoiceId
- showtimeId
- staffId
- memberId
- createdAt
- totalAmount

Thêm Invoice vào InvoiceRepository.

**Bước 10: Tạo các Ticket**

Với từng BookingItem, tạo Ticket chứa:

- ticketId
- invoiceId
- showtimeId
- seatId
- ticketType
- basePrice
- discountAmount
- paidAmount
- pointsEarned
- status = SOLD
- cancelledAt = ""
- cancelledBy = ""
- cancelReason = ""
- refundAmount = 0

Các giá trị giá và điểm phải được tính tại thời điểm bán.

Không tính lại lịch sử khi bảng giá thay đổi.

**Bước 11: Thêm Ticket**

Thêm lần lượt từng Ticket vào TicketRepository.

Repository chịu trách nhiệm kiểm tra dữ liệu và thiết lập liên kết với các Model liên quan.

**Bước 12: Cộng điểm thành viên**

Nếu có thành viên:

```cpp
member->setPoints(
    member->getPoints() +
    summary.earnedPoints
);
```

Phải giữ số điểm cũ để có thể khôi phục nếu cần.

### 11.3. Giai đoạn 3 — Lưu giao dịch

**Bước 13: Lưu Invoice**

```cpp
invoiceRepo.save();
```

**Bước 14: Lưu Ticket**

```cpp
ticketRepo.save();
```

**Bước 15: Lưu Member**

Nếu có thành viên:

```cpp
memberRepo.save();
```

Phải kiểm tra kết quả từng lần lưu.

Không dùng một biểu thức `||` duy nhất nếu cần xác định chính xác bước lưu bị lỗi.

**Bước 16: Hoàn tất**

Chỉ sau khi tất cả lần lưu thành công:

```cpp
invoiceId = newInvoiceId;
return true;
```

---

## 12. Rollback trong BookingService

### 12.1. Khái niệm

Rollback là quá trình hoàn tác những thay đổi đã thực hiện khi giao dịch không thể hoàn thành.

Trong Bước 10, phân biệt:

1. Rollback dữ liệu trong RAM.
2. Xử lý lỗi khi lưu nhiều file TXT.

Hai trường hợp này không tương đương nhau.

### 12.2. Rollback trong RAM

Giả sử một giao dịch tạo:

```text
INV0001
    |
    +-- TKT0001
    +-- TKT0002
    +-- TKT0003
```

Nếu thêm `TKT0003` thất bại, không được giữ lại Invoice và hai Ticket đã thêm.

Cần khôi phục trạng thái trước giao dịch.

### 12.3. Thông tin cần theo dõi

Trong quá trình tạo giao dịch, cần biết:

- Invoice nào vừa được thêm.
- Những Ticket nào đã thêm thành công.
- Thành viên nào đã được cập nhật điểm.
- Số điểm thành viên trước giao dịch.

Có thể dùng `vector<string>` để theo dõi ID các Ticket đã thêm thành công.

Không cần xây dựng một hệ thống giao dịch tổng quát chỉ để xử lý một nghiệp vụ bán vé.

### 12.4. Thứ tự rollback

Khi thất bại trước giai đoạn lưu file:

1. Khôi phục điểm thành viên nếu đã thay đổi.
2. Tháo và xóa các Ticket mới tạo, theo thứ tự ngược.
3. Tháo và xóa Invoice vừa tạo.
4. Kiểm tra các liên kết đã được khôi phục.
5. Trả về thất bại.

Phải tháo Ticket trước Invoice vì Ticket có liên kết tới Invoice.

### 12.5. Trách nhiệm của Repository

Rollback không được thực hiện bằng cách gọi `delete` trực tiếp lên đối tượng đã thuộc quyền sở hữu Repository.

Các Repository cần cung cấp cơ chế hoàn tác phù hợp.

Cơ chế này phải:

- Chỉ áp dụng cho đối tượng mới tạo trong giao dịch chưa được lưu.
- Tháo liên kết hai chiều.
- Xóa đối tượng khỏi danh sách lưu trữ.
- Giải phóng bộ nhớ đúng một lần.
- Không xóa đối tượng thuộc Repository khác.
- Không ghi file trong quá trình rollback RAM.

**Lưu ý triển khai:** Phiên bản TicketRepository và InvoiceRepository hiện tại chưa cung cấp đầy đủ API hoàn tác này.

Cần thiết kế và kiểm thử API hỗ trợ trước khi hoàn thiện rollback trong `createBooking()`.

Không sử dụng hàm xóa nghiệp vụ thông thường để xóa vé hoặc hóa đơn lịch sử.

### 12.6. Lỗi khi lưu file TXT

Ví dụ:

```text
invoiceRepo.save()  -> true
ticketRepo.save()   -> false
memberRepo.save()   -> chua thuc hien
```

Lúc này:

- `invoices.txt` có thể đã thay đổi.
- `tickets.txt` có thể chưa được cập nhật.
- `members.txt` chưa được cập nhật.

Rollback trong RAM không thể khôi phục file đã lưu thành công.

### 12.7. Phương án xử lý được chọn

Trong phạm vi PBL2, chưa triển khai cơ chế giao dịch nguyên tử cho nhiều file TXT.

Khi có lỗi ghi:

- Dừng quá trình lưu.
- Không trả về thành công.
- Báo lỗi nghiêm trọng bằng ngoại lệ.
- Nêu rõ file hoặc bước ghi thất bại nếu xác định được.
- Không tự động thực hiện lại giao dịch.
- Không tiếp tục bán vé trên bộ dữ liệu có nguy cơ không nhất quán.
- Yêu cầu kiểm tra và khôi phục dữ liệu trước khi tiếp tục.

Không được tự động rollback RAM rồi coi giao dịch như chưa từng xảy ra, vì file TXT có thể đã lưu một phần.

### 12.8. Giới hạn thiết kế

Cơ chế này bảo đảm có phương án hoàn tác khi tạo giao dịch trong RAM thất bại.

Nó **không bảo đảm tính nguyên tử của giao dịch trên nhiều file TXT**.

Nếu muốn khôi phục tự động sau mọi lỗi ghi file, cần bổ sung cơ chế giao dịch lưu trữ riêng trong tương lai.

---

## 13. Quy tắc quản lý bộ nhớ

### 13.1. Quyền sở hữu đối tượng

`BookingService` tạo Invoice và Ticket bằng `new`.

Trước khi Repository nhận đối tượng thành công, BookingService chịu trách nhiệm giải phóng nếu thêm thất bại.

Sau khi `add()` thành công, Repository quản lý vòng đời đối tượng.

### 13.2. Khi add() thất bại

Ví dụ:

```cpp
Ticket* ticket = new Ticket(...);

if (!ticketRepo.add(ticket))
{
    delete ticket;

    // Rollback cac doi tuong da them.
    return false;
}
```

Đoạn trên chỉ minh họa nguyên tắc đối với con trỏ chưa được Repository nhận quyền sở hữu.

Cần tuân thủ chính xác hợp đồng `add()` trong `data_access.md`.

### 13.3. Liên kết Model

Một Ticket có thể liên kết với:

- Showtime.
- Seat.
- Invoice.

Một Invoice có thể liên kết với:

- Showtime.
- User.
- Member.
- Danh sách Ticket.

Khi rollback phải tháo các liên kết do Repository thiết lập, bao gồm liên kết ngược.

Không được để con trỏ treo.

### 13.4. Ngoại lệ

Ngoài các trường hợp `add()` trả `false`, cần xử lý cả ngoại lệ phát sinh trong quá trình tạo đối tượng, cấp ID, tính toán hoặc thêm dữ liệu.

Nếu giao dịch đã thay đổi RAM, ngoại lệ không được làm bỏ qua quá trình dọn dẹp cần thiết.

Không được nuốt ngoại lệ nghiêm trọng rồi trả `false` như một lỗi nghiệp vụ thông thường.

---

## 14. Quy tắc dữ liệu và tính nhất quán

### 14.1. Invoice

- Một Invoice phải có ít nhất một Ticket.
- Các Ticket trong Invoice thuộc cùng một suất chiếu.
- Tổng tiền Invoice bằng tổng tiền thực trả của các Ticket khi bán.
- Invoice giữ nguyên tổng tiền ban đầu sau khi có nghiệp vụ hủy vé.
- Invoice không bị xóa vật lý sau khi bán thành công.

### 14.2. Ticket

- ID duy nhất.
- Ghế thuộc phòng của suất chiếu.
- Không tồn tại hai vé SOLD cùng `(showtimeId, seatId)`.
- Giá gốc, tiền giảm và tiền thực trả không âm.
- Điểm tích lũy không âm.
- Ticket mới bán có trạng thái SOLD.
- Các trường hủy vé ban đầu rỗng hoặc bằng 0.

### 14.3. Member

- Không có thành viên thì không cộng điểm.
- Có thành viên thì cộng tổng điểm của từng vé.
- Không cộng điểm trước khi hoàn tất việc tạo Invoice và Ticket.
- Phải kiểm tra giới hạn số nguyên.
- Khi giao dịch trong RAM thất bại, khôi phục điểm cũ.

### 14.4. Thời gian

- Suất phải chưa bắt đầu tại thời điểm xác nhận bán.
- Không được bán vé cho suất CANCELLED.
- `createdAt` lấy từ DateTime tại thời điểm tạo giao dịch.

### 14.5. ID

Sử dụng `IdGenerator::generateNextId()` với danh sách ID đang nạp.

Không sử dụng `sequences.txt`.

ID Invoice và Ticket phải được cấp đủ trước khi bắt đầu thêm dữ liệu.

Vé và hóa đơn đã bán thành công được giữ làm lịch sử, không xóa vật lý.

---

## 15. Quy tắc xử lý lỗi

### 15.1. Trả về false

Áp dụng cho các trường hợp như:

- Repository chưa load.
- Suất chiếu không tồn tại.
- Suất chiếu đã bắt đầu.
- Suất chiếu bị hủy.
- Phòng không hoạt động.
- Danh sách vé rỗng.
- Ghế không tồn tại.
- Ghế không thuộc phòng.
- Ghế đã bán.
- Ghế bị chọn trùng.
- Loại vé không hợp lệ.
- Thành viên không tồn tại.
- Chưa xác nhận độ tuổi.
- Chưa xác nhận thu tiền.
- Giao dịch không thể tạo nhưng đã rollback RAM an toàn.

### 15.2. Ngoại lệ

Áp dụng theo hợp đồng các Service hiện có đối với:

- Chưa đăng nhập hoặc không đủ quyền.
- Lỗi dữ liệu nghiêm trọng.
- Lỗi cấp ID không thể xử lý.
- Lỗi không thể hoàn tác dữ liệu trong RAM.
- Lỗi ghi file khiến trạng thái giao dịch không chắc chắn.

Không tự ý thay đổi kiểu ngoại lệ do AuthService hoặc Repository đã quy định.

### 15.3. invoiceId

Trước khi thực hiện:

```cpp
invoiceId = "";
```

Khi giao dịch hoàn tất thành công:

```cpp
invoiceId = newInvoiceId;
```

Nếu giao dịch thất bại, `invoiceId` không được chứa một mã hóa đơn được báo là đã bán thành công.

---

## 16. Tích hợp với GUI

### 16.1. Xem ghế trống

GUI gọi:

```cpp
vector<Seat*> seats =
    bookingService.getAvailableSeats(
        showtimeId
    );
```

GUI sử dụng danh sách để hiển thị sơ đồ ghế.

### 16.2. Chọn ghế và Undo

GUI tự quản lý:

```text
Danh sach ghe da chon
Stack Undo
Loai ve cua tung ghe
```

Khi chọn ghế:

- Thêm ghế vào danh sách.
- Đẩy thao tác vào Stack.

Khi Undo:

- Lấy thao tác gần nhất khỏi Stack.
- Cập nhật danh sách ghế và giao diện.

Khi đổi suất hoặc bỏ giao dịch:

- Xóa danh sách lựa chọn.
- Xóa Stack Undo.

Các thao tác này không tạo Ticket hoặc Invoice.

### 16.3. Tính tiền

GUI tạo:

```cpp
vector<BookingItem> items;
```

Sau đó gọi:

```cpp
BookingSummary summary;

bool success =
    bookingService.calculateBooking(
        showtimeId,
        items,
        memberId,
        summary
    );
```

Nếu thành công, GUI hiển thị giá và điểm dự kiến.

Nếu thất bại, GUI yêu cầu kiểm tra lại lựa chọn.

### 16.4. Xác nhận bán

Sau khi nhân viên xác nhận điều kiện khách và đã thu tiền:

```cpp
string invoiceId;

bool success =
    bookingService.createBooking(
        showtimeId,
        items,
        memberId,
        ageConfirmed,
        paymentConfirmed,
        invoiceId
    );
```

GUI chỉ hiển thị giao dịch thành công khi hàm trả `true`.

### 16.5. Lỗi nghiêm trọng

Nếu Backend ném ngoại lệ do lưu file dở, GUI không được tự động gọi lại `createBooking()`.

Cần hiển thị lỗi và yêu cầu kiểm tra trạng thái dữ liệu.

---

## 17. Kiểm thử

File dự kiến:

```text
tests/test_booking_service.cpp
```

### 17.1. getAvailableSeats()

- Suất hợp lệ, tất cả ghế trống.
- Một số ghế đã bán.
- Tất cả ghế đã bán.
- Suất không tồn tại.
- Suất CANCELLED.
- Suất đã bắt đầu.







- Phòng không hoạt động.
- Repository chưa load.
- Vé CANCELLED không chiếm ghế.

### 17.2. calculateBooking()

- Một vé thường.
- Một vé học sinh.
- Ghế VIP.
- Ghế đôi.
- Có thành viên.
- Không có thành viên.
- Áp dụng mức giảm cao nhất.
- Không cộng dồn ưu đãi.
- Nhiều vé cùng hóa đơn.
- Tính điểm từng vé.
- Danh sách rỗng.
- Ghế trùng.
- Ghế đã bán.
- Ghế không thuộc phòng.
- Loại vé không hợp lệ.
- Thành viên không tồn tại.
- Kết quả không làm thay đổi Repository.

### 17.3. createBooking()

- Admin bán vé.
- Staff bán vé.
- Không có quyền bán.
- Chưa đăng nhập.
- Chưa xác nhận độ tuổi.
- Chưa xác nhận thu tiền.
- Bán một vé.
- Bán nhiều vé.
- Bán không có thành viên.
- Bán có thành viên.
- Cộng điểm chính xác.
- Invoice có đúng số Ticket.
- Ticket liên kết đúng Invoice, Showtime và Seat.
- Sinh ID đúng.
- Không tạo trùng ghế SOLD.
- Lưu và load lại dữ liệu thành công.

### 17.4. Rollback

- Thêm Invoice thất bại.
- Thêm Ticket đầu tiên thất bại.
- Thêm Ticket ở giữa thất bại.
- Thêm Ticket cuối cùng thất bại.
- Khôi phục số lượng Invoice.
- Khôi phục số lượng Ticket.
- Khôi phục điểm thành viên.
- Không để lại liên kết sai.
- Không giải phóng bộ nhớ hai lần.
- Ngoại lệ trong quá trình tạo giao dịch.
- Lỗi lưu Invoice.
- Lỗi lưu Ticket sau khi Invoice đã lưu.
- Lỗi lưu Member sau khi Invoice và Ticket đã lưu.
- Không thông báo thành công khi ghi dở.
- Không tự động bán lại sau lỗi ghi dở.

Các ca lỗi ghi file phải được kiểm thử bằng môi trường dữ liệu tạm, không sử dụng trực tiếp dữ liệu vận hành.

---

## 18. Những thay đổi cần triển khai

### 18.1. BookingService.h

- Giữ ba hàm public.
- Giữ `BookingItem`.
- Giữ `BookingSummary`.
- Bổ sung `ageConfirmed` và `paymentConfirmed` cho `createBooking()`.
- Không thêm Undo.
- Không thêm `errorMessage`.

### 18.2. BookingService.cpp

- Rà soát `getAvailableSeats()`.
- Sửa tính điểm trong `calculateBooking()`.
- Bỏ logic phân bổ điểm dư cho vé cuối.
- Kiểm tra xác nhận độ tuổi và thu tiền.
- Hoàn thiện kiểm tra dữ liệu trước giao dịch.
- Bổ sung rollback trong RAM.
- Kiểm tra từng lần lưu TXT.
- Phân biệt lỗi nghiệp vụ và lỗi lưu dữ liệu nghiêm trọng.

### 18.3. TicketRepository và InvoiceRepository

Cần bổ sung cơ chế hoàn tác đối tượng vừa tạo trong giao dịch chưa lưu.

Thiết kế API phải bảo đảm:

- Không xóa lịch sử đã lưu.
- Tháo liên kết an toàn.
- Giải phóng bộ nhớ đúng.
- Không tự động ghi file.
- Không phá vỡ quy chuẩn Repository hiện tại.

### 18.4. Tài liệu liên quan

Sau khi code và test thành công, cập nhật:

- `docs/Business_UI_Contract.md`
- `docs/TongQuan/business_rules.md`
- `docs/TongQuan/TongQuanDeTai.md`
- `docs/development/TongQuanTienDo.md`
- `docs/development/tasks/Task_Hoang.md`
- `docs/development/tasks/Task_Kiet.md`

Đặc biệt phải cập nhật quyết định Undo thuộc GUI.

---

## 19. Trạng thái triển khai

### 19.1. Đã có trong mã nguồn Bước 10

- `BookingItem`.
- `BookingSummary`.
- Constructor.
- `getAvailableSeats()`.
- `calculateBooking()`.
- `createBooking()` phiên bản đầu.
- Kiểm tra quyền `TICKET_SELL`.
- Sinh ID Invoice và Ticket.
- Tạo Invoice và Ticket.
- Cập nhật điểm thành viên.
- Gọi lưu các Repository.

### 19.2. Cần sửa hoặc bổ sung

- Tính điểm theo từng vé.
- Bỏ phân bổ điểm dư.
- Xác nhận độ tuổi.
- Xác nhận thu tiền.
- Rollback khi thêm đối tượng thất bại.
- Xử lý ngoại lệ trong quá trình tạo giao dịch.
- Xử lý lỗi lưu nhiều file.
- Kiểm thử riêng BookingService.
- Cập nhật Business – UI Contract.

### 19.3. Điều kiện nghiệm thu

Chỉ xem Bước 10 hoàn thành khi:

- API đã được thống nhất với GUI.
- Các nghiệp vụ bán vé đúng quy tắc.
- Không còn dữ liệu dở dang trong RAM sau lỗi tạo giao dịch có thể hoàn tác.
- Lỗi lưu nhiều file được xử lý theo giới hạn thiết kế đã công bố.
- Các bài test cần thiết đều đạt.
- Tài liệu phản ánh đúng code.
- Đã kiểm tra tích hợp với các Repository và Service liên quan.

**Trạng thái hiện tại: Đã chốt thiết kế mục tiêu, chưa hoàn tất triển khai và kiểm thử.**
