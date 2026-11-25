# ReportService — thống kê và báo cáo (Bước 11)

## 1. Tổng quan

`ReportService` chỉ đọc dữ liệu từ Invoice, Ticket, Showtime và Movie đã nạp. Không tạo `struct` báo cáo, Model, Repository hay TXT mới. Constructor giữ tham chiếu; các kết quả `Movie*` là con trỏ mượn. Bảy API public nằm trong `src/business/ReportService.h/.cpp`.

Mỗi nghiệp vụ yêu cầu Admin và mã RBAC `REPORT_VIEW`. Khoảng `[from,to)` dùng DateTime đầy đủ; GUI có thể đổi ngày/tháng được chọn thành hai mốc rồi gọi cùng API. `getTotalSales()` theo `Invoice::createdAt`, `getTotalRefund()` theo `Ticket::cancelledAt`; hiệu hai tổng có thể âm. Đếm vé bán theo ngày lập hóa đơn kể cả vé đã hủy; top phim chỉ tính vé hiện còn SOLD theo kỳ bán. Phim không bán không xuất hiện trong top. Phim không tồn tại ném `invalid_argument` ở `getMovieRevenue()`.

Tiền lấy từ snapshot lịch sử trên Invoice/Ticket, không tính lại theo Pricing. Repository chưa nạp, liên kết hoặc số tiền từng bản ghi bất hợp lệ ném ngoại lệ; kỳ hợp lệ không có dữ liệu trả `0` hoặc vector rỗng. **Giới hạn code hiện tại:** helper kiểm tra vé chưa đối chiếu tổng `paidAmount` của cả hóa đơn với `Invoice::totalAmount`. Top phim dùng `std::sort()` và hàm so sánh riêng, không dùng Max Heap.

| API public | Giá trị trả về |
| --- | --- |
| `getTotalSales(from,to)` | Tổng tiền bán theo ngày lập hóa đơn |
| `getTotalRefund(from,to)` | Tổng tiền hoàn theo ngày hủy |
| `getActualRevenue(from,to)` | Tiền bán trừ tiền hoàn |
| `getSoldTicketCount(from,to)` | Số Ticket tạo khi bán trong kỳ |
| `getCancelledTicketCount(from,to)` | Số Ticket hủy trong kỳ |
| `getMovieRevenue(movieId,from,to)` | Doanh thu phim từ vé còn SOLD |
| `getTopMovies(from,to,limit)` | Danh sách phim xếp theo doanh thu |

## 2. ReportService() — khởi tạo

### A. Tổng quan

Nhận 5 tham chiếu Repository/AuthService, không sở hữu, không load hoặc save; constructor không trả giá trị.

### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Nhận các tham chiếu
    |
    v
Khởi tạo thành viên
    |
    v
Kết thúc
```

### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1.**

```cpp
ReportService::ReportService(InvoiceRepository& invoiceRepo,
                             TicketRepository& ticketRepo,
                             MovieRepository& movieRepo,
                             ShowtimeRepository& showtimeRepo,
                             AuthService& authService)
    : invoiceRepo(invoiceRepo),
      ticketRepo(ticketRepo),
      movieRepo(movieRepo),
      showtimeRepo(showtimeRepo),
      authService(authService)
{
}
```

Danh sách sau dấu `:` khởi tạo từng thành viên tham chiếu bằng đối tượng caller truyền vào. Service không sao chép dữ liệu và không tự đọc TXT; các đối tượng này phải sống lâu hơn Service.

## 3. requireReportAccess() — helper private

### A. Tổng quan

Không có tham số, trả `void`; chỉ Admin có mã quyền `REPORT_VIEW` được tiếp tục. Thiếu quyền ném ngoại lệ.

### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Yêu cầu vai trò Admin
    |
    v
Kiểm tra mã REPORT_VIEW
    |
    v
Thiếu quyền: throw
    |
    v
Tiếp tục
    |
    v
Kết thúc
```

### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1.**

