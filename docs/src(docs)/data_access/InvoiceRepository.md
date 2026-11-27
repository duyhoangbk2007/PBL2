# InvoiceRepository

## 1. Tổng quan

`InvoiceRepository` quản lý hóa đơn tại tầng Data Access, triển khai `IRepository<Invoice>` và sử dụng `Repository<Invoice>` bằng composition. Các Repository phụ thuộc được truyền qua interface: `IRepository<Showtime>&`, `IRepository<User>&`, `IRepository<Member>&`.

**Trạng thái: CHỐT MARKDOWN.** Chưa hoàn thành kiểm thử.

**File liên quan:** `src/data_access/InvoiceRepository.h/.cpp`, `src/data_access/IRepository.h`, `src/data_access/Repository.h`, `src/models/Invoice.h/.cpp`, `src/models/Showtime.h`, `src/models/User.h`, `src/models/Member.h`, `src/utils/Validator.h`, `src/utils/DateTime.h`.

**Quyết định nghiệp vụ:** `update()` luôn trả `false`. `removeById()` chỉ được Service sử dụng để hoàn tác hóa đơn vừa tạo trong giao dịch bán vé thất bại. Repository kiểm tra Invoice tồn tại và không còn Ticket liên kết, nhưng chưa thể phân biệt hóa đơn đang hoàn tác với hóa đơn lịch sử đã lưu. Tầng Repository không phân quyền, không tính giá vé và không điều phối giao dịch nhiều file.

## 2. Kiến trúc

```text
Business Service ---> IRepository<Invoice> <--- InvoiceRepository
                                                  | chứa
                                                  v
                                           Repository<Invoice>
                                                  |
                                   tham chiếu qua interface
                                   +-- IRepository<Showtime>
                                   +-- IRepository<User>
                                   +-- IRepository<Member>
```

- `Repository<Invoice>` sở hữu các con trỏ `Invoice*` sau khi `load()` hoặc `add()` thành công; tự giải phóng khi xóa hoặc hủy.
- `InvoiceRepository` chỉ mượn ba dependency, không sở hữu các đối tượng Showtime/User/Member.
- Các Repository phụ thuộc phải `load()` trước InvoiceRepository và sống lâu hơn InvoiceRepository.
- TicketRepository phải được hủy hoặc tháo liên kết Ticket trước khi hủy InvoiceRepository.
- `IRepository<Invoice>` có chín hàm chung; `removeById()` phục vụ hoàn tác giao dịch bán vé thất bại theo điều phối của Service.

## 3. Định dạng dữ liệu

Mỗi dòng có đúng **6 trường**, không có header:

```text
invoiceId|showtimeId|staffId|memberId|createdAt|totalAmount
```

| Trường | Điều kiện |
|---|---|
| `invoiceId` | ID hợp lệ tiền tố `INV`, không trùng. |
| `showtimeId` | ID hợp lệ tiền tố `SHW`, Showtime tồn tại. |
| `staffId` | ID hợp lệ tiền tố `USR`, User tồn tại và có role `STAFF` hoặc `ADMIN`. |
| `memberId` | Có thể rỗng; nếu có phải hợp lệ tiền tố `MEM` và Member tồn tại. |
| `createdAt` | DateTime hợp lệ, phải trước thời điểm bắt đầu Showtime. |
| `totalAmount` | Số hữu hạn, không âm. |

**Không có trường điểm trong TXT Invoice**; điểm liên quan đến Ticket/Member theo thiết kế hiện tại. Không thay đổi schema trong đợt refactor này.

## 4. Thiết kế lớp

### 4.1. Thuộc tính

| Thuộc tính | Kiểu | Vai trò |
|---|---|---|
| `repo` | `Repository<Invoice>` | Lưu trữ, sở hữu Invoice, đọc/ghi TXT. |
| `showtimeRepo` | `IRepository<Showtime>&` | Tra cứu suất chiếu. |
| `userRepo` | `IRepository<User>&` | Tra cứu nhân viên bán vé. |
| `memberRepo` | `IRepository<Member>&` | Tra cứu thành viên tùy chọn. |

### 4.2. Hàm private

| Hàm | Trách nhiệm |
|---|---|
| `getInvoiceId()` | Callback lấy ID; an toàn với `nullptr`. |
| `stringToInvoice()` | Tách sáu trường, chuyển số, gọi `isValidInvoice()` một lần. |
| `invoiceToString()` | Kiểm tra dữ liệu và chuyển thành một dòng TXT. |
| `isValidInvoice()` | Chỉ kiểm tra thuộc tính, không truy vấn Repository khác. |
| `isValidReference()` | Caller đã kiểm tra `isValidInvoice()`; kiểm tra loaded, đối tượng tham chiếu, role và thời gian. |
| `unlinkAll()` | Gỡ các liên kết Invoice khi load thất bại hoặc destructor chạy. |

### 4.3. Hàm public

