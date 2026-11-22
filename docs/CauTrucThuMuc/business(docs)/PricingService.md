# PricingService — Giá vé, ưu đãi và điểm (Bước 09)

## 1. Mục đích và phụ thuộc

`PricingService` ở `src/business/PricingService.h/.cpp` nhận `PricingRepository&` và `AuthService&` trong constructor. Service dùng `SeatType` từ `Seat.h`, `TicketType` từ `Ticket.h` và `enum class DiscountType { STUDENT, MEMBER }` khai báo trong `PricingService.h`. Lớp không trực tiếp đọc/ghi TXT và không cộng/trừ điểm vào `Member`. Giá trị `Pricing` được Repository giữ; `load()` chỉ chuyển tiếp đến `pricingRepo.load()`.

## 2. API và phân quyền hiện có

| Hàm | Kết quả và quyền |
| --- | --- |
| `PricingService(PricingRepository&, AuthService&)` | Nhận hai tham chiếu, không tự nạp dữ liệu. |
| `load()` | `bool`; gọi `pricingRepo.load()`, **không** kiểm tra quyền. |
| `getPricing() const` | `const Pricing*`; gọi `requirePermission("PRICING_VIEW")`, rồi lấy cấu hình; có thể trả `nullptr` nếu chưa load. |
| `updateSeatPrice(SeatType, double)` | `bool`; gọi `requirePermission("PRICING_UPDATE")`, thay một giá và gọi `pricingRepo.update()`; không tự thay các giá/tỷ lệ còn lại. |
| `updateDiscountRate(DiscountType, double)` | `bool`; gọi `requirePermission("PRICING_UPDATE")`, thay một tỷ lệ rồi gọi Repository. |
| `getBasePrice(SeatType) const` | `double`; gọi `requirePermission("PRICING_VIEW")`; trả giá STANDARD/VIP/COUPLE. |
| `calculateDiscount(double basePrice, TicketType ticketType, bool isMember) const` | `double`; gọi **`requireLogin()`**, tính tiền giảm. |
| `calculatePaidAmount(double basePrice, double discountAmount) const` | `double`; gọi **`requireLogin()`**, trả tiền phải trả. |
| `calculatePoints(double paidAmount, bool isMember) const` | `int`; gọi **`requireLogin()`**, tính điểm. |

`PRICING_VIEW` hiện được cấp cho Admin và Staff, `PRICING_UPDATE` chỉ cho Admin trong `role_permissions.txt`. Quyền thực tế vẫn phụ thuộc file đang nạp; Admin thiếu quyền cũng bị từ chối. **Khác biệt cần lưu ý:** ba hàm tính `calculateDiscount`, `calculatePaidAmount`, `calculatePoints` chỉ đòi đăng nhập, chưa gọi `requirePermission("PRICING_VIEW")`. Không mô tả chúng là đã được chặn bằng mã quyền này. `load()` là thao tác dữ liệu nội bộ, không phải API GUI có phân quyền.

## 3. Quy tắc cập nhật cấu hình

`updateSeatPrice()` từ chối khi chưa load, giá mới không hữu hạn hoặc `<= 0`, hoặc `SeatType` không phải STANDARD/VIP/COUPLE. `updateDiscountRate()` từ chối khi chưa load, tỷ lệ không hữu hạn/ngoài `[0,100]`, hoặc `DiscountType` khác STUDENT/MEMBER. Cả hai kiểm tra quyền **trước** dữ liệu, tạo `Pricing` mới từ cấu hình hiện hành và gọi `PricingRepository::update()`. `Pricing::isValid()` cũng kiểm tra số hữu hạn ở cả năm thuộc tính. Repository ghi file thành công rồi mới thay cấu hình trong RAM; hai API không cần gọi `save()` riêng. Nếu `load()` được gọi lại và đọc lỗi, cấu hình hợp lệ đang dùng vẫn được giữ nguyên.

## 4. Tính giá và điểm