```cpp
void ReportService::requireReportAccess() const
{
    authService.requireAdmin();
    authService.requirePermission("REPORT_VIEW");
}
```

`requireAdmin()` chặn Staff/khách trước khi đọc dữ liệu. `requirePermission()` kiểm tra bảng RBAC đang nạp; vai trò Admin không tự động có quyền nếu file thiếu cặp `REPORT_VIEW`.

## 4. validateRange() — helper private

### A. Tổng quan

Nhận `from`, `to`; trả `void`. Yêu cầu hai mốc DateTime hợp lệ và `from < to`; sai ném `invalid_argument`.

### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Kiểm tra định dạng from và to
    |
    v
Kiểm tra from < to
    |
    v
Sai: throw
    |
    v
Tiếp tục
    |
    v
Kết thúc
```

### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1.**

```cpp
void ReportService::validateRange(const string& from, const string& to) const
{
    if (!Validator::isValidDateTime(from) ||
        !Validator::isValidDateTime(to) ||
        !DateTime::isBefore(from, to))
    {
        throw invalid_argument("Khoang thoi gian bao cao khong hop le");
    }
}
```

`Validator` loại chuỗi ngày giờ sai; `DateTime::isBefore()` xác nhận khoảng không rỗng. Mốc lọc sau đó dùng `[from,to)`, nên `to` không được tính.

## 5. validateInvoice() — helper private

### A. Tổng quan

Nhận `const Invoice*`, trả `void`; loại con trỏ rỗng, thời điểm lập sai và tổng tiền âm/không hữu hạn bằng `runtime_error`.

### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Kiểm tra con trỏ Invoice
    |
    v
Kiểm tra createdAt và totalAmount
    |
    v
Sai: throw
    |
    v
Tiếp tục
    |
    v
Kết thúc
```

### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1.**

```cpp
void ReportService::validateInvoice(const Invoice* invoice) const
{
    if (invoice == nullptr ||
        !Validator::isValidDateTime(invoice->getCreatedAt()) ||
        !isfinite(invoice->getTotalAmount()) ||
        invoice->getTotalAmount() < 0)
    {
        throw runtime_error("Du lieu hoa don bao cao khong hop le");
    }
}
```

Kiểm tra `nullptr` trước khi dùng `->`. `isfinite()` loại NaN/vô cực để các phép cộng không biến thành kết quả sai; tổng tiền hóa đơn lịch sử phải không âm.

## 6. requireInvoiceForTicket() — helper private

### A. Tổng quan

Nhận `Ticket*`, trả `Invoice*` liên kết. Kiểm tra tiền của vé, quan hệ Ticket–Invoice–Showtime và dữ liệu hủy; bất nhất ở các kiểm tra này ném `runtime_error`. Hiện chưa đối chiếu danh sách vé hoặc tổng tiền của toàn hóa đơn.

### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Kiểm tra Ticket và số tiền
    |
    v
Tra Invoice/Showtime và kiểm tra liên kết
    |
    v
Kiểm tra createdAt và totalAmount của Invoice
    |
    v
Kiểm tra SOLD/CANCELLED và dữ liệu hủy
    |
    v
Sai ở bất kỳ bước: throw
    |
    v
Trả Invoice
    |
    v
