# TicketRepository

## 1. Tổng quan

`TicketRepository` thuộc tầng **Data Access**, chịu trách nhiệm quản lý dữ liệu vé, kiểm tra tính hợp lệ và duy trì liên kết giữa Ticket với Showtime, Seat và Invoice.

Lớp triển khai `IRepository<Ticket>` và sử dụng `Repository<Ticket>` theo cơ chế composition.

**Các file liên quan:**
- `src/data_access/TicketRepository.h`
- `src/data_access/TicketRepository.cpp`
- `src/data_access/IRepository.h`
- `src/data_access/Repository.h`
- `src/models/Ticket.h/.cpp`
- `src/models/Invoice.h/.cpp`

**Trạng thái: ĐANG REVIEW THIẾT KẾ.**

### Những thay đổi so với phiên bản cũ

| Thành phần | Thiết kế mới |
|---|---|
| Kế thừa | `IRepository<Ticket>` |
| Dependency | `IRepository<Showtime>&`, `IRepository<Seat>&`, `IRepository<Invoice>&` |
| `pendingIds` | Loại bỏ |
| `rollbackPending()` | Thay bằng `removeById()` |
| `update()` | Hỗ trợ chuyển `SOLD → CANCELLED` |
| `getByShowtime()` | Loại bỏ |
| `getByInvoice()` | Loại bỏ |
| `isSeatSold()` | Chuyển thành private |
| Tra cứu Seat | Dùng `IRepository<Seat>::findById(roomId + "|" + seatId)` theo khóa ghép hiện tại |

## 2. Kiến trúc và quản lý bộ nhớ

```text
             IRepository<Ticket>
                     ^
                     |
              TicketRepository
                     |
          +----------+----------+
          |          |          |
 Repository<Ticket>  |          |
                     |          |
       IRepository<Showtime>&    |
       IRepository<Seat>&        |
       IRepository<Invoice>& ----+
```

`Repository<Ticket>` chịu trách nhiệm sở hữu và giải phóng các đối tượng Ticket đã được thêm thành công hoặc được nạp từ TXT.

Các Repository được inject chỉ là tham chiếu, không thuộc quyền sở hữu của TicketRepository.

**Quy tắc ownership:**

- `add()` thành công: Repository sở hữu `Ticket*`.
- `add()` thất bại: caller vẫn sở hữu `Ticket*`.
- `removeById()` thành công: Ticket được unlink trước khi xóa.
- `load()` lỗi sau khi nạp dữ liệu: unlink toàn bộ rồi clear.
- Destructor: unlink trước khi `Repository<Ticket>` giải phóng bộ nhớ.

`Invoice` quản lý danh sách con trỏ Ticket thông qua `linkTicket()` và `unlinkTicket()`. Vì vậy phải tháo liên kết trước khi xóa Ticket.

## 3. Định dạng dữ liệu TXT

Mỗi Ticket được lưu trên một dòng với 14 trường:

```text
ticketId|invoiceId|showtimeId|seatId|ticketType|basePrice|discountAmount|paidAmount|pointsEarned|status|cancelledAt|cancelledBy|cancelReason|refundAmount
```

### 3.1. Quy tắc validation

- `ticketId`: ID hợp lệ với tiền tố `TKT`.
- `invoiceId`: ID hợp lệ với tiền tố `INV`.
- `showtimeId`: ID hợp lệ với tiền tố `SHW`.
- `seatId`: chuỗi hợp lệ.
- `ticketType`: `REGULAR` hoặc `STUDENT`.
- `basePrice`, `discountAmount`, `paidAmount`, `refundAmount`: hữu hạn, không âm.
- `discountAmount <= basePrice`.
- `paidAmount = basePrice - discountAmount`, sai số tối đa `0.000001`.
- `pointsEarned >= 0`.
- `status`: `SOLD` hoặc `CANCELLED`.

Với vé `SOLD`:
- Thông tin hủy phải rỗng.
- `refundAmount = 0`.

Với vé `CANCELLED`:
- Thông tin hủy phải hợp lệ.
- `refundAmount = paidAmount`.
- Thời điểm hủy không trước thời điểm tạo hóa đơn và phải trước giờ bắt đầu suất chiếu.

### 3.2. Quy tắc liên kết

- Showtime phải tồn tại.
- Invoice phải tồn tại.
- Seat phải thuộc Room của Showtime.
- Invoice và Ticket phải cùng Showtime.
- Ticket có `pointsEarned > 0` phải thuộc Invoice đã liên kết với Member hợp lệ.
- Chỉ có tối đa một Ticket `SOLD` cho cùng `(showtimeId, seatId)`.
- Ticket `CANCELLED` vẫn tồn tại trong lịch sử nhưng không chiếm ghế.

## 4. Thiết kế lớp

### 4.1. Thuộc tính

| Thuộc tính | Kiểu | Vai trò |
|---|---|---|
| `repo` | `Repository<Ticket>` | Quản lý dữ liệu Ticket |
| `showtimeRepo` | `IRepository<Showtime>&` | Tra cứu Showtime |
| `seatRepo` | `IRepository<Seat>&` | Tra cứu Seat |
| `invoiceRepo` | `IRepository<Invoice>&` | Tra cứu Invoice |

### 4.2. Các hàm private

| Hàm | Trách nhiệm |
|---|---|
| `getTicketId()` | Lấy ID Ticket |
| `stringToTicket()` | Chuyển TXT thành Ticket |
| `ticketToString()` | Chuyển Ticket thành TXT |
| `isValidTicket()` | Kiểm tra dữ liệu Ticket |
| `isValidReference()` | Kiểm tra tham chiếu |
| `isSeatSold()` | Kiểm tra ghế đã bán |
| `unlinkAll()` | Gỡ toàn bộ liên kết |

### 4.3. Các hàm public

Triển khai đủ 9 hàm từ `IRepository<Ticket>`:

1. `load()`
2. `save()`
3. `add()`
4. `update()`
5. `removeById()`
6. `findById()`
7. `getAt()`
8. `count()`
9. `isLoaded()`

Ngoài ra có constructor và destructor.

## 5. Mã nguồn dự kiến

### 5.1. TicketRepository.h

```cpp
#ifndef TICKET_REPOSITORY_H
#define TICKET_REPOSITORY_H

#include <string>
#include "IRepository.h"
#include "Repository.h"
#include "../models/Ticket.h"

class Showtime;
class Seat;
class Invoice;

class TicketRepository : public IRepository<Ticket>
{
private:
    Repository<Ticket> repo;
    IRepository<Showtime>& showtimeRepo;
    IRepository<Seat>& seatRepo;
    IRepository<Invoice>& invoiceRepo;

    static bool isValidTicket(const Ticket* ticket);
    static std::string getTicketId(Ticket* ticket);
    static Ticket* stringToTicket(const std::string& line);
    static std::string ticketToString(Ticket* ticket);

    bool isValidReference(const Ticket* ticket);

    bool isSeatSold(const std::string& showtimeId,
                    const std::string& seatId);
    void unlinkAll();

public:
    TicketRepository(const std::string& filePath,
                     IRepository<Showtime>& showtimeRepo,
                     IRepository<Seat>& seatRepo,
                     IRepository<Invoice>& invoiceRepo);
    ~TicketRepository() override;

    // Doc va ghi du lieu
    bool load() override;
    bool save() override;

    // Quan ly du lieu
    bool add(Ticket* ticket) override;
    bool update(const Ticket& newValues) override;
    bool removeById(const std::string& ticketId) override;

    // Truy van du lieu
    Ticket* findById(const std::string& ticketId) override;
    Ticket* getAt(int index) override;
    int count() override;
    bool isLoaded() override;
};

#endif
```

### 5.2. TicketRepository.cpp

