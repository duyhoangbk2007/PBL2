# Nhật ký Bước 05 — Đăng nhập và phân quyền

**Phụ trách Backend:** Hoàng theo [phân công hiện hành](PhanCongCongViec.md). **Trạng thái:** đang thực hiện; chưa nghiệm thu. Kiệt phụ trách màn hình đăng nhập và tích hợp GUI ở Bước 12.

## Mục tiêu

Hoàn thiện `UserRepository` để quản lý tài khoản trong `users.txt` và `AuthService` để đăng nhập, đăng xuất, kiểm tra trạng thái tài khoản và quyền Admin/Staff. Service nghiệp vụ dùng `requireLogin()` hoặc `requireAdmin()` trước thao tác cần quyền; GUI chỉ gọi Service.

## Hiện trạng mã

- Đã có `src/data_access/UserRepository.h/.cpp`: đọc/ghi tài khoản, tra theo `userId` và `username`, thêm/sửa dữ liệu và giữ chỉ mục trong bộ nhớ.
- Đã có `src/business/AuthService.h/.cpp`: đăng nhập, đăng xuất, lấy tài khoản hiện tại, kiểm tra vai trò và yêu cầu đăng nhập/Admin. Các hàm `requireLogin()`/`requireAdmin()` có thể ném exception.
- Đã có `tests/test_auth.cpp`. Sự tồn tại của mã và bài test chưa đủ để đánh dấu toàn bước hoàn thành.

## Checklist

- [x] Có mã `UserRepository` và `AuthService` cùng bài test xác thực.
- [x] Kiểm thử các luồng đăng nhập/quyền cơ bản: Admin, Staff, tài khoản vô hiệu hóa, đăng xuất/chuyển tài khoản và thông tin đăng nhập sai đều đạt trong lần chạy hiện tại.
- [ ] Hoàn thiện hợp đồng lỗi khi Repository chưa nạp, file thiếu, bản ghi sai/trùng, từ chối reload và lỗi ghi.
- [ ] Chạy lại toàn bộ `test_auth.cpp` đạt; lần kiểm tra đã ghi ngày 2026-11-17 còn 5 ca lỗi. Đối chiếu tiếp quyền từ các Service sử dụng `AuthService`.
- [ ] Cập nhật tài liệu theo hành vi đã kiểm chứng và nghiệm thu Bước 05.

## Kiểm thử và vướng mắc

Ngày 2026-11-17, Codex biên dịch C++17 với `-I . -I src`, liên kết `tests/test_auth.cpp` với các `.cpp` trong `src/` trừ `src/main.cpp`, rồi chạy trong thư mục tạm: **9 PASS, 5 FAIL**. Các ca chưa đạt:

1. Repository chưa nạp hoặc file thiếu: bài test chờ `runtime_error`, thao tác hiện thành công.
2. Bản ghi sai: thông báo lỗi thiếu tên file/dòng theo bài test.
3. Khóa trùng: thông báo lỗi thiếu ngữ cảnh theo bài test.
4. Từ chối reload: bài test chờ `runtime_error`, thao tác hiện thành công.
5. Lỗi ghi: bài test chờ `runtime_error`, thao tác hiện thành công.

Đây là khác biệt giữa hành vi mã hiện tại và hợp đồng mà test kiểm tra; cần đối chiếu API đã thống nhất trước khi sửa code. Không thay đổi code trong lần cập nhật tài liệu này. Bước 05 chưa nghiệm thu dù các ca đăng nhập cơ bản đã đạt.

## Việc tiếp theo và điều kiện hoàn thành

Hoàng xử lý các ca lỗi còn lại, chạy lại `test_auth.cpp`, rồi kiểm tra tích hợp phân quyền ở Service phim/phòng/ghế/suất. Chỉ đánh dấu hoàn thành khi các ca bắt buộc đạt và tài liệu phản ánh đúng kết quả. Kiệt dùng API xác thực đã thống nhất khi xây dựng GUI ở Bước 12.