| Hàm | Hành vi |
|---|---|
| `load()` | Đọc file, kiểm tra tham chiếu, liên kết; dọn toàn bộ nếu liên kết thất bại. |
| `save()` | Chuyển tiếp sang `repo.save()`. |
| `add()` | Thêm Invoice hợp lệ, chưa liên kết; nếu thất bại phải dọn liên kết vừa tạo. |
| `update()` | Luôn `false`, không hỗ trợ sửa hóa đơn. |
| `removeById()` | Chỉ phục vụ Service hoàn tác hóa đơn vừa tạo khi bán vé thất bại: kiểm tra loaded, Invoice tồn tại và không còn Ticket; tháo liên kết rồi xóa. Repository chưa phân biệt hóa đơn đang hoàn tác với lịch sử đã lưu. |
| `findById()`, `getAt()`, `count()`, `isLoaded()` | Chuyển tiếp sang `repo`. |

### 4.4. Thứ tự kiểm tra và tránh lặp

1. `load()`: kiểm tra dependency loaded → `repo.load()` (parser đã validate dữ liệu) → validate tham chiếu → liên kết. Không gọi `isValidInvoice()` lại từ `isValidReference()`.
2. `add()`: kiểm tra loaded và `isValidInvoice()` → loại ID trùng và con trỏ đã quản lý → loại Invoice đã liên kết → `isValidReference()` → tạo liên kết → `repo.add()`. Kiểm tra trùng trước liên kết để tránh phải rollback không cần thiết.
3. `isValidReference()` không tự kiểm tra thuộc tính; **chỉ gọi sau khi đối tượng đã được validate** (parser hoặc `add()`).
4. `Repository<T>::add()` vẫn tự kiểm tra lại ID và con trỏ như lớp phòng vệ cuối; đây là kiểm tra ở ranh giới sở hữu, không phải lặp nghiệp vụ.
5. `removeById()` kiểm tra loaded, tìm Invoice theo ID và từ chối nếu còn Ticket; sau đó tháo liên kết và gọi `repo.removeById()`. Không kiểm tra lại ID, thời gian, role hoặc dữ liệu đã được xác thực khi thêm.
6. `update()` không làm gì ngoài trả `false`.

## 5. Mã nguồn

Các khối sau là **thiết kế đã chốt**; chưa hoàn thành kiểm thử.

### 5.1. InvoiceRepository.h

```cpp
#ifndef INVOICE_REPOSITORY_H
#define INVOICE_REPOSITORY_H

#include <string>
#include <vector>
#include "IRepository.h"
#include "Repository.h"
#include "../models/Invoice.h"

class Showtime;
class User;
class Member;

class InvoiceRepository : public IRepository<Invoice>
{
private:
    Repository<Invoice> repo;
    IRepository<Showtime>& showtimeRepo;
    IRepository<User>& userRepo;
    IRepository<Member>& memberRepo;

    // Chuyen doi du lieu
    static std::string getInvoiceId(Invoice* invoice);
    static Invoice* stringToInvoice(const std::string& line);
    static std::string invoiceToString(Invoice* invoice);

    // Kiem tra du lieu va tham chieu
    static bool isValidInvoice(const Invoice* invoice);
    bool isValidReference(const Invoice* invoice);

    // Quan ly lien ket
    void unlinkAll();

public:
    InvoiceRepository(const std::string& filePath,
                      IRepository<Showtime>& showtimeRepo,
                      IRepository<User>& userRepo,
                      IRepository<Member>& memberRepo);
    ~InvoiceRepository() override;

    // Doc va ghi du lieu
    bool load() override;
    bool save() override;

    // Quan ly du lieu
    bool add(Invoice* invoice) override;
    bool update(const Invoice& newValues) override;
    bool removeById(const std::string& invoiceId) override;

    // Truy van du lieu
    Invoice* findById(const std::string& invoiceId) override;
    Invoice* getAt(int index) override;
    int count() override;
    bool isLoaded() override;

};

#endif
```

### 5.2. InvoiceRepository.cpp

