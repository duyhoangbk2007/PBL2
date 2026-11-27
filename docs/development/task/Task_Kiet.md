# Công việc của Kiệt — giai đoạn refactor

## Tiến độ Data Access

Hoàng và Kiệt làm Backend song song trên nhánh riêng từ mốc chung `5d663e3`. Kiệt chuyển trọng tâm sang các Service đã tồn tại; Hoàng tiếp tục Data Access. Tránh chỉnh sửa chồng chéo và thống nhất trước khi đổi API chung.

- [x] Phối hợp CHỐT MARKDOWN và xây dựng `IRepository<T>`.
- [x] Cập nhật tài liệu dùng chung `Repository.md` theo code hiện tại; kết quả `test_irepository.cpp` xác nhận gián tiếp `Repository<Movie>`, chưa xác nhận các Model khác.
- [x] Triển khai DIP cho `RoomRepository` theo `IRepository<Room>`; Hoàng đã rà soát, chuẩn hóa tài liệu và kiểm thử lại `tests/test_room_repository.cpp` ngày 25/11/2026: 72 PASS, 0 FAIL.
- [x] `UserRepository` thuộc phạm vi phụ trách đã CHỐT MARKDOWN và triển khai DIP theo `IRepository<User>`; bỏ hai `unordered_map`, dùng `Repository<User>` làm lưu trữ duy nhất và chuẩn hóa validation. `tests/test_user_repository.cpp` ngày 25/11/2026 đạt 69 PASS, 0 FAIL.
- [x] `MemberRepository` thuộc phạm vi phụ trách đã CHỐT MARKDOWN, triển khai DIP theo `IRepository<Member>`, bỏ `membersById`, giữ `membersByPhone` và dùng `Validator::isValidPhone()`. Mã nguồn, tài liệu và `tests/test_member_repository.cpp` đã được chuẩn hóa, kiểm thử ngày 25/11/2026 đạt 204 PASS, 0 FAIL; liên kết Invoice chưa kiểm thử.
- [x] Ghi nhận dependency dùng chung `ShowtimeRepository` (Hoàng phụ trách) đã CHỐT MARKDOWN, triển khai DIP và kiểm thử `tests/test_showtime_repository.cpp` ngày 26/11/2026: Passed 85, Failed 0, Total 85. Kết quả chỉ xác nhận phạm vi Repository; bảo vệ lịch sử Ticket/Invoice sẽ hoàn thiện tại ShowtimeService, không thuộc kết quả kiểm thử này.
- [ ] Đối chiếu các Repository còn lại với kế hoạch sau khi hoàn tất nhóm Service ưu tiên; không mở thêm Repository trong giai đoạn song song hiện tại.

- [x] `SeatRepository` đã CHỐT MARKDOWN, triển khai DIP; `tests/test_seat_repository.cpp` ngày 26/11/2026 đạt Passed 81, Failed 0, Total 81 trong phạm vi bài test.
- [ ] Kiểm thử nhánh lỗi nội bộ/rollback và hồi quy liên quan của SeatRepository; SeatService còn phải kiểm tra lịch sử Ticket trước khi xóa ghế.

Mẫu chuẩn đã hoàn thành: `MovieRepository.h/.cpp` và `MovieRepository.md` đã CHỐT MARKDOWN, refactor DIP và validation, kiểm thử qua `tests/test_irepository.cpp` ngày 24/11/2026 (81 PASS, 0 FAIL). Kiệt áp dụng mẫu theo `AGENTS.md` và điều chỉnh logic cho từng Model; RoomRepository, SeatRepository, UserRepository và MemberRepository đã được xác nhận trong phạm vi bài test riêng, các Repository khác của Kiệt chưa được xác nhận.

## Tiến độ Business Service

