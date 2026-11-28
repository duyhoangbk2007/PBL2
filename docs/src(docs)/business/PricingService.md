# PricingService

> Trạng thái: CHỐT MARKDOWN — đã triển khai và hoàn thành trong phạm vi kiểm thử ngày 28/11/2026.

## 1. Tổng quan

PricingService thuộc tầng Business Logic, chịu trách nhiệm xem và cập nhật bảng giá, tính giá gốc, tiền giảm, tiền thực trả và điểm tích lũy trên từng vé. Không trực tiếp đọc ghi TXT, không cộng hoặc thu hồi điểm trên Member, không tạo Ticket hoặc Invoice.

File liên quan: `src/business/PricingService.h`, `src/business/PricingService.cpp`, `src/data_access/PricingRepository.h/.cpp`, `src/models/Pricing.h/.cpp`, `src/models/Seat.h`, `src/models/Ticket.h`, `src/business/AuthService.h`, `src/business/BookingService.cpp`.

Nguồn rà soát: nhánh `refactor/Kiet`, commit `ce0a653cfaf4d5b138b3943ef372c3519b642a86`; AGENTS.md, validation_rules.md, business_rules.md, data_format.md. Bố cục theo MemberService.md.

## 2. Kiến trúc

```text
GUI / BookingService
          |
          v
    PricingService ------> AuthService
          |
          v
    PricingRepository
       |          |
       v          v
    Pricing    pricing.txt
```

- DI: truyền hai dependency qua constructor bằng tham chiếu, Service không sở hữu chúng.
- Ngoại lệ DIP: PricingRepository quản lý một cấu hình không có ID, không phù hợp với CRUD của IRepository<T>. Giữ concrete dependency qua DI, không thêm interface mới; ngoại lệ đã ghi trong AGENTS.md, không tuyên bố đã áp dụng DIP đầy đủ.
- Permission-only: dùng requirePermission, không dùng role cứng hoặc requireLogin thay cho quyền.
- PricingRepository kiểm tra Pricing::isValid(), ghi TXT trước khi thay cấu hình RAM. Service không gọi save thêm.

## 3. Thuộc tính và API

| Thuộc tính | Kiểu | Vai trò |
|---|---|---|
| pricingRepo | PricingRepository& | Nạp, truy xuất và cập nhật cấu hình |
| authService | AuthService& | Kiểm tra Permission |

DiscountType là enum class gồm STUDENT và MEMBER, giữ nguyên vị trí và API.

| Hàm | Permission | Kết quả |
|---|---|---|
| load() | Không | bool |
| getPricing() const | PRICING_VIEW | const Pricing* |
| updateSeatPrice(seatType, newPrice) | PRICING_UPDATE | bool |
| updateDiscountRate(discountType, newRate) | PRICING_UPDATE | bool |
| getBasePrice(seatType) const | PRICING_VIEW | double |
| calculateDiscount(basePrice, ticketType, isMember) const | PRICING_VIEW | double |
| calculatePaidAmount(basePrice, discountAmount) const | PRICING_VIEW | double |
| calculatePoints(paidAmount, isMember) const | PRICING_VIEW | int |

2 thuộc tính, 1 constructor, 8 hàm public, 0 hàm private. Thiếu quyền: requirePermission ném exception trước xử lý. Con trỏ Pricing chỉ mượn, không delete; không lưu giữ như snapshot độc lập. load() có thể nạp lại theo hợp đồng PricingRepository; nếu nạp thất bại thì loaded=false. Hai hàm update trả false khi cấu hình chưa nạp, enum sai, dữ liệu không hợp lệ hoặc ghi thất bại; exception không bị nuốt. Các hàm tính toán giữ hợp đồng exception hiện tại.

## 4. Mã nguồn đã chốt

### 4.1. PricingService.h