```cpp
#include "InvoiceRepository.h"
#include "../models/Showtime.h"
#include "../models/User.h"
#include "../models/Member.h"
#include "../utils/Validator.h"
#include "../utils/DateTime.h"
#include <cmath>
#include <stdexcept>
#include <vector>
using namespace std;

bool InvoiceRepository::isValidInvoice(const Invoice* invoice)
{
    if (invoice == nullptr)
    {
        return false;
    }
    if (!Validator::isValidId(invoice->getInvoiceId(), "INV") ||
        !Validator::isValidId(invoice->getShowtimeId(), "SHW") ||
        !Validator::isValidId(invoice->getStaffId(), "USR"))
    {
        return false;
    }
    if (!invoice->getMemberId().empty() &&
        !Validator::isValidId(invoice->getMemberId(), "MEM"))
    {
        return false;
    }
    if (!Validator::isValidDateTime(invoice->getCreatedAt()))
    {
        return false;
    }
    double totalAmount = invoice->getTotalAmount();
    if (!isfinite(totalAmount) || totalAmount < 0)
    {
        return false;
    }
    return true;
}

InvoiceRepository::InvoiceRepository(const string& filePath,
                                     IRepository<Showtime>& showtimeRepo,
                                     IRepository<User>& userRepo,
                                     IRepository<Member>& memberRepo)
    : repo(filePath, getInvoiceId, stringToInvoice, invoiceToString),
      showtimeRepo(showtimeRepo),
      userRepo(userRepo),
      memberRepo(memberRepo)
{
}

string InvoiceRepository::getInvoiceId(Invoice* invoice)
{
    if (invoice == nullptr)
    {
        return "";
    }
    return invoice->getInvoiceId();
}

Invoice* InvoiceRepository::stringToInvoice(const string& line)
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
    if (fields.size() != 6)
    {
        throw runtime_error("Sai so luong truong Invoice");
    }

    double totalAmount = 0;
    try
    {
        if (!Validator::isValidNumber(fields[5]))
        {
            throw runtime_error("Sai dinh dang tong tien");
        }
        size_t pos = 0;
        totalAmount = stod(fields[5], &pos);
        if (pos != fields[5].size())
        {
            throw runtime_error("Sai dinh dang tong tien");
        }
    }
    catch (const exception&)
    {
        throw runtime_error("Tong tien Invoice khong hop le");
    }

    Invoice* invoice = new Invoice(fields[0],
                                   fields[1],
                                   fields[2],
                                   fields[3],
                                   fields[4],
                                   totalAmount);
    if (!isValidInvoice(invoice))
    {
        delete invoice;
        throw runtime_error("Du lieu Invoice khong hop le");
    }
    return invoice;
}

string InvoiceRepository::invoiceToString(Invoice* invoice)
{
    if (!isValidInvoice(invoice))
    {
        throw runtime_error("Invoice khong hop le khi ghi file");
    }
    return invoice->getInvoiceId() + "|" +
           invoice->getShowtimeId() + "|" +
           invoice->getStaffId() + "|" +
           invoice->getMemberId() + "|" +
           invoice->getCreatedAt() + "|" +
           to_string(invoice->getTotalAmount());
}

InvoiceRepository::~InvoiceRepository()
{
    unlinkAll();
}

// Caller da kiem tra isValidInvoice().
bool InvoiceRepository::isValidReference(const Invoice* invoice)
{
    if (!showtimeRepo.isLoaded() ||
        !userRepo.isLoaded() ||
        !memberRepo.isLoaded())
    {
        return false;
    }

    Showtime* showtime = showtimeRepo.findById(invoice->getShowtimeId());
    User* seller = userRepo.findById(invoice->getStaffId());
    if (showtime == nullptr || seller == nullptr)
    {
        return false;
    }
    if (seller->getRole() != UserRole::STAFF &&
        seller->getRole() != UserRole::ADMIN)
    {
        return false;
    }
    if (!DateTime::isBefore(invoice->getCreatedAt(),
                            showtime->getStartAt()))
    {
        return false;
    }
    if (!invoice->getMemberId().empty())
    {
        Member* member = memberRepo.findById(invoice->getMemberId());
        if (member == nullptr)
        {
            return false;
        }
    }
    return true;
}

void InvoiceRepository::unlinkAll()
{
    for (int i = 0; i < repo.count(); i++)
    {
        Invoice* invoice = repo.getAt(i);
        if (invoice != nullptr)
        {
            invoice->unlinkReferences();
        }
    }
}

bool InvoiceRepository::load()
{
    if (!showtimeRepo.isLoaded() ||
        !userRepo.isLoaded() ||
        !memberRepo.isLoaded())
    {
        return false;
    }
    if (!repo.load())
    {
        return false;
    }

    try
    {
        // Giai doan 1: kiem tra toan bo tham chieu
        for (int i = 0; i < repo.count(); i++)
        {
            Invoice* invoice = repo.getAt(i);
            if (!isValidReference(invoice))
            {
                throw runtime_error("Invoice co tham chieu khong hop le");
            }
        }
        // Giai doan 2: lien ket cac doi tuong
        for (int i = 0; i < repo.count(); i++)
        {
            Invoice* invoice = repo.getAt(i);
            Showtime* showtime = showtimeRepo.findById(invoice->getShowtimeId());
            User* seller = userRepo.findById(invoice->getStaffId());
            Member* member = nullptr;
            if (!invoice->getMemberId().empty())
            {
                member = memberRepo.findById(invoice->getMemberId());
            }
            if (!invoice->linkShowtime(showtime) ||
                !invoice->linkSeller(seller) ||
                !invoice->linkMember(member))
            {
                throw runtime_error("Khong the lien ket Invoice");
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

bool InvoiceRepository::save()
{
    return repo.save();
}

bool InvoiceRepository::add(Invoice* invoice)
{
    if (!repo.isLoaded() || !isValidInvoice(invoice))
    {
        return false;
    }
    if (repo.findById(invoice->getInvoiceId()) != nullptr)
    {
        return false;
    }
    for (int i = 0; i < repo.count(); i++)
    {
        if (repo.getAt(i) == invoice)
        {
            return false;
        }
    }
    if (invoice->getShowtime() != nullptr ||
        invoice->getSeller() != nullptr ||
        invoice->getMember() != nullptr)
    {
        return false;
    }
    if (!isValidReference(invoice))
    {
        return false;
    }

    Showtime* showtime = showtimeRepo.findById(invoice->getShowtimeId());
    User* seller = userRepo.findById(invoice->getStaffId());
    Member* member = nullptr;
    if (!invoice->getMemberId().empty())
    {
        member = memberRepo.findById(invoice->getMemberId());
    }

    try
    {
        if (!invoice->linkShowtime(showtime) ||
            !invoice->linkSeller(seller) ||
            !invoice->linkMember(member))
        {
            invoice->unlinkReferences();
            return false;
        }

        if (!repo.add(invoice))
        {
            invoice->unlinkReferences();
            return false;
        }
    }
    catch (...)
    {
        invoice->unlinkReferences();
        throw;
    }
    return true;
}

bool InvoiceRepository::update(const Invoice& newValues)
{
    return false;
}

bool InvoiceRepository::removeById(const string& invoiceId)
{
    if (!repo.isLoaded())
    {
        return false;
    }
    Invoice* invoice = repo.findById(invoiceId);
    if (invoice == nullptr || invoice->getTicketCount() != 0)
    {
        return false;
    }
    invoice->unlinkReferences();
    return repo.removeById(invoiceId);
}

Invoice* InvoiceRepository::findById(const string& invoiceId)
{
    return repo.findById(invoiceId);
}

Invoice* InvoiceRepository::getAt(int index)
{
    return repo.getAt(index);
}

int InvoiceRepository::count()
{
    return repo.count();
}

bool InvoiceRepository::isLoaded()
{
    return repo.isLoaded();
}
```