Kết thúc
```

### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1.**

```cpp
Invoice* ReportService::requireInvoiceForTicket(Ticket* ticket)
{
    // kiểm tra ticket và số tiền
    if (ticket == nullptr ||
        !isfinite(ticket->getPaidAmount()) ||
        ticket->getPaidAmount() < 0 ||
        !isfinite(ticket->getRefundAmount()) ||
        ticket->getRefundAmount() < 0)
    {
        throw runtime_error("Du lieu ve bao cao khong hop le");
    }
```

Kiểm tra con trỏ Ticket và các khoản tiền hữu hạn, không âm trước khi truy cập. Nếu dữ liệu sai, hàm ném `runtime_error` thay vì bỏ vé khỏi thống kê.

**Bước 2.**

```cpp
    Invoice* invoice = invoiceRepo.findById(ticket->getInvoiceId());
    Showtime* showtime = showtimeRepo.findById(ticket->getShowtimeId());
    // kiểm tra liên kết
    if (invoice == nullptr || showtime == nullptr ||
        ticket->getInvoice() != invoice ||
        ticket->getShowtime() != showtime ||
        invoice->getShowtimeId() != ticket->getShowtimeId())
    {
        throw runtime_error("Lien ket ve, hoa don hoac suat chieu khong hop le");
    }

    validateInvoice(invoice);
```

Tra Invoice và Showtime bằng ID lưu trên vé, đối chiếu các con trỏ liên kết và `showtimeId`; sau đó kiểm tra ngày lập và tổng tiền Invoice bằng `validateInvoice()`. Thiếu liên kết hoặc sai tham chiếu đều ném lỗi.

**Bước 3.**

```cpp
    // Kiểm tra vé đang SOLD
    if (ticket->getStatus() == TicketStatus::SOLD)
    {
        if (ticket->getRefundAmount() != 0 ||
            !ticket->getCancelledAt().empty() ||
            !ticket->getCancelledBy().empty() ||
            !ticket->getCancelReason().empty())
        {
            throw runtime_error("Ve dang ban co du lieu huy");
        }
    }
    else if (ticket->getStatus() == TicketStatus::CANCELLED)
    {
        if (!Validator::isValidDateTime(ticket->getCancelledAt()) ||
            !Validator::isValidId(ticket->getCancelledBy(), "USR") ||
            !Validator::isValidText(ticket->getCancelReason()) ||
            !DateTime::isBefore(ticket->getCancelledAt(), showtime->getStartAt()) ||
            DateTime::isBefore(ticket->getCancelledAt(), invoice->getCreatedAt()) ||
            fabs(ticket->getRefundAmount() - ticket->getPaidAmount()) > 0.000001)
        {
            throw runtime_error("Du lieu huy ve khong hop le");
        }
    }
    else
    {
        throw runtime_error("Trang thai ve khong hop le");
    }

    return invoice;
}
```

Vé SOLD không được có tiền hoàn hay dữ liệu hủy. Vé CANCELLED cần ngày, người hủy, lý do hợp lệ; thời điểm hủy trước suất, không trước ngày bán, tiền hoàn bằng `paidAmount` trong sai số nhỏ. Hàm trả Invoice liên kết. Code hiện tại **không** đối chiếu số vé và tổng `paidAmount` của cả hóa đơn.
## 7. calculateMovieRevenue() — helper private

### A. Tổng quan

Nhận `movieId`, `[from,to)` và `bool& hasSoldTicket`; trả doanh thu phim. Cờ đầu ra cho biết có vé SOLD trong kỳ, kể cả vé có giá 0.

### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Đặt cờ false và tổng 0
    |
    v
Duyệt vé, xác nhận Invoice/Showtime/Movie
    |
    v
Chọn vé SOLD đúng phim và kỳ bán
    |
    v
Cộng paidAmount, kiểm tra số hữu hạn
    |
    v
Trả tổng
    |
    v
Kết thúc
```

### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1.**

```cpp
double ReportService::calculateMovieRevenue(const string& movieId, const string& from, const string& to, bool& hasSoldTicket)
{
    hasSoldTicket = false;
    double revenue = 0;

    for (int i = 0; i < ticketRepo.count(); i++)
    {
        Ticket* ticket = ticketRepo.getAt(i);
        Invoice* invoice = requireInvoiceForTicket(ticket);

        const Showtime* showtime = ticket->getShowtime();
        Movie* movie = movieRepo.findById(showtime->getMovieId());

        if (movie == nullptr || showtime->getMovie() != movie)
        {
            throw runtime_error("Suat chieu khong lien ket phim hop le");
        }
```

