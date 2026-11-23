# InvoiceService — lịch sử và trạng thái hóa đơn (Bước 11)

## 1. Tổng quan

`InvoiceService` là Service chỉ đọc. Nó dùng `InvoiceRepository`, `TicketRepository` và `AuthService` bằng tham chiếu; không tạo, sửa hoặc lưu Invoice/Ticket. Bán và hủy vé thuộc `BookingService`. Các con trỏ trả về là con trỏ mượn; caller không được `delete` hay giữ sau khi Repository bị hủy. Các hàm tra cứu lịch sử chỉ cần InvoiceRepository đã nạp; các hàm tính trạng thái và tiền cần thêm TicketRepository đã nạp.

Admin cần `INVOICE_VIEW_ALL`; Staff cần `INVOICE_VIEW_OWN` và chỉ xem hóa đơn mình lập. Staff được tra hóa đơn cũ **theo mã**, còn danh sách lịch sử chỉ trong ngày hiện tại. Khoảng lọc dùng `[from, to)` theo `createdAt`; GUI đổi ngày cuối được chọn thành 00:00 ngày kế tiếp. Quyền được kiểm tra từ bảng RBAC đang nạp, không suy chỉ từ vai trò.

`InvoiceStatus` có ba giá trị `ACTIVE`, `PARTIALLY_CANCELLED`, `CANCELLED`; chúng được tính từ Ticket mỗi lần gọi. Tiền hoàn lấy snapshot trên vé CANCELLED; tiền còn lại bằng tổng gốc trừ tiền hoàn. Hóa đơn/vé bất nhất, Repository chưa nạp hoặc thiếu quyền ném ngoại lệ. Riêng `findInvoiceById()` trả `nullptr` khi mã không tồn tại.

| API public | Kết quả |
| --- | --- |
| `findInvoiceById(invoiceId)` | Con trỏ Invoice mượn hoặc `nullptr` |
| `getInvoiceHistory(from, to, staffId, memberId)` | Vector con trỏ đã lọc và sắp xếp |
| `getInvoiceStatus(invoiceId)` | Trạng thái suy từ Ticket |
| `getRefundAmount(invoiceId)` | Tổng tiền hoàn |
| `getRemainingAmount(invoiceId)` | Tiền còn lại |

Mã nguồn: `src/business/InvoiceService.h/.cpp`. Các helper private được tài liệu hóa cùng các API public bên dưới.

## 2. InvoiceService() — khởi tạo

### A. Tổng quan

Nhận `InvoiceRepository&`, `TicketRepository&`, `AuthService&`; constructor không trả giá trị. Các đối tượng được tham chiếu phải sống lâu hơn Service; hàm không nạp hoặc ghi TXT.

### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Nhận ba tham chiếu phụ thuộc
    |
    v
Khởi tạo ba thành viên
    |
    v
Kết thúc
```

### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1.**

```cpp
InvoiceService::InvoiceService(InvoiceRepository& invoiceRepo,
                               TicketRepository& ticketRepo,
                               AuthService& authService)
    : invoiceRepo(invoiceRepo),
      ticketRepo(ticketRepo),
      authService(authService)
{
}
```

Danh sách sau dấu `:` khởi tạo các thành viên tham chiếu từ đối tượng có sẵn. Dấu `&` không sao chép Repository; thân constructor rỗng vì Service không tự nạp dữ liệu.

## 3. requireViewPermission() — helper private

### A. Tổng quan

Không có tham số; trả `const User&` của tài khoản đang đăng nhập. Yêu cầu mã `INVOICE_VIEW_ALL` cho Admin hoặc `INVOICE_VIEW_OWN` cho Staff; thiếu quyền hoặc vai trò sai ném ngoại lệ.

### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Yêu cầu đăng nhập
    |
    v
Xác định vai trò
    |
    v
Admin: kiểm tra INVOICE_VIEW_ALL
    |
    v
Staff: kiểm tra INVOICE_VIEW_OWN
    |
    v
Vai trò khác hoặc thiếu quyền: throw
    |
    v
Trả User hiện tại
```

### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1.**

```cpp
const User& InvoiceService::requireViewPermission() const
{
    const User& user = authService.requireLogin();
```