- [ ] Tiếp tục rà soát dependency thực tế của từng Service còn lại trước khi thiết kế.
- [x] `MovieService` đã CHỐT MARKDOWN, refactor DIP qua `IRepository<Movie>&`, DI qua constructor và Permission-only; giữ nguyên 12 hàm public. `.h/.cpp` khớp `MovieService.md` 100%. `tests/test_movie_service.cpp` ngày 26/11/2026 biên dịch C++17 và chạy đạt Passed: 41, Failed: 0, Total: 41; hoàn thành trong phạm vi đã kiểm thử.
- [x] `RoomService` đã CHỐT MARKDOWN, refactor DIP qua `IRepository<Room>&`, DI qua constructor và Permission-only; có 9 hàm public gồm `setRoomActive()`. `Room::getShowtimeAt()` đã bổ sung và ghi trong `models.md`; `.h/.cpp` của Service khớp `RoomService.md` 100%. `tests/test_room_service.cpp` ngày 27/11/2026 biên dịch C++17 và chạy đạt Passed: 43, Failed: 0, Total: 43; hoàn thành trong phạm vi đã kiểm thử.
- [x] `AuthService` đã CHỐT MARKDOWN và hoàn thành trong phạm vi kiểm thử: DIP qua `IRepository<User>&`, DI qua constructor, Permission-only; giữ `RolePermissionRepository&` đặc thù. Có 7 hàm public, 2 helper private, bỏ `isAdmin()`, `isStaff()`, `requireAdmin()`; `.h/.cpp` khớp `AuthService.md` 100%. Biên dịch C++17 ngày 27/11/2026 và chạy `tests/test_auth_service.cpp` đạt Passed: 31, Failed: 0, Total: 31; hồi quy MovieService 41/41 PASS, RoomService 43/43 PASS.
- [x] `StaffService` đã CHỐT MARKDOWN và hoàn thành trong phạm vi kiểm thử: DIP qua `IRepository<User>&`, DI, Permission-only; 9 hàm public, không có helper private. `setStaffActive()` thay `deactivateStaff()` và dùng `STAFF_DEACTIVATE` cho cả bật/tắt. `.h/.cpp` khớp `StaffService.md` 100%; biên dịch C++17 và chạy ngày 27/11/2026: StaffService 32/32 PASS, AuthService 31/31 PASS, MovieService 41/41 PASS, RoomService 43/43 PASS.
- [ ] Ưu tiên `MemberService`: chuyển phụ thuộc `MemberRepository` đã refactor sang `IRepository<Member>`; liên kết Invoice của Repository chưa kiểm thử.
- [ ] Ưu tiên `ShowtimeService`: chuyển phụ thuộc `ShowtimeRepository`, `MovieRepository`, `RoomRepository` sang `IRepository<T>` phù hợp; kiểm tra Ticket/Invoice khi hủy, xóa suất chờ Hoàng refactor và kiểm thử hai Repository tương ứng.
- [ ] Với từng Service, thực hiện rà soát → thiết kế → CHỐT MARKDOWN → code → test → cập nhật tiến độ; kiểm tra DIP, Permission-only, quy tắc nghiệp vụ và quy ước code trong `AGENTS.md`.

Kiệt chỉ refactor Service đã tồn tại trong giai đoạn này, không triển khai Service mới. Các phần còn phụ thuộc Repository chưa sẵn sàng được thực hiện sau và chỉ đánh dấu hoàn thành khi có kết quả kiểm thử thực tế.

`MovieService.md` là mẫu chuẩn về cấu trúc, DIP/DI, Permission-only và cách trình bày cho các Service tiếp theo; không sao chép logic nghiệp vụ. Giới hạn kết quả hiện tại: chưa kiểm thử Repository giả lập, nhánh ngoại lệ nội bộ của add, lỗi ghi file, hồi quy GUI/caller và tích hợp load Showtime thực tế.

Giới hạn kết quả RoomService: chưa kiểm thử `startAt` bằng đúng thời điểm hiện tại, lỗi ghi file, tích hợp nạp Showtime thực tế và hồi quy GUI/caller.

Giới hạn kết quả AuthService: chưa kiểm thử tái sử dụng User ID, mọi dạng TXT quyền sai định dạng và hồi quy GUI/toàn ứng dụng. BookingService, ReportService và các Service còn dùng API AuthService cũ sẽ được xử lý khi đến lượt refactor.

Giới hạn kết quả StaffService: chưa kiểm thử lỗi ghi TXT và tích hợp nạp Invoice từ Repository thực. `tests/legacy_test/test_staff_service.cpp` còn sáu lời gọi `deactivateStaff()`; sẽ xử lý khi cập nhật test legacy, không sửa test để ép PASS.