```cpp
#include "TicketRepository.h"
#include "../models/Showtime.h"
#include "../models/Seat.h"
#include "../models/Invoice.h"
#include "../utils/Validator.h"
#include "../utils/DateTime.h"
#include <cmath>
#include <stdexcept>
#include <vector>
using namespace std;

bool TicketRepository::isValidTicket(const Ticket* ticket)
{
    if (ticket == nullptr)
    {
        return false;
    }

    if (!Validator::isValidId(ticket->getTicketId(), "TKT") ||
        !Validator::isValidId(ticket->getInvoiceId(), "INV") ||
        !Validator::isValidId(ticket->getShowtimeId(), "SHW") ||
        !Validator::isValidText(ticket->getSeatId()))
    {
        return false;
    }

    if (ticket->getTicketType() != TicketType::REGULAR &&
        ticket->getTicketType() != TicketType::STUDENT)
    {
        return false;
    }

    TicketStatus status = ticket->getStatus();

    if (status != TicketStatus::SOLD &&
        status != TicketStatus::CANCELLED)
    {
        return false;
    }

    double base = ticket->getBasePrice();
    double discount = ticket->getDiscountAmount();
    double paid = ticket->getPaidAmount();
    double refund = ticket->getRefundAmount();

    if (!isfinite(base) || !isfinite(discount) ||
        !isfinite(paid) || !isfinite(refund) ||
        base < 0 || discount < 0 ||
        paid < 0 || refund < 0 ||
        ticket->getPointsEarned() < 0 ||
        discount > base ||
        fabs(paid - (base - discount)) > 0.000001)
    {
        return false;
    }

    if (status == TicketStatus::SOLD)
    {
        return ticket->getCancelledAt().empty() &&
               ticket->getCancelledBy().empty() &&
               ticket->getCancelReason().empty() &&
               refund == 0;
    }

    return Validator::isValidDateTime(ticket->getCancelledAt()) &&
           Validator::isValidId(ticket->getCancelledBy(), "USR") &&
           Validator::isValidText(ticket->getCancelReason()) &&
           fabs(refund - paid) <= 0.000001;
}

TicketRepository::TicketRepository(const string& filePath,
                                     IRepository<Showtime>& showtimeRepo,
                                     IRepository<Seat>& seatRepo,
                                     IRepository<Invoice>& invoiceRepo)
    : repo(filePath, getTicketId, stringToTicket, ticketToString),
      showtimeRepo(showtimeRepo),
      seatRepo(seatRepo),
      invoiceRepo(invoiceRepo)
{
}

string TicketRepository::getTicketId(Ticket* ticket)
{
    if (ticket == nullptr)
    {
        return "";
    }
    return ticket->getTicketId();
}

Ticket* TicketRepository::stringToTicket(const string& line)
{
    vector<string> fields;
    size_t start = 0;

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

    if (fields.size() != 14)
    {
        throw runtime_error("Sai so luong truong Ticket");
    }

    TicketType type;

    if (fields[4] == "REGULAR")
    {
        type = TicketType::REGULAR;
    }
    else if (fields[4] == "STUDENT")
    {
        type = TicketType::STUDENT;
    }
    else
    {
        throw runtime_error("Loai ve khong hop le");
    }

    TicketStatus status;

    if (fields[9] == "SOLD")
    {
        status = TicketStatus::SOLD;
    }
    else if (fields[9] == "CANCELLED")
    {
        status = TicketStatus::CANCELLED;
    }
    else
    {
        throw runtime_error("Trang thai ve khong hop le");
    }

    double basePrice;
    double discountAmount;
    double paidAmount;
    double refundAmount;
    int pointsEarned;

    try
    {
        int indexes[] = {5, 6, 7, 8, 13};

        for (int index : indexes)
        {
            if (!Validator::isValidNumber(fields[index]))
            {
                throw runtime_error("Sai dinh dang so");
            }
        }

        size_t p1 = 0;
        size_t p2 = 0;
        size_t p3 = 0;
        size_t p4 = 0;
        size_t p5 = 0;

        basePrice = stod(fields[5], &p1);
        discountAmount = stod(fields[6], &p2);
        paidAmount = stod(fields[7], &p3);
        pointsEarned = stoi(fields[8], &p4);
        refundAmount = stod(fields[13], &p5);

        if (p1 != fields[5].size() ||
            p2 != fields[6].size() ||
            p3 != fields[7].size() ||
            p4 != fields[8].size() ||
            p5 != fields[13].size())
        {
            throw runtime_error("Sai dinh dang so");
        }
    }
    catch (const exception&)
    {
        throw runtime_error("Gia tien hoac diem Ticket khong hop le");
    }

    Ticket* ticket = new Ticket(
        fields[0],
        fields[1],
        fields[2],
        fields[3],
        type,
        basePrice,
        discountAmount,
        paidAmount,
        pointsEarned,
        status,
        fields[10],
        fields[11],
        fields[12],
        refundAmount
    );

    if (!isValidTicket(ticket))
    {
        delete ticket;
        throw runtime_error("Du lieu Ticket khong hop le");
    }

    return ticket;
}

string TicketRepository::ticketToString(Ticket* ticket)
{
    if (!isValidTicket(ticket))
    {
        throw runtime_error("Ticket khong hop le khi ghi file");
    }

    string type = ticket->getTicketType() == TicketType::REGULAR
        ? "REGULAR" : "STUDENT";

    string status = ticket->getStatus() == TicketStatus::SOLD
        ? "SOLD" : "CANCELLED";

    return ticket->getTicketId() + "|" +
           ticket->getInvoiceId() + "|" +
           ticket->getShowtimeId() + "|" +
           ticket->getSeatId() + "|" +
           type + "|" +
           to_string(ticket->getBasePrice()) + "|" +
           to_string(ticket->getDiscountAmount()) + "|" +
           to_string(ticket->getPaidAmount()) + "|" +
           to_string(ticket->getPointsEarned()) + "|" +
           status + "|" +
           ticket->getCancelledAt() + "|" +
           ticket->getCancelledBy() + "|" +
           ticket->getCancelReason() + "|" +
           to_string(ticket->getRefundAmount());
}

TicketRepository::~TicketRepository()
{
    unlinkAll();
}

bool TicketRepository::isValidReference(const Ticket* ticket)
{
    if (!showtimeRepo.isLoaded() ||
        !seatRepo.isLoaded() ||
        !invoiceRepo.isLoaded())
    {
        return false;
    }

    Showtime* showtime = showtimeRepo.findById(ticket->getShowtimeId());
    Invoice* invoice = invoiceRepo.findById(ticket->getInvoiceId());

    if (showtime == nullptr || invoice == nullptr)
    {
        return false;
    }

    Seat* seat = seatRepo.findById(showtime->getRoomId() + "|" +
                                   ticket->getSeatId());
    if (seat == nullptr ||
        invoice->getShowtimeId() != ticket->getShowtimeId())
    {
        return false;
    }

    if (ticket->getPointsEarned() > 0 && invoice->getMember() == nullptr)
    {
        return false;
    }

    if (ticket->getStatus() == TicketStatus::CANCELLED)
    {
        if (!DateTime::isBefore(ticket->getCancelledAt(),
                                showtime->getStartAt()) ||
            DateTime::isBefore(ticket->getCancelledAt(),
                               invoice->getCreatedAt()))
        {
            return false;
        }
    }

    return true;
}

bool TicketRepository::isSeatSold(const string& showtimeId,
                                  const string& seatId)
{
    for (int i = 0; i < repo.count(); i++)
    {
        Ticket* ticket = repo.getAt(i);

        if (ticket->getShowtimeId() == showtimeId &&
            ticket->getSeatId() == seatId &&
            ticket->getStatus() == TicketStatus::SOLD)
        {
            return true;
        }
    }

    return false;
}

void TicketRepository::unlinkAll()
{
    for (int i = 0; i < repo.count(); i++)
    {
        Ticket* ticket = repo.getAt(i);

        if (ticket != nullptr)
        {
            ticket->unlinkReferences();
        }
    }
}

bool TicketRepository::load()
{
    if (!showtimeRepo.isLoaded() ||
        !seatRepo.isLoaded() ||
        !invoiceRepo.isLoaded())
    {
        return false;
    }

    if (!repo.load())
    {
        return false;
    }

    try
    {
        // Kiem tra toan bo du lieu truoc khi lien ket
        for (int i = 0; i < repo.count(); i++)
        {
            Ticket* ticket = repo.getAt(i);

            if (!isValidReference(ticket))
            {
                throw runtime_error(
                    "Ticket co tham chieu khong hop le"
                );
            }

            if (ticket->getStatus() != TicketStatus::SOLD)
            {
                continue;
            }

            for (int j = 0; j < i; j++)
            {
                Ticket* other = repo.getAt(j);

                if (other->getStatus() == TicketStatus::SOLD &&
                    other->getShowtimeId() == ticket->getShowtimeId() &&
                    other->getSeatId() == ticket->getSeatId())
                {
                    throw runtime_error("Trung ghe da ban");
                }
            }
        }

        // Lien ket sau khi tat ca ban ghi hop le
        for (int i = 0; i < repo.count(); i++)
        {
            Ticket* ticket = repo.getAt(i);

            Showtime* showtime = showtimeRepo.findById(ticket->getShowtimeId());
            Seat* seat = seatRepo.findById(showtime->getRoomId() + "|" +
                                           ticket->getSeatId());
            Invoice* invoice = invoiceRepo.findById(ticket->getInvoiceId());

            if (!ticket->linkShowtime(showtime) ||
                !ticket->linkSeat(seat) ||
                !ticket->linkInvoice(invoice))
            {
                throw runtime_error(
                    "Khong the lien ket Ticket"
                );
            }
        }
    }
    catch (...)
    {
        unlinkAll();
        repo.clear();
        throw;
    }

    return true;
}

bool TicketRepository::save()
{
    return repo.save();
}

bool TicketRepository::add(Ticket* ticket)
{
    if (!repo.isLoaded() || !isValidTicket(ticket))
    {
        return false;
    }

    if (repo.findById(ticket->getTicketId()) != nullptr)
    {
        return false;
    }

    for (int i = 0; i < repo.count(); i++)
    {
        if (repo.getAt(i) == ticket)
        {
            return false;
        }
    }

    if (ticket->getShowtime() != nullptr ||
        ticket->getSeat() != nullptr ||
        ticket->getInvoice() != nullptr)
    {
        return false;
    }
    if (!isValidReference(ticket))
    {
        return false;
    }

    if (ticket->getStatus() == TicketStatus::SOLD &&
        isSeatSold(ticket->getShowtimeId(), ticket->getSeatId()))
    {
        return false;
    }

    Showtime* showtime = showtimeRepo.findById(ticket->getShowtimeId());
    Seat* seat = seatRepo.findById(showtime->getRoomId() + "|" +
                                   ticket->getSeatId());
    Invoice* invoice = invoiceRepo.findById(ticket->getInvoiceId());

    try
    {
        if (!ticket->linkShowtime(showtime) ||
            !ticket->linkSeat(seat) ||
            !ticket->linkInvoice(invoice))
        {
            ticket->unlinkReferences();
            return false;
        }

        if (!repo.add(ticket))
        {
            ticket->unlinkReferences();
            return false;
        }
    }
    catch (...)
    {
        ticket->unlinkReferences();
        throw;
    }

    return true;
}

bool TicketRepository::update(const Ticket& newValues)
{
    if (!repo.isLoaded() || !isValidTicket(&newValues))
    {
        return false;
    }

    Ticket* ticket = repo.findById(newValues.getTicketId());

    if (ticket == nullptr ||
        ticket->getStatus() != TicketStatus::SOLD ||
        newValues.getStatus() != TicketStatus::CANCELLED)
    {
        return false;
    }

    // Chi duoc thay doi thong tin huy ve
    if (ticket->getInvoiceId() != newValues.getInvoiceId() ||
        ticket->getShowtimeId() != newValues.getShowtimeId() ||
        ticket->getSeatId() != newValues.getSeatId() ||
        ticket->getTicketType() != newValues.getTicketType() ||
        ticket->getBasePrice() != newValues.getBasePrice() ||
        ticket->getDiscountAmount() != newValues.getDiscountAmount() ||
        ticket->getPaidAmount() != newValues.getPaidAmount() ||
        ticket->getPointsEarned() != newValues.getPointsEarned())
    {
        return false;
    }

    if (!isValidReference(&newValues))
    {
        return false;
    }

    ticket->markCancelled(
        newValues.getCancelledAt(),
        newValues.getCancelledBy(),
        newValues.getCancelReason()
    );

    return true;
}

bool TicketRepository::removeById(const string& ticketId)
{
    if (!repo.isLoaded())
    {
        return false;
    }

    Ticket* ticket = repo.findById(ticketId);

    if (ticket == nullptr)
    {
        return false;
    }

    ticket->unlinkReferences();

    return repo.removeById(ticketId);
}

Ticket* TicketRepository::findById(const string& ticketId)
{
    return repo.findById(ticketId);
}

Ticket* TicketRepository::getAt(int index)
{
    return repo.getAt(index);
}

int TicketRepository::count()
{
    return repo.count();
}

bool TicketRepository::isLoaded()
{
    return repo.isLoaded();
}
```
## 6. Giải thích thư viện và từ khóa