```cpp
#ifndef PRICING_SERVICE_H
#define PRICING_SERVICE_H

#include "../data_access/PricingRepository.h"
#include "../models/Pricing.h"
#include "../models/Seat.h"
#include "../models/Ticket.h"
#include "AuthService.h"

enum class DiscountType
{
    STUDENT,
    MEMBER
};

class PricingService
{
private:
    PricingRepository& pricingRepo;
    AuthService& authService;

public:
    PricingService(PricingRepository& pricingRepo,
                   AuthService& authService);

    // Quan ly du lieu
    bool load();
    const Pricing* getPricing() const;

    // Cap nhat bang gia
    bool updateSeatPrice(SeatType seatType, double newPrice);
    bool updateDiscountRate(DiscountType discountType, double newRate);

    // Tinh gia ve va diem
    double getBasePrice(SeatType seatType) const;
    double calculateDiscount(double basePrice,
                             TicketType ticketType,
                             bool isMember) const;
    double calculatePaidAmount(double basePrice, double discountAmount) const;
    int calculatePoints(double paidAmount, bool isMember) const;
};

#endif
```

### 4.2. PricingService.cpp

```cpp
#include "PricingService.h"
#include <stdexcept>
#include <cmath>
#include <limits>
using namespace std;

PricingService::PricingService(PricingRepository& pricingRepo,
                               AuthService& authService)
    : pricingRepo(pricingRepo),
      authService(authService)
{
}

bool PricingService::load()
{
    return pricingRepo.load();
}

const Pricing* PricingService::getPricing() const
{
    authService.requirePermission("PRICING_VIEW");
    return pricingRepo.getPricing();
}

bool PricingService::updateSeatPrice(SeatType seatType, double newPrice)
{
    authService.requirePermission("PRICING_UPDATE");

    const Pricing* current = pricingRepo.getPricing();
    if (current == nullptr)
    {
        return false;
    }

    double standardPrice = current->getStandardPrice();
    double vipPrice = current->getVipPrice();
    double couplePrice = current->getCouplePrice();

    if (seatType == SeatType::STANDARD)
    {
        standardPrice = newPrice;
    }
    else if (seatType == SeatType::VIP)
    {
        vipPrice = newPrice;
    }
    else if (seatType == SeatType::COUPLE)
    {
        couplePrice = newPrice;
    }
    else
    {
        return false;
    }

    Pricing newPricing(standardPrice,
                       vipPrice,
                       couplePrice,
                       current->getStudentDiscount(),
                       current->getMemberDiscount());
    return pricingRepo.update(newPricing);
}

bool PricingService::updateDiscountRate(DiscountType discountType, double newRate)
{
    authService.requirePermission("PRICING_UPDATE");

    const Pricing* current = pricingRepo.getPricing();
    if (current == nullptr)
    {
        return false;
    }

    double studentDiscount = current->getStudentDiscount();
    double memberDiscount = current->getMemberDiscount();

    if (discountType == DiscountType::STUDENT)
    {
        studentDiscount = newRate;
    }
    else if (discountType == DiscountType::MEMBER)
    {
        memberDiscount = newRate;
    }
    else
    {
        return false;
    }

    Pricing newPricing(current->getStandardPrice(),
                       current->getVipPrice(),
                       current->getCouplePrice(),
                       studentDiscount,
                       memberDiscount);
    return pricingRepo.update(newPricing);
}

double PricingService::getBasePrice(SeatType seatType) const
{
    authService.requirePermission("PRICING_VIEW");

    const Pricing* current = pricingRepo.getPricing();
    if (current == nullptr)
    {
        throw runtime_error("Bang gia chua duoc nap");
    }
    if (seatType == SeatType::STANDARD)
    {
        return current->getStandardPrice();
    }
    if (seatType == SeatType::VIP)
    {
        return current->getVipPrice();
    }
    if (seatType == SeatType::COUPLE)
    {
        return current->getCouplePrice();
    }
    throw invalid_argument("Loai ghe khong hop le");
}

double PricingService::calculateDiscount(double basePrice,
                                         TicketType ticketType,
                                         bool isMember) const
{
    authService.requirePermission("PRICING_VIEW");

    const Pricing* current = pricingRepo.getPricing();
    if (current == nullptr)
    {
        throw runtime_error("Bang gia chua duoc nap");
    }
    if (!isfinite(basePrice) || basePrice <= 0)
    {
        throw invalid_argument("Gia ve khong hop le");
    }

    double discountRate = 0;
    if (ticketType == TicketType::STUDENT)
    {
        discountRate = current->getStudentDiscount();
    }
    else if (ticketType != TicketType::REGULAR)
    {
        throw invalid_argument("Loai ve khong hop le");
    }
    if (isMember && current->getMemberDiscount() > discountRate)
    {
        discountRate = current->getMemberDiscount();
    }
    return basePrice * (discountRate / 100.0);
}

double PricingService::calculatePaidAmount(double basePrice, double discountAmount) const
{
    authService.requirePermission("PRICING_VIEW");

    if (!isfinite(basePrice) || !isfinite(discountAmount) ||
        basePrice <= 0 || discountAmount < 0 ||
        discountAmount > basePrice)
    {
        throw invalid_argument("So tien khong hop le");
    }
    return basePrice - discountAmount;
}

int PricingService::calculatePoints(double paidAmount, bool isMember) const
{
    authService.requirePermission("PRICING_VIEW");

    if (!isfinite(paidAmount) || paidAmount < 0)
    {
        throw invalid_argument("So tien thanh toan khong hop le");
    }
    if (!isMember)
    {
        return 0;
    }

    double points = floor(paidAmount / 10000.0);
    if (points > numeric_limits<int>::max())
    {
        throw invalid_argument("So diem vuot gioi han");
    }
    return static_cast<int>(points);
}
```

