# BookingService — bán và hủy vé tại quầy

## 1. Tổng quan

`BookingService` thuộc tầng Business. Lớp nhận các Repository và `PricingService`, `AuthService` bằng tham chiếu; không sở hữu chúng và không đọc/ghi TXT trực tiếp. GUI chọn ghế, quản lý Undo và xác nhận tại quầy; Service kiểm tra lại dữ liệu khi bán hoặc hủy.

| Thành phần | Vai trò |
| --- | --- |
| `BookingService.h/.cpp` | Kiểm tra nghiệp vụ, điều phối tạo/hủy vé và lưu |
| `ShowtimeRepository`, `SeatRepository` | Suất, phòng và ghế |
| `TicketRepository`, `InvoiceRepository` | Vé, hóa đơn, liên kết Model và lưu TXT |
| `MemberRepository` | Điểm thành viên |
| `PricingService`, `AuthService` | Giá/điểm và phân quyền |
| `Ticket`, `Invoice`, `Member` | Dữ liệu giao dịch |
| `DateTime`, `IdGenerator`, `Validator` | Thời gian, cấp mã, kiểm tra lý do |

Mã nguồn ở `src/business/`, `src/data_access/`, `src/models/`, `src/utils/`. Dữ liệu tương ứng ở các file `showtimes.txt`, `seats.txt`, `tickets.txt`, `invoices.txt`, `members.txt`, `pricing.txt`.

| API | Kết quả |
| --- | --- |
| `getAvailableSeats(showtimeId)` | `vector<Seat*>` ghế chưa bán của suất hợp lệ |
| `calculateBooking(showtimeId, items, memberId, summary)` | `bool`; tính giá từng vé, tổng tiền và tổng điểm |
| `createBooking(showtimeId, items, memberId, ageConfirmed, paymentConfirmed, invoiceId)` | `bool`; tạo và lưu giao dịch |
| `cancelTicket(ticketId, reason, refundConfirmed)` | `bool`; hủy một vé và lưu thay đổi |

`BookingItem` chứa `seatId` và `ticketType`. `BookingSummary` chứa `ticketPrices` theo thứ tự `items`, `totalAmount` và `earnedPoints`. `rollbackBooking()` là hàm private, không phải API của GUI. Hàm hủy vé thuộc Bước 11 và được giải thích ở Mục 20.

## Hàm khởi tạo BookingService()

### Tổng quan

Nhận các Repository, `PricingService` và `AuthService` bằng tham chiếu; không nạp hoặc lưu dữ liệu. Constructor không có giá trị trả về. Các đối tượng được truyền vào phải tồn tại lâu hơn `BookingService`.

### Quy trình thực hiện

```text
Bắt đầu
    |
    v
Nhận bảy tham chiếu phụ thuộc
    |
    v
Khởi tạo các thành viên cùng tên
    |
    v
Kết thúc
```

### Giải thích chi tiết từng bước kèm code C++

**Bước 1 — Nhận các tham chiếu và khởi tạo thành viên.**

```cpp
BookingService::BookingService(
    ShowtimeRepository& showtimeRepo,
    SeatRepository& seatRepo,
    TicketRepository& ticketRepo,
    InvoiceRepository& invoiceRepo,
    MemberRepository& memberRepo,
    PricingService& pricingService,
    AuthService& authService
)
    : showtimeRepo(showtimeRepo),
      seatRepo(seatRepo),
      ticketRepo(ticketRepo),
      invoiceRepo(invoiceRepo),
      memberRepo(memberRepo),
      pricingService(pricingService),
      authService(authService)
{
}
```

Dấu `&` tạo tham chiếu đến đối tượng bên ngoài; danh sách sau dấu `:` khởi tạo các thành viên trước khi chạy thân constructor. Thân hàm rỗng vì không cần xử lý thêm.

## 2. getAvailableSeats() — xem ghế trống

### Tổng quan

1. Kiểm tra Showtime, Seat và Ticket Repository đã load; thiếu một nguồn thì trả danh sách rỗng.
2. Tìm suất; yêu cầu `SCHEDULED` và chưa bắt đầu.
3. Lấy Room đã liên kết, yêu cầu phòng đang hoạt động.
4. Duyệt ghế của Room, bỏ qua con trỏ rỗng và ghế có Ticket `SOLD` cùng suất; trả các ghế còn lại.