1. `getBasePrice()` lấy giá theo loại ghế; chưa load ném `runtime_error`, enum ghế sai ném `invalid_argument`.
2. `calculateDiscount()` yêu cầu giá gốc hữu hạn và `> 0`, `TicketType` là REGULAR hoặc STUDENT, và bảng giá đã load. Vé STUDENT đủ điều kiện tỷ lệ học sinh; `isMember = true` đủ điều kiện tỷ lệ thành viên. Chọn **tỷ lệ lớn nhất**, không cộng dồn: `discountAmount = basePrice × rate / 100`.
3. `calculatePaidAmount()` yêu cầu giá gốc hữu hạn `> 0`, tiền giảm hữu hạn trong `[0, basePrice]`; trả `basePrice - discountAmount`, không làm tròn về số nguyên.
4. `calculatePoints()` yêu cầu tiền thực trả hữu hạn và `>= 0`. Không có thành viên trả `0`; có thành viên trả `floor(paidAmount / 10000)`. Nếu kết quả vượt `int` tối đa, ném `invalid_argument`.

Ba hàm tính tiền/điểm và `getBasePrice()` có thể ném `invalid_argument` cho đầu vào/enum sai; hàm tính cũng có thể ném `runtime_error` khi chưa đăng nhập hoặc chưa load bảng giá. Code hiện tại **không có** `PriceResult` hay `calculateTicketPrice()`; caller cần phối hợp các hàm trên. Một ghế COUPLE vẫn là một vé; việc xác minh điều kiện học sinh và ghi giá/điểm vào vé thuộc luồng bán vé ở bước sau.

## 5. Kiểm thử và phần chưa hoàn thiện

`tests/test_pricing.cpp` kiểm tra Model, Repository, Service trong một file test: giá/tỷ lệ mặc định, dữ liệu file sai, cập nhật/nạp lại (gồm giữ cấu hình cũ khi reload lỗi), phân quyền khách/Staff/Admin, giá, ưu đãi không cộng dồn và mốc điểm. Lần biên dịch/chạy C++17 ngày 2026-11-20 đạt **69 PASS, 0 FAIL**, thoát mã 0 cho **toàn bài test**. Bộ test chưa có ca riêng cho đầu vào `NaN`/vô cực.

Service mới **tính** điểm, chưa cập nhật điểm thành viên, bán/hủy vé hoặc lưu snapshot giá vào `Ticket`. Chưa có kiểm thử tích hợp với `BookingService` và chưa nghiệm thu toàn Bước 09. Xem [tài liệu Bước 09](../../development/Buoc09_QuanLyGiaVeUuDaiVaTichDiem.md).

## Chi tiết triển khai từng hàm (PricingService.cpp)

### PricingService()

#### A. Tổng quan

- **Mục đích:** Khởi tạo Service với các Repository và AuthService.
- **Tham số:** `PricingRepository& pricingRepo, AuthService& authService) : pricingRepo(pricingRepo), authService(authService`.
- **Giá trị trả về:** `constructor` (không trả giá trị).
- **Điều kiện quan trọng:** điều kiện và lỗi thể hiện trực tiếp trong mã nguồn.

#### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Chuẩn bị và xử lý dữ liệu
    |
    v
Kết thúc
```

#### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1 — Chuẩn bị và xử lý dữ liệu.**

```cpp
PricingService::PricingService(PricingRepository& pricingRepo, AuthService& authService)
    : pricingRepo(pricingRepo), authService(authService)
{
}
```

Các câu lệnh được thực thi theo đúng thứ tự trong `.cpp`; điều kiện và nhánh trả về nằm ngay trong code. Giá trị được khai báo ở đây được dùng trong các nhánh tiếp theo; hãy theo dõi nơi nó được trả về hoặc chuyển vào Repository để hiểu tác động lên trạng thái.

### load()

#### A. Tổng quan

- **Mục đích:** Kiểm tra dữ liệu đã nạp. Xem các nhánh cụ thể trong code.
- **Tham số:** `không có`.
- **Giá trị trả về:** `bool`; các nhánh `return` được ghi đầy đủ bên dưới.
- **Điều kiện quan trọng:** điều kiện và lỗi thể hiện trực tiếp trong mã nguồn.

#### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Kiểm tra dữ liệu đã nạp
    |
    v
Kết thúc
```

#### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1 — Kiểm tra dữ liệu đã nạp.**

```cpp
bool PricingService::load()
{
    return pricingRepo.load();
}
```

Các câu lệnh được thực thi theo đúng thứ tự trong `.cpp`; điều kiện và nhánh trả về nằm ngay trong code. Giá trị được khai báo ở đây được dùng trong các nhánh tiếp theo; hãy theo dõi nơi nó được trả về hoặc chuyển vào Repository để hiểu tác động lên trạng thái.