## 6. Giải thích thư viện và từ khóa

### 6.1. Các thư viện và header

| Thành phần | Vai trò |
|---|---|
| `IRepository.h` | Interface chung, định nghĩa chín hàm phải `override`. |
| `Repository.h` | Quản lý danh sách Invoice, quyền sở hữu con trỏ, đọc/ghi TXT và kiểm tra khóa. |
| `Invoice.h` | Model hóa đơn và các phương thức liên kết. |
| `Showtime.h`, `User.h`, `Member.h` | Cần định nghĩa đầy đủ để gọi getter và thao tác với các đối tượng tham chiếu. |
| `Validator.h` | Kiểm tra mã định danh, thời gian và dữ liệu số. |
| `DateTime.h` | So sánh thời điểm lập hóa đơn với giờ bắt đầu suất chiếu. |
| `<string>` | `string`, `find`, `substr`, `to_string`, `stod`. |
| `<vector>` | Tách trường TXT. |
| `<cmath>` | `isfinite()` kiểm tra tổng tiền không phải NaN hoặc vô cực. |
| `<stdexcept>` | `runtime_error` để thông báo lỗi dữ liệu hoặc liên kết. |

### 6.2. `static` và callback

`getInvoiceId()`, `stringToInvoice()`, `invoiceToString()` và `isValidInvoice()` là hàm `static` vì không cần truy cập `this`. Ba hàm đầu được truyền vào `Repository<Invoice>` dưới dạng con trỏ hàm để lấy khóa, parse và serialize.

### 6.3. `override`, kế thừa và composition

`InvoiceRepository : public IRepository<Invoice>` triển khai hợp đồng chung. `override` giúp trình biên dịch phát hiện sai chữ ký. `Repository<Invoice> repo` là composition, không phải kế thừa. Ba tham chiếu `IRepository<...>&` là dependency injection, không chuyển quyền sở hữu.

### 6.4. `const`, tham chiếu và con trỏ

`const string&` tránh sao chép chuỗi; `const Invoice*` cho phép đọc mà không sửa dữ liệu; `Invoice*` dùng để quản lý hoặc thiết lập liên kết. Các con trỏ đến Showtime, User, Member là con trỏ mượn.

### 6.5. `try`, `catch`, `throw`

`throw runtime_error(...)` dùng cho dữ liệu TXT hoặc liên kết bất thường. `catch (...)` bắt mọi ngoại lệ, dọn liên kết đang tạo rồi `throw;` để giữ nguyên ngoại lệ gốc. `false` dùng cho các điều kiện từ chối thông thường, không thay thế cho mọi lỗi bất thường.

## 7. Giải thích các hàm private

Phần này giải thích **mục đích, thứ tự thực hiện, điều kiện trả về và lý do thiết kế** của từng hàm. Mã nguồn đầy đủ tương ứng nằm ở mục 5.

### 7.1. `isValidInvoice(const Invoice* invoice)`

**Mục đích:** Xác nhận **thuộc tính nội tại** của một Invoice, không truy cập các Repository khác.