## 5. Giải thích từng hàm

Header dùng include guard tránh khai báo lặp, include trực tiếp các kiểu được sử dụng. `enum class` giới hạn tên giá trị trong DiscountType. Các dependency private là tham chiếu; public là API cho caller. `const` cuối hàm truy vấn/tính toán không cho thay đổi trực tiếp thuộc tính Service, không tự bảo đảm dependency bất biến. `<stdexcept>` cung cấp runtime_error và invalid_argument; `<cmath>` cung cấp isfinite và floor; `<limits>` cung cấp numeric_limits. `using namespace std;` chỉ đặt trong cpp.

### 5.1. Constructor

**Tổng quan:** Nhận hai tham chiếu repository và AuthService; không trả giá trị, không load.

**Luồng hoạt động chi tiết:**

```text
Nhận dependency -> Khởi tạo tham chiếu -> Kết thúc
```

**Giải thích chi tiết từng đoạn code:** Initializer list sau dấu : gắn pricingRepo và authService với đối tượng caller truyền vào. Tham chiếu phải được khởi tạo ngay, không tạo bản sao; caller phải bảo đảm vòng đời dependency dài hơn Service. Thân hàm rỗng vì không cần xử lý thêm.

### 5.2. load()

**Tổng quan:** Nạp cấu hình từ TXT; trả bool, có thể ném exception từ Repository. Không yêu cầu quyền để khởi tạo hệ thống.

**Luồng hoạt động chi tiết:**

```text
Gọi pricingRepo.load() -> Trả kết quả hoặc truyền exception
```

**Giải thích chi tiết từng đoạn code:** return pricingRepo.load() chuyển toàn bộ trách nhiệm đọc, parse, kiểm tra và trạng thái loaded cho Repository. Khác CRUD thông thường, PricingRepository cho nạp lại và vô hiệu hóa cấu hình nếu nạp thất bại.

### 5.3. getPricing()

**Tổng quan:** Trả const Pricing* cho người có PRICING_VIEW; nullptr nếu chưa nạp.

**Luồng hoạt động chi tiết:**

```text
Kiểm tra quyền -> [Thiếu: exception] -> getPricing() -> con trỏ hoặc nullptr
```

**Giải thích chi tiết từng đoạn code:** requirePermission kiểm tra phiên và quyền trước khi đọc. Repository trả địa chỉ cấu hình nội bộ nếu loaded; const ngăn sửa trực tiếp qua con trỏ. Không delete hoặc coi con trỏ là bản sao lịch sử.

### 5.4. updateSeatPrice()

**Tổng quan:** Nhận loại ghế và giá mới; trả true nếu cập nhật và ghi file thành công, false nếu thất bại.

**Luồng hoạt động chi tiết:**

```text
Kiểm tra quyền -> Lấy current -> [null: false]
 -> Sao chép 3 giá -> Chọn loại ghế -> [enum sai: false]
 -> Tạo Pricing mới -> repo.update() -> bool
```