### Quy trình thực hiện

```text
Bắt đầu với showtimeId, tạo result rỗng
    |
    v
Kiểm tra showtimeRepo, seatRepo, ticketRepo đã load
    | thiếu dữ liệu: trả result rỗng
    v
Tìm Showtime theo showtimeId, kiểm tra SCHEDULED
    | không hợp lệ: trả result rỗng
    v
Kiểm tra hiện tại trước startAt
    | đã đến giờ: trả result rỗng
    v
Lấy Room liên kết, kiểm tra phòng đang hoạt động
    | không hợp lệ: trả result rỗng
    v
Duyệt từng Seat trong Room, bỏ qua nullptr
    |
    v
Gọi ticketRepo.isSeatSold(showtimeId, seatId)
    | chưa bán: thêm Seat vào result
    v
Trả danh sách result
```

### Giải thích chi tiết từng bước kèm code C++

**Bước 1 — Khởi tạo**

```cpp
vector<Seat*> BookingService::getAvailableSeats(const string& showtimeId)
{
    vector<Seat*> result;
```

Tạo danh sách ghế kết quả rỗng.

**Bước 2 — Repository**

```cpp
    if (!showtimeRepo.isLoaded() ||
        !seatRepo.isLoaded() ||
        !ticketRepo.isLoaded())
    {
        return result;
    }
```

Nếu một nguồn chưa load, trả danh sách rỗng.

**Bước 3 — Showtime**

```cpp
    Showtime* showtime = showtimeRepo.findById(showtimeId);
    if (showtime == nullptr ||
        showtime->getStatus() != ShowtimeStatus::SCHEDULED)
    {
        return result;
    }
```

`nullptr` là không tìm thấy; chỉ nhận suất `SCHEDULED`.

**Bước 4 — Thời gian**

```cpp
    if (!DateTime::isBefore(DateTime::getCurrentDateTime(), showtime->getStartAt()))
    {
        return result;
    }
```

Giờ hiện tại phải trước giờ bắt đầu.

**Bước 5 — Room**

```cpp
    Room* room = showtime->getRoom();
    if (room == nullptr || !room->isActive())
    {
        return result;
    }
```

Phòng liên kết phải tồn tại và đang hoạt động.

**Bước 6 — Duyệt ghế**

```cpp
    for (int i = 0; i < room->getSeatCount(); i++)
    {
        Seat* seat = room->getSeatAt(i);
        if (seat == nullptr)
        {
            continue;
        }

        if (!ticketRepo.isSeatSold(showtimeId, seat->getSeatId()))
        {
            result.push_back(seat);
        }
    }
```

`continue` bỏ ghế rỗng; `isSeatSold()` lọc vé `SOLD`; `push_back()` thêm ghế trống.

**Bước 7 — Kết quả**

```cpp
    return result;
}
```

Ghế trả về là con trỏ mượn từ Repository, không giữ chỗ.

## 3. calculateBooking() — kiểm tra lựa chọn và báo giá

### Tổng quan

Hàm kiểm tra lựa chọn và tính `BookingSummary` mà không tạo Invoice/Ticket hoặc ghi TXT. Điểm được tính riêng từng vé rồi cộng lại.

### Quy trình thực hiện

```text
Bắt đầu, đặt summary mặc định và tạo calculated
    |
    v
Kiểm tra Repository đã load
    | chưa load: return false
    v
Kiểm tra Showtime SCHEDULED, chưa bắt đầu và Room hoạt động
    | không hợp lệ: return false
    v
Kiểm tra items không rỗng và Member nếu có
    | không hợp lệ: return false
    v
Duyệt từng BookingItem, tìm Seat theo roomId và seatId
    | không có ghế: return false
    v
Kiểm tra ghế chưa bán, không trùng, ticketType hợp lệ
    | không hợp lệ: return false
    v
Tính giá gốc, giảm giá, tiền trả và điểm của vé
    |
    v
Kiểm tra tổng tiền hữu hạn, tổng điểm không tràn
    | không hợp lệ: return false
    v
Cộng vào calculated, tiếp tục đến hết items
    |
    v
Gán calculated cho summary; return true
```