### 6.1. Các thư viện và header

| Thành phần | Vai trò |
|---|---|
| `IRepository.h` | Interface chung, định nghĩa 9 hàm mà TicketRepository phải triển khai. |
| `Repository.h` | Quản lý danh sách Ticket, quyền sở hữu con trỏ, đọc/ghi TXT và kiểm tra khóa. |
| `Ticket.h` | Model Ticket, trạng thái vé và các phương thức liên kết. |
| `Showtime.h` | Truy xuất suất chiếu, phòng chiếu và thời gian bắt đầu. |
| `Seat.h` | Kiểm tra Seat thuộc Room của Showtime. |
| `Invoice.h` | Kiểm tra hóa đơn và liên kết Ticket với Invoice. |
| `Validator.h` | Kiểm tra ID, chuỗi, ngày giờ và định dạng số. |
| `DateTime.h` | So sánh thời điểm hủy vé với thời điểm tạo hóa đơn và bắt đầu suất chiếu. |
| `<string>` | `string`, `find`, `substr`, `to_string`, `stod`, `stoi`. |
| `<vector>` | Lưu các trường được tách từ dòng TXT. |
| `<cmath>` | `isfinite()` và `fabs()` để kiểm tra số thực và sai số tiền. |
| `<stdexcept>` | `runtime_error` để thông báo lỗi dữ liệu hoặc liên kết. |

### 6.2. `static` và callback

`getTicketId()`, `stringToTicket()`, `ticketToString()` và `isValidTicket()` là các hàm `static` vì không cần truy cập thuộc tính của đối tượng `TicketRepository`.

Ba hàm đầu được truyền vào `Repository<Ticket>` dưới dạng con trỏ hàm:

- `getTicketId()`: lấy khóa chính.
- `stringToTicket()`: chuyển dòng TXT thành Ticket.
- `ticketToString()`: chuyển Ticket thành dòng TXT.

`isValidTicket()` được các hàm khác gọi để kiểm tra dữ liệu thuộc tính của Ticket.

### 6.3. `override`, kế thừa và composition

`TicketRepository : public IRepository<Ticket>` triển khai hợp đồng chung của tầng Data Access.

`override` giúp trình biên dịch kiểm tra chữ ký hàm có khớp với interface hay không.

`Repository<Ticket> repo` là composition, không phải kế thừa. Thành viên này chịu trách nhiệm quản lý danh sách và bộ nhớ của Ticket.

Ba tham chiếu `IRepository<Showtime>&`, `IRepository<Seat>&` và `IRepository<Invoice>&` được truyền vào constructor theo cơ chế dependency injection.

TicketRepository không sở hữu các dependency này.

### 6.4. `const`, tham chiếu và con trỏ

- `const string&`: truyền chuỗi mà không cần sao chép.
- `const Ticket*`: đọc dữ liệu Ticket mà không sửa đối tượng.
- `const Ticket&`: nhận dữ liệu cập nhật mà không sao chép toàn bộ Ticket.
- `Ticket*`: truy cập đối tượng Ticket đang được Repository quản lý.
- `IRepository<T>&`: tham chiếu đến dependency thông qua interface.

Các con trỏ đến Showtime, Seat và Invoice là con trỏ tham chiếu, không chuyển quyền sở hữu.

### 6.5. `try`, `catch`, `throw`

`throw runtime_error(...)` được sử dụng khi gặp dữ liệu TXT hoặc liên kết bất thường.

`catch (...)` bắt mọi ngoại lệ để dọn các liên kết đã tạo, sau đó `throw;` ném lại ngoại lệ gốc.

Trong `load()`, nếu xảy ra lỗi sau khi dữ liệu đã được nạp, cần gọi `unlinkAll()` trước `repo.clear()`.

Trong `add()`, nếu liên kết hoặc thêm dữ liệu thất bại, phải gọi `ticket->unlinkReferences()` để khôi phục trạng thái trước khi thêm.

Giá trị `false` được dùng cho các điều kiện từ chối thông thường, chẳng hạn Ticket không hợp lệ, ID trùng hoặc ghế đã được bán.

### 6.6. `isfinite()` và `fabs()`

`isfinite()` thuộc thư viện `<cmath>`, dùng để kiểm tra một số thực có phải giá trị hữu hạn hay không.

Hàm từ chối các giá trị như NaN hoặc vô cực.

`fabs()` trả về giá trị tuyệt đối của một số thực.

Ví dụ:

```cpp
fabs(paid - (base - discount)) > 0.000001
```

Điều kiện này kiểm tra sai lệch giữa số tiền thực trả và kết quả tính từ giá gốc trừ giảm giá.

Do `double` có thể xuất hiện sai số biểu diễn, thiết kế sử dụng ngưỡng `0.000001` thay vì yêu cầu hai kết quả phải bằng nhau tuyệt đối.

---

## 7. Giải thích các hàm private

Phần này giải thích **mục đích, thứ tự thực hiện, điều kiện trả về và lý do thiết kế** của từng hàm. Mã nguồn đầy đủ tương ứng nằm ở mục 5.

### 7.1. `isValidTicket(const Ticket* ticket)`

**Mục đích:** Kiểm tra các thuộc tính nội tại của Ticket mà không truy cập Repository khác.

**Thứ tự kiểm tra:**

1. Con trỏ Ticket không được `nullptr`.
2. `ticketId`, `invoiceId`, `showtimeId` phải có tiền tố lần lượt `TKT`, `INV`, `SHW`.
3. `seatId` phải là chuỗi hợp lệ.
4. `ticketType` chỉ được là `REGULAR` hoặc `STUDENT`.
5. `status` chỉ được là `SOLD` hoặc `CANCELLED`.
6. `basePrice`, `discountAmount`, `paidAmount`, `refundAmount` phải hữu hạn và không âm.
7. `pointsEarned` không được âm.
8. `discountAmount` không được lớn hơn `basePrice`.
9. `paidAmount` phải bằng `basePrice - discountAmount` trong giới hạn sai số `0.000001`.

**Nếu Ticket đang SOLD:**

- `cancelledAt` phải rỗng.
- `cancelledBy` phải rỗng.
- `cancelReason` phải rỗng.
- `refundAmount` phải bằng 0.

**Nếu Ticket đang CANCELLED:**

- `cancelledAt` phải có định dạng ngày giờ hợp lệ.
- `cancelledBy` phải có tiền tố `USR`.
- `cancelReason` phải là chuỗi hợp lệ.
- `refundAmount` phải bằng `paidAmount` trong giới hạn sai số.

**Kết quả:** Trả `false` khi bất kỳ điều kiện nào không đạt; chỉ trả `true` khi toàn bộ thuộc tính hợp lệ.

**Vì sao tách hàm?**

Quy tắc kiểm tra thuộc tính được sử dụng chung cho parser, serializer, `add()` và `update()`.

Không kiểm tra sự tồn tại của Showtime, Seat hoặc Invoice ở đây để tránh trộn validation thuộc tính với validation tham chiếu.

**Luồng hoạt động chi tiết:**

```text
Ticket null? -- có --> false
        |
ID/seatId sai? -- có --> false
        |
Loại vé sai? -- có --> false
        |
Status sai? -- có --> false
        |
Tiền/điểm/quan hệ giá sai? -- có --> false
        |
SOLD? -- có --> trả kết quả kiểm tra trường hủy rỗng, refund == 0
        |
       không (CANCELLED)
        v
trả kết quả DateTime, cancelledBy, lý do, refund == paid trong sai số
```