Đặt cờ `hasSoldTicket` về `false` và tổng về 0. Duyệt từng vé, dùng `requireInvoiceForTicket()` xác nhận Invoice/Showtime, rồi tra Movie theo suất; phim thiếu hoặc liên kết sai ném lỗi.

**Bước 2.**

```cpp
        if (ticket->getStatus() != TicketStatus::SOLD ||
            showtime->getMovieId() != movieId ||
            invoice->getCreatedAt() < from ||
            invoice->getCreatedAt() >= to)
        {
            continue;
        }
```

Các vé không còn SOLD, khác phim hoặc hóa đơn lập ngoài `[from, to)` được bỏ qua bằng `continue`. Điều kiện dùng ngày **bán** của Invoice, không dùng ngày hủy hay ngày suất chiếu.

**Bước 3.**

```cpp
        hasSoldTicket = true;
        revenue += ticket->getPaidAmount();

        if (!isfinite(revenue))
        {
            throw runtime_error("Doanh thu phim khong hop le");
        }
    }

    return revenue;
}
```

Với vé phù hợp, đặt cờ để top phim vẫn nhận diện vé có giá 0, cộng `paidAmount` lịch sử và chặn tổng không hữu hạn. Trả doanh thu đã cộng.
## 8. compareMovieRevenue() — helper so sánh riêng

### A. Tổng quan

Nhận hai `pair<Movie*, double>`, trả `bool` cho `std::sort()`: doanh thu giảm dần, hòa thì `movieId` tăng dần.

### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
So sánh doanh thu
    |
    v
Khác nhau: doanh thu lớn đứng trước
    |
    v
Bằng nhau: movieId nhỏ đứng trước
    |
    v
Kết thúc
```

### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1.**

```cpp
bool ReportService::compareMovieRevenue(const pair<Movie*, double>& movieA,
                                        const pair<Movie*, double>& movieB)
{
    if (movieA.second != movieB.second)
    {
        return movieA.second > movieB.second;
    }
    return movieA.first->getMovieId() < movieB.first->getMovieId();
}
```

`pair.second` chứa doanh thu: giá trị lớn đứng trước. Nếu bằng nhau, so `movieId` tăng dần. Hàm so sánh riêng được truyền cho `std::sort()`; không dùng Max Heap.
## 9. getTotalSales()

### A. Tổng quan

Nhận `[from,to)`, trả `double` tổng `Invoice::totalAmount` theo `createdAt`. Chỉ cần InvoiceRepository đã load; không trừ hoàn tiền.

### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Kiểm tra Admin/RBAC và khoảng
    |
    v
Kiểm tra InvoiceRepository
    |
    v
Duyệt hóa đơn, xác nhận dữ liệu
    |
    v
Nếu createdAt thuộc kỳ, cộng tổng gốc
    |
    v
Trả tổng
    |
    v
Kết thúc
```

### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1.**

```cpp
double ReportService::getTotalSales(const string& from, const string& to)
{
    requireReportAccess();
    validateRange(from, to);
    if (!invoiceRepo.isLoaded())
    {
        throw runtime_error("Chua nap du lieu hoa don");
    }
```

Quyền và thời gian được kiểm tra trước khi đọc dữ liệu. Repository chưa load là lỗi, không trả `0` vì dễ hiểu nhầm là kỳ không bán.

**Bước 2.**

```cpp
    double total = 0;
    for (int i = 0; i < invoiceRepo.count(); i++)
    {
        Invoice* invoice = invoiceRepo.getAt(i);
        validateInvoice(invoice);
        if (invoice->getCreatedAt() >= from && invoice->getCreatedAt() < to)
        {
            total += invoice->getTotalAmount();
            if (!isfinite(total))
            {
                throw runtime_error("Tong tien ban khong hop le");
            }
        }
    }
    return total;
}
```

Mỗi Invoice được kiểm tra kể cả khi nằm ngoài kỳ, tránh che giấu dữ liệu lỗi. Điều kiện `createdAt < to` là cận trên mở; tổng vượt số hữu hạn ném lỗi.