### getPricing()

#### A. Tổng quan

- **Mục đích:** Kiểm tra quyền. Xem các nhánh cụ thể trong code.
- **Tham số:** `không có`.
- **Giá trị trả về:** `const Pricing*`; các nhánh `return` được ghi đầy đủ bên dưới.
- **Điều kiện quan trọng:** Kiểm tra quyền qua `AuthService`; điều kiện và lỗi thể hiện trực tiếp trong mã nguồn.

#### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Kiểm tra quyền
    |
    v
Kết thúc
```

#### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1 — Kiểm tra quyền.**

```cpp
const Pricing* PricingService::getPricing() const
{
    authService.requirePermission("PRICING_VIEW");
    return pricingRepo.getPricing();
}
```

`AuthService` kiểm tra quyền và có thể ném ngoại lệ. Quyền `PRICING_VIEW` được kiểm tra trước bước xử lý kế tiếp; thiếu quyền sẽ ném ngoại lệ, vì vậy caller không nên xem đó là kết quả `false`.

### updateSeatPrice()

#### A. Tổng quan

- **Mục đích:** Kiểm tra quyền. Xem các nhánh cụ thể trong code.
- **Tham số:** `SeatType seatType, double newPrice`.
- **Giá trị trả về:** `bool`; các nhánh `return` được ghi đầy đủ bên dưới.
- **Điều kiện quan trọng:** Kiểm tra quyền qua `AuthService`; kiểm tra trạng thái nạp và con trỏ trước khi dùng.

#### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Kiểm tra quyền
    |
    v
Kiểm tra điều kiện
    |
    v
Từ chối dữ liệu không hợp lệ
    |
    v
Cập nhật qua Repository
    |
    v
Kết thúc
```

#### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1 — Kiểm tra quyền.**

```cpp
bool PricingService::updateSeatPrice(SeatType seatType, double newPrice)
{
    authService.requirePermission("PRICING_UPDATE");

    const Pricing* current = pricingRepo.getPricing();
    if (current == nullptr || !isfinite(newPrice) || newPrice <= 0)
    {
        return false;
    }
    double standardPrice = current->getStandardPrice();
```

`AuthService` kiểm tra quyền và có thể ném ngoại lệ. `nullptr` cho biết không có đối tượng để truy cập. `false` báo thao tác không thành công. `isfinite` kiểm tra số hữu hạn; `floor` làm tròn xuống. Quyền `PRICING_UPDATE` được kiểm tra trước bước xử lý kế tiếp; thiếu quyền sẽ ném ngoại lệ, vì vậy caller không nên xem đó là kết quả `false`. Nhánh `false` dừng hàm tại đây; những câu lệnh phía sau không chạy, nên không có cập nhật nào từ các bước sau. `isfinite` loại NaN/vô cực trước phép tính; `floor` lấy phần nguyên thấp hơn hoặc bằng giá trị thực.

**Bước 2 — Kiểm tra điều kiện.**

```cpp
    double vipPrice = current->getVipPrice();
    double couplePrice = current->getCouplePrice();

    if (seatType == SeatType::STANDARD)
    {
        standardPrice = newPrice;
    }
    else if (seatType == SeatType::VIP)
    {
        vipPrice = newPrice;
```

Các câu lệnh được thực thi theo đúng thứ tự trong `.cpp`; điều kiện và nhánh trả về nằm ngay trong code. Giá trị được khai báo ở đây được dùng trong các nhánh tiếp theo; hãy theo dõi nơi nó được trả về hoặc chuyển vào Repository để hiểu tác động lên trạng thái.

**Bước 3 — Từ chối dữ liệu không hợp lệ.**

```cpp
    }
    else if (seatType == SeatType::COUPLE)
    {
        couplePrice = newPrice;
    }
    else
    {
        return false;
    }
    Pricing newPricing(
```

`false` báo thao tác không thành công. Nhánh `false` dừng hàm tại đây; những câu lệnh phía sau không chạy, nên không có cập nhật nào từ các bước sau.

**Bước 4 — Cập nhật qua Repository.**

