# Công việc của Hoàng — giai đoạn refactor

## Tiến độ Data Access

- [x] Phối hợp CHỐT MARKDOWN và xây dựng `IRepository<T>`.
- [x] Refactor `MovieRepository` theo `IRepository<Movie>` và quy tắc validation; CHỐT MARKDOWN, đồng bộ `.cpp` với tài liệu.
- [x] Biên dịch và chạy `tests/test_irepository.cpp` ngày 24/11/2026: 81 PASS, 0 FAIL.
- [x] Hoàn thành rà soát `Repository.h` và đồng bộ `Repository.md`; kiểm thử gián tiếp qua bài test trên.
- [x] Rà soát, chuẩn hóa `RoomRepository.md` theo `.h/.cpp` thực tế và kiểm thử lại `tests/test_room_repository.cpp` ngày 25/11/2026: 72 PASS, 0 FAIL. Kiệt là người triển khai DIP cho RoomRepository.
- [x] Ghi nhận kết quả kiểm thử `UserRepository` ngày 25/11/2026: `tests/test_user_repository.cpp` biên dịch và chạy đạt 69 PASS, 0 FAIL; phạm vi xác nhận giới hạn ở bài test Repository.
- [x] Rà soát và chuẩn hóa tài liệu `MemberRepository.md`, đồng bộ mã nguồn và ghi nhận `tests/test_member_repository.cpp` ngày 25/11/2026: 204 PASS, 0 FAIL. Chưa kiểm thử liên kết Invoice.
- [x] `ShowtimeRepository.md` đã CHỐT MARKDOWN; `.h/.cpp` đã triển khai và khớp 100% với code trong tài liệu. `tests/test_showtime_repository.cpp` ngày 26/11/2026 biên dịch và chạy đạt Passed 85, Failed 0, Total 85 trong phạm vi bài test.
- [ ] Kiểm thử nhánh lỗi nội bộ/rollback do ngoại lệ và kiểm hồi quy liên quan của ShowtimeRepository. Bảo vệ lịch sử Ticket/Invoice sẽ hoàn thiện tại ShowtimeService sau khi các Repository liên quan được refactor.
- [ ] Refactor các Repository còn lại thuộc phạm vi phân công trong `../PhanCongCongViec.md`.

Hoàng tham chiếu `MovieRepository.h/.cpp` và `MovieRepository.md` theo `AGENTS.md`, điều chỉnh logic cho từng Model. Kết quả RoomRepository, UserRepository, MemberRepository và ShowtimeRepository chỉ xác nhận phạm vi các bài test tương ứng; các Repository khác chưa được xác minh trong lần cập nhật này.