**Giải thích chi tiết từng đoạn code:** PRICING_UPDATE được kiểm tra trước. current là con trỏ chỉ đọc; kiểm tra null trước khi dùng ->. Ba biến double sao chép giá hiện hành. Chuỗi if/else if chỉ thay đúng một giá theo SeatType; else từ chối enum không hợp lệ. newPricing là đối tượng cục bộ gồm ba giá và hai tỷ lệ cũ, không sửa cấu hình đang dùng. update nhận đối tượng theo const reference, kiểm tra isValid rồi ghi TXT; chỉ thay RAM nếu ghi thành công. Không lặp kiểm tra giá hữu hạn/dương tại Service; không gọi save lần nữa.

### 5.5. updateDiscountRate()

**Tổng quan:** Nhận DiscountType và tỷ lệ phần trăm mới; trả bool theo kết quả cập nhật.

**Luồng hoạt động chi tiết:**

```text
Kiểm tra quyền -> Lấy current -> [null: false]
 -> Sao chép 2 tỷ lệ -> Chọn loại giảm -> [enum sai: false]
 -> Tạo Pricing mới -> repo.update() -> bool
```

**Giải thích chi tiết từng đoạn code:** Kiểm tra PRICING_UPDATE, lấy con trỏ và chặn nullptr. Hai biến double giữ tỷ lệ hiện tại. if/else if thay đúng STUDENT hoặc MEMBER; else trả false. newPricing giữ nguyên ba giá, truyền hai tỷ lệ đã xử lý. Repository kiểm tra tỷ lệ hữu hạn trong [0,100] và ghi file trước khi thay RAM. Không kiểm tra trùng tầng.

### 5.6. getBasePrice()

**Tổng quan:** Nhận SeatType; trả giá gốc double. Cần PRICING_VIEW và cấu hình đã nạp.

**Luồng hoạt động chi tiết:**

```text
Kiểm tra quyền -> Lấy current -> [null: runtime_error]
 -> STANDARD/VIP/COUPLE: trả giá tương ứng
 -> Loại khác: invalid_argument
```

**Giải thích chi tiết từng đoạn code:** requirePermission chạy trước truy xuất. current null nghĩa là không có cấu hình khả dụng; runtime_error báo lỗi trạng thái. Ba if trả ngay getter phù hợp. Nếu không khớp enum nào, invalid_argument báo tham số sai. Không kiểm tra lại giá vì cấu hình đã được Model/Repository xác thực.

### 5.7. calculateDiscount()

**Tổng quan:** Nhận giá gốc, loại vé, cờ thành viên; trả tiền giảm double, không cộng dồn ưu đãi.

**Luồng hoạt động chi tiết:**

```text
Kiểm tra quyền -> Lấy current -> [null: runtime_error]
 -> Kiểm tra basePrice -> [sai: invalid_argument]
 -> rate=0 -> STUDENT: lấy tỷ lệ; REGULAR: giữ 0; khác: exception
 -> Có thành viên và tỷ lệ cao hơn? Có: thay rate
 -> Trả basePrice * (rate / 100)
```

**Giải thích chi tiết từng đoạn code:** PRICING_VIEW thay requireLogin. Kiểm tra current trước khi đọc. isfinite loại NaN và vô cực; giá gốc phải dương vì đây là đầu vào tính toán không qua Repository. discountRate bắt đầu bằng 0; STUDENT lấy tỷ lệ học sinh, REGULAR giữ 0, enum khác ném lỗi. Toán tử && chỉ xét ưu đãi thành viên khi isMember đúng; so sánh > chọn tỷ lệ cao nhất, không cộng. Chia tỷ lệ cho 100.0 trước khi nhân để hạn chế tràn số trung gian; kết quả vẫn là double, không làm tròn tiền. Có thể khác sai số rất nhỏ so với thứ tự phép tính cũ.

### 5.8. calculatePaidAmount()

**Tổng quan:** Nhận giá gốc và tiền giảm; trả tiền thực trả double. Không cần bảng giá đã nạp.

**Luồng hoạt động chi tiết:**

```text
Kiểm tra quyền -> Kiểm tra hai số hữu hạn, giá > 0, 0 <= giảm <= giá
 -> [Sai: invalid_argument] -> Trả giá - giảm
```