## 10. getTotalRefund()

### A. Tổng quan

Nhận `[from,to)`, trả `double` tổng tiền hoàn theo `Ticket::cancelledAt`, không theo ngày lập Invoice.

### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Kiểm tra quyền, thời gian và Repository
    |
    v
Duyệt/kiểm tra từng Ticket
    |
    v
Chọn vé CANCELLED có cancelledAt trong kỳ
    |
    v
Cộng refundAmount
    |
    v
Trả tổng
    |
    v
Kết thúc
```

### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1.**

```cpp
double ReportService::getTotalRefund(const string& from,
                                     const string& to)
{
    requireReportAccess();
    validateRange(from, to);
    if (!invoiceRepo.isLoaded() || !ticketRepo.isLoaded() ||
        !showtimeRepo.isLoaded())
    {
        throw runtime_error("Chua nap du lieu hoa don, ve hoac suat chieu");
    }
```

Kiểm tra quyền `REPORT_VIEW` cho Admin, khoảng thời gian hợp lệ và Invoice/Ticket/Showtime Repository đã nạp. Thiếu dữ liệu ném `runtime_error`.

**Bước 2.**

```cpp
    double totalRefund = 0;
    for (int i = 0; i < ticketRepo.count(); i++)
    {
        Ticket* ticket = ticketRepo.getAt(i);
        requireInvoiceForTicket(ticket);
        if (ticket->getStatus() != TicketStatus::CANCELLED ||
            ticket->getCancelledAt() < from ||
            ticket->getCancelledAt() >= to)
        {
            continue;
        }
        totalRefund += ticket->getRefundAmount();
        if (!isfinite(totalRefund))
        {
            throw runtime_error("Tong tien hoan khong hop le");
        }
    }
    return totalRefund;
}
```

Duyệt vé và xác thực qua helper; vé khác `CANCELLED` hoặc hủy ngoài `[from,to)` được bỏ qua. Cộng `refundAmount` theo ngày hủy, kiểm tra tổng hữu hạn rồi trả kết quả.
## 11. getActualRevenue()

### A. Tổng quan

Nhận `[from,to)`, trả tiền bán trừ tiền hoàn; kết quả có thể âm. Lỗi từ hai hàm thành phần được giữ nguyên.

### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Lấy tổng bán
    |
    v
Lấy tổng hoàn
    |
    v
Lấy hiệu và kiểm tra hữu hạn
    |
    v
Trả doanh thu thực tế
    |
    v
Kết thúc
```

### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1.**

```cpp
double ReportService::getActualRevenue(const string& from, const string& to)
{
    double sales = getTotalSales(from, to);
    double refund = getTotalRefund(from, to);
    double revenue = sales - refund;
    if (!isfinite(revenue))
    {
        throw runtime_error("Doanh thu thuc te khong hop le");
    }
    return revenue;
}
```

Tái dùng hai phép thống kê theo thời điểm riêng: bán theo `createdAt`, hoàn theo `cancelledAt`. Doanh thu ngày có thể âm khi hoàn vé bán trước đó; chỉ NaN/vô cực là lỗi.

## 12. getSoldTicketCount()

### A. Tổng quan

Nhận `[from,to)`, trả `int` số vé phát sinh từ hóa đơn lập trong kỳ, gồm cả vé sau đó đã hủy; ghế đôi vẫn là một Ticket.

### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Kiểm tra quyền, khoảng và Repository
    |
    v
Duyệt Ticket, lấy Invoice hợp lệ
    |
    v
createdAt thuộc kỳ: tăng bộ đếm
    |
    v
Chặn tràn int
    |
    v
Trả số vé
    |
    v