`requireLogin()` kiểm tra tài khoản còn hoạt động và trả tham chiếu `User`; thất bại sẽ ném ngoại lệ trước khi truy vấn hóa đơn.

**Bước 2.**

```cpp
    if (user.getRole() == UserRole::ADMIN)
    {
        authService.requirePermission("INVOICE_VIEW_ALL");
    }
    else if (user.getRole() == UserRole::STAFF)
    {
        authService.requirePermission("INVOICE_VIEW_OWN");
    }
    else
    {
        throw runtime_error("Vai tro khong duoc xem hoa don");
    }
```

Mỗi vai trò dùng mã quyền riêng qua RBAC đang nạp. `requirePermission()` có thể ném ngoại lệ; Admin không tự được cấp quyền nếu file quyền thiếu. Vai trò ngoài Admin/Staff bị từ chối rõ ràng.

**Bước 3.**

```cpp
    return user;
}
```

Trả tham chiếu đến `User` hiện có; không tạo bản sao tài khoản.

## 4. canViewInvoice() — helper private

### A. Tổng quan

Nhận `const Invoice*`, trả `bool` cho phạm vi một hóa đơn. Helper được gọi sau `requireViewPermission()`; Admin xem mọi hóa đơn, Staff chỉ xem hóa đơn có `staffId` bằng ID của mình.

### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Lấy tài khoản hiện tại
    |
    v
Invoice hoặc User rỗng: false
    |
    v
Admin: true
    |
    v
Staff: so sánh staffId với userId
    |
    v
Trả kết quả
```

### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1.**

```cpp
bool InvoiceService::canViewInvoice(const Invoice* invoice) const
{
    const User* user = authService.getCurrentUser();
    if (invoice == nullptr || user == nullptr)
    {
        return false;
    }
```

`getCurrentUser()` trả con trỏ rỗng nếu phiên không hợp lệ. Kiểm tra cả hai con trỏ trước khi dùng toán tử `->`; helper không sửa Invoice.

**Bước 2.**

```cpp
    if (user->getRole() == UserRole::ADMIN)
    {
        return true;
    }
    return user->getRole() == UserRole::STAFF &&
           invoice->getStaffId() == user->getUserId();
}
```

Admin đã qua kiểm tra RBAC ở caller nên được xem tất cả. Với Staff, so sánh ID người lập từ Invoice với ID tài khoản đang đăng nhập; `staffId` do GUI nhập không quyết định quyền.

## 5. findInvoiceById() — tra cứu theo mã

### A. Tổng quan

Nhận `invoiceId`; trả con trỏ `Invoice*` mượn hoặc `nullptr` nếu mã không tồn tại. Thiếu quyền, Repository chưa nạp hoặc hóa đơn ngoài phạm vi Staff đều ném ngoại lệ.

### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Kiểm tra đăng nhập và RBAC
    |
    v
Kiểm tra InvoiceRepository đã load
    |
    v
Tìm Invoice theo mã
    |
    v
Không có: nullptr
    |
    v
Kiểm tra phạm vi người lập
    |
    v
Ngoài phạm vi: throw
    |
    v
Trả con trỏ mượn
```

### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1.**

```cpp
Invoice* InvoiceService::findInvoiceById(const string& invoiceId)
{
    requireViewPermission();
    if (!invoiceRepo.isLoaded())
    {
        throw runtime_error("Chua nap du lieu hoa don");
    }
```

Quyền được kiểm tra trước khi đụng đến dữ liệu. `isLoaded()` ngăn dùng danh sách hóa đơn chưa nạp; trường hợp này ném lỗi thay vì coi như không tìm thấy.

**Bước 2.**

```cpp
    Invoice* invoice = invoiceRepo.findById(invoiceId);
    if (invoice == nullptr)
    {
        return nullptr;
    }
```

Repository trả con trỏ đến Invoice đang quản lý. Mã không có là kết quả tra cứu hợp lệ nên trả `nullptr`, không cấp phát đối tượng mới.

**Bước 3.**

```cpp
    if (!canViewInvoice(invoice))
    {
        throw runtime_error("Khong duoc xem hoa don cua nhan vien khac");
    }
    return invoice;
}
```

`canViewInvoice()` chặn Staff xem hóa đơn do người khác lập; trả con trỏ chỉ sau khi kiểm tra phạm vi. Caller không được `delete` hoặc sửa trực tiếp con trỏ này.

