# Bước 07 — Quản lý nhân viên

**Phụ trách Backend:** Hoàng. **Trạng thái:** `StaffService` và kiểm thử riêng đã đạt trong phạm vi hiện có; chưa nghiệm thu các phụ thuộc lịch sử hóa đơn. GUI quản lý nhân viên thuộc Kiệt ở Bước 12.

## Hiện trạng mã

`src/business/StaffService.h/.cpp` nhận `UserRepository&` và `AuthService&`; dùng cùng Repository tài khoản với đăng nhập, không có `StaffRepository` riêng. `load()` và `save()` chuyển tiếp đến `UserRepository`, không tự kiểm tra quyền và không tự lưu sau mỗi thay đổi.

- `addStaff(User*)`: yêu cầu Admin, chỉ nhận role `STAFF`, chuyển quyền sở hữu con trỏ cho Repository khi thêm thành công.
- `updateStaff(const User&)`: yêu cầu Admin; chỉ sửa tài khoản Staff đã có, không đổi role hoặc trạng thái `active`. Repository kiểm tra dữ liệu và username trùng.
- `deactivateStaff(staffId)`: yêu cầu Admin; đổi `active` từ `true` sang `false`, từ chối Admin, ID không tồn tại và Staff đã vô hiệu hóa.
- `findStaffById()`, `findStaffByUsername()`, `getStaffAt()`, `getStaffCount()`: yêu cầu Admin và chỉ trả/đếm role `STAFF`; kết quả tra cứu là `const User*`. Danh sách và số lượng vẫn bao gồm Staff không hoạt động.

Các hàm có kiểm tra quyền gọi `requireAdmin()` trước thao tác; thiếu quyền có thể ném exception. Chi tiết API và quyền sở hữu con trỏ ở [StaffService](<../src/business/StaffService.md>).

## Checklist

- [x] Dùng chung `UserRepository`/`AuthService`, giữ kiểm tra quyền trong Service và lọc role `STAFF`.
- [x] Thêm, sửa thông tin, tìm kiếm, duyệt/đếm và vô hiệu hóa Staff; không sửa Admin qua `StaffService`.
- [x] Kiểm thử dữ liệu hợp lệ/sai/trùng, quyền, trạng thái và lưu/nạp lại bằng `tests/test_staff_service.cpp`: mốc tài liệu trước ghi **59/59 PASS**; lần biên dịch/chạy C++17 trên nhánh hiện tại đạt **67/67 PASS, 0 FAIL**, thoát mã 0.
- [ ] Kiểm chứng tác động đến hóa đơn đã tham chiếu nhân viên khi có Repository giao dịch; hiện chưa có test tích hợp này.

## Giới hạn hiện tại

Không có API kích hoạt lại hoặc xóa vật lý nhân viên. Việc vô hiệu hóa giữ ID để phục vụ lịch sử; không coi xóa vật lý là chức năng đã triển khai. `load()`/`save()` không kiểm tra quyền và cần được gọi bởi luồng quản lý dữ liệu phù hợp. Bước 05 về xác thực vẫn có các ca test lỗi riêng, nên kết quả test Staff không thay thế nghiệm thu Bước 05.