```cpp
        standardPrice,
        vipPrice,
        couplePrice,
        current->getStudentDiscount(),
        current->getMemberDiscount()
    );
    return pricingRepo.update(newPricing);
}
```

Repository thực hiện thao tác dữ liệu. `update()` chuyển việc thay đổi dữ liệu cho Repository; kết quả trả về phản ánh kiểm tra và thao tác cập nhật ở tầng dữ liệu.

### updateDiscountRate()

#### A. Tổng quan

- **Mục đích:** Kiểm tra quyền. Xem các nhánh cụ thể trong code.
- **Tham số:** `DiscountType discountType, double newRate`.
- **Giá trị trả về:** `bool`; các nhánh `return` được ghi đầy đủ bên dưới.
- **Điều kiện quan trọng:** Kiểm tra quyền qua `AuthService`; kiểm tra trạng thái nạp và con trỏ trước khi dùng.

#### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Kiểm tra quyền
    |
    v
Kiểm tra điều kiện
    |
    v
Từ chối dữ liệu không hợp lệ
    |
    v
Cập nhật qua Repository
    |
    v
Kết thúc
```

#### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1 — Kiểm tra quyền.**

```cpp
bool PricingService::updateDiscountRate(DiscountType discountType, double newRate)
{
    authService.requirePermission("PRICING_UPDATE");

    const Pricing* current = pricingRepo.getPricing();
    if (current == nullptr || !isfinite(newRate) || newRate < 0 || newRate > 100)
    {
        return false;
    }
    double studentDiscount = current->getStudentDiscount();
```

`AuthService` kiểm tra quyền và có thể ném ngoại lệ. `nullptr` cho biết không có đối tượng để truy cập. `false` báo thao tác không thành công. `isfinite` kiểm tra số hữu hạn; `floor` làm tròn xuống. Quyền `PRICING_UPDATE` được kiểm tra trước bước xử lý kế tiếp; thiếu quyền sẽ ném ngoại lệ, vì vậy caller không nên xem đó là kết quả `false`. Nhánh `false` dừng hàm tại đây; những câu lệnh phía sau không chạy, nên không có cập nhật nào từ các bước sau. `isfinite` loại NaN/vô cực trước phép tính; `floor` lấy phần nguyên thấp hơn hoặc bằng giá trị thực.

**Bước 2 — Kiểm tra điều kiện.**

```cpp
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
```

Các câu lệnh được thực thi theo đúng thứ tự trong `.cpp`; điều kiện và nhánh trả về nằm ngay trong code. Giá trị được khai báo ở đây được dùng trong các nhánh tiếp theo; hãy theo dõi nơi nó được trả về hoặc chuyển vào Repository để hiểu tác động lên trạng thái.

**Bước 3 — Từ chối dữ liệu không hợp lệ.**

```cpp
    {
        return false;
    }
    Pricing newPricing(
        current->getStandardPrice(),
        current->getVipPrice(),
        current->getCouplePrice(),
        studentDiscount,
        memberDiscount
    );
```

`false` báo thao tác không thành công. Nhánh `false` dừng hàm tại đây; những câu lệnh phía sau không chạy, nên không có cập nhật nào từ các bước sau.

**Bước 4 — Cập nhật qua Repository.**

```cpp
    return pricingRepo.update(newPricing);
}
```

Repository thực hiện thao tác dữ liệu. `update()` chuyển việc thay đổi dữ liệu cho Repository; kết quả trả về phản ánh kiểm tra và thao tác cập nhật ở tầng dữ liệu.

### getBasePrice()

#### A. Tổng quan

- **Mục đích:** Kiểm tra quyền. Xem các nhánh cụ thể trong code.
- **Tham số:** `SeatType seatType`.
- **Giá trị trả về:** `double`; các nhánh `return` được ghi đầy đủ bên dưới.
- **Điều kiện quan trọng:** Kiểm tra quyền qua `AuthService`; kiểm tra trạng thái nạp và con trỏ trước khi dùng.

#### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Kiểm tra quyền
    |
    v
Trả kết quả
    |
    v
Ném ngoại lệ khi lỗi
    |
    v
Kết thúc
```

#### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1 — Kiểm tra quyền.**