**Giải thích từng đoạn code:** Kiểm tra `nullptr` đứng trước mọi `->`. `Validator::isValidId()` xác minh ba mã, còn `isValidText()` kiểm tra `seatId`; các toán tử `||` trả `false` nếu một điều kiện sai. Hàm chỉ chấp nhận hai giá trị `TicketType`, sau đó lưu `status` để kiểm tra enum một lần và chọn nhánh SOLD/CANCELLED. Bốn biến `base`, `discount`, `paid`, `refund` giúp kiểm tra `isfinite`, dấu không âm và quan hệ giá; `fabs` cho phép sai số `0.000001`. Nhánh SOLD trả trực tiếp kết quả của bốn điều kiện, không đi vào nhánh cuối. Nhánh CANCELLED dùng `Validator` cho thời gian, ID người hủy, lý do và kiểm tra tiền hoàn; `&&` làm cả nhóm sai khi một ràng buộc không đạt. Hàm không đổi trạng thái Ticket.

### 7.2. `getTicketId(Ticket* ticket)`

**Mục đích:** Cung cấp khóa chính cho `Repository<Ticket>` thông qua callback.

**Quy trình:**

1. Nếu `ticket == nullptr`, trả chuỗi rỗng để tránh truy cập con trỏ không hợp lệ.
2. Nếu hợp lệ, trả `ticket->getTicketId()`.

**Vì sao không kiểm tra Validator ở đây?**

Callback chỉ có nhiệm vụ lấy khóa. Việc kiểm tra định dạng ID thuộc `isValidTicket()`, còn kiểm tra khóa trùng hoặc rỗng trong danh sách thuộc `Repository<T>`.

**Luồng hoạt động chi tiết:**

```text
ticket == nullptr? -- có --> trả ""
        |
       không
        v
trả ticket->getTicketId()
```

**Giải thích từng đoạn code:** Nhánh `if` chặn dereference con trỏ null. Nhánh cuối trả bản sao `string` của ID cho callback; hàm không kiểm tra định dạng hoặc thay đổi quyền sở hữu Ticket.

### 7.3. `stringToTicket(const string& line)`

**Mục đích:** Chuyển một dòng TXT thành đối tượng `Ticket*`. Đây là callback được `Repository<Ticket>::load()` gọi.

**Quy trình chi tiết:**

1. Dùng `find('|', start)` để tìm từng dấu phân cách, `substr()` lấy nội dung từng trường và đưa vào `vector<string> fields`.
2. Kiểm tra đúng **14 trường**. Nếu thiếu hoặc thừa, ném `runtime_error`.
3. Chuyển trường `ticketType` thành `TicketType::REGULAR` hoặc `TicketType::STUDENT`.
4. Chuyển trường `status` thành `TicketStatus::SOLD` hoặc `TicketStatus::CANCELLED`.
5. Kiểm tra định dạng năm trường số bằng `Validator::isValidNumber()`.
6. Dùng `stod()` để chuyển các trường tiền thành `double`, `stoi()` để chuyển điểm thành `int`.
7. Kiểm tra vị trí kết thúc chuyển đổi để bảo đảm không còn ký tự thừa.
8. Tạo đối tượng `new Ticket(...)` từ 14 giá trị.
9. Gọi `isValidTicket()` một lần để kiểm tra thuộc tính.
10. Nếu không hợp lệ, `delete ticket` rồi ném ngoại lệ; nếu hợp lệ, trả con trỏ.

**Vì sao sử dụng `try/catch` khi chuyển số?**

`stod()` và `stoi()` có thể ném ngoại lệ khi chuỗi không chuyển đổi được hoặc giá trị vượt phạm vi cho phép.

Hàm chuyển các lỗi này thành thông báo thống nhất về dữ liệu Ticket không hợp lệ.

**Ai sở hữu đối tượng?**

Sau khi callback trả thành công, `Repository<Ticket>` tiếp nhận đối tượng trong quá trình load.

Nếu parse thất bại sau khi cấp phát bằng `new`, callback tự giải phóng đối tượng.

**Không làm ở đây:** Không tìm Showtime, Seat hoặc Invoice. Việc kiểm tra tham chiếu được thực hiện sau khi toàn bộ dữ liệu TXT đã được nạp.

**Luồng hoạt động chi tiết:**

```text
Tách line thành fields bằng find/substr
        |
Đủ 14 trường? -- không --> runtime_error
        |
Đổi type rồi status -- giá trị lạ --> runtime_error
        |
Kiểm tra 5 chuỗi số, stod/stoi, vị trí parse
        |                         |
        |                    exception --> runtime_error thống nhất
        v
new Ticket(14 giá trị)
        |
isValidTicket()? -- không --> delete ticket, runtime_error
        |
       có
        v
trả Ticket*
```

**Giải thích từng đoạn code:** `fields` giữ đúng cả trường rỗng: `start` đánh dấu đầu trường, `find('|', start)` tìm dấu kế tiếp; khi không còn dấu, `substr(start)` lấy phần cuối rồi `break`. `fields.size()` chặn thiếu hoặc thừa trường trước mọi truy cập `fields[index]`. Hai chuỗi enum được ánh xạ bằng nhánh `if/else if`; giá trị khác ném lỗi riêng. Năm biến số nhận các trường 5, 6, 7, 8, 13. `indexes` duyệt đúng năm chuỗi để Validator kiểm tra trước khi chuyển đổi; `p1` đến `p5` nhận vị trí kết thúc của `stod/stoi`, so với kích thước chuỗi để loại ký tự thừa. `catch (const exception&)` cũng bắt `runtime_error` do chính nhánh định dạng ném và đổi thành thông báo số Ticket không hợp lệ. Sau `new`, chỉ callback còn giữ quyền sở hữu cho đến khi trả con trỏ; nhánh validation sai phải `delete`, còn nhánh đúng giao con trỏ cho `Repository<Ticket>::load()`.

### 7.4. `ticketToString(Ticket* ticket)`

**Mục đích:** Chuyển Ticket thành một dòng TXT đúng schema để `Repository<Ticket>::save()` ghi ra file.

**Quy trình:**

1. Gọi `isValidTicket(ticket)`; nếu không hợp lệ, ném `runtime_error`.
2. Chuyển `TicketType` thành chuỗi `REGULAR` hoặc `STUDENT`.
3. Chuyển `TicketStatus` thành chuỗi `SOLD` hoặc `CANCELLED`.
4. Ghép 14 trường theo đúng thứ tự schema, phân cách bằng `|`.
5. Dùng `to_string()` chuyển các giá trị tiền và điểm thành chuỗi.
6. Trả chuỗi kết quả; không tự mở hoặc ghi file.

**Vì sao kiểm tra dữ liệu trước khi save?**

Dữ liệu Ticket trong RAM có thể đã thay đổi sau khi được thêm hoặc nạp. Serializer cần từ chối ghi dữ liệu thuộc tính không hợp lệ.

**Lưu ý:** Hàm không tính lại giá vé theo PricingConfig vì giá đã bán là dữ liệu lịch sử.

**Luồng hoạt động chi tiết:**

```text
isValidTicket(ticket)? -- không --> runtime_error
        |
Tạo chuỗi type và status từ enum hợp lệ
        |
Ghép 14 trường theo schema, to_string cho số
        |
Trả dòng TXT; Repository chung chịu trách nhiệm ghi
```

**Giải thích từng đoạn code:** `isValidTicket()` chặn con trỏ null và thuộc tính sai trước mọi getter. Hai biểu thức điều kiện `?:` chọn đúng nhãn enum vì enum đã được kiểm tra. Chuỗi `return` ghép lần lượt bốn ID, loại vé, bốn giá trị số, trạng thái và bốn trường hủy; `to_string()` chuyển các số nhưng có giới hạn độ chính xác của `double`. Hàm không kiểm tra lại tham chiếu và không tự ghi file.

### 7.5. `isValidReference(const Ticket* ticket)`

**Mục đích:** Kiểm tra các quan hệ mà Ticket khai báo bằng ID.

**Tiền điều kiện:** Caller đã xác nhận `isValidTicket(ticket) == true`. Hàm không gọi lại `isValidTicket()`.

**Thứ tự kiểm tra:**

1. `showtimeRepo`, `seatRepo` và `invoiceRepo` đều đã load.
2. Tìm Showtime theo `showtimeId` và Invoice theo `invoiceId`; cả hai phải tồn tại.
3. Sau khi xác nhận Showtime và Invoice tồn tại, tìm Seat bằng `seatRepo.findById(showtime->getRoomId() + "|" + ticket->getSeatId())`; khóa ghép bảo đảm Seat thuộc đúng Room của Showtime.
4. Kiểm tra `invoice->getShowtimeId()` phải bằng `ticket->getShowtimeId()`.
5. Nếu `pointsEarned > 0`, Invoice đã tìm được phải có con trỏ Member liên kết khác null.
6. Nếu Ticket đang `CANCELLED`, kiểm tra thời gian hủy phải trước thời điểm bắt đầu Showtime.
7. Kiểm tra thời gian hủy không được trước thời điểm tạo Invoice.

**Kết quả:** Trả `true` nếu toàn bộ tham chiếu hợp lệ; ngược lại trả `false`.

**Luồng hoạt động:**