**Giải thích chi tiết từng đoạn code:** PRICING_VIEW bảo vệ phép tính. isfinite kiểm tra cả hai tham số; || nghĩa là chỉ cần một điều kiện sai là ném lỗi. Cho phép giảm bằng giá gốc (thực trả 0). Phép trừ trả tiền thực trả, không ghi file hoặc thay đổi Model.

### 5.9. calculatePoints()

**Tổng quan:** Nhận tiền thực trả và cờ thành viên; trả int điểm của một vé. Không cần bảng giá đã nạp.

**Luồng hoạt động chi tiết:**

```text
Kiểm tra quyền -> Kiểm tra paidAmount -> [Sai: invalid_argument]
 -> Không là thành viên: 0
 -> floor(paidAmount / 10000) -> [Vượt int max: invalid_argument]
 -> static_cast<int> -> Trả điểm
```

**Giải thích chi tiết từng đoạn code:** PRICING_VIEW chạy trước. Tiền phải hữu hạn và không âm kể cả khách không là thành viên. !isMember trả 0 ngay. floor trong cmath làm tròn xuống; chia 10000.0 theo quy tắc một điểm mỗi 10.000 VND thực trả trên từng vé. points vẫn là double để kiểm tra giới hạn trước khi chuyển kiểu. numeric_limits<int>::max() lấy giới hạn int của môi trường biên dịch thay số viết cứng. static_cast<int> chuyển kết quả đã kiểm tra sang int; không cộng trực tiếp vào Member. Kiểm thử biên trên môi trường C++17 của dự án.

## 6. Nội dung thay đổi và điểm cần chốt

- Giữ 8 hàm public, không thêm save hoặc helper, không đổi enum/API.
- Ngoại lệ DIP của PricingRepository đã ghi ngắn gọn trong AGENTS.md; Service nhận dependency qua constructor.
- Ba hàm tính toán chuyển từ requireLogin sang PRICING_VIEW. Đây là thay đổi chính sách quyền: tài khoản chỉ có TICKET_SELL mà thiếu PRICING_VIEW sẽ không thể hoàn thành luồng tính giá. Bảng quyền mặc định cấp cả hai cho ADMIN/STAFF. Quyền tùy chỉnh đã được kiểm thử; phạm vi PRICING_VIEW đã cập nhật trong business_rules.md. BookingService chưa được tích hợp lại.
- Loại validation trùng ở update; giữ kiểm tra nullptr, enum và tham số tính toán.
- Dùng numeric_limits và đổi thứ tự phép nhân/chia; không đổi công thức nghiệp vụ. Không cộng dồn ưu đãi, không làm tròn tiền, điểm tính riêng từng vé.
- Giá mặc định STANDARD 70000, VIP 100000, COUPLE 160000, STUDENT 10%, MEMBER 5%; cấu hình thực tế lấy từ TXT, không dùng mặc định thay cho file lỗi. Ví dụ COUPLE, STUDENT và thành viên: giảm 16000, trả 144000, 14 điểm.
- update ghi TXT trước khi thay RAM; lỗi ghi có thể làm file thay đổi một phần, không bảo đảm ghi nguyên tử. to_string có giới hạn độ chính xác khi serialize; không thay schema hoặc cơ chế lưu trong phạm vi này.
- BookingService giữ snapshot giá/điểm trên vé và chịu trách nhiệm cộng/thu hồi điểm. Không sửa các caller chưa refactor.

## 7. Kiểm thử

Ngày 28/11/2026, `tests/test_pricing_service.cpp` dùng PricingRepository thực và TXT tạm riêng; biên dịch C++17 và chạy đạt **Passed: 67, Failed: 0, Total: 67**. Hồi quy MemberService **45/45**, StaffService **32/32**, AuthService **31/31**, MovieService **41/41**, RoomService **43/43 PASS**. Tổng sáu bài: **259 PASS, 0 FAIL**. `.h/.cpp` khớp hai khối code Markdown 100%; giữ 8 hàm public, không có hàm private, DI qua `PricingRepository&` đặc thù và Permission-only.

