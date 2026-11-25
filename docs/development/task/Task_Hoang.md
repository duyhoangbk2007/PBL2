# Công việc của Hoàng — giai đoạn refactor

## Tiến độ Data Access

- [x] Phối hợp CHỐT MARKDOWN và xây dựng `IRepository<T>`.
- [x] Refactor `MovieRepository` theo `IRepository<Movie>` và quy tắc validation; CHỐT MARKDOWN, đồng bộ `.cpp` với tài liệu.
- [x] Biên dịch và chạy `tests/test_irepository.cpp` ngày 24/11/2026: 81 PASS, 0 FAIL.
- [x] Hoàn thành rà soát `Repository.h` và đồng bộ `Repository.md`; kiểm thử gián tiếp qua bài test trên.
- [x] Rà soát, chuẩn hóa `RoomRepository.md` theo `.h/.cpp` thực tế và kiểm thử lại `tests/test_room_repository.cpp` ngày 25/11/2026: 72 PASS, 0 FAIL. Kiệt là người triển khai DIP cho RoomRepository.
- [x] Ghi nhận kết quả kiểm thử `UserRepository` ngày 25/11/2026: `tests/test_user_repository.cpp` biên dịch và chạy đạt 69 PASS, 0 FAIL; phạm vi xác nhận giới hạn ở bài test Repository.
- [ ] Refactor các Repository còn lại thuộc phạm vi phân công trong `../PhanCongCongViec.md`.

Hoàng tham chiếu `MovieRepository.h/.cpp` và `MovieRepository.md` theo `AGENTS.md`, điều chỉnh logic cho từng Model. Kết quả RoomRepository và UserRepository chỉ xác nhận phạm vi các bài test tương ứng; các Repository khác chưa được xác minh trong lần cập nhật này.