**Thứ tự kiểm tra:**
1. Con trỏ không được `nullptr`.
2. `invoiceId`, `showtimeId`, `staffId` có tiền tố lần lượt `INV`, `SHW`, `USR`.
3. `memberId` được phép rỗng; nếu không rỗng phải có tiền tố `MEM`.
4. `createdAt` phải có định dạng ngày giờ hợp lệ.
5. `totalAmount` phải hữu hạn (`isfinite`) và không âm.

**Kết quả:** Trả `false` ngay tại điều kiện đầu tiên không đạt; chỉ trả `true` khi toàn bộ thuộc tính hợp lệ.

**Lý do tách hàm:** Một quy tắc dữ liệu dùng chung cho parser, serializer và `add()`. Không kiểm tra sự tồn tại của Showtime/User/Member tại đây để tránh trộn validation thuộc tính với validation tham chiếu.

### 7.2. `getInvoiceId(Invoice* invoice)`

**Mục đích:** Cung cấp khóa chính cho `Repository<Invoice>` thông qua callback.

**Quy trình:**
1. Nếu `invoice == nullptr`, trả chuỗi rỗng để không truy cập con trỏ không hợp lệ.
2. Nếu hợp lệ, trả `invoice->getInvoiceId()`.

**Vì sao không kiểm tra `Validator` ở đây?** Callback chỉ có nhiệm vụ lấy khóa. Việc kiểm tra cấu trúc ID nằm trong `isValidInvoice()`; kiểm tra ID trùng/rỗng khi quản lý danh sách thuộc `Repository<T>`.

### 7.3. `stringToInvoice(const string& line)`

**Mục đích:** Chuyển một dòng TXT thành đối tượng `Invoice*`. Đây là callback được `Repository<Invoice>::load()` gọi.

**Quy trình chi tiết:**
1. Dùng `find('|', start)` để tìm từng dấu phân cách, `substr()` lấy từng trường và đưa vào `vector<string> fields`.
2. Kiểm tra đúng **6 trường**. Nếu thiếu hoặc thừa, ném `runtime_error`.
3. Kiểm tra trường `totalAmount` là số hợp lệ rồi dùng `stod(fields[5], &pos)` để chuyển thành `double`.
4. So sánh `pos` với độ dài trường để bảo đảm toàn bộ chuỗi đã được chuyển, không còn ký tự thừa.
5. Tạo `new Invoice(...)` từ sáu giá trị.
6. Gọi `isValidInvoice()` **một lần** để kiểm tra thuộc tính.
7. Nếu không hợp lệ, `delete invoice` rồi `throw`; nếu hợp lệ, trả con trỏ.

**Vì sao có `try/catch` quanh `stod()`?** Phép chuyển đổi có thể ném ngoại lệ khi chuỗi không chuyển được hoặc vượt phạm vi số. Hàm đổi các lỗi này thành thông báo thống nhất.

**Ai sở hữu đối tượng?** Sau khi callback trả thành công, `Repository<T>` tiếp nhận đối tượng trong quá trình load. Nếu parse thất bại sau `new`, callback tự xóa đối tượng vừa tạo.

**Không làm ở đây:** Không tìm Showtime/User/Member; giai đoạn kiểm tra tham chiếu của `load()` sẽ xử lý sau khi dữ liệu được đọc xong.

### 7.4. `invoiceToString(Invoice* invoice)`

**Mục đích:** Chuyển Invoice thành một dòng TXT đúng schema để `Repository<Invoice>::save()` ghi ra file.

**Quy trình:**
1. Gọi `isValidInvoice(invoice)`; nếu không hợp lệ, ném `runtime_error`.
2. Ghép lần lượt `invoiceId|showtimeId|staffId|memberId|createdAt|totalAmount`.
3. Dùng `to_string()` chuyển tổng tiền thành chuỗi.
4. Trả chuỗi; không tự mở hoặc ghi file.

**Vì sao vẫn kiểm tra Model khi save?** Dữ liệu trong RAM có thể đã bị thay đổi sau lúc add/load; serializer cần từ chối ghi thuộc tính sai. Không cần lặp lại việc tra các dependency nếu hợp đồng hiện tại chỉ yêu cầu serialize dữ liệu.

### 7.5. `isValidReference(const Invoice* invoice)`

**Mục đích:** Kiểm tra các quan hệ mà một Invoice khai báo bằng ID.

**Tiền điều kiện:** Caller đã xác nhận `isValidInvoice(invoice) == true`. Hàm không gọi lại `isValidInvoice()`.

**Thứ tự kiểm tra:**
1. `showtimeRepo`, `userRepo`, `memberRepo` đều đã load.
2. Tìm Showtime theo `showtimeId` và User theo `staffId`; cả hai phải tồn tại.
3. Người bán có role `STAFF` hoặc `ADMIN`.
4. `createdAt` phải trước `showtime->getStartAt()`.
5. Nếu `memberId` không rỗng, tìm Member và yêu cầu tồn tại; nếu rỗng thì bỏ qua.

