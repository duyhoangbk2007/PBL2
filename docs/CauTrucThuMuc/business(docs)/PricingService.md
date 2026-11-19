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

`updateSeatPrice()` từ chối khi chưa load, giá mới không hữu hạn hoặc `<= 0`, hoặc `SeatType` không phải STANDARD/VIP/COUPLE. `updateDiscountRate()` từ chối khi chưa load, tỷ lệ không hữu hạn/ngoài `[0,100]`, hoặc `DiscountType` khác STUDENT/MEMBER. Cả hai kiểm tra quyền **trước** dữ liệu, tạo `Pricing` mới từ cấu hình hiện hành và gọi `PricingRepository::update()`. Repository ghi file thành công rồi mới thay cấu hình trong RAM; hai API không cần gọi `save()` riêng.

## 4. Tính giá và điểm

1. `getBasePrice()` lấy giá theo loại ghế; chưa load ném `runtime_error`, enum ghế sai ném `invalid_argument`.
2. `calculateDiscount()` yêu cầu giá gốc hữu hạn và `> 0`, `TicketType` là REGULAR hoặc STUDENT, và bảng giá đã load. Vé STUDENT đủ điều kiện tỷ lệ học sinh; `isMember = true` đủ điều kiện tỷ lệ thành viên. Chọn **tỷ lệ lớn nhất**, không cộng dồn: `discountAmount = basePrice × rate / 100`.
3. `calculatePaidAmount()` yêu cầu giá gốc hữu hạn `> 0`, tiền giảm hữu hạn trong `[0, basePrice]`; trả `basePrice - discountAmount`, không làm tròn về số nguyên.
4. `calculatePoints()` yêu cầu tiền thực trả hữu hạn và `>= 0`. Không có thành viên trả `0`; có thành viên trả `floor(paidAmount / 10000)`. Nếu kết quả vượt `int` tối đa, ném `invalid_argument`.

Ba hàm tính tiền/điểm và `getBasePrice()` có thể ném `invalid_argument` cho đầu vào/enum sai; hàm tính cũng có thể ném `runtime_error` khi chưa đăng nhập hoặc chưa load bảng giá. Code hiện tại **không có** `PriceResult` hay `calculateTicketPrice()`; caller cần phối hợp các hàm trên. Một ghế COUPLE vẫn là một vé; việc xác minh điều kiện học sinh và ghi giá/điểm vào vé thuộc luồng bán vé ở bước sau.

## 5. Kiểm thử và phần chưa hoàn thiện

`tests/test_pricing.cpp` kiểm tra Model, Repository, Service trong một file test: giá/tỷ lệ mặc định, dữ liệu file sai, cập nhật/nạp lại, phân quyền khách/Staff/Admin, giá, ưu đãi không cộng dồn và mốc điểm. Lần biên dịch/chạy C++17 ngày 2026-11-19 đạt **63 PASS, 0 FAIL**, thoát mã 0 cho **toàn bài test**.

Service mới **tính** điểm, chưa cập nhật điểm thành viên, bán/hủy vé hoặc lưu snapshot giá vào `Ticket`. Chưa có kiểm thử tích hợp với `BookingService` và chưa nghiệm thu toàn Bước 09. Xem [tài liệu Bước 09](../../development/Buoc09_QuanLyGiaVeUuDaiVaTichDiem.md).
