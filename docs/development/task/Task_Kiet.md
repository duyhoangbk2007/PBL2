# Công việc của Kiệt — giai đoạn refactor

## Tiến độ Data Access

- [x] Phối hợp CHỐT MARKDOWN và xây dựng `IRepository<T>`.
- [x] Cập nhật tài liệu dùng chung `Repository.md` theo code hiện tại; kết quả `test_irepository.cpp` xác nhận gián tiếp `Repository<Movie>`, chưa xác nhận các Model khác.
- [x] Triển khai DIP cho `RoomRepository` theo `IRepository<Room>`; Hoàng đã rà soát, chuẩn hóa tài liệu và kiểm thử lại `tests/test_room_repository.cpp` ngày 25/11/2026: 72 PASS, 0 FAIL.
- [x] `UserRepository` thuộc phạm vi phụ trách đã CHỐT MARKDOWN và triển khai DIP theo `IRepository<User>`; bỏ hai `unordered_map`, dùng `Repository<User>` làm lưu trữ duy nhất và chuẩn hóa validation. `tests/test_user_repository.cpp` ngày 25/11/2026 đạt 69 PASS, 0 FAIL.
- [x] `MemberRepository` thuộc phạm vi phụ trách đã CHỐT MARKDOWN, triển khai DIP theo `IRepository<Member>`, bỏ `membersById`, giữ `membersByPhone` và dùng `Validator::isValidPhone()`. Mã nguồn, tài liệu và `tests/test_member_repository.cpp` đã được chuẩn hóa, kiểm thử ngày 25/11/2026 đạt 204 PASS, 0 FAIL; liên kết Invoice chưa kiểm thử.
- [x] Ghi nhận dependency dùng chung `ShowtimeRepository` (Hoàng phụ trách) đã CHỐT MARKDOWN, triển khai DIP và kiểm thử `tests/test_showtime_repository.cpp` ngày 26/11/2026: Passed 85, Failed 0, Total 85. Kết quả chỉ xác nhận phạm vi Repository; bảo vệ lịch sử Ticket/Invoice sẽ hoàn thiện tại ShowtimeService, không thuộc kết quả kiểm thử này.
- [x] `SeatRepository` thuộc phạm vi phụ trách đã CHỐT MARKDOWN, triển khai DIP, chuẩn hóa comment và đối chiếu `.h/.cpp` khớp hai khối code trong tài liệu. `tests/test_seat_repository.cpp` ngày 26/11/2026 biên dịch và chạy đạt Passed 81, Failed 0, Total 81 trong phạm vi bài test.
- [ ] Kiểm thử các nhánh lỗi nội bộ/rollback do ngoại lệ và kiểm hồi quy liên quan của SeatRepository.
- [ ] Hoàn thiện SeatService: kiểm tra Permission và lịch sử Ticket, kể cả vé đã hủy, trước khi cho xóa ghế.
- [ ] Refactor các Repository thuộc phạm vi phân công trong `../PhanCongCongViec.md`.

Mẫu chuẩn đã hoàn thành: `MovieRepository.h/.cpp` và `MovieRepository.md` đã CHỐT MARKDOWN, refactor DIP và validation, kiểm thử qua `tests/test_irepository.cpp` ngày 24/11/2026 (81 PASS, 0 FAIL). Kiệt áp dụng mẫu theo `AGENTS.md` và điều chỉnh logic cho từng Model; RoomRepository, SeatRepository, UserRepository và MemberRepository đã được xác nhận trong phạm vi bài test riêng, các Repository khác của Kiệt chưa được xác nhận.