Kết thúc
```

### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1.**

```cpp
int ReportService::getSoldTicketCount(const string& from,
                                      const string& to)
{
    requireReportAccess();
    validateRange(from, to);
    if (!invoiceRepo.isLoaded() || !ticketRepo.isLoaded() ||
        !showtimeRepo.isLoaded())
    {
        throw runtime_error("Chua nap du lieu hoa don, ve hoac suat chieu");
    }
```

Kiểm tra quyền, khoảng và ba Repository cần dùng; dữ liệu chưa nạp không được hiểu là không có vé.

**Bước 2.**

```cpp
    int soldCount = 0;
    for (int i = 0; i < ticketRepo.count(); i++)
    {
        Ticket* ticket = ticketRepo.getAt(i);
        Invoice* invoice = requireInvoiceForTicket(ticket);
        if (invoice->getCreatedAt() < from ||
            invoice->getCreatedAt() >= to)
        {
            continue;
        }
        if (soldCount == numeric_limits<int>::max())
        {
            throw runtime_error("So ve ban vuot gioi han");
        }
        soldCount++;
    }
    return soldCount;
}
```

Mỗi Ticket được đối chiếu Invoice. Hóa đơn lập ngoài kỳ bị bỏ qua; Ticket trong kỳ được đếm kể cả hiện đã hủy. Chặn tràn `int` trước khi tăng bộ đếm.
## 13. getCancelledTicketCount()

### A. Tổng quan

Nhận `[from,to)`, trả `int` số Ticket CANCELLED có thời điểm hủy trong kỳ, độc lập ngày bán.

### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Kiểm tra quyền, khoảng và Repository
    |
    v
Duyệt và xác thực Ticket
    |
    v
CANCELLED đúng kỳ: tăng bộ đếm
    |
    v
Chặn tràn int
    |
    v
Trả số vé hủy
    |
    v
Kết thúc
```

### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1.**

```cpp
int ReportService::getCancelledTicketCount(const string& from,
                                           const string& to)
{
    requireReportAccess();
    validateRange(from, to);
    if (!invoiceRepo.isLoaded() || !ticketRepo.isLoaded() ||
        !showtimeRepo.isLoaded())
    {
        throw runtime_error("Chua nap du lieu hoa don, ve hoac suat chieu");
    }
```

Kiểm tra quyền, khoảng và Repository; nếu thiếu dữ liệu ném lỗi.

**Bước 2.**

```cpp
    int cancelledCount = 0;
    for (int i = 0; i < ticketRepo.count(); i++)
    {
        Ticket* ticket = ticketRepo.getAt(i);
        requireInvoiceForTicket(ticket);
        if (ticket->getStatus() != TicketStatus::CANCELLED ||
            ticket->getCancelledAt() < from ||
            ticket->getCancelledAt() >= to)
        {
            continue;
        }
        if (cancelledCount == numeric_limits<int>::max())
        {
            throw runtime_error("So ve huy vuot gioi han");
        }
        cancelledCount++;
    }

    return cancelledCount;
}
```

Duyệt và xác thực từng Ticket; chỉ tăng đếm khi `CANCELLED` và `cancelledAt` nằm trong `[from,to)`. `continue` bỏ vé không đạt; kiểm tra tràn `int` trước phép tăng.
## 14. getMovieRevenue()

### A. Tổng quan

Nhận `movieId` và `[from,to)`; trả `double` theo vé còn SOLD thuộc kỳ bán. Phim hợp lệ không bán trả `0`; mã không tồn tại ném `invalid_argument`.

### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Kiểm tra quyền, khoảng và bốn Repository
    |
    v
Tìm movieId, không có: throw
    |
    v
Gọi helper tính doanh thu
    |
    v
Trả số tiền
    |
    v