**Kết quả:** `true` nếu tất cả tham chiếu hợp lệ, `false` nếu bất kỳ điều kiện nào thất bại.

**Phân chia trách nhiệm:** `isValidInvoice()` kiểm tra **ID đúng định dạng**; `isValidReference()` kiểm tra **ID trỏ đến đối tượng có thật và quan hệ hợp lệ**. Các hàm `link...()` của Model kiểm tra tính nhất quán khi thiết lập liên kết thực tế.

### 7.6. `unlinkAll()`

**Mục đích:** Tháo các liên kết từ mọi Invoice đang được quản lý, trước khi giải phóng chúng.

**Quy trình:**
1. Duyệt các vị trí `0` đến `repo.count() - 1`.
2. Lấy `Invoice*` bằng `repo.getAt(i)`.
3. Nếu con trỏ khác null, gọi `invoice->unlinkReferences()`.

**Nơi sử dụng:** Destructor và nhánh xử lý lỗi của `load()`.

**Lưu ý:** `Invoice::unlinkReferences()` hiện trả `void`, nên caller không biết một thao tác gỡ liên kết hai chiều có thất bại nội bộ hay không. Đây là giới hạn của Model hiện tại, không được mô tả như bảo đảm rollback tuyệt đối.

## 8. Giải thích các hàm public

### 8.1. Constructor

**Mục đích:** Nhận đường dẫn file và ba dependency bằng interface.

**Quy trình:** Khởi tạo `repo(filePath, getInvoiceId, stringToInvoice, invoiceToString)`, rồi lưu tham chiếu `showtimeRepo`, `userRepo`, `memberRepo`.

**Ý nghĩa:** `Repository<Invoice>` phụ trách lưu trữ chung; InvoiceRepository phụ trách quy tắc Invoice. Dependency injection giúp không phụ thuộc trực tiếp vào các lớp Repository cụ thể.

### 8.2. Destructor

**Mục đích:** Tháo liên kết trước khi thành viên `repo` bị hủy và giải phóng các Invoice.

**Quy trình:** Gọi `unlinkAll()`. Sau khi destructor body kết thúc, các thành viên bị hủy theo cơ chế C++.

**Điều kiện vòng đời:** TicketRepository phải tháo Ticket trước; Showtime/User/Member Repository phải còn tồn tại trong lúc InvoiceRepository tháo liên kết.

### 8.3. `load()`

**Mục đích:** Nạp Invoice từ TXT và thiết lập các quan hệ với Model khác.

**Thứ tự thực hiện:**
1. Kiểm tra ba dependency đã load; nếu chưa, trả `false`.
2. Gọi `repo.load()`; Repository chung tự kiểm tra trạng thái đã load, đọc file, parse, kiểm tra khóa và quản lý bộ nhớ.
3. **Vòng 1:** Duyệt mọi Invoice, gọi `isValidReference()`; gặp tham chiếu sai thì ném `runtime_error`.
4. **Vòng 2:** Lấy Showtime, Seller và Member (Member có thể null khi không có mã), lần lượt gọi `linkShowtime()`, `linkSeller()`, `linkMember()`.
5. Nếu có lỗi trong hai vòng, gọi `unlinkAll()`, `repo.clear()` và ném lại ngoại lệ.

**Vì sao chia hai vòng?** Không bắt đầu tạo liên kết nếu còn một Invoice có tham chiếu không hợp lệ. Nhờ vậy giảm số liên kết cần hoàn tác khi dữ liệu đầu vào sai.

**Tránh kiểm tra lặp:** Parser đã gọi `isValidInvoice()`; vòng 1 chỉ kiểm tra tham chiếu. `repo.load()` đã xử lý loaded và khóa trùng.

**Giới hạn:** Nếu thao tác dọn dẹp tự phát sinh lỗi, cần xem xét thêm khả năng bảo đảm trạng thái. `load()` không tạo transaction trên nhiều file.

### 8.4. `save()`

**Mục đích:** Ghi các Invoice đang quản lý xuống TXT.

**Quy trình:** Gọi `repo.save()` và trả trực tiếp kết quả của hàm này.

**Lưu ý:** `save()` của Invoice không đồng nghĩa Ticket đã được lưu. Hai file TXT không có transaction nguyên tử chung.

### 8.5. `add(Invoice* invoice)`

**Mục đích:** Thêm một hóa đơn mới và tạo liên kết hợp lệ.

**Thứ tự kiểm tra và thực hiện:**
1. `repo.isLoaded()` phải true; gọi `isValidInvoice(invoice)` để kiểm tra dữ liệu và null.
2. Từ chối ID đã tồn tại và con trỏ đã được quản lý **trước khi thay đổi liên kết**.
3. Từ chối Invoice đã có liên kết Showtime/Seller/Member.
4. Gọi `isValidReference()` để xác nhận dependency, role và thời gian.
5. Tra Showtime, Seller, Member tương ứng.
6. Trong `try`, gọi `linkShowtime()`, `linkSeller()`, `linkMember()`, sau đó `repo.add(invoice)`.
7. Nếu liên kết hoặc thêm thất bại, tháo liên kết vừa tạo; nếu ném ngoại lệ, dọn rồi ném lại.