### Giải thích chi tiết từng bước kèm code C++

**Bước 1 — Báo giá tạm**

```cpp
bool BookingService::calculateBooking(const string& showtimeId, const vector<BookingItem>& items, const string& memberId, BookingSummary& summary)
{
    summary = BookingSummary{};
    BookingSummary calculated;
```

`BookingSummary{}` xóa giá trị cũ; `calculated` giữ dữ liệu chưa hoàn tất.

**Bước 2 — Repository**

```cpp
    // Kiem tra cac Repository
    if (!showtimeRepo.isLoaded() ||
        !seatRepo.isLoaded() ||
        !ticketRepo.isLoaded() ||
        !memberRepo.isLoaded())
    {
        return false;
    }
```

Thiếu dữ liệu đã nạp thì trả `false`.

**Bước 3 — Showtime và Room**

```cpp
    // Kiem tra suat chieu
    Showtime* showtime = showtimeRepo.findById(showtimeId);
    if (showtime == nullptr ||
        showtime->getStatus() != ShowtimeStatus::SCHEDULED)
    {
        return false;
    }
    if (!DateTime::isBefore(DateTime::getCurrentDateTime(), showtime->getStartAt()))
    {
        return false;
    }
    Room* room = showtime->getRoom();
    if (room == nullptr || !room->isActive())
    {
        return false;
    }
```

Suất phải `SCHEDULED`, chưa bắt đầu; phòng đang hoạt động.

**Bước 4 — Items và Member**

```cpp

    if (items.empty())
    {
        return false;
    }
    // Kiem tra thanh vien
    bool isMember = !memberId.empty();

    if (isMember && memberRepo.findById(memberId) == nullptr)
    {
        return false;
    }
```

Danh sách không rỗng; Member được tra khi `memberId` có giá trị.

**Bước 5 — Tìm Seat**

```cpp

    // Tinh tien tung ve
    for (int i = 0; i < static_cast<int>(items.size()); i++)
    {
        const BookingItem& item = items[i];

        // Kiem tra ghe thuoc phong
        Seat* seat = seatRepo.findById(showtime->getRoomId(), item.seatId);

        if (seat == nullptr)
        {
            return false;
        }
```

Duyệt từng `BookingItem`, tìm ghế theo phòng và mã ghế.

**Bước 6 — Ghế đã bán**

```cpp

        // Kiem tra ghe da ban
        if (ticketRepo.isSeatSold(showtimeId, item.seatId))
        {
            return false;
        }
```

Vé `SOLD` cùng suất/ghế khiến lựa chọn không hợp lệ.

**Bước 7 — Trùng ghế và loại vé**

```cpp
        // Kiem tra ghe bi chon trung
        for (int j = 0; j < i; j++)
        {
            if (items[j].seatId == item.seatId)
            {
                return false;
            }
        }

        // Kiem tra loai ve
        if (item.ticketType != TicketType::REGULAR &&
            item.ticketType != TicketType::STUDENT)
        {
            return false;
        }
```

Vòng trong tìm ghế chọn lặp; chỉ nhận `REGULAR`/`STUDENT`.

**Bước 8 — Giá và điểm**

```cpp

        // Tinh gia ve
        double basePrice = pricingService.getBasePrice(seat->getSeatType());
        double discount = pricingService.calculateDiscount(basePrice, item.ticketType, isMember);
        double paidAmount = pricingService.calculatePaidAmount(basePrice, discount);
        int points = pricingService.calculatePoints(paidAmount, isMember);
        if (!isfinite(paidAmount) || paidAmount < 0 ||
            !isfinite(calculated.totalAmount + paidAmount) ||
            points > numeric_limits<int>::max() - calculated.earnedPoints)
        {
            return false;
        }
        calculated.ticketPrices.push_back(paidAmount);
        calculated.totalAmount += paidAmount;
        calculated.earnedPoints += points;
    }
```

Tính trên từng vé; `isfinite()` chặn NaN/vô cực, `numeric_limits<int>::max()` chặn tràn điểm. `push_back()` giữ đúng thứ tự vé.

**Bước 9 — Công bố**

```cpp
    summary = calculated;
    return true;
}
```

Chỉ gán `summary` sau khi tất cả vé đã tính xong.

