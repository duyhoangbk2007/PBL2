# Bước 09 — Quản lý giá vé, ưu đãi và tích điểm

**Phụ trách Backend:** Hoàng. **Trạng thái:** đang thực hiện. Model `Pricing`, `PricingRepository` và `PricingService` đã có mã; test chung `test_pricing.cpp` đạt **69 PASS, 0 FAIL** ngày 2026-11-20. Chưa có luồng bán/hủy vé để cập nhật điểm thành viên hoặc lưu snapshot giá vào vé. GUI thuộc Kiệt từ Bước 12.

## 1. Thiết kế hiện hành

- `src/models/Pricing.h/.cpp` giữ ba giá ghế STANDARD/VIP/COUPLE và hai tỷ lệ giảm STUDENT/MEMBER. Constructor mặc định đặt **70000, 100000, 160000 VND; 10%, 5%**. Model không có ID; `isValid()` kiểm tra `std::isfinite()` cho cả năm thuộc tính, giá dương và tỷ lệ trong `[0,100]`.
- `src/data_access/PricingRepository.h/.cpp` giữ một `Pricing`, đọc/ghi năm dòng `KEY|VALUE` trong file do constructor nhận; không dùng `Repository<T>` vì cấu hình không có ID. `load()` chỉ thay cấu hình và trạng thái nạp sau khi kiểm tra toàn file thành công, giữ cấu hình cũ nếu reload lỗi. `update()` ghi file rồi mới thay cấu hình RAM khi ghi báo thành công.
- `src/business/PricingService.h/.cpp` nhận Repository/AuthService bằng tham chiếu. Xem giá dùng `PRICING_VIEW`, sửa giá/tỷ lệ dùng `PRICING_UPDATE`; ba hàm tính giảm, tiền trả, điểm hiện chỉ gọi `requireLogin()`.

Xem [Pricing trong models.md](../CauTrucThuMuc/models.md#pricing--cấu-hình-giá-và-ưu-đãi), [PricingRepository](<../CauTrucThuMuc/data_acess(docs)/PricingRepository.md>), [PricingService](<../CauTrucThuMuc/business(docs)/PricingService.md>) và [định dạng TXT](../TongQuan/data_format.md#pricingtxt).

## 2. Quy tắc giá và dữ liệu

`pricing.txt` có đúng năm khóa `STANDARD`, `VIP`, `COUPLE`, `STUDENT_DISCOUNT`, `MEMBER_DISCOUNT`, mỗi khóa một dòng, không có schema/tiêu đề. Giá dùng `double` VND; tỷ lệ là phần trăm. Giảm giá lấy mức lớn nhất trong các mức STUDENT/MEMBER đủ điều kiện, không cộng dồn. Tiền phải trả bằng giá gốc trừ tiền giảm; thành viên được `floor(paidAmount / 10000)` điểm **theo từng vé**. Điểm chỉ được tính ở Bước 09, chưa được cộng/thu hồi trong dữ liệu thành viên.

## 3. Checklist

- [x] Triển khai `Pricing` với hai constructor, năm getter và `isValid()` kiểm tra số hữu hạn cho cả năm thuộc tính.
- [x] Triển khai `PricingRepository` cho nạp/lưu/cập nhật một cấu hình và kiểm tra file `KEY|VALUE`; giữ cấu hình hợp lệ trước đó nếu reload thất bại.
- [x] Triển khai `PricingService` cho xem/sửa bảng giá, tính giá gốc, giảm không cộng dồn, tiền thực trả và điểm; dùng RBAC `PRICING_VIEW`/`PRICING_UPDATE` ở các hàm xem/sửa.
- [x] Biên dịch và chạy riêng `tests/test_pricing.cpp` trên nhánh hiện tại ngày 2026-11-20: **69 PASS, 0 FAIL**, thoát mã 0. Kết quả là tổng ca cho Model, Repository và Service; có ca xác nhận reload lỗi giữ cấu hình cũ.
- [ ] Bổ sung ca test trực tiếp cho `NaN`/vô cực và hoàn thiện cơ chế báo file/dòng, ghi file an toàn khi lỗi; bộ test hiện chưa kiểm chứng các trường hợp này.
- [ ] Tích hợp `PricingService` vào luồng bán/hủy vé để ghi giá, tiền giảm, tiền trả và điểm lên vé, cập nhật/thu hồi điểm thành viên; chưa có `BookingService`/Repository vé để kiểm chứng.

Không đánh dấu toàn Bước 09 hoàn thành chỉ từ 69 ca test hiện có. Code không có `PricingConfig`, `PriceResult` hoặc `calculateTicketPrice()` như bản thiết kế trước; tài liệu hiện hành dùng đúng lớp `Pricing` và các API đã triển khai.
