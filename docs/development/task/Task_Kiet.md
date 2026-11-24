# Công việc của Kiệt — giai đoạn refactor

## Tiến độ Data Access

- [x] Phối hợp CHỐT MARKDOWN và xây dựng `IRepository<T>`.
- [x] Cập nhật tài liệu dùng chung `Repository.md` theo code hiện tại; kết quả `test_irepository.cpp` xác nhận gián tiếp `Repository<Movie>`, chưa xác nhận các Model khác.
- [ ] Refactor các Repository thuộc phạm vi phân công trong `../PhanCongCongViec.md`.

Mẫu chung đã sẵn sàng: `MovieRepository.h/.cpp` và `MovieRepository.md` đã CHỐT MARKDOWN, refactor và được kiểm thử qua `tests/test_irepository.cpp` ngày 24/11/2026 (81 PASS, 0 FAIL). Kiệt tham chiếu mẫu theo `AGENTS.md` và điều chỉnh logic cho từng Model; kết quả test này chưa xác nhận các Repository của Kiệt.