## 4. createBooking() — bán và lưu nhiều vé

### Tổng quan

Hàm yêu cầu quyền bán, xác nhận tuổi và thu tiền, tạo một Invoice cùng nhiều Ticket rồi tự lưu TXT. Người bán lấy từ phiên đăng nhập. Lỗi trước khi lưu được hoàn tác trong RAM; lỗi ghi file ném ngoại lệ.

### Quy trình thực hiện

```text
Bắt đầu, xóa invoiceId; kiểm tra quyền TICKET_SELL
    | thiếu quyền: throw
    v
Kiểm tra storageFailed, xác nhận tuổi và thu tiền
    | lỗi trước: throw; thiếu xác nhận: return false
    v
Kiểm tra Repository, gọi calculateBooking()
    | không hợp lệ: return false
    v
Lấy staffId, createdAt, Showtime và Member
    |
    v
Sinh trước ID Invoice và các ID Ticket
    |
    v
Kiểm tra giới hạn điểm, chuẩn bị giá/điểm từng vé
    | không hợp lệ: return false
    v
Tạo và thêm Invoice, rồi từng Ticket
    | lỗi trước lưu: rollbackBooking(); false hoặc throw
    v
Cộng điểm Member nếu có
    |
    v
Lưu Invoice → Ticket → Member (nếu có)
    | lỗi ghi: storageFailed = true; throw
    v
Gán invoiceId; return true
```

### Giải thích chi tiết từng bước kèm code C++

**Bước 1 — Quyền và xác nhận**

```cpp
bool BookingService::createBooking(const string& showtimeId,
                                   const vector<BookingItem>& items,
                                   const string& memberId,
                                   bool ageConfirmed,
                                   bool paymentConfirmed,
                                   string& invoiceId)
{
    invoiceId = "";
    const User& staff = authService.requirePermission("TICKET_SELL");
    if (storageFailed)
    {
        throw runtime_error("Giao dich truoc ghi TXT loi; can kiem tra du lieu truoc khi ban tiep");
    }
    if (!ageConfirmed || !paymentConfirmed)
    {
        return false;
    }
```

`requirePermission()` lấy người bán hoặc ném lỗi. `storageFailed` chặn bán tiếp; thiếu xác nhận trả `false`.

**Bước 2 — Kiểm tra và báo giá**

```cpp

    // 2. Kiem tra Repository
    if (!showtimeRepo.isLoaded() ||
        !seatRepo.isLoaded() ||
        !ticketRepo.isLoaded() ||
        !invoiceRepo.isLoaded() ||
        !memberRepo.isLoaded())
    {
        return false;
    }
    // 3. Kiem tra du lieu va tinh tien
    BookingSummary summary;
    if (!calculateBooking(showtimeId, items, memberId, summary))
    {
        return false;
    }
```

Gọi `calculateBooking()` để kiểm tra lại ghế và giá tại lúc bán.

**Bước 3 — Thông tin giao dịch**

```cpp

    // 4. Lay thong tin giao dich
    string staffId = staff.getUserId();
    string createdAt = DateTime::getCurrentDateTime();
    Showtime* showtime = showtimeRepo.findById(showtimeId);
    Member* member = nullptr;
    if (!memberId.empty())
    {
        member = memberRepo.findById(memberId);
    }
```

Lấy người bán từ phiên đăng nhập và thời điểm hiện tại; Member có thể là `nullptr`.

**Bước 4 — ID Invoice**

```cpp
    // 5. Sinh ID hoa don
    vector<string> invoiceIds;
    for (int i = 0; i < invoiceRepo.count(); i++)
    {
        Invoice* invoice = invoiceRepo.getAt(i);
        if (invoice == nullptr)
        {
            return false;
        }
        invoiceIds.push_back(invoice->getInvoiceId());
    }
    string newInvoiceId = IdGenerator::generateNextId("INV", invoiceIds);
```

Duyệt ID đang nạp rồi sinh ID kế tiếp.

**Bước 5 — ID Ticket**