```text
Dependency đã load? -- không --> false
        |
       có
        v
Tìm Showtime và Invoice -- thiếu --> false
        |
        v
Tra Seat bằng khóa roomId|seatId -- thiếu --> false
        |
        v
Invoice cùng Showtime? -- không --> false
        |
        v
Điểm > 0 và Invoice không có Member? -- có --> false
        |
        v
Vé CANCELLED? -- có --> kiểm tra mốc thời gian -- sai --> false
        |
        v
       true
```

**Phân chia trách nhiệm:**

- `isValidTicket()` kiểm tra định dạng và thuộc tính.
- `isValidReference()` kiểm tra sự tồn tại và quan hệ giữa các đối tượng.
- Các hàm `link...()` của Model thiết lập liên kết thực tế.
- Business Service kiểm tra quyền và điều kiện nghiệp vụ của thao tác bán/hủy vé.

**Giải thích từng đoạn code:** Ba `isLoaded()` bảo vệ các lời gọi qua dependency; hàm được tái dùng từ `load()`, `add()` và `update()`, nên không dựa vào một caller duy nhất. Hai `findById()` lấy Showtime và Invoice; phải kiểm tra cả hai khác null trước khi dùng `showtime->getRoomId()`. `seatRepo.findById()` nhận đúng khóa ghép mà SeatRepository quản lý. Điều kiện tiếp theo loại Seat thiếu hoặc Invoice thuộc suất khác. Nếu Ticket có điểm dương, `invoice->getMember()` phải khác null; đây là Member đã được InvoiceRepository kiểm tra và liên kết, nên không cần tra cứu lại Invoice hoặc Member. Ticket có điểm bằng 0 vẫn hợp lệ dù Invoice có hoặc không có Member. Chỉ khi status là CANCELLED, `DateTime::isBefore(cancelledAt, startAt)` phải đúng và `DateTime::isBefore(cancelledAt, createdAt)` phải sai, cho phép thời điểm hủy bằng thời điểm lập hóa đơn. Nhánh SOLD bỏ qua kiểm tra thời gian hủy vì `isValidTicket()` đã yêu cầu các trường hủy rỗng. Mọi nhánh từ chối trả `false`; cuối hàm trả `true`, không tạo liên kết.

### 7.6. `isSeatSold(const string& showtimeId, const string& seatId)`

**Mục đích:** Kiểm tra một ghế đã có Ticket đang bán cho suất chiếu tương ứng hay chưa.

**Quy trình:**

1. Duyệt toàn bộ Ticket đang được quản lý.
2. So sánh `showtimeId` và `seatId`.
3. Nếu cả hai trùng và Ticket có trạng thái `SOLD`, trả `true`.
4. Nếu duyệt hết mà không tìm thấy, trả `false`.

**Vì sao chỉ xét Ticket SOLD?**

Ticket `CANCELLED` phải được giữ lại để bảo toàn lịch sử, nhưng không tiếp tục chiếm ghế.

Vì vậy, nhiều Ticket lịch sử có thể cùng `(showtimeId, seatId)`, miễn là chỉ có tối đa một Ticket đang `SOLD`.

**Nơi sử dụng:** `add()` kiểm tra ghế trước khi thêm Ticket mới.

**Tiền điều kiện:** `add()` đã kiểm tra `repo.isLoaded()` trước khi gọi hàm này; helper không lặp lại kiểm tra loaded.

**Luồng hoạt động chi tiết:**

```text
Duyệt i = 0 .. repo.count()-1
        |
Ticket cùng showtimeId, seatId và SOLD? -- có --> true
        |
       không: duyệt tiếp
        v
Hết danh sách --> false
```

**Giải thích từng đoạn code:** `repo.getAt(i)` lấy từng con trỏ do Repository đang sở hữu. Toán tử `&&` yêu cầu trùng cả mã suất, mã ghế và trạng thái SOLD; vé CANCELLED không chiếm ghế. Hàm dừng ngay ở vé phù hợp, còn khi vòng `for` kết thúc trả `false`. Không tự kiểm tra loaded hoặc validation Ticket lần nữa.

### 7.7. `unlinkAll()`

**Mục đích:** Tháo các liên kết của toàn bộ Ticket trước khi giải phóng dữ liệu.

**Quy trình:**

1. Duyệt từ vị trí `0` đến `repo.count() - 1`.
2. Lấy `Ticket*` bằng `repo.getAt(i)`.
3. Nếu con trỏ khác null, gọi `ticket->unlinkReferences()`.

**Nơi sử dụng:**

- Destructor của TicketRepository.
- Nhánh xử lý lỗi của `load()`.

**Vì sao phải unlink trước khi clear?**

Invoice lưu danh sách con trỏ Ticket. Nếu Ticket bị xóa trước khi tháo liên kết, Invoice có thể giữ con trỏ đến vùng nhớ đã được giải phóng.

`unlinkAll()` chỉ tháo liên kết, không xóa Ticket. Việc giải phóng bộ nhớ thuộc `Repository<Ticket>`.

**Luồng hoạt động chi tiết:**

```text
Duyệt i = 0 .. repo.count()-1
        |
ticket = repo.getAt(i)
        |
ticket khác null? -- có --> ticket->unlinkReferences()
        |
       duyệt tiếp đến hết, rồi trả void
```

**Giải thích từng đoạn code:** Vòng `for` dùng `repo.count()` hiện tại và `repo.getAt(i)` để truy cập từng Ticket. Nhánh null tránh dereference; `Ticket::unlinkReferences()` gỡ Ticket khỏi danh sách của Invoice rồi xóa con trỏ Showtime/Seat trong Ticket. Hàm không gọi `repo.clear()` hoặc `delete`; caller của `load()` dọn bộ nhớ sau khi gỡ, còn destructor để thành viên `repo` tự hủy. Code không có `catch` ở đây, nên ngoại lệ nếu phát sinh sẽ truyền lên caller.

---

## 8. Giải thích các hàm public

### 8.1. Constructor

**Mục đích:** Nhận đường dẫn TXT và ba dependency bằng interface.

**Quy trình:**

1. Nhận `filePath`.
2. Nhận tham chiếu `IRepository<Showtime>&`.
3. Nhận tham chiếu `IRepository<Seat>&`.
4. Nhận tham chiếu `IRepository<Invoice>&`.
5. Khởi tạo `repo(filePath, getTicketId, stringToTicket, ticketToString)`.
6. Lưu các tham chiếu dependency.

**Ý nghĩa:** `Repository<Ticket>` phụ trách lưu trữ chung; TicketRepository phụ trách các quy tắc riêng của Ticket.

Dependency injection giúp TicketRepository không phụ thuộc trực tiếp vào ShowtimeRepository, SeatRepository hoặc InvoiceRepository.

**Luồng hoạt động chi tiết:**

```text
Nhận filePath và ba IRepository<T>&
        |
Khởi tạo repo với đường dẫn và ba callback
        |
Gắn showtimeRepo, seatRepo, invoiceRepo
        |
Thân constructor rỗng
```

**Giải thích từng đoạn code:** Initializer list khởi tạo `repo` trước, rồi ba thành viên tham chiếu theo thứ tự khai báo trong header. Các callback static cung cấp khóa, parser và serializer cho `Repository<Ticket>`. Constructor không gọi `load()` và không nhận quyền sở hữu ba dependency.

### 8.2. Destructor

**Mục đích:** Tháo liên kết trước khi thành viên `repo` bị hủy và giải phóng các Ticket.

**Quy trình:** Gọi `unlinkAll()`. Sau khi destructor body kết thúc, các thành viên bị hủy theo cơ chế C++.

**Điều kiện vòng đời:** InvoiceRepository, ShowtimeRepository và SeatRepository phải còn tồn tại trong quá trình TicketRepository tháo liên kết.

Đặc biệt, TicketRepository phải được hủy trước InvoiceRepository để tránh Invoice giữ con trỏ Ticket không hợp lệ.

**Luồng hoạt động chi tiết:**

```text
~TicketRepository() --> unlinkAll() --> hết thân destructor
                                     --> Repository<Ticket> tự hủy các Ticket
```

**Giải thích từng đoạn code:** Thân destructor chỉ gọi `unlinkAll()`. Sau khi thân kết thúc, C++ hủy các thành viên; `repo` giải phóng các Ticket mà nó sở hữu. Không có nhánh trả về hoặc `catch` trong destructor dự kiến, nên việc gỡ liên kết cần chạy khi các dependency còn sống.

### 8.3. `load()`

**Mục đích:** Nạp Ticket từ TXT, kiểm tra dữ liệu và thiết lập các quan hệ với Showtime, Seat và Invoice.

**Thứ tự thực hiện:**

1. Kiểm tra ba dependency đã load; nếu không đạt, trả `false`. `repo.load()` tự xử lý trường hợp Repository đã load.
2. Gọi `repo.load()` để đọc TXT, parse Ticket, kiểm tra khóa trùng và quản lý bộ nhớ.
3. **Vòng 1:** Duyệt mọi Ticket, gọi `isValidReference()` để kiểm tra tham chiếu, quan hệ điểm–Member và thời gian hủy khi cần.
4. Với Ticket `SOLD`, so sánh với các Ticket trước đó để phát hiện trùng `(showtimeId, seatId)` khi cả hai cùng `SOLD`.
5. **Vòng 2:** Lấy Showtime và Invoice tương ứng; tra Seat bằng khóa ghép `showtime->getRoomId() + "|" + ticket->getSeatId()`.
6. Lần lượt gọi `linkShowtime()`, `linkSeat()` và `linkInvoice()`.
7. Nếu bất kỳ liên kết nào thất bại, ném `runtime_error`.
8. Nếu có lỗi trong hai vòng, gọi `unlinkAll()`, `repo.clear()` và ném lại ngoại lệ.