```cpp
double PricingService::getBasePrice(SeatType seatType) const
{
    authService.requirePermission("PRICING_VIEW"); 
    
    const Pricing* current = pricingRepo.getPricing();
    if (current == nullptr)
    {
        throw runtime_error("Bang gia chua duoc nap");
    }
    if (seatType == SeatType::STANDARD)
```

`AuthService` kiểm tra quyền và có thể ném ngoại lệ. `nullptr` cho biết không có đối tượng để truy cập. `throw` chuyển lỗi cho bên gọi. Quyền `PRICING_VIEW` được kiểm tra trước bước xử lý kế tiếp; thiếu quyền sẽ ném ngoại lệ, vì vậy caller không nên xem đó là kết quả `false`. Ngoại lệ được chuyển cho caller xử lý; nó khác với `false` dùng cho trường hợp nghiệp vụ không đạt.

**Bước 2 — Trả kết quả.**

```cpp
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
```

Các câu lệnh được thực thi theo đúng thứ tự trong `.cpp`; điều kiện và nhánh trả về nằm ngay trong code. Giá trị được khai báo ở đây được dùng trong các nhánh tiếp theo; hãy theo dõi nơi nó được trả về hoặc chuyển vào Repository để hiểu tác động lên trạng thái.

**Bước 3 — Ném ngoại lệ khi lỗi.**

```cpp
    }
    throw invalid_argument("Loai ghe khong hop le");
}
```

`throw` chuyển lỗi cho bên gọi. Ngoại lệ được chuyển cho caller xử lý; nó khác với `false` dùng cho trường hợp nghiệp vụ không đạt.

### calculateDiscount()

#### A. Tổng quan

- **Mục đích:** Kiểm tra quyền. Xem các nhánh cụ thể trong code.
- **Tham số:** `double basePrice, TicketType ticketType, bool isMember`.
- **Giá trị trả về:** `double`; các nhánh `return` được ghi đầy đủ bên dưới.
- **Điều kiện quan trọng:** Kiểm tra quyền qua `AuthService`; kiểm tra trạng thái nạp và con trỏ trước khi dùng.

#### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Kiểm tra quyền
    |
    v
Ném ngoại lệ khi lỗi
    |
    v
Ném ngoại lệ khi lỗi
    |
    v
Kết thúc
```

#### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1 — Kiểm tra quyền.**

```cpp
double PricingService::calculateDiscount(double basePrice, TicketType ticketType, bool isMember) const
{
    authService.requireLogin();

    const Pricing* current = pricingRepo.getPricing();
    if (current == nullptr)
    {
        throw runtime_error("Bang gia chua duoc nap");
    }
    if (!isfinite(basePrice) || basePrice <= 0)
```

`AuthService` kiểm tra quyền và có thể ném ngoại lệ. `nullptr` cho biết không có đối tượng để truy cập. `throw` chuyển lỗi cho bên gọi. `isfinite` kiểm tra số hữu hạn; `floor` làm tròn xuống. `requireLogin()` bảo đảm có tài khoản đang đăng nhập trước khi truy vấn hoặc cập nhật. Ngoại lệ được chuyển cho caller xử lý; nó khác với `false` dùng cho trường hợp nghiệp vụ không đạt. `isfinite` loại NaN/vô cực trước phép tính; `floor` lấy phần nguyên thấp hơn hoặc bằng giá trị thực.

**Bước 2 — Ném ngoại lệ khi lỗi.**

```cpp
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
```

`throw` chuyển lỗi cho bên gọi. Ngoại lệ được chuyển cho caller xử lý; nó khác với `false` dùng cho trường hợp nghiệp vụ không đạt.

**Bước 3 — Ném ngoại lệ khi lỗi.**

```cpp
        throw invalid_argument("Loai ve khong hop le");
    }
    // áp dụng mức giảm giá coa nhất
    if (isMember && current->getMemberDiscount() > discountRate)
    {
        discountRate = current->getMemberDiscount();
    }
    return basePrice * discountRate / 100.0;
}
```

`throw` chuyển lỗi cho bên gọi. Ngoại lệ được chuyển cho caller xử lý; nó khác với `false` dùng cho trường hợp nghiệp vụ không đạt.

### calculatePaidAmount()

#### A. Tổng quan

- **Mục đích:** Kiểm tra quyền. Xem các nhánh cụ thể trong code.
- **Tham số:** `double basePrice, double discountAmount`.
- **Giá trị trả về:** `double`; các nhánh `return` được ghi đầy đủ bên dưới.
- **Điều kiện quan trọng:** Kiểm tra quyền qua `AuthService`; điều kiện và lỗi thể hiện trực tiếp trong mã nguồn.

#### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Kiểm tra quyền
    |
    v
Trả kết quả
    |
    v
Kết thúc
```

#### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1 — Kiểm tra quyền.**

```cpp
double PricingService::calculatePaidAmount(double basePrice, double discountAmount) const
{
    authService.requireLogin();

    if (!isfinite(basePrice) || !isfinite(discountAmount) ||
        basePrice <= 0 || discountAmount < 0 ||
        discountAmount > basePrice)
    {
        throw invalid_argument("So tien khong hop le");
    }
```

`AuthService` kiểm tra quyền và có thể ném ngoại lệ. `throw` chuyển lỗi cho bên gọi. `isfinite` kiểm tra số hữu hạn; `floor` làm tròn xuống. `requireLogin()` bảo đảm có tài khoản đang đăng nhập trước khi truy vấn hoặc cập nhật. Ngoại lệ được chuyển cho caller xử lý; nó khác với `false` dùng cho trường hợp nghiệp vụ không đạt. `isfinite` loại NaN/vô cực trước phép tính; `floor` lấy phần nguyên thấp hơn hoặc bằng giá trị thực.

**Bước 2 — Trả kết quả.**

```cpp

    return basePrice - discountAmount;
}
```

Các câu lệnh được thực thi theo đúng thứ tự trong `.cpp`; điều kiện và nhánh trả về nằm ngay trong code. Giá trị được khai báo ở đây được dùng trong các nhánh tiếp theo; hãy theo dõi nơi nó được trả về hoặc chuyển vào Repository để hiểu tác động lên trạng thái.

### calculatePoints()

#### A. Tổng quan

- **Mục đích:** Kiểm tra quyền. Xem các nhánh cụ thể trong code.
- **Tham số:** `double paidAmount, bool isMember`.
- **Giá trị trả về:** `int`; các nhánh `return` được ghi đầy đủ bên dưới.
- **Điều kiện quan trọng:** Kiểm tra quyền qua `AuthService`; điều kiện và lỗi thể hiện trực tiếp trong mã nguồn.

#### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Kiểm tra quyền
    |
    v
Ném ngoại lệ khi lỗi
    |
    v
Kết thúc
```

#### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1 — Kiểm tra quyền.**

```cpp
int PricingService::calculatePoints(double paidAmount, bool isMember) const
{
    authService.requireLogin();

    if (!isfinite(paidAmount) || paidAmount < 0)
    {
        throw invalid_argument("So tien thanh toan khong hop le");
    }
    if (!isMember)
    {
```

`AuthService` kiểm tra quyền và có thể ném ngoại lệ. `throw` chuyển lỗi cho bên gọi. `isfinite` kiểm tra số hữu hạn; `floor` làm tròn xuống. `requireLogin()` bảo đảm có tài khoản đang đăng nhập trước khi truy vấn hoặc cập nhật. Ngoại lệ được chuyển cho caller xử lý; nó khác với `false` dùng cho trường hợp nghiệp vụ không đạt. `isfinite` loại NaN/vô cực trước phép tính; `floor` lấy phần nguyên thấp hơn hoặc bằng giá trị thực.

**Bước 2 — Ném ngoại lệ khi lỗi.**

```cpp
        return 0;
    }
    double points = floor(paidAmount / 10000.0); //floor làm tròn xuống số nguyên gần nhất
    if (points > 2147483647.0)
    {
        throw invalid_argument("So diem vuot gioi han");
    }
    return static_cast<int>(points);
}
```

`throw` chuyển lỗi cho bên gọi. `static_cast` chuyển kiểu tường minh. `isfinite` kiểm tra số hữu hạn; `floor` làm tròn xuống. Ngoại lệ được chuyển cho caller xử lý; nó khác với `false` dùng cho trường hợp nghiệp vụ không đạt. `static_cast` nêu rõ kiểu đích khi chuyển giá trị, giúp tránh chuyển kiểu ngầm khó nhận ra. `isfinite` loại NaN/vô cực trước phép tính; `floor` lấy phần nguyên thấp hơn hoặc bằng giá trị thực.