```cpp
    // 6. Lay danh sach ID ve
    vector<string> ticketIds;
    for (int i = 0; i < ticketRepo.count(); i++)
    {
        Ticket* ticket = ticketRepo.getAt(i);
        if (ticket == nullptr)
        {
            return false;
        }
        ticketIds.push_back(ticket->getTicketId());
    }

    // Sinh truoc ID cho tat ca ve
    vector<string> newTicketIds;
    for (int i = 0; i < static_cast<int>(items.size()); i++)
    {
        string ticketId = IdGenerator::generateNextId("TKT", ticketIds);
        newTicketIds.push_back(ticketId);
        ticketIds.push_back(ticketId);
    }
```

Sinh đủ mã trước khi sửa RAM; thêm ID vừa sinh vào danh sách để tránh trùng.

**Bước 6 — Giới hạn và giá/điểm**

```cpp

    // 7. Kiem tra gioi han diem
    if (member != nullptr && summary.earnedPoints > numeric_limits<int>::max() - member->getPoints())
    {
        return false;
    }

    vector<double> basePrices; // lưu giá gốc
    vector<int> ticketPoints;
    for (size_t i = 0; i < items.size(); i++)
    {
        Seat* seat = seatRepo.findById(showtime->getRoomId(), items[i].seatId);
        if (seat == nullptr)
        {
            return false;
        }
        basePrices.push_back(pricingService.getBasePrice(seat->getSeatType()));
        ticketPoints.push_back(pricingService.calculatePoints(summary.ticketPrices[i], member != nullptr));
    }
```

Chặn tràn điểm, chuẩn bị giá gốc và điểm của từng Ticket.

**Bước 7 — Theo dõi rollback**

```cpp

    vector<string> addedTicketIds; // theo dõi các id vừa thêm
    addedTicketIds.reserve(items.size()); //thêm dung lượng cho vector
    bool invoiceAdded = false; // Đánh dấu Invoice đã được thêm hay chưa
    bool ticketAddFailed = false; // vé thêm vào bị lỗi
    int oldPoints = member == nullptr ? 0 : member->getPoints();
```

`reserve()` chuẩn bị dung lượng; giữ ID vừa thêm, cờ Invoice và điểm cũ.

**Bước 8 — Thêm Invoice**

```cpp
    try
    {
        // tạo hóa đơn
        Invoice* invoice = new Invoice(newInvoiceId, showtimeId, staffId, memberId, createdAt, summary.totalAmount);
        try
        {
            if (!invoiceRepo.add(invoice))
            {
                delete invoice;
                return false;
            }
        }
        catch (...)
        {
            delete invoice;
            throw;
        }
        invoiceAdded = true;
```

`new` cấp phát; Service xóa nếu Repository chưa nhận quyền sở hữu.

**Bước 9 — Thêm Ticket**

```cpp
        // Tạo từng vé
        for (size_t i = 0; i < items.size(); i++)
        {
            double paidAmount = summary.ticketPrices[i];
            addedTicketIds.push_back(newTicketIds[i]);
            Ticket* ticket = nullptr;
            try
            {
                ticket = new Ticket(newTicketIds[i], newInvoiceId, showtimeId,
                                    items[i].seatId, items[i].ticketType,
                                    basePrices[i], basePrices[i] - paidAmount,
                                    paidAmount, ticketPoints[i], TicketStatus::SOLD,
                                    "", "", "", 0);
                if (!ticketRepo.add(ticket))
                {
                    delete ticket;
                    addedTicketIds.pop_back();
                    ticketAddFailed = true;
                    break;
                }
            }
            catch (...)
            {
                delete ticket;
                addedTicketIds.pop_back();
                throw;
            }
        }
```

Tạo từng vé, gọi `add()`. Nhánh thất bại xóa con trỏ chưa nhận và bỏ ID khỏi danh sách.

**Bước 10 — Cộng điểm/ngoại lệ RAM**

```cpp
        
        if (!ticketAddFailed && member != nullptr)
        {
            member->setPoints(oldPoints + summary.earnedPoints);
        }
    }
    catch (...)
    {
        rollbackBooking(addedTicketIds,newInvoiceId, invoiceAdded, member, oldPoints);
        throw;
    }
```

Chỉ cộng khi mọi vé đã thêm. `catch (...)` rollback và `throw;` ném lại lỗi.

**Bước 11 — `add()` trả false**