**Vì sao chia hai vòng?**

Không bắt đầu tạo liên kết khi còn một Ticket có tham chiếu sai hoặc vi phạm quy tắc trùng ghế SOLD.

Nhờ đó giảm số liên kết cần hoàn tác khi dữ liệu TXT không hợp lệ.

**Luồng hoạt động:**

```text
Dependency đã load? -- không --> false
        |
        v
repo.load() -- false --> false
       |-- ném ngoại lệ parse --> truyền lên (chưa vào try bên dưới)
        |
        v
Vòng 1: tham chiếu và trùng ghế SOLD -- lỗi --> catch
        |
        v
Vòng 2: tra Seat bằng khóa ghép, link ba đối tượng -- lỗi --> catch
        |                                                     |
        v                                                     v
       true                                      unlinkAll(), repo.clear(), ném lại
```

**Vì sao phải kiểm tra trùng ghế?**

`Repository<Ticket>` chỉ kiểm tra trùng `ticketId`, không kiểm tra hai Ticket khác ID nhưng cùng chiếm một ghế.

Đây là quy tắc dữ liệu riêng của TicketRepository.

**Tránh kiểm tra lặp:**

- Parser đã gọi `isValidTicket()`.
- `repo.load()` đã kiểm tra khóa trùng.
- Vòng 1 chỉ kiểm tra tham chiếu và xung đột ghế SOLD.
- Vòng 2 chỉ thiết lập liên kết.

**Giới hạn:** Hàm không đối chiếu tổng tiền các Ticket với `Invoice::totalAmount` và không tạo transaction giữa các file TXT.

**Giải thích từng đoạn code:** Điều kiện đầu kiểm tra ba dependency theo thứ tự Showtime, Seat, Invoice và trả `false` trước khi đụng vào `repo` nếu một nơi chưa load. `repo.load()` tự xử lý file không đọc được, trạng thái đã load, parse, ID rỗng/trùng và quyền sở hữu dữ liệu; `false` được chuyển tiếp, còn ngoại lệ ở bước này truyền ra ngoài vì `try` bắt đầu sau đó. Trong `try`, vòng thứ nhất dùng `isValidReference(ticket)` cho từng bản ghi; `runtime_error` báo tham chiếu sai. Vé khác SOLD đi qua `continue`, còn vé SOLD so với các vị trí `j < i` để phát hiện cùng suất và ghế đang SOLD. Vòng thứ hai chỉ chạy sau khi vòng một hoàn tất: lấy Showtime, tra Seat qua khóa ghép và lấy Invoice, rồi gọi ba `link...()` theo thứ tự; điều kiện `||` dừng ở liên kết đầu tiên thất bại và ném lỗi. `catch (...)` chỉ xử lý lỗi trong hai vòng: `unlinkAll()` gỡ các liên kết đã tạo, `repo.clear()` giải phóng Ticket đã nạp và đặt trạng thái chưa load, `throw;` giữ ngoại lệ gốc. Nhánh cuối trả `true` khi toàn bộ hai vòng thành công.

### 8.4. `save()`

**Mục đích:** Ghi danh sách Ticket đang quản lý xuống TXT.

**Quy trình:** Gọi `repo.save()` và trả trực tiếp kết quả.

**Vì sao đơn giản?**

`Repository<Ticket>` đã chịu trách nhiệm serialize và ghi file.

TicketRepository không còn `pendingIds`, nên không cần xử lý danh sách ID sau khi lưu.

**Lưu ý:** `save()` của Ticket không đồng nghĩa Invoice hoặc Member đã được lưu thành công. Các file TXT không có transaction nguyên tử chung.

**Luồng hoạt động chi tiết:**

```text
save() --> repo.save() --> trả bool hoặc chuyển tiếp ngoại lệ
```

**Giải thích từng đoạn code:** `return repo.save()` trả nguyên kết quả ghi file của Repository chung. `ticketToString()` được gọi từ tầng chung khi serialize và có thể ném lỗi dữ liệu; hàm này không bắt ngoại lệ, không tự thay đổi RAM hoặc quản lý danh sách pending.

### 8.5. `add(Ticket* ticket)`

**Mục đích:** Thêm Ticket mới và tạo liên kết hợp lệ với Showtime, Seat và Invoice.

**Thứ tự kiểm tra và thực hiện:**

1. `repo.isLoaded()` phải true; gọi `isValidTicket(ticket)` để kiểm tra dữ liệu và null.
2. Từ chối `ticketId` đã tồn tại.
3. Duyệt danh sách để từ chối con trỏ Ticket đã được quản lý.
4. Từ chối Ticket đã có liên kết Showtime, Seat hoặc Invoice.
5. Gọi `isValidReference()` để xác nhận các tham chiếu và quan hệ điểm–Member hợp lệ.
6. Nếu Ticket đang `SOLD`, gọi `isSeatSold()` để bảo đảm ghế chưa bị chiếm.
7. Tra Showtime và Invoice tương ứng; tra Seat bằng khóa ghép `showtime->getRoomId() + "|" + ticket->getSeatId()`.
8. Trong `try`, lần lượt gọi `linkShowtime()`, `linkSeat()`, `linkInvoice()`; nếu thành công mới gọi `repo.add(ticket)`.
9. Nếu liên kết hoặc thêm thất bại, gọi `ticket->unlinkReferences()` và trả `false`.
10. Nếu phát sinh ngoại lệ, tháo liên kết rồi ném lại.

**Vì sao kiểm tra trùng ID và con trỏ trước khi link?**

Nếu Ticket đã được Repository quản lý mà ta tiếp tục tạo hoặc tháo liên kết, trạng thái của đối tượng hiện có có thể bị thay đổi ngoài ý muốn.

Kiểm tra trước giúp tránh tác dụng phụ lên dữ liệu đang quản lý.

**Luồng hoạt động:**

```text
Loaded và Ticket hợp lệ? -- không --> false
        |
        v
ID/con trỏ trùng hoặc đã link? -- có --> false
        |
        v
Tham chiếu sai hoặc ghế SOLD đã bán? -- có --> false
        |
        v
Tra Showtime, Seat bằng khóa ghép, Invoice
        |
        v
try: link ba đối tượng -- thất bại --> unlinkReferences(), false
        |
        v
repo.add() -- thất bại --> unlinkReferences(), false
        |
        v
       true       (ngoại lệ trong try --> unlinkReferences(), ném lại)
```

**Vì sao vẫn kiểm tra trùng trong `Repository<T>::add()`?**

Đó là kiểm tra phòng vệ chung cho mọi loại Model.

TicketRepository kiểm tra sớm vì thao tác liên kết có tác dụng phụ, đặc biệt là `Invoice::linkTicket()` làm thay đổi danh sách Ticket của Invoice.

**Quyền sở hữu:**

- `repo.add()` thành công: Repository sở hữu Ticket.
- `add()` thất bại: Caller vẫn sở hữu Ticket.
- TicketRepository không được xóa con trỏ đầu vào khi từ chối thêm.

**Giải thích từng đoạn code:** Điều kiện đầu kiểm tra loaded trước, rồi `isValidTicket(ticket)`; toán tử `||` ngắn mạch nên không dereference khi đầu vào sai. `repo.findById()` loại ID đã có, vòng `for` loại đúng địa chỉ con trỏ đã được quản lý: hai kiểm tra này phải đứng trước liên kết vì nhánh lỗi phía sau gọi `unlinkReferences()`. Ba getter liên kết loại Ticket đã link trước khi kiểm tra tham chiếu. `isValidReference()` xác minh dependency, Showtime, Seat theo khóa ghép, Invoice và thời gian hủy nếu cần. Chỉ vé SOLD mới gọi `isSeatSold()`; vé CANCELLED lịch sử không chiếm ghế. Sau các tiền điều kiện, ba con trỏ Showtime/Seat/Invoice được tra lại để đưa vào `linkShowtime()`, `linkSeat()`, `linkInvoice()` theo thứ tự. Mỗi nhánh trả `false` trong `try` tháo liên kết đã tạo; `repo.add()` chỉ được gọi sau khi cả ba link thành công và chuyển quyền sở hữu khi trả `true`. `catch (...)` tháo liên kết rồi `throw;`, giữ ngoại lệ gốc; caller vẫn sở hữu con trỏ nếu việc thêm chưa thành công.

### 8.6. `update(const Ticket& newValues)`

**Mục đích:** Cập nhật Ticket từ trạng thái `SOLD` sang `CANCELLED` mà không làm thay đổi thông tin bán vé ban đầu.

**Thứ tự kiểm tra và thực hiện:**

1. Kiểm tra `repo.isLoaded()` và `isValidTicket(&newValues)`.
2. Tìm Ticket hiện tại theo `newValues.getTicketId()`.
3. Nếu không tồn tại, trả `false`.
4. Ticket hiện tại phải đang `SOLD`.
5. `newValues` phải có trạng thái `CANCELLED`.
6. So sánh các thuộc tính bất biến giữa Ticket hiện tại và `newValues`.
7. Gọi `isValidReference(&newValues)` để kiểm tra các tham chiếu, quan hệ điểm–Member và thời gian hủy.
8. Nếu tất cả điều kiện hợp lệ, gọi `ticket->markCancelled(...)`.
9. Trả `true`.