## 6. requireInvoice() — helper private

### A. Tổng quan

Nhận `invoiceId`, trả `Invoice*` tồn tại và được phép xem. Dùng cho các hàm cần tính toán; mã không tìm thấy ném `invalid_argument`, các lỗi quyền/tải dữ liệu từ `findInvoiceById()` được giữ nguyên.

### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Gọi findInvoiceById()
    |
    v
Không tìm thấy: throw
    |
    v
Trả Invoice*
```

### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1.**

```cpp
Invoice* InvoiceService::requireInvoice(const string& invoiceId)
{
    Invoice* invoice = findInvoiceById(invoiceId);
    if (invoice == nullptr)
    {
        throw invalid_argument("Khong tim thay hoa don: " + invoiceId);
    }
    return invoice;
}
```

Gọi API tra cứu chung để dùng lại kiểm tra quyền, phạm vi và trạng thái nạp. Các hàm trả trạng thái hoặc số tiền không có giá trị sentinel an toàn, nên mã không tồn tại phải ném ngoại lệ.

## 7. getInvoiceHistory() — lọc lịch sử

### A. Tổng quan

Nhận `[from, to)` ở định dạng `YYYY-MM-DDTHH:MM:SS`, `staffId` và `memberId` tùy chọn (chuỗi rỗng là không lọc); trả `vector<Invoice*>` mượn. Admin xem mọi khoảng; Staff chỉ liệt kê hóa đơn của mình trong ngày hiện tại.

### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Kiểm tra quyền và InvoiceRepository đã load
    |
    v
Kiểm tra hai mốc thời gian và from < to
    |
    v
Staff: giới hạn [đầu hôm nay, đầu ngày mai]
    |
    v
Staff chỉ được lọc ID của chính mình
    |
    v
Duyệt và kiểm tra từng Invoice
    |
    v
Áp dụng thời gian, phạm vi, nhân viên, thành viên
    |
    v
Sắp xếp createdAt giảm, invoiceId tăng khi trùng
    |
    v
Trả vector con trỏ
```

### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1.**

```cpp
vector<Invoice*> InvoiceService::getInvoiceHistory(const string& from,
                                                    const string& to,
                                                    const string& staffId,
                                                    const string& memberId)
{
    const User& user = requireViewPermission();
    if (!invoiceRepo.isLoaded())
    {
        throw runtime_error("Chua nap du lieu hoa don");
    }
    if (!Validator::isValidDateTime(from) ||
        !Validator::isValidDateTime(to) ||
        !DateTime::isBefore(from, to))
    {
        throw invalid_argument("Khoang thoi gian khong hop le");
    }
```

`Validator::isValidDateTime()` xác nhận định dạng; `DateTime::isBefore()` bảo đảm khoảng không rỗng. Thiếu quyền hoặc chưa nạp ném lỗi trước khi lọc, thay vì trả danh sách rỗng dễ gây hiểu sai.

**Bước 2.**

```cpp
    if (user.getRole() == UserRole::STAFF)
    {
        string todayStart = DateTime::getCurrentDateTime().substr(0, 10) + "T00:00:00";
        string tomorrowStart = DateTime::addMinutes(todayStart, 24 * 60);
        if (from < todayStart || to > tomorrowStart)
        {
            throw invalid_argument("Staff chi duoc liet ke hoa don trong ngay hien tai");
        }
        if (!staffId.empty() && staffId != user.getUserId())
        {
            throw runtime_error("Staff khong duoc loc hoa don cua nhan vien khac");
        }
    }

```

`substr(0, 10)` lấy phần ngày hiện tại; cộng 24 giờ để có mốc 00:00 ngày kế. Staff phải truyền khoảng nằm trong ngày này. Nếu chỉ định `staffId` khác ID đang đăng nhập, Service từ chối thay vì tin bộ lọc GUI.

**Bước 3.**

```cpp
    vector<Invoice*> result;
    for (int i = 0; i < invoiceRepo.count(); i++)
    {
        Invoice* invoice = invoiceRepo.getAt(i);
        if (invoice == nullptr ||
            !Validator::isValidDateTime(invoice->getCreatedAt()))
        {
            throw runtime_error("Du lieu hoa don khong nhat quan");
        }
        if (invoice->getCreatedAt() < from || invoice->getCreatedAt() >= to ||
            !canViewInvoice(invoice) ||
            (!staffId.empty() && invoice->getStaffId() != staffId) ||
            (!memberId.empty() && invoice->getMemberId() != memberId))
        {
            continue;
        }
        result.push_back(invoice);
    }
```