```cpp
    if (ticketAddFailed)
    {
        rollbackBooking(addedTicketIds,newInvoiceId, invoiceAdded, member, oldPoints);
        return false;
    }
```

Rollback RAM rồi trả `false`.

**Bước 12 — Lưu TXT**

```cpp

    // Tu lan ghi dau tien, file co the da thay doi du save() tra false.
    try
    {
        if (!invoiceRepo.save())
        {
            throw runtime_error("Khong the ghi file Invoice");
        }
        if (!ticketRepo.save())
        {
            throw runtime_error("Khong the ghi file Ticket; Invoice co the da duoc luu");
        }
        if (member != nullptr && !memberRepo.save())
        {
            throw runtime_error("Khong the ghi file Member; Invoice/Ticket co the da duoc luu");
        }
    }
    catch (...)
    {
        storageFailed = true;
        throw;
    }
```

Lưu Invoice → Ticket → Member; lỗi ghi đặt `storageFailed` và ném ngoại lệ.

**Bước 13 — Thành công**

```cpp
    invoiceId = newInvoiceId;
    return true;
}
```

Chỉ lúc này mới gán `invoiceId` và trả `true`.

#### rollbackBooking() — hoàn tác RAM trước khi lưu

**Bước 1 — Điểm**

```cpp
void BookingService::rollbackBooking(const vector<string>& addedTicketIds,
                                     const string& invoiceId,
                                     bool invoiceAdded,
                                     Member* member,
                                     int oldPoints)
{
    // Khoi phuc diem thanh vien
    if (member != nullptr)
    {
        member->setPoints(oldPoints);
    }
```

Nếu có Member, khôi phục điểm cũ.

**Bước 2 — Ticket**

```cpp
    // Rollback Ticket theo thu tu nguoc
    for (int i = static_cast<int>(addedTicketIds.size()) - 1; i >= 0; i--)
    {
        if (!ticketRepo.rollbackPending(addedTicketIds[i]))
        {
            throw runtime_error("Khong the rollback Ticket trong RAM");
        }
    }
```

Duyệt vé vừa thêm từ cuối về đầu; `rollbackPending()` tháo liên kết và xóa vé chưa lưu.

**Bước 3 — Invoice**

```cpp
    // Rollback Invoice sau khi da xoa het Ticket
    if (invoiceAdded && !invoiceRepo.rollbackPending(invoiceId))
    {
        throw runtime_error("Khong the rollback Invoice trong RAM");
    }
}
```

Sau khi vé đã tháo hết mới hoàn tác Invoice pending.

## 5. Kiểm thử Bước 10 và giới hạn

`tests/test_booking_service.cpp` đã chạy **55 PASS, 0 FAIL** ngày 2026-11-22: báo giá và điểm từng vé, quyền bán, xác nhận tại quầy, liên kết Model, lưu/đọc lại, rollback trực tiếp một/nhiều Ticket pending và lỗi ghi Invoice/Ticket/Member bằng file tạm. Test kiểm tra `rollbackPending()` nhiều Ticket qua Repository; **chưa tự động ép `TicketRepository::add()` thất bại giữa `createBooking()`** để đi qua `rollbackBooking()` trong tình huống đó.

## 20. Hủy vé — Bước 11

### 20.1. Tổng quan

`cancelTicket(ticketId, reason, refundConfirmed)` hủy **một Ticket**. Chỉ Admin có quyền `TICKET_CANCEL` được gọi; Admin ID lấy từ phiên đăng nhập. Vé và hóa đơn được giữ làm lịch sử, Invoice không đổi tổng tiền. GUI nhập lý do, xác nhận đã hoàn tiền tại quầy, gọi Service và làm mới sơ đồ ghế. Service không thực hiện chuyển tiền điện tử.

Trạng thái bán ghế được suy ra từ `TicketRepository::isSeatSold(showtimeId, seatId)`. Sau khi vé chuyển từ `SOLD` sang `CANCELLED`, ghế tự trở lại danh sách có thể bán; `Seat` không bị sửa. Nếu bán lại, hệ thống tạo Ticket ID mới và giữ vé cũ.

### 20.2. Tổng quan quy trình