**Các thuộc tính bất biến:**

- `ticketId`
- `invoiceId`
- `showtimeId`
- `seatId`
- `ticketType`
- `basePrice`
- `discountAmount`
- `paidAmount`
- `pointsEarned`

**Vì sao không cho phép thay đổi các thuộc tính này?**

Chúng phản ánh giao dịch bán vé ban đầu. Việc hủy vé không được làm thay đổi thông tin lịch sử của giao dịch.

**Vì sao chỉ hỗ trợ SOLD → CANCELLED?**

Ticket đã hủy không được tự động khôi phục về trạng thái SOLD.

Đồng thời, Ticket đã CANCELLED không được cập nhật lại thông tin hủy bằng hàm này.

**Luồng hoạt động:**

```text
Loaded và newValues hợp lệ? -- không --> false
        |
        v
Tìm Ticket; SOLD -> CANCELLED? -- không --> false
        |
        v
Thuộc tính bán vé bất biến? -- không --> false
        |
        v
Tham chiếu và thời gian hủy hợp lệ? -- không --> false
        |
        v
markCancelled() --> true
```

**Vì sao sử dụng `markCancelled()`?**

Model Ticket đã có phương thức này để cập nhật đồng bộ:

- `cancelledAt`
- `cancelledBy`
- `cancelReason`
- `status`
- `refundAmount`

Do đó không cần bổ sung setter riêng cho từng thuộc tính.

**Lưu ý:** `markCancelled()` tự đặt `refundAmount = paidAmount`, nên `newValues` phải có dữ liệu hoàn tiền phù hợp với quy tắc validation.

**Phân chia trách nhiệm:**

TicketRepository kiểm tra tính hợp lệ của dữ liệu và thực hiện chuyển trạng thái.

BookingService chịu trách nhiệm:

- Kiểm tra quyền `TICKET_CANCEL`.
- Xác nhận hoàn tiền.
- Kiểm tra các điều kiện nghiệp vụ hủy vé.
- Thu hồi điểm thành viên.
- Điều phối lưu TXT.

**Giới hạn:** `update()` chỉ thay đổi dữ liệu trong RAM, không tự gọi `save()`.

**Giải thích từng đoạn code:** Điều kiện đầu kiểm tra `repo.isLoaded()` và dùng `isValidTicket(&newValues)` cho toàn bộ dữ liệu mới, gồm status, trường hủy và tiền hoàn; vì vậy không lặp các phép kiểm tra này ở các nhánh sau. `repo.findById(newValues.getTicketId())` lấy Ticket đang được sở hữu. Điều kiện kế tiếp đồng thời yêu cầu tồn tại, hiện là SOLD và dữ liệu mới là CANCELLED. Khối so sánh tiếp theo giữ nguyên invoiceId, showtimeId, seatId, loại vé, giá gốc, giảm giá, thực trả và điểm; ID đã được dùng làm khóa tìm kiếm nên không cần so lại. `isValidReference(&newValues)` kiểm tra các dependency còn load, tham chiếu và mốc hủy so với suất/hóa đơn; đây là kiểm tra ở thời điểm cập nhật, không trùng với validation thuộc tính. Cuối cùng `markCancelled()` nhận ba chuỗi hủy và tự cập nhật status, refund; hàm trả `true` sau lời gọi này. Không có `try/catch` hoặc ghi TXT trong `update()`.

### 8.7. `removeById(const string& ticketId)`

**Mục đích:** Cho phép Service hoàn tác Ticket mới được thêm khi giao dịch bán vé thất bại.

**Thứ tự kiểm tra và thực hiện:**

1. Nếu Repository chưa load, trả `false`.
2. Tìm Ticket theo ID.
3. Nếu Ticket không tồn tại, trả `false`.
4. Gọi `ticket->unlinkReferences()` để tháo liên kết với Invoice, Showtime và Seat.
5. Gọi `repo.removeById(ticketId)` để xóa Ticket khỏi danh sách, giải phóng bộ nhớ và trả kết quả.

**Vì sao phải unlink trước khi xóa?**

Invoice đang lưu con trỏ đến Ticket trong danh sách của mình.

Nếu xóa Ticket trước, Invoice có thể giữ con trỏ đến vùng nhớ không còn hợp lệ.

**Luồng hoạt động:**

```text
Loaded? -- không --> false
   |
   v
Tìm Ticket -- thiếu --> false
   |
   v
unlinkReferences() --> repo.removeById(ticketId) --> trả kết quả
```

**Vì sao bỏ `rollbackPending()`?**

Thiết kế mới sử dụng `removeById()` thuộc `IRepository<Ticket>` để thống nhất API với các Repository đã refactor.

Không cần duy trì `pendingIds` riêng trong TicketRepository.

**Phạm vi sử dụng:** Chỉ Service được sử dụng hàm này để hoàn tác Ticket vừa thêm trong giao dịch chưa ghi TXT.

**Giới hạn an toàn:**

Repository không tự phân biệt Ticket mới thêm với Ticket lịch sử đã lưu.

Do đó, `removeById()` về mặt kỹ thuật có thể xóa một Ticket lịch sử nếu caller sử dụng sai.

Service phải kiểm soát ngữ cảnh gọi. Không dùng hàm này để hủy vé thông thường.

Nếu tháo liên kết thành công nhưng `repo.removeById()` thất bại, Ticket có thể còn trong RAM nhưng mất liên kết. Đây không phải cơ chế transaction tuyệt đối và cần được kiểm thử.

**Giải thích từng đoạn code:** `repo.isLoaded()` và `repo.findById(ticketId)` được kiểm tra trước khi thay đổi liên kết. Nếu thiếu trạng thái hoặc đối tượng, trả `false`. Với Ticket tồn tại, `unlinkReferences()` tháo con trỏ Ticket khỏi Invoice và xóa các liên kết đến Showtime/Seat; sau đó `return repo.removeById(ticketId)` giao việc xóa node, giải phóng Ticket và kết quả `bool` cho Repository chung. Không gọi `save()` hay kiểm tra trạng thái SOLD/CANCELLED, vì phạm vi rollback do Service quyết định. Code không bắt ngoại lệ hoặc khôi phục liên kết nếu xóa ở tầng dưới thất bại.

### 8.8. `findById(const string& ticketId)`

**Mục đích:** Tìm Ticket theo khóa chính.

**Quy trình:** Gọi `repo.findById(ticketId)` và trả trực tiếp kết quả.

Nếu không tìm được, kết quả là `nullptr`.

Không tự duyệt lại danh sách vì Repository chung đã thực hiện chức năng này.

**Luồng hoạt động chi tiết:**

```text
Nhận ticketId --> repo.findById(ticketId) --> trả Ticket* hoặc nullptr
```

**Giải thích từng đoạn code:** `return` chuyển nguyên con trỏ do `repo.findById()` tìm được. Đây là con trỏ mượn đến Ticket thuộc quyền sở hữu của Repository; hàm không kiểm tra loaded hoặc tạo bản sao.

### 8.9. `getAt(int index)`

**Mục đích:** Lấy Ticket theo vị trí trong danh sách.

**Quy trình:** Chuyển tiếp sang `repo.getAt(index)`.

Không tự kiểm tra chỉ số vì Repository chung đã xử lý.

**Luồng hoạt động chi tiết:**

```text
Nhận index --> repo.getAt(index) --> trả Ticket* hoặc nullptr
```

**Giải thích từng đoạn code:** `return repo.getAt(index)` chuyển tiếp việc kiểm tra vị trí cho Repository chung. Con trỏ trả về không chuyển quyền sở hữu; không có nhánh hoặc exception riêng trong hàm này.

### 8.10. `count()`

**Mục đích:** Lấy số lượng Ticket đang được quản lý trong RAM.

**Quy trình:** Trả `repo.count()`.

Không duy trì biến đếm riêng trong TicketRepository.

**Luồng hoạt động chi tiết:**

```text
count() --> repo.count() --> trả số Ticket trong RAM
```

**Giải thích từng đoạn code:** Hàm chỉ `return repo.count()`; Repository chung đọc số nút hiện có, kể cả khi chưa load thì kết quả phản ánh danh sách hiện tại. Không duyệt hoặc thay đổi Ticket.

### 8.11. `isLoaded()`

**Mục đích:** Kiểm tra Repository đã nạp dữ liệu hay chưa.

**Quy trình:** Trả `repo.isLoaded()`.

Không tạo thêm cờ `loaded` trong TicketRepository.

**Luồng hoạt động chi tiết:**

```text
isLoaded() --> repo.isLoaded() --> trả bool
```

**Giải thích từng đoạn code:** `return repo.isLoaded()` chuyển tiếp cờ trạng thái từ Repository chung; hàm không tự nạp dữ liệu và không thay đổi trạng thái.

---

## 9. Quản lý bộ nhớ và liên kết

`Repository<Ticket>` sở hữu Ticket sau khi `load()` hoặc `add()` thành công.

Caller vẫn sở hữu Ticket nếu `add()` thất bại trước khi chuyển quyền sở hữu.

Các liên kết đến Showtime, Seat và Invoice không chuyển quyền sở hữu.

Đặc biệt, `Invoice::linkTicket()` lưu con trỏ Ticket vào danh sách của Invoice. Vì vậy, trước khi giải phóng Ticket phải gọi `unlinkReferences()` để tháo liên kết hai chiều.

