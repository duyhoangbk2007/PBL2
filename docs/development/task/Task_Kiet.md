# Công việc của Kiệt — giai đoạn refactor

## Tiến độ Data Access

- [x] Phối hợp CHỐT MARKDOWN và xây dựng `IRepository<T>`.
- [x] Cập nhật tài liệu dùng chung `Repository.md` theo code hiện tại; kết quả `test_irepository.cpp` xác nhận gián tiếp `Repository<Movie>`, chưa xác nhận các Model khác.
- [x] Triển khai DIP cho `RoomRepository` theo `IRepository<Room>`; Hoàng đã rà soát, chuẩn hóa tài liệu và kiểm thử lại `tests/test_room_repository.cpp` ngày 25/11/2026: 72 PASS, 0 FAIL.
- [x] `UserRepository` thuộc phạm vi phụ trách đã CHỐT MARKDOWN và triển khai DIP theo `IRepository<User>`; bỏ hai `unordered_map`, dùng `Repository<User>` làm lưu trữ duy nhất và chuẩn hóa validation. `tests/test_user_repository.cpp` ngày 25/11/2026 đạt 69 PASS, 0 FAIL.
- [x] `MemberRepository` thuộc phạm vi phụ trách đã CHỐT MARKDOWN, triển khai DIP theo `IRepository<Member>`, bỏ `membersById`, giữ `membersByPhone` và dùng `Validator::isValidPhone()`. Mã nguồn, tài liệu và `tests/test_member_repository.cpp` đã được chuẩn hóa, kiểm thử ngày 25/11/2026 đạt 204 PASS, 0 FAIL; liên kết Invoice chưa kiểm thử.
- [ ] Refactor các Repository thuộc phạm vi phân công trong `../PhanCongCongViec.md`.

Mẫu chuẩn đã hoàn thành: `MovieRepository.h/.cpp` và `MovieRepository.md` đã CHỐT MARKDOWN, refactor DIP và validation, kiểm thử qua `tests/test_irepository.cpp` ngày 24/11/2026 (81 PASS, 0 FAIL). Kiệt áp dụng mẫu theo `AGENTS.md` và điều chỉnh logic cho từng Model; RoomRepository, UserRepository và MemberRepository đã được xác nhận trong phạm vi bài test riêng, các Repository khác của Kiệt chưa được xác nhận.