1. Yêu cầu vai trò Admin và quyền RBAC `TICKET_CANCEL`; chặn khi `storageFailed` đã bật.
2. Kiểm tra các Repository đã load, đã xác nhận hoàn tiền và `reason` hợp lệ theo `Validator::isValidText()`.
3. Tìm Ticket `SOLD`; đối chiếu con trỏ Invoice, Showtime, Seat và bảo đảm Ticket có trong danh sách Ticket của Invoice.
4. Lấy thời điểm hiện tại; suất chưa bắt đầu và thời điểm hủy không trước thời điểm lập Invoice.
5. Nếu Invoice có Member, đối chiếu liên kết và bảo đảm điểm hiện có đủ thu hồi đúng `ticket->getPointsEarned()`.
6. Gọi `Ticket::markCancelled()`, trừ điểm Member nếu có; lưu **Ticket → Member**. Không lưu Invoice vì dữ liệu Invoice không đổi.

### 20.3. Quy trình thực hiện

```text
Bắt đầu, requireAdmin() và requirePermission("TICKET_CANCEL")
    | thiếu quyền: throw
    v
Kiểm tra storageFailed và các Repository đã load
    | lỗi lưu trước: throw; chưa load: return false
    v
Kiểm tra refundConfirmed và Validator::isValidText(reason)
    | không hợp lệ: return false
    v
Tìm Ticket theo ticketId, yêu cầu trạng thái SOLD
    | không hợp lệ: return false
    v
Đối chiếu Invoice, Showtime, Seat và liên kết Repository
    | liên kết sai: return false
    v
Xác nhận Ticket có trong danh sách của Invoice
    | không có: return false
    v
Lấy cancelledAt; kiểm tra suất chưa bắt đầu
    | đã bắt đầu: return false
    v
Kiểm tra cancelledAt không trước Invoice.createdAt
    | không hợp lệ: return false
    v
Nếu có Member: kiểm tra liên kết và đủ pointsEarned
    | không đủ điểm: return false
    v
Gọi ticket->markCancelled(); trừ điểm Member nếu có
    |
    v
Lưu ticketRepo.save(), rồi memberRepo.save() nếu có Member
    | lỗi ghi: storageFailed = true; throw
    v
Return true; Ticket hủy không còn chiếm ghế
```

### 20.4. Giải thích chi tiết từng bước kèm code C++

**Bước 1 — Phân quyền và chặn dữ liệu lỗi**

```cpp
bool BookingService::cancelTicket(const string& ticketId, const string& reason, bool refundConfirmed)
{
    const User& admin = authService.requireAdmin();
    authService.requirePermission("TICKET_CANCEL");
    if (storageFailed)
    {
        throw runtime_error("Giao dich truoc ghi TXT loi; can kiem tra du lieu truoc khi huy tiep");
    }
```

`requireAdmin()` kiểm tra vai trò; `requirePermission()` kiểm tra RBAC. `storageFailed` ngăn hủy tiếp trên cùng Service sau lỗi ghi.

**Bước 2 — Kiểm tra nguồn và xác nhận**

```cpp
    if (!showtimeRepo.isLoaded() || !seatRepo.isLoaded() ||
        !ticketRepo.isLoaded() || !invoiceRepo.isLoaded() ||
        !memberRepo.isLoaded())
    {
        return false;
    }
    if (!refundConfirmed || !Validator::isValidText(reason))
    {
        return false;
    }
```

Repository phải đã load. GUI phải xác nhận hoàn tiền, và `Validator::isValidText()` từ chối lý do rỗng, chỉ khoảng trắng, ký tự `|` hoặc ký tự điều khiển.

**Bước 3 — Tìm vé đang bán**

```cpp
    Ticket* ticket = ticketRepo.findById(ticketId);
    if (ticket == nullptr || ticket->getStatus() != TicketStatus::SOLD)
    {
        return false;
    }
```

Chỉ Ticket tồn tại và còn `SOLD` được hủy.

**Bước 4 — Đối chiếu liên kết cơ bản**

```cpp
    Invoice* invoice = ticket->getInvoice();
    const Showtime* showtime = ticket->getShowtime();
    if (invoice == nullptr || showtime == nullptr || ticket->getSeat() == nullptr ||
        invoiceRepo.findById(ticket->getInvoiceId()) != invoice ||
        showtimeRepo.findById(ticket->getShowtimeId()) != showtime ||
        invoice->getShowtimeId() != ticket->getShowtimeId())
    {
        return false;
    }
```