Duyệt danh sách đang nạp, phát hiện con trỏ hoặc `createdAt` sai. Điều kiện `createdAt >= to` thực hiện cận trên mở. `canViewInvoice()` áp dụng phạm vi độc lập với `staffId`; mỗi Invoice đạt mọi bộ lọc mới được đưa vào vector tạm, không sao chép Model.

**Bước 4.**

```cpp
    sort(result.begin(), result.end(), [](const Invoice* first, const Invoice* second)
    {
        if (first->getCreatedAt() != second->getCreatedAt())
        {
            return first->getCreatedAt() > second->getCreatedAt();
        }
        return first->getInvoiceId() < second->getInvoiceId();
    });
    return result;
}
```

`std::sort` chỉ sắp xếp vector kết quả, không đổi thứ tự Repository. Lambda là hàm so sánh ngắn: thời điểm mới hơn đứng trước; nếu bằng nhau thì mã hóa đơn nhỏ hơn đứng trước để thứ tự ổn định theo quy tắc.

## 8. getValidTickets() — helper private

### A. Tổng quan

Nhận `Invoice*` đã được `requireInvoice()` xác nhận; trả `vector<Ticket*>` thuộc hóa đơn. Hàm chỉ đọc dữ liệu; kiểu con trỏ này cần thiết vì `Invoice::getTicketAt()` hiện chưa có overload `const`. Ném ngoại lệ nếu TicketRepository chưa nạp, không có vé, liên kết sai, trạng thái/tiền hoàn sai hoặc tổng tiền vé khác hóa đơn.

### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Kiểm tra TicketRepository đã load
    |
    v
Lấy vé theo invoiceId
    |
    v
Danh sách rỗng hoặc số vé lệch: throw
    |
    v
Kiểm tra ID, liên kết, suất và số tiền từng vé
    |
    v
Đối chiếu vé trong danh sách của Invoice
    |
    v
Kiểm tra trạng thái SOLD/CANCELLED và tiền hoàn
    |
    v
Cộng paidAmount; so với totalAmount
    |
    v