**Vì sao kiểm tra trùng trước khi link?** Nếu đối tượng đã nằm trong Repository mà ta gọi `unlinkReferences()` khi thất bại, ta có thể vô tình tháo liên kết của đối tượng đang được quản lý. Kiểm tra trước giúp bảo vệ trạng thái hiện có.

**Vì sao `Repository<T>::add()` vẫn kiểm tra trùng?** Đó là kiểm tra phòng vệ của lớp sở hữu dữ liệu, cần thiết cho mọi loại Model. InvoiceRepository chỉ kiểm tra sớm để tránh tác dụng phụ lên liên kết.

**Quyền sở hữu:** Khi `repo.add()` thành công, Repository sở hữu Invoice. Nếu trả `false` trước khi chuyển quyền sở hữu, caller vẫn phải quản lý con trỏ đầu vào.

### 8.6. `update(const Invoice& newValues)`

**Mục đích:** Đáp ứng hàm thuần ảo của `IRepository<Invoice>`.

**Hành vi:** Luôn trả `false`. Không cần kiểm tra loaded, ID hoặc dữ liệu vì không hỗ trợ cập nhật Invoice; không thay đổi RAM/TXT. Tham số không được sử dụng trong xử lý.

### 8.7. `removeById(const string& invoiceId)`

**Mục đích:** Cho phép Service hoàn tác Invoice khi giao dịch bán vé thất bại.

**Thứ tự kiểm tra và thực hiện:**
1. Nếu Repository chưa load, trả `false`.
2. Tìm Invoice theo ID; nếu không tồn tại, trả `false`.
3. Nếu `invoice->getTicketCount() != 0`, trả `false` để không xóa hóa đơn đang có Ticket.
4. Gọi `invoice->unlinkReferences()` để tháo liên kết Showtime/Seller/Member.
5. Gọi `repo.removeById(invoiceId)` để xóa khỏi danh sách, giải phóng Invoice và trả trực tiếp kết quả.

**Phạm vi sử dụng:** Service chỉ được gọi hàm này để hoàn tác hóa đơn vừa tạo trong giao dịch bán vé thất bại. Repository chỉ kiểm tra Invoice tồn tại và không còn Ticket liên kết sau khi xác nhận đã load; nó chưa thể phân biệt hóa đơn đang hoàn tác với hóa đơn lịch sử đã lưu. Vì vậy thiết kế này chưa bảo vệ tuyệt đối dữ liệu lịch sử nếu Service gọi sai ngữ cảnh.

**Giới hạn an toàn:** Nếu tháo liên kết xong nhưng `repo.removeById()` thất bại, Invoice có thể còn trong RAM nhưng mất liên kết. Cần kiểm thử đường lỗi và cân nhắc khôi phục liên kết khi triển khai; không mặc định đây là transaction tuyệt đối.

### 8.8. `findById(const string& invoiceId)`

**Mục đích:** Tìm Invoice theo khóa chính.

**Quy trình:** Gọi `repo.findById(invoiceId)` và trả kết quả; không tự duyệt lại danh sách. Nếu không tìm được hoặc Repository chưa load, kết quả theo quy ước của Repository chung là `nullptr`.

### 8.9. `getAt(int index)`

**Mục đích:** Lấy Invoice theo vị trí trong danh sách.

**Quy trình:** Chuyển tiếp sang `repo.getAt(index)`. Không tự kiểm tra chỉ số vì Repository chung đã xử lý.

### 8.10. `count()`

**Mục đích:** Lấy số Invoice đang quản lý trong RAM.

**Quy trình:** Trả `repo.count()`; không duy trì biến đếm riêng.

### 8.11. `isLoaded()`

**Mục đích:** Kiểm tra Repository đã nạp dữ liệu hay chưa.

**Quy trình:** Trả `repo.isLoaded()`; không tạo thêm cờ `loaded` trong InvoiceRepository.

## 9. Quản lý bộ nhớ và liên kết

`Repository<Invoice>` sở hữu Invoice sau khi `load()` hoặc `add()` thành công; caller sở hữu Invoice nếu `add()` trả `false`. Các liên kết đến Showtime, User, Member và Ticket là tham chiếu, không chuyển quyền sở hữu. Trước khi giải phóng Invoice phải tháo các liên kết liên quan. Khi rollback bán vé, phải rollback Ticket trước Invoice; khi hủy hệ thống cũng cần hủy TicketRepository trước InvoiceRepository.

Nếu lưu Invoice thành công nhưng lưu Ticket thất bại, không có transaction tự động để khôi phục cả hai file. Repository chưa phân biệt Invoice đã lưu hay chưa; Service phải kiểm soát phạm vi hoàn tác.