Kiểm tra Invoice, Showtime, Seat và con trỏ của chúng còn đúng đối tượng trong Repository.

**Bước 5 — Xác nhận Ticket thuộc Invoice**

```cpp
    bool linkedToInvoice = false;
    for (int i = 0; i < invoice->getTicketCount(); i++)
    {
        if (invoice->getTicketAt(i) == ticket)
        {
            linkedToInvoice = true;
            break;
        }
    }
    if (!linkedToInvoice)
    {
        return false;
    }
```

Duyệt danh sách vé của Invoice; chỉ so sánh ID sẽ chưa đủ khi con trỏ liên kết đã sai.

**Bước 6 — Kiểm tra thời gian**

```cpp

    const string cancelledAt = DateTime::getCurrentDateTime();
    if (!DateTime::isBefore(cancelledAt, showtime->getStartAt()) ||
        DateTime::isBefore(cancelledAt, invoice->getCreatedAt()))
    {
        return false;
    }
```

`cancelledAt` phải trước giờ chiếu và không trước thời điểm lập hóa đơn.

**Bước 7 — Kiểm tra Member và điểm**

```cpp

    Member* member = nullptr;
    if (!invoice->getMemberId().empty())
    {
        member = memberRepo.findById(invoice->getMemberId());
        if (member == nullptr || invoice->getMember() != member ||
            member->getPoints() < ticket->getPointsEarned())
        {
            return false;
        }
    }
    else if (invoice->getMember() != nullptr)
    {
        return false;
    }
```

Nếu Invoice có Member, liên kết phải đúng và điểm đủ thu hồi `pointsEarned` đã lưu trên Ticket. Không tính lại điểm.

**Bước 8 — Đổi Ticket và điểm trong RAM**

```cpp
    // ghi lại thông tin hủy trên vé
    ticket->markCancelled(cancelledAt, admin.getUserId(), reason);
    if (member != nullptr)
    {
        member->setPoints(member->getPoints() - ticket->getPointsEarned());
    }
```

`markCancelled()` ghi thời gian, Admin, lý do, trạng thái `CANCELLED` và tiền hoàn bằng tiền trả; Member bị trừ đúng điểm của vé.

**Bước 9 — Lưu Ticket rồi Member**

```cpp

    try
    {
        if (!ticketRepo.save())
        {
            throw runtime_error("Khong the ghi file Ticket khi huy ve");
        }
        if (member != nullptr && !memberRepo.save())
        {
            throw runtime_error("Khong the ghi file Member sau khi huy ve; Ticket co the da duoc luu");
        }
    }
    catch (...)
    {
        storageFailed = true;
        throw;
    }
```

Không lưu Invoice vì dữ liệu Invoice không đổi. Lỗi ghi đặt `storageFailed` và ném ngoại lệ; file có thể đã ghi một phần.

**Bước 10 — Trả thành công**

```cpp
    return true;
}
```

Chỉ khi các lần ghi cần thiết hoàn tất mới trả `true`. Ghế tự trống vì `isSeatSold()` bỏ qua Ticket `CANCELLED`.

### 20.5. Kiểm thử và phạm vi còn lại

`tests/test_cancel_ticket.cpp` đã chạy **32 PASS, 0 FAIL** ngày 2026-11-22: hủy hợp lệ, quyền Admin/Staff và thiếu permission, xác nhận hoàn tiền, lý do, điểm không đủ, suất đã bắt đầu, liên kết thiếu, thời điểm hủy trước Invoice, một vé trong hóa đơn nhiều vé, ghế trống và bán lại, không có Member, đọc lại vé hủy, lỗi lưu Ticket/Member và chặn giao dịch tiếp trên cùng Service. Test Repository vé/hóa đơn liên quan đã chạy **149 PASS, 0 FAIL**.

Phần hủy vé Backend đã triển khai và kiểm thử PASS. Bước 11 còn nghiệp vụ lịch sử hóa đơn, suy ra trạng thái Invoice/tiền còn lại và thống kê. Chưa có transaction đa file hoặc tự phục hồi sau lỗi ghi; `storageFailed` chỉ sống cùng đối tượng Service.