Trả danh sách vé hợp lệ
```

### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1.**

```cpp
vector<Ticket*> InvoiceService::getValidTickets(Invoice* invoice)
{
    if (!ticketRepo.isLoaded())
    {
        throw runtime_error("Chua nap du lieu ve");
    }
    vector<Ticket*> tickets = ticketRepo.getByInvoice(invoice->getInvoiceId());
    if (tickets.empty() || invoice->getTicketCount() != static_cast<int>(tickets.size()))
    {
        throw runtime_error("Hoa don khong co ve hoac lien ket ve khong nhat quan");
    }
```

`getByInvoice()` đọc vé đang nằm trong Repository. Hóa đơn phải có ít nhất một vé; `static_cast<int>` đổi `size_t` của vector sang `int` để so với `getTicketCount()`. Hai danh sách lệch số lượng là dữ liệu bất nhất.

**Bước 2.**

```cpp
    double paidTotal = 0;
    for (Ticket* ticket : tickets)
    {
        if (ticket == nullptr || ticket->getInvoice() != invoice ||
            ticket->getInvoiceId() != invoice->getInvoiceId() ||
            ticket->getShowtimeId() != invoice->getShowtimeId() ||
            !isfinite(ticket->getPaidAmount()) || ticket->getPaidAmount() < 0 ||
            !isfinite(ticket->getRefundAmount()) || ticket->getRefundAmount() < 0)
        {
            throw runtime_error("Du lieu ve khong nhat quan");
        }
```

Mỗi vé phải liên kết đúng Invoice, có cùng ID hóa đơn và suất, tiền trả/hoàn là số hữu hạn không âm. `isfinite()` loại NaN/vô cực trước phép tính; nếu sai, ném lỗi trước khi dùng số tiền.

**Bước 3.**

```cpp
        bool linked = false;
        for (int i = 0; i < invoice->getTicketCount(); i++)
        {
            if (invoice->getTicketAt(i) == ticket)
            {
                linked = true;
                break;
            }
        }
        if (!linked)
        {
            throw runtime_error("Ve khong nam trong danh sach cua hoa don");
        }
```

Kiểm tra từng con trỏ Ticket thực sự nằm trong danh sách liên kết của Invoice. Dù con trỏ `ticket->getInvoice()` đúng, thiếu liên kết ở chiều ngược lại vẫn là trạng thái sai. `break` dừng vòng trong khi đã tìm thấy vé.

**Bước 4.**

```cpp
        if (ticket->getStatus() == TicketStatus::SOLD)
        {
            if (ticket->getRefundAmount() != 0)
            {
                throw runtime_error("Ve dang ban co tien hoan");
            }
        }
        else if (ticket->getStatus() == TicketStatus::CANCELLED)
        {
            if (fabs(ticket->getRefundAmount() - ticket->getPaidAmount()) > 0.000001)
            {
                throw runtime_error("Tien hoan ve khong nhat quan");
            }
        }
        else
        {
            throw runtime_error("Trang thai ve khong hop le");
        }
        paidTotal += ticket->getPaidAmount();
    }
```

Vé SOLD không được có tiền hoàn; vé CANCELLED phải hoàn đúng `paidAmount`. `fabs()` lấy độ lệch tuyệt đối, ngưỡng nhỏ cho sai số `double`. Enum khác hai trạng thái hợp lệ gây lỗi. `paidTotal` cộng giá thực trả gốc, kể cả vé đã hủy, để đối chiếu tổng hóa đơn lịch sử.

**Bước 5.**

```cpp
    if (!isfinite(invoice->getTotalAmount()) || invoice->getTotalAmount() < 0 ||
        !isfinite(paidTotal) ||
        fabs(paidTotal - invoice->getTotalAmount()) > 0.00001)
    {
        throw runtime_error("Tong tien hoa don khong khop voi cac ve");
    }
    return tickets;
}
```

Tổng tiền hóa đơn phải hữu hạn, không âm và bằng tổng `paidAmount` của các vé trong sai số nhỏ. Lỗi bị ném thay vì trả một trạng thái hoặc số tiền có vẻ hợp lệ. Hàm chỉ trả con trỏ mượn, không thay đổi Ticket/Invoice.

## 9. getInvoiceStatus() — trạng thái hóa đơn

### A. Tổng quan

Nhận `invoiceId`, trả `InvoiceStatus`: `ACTIVE` nếu mọi vé SOLD, `CANCELLED` nếu mọi vé CANCELLED, còn lại `PARTIALLY_CANCELLED`. Hóa đơn/vé lỗi ném ngoại lệ; trạng thái không lưu vào TXT.

### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Lấy Invoice hợp lệ và vé đã đối chiếu
    |
    v
Đếm SOLD và CANCELLED
    |
    v
Không có vé hủy: ACTIVE
    |
    v
Không còn vé bán: CANCELLED
    |
    v
Còn cả hai: PARTIALLY_CANCELLED
```

### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1.**

```cpp
InvoiceStatus InvoiceService::getInvoiceStatus(const string& invoiceId)
{
    Invoice* invoice = requireInvoice(invoiceId);
    vector<Ticket*> tickets = getValidTickets(invoice);
    int soldCount = 0;
    int cancelledCount = 0;
    for (Ticket* ticket : tickets)
    {
        if (ticket->getStatus() == TicketStatus::SOLD)
        {
            soldCount++;
        }
        else
        {
            cancelledCount++;
        }
    }
```

`requireInvoice()` bảo đảm quyền và mã tồn tại; `getValidTickets()` loại danh sách rỗng hoặc bất nhất. Vòng lặp chỉ đếm hai trạng thái đã xác thực nên không cần một giá trị trạng thái dự phòng.

**Bước 2.**

```cpp
    if (cancelledCount == 0)
    {
        return InvoiceStatus::ACTIVE;
    }
    if (soldCount == 0)
    {
        return InvoiceStatus::CANCELLED;
    }
    return InvoiceStatus::PARTIALLY_CANCELLED;
}
```