## 10. DIP, trách nhiệm validation và rollback

### 10.1. DIP và DI

Service có thể phụ thuộc `IRepository<Invoice>&`; InvoiceRepository phụ thuộc `IRepository<Showtime>&`, `IRepository<User>&`, `IRepository<Member>&`. Lớp concrete được truyền khi khởi tạo. `removeById()` thuộc interface chung nhưng chỉ được Service dùng để hoàn tác giao dịch bán vé thất bại.

### 10.2. Phân chia trách nhiệm kiểm tra

| Tầng | Kiểm tra |
|---|---|
| `Repository<Invoice>` | Loaded, null, khóa rỗng/trùng, trùng con trỏ, đọc/ghi TXT, quyền sở hữu. |
| `InvoiceRepository` | Thuộc tính Invoice, tham chiếu, liên kết và tháo liên kết khi hoàn tác. |
| `Invoice` Model | Tính hợp lệ khi thiết lập liên kết và danh sách Ticket. |
| Business Service | Phân quyền, bán vé, tính giá, thứ tự rollback giữa các Repository và điều phối lưu file. |

### 10.3. Thứ tự kiểm tra và tránh lặp

- Parser kiểm tra định dạng và thuộc tính; `load()` chỉ kiểm tra tham chiếu rồi liên kết.
- `add()` kiểm tra các điều kiện **trước khi có tác dụng phụ**, rồi mới link và chuyển quyền sở hữu cho Repository chung.
- Không gọi `isValidInvoice()` từ `isValidReference()`.
- Không lặp lại kiểm tra dữ liệu Model trong `removeById()`; chỉ kiểm tra loaded, sự tồn tại của Invoice và Ticket liên kết.
- `update()` trả `false` ngay, không chạy validation thừa.

### 10.4. Phạm vi rollback

`removeById()` chỉ được Service sử dụng để hoàn tác hóa đơn vừa tạo trong giao dịch bán vé thất bại khi Invoice không còn Ticket. Repository chỉ kiểm tra Invoice tồn tại và không còn Ticket liên kết; nó chưa phân biệt hóa đơn đang hoàn tác với hóa đơn lịch sử đã lưu và chưa bảo vệ tuyệt đối lịch sử nếu bị gọi sai ngữ cảnh. Đây không phải transaction xuyên nhiều file TXT. Service phải hoàn tác các Ticket liên quan trước Invoice và kiểm tra kết quả từng bước.

`BookingService.cpp` hiện vẫn gọi `InvoiceRepository::rollbackPending()` trong luồng hoàn tác. Khi tích hợp thiết kế này, cần chuyển lời gọi đó sang `removeById()` đúng tại nhánh hoàn tác hóa đơn vừa tạo sau khi đã hoàn tác Ticket. `BookingService.cpp` chưa được sửa trong phạm vi triển khai này; lời gọi cũ hiện không tương thích với API mới.

## 11. Các thay đổi so với phiên bản cũ

| Nội dung | Phiên bản cũ | Thiết kế refactor |
|---|---|---|
| Interface | Không kế thừa `IRepository<Invoice>` | Kế thừa, triển khai chín hàm `override`. |
| Dependency | `ShowtimeRepository&`, `UserRepository&`, `MemberRepository&` | Ba `IRepository<T>&` tương ứng. |
| `update()` | Chưa có | Luôn trả `false`. |
| `removeById()` | Chưa có | Service dùng để hoàn tác bán vé thất bại khi Invoice không còn Ticket; Repository chưa phân biệt đã lưu hay chưa. |
| Validation | `isValidReference()` gọi lại `isValidInvoice()` | Tách kiểm tra thuộc tính và tham chiếu để tránh lặp. |
| Schema TXT | Sáu trường | Giữ nguyên sáu trường. |
| Trạng thái | Code cũ đã tồn tại | **CHỐT MARKDOWN; chưa xác nhận test.** |

### 11.1. Các trường hợp cần kiểm thử

1. Load file rỗng, nhiều Invoice, load lại, dependency chưa load.
2. Sai định dạng, trùng ID, thiếu tham chiếu, role sai, thời gian không hợp lệ; dọn khi load thất bại.
3. Add hợp lệ có/không Member, null, trùng ID, trùng con trỏ, Invoice đã link, lỗi tham chiếu.
4. `update()` luôn `false`; `removeById()` từ chối khi chưa load, không có Invoice hoặc còn Ticket, và kiểm tra liên kết hai chiều sau khi xóa.
5. Service chỉ gọi `removeById()` khi hoàn tác bán vé thất bại; kiểm tra riêng trường hợp Invoice đã lưu vì Repository chưa phân biệt trạng thái này.
6. Save thành công/thất bại và bốn hàm truy vấn.
7. Tích hợp Ticket–Invoice: rollback Ticket trước Invoice, hủy đúng thứ tự và kiểm tra tham chiếu.

**Trạng thái tài liệu: CHỐT MARKDOWN.** Chưa hoàn thành kiểm thử thiết kế này.