Kết thúc
```

### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1.**

```cpp
double ReportService::getMovieRevenue(const string& movieId,
                                      const string& from,
                                      const string& to)
{
    requireReportAccess();
    validateRange(from, to);
    if (!invoiceRepo.isLoaded() || !ticketRepo.isLoaded() ||
        !showtimeRepo.isLoaded() || !movieRepo.isLoaded())
    {
        throw runtime_error("Chua nap du lieu bao cao phim");
    }
    if (movieRepo.findById(movieId) == nullptr)
    {
        throw invalid_argument("Khong tim thay phim: " + movieId);
    }
    bool hasSoldTicket = false;
    return calculateMovieRevenue(movieId, from, to, hasSoldTicket);
}
```

Các nguồn Invoice/Ticket/Showtime/Movie đều cần đã load. Phim không có trong Repository là đầu vào sai; phim có thật nhưng không có vé hợp lệ sẽ nhận `0` từ helper.

## 15. getTopMovies()

### A. Tổng quan

Nhận `[from,to)` và `limit > 0`; trả `vector<Movie*>` mượn, chỉ gồm phim có vé SOLD trong kỳ, xếp doanh thu giảm và `movieId` tăng khi hòa.

### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Kiểm tra quyền, khoảng, limit và Repository
    |
    v
Duyệt phim, tính doanh thu từng phim
    |
    v
Giữ phim có vé SOLD trong kỳ
    |
    v
std::sort bằng hàm so sánh riêng
    |
    v
Lấy tối đa limit con trỏ Movie
    |
    v
Trả vector
    |
    v
Kết thúc
```

### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1.**

```cpp
vector<Movie*> ReportService::getTopMovies(const string& from, const string& to, int limit)
{
    requireReportAccess();
    validateRange(from, to);
    if (limit <= 0)
    {
        throw invalid_argument("Gioi han top phim phai duong");
    }
    if (!invoiceRepo.isLoaded() || !ticketRepo.isLoaded() ||
        !showtimeRepo.isLoaded() || !movieRepo.isLoaded())
    {
        throw runtime_error("Chua nap du lieu bao cao phim");
    }
```

Kiểm tra quyền Admin, khoảng thời gian, `limit > 0` và đủ bốn Repository. Đầu vào sai ném `invalid_argument`; chưa nạp ném `runtime_error`.

**Bước 2.**

```cpp
    vector<pair<Movie*, double>> ranking;
    for (int i = 0; i < movieRepo.count(); i++)
    {
        Movie* movie = movieRepo.getAt(i);
        if (movie == nullptr)
        {
            throw runtime_error("Du lieu phim khong hop le");
        }
        bool hasSoldTicket = false;
        double revenue = calculateMovieRevenue(movie->getMovieId(), from, to, hasSoldTicket);
        if (hasSoldTicket)
        {
            ranking.push_back(make_pair(movie, revenue));
        }
    }
```

Duyệt Movie từ Repository; với từng phim gọi **`calculateMovieRevenue()`** và chỉ thêm `pair<Movie*, double>` khi cờ `hasSoldTicket` là true. Đây là danh sách tạm, không tạo Model mới.

**Bước 3.**

```cpp
    sort(ranking.begin(), ranking.end(), compareMovieRevenue);
    vector<Movie*> result;
    for (int i = 0; i < static_cast<int>(ranking.size()) && i < limit; i++)
    {
        result.push_back(ranking[i].first);
    }
    return result;
}
```

`std::sort()` gọi `compareMovieRevenue()` để xếp giảm theo doanh thu, hòa theo ID tăng. Sau đó lấy tối đa `limit` con trỏ Movie; kết quả không sở hữu đối tượng.
## 16. Kiểm thử và giới hạn

`tests/test_report_service.cpp` dùng TXT tạm. **Lần chạy lại trên code hiện tại: 31 PASS, 1 FAIL**; ca `invoice total inconsistent with tickets rejected` thất bại vì `requireInvoiceForTicket()` không còn đối chiếu tổng tiền vé với `Invoice::totalAmount`. Các ca còn lại bao phủ 7 API, quyền, ngày/tháng, hoàn khác ngày bán, doanh thu âm, thứ tự top phim, giới hạn kết quả, liên kết lỗi và truy vấn không ghi TXT. Chưa thể xác nhận bộ test ReportService PASS hoàn toàn. Service chỉ tính trên dữ liệu RAM; không có transaction nhiều file.