Nếu không có vé hủy thì mọi vé còn SOLD; nếu không còn vé SOLD thì mọi vé đã hủy. Trường hợp còn cả hai trả trạng thái hủy một phần. Enum được tính tại lúc gọi, không thêm trường vào Model/TXT.

## 10. getRefundAmount() — tổng tiền hoàn

### A. Tổng quan

Nhận `invoiceId`, trả tổng `refundAmount` của Ticket CANCELLED bằng `double` VND. Vé SOLD không cộng; dữ liệu bất nhất ném ngoại lệ. Hàm không dùng bảng giá hiện tại và không ghi TXT.

### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Lấy Invoice và danh sách vé hợp lệ
    |
    v
Cộng refundAmount của vé CANCELLED
    |
    v
Kiểm tra tổng hoàn hữu hạn, không vượt tổng gốc
    |
    v
Trả tiền hoàn
```

### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1.**

```cpp
double InvoiceService::getRefundAmount(const string& invoiceId)
{
    Invoice* invoice = requireInvoice(invoiceId);
    vector<Ticket*> tickets = getValidTickets(invoice);
    double refund = 0;
    for (Ticket* ticket : tickets)
    {
        if (ticket->getStatus() == TicketStatus::CANCELLED)
        {
            refund += ticket->getRefundAmount();
        }
    }
```

Hàm dùng snapshot tiền hoàn đã ghi trên từng vé hủy; vé còn SOLD không đóng góp vào tổng hoàn. `requireInvoice()` kiểm tra quyền; `getValidTickets()` kiểm tra liên kết và quy tắc hoàn từng vé.

**Bước 2.**

```cpp
    if (!isfinite(refund) || refund > invoice->getTotalAmount() + 0.00001)
    {
        throw runtime_error("Tong tien hoan vuot tien hoa don");
    }
    return refund;
}
```

Cộng nhiều `double` có thể tạo sai số nhỏ, nên so sánh với tổng gốc bằng biên `0.00001`. Tổng không hữu hạn hoặc vượt tiền hóa đơn là lỗi, không được trả `0` để che giấu.

## 11. getRemainingAmount() — tiền còn lại

### A. Tổng quan

Nhận `invoiceId`, trả `invoice.totalAmount - refundAmount` bằng VND. Không sửa tổng tiền gốc; dữ liệu bất nhất ném ngoại lệ.

### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Lấy Invoice hợp lệ
    |
    v
Tính tiền hoàn bằng getRefundAmount()
    |
    v
Lấy tổng gốc trừ tiền hoàn
    |
    v
Giá trị không hữu hạn hoặc âm đáng kể: throw
    |
    v
Làm tròn sai số âm rất nhỏ về 0
    |
    v
Trả tiền còn lại
```

### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1.**

```cpp
double InvoiceService::getRemainingAmount(const string& invoiceId)
{
    Invoice* invoice = requireInvoice(invoiceId);
    double refund = getRefundAmount(invoiceId);
    double remaining = invoice->getTotalAmount() - refund;
    if (!isfinite(remaining) || remaining < -0.00001)
    {
        throw runtime_error("Tien con lai cua hoa don khong hop le");
    }
```

`getRefundAmount()` dùng lại toàn bộ kiểm tra quyền và dữ liệu vé. Hóa đơn giữ nguyên tổng tiền gốc để tra lịch sử. Nếu kết quả không hữu hạn hoặc âm vượt dung sai thì ném lỗi.

**Bước 2.**

```cpp
    if (remaining < 0)
    {
        remaining = 0;
    }
    return remaining;
}
```

Sai số `double` sát 0 có thể tạo giá trị âm rất nhỏ sau phép trừ; nhánh này đưa đúng trường hợp đó về 0. Mọi âm đáng kể đã bị từ chối ở bước trước, nên đây không phải giá trị thay thế cho lỗi.

## 12. Kiểm thử và giới hạn

`tests/test_invoice_service.cpp` dùng TXT tạm và đã chạy thực tế: **37 PASS, 0 FAIL**. Test bao phủ RBAC, phạm vi Staff, lọc và sắp xếp, ba trạng thái, hoàn một phần/toàn bộ, Repository chưa nạp, dữ liệu bất nhất và xác nhận truy vấn không ghi TXT. Service chỉ đọc dữ liệu đã nạp; nó không xử lý transaction đa file hay tự sửa dữ liệu lịch sử lỗi.