- [x] Constructor DI và toàn bộ 8 hàm public.
- [x] Load đúng 5 khóa, thiếu file, dữ liệu sai, trùng khóa, reload thành công/thất bại; getPricing null khi chưa nạp.
- [x] PRICING_VIEW và PRICING_UPDATE độc lập; chưa đăng nhập, inactive, thiếu quyền kể cả ADMIN; chỉ TICKET_SELL không đủ cho phép tính.
- [x] Cập nhật ba loại ghế và hai tỷ lệ; giữ thuộc tính khác, nạp lại từ TXT xác nhận lưu.
- [x] Enum sai, giá 0/âm/NaN/vô cực, tỷ lệ âm/trên 100/NaN/vô cực; tỷ lệ 0 và 100 hợp lệ; không đổi RAM khi update thất bại.
- [x] Lỗi ghi TXT trong thư mục tạm: update trả false và cấu hình RAM giữ nguyên.
- [x] getBasePrice đủ ba loại, enum sai và chưa nạp.
- [x] calculateDiscount: REGULAR/STUDENT, có/không thành viên, tỷ lệ bằng nhau hoặc thành viên cao hơn, không cộng dồn, enum và giá sai, chưa nạp, giá hữu hạn rất lớn không tràn trung gian.
- [x] calculatePaidAmount: giảm 0/toàn bộ, phần thập phân, giảm âm/vượt giá, NaN/vô cực; không yêu cầu loaded.
- [x] calculatePoints: không thành viên, 0, dưới/đúng 10000, phần dư, giới hạn int/vượt giới hạn, âm/NaN/vô cực; không yêu cầu loaded.
- [x] Hồi quy Member 45, Staff 32, Auth 31, Movie 41, Room 43; 192/192 PASS.
- [ ] Chưa kiểm chứng tính nguyên tử của TXT nếu ghi thất bại giữa chừng và tích hợp BookingService.

Lệnh PowerShell tham khảo; khi chạy thực tế đã chọn các dependency cần thiết, loại Service chưa refactor:

```powershell
$src = Get-ChildItem src -Recurse -Filter *.cpp | Where-Object {
    $_.FullName -notmatch '[\\/](presentation|gui)[\\/]' -and
    ($_.FullName -notmatch '[\\/]business[\\/]' -or $_.Name -in @('PricingService.cpp','AuthService.cpp')) -and
    $_.Name -ne 'main.cpp'
}
g++ -std=c++17 -I src tests/test_pricing_service.cpp $src.FullName -o test_pricing_service.exe
if ($LASTEXITCODE -eq 0) { .\test_pricing_service.exe }
```

Mỗi test có main riêng. BookingService chưa được biên dịch trong cấu hình test do còn gọi API cũ `InvoiceRepository::rollbackPending()` và `AuthService::requireAdmin()`; chưa sửa module này.

## 8. Bàn giao

- [x] Rà soát mã nguồn và đề xuất thiết kế.
- [x] Soạn Markdown dự thảo.
- [x] Review, xác nhận ngoại lệ DIP và phạm vi PRICING_VIEW, CHỐT MARKDOWN.
- [x] Triển khai .h/.cpp khớp 100%.
- [x] Biên dịch C++17 `PricingService.cpp` và kiểm tra cú pháp cùng `AuthService.cpp`, `PricingRepository.cpp`, `Pricing.cpp`; hai file `.h/.cpp` khớp khối code Markdown 100%.
- [x] Viết và chạy test PricingService 67/67 PASS cùng năm bài hồi quy 192/192 PASS; hoàn thành module trong phạm vi đã kiểm thử.
- [x] Cập nhật PricingService.md, AGENTS.md, business_rules.md và tài liệu tiến độ, phân công, Task_Kiet; ghi rõ giới hạn chưa kiểm chứng.
- [ ] Commit/push do người phụ trách thực hiện.

Vị trí tài liệu chi tiết theo cấu trúc nhóm hiện dùng: `docs/src(docs)/business/PricingService.md`; bản thiết kế đã CHỐT MARKDOWN, triển khai và kiểm thử trong phạm vi nêu trên. Kiểm tra cú pháp `BookingService.cpp` còn lỗi tại lời gọi `InvoiceRepository::rollbackPending()` và `AuthService::requireAdmin()` đã không còn tồn tại; chưa xử lý Service ngoài phạm vi.
