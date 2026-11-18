# Bước 08 — Quản lý thành viên

**Phụ trách Backend:** Hoàng. **Trạng thái:** đang thực hiện. `MemberRepository` đã có mã và test riêng đạt; `MemberService` chưa triển khai. GUI thành viên thuộc Kiệt ở Bước 12.

## Hiện trạng mã

`src/data_access/MemberRepository.h/.cpp` chứa `Repository<Member>` và hai chỉ mục `unordered_map` theo `memberId` và số điện thoại. Định dạng TXT là `memberId|fullName|phone|points`, không có schema hoặc dòng tiêu đề. Repository nhận đường dẫn file từ constructor, không tự kiểm tra quyền hoặc lưu sau mỗi thay đổi.

- `load()` nạp một lần, dựng hai chỉ mục; dữ liệu sai/trùng gây exception và dọn trạng thái nạp lỗi. `save()` yêu cầu đã nạp và báo lỗi khi ghi thất bại.
- `add(Member*)` yêu cầu dữ liệu hợp lệ, ID và số điện thoại duy nhất, điểm ban đầu bằng 0; Repository sở hữu con trỏ khi thêm thành công.
- `update(const Member&)` chỉ sửa họ tên và số điện thoại; không đổi ID hoặc điểm. Chỉ mục điện thoại được cập nhật cùng dữ liệu.
- `findById()`, `findByPhone()`, `getAt()`, `count()`, `isLoaded()` cung cấp tra cứu và trạng thái trong RAM.

ID dùng tiền tố `MEM` theo `Validator::isValidId`; số điện thoại có đúng 10 chữ số và bắt đầu bằng `0`; điểm không âm. Chi tiết API và các trường hợp lỗi ở [MemberRepository](<../CauTrucThuMuc/data_acess(docs)/MemberRepository.md>).

## Checklist

- [x] Triển khai parse/serialize, nạp/lưu, thêm/cập nhật/tra cứu thành viên và kiểm tra ID, số điện thoại duy nhất, điểm hợp lệ.
- [x] Biên dịch và chạy riêng `tests/test_member_repository.cpp` trên nhánh hiện tại ngày 2026-11-18: **63 PASS, 0 FAIL**, thoát mã 0. Test dùng file TXT tạm và kiểm tra cả lưu/nạp lại.
- [ ] Thiết kế và triển khai `MemberService` với phân quyền, API quản lý thành viên và test riêng.
- [ ] Kiểm tra liên kết hóa đơn và thay đổi điểm trong luồng bán/hủy vé khi `InvoiceRepository`, `TicketRepository` và các Service giao dịch sẵn sàng.

Kết quả Repository không xác nhận Service hoặc toàn Bước 08 đã hoàn thành. Hiện chưa có API xóa Member trong Repository; chưa kiểm chứng điều kiện xóa khi có hóa đơn tham chiếu.