**Thứ tự rollback khi bán vé thất bại:**

1. Khôi phục điểm Member nếu đã thay đổi.
2. Xóa các Ticket vừa thêm theo thứ tự ngược.
3. Sau khi Invoice không còn Ticket liên kết, xóa Invoice vừa thêm.

**Thứ tự hủy Repository:**

TicketRepository phải tháo liên kết trước khi InvoiceRepository và các Repository dependency bị giải phóng.

Nếu lưu Invoice thành công nhưng lưu Ticket thất bại, không có transaction tự động để khôi phục cả hai file TXT.

Repository không phân biệt Ticket đã lưu hay chưa. Service phải kiểm soát phạm vi hoàn tác.

---

## 10. DIP, trách nhiệm validation và rollback

### 10.1. DIP và DI

TicketRepository kế thừa `IRepository<Ticket>` và sử dụng `Repository<Ticket>` theo composition.

Các dependency gồm:

- `IRepository<Showtime>&`
- `IRepository<Seat>&`
- `IRepository<Invoice>&`

Concrete Repository được truyền vào khi khởi tạo.

Việc sử dụng interface giúp TicketRepository không phụ thuộc trực tiếp vào các lớp Repository cụ thể.

`IRepository<Seat>::findById(const string&)` nhận khóa dạng chuỗi. `SeatRepository` hiện dùng `roomId + "|" + seatId` làm khóa; TicketRepository ghép khóa này để tra Seat qua interface, không cần phụ thuộc overload của lớp cụ thể.

### 10.2. Phân chia trách nhiệm kiểm tra

| Tầng | Trách nhiệm |
|---|---|
| `Repository<Ticket>` | Loaded, null, khóa rỗng/trùng, trùng con trỏ, đọc/ghi TXT, quyền sở hữu. |
| `TicketRepository` | Thuộc tính Ticket, tham chiếu, xung đột ghế SOLD, liên kết, cập nhật trạng thái và tháo liên kết khi rollback. |
| `Ticket` Model | Lưu thuộc tính, thiết lập/tháo liên kết và thay đổi trạng thái hủy. |
| `Invoice` Model | Quản lý danh sách Ticket liên kết với Invoice. |
| Business Service | Phân quyền, bán/hủy vé, tính giá, hoàn tiền, điểm thành viên, rollback giữa các Repository và điều phối lưu TXT. |

### 10.3. Thứ tự kiểm tra và tránh lặp

- Parser kiểm tra định dạng và thuộc tính; `load()` chỉ kiểm tra tham chiếu, xung đột ghế và liên kết.
- `load()` không kiểm tra lại trạng thái loaded của Repository chung vì `repo.load()` đã xử lý. `isValidReference()` vẫn kiểm tra trạng thái ba dependency vì `add()` và `update()` cũng gọi hàm này độc lập.
- `add()` kiểm tra các điều kiện trước khi có tác dụng phụ, sau đó mới link và chuyển quyền sở hữu.
- `add()` kiểm tra trùng ID/con trỏ trước khi link để tránh tháo nhầm liên kết của Ticket đã được Repository sở hữu; `repo.add()` vẫn kiểm tra lại tại ranh giới chuyển quyền sở hữu.
- Không gọi `isValidTicket()` bên trong `isValidReference()`.
- `isSeatSold()` chỉ kiểm tra trạng thái ghế theo dữ liệu Ticket; `add()` đã kiểm tra loaded trước khi gọi helper.
- `update()` dùng `isValidTicket()` cho dữ liệu hủy và `isValidReference()` cho tham chiếu, thời gian hủy; chỉ so sánh riêng các thuộc tính bán vé phải bất biến trước khi gọi `markCancelled()`.
- `removeById()` không lặp validation Ticket; chỉ kiểm tra loaded, sự tồn tại và tháo liên kết.

### 10.4. Phạm vi rollback

`removeById()` chỉ được Service sử dụng để hoàn tác Ticket vừa thêm khi giao dịch bán vé thất bại trước khi ghi dữ liệu.

Service phải hoàn tác Ticket trước Invoice để không để Invoice giữ con trỏ đến Ticket đã bị xóa.

Repository không tự phân biệt Ticket đã lưu hay chưa, nên đây không phải transaction xuyên nhiều file TXT.

**Hủy vé thông thường không phải rollback.**

Khi hủy vé, Ticket vẫn tồn tại trong Repository và được chuyển từ `SOLD` sang `CANCELLED` bằng `update()`.

### 10.5. Giới hạn của thiết kế

1. `removeById()` không tự bảo vệ Ticket lịch sử đã được lưu TXT.
2. `update()` không tự lưu dữ liệu hoặc xử lý điểm thành viên.
3. Repository không bảo đảm transaction atomic giữa Ticket, Invoice và Member.
4. `load()` không tự đối chiếu tổng tiền các Ticket với `Invoice::totalAmount` hoặc bảo đảm mỗi Invoice có ít nhất một Ticket; việc đối chiếu toàn bộ hóa đơn cần xử lý riêng sau khi nạp Ticket.
5. Repository không kiểm tra quyền Admin của người hủy vé.
6. `ticketToString()` tiếp tục dùng `to_string(double)` theo thiết kế hiện tại, ghi sáu chữ số thập phân. Giá trị nạp lại có thể khác giá trị trong RAM hoặc không còn thỏa ràng buộc sai số; đây là giới hạn độ chính xác đã được chấp nhận ở phạm vi refactor này, cần kiểm thử và xử lý riêng nếu yêu cầu bảo toàn số thực cao hơn.
7. Việc tìm Seat theo khóa ghép dùng `IRepository<Seat>::findById()`; Repository chung hiện duyệt danh sách tuyến tính.
8. `BookingService` còn gọi `isSeatSold()` và `rollbackPending()`, còn `InvoiceService` gọi `getByInvoice()`; các lời gọi này phải được refactor khi tích hợp API mới. Đây là công việc tích hợp còn lại, không chặn việc chốt và triển khai riêng TicketRepository. Chưa coi toàn hệ thống đã tương thích hoặc biên dịch được với API mới.

---

## 11. Các thay đổi so với phiên bản cũ

| Nội dung | Phiên bản cũ | Thiết kế refactor |
|---|---|---|
| Interface | Không kế thừa `IRepository<Ticket>` | Kế thừa và triển khai 9 hàm `override`. |
| Dependency | Concrete Repository | Ba `IRepository<T>&` tương ứng. |
| `update()` | Chưa có | Hỗ trợ chuyển `SOLD → CANCELLED`. |
| `removeById()` | Chưa có | Hỗ trợ rollback Ticket mới thêm. |
| `pendingIds` | Có | Loại bỏ. |
| `rollbackPending()` | Có | Loại bỏ, thay bằng `removeById()`. |
| `getByShowtime()` | Public | Loại bỏ. |
| `getByInvoice()` | Public | Loại bỏ. |
| `isSeatSold()` | Public | Chuyển thành private. |
| Tra cứu Seat | Tìm qua lớp cụ thể | Dùng `IRepository<Seat>::findById()` với khóa ghép. |
| `load()` | Kiểm tra tham chiếu, trùng ghế và liên kết | Giữ logic chính, điều chỉnh dependency và tìm Seat. |
| `add()` | Validation, liên kết và pending rollback | Giữ logic chính, bỏ pending và điều chỉnh tìm Seat. |
| `save()` | Ghi file và xóa pendingIds | Trả trực tiếp `repo.save()`. |
| Schema TXT | 14 trường | Giữ nguyên 14 trường. |
| Trạng thái | Code cũ đã tồn tại | Chỉ thiết kế Markdown, chưa triển khai và kiểm thử. |

### 11.1. Các trường hợp cần kiểm thử

1. Load file rỗng, nhiều Ticket, load lại và dependency chưa load.
2. Sai số trường TXT, sai enum, ID, giá tiền, điểm và thông tin hủy.
3. Tham chiếu không tồn tại, Seat sai Room, Invoice sai Showtime, điểm dương khi Invoice không có Member và thời gian hủy không hợp lệ.
4. Hai Ticket SOLD trùng ghế phải bị từ chối; SOLD và CANCELLED cùng ghế được chấp nhận.
5. `add()` hợp lệ, null, trùng ID, trùng con trỏ, Ticket đã link, lỗi tham chiếu và rollback liên kết.
6. `update()` thành công khi SOLD → CANCELLED; từ chối các chuyển trạng thái khác, thay đổi thuộc tính bất biến hoặc dữ liệu hủy không hợp lệ.
7. `removeById()` từ chối khi chưa load hoặc không có Ticket; kiểm tra unlink Invoice và giải phóng bộ nhớ khi xóa.
8. `save()` thành công/thất bại, load lại dữ liệu và kiểm tra sai số tiền.
9. Kiểm tra bốn hàm truy vấn chung.
10. Tích hợp BookingService: kiểm tra ghế, bán vé, rollback Ticket trước Invoice, hủy vé và thu hồi điểm Member.

**Trạng thái tài liệu: ĐANG REVIEW THIẾT KẾ.**

Chưa triển khai code `.h/.cpp`, chưa xác nhận biên dịch hoặc kiểm thử.

Chỉ chuyển sang **CHỐT MARKDOWN** khi đã rà soát xong toàn bộ thiết kế và bảo đảm mã nguồn dự kiến ở mục 5 phù hợp với các quy tắc đã thống nhất.
