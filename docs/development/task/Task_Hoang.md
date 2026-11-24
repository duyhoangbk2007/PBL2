# Công việc của Hoàng — giai đoạn refactor

## Tiến độ Data Access

- [x] Phối hợp CHỐT MARKDOWN và xây dựng `IRepository<T>`.
- [x] Refactor `MovieRepository` theo `IRepository<Movie>` và quy tắc validation; CHỐT MARKDOWN, đồng bộ `.cpp` với tài liệu.
- [x] Biên dịch và chạy `tests/test_irepository.cpp` ngày 24/11/2026: 81 PASS, 0 FAIL.
- [x] Hoàn thành rà soát `Repository.h` và đồng bộ `Repository.md`; kiểm thử gián tiếp qua bài test trên.
- [ ] Refactor các Repository còn lại thuộc phạm vi phân công trong `../PhanCongCongViec.md`.

Hoàng tham chiếu `MovieRepository.h/.cpp` và `MovieRepository.md` theo `AGENTS.md`, điều chỉnh logic cho từng Model. Các test Repository khác chưa được xác minh trong lần cập nhật này.
