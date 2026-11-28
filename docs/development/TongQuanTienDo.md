# Tổng quan tiến độ PBL2 – Giai đoạn Refactor

## 1. Giới thiệu

Tài liệu này theo dõi tiến độ phát triển và hoàn thiện hệ thống quản lý bán vé rạp chiếu phim PBL2 trong giai đoạn refactor.

Dự án đã trải qua giai đoạn xây dựng ban đầu. Trong giai đoạn mới, nhóm tập trung vào việc chuẩn hóa kiến trúc, hoàn thiện các chức năng còn thiếu, tích hợp giao diện Qt và kiểm thử toàn bộ hệ thống.

Tài liệu phát triển trước refactor được lưu tại `docs/development/pre_refactor/` để tiện tra cứu.

## 2. Mục tiêu

Giai đoạn refactor tập trung vào các mục tiêu:

- Áp dụng Dependency Inversion Principle (DIP) với duy nhất một interface chung `IRepository<T>`.
- Chuẩn hóa phân quyền RBAC dựa trên Permission, không kiểm tra vai trò viết cứng trong Service.
- Phân chia rõ trách nhiệm giữa Model, Repository, Service và GUI.
- Giữ lại `Repository<T>` và các cấu trúc dữ liệu đang sử dụng.
- Hoàn thiện những chức năng Backend chưa triển khai.
- Đồng bộ Backend và giao diện Qt thông qua Business – UI Contract.
- Kiểm thử toàn bộ chức năng trước khi hoàn thiện đồ án.

Ưu tiên giữ nguyên các quy tắc nghiệp vụ đã chốt, tránh thay đổi không cần thiết.

Hoàng và Kiệt refactor Backend song song trên nhánh riêng từ mốc chung `5d663e3`: Hoàng tiếp tục Data Access, Kiệt refactor các Service đã tồn tại, tránh chỉnh sửa chồng chéo.

## 3. Quy trình thực hiện

Mỗi module được triển khai theo quy trình **Markdown-first**:

1. Đọc code và tài liệu hiện tại.
2. Thiết kế API, các thuộc tính và hàm cần thiết.
3. Viết đầy đủ code `.h` và `.cpp` dự kiến vào tài liệu Markdown.
4. Giải thích chi tiết từng hàm, luồng hoạt động và các đoạn code.
5. Review và chốt Markdown.
6. Triển khai code đúng theo Markdown đã chốt.
7. Biên dịch, kiểm thử và sửa lỗi thông qua Markdown.
8. Đối chiếu code với tài liệu và cập nhật tiến độ.

Không đánh dấu hoàn thành một module nếu chưa có kết quả kiểm thử phù hợp.

## 4. Lộ trình tổng quan

| Giai đoạn | Nội dung | Trạng thái |
|---|---|---|
| 01 | Chuẩn hóa tài liệu và thiết kế kiến trúc refactor | Đang thực hiện |
| 02 | Xây dựng `IRepository<T>` và chuẩn hóa tầng Data Access | Đang thực hiện: IRepository, Repository, MovieRepository, RoomRepository, SeatRepository, UserRepository, MemberRepository và ShowtimeRepository hoàn thành trong phạm vi đã ghi nhận; các Repository khác chưa xác nhận |
| 03 | Refactor các Business Service theo DIP và Permission-only | Đang thực hiện: MovieService, RoomService, AuthService, StaffService, MemberService và PricingService hoàn thành trong phạm vi đã kiểm thử; các Service còn lại chưa xác nhận |
| 04 | Hoàn thiện các nghiệp vụ Backend còn thiếu | Chưa bắt đầu |
| 05 | Đồng bộ Business – UI Contract và tích hợp giao diện Qt | Chưa xác minh |
| 06 | Hoàn thiện giao diện và các luồng sử dụng | Chưa xác minh |
| 07 | Kiểm thử tích hợp, xử lý lỗi và rà soát toàn hệ thống | Chưa bắt đầu |
| 08 | Hoàn thiện tài liệu, đóng gói và chuẩn bị báo cáo | Chưa bắt đầu |

Trạng thái có thể sử dụng:

- **Chưa bắt đầu:** Chưa triển khai công việc trong giai đoạn refactor.
- **Đang thực hiện:** Đã bắt đầu nhưng chưa đáp ứng đầy đủ điều kiện hoàn thành.
- **Chờ kiểm thử:** Đã triển khai, đang chờ xác minh.
- **Hoàn thành:** Đã kiểm tra đầy đủ và đáp ứng yêu cầu.
- **Chưa xác minh:** Chưa đối chiếu đủ tài liệu, mã nguồn hoặc kết quả thực tế.

## 5. Nội dung các giai đoạn

### Giai đoạn 01 – Chuẩn hóa tài liệu và kiến trúc

- [ ] Hoàn thiện `AGENTS.md`.
- [ ] Hoàn thiện `README.md`.
- [ ] Chuẩn hóa tài liệu trong `docs/architecture/`.
- [ ] Chuẩn hóa tài liệu trong `docs/overview/`.
- [ ] Lưu tài liệu phát triển cũ vào `pre_refactor/`.
- [ ] Xây dựng kế hoạch refactor và phân công công việc mới.

**Kết quả cần đạt:** Nhóm thống nhất kiến trúc, quy trình và phạm vi trước khi sửa code.

### Giai đoạn 02 – Refactor Data Access

- [x] Thiết kế, CHỐT MARKDOWN, triển khai và kiểm thử `IRepository<T>`.
- [x] Refactor `MovieRepository` theo interface và quy tắc validation; CHỐT MARKDOWN, đồng bộ `.cpp` với tài liệu và kiểm thử qua `test_irepository.cpp`.
- [x] Hoàn thành rà soát `Repository<T>` và cập nhật `Repository.md`; kiểm thử gián tiếp qua `test_irepository.cpp` (81 PASS, 0 FAIL). Chưa kiểm thử trực tiếp mọi API của template.
- [x] Kiệt triển khai DIP cho `RoomRepository`; Hoàng rà soát, chuẩn hóa tài liệu và kiểm thử lại. Mã `.h/.cpp` khớp `RoomRepository.md`; `tests/test_room_repository.cpp` ngày 25/11/2026 đạt 72 PASS, 0 FAIL.
- [x] `UserRepository` đã CHỐT MARKDOWN, triển khai DIP và đồng bộ `.h/.cpp` với tài liệu; bỏ hai `unordered_map`, tập trung validation riêng trong `isValidUser()` và dùng `Repository<User>` cho kiểm tra lưu trữ chung. `tests/test_user_repository.cpp` ngày 25/11/2026 đạt 69 PASS, 0 FAIL.
- [x] `MemberRepository` đã refactor theo DIP, bỏ `membersById`, giữ chỉ mục phone, chuẩn hóa validation qua `Validator::isValidPhone()` và đồng bộ mã nguồn, tài liệu. `tests/test_member_repository.cpp` ngày 25/11/2026 biên dịch và chạy đạt 204 PASS, 0 FAIL; chưa kiểm thử liên kết Invoice.
- [x] `ShowtimeRepository` đã CHỐT MARKDOWN, triển khai DIP và đối chiếu `.h/.cpp` khớp 100% với hai khối code trong `ShowtimeRepository.md`. `tests/test_showtime_repository.cpp` ngày 26/11/2026 biên dịch và chạy đạt Passed 85, Failed 0, Total 85; nhánh lỗi nội bộ/rollback do ngoại lệ và kiểm hồi quy liên quan còn chờ kiểm thử.
- [x] `SeatRepository` đã CHỐT MARKDOWN, triển khai DIP, chuẩn hóa comment và đối chiếu `.h/.cpp` với hai khối code trong `SeatRepository.md`. `tests/test_seat_repository.cpp` ngày 26/11/2026 biên dịch và chạy đạt Passed 81, Failed 0, Total 81; nhánh lỗi nội bộ/rollback do ngoại lệ và kiểm hồi quy liên quan còn chờ kiểm thử. Điều kiện lịch sử Ticket thuộc SeatService.
- [ ] Chuẩn hóa các Concrete Repository theo interface chung.
- [ ] Tiếp tục rà soát `SeatRepository` trong phạm vi chưa kiểm thử; refactor và kiểm thử `InvoiceRepository`, `TicketRepository` còn theo kế hoạch.
- [ ] Bảo đảm tính toàn vẹn dữ liệu và các liên kết giữa đối tượng.
- [ ] Kiểm thử các Repository sau khi refactor.

**Kết quả cần đạt:** Các Repository phù hợp với kiến trúc DIP và giữ nguyên hành vi nghiệp vụ đã chốt.

**Kết quả đã xác minh (24/11/2026):** Sau điều chỉnh validation, `tests/test_irepository.cpp` biên dịch bằng C++17 cùng các `.cpp` trong `src/` (trừ `main.cpp`) và chạy đạt 81 PASS, 0 FAIL. Kết quả xác nhận interface, MovieRepository và gián tiếp `Repository<Movie>` trong phạm vi bài test; chưa xác nhận toàn bộ tầng Data Access.

**Kết quả đã xác minh (25/11/2026):** `tests/test_room_repository.cpp` biên dịch bằng C++17 cùng các `.cpp` trong `src/` (trừ `main.cpp`) và chạy đạt 72 PASS, 0 FAIL. Kết quả xác nhận `RoomRepository` trong phạm vi bài test; kết quả `RoomService` được ghi riêng ở giai đoạn 03.

**Kết quả UserRepository đã xác minh (25/11/2026):** `tests/test_user_repository.cpp` biên dịch bằng C++17 cùng các `.cpp` trong `src/` (trừ `main.cpp`) và chạy đạt 69 PASS, 0 FAIL. Kết quả chỉ xác nhận các trường hợp trong bài test, chưa xác nhận toàn bộ Service hoặc các lỗi ghi file.

**Kết quả MemberRepository đã xác minh (25/11/2026):** `tests/test_member_repository.cpp` biên dịch bằng C++17 cùng các `.cpp` trong `src/` (trừ `main.cpp`) và chạy đạt 204 PASS, 0 FAIL. Kết quả xác nhận các ca trong bài test, gồm `Validator::isValidPhone()`; liên kết Invoice và lỗi setter chưa được kiểm thử.

**Kết quả ShowtimeRepository đã xác minh (26/11/2026):** `tests/test_showtime_repository.cpp` biên dịch bằng C++17 cùng các `.cpp` trong `src/` (trừ `main.cpp`) và chạy đạt **Passed: 85, Failed: 0, Total: 85**. Kết quả xác nhận chín hàm qua `IRepository<Showtime>`, load/save, CRUD, lịch và liên kết trong phạm vi bài test. Chưa kích hoạt các nhánh rollback do ngoại lệ ở add/update; bảo vệ lịch sử Ticket và Invoice sẽ hoàn thiện tại ShowtimeService.

**Kết quả SeatRepository đã xác minh (26/11/2026):** `tests/test_seat_repository.cpp` biên dịch bằng C++17 cùng các `.cpp` trong `src/` (trừ `main.cpp`) và chạy đạt **Passed: 81, Failed: 0, Total: 81**. Kết quả xác nhận interface, load/save, CRUD, truy vấn, liên kết Seat–Room, rollback load khi Seat sau lỗi và destructor trong phạm vi bài test. Chưa kích hoạt các nhánh lỗi nội bộ/rollback do ngoại lệ; SeatService vẫn phải kiểm tra lịch sử Ticket, kể cả vé đã hủy, trước khi xóa ghế.

### Giai đoạn 03 – Refactor Business Service

- [x] `MovieService` đã CHỐT MARKDOWN, refactor DIP qua `IRepository<Movie>&` và DI qua constructor, dùng Permission-only, giữ 12 hàm public; `.h/.cpp` khớp Markdown 100%. `tests/test_movie_service.cpp` ngày 26/11/2026 biên dịch C++17 và chạy đạt Passed: 41, Failed: 0, Total: 41 trong phạm vi bài test.
- [x] `RoomService` đã CHỐT MARKDOWN, refactor DIP qua `IRepository<Room>&`, DI qua constructor và Permission-only; có 9 hàm public gồm `setRoomActive()`. `Room::getShowtimeAt()` đã bổ sung; `.h/.cpp` của Service khớp Markdown 100%. `tests/test_room_service.cpp` ngày 27/11/2026 biên dịch C++17 và chạy đạt Passed: 43, Failed: 0, Total: 43 trong phạm vi bài test.
- [x] `AuthService` đã CHỐT MARKDOWN, refactor DIP qua `IRepository<User>&` và DI qua constructor, Permission-only; giữ `RolePermissionRepository&` đặc thù, có 7 hàm public và 2 helper private, bỏ `isAdmin()`, `isStaff()`, `requireAdmin()`. `.h/.cpp` khớp Markdown 100%; `tests/test_auth_service.cpp` ngày 27/11/2026 biên dịch C++17 và chạy đạt Passed: 31, Failed: 0, Total: 31.
- [x] `StaffService` đã CHỐT MARKDOWN, refactor DIP qua `IRepository<User>&`, DI và Permission-only; có 9 hàm public, không có helper private. Thay `deactivateStaff()` bằng `setStaffActive()` dùng `STAFF_DEACTIVATE` cho cả bật/tắt; `.h/.cpp` khớp Markdown 100%. `tests/test_staff_service.cpp` ngày 27/11/2026 biên dịch C++17 và chạy đạt Passed: 32, Failed: 0, Total: 32.
- [x] `MemberService` đã CHỐT MARKDOWN, refactor DIP qua `IRepository<Member>&`, DI và Permission-only; giữ 8 hàm public, không có hàm private. Tìm theo phone qua `getAt()`; không đổi nghiệp vụ thành viên và điểm. `.h/.cpp` khớp Markdown 100%. `tests/test_member_service.cpp` ngày 27/11/2026 biên dịch C++17 và chạy đạt Passed: 45, Failed: 0, Total: 45; rollback khi load phone trùng và load lại sau khi sửa TXT đạt.
- [x] `PricingService` đã CHỐT MARKDOWN, giữ ngoại lệ DIP `PricingRepository&` qua DI, Permission-only với `PRICING_VIEW`/`PRICING_UPDATE`; 8 hàm public, không có hàm private. Bỏ validation trùng ở hai hàm update, dùng `numeric_limits<int>::max()` và hạn chế tràn trung gian khi tính giảm giá; giữ nghiệp vụ, schema TXT và ghi file trước RAM. `.h/.cpp` khớp Markdown 100%. `tests/test_pricing_service.cpp` ngày 28/11/2026 biên dịch C++17 và chạy đạt Passed: 67, Failed: 0, Total: 67.
- [ ] Kiệt tiếp tục rà soát dependency thực tế của `ShowtimeService` đã tồn tại; không triển khai Service mới trong giai đoạn hiện tại.
- [ ] Ghi rõ phần chờ: các Service còn gọi API AuthService cũ sẽ xử lý khi đến lượt; `ShowtimeService` cần Ticket/Invoice để bảo vệ lịch sử khi hủy, xóa suất; liên kết Invoice của `MemberRepository` chưa kiểm thử.
- [ ] Chuyển phụ thuộc Repository sang `IRepository<T>` tại các vị trí phù hợp.
- [ ] Chuẩn hóa kiểm tra Permission thông qua `AuthService`.
- [ ] Chuyển các điều kiện nghiệp vụ về đúng tầng Service.
- [ ] Kiểm thử từng Service.

**Kết quả cần đạt:** Các Service tuân thủ DIP, Permission-only và phân chia trách nhiệm rõ ràng.

**Kết quả MovieService đã xác minh (26/11/2026):** Biên dịch C++17 cùng toàn bộ `.cpp` trong `src/` trừ `main.cpp` thành công; chạy `tests/test_movie_service.cpp` đạt **Passed: 41, Failed: 0, Total: 41**. Hoàn thành refactor trong phạm vi đã kiểm thử. Chưa kiểm thử Repository giả lập, nhánh ngoại lệ nội bộ của add, lỗi ghi file, hồi quy GUI/caller và tích hợp load Showtime thực tế. Dùng `MovieService.md` làm mẫu cấu trúc, DIP/DI, Permission-only và bố cục tài liệu cho các Service tiếp theo, không sao chép logic nghiệp vụ máy móc.

**Kết quả RoomService đã xác minh (27/11/2026):** Biên dịch C++17 cùng toàn bộ `.cpp` phù hợp trong `src/` trừ `main.cpp` thành công; chạy `tests/test_room_service.cpp` đạt **Passed: 43, Failed: 0, Total: 43**. Hoàn thành refactor trong phạm vi đã kiểm thử. Chưa kiểm thử `startAt` bằng đúng thời điểm hiện tại, lỗi ghi file, tích hợp nạp Showtime thực tế và hồi quy GUI/caller.

**Kết quả AuthService đã xác minh (27/11/2026):** Biên dịch C++17 với các dependency cần thiết và chạy lần lượt `tests/test_auth_service.cpp` **31/31 PASS**, `tests/test_movie_service.cpp` **41/41 PASS**, `tests/test_room_service.cpp` **43/43 PASS**. Hoàn thành AuthService trong phạm vi đã kiểm thử. Chưa kiểm thử tái sử dụng User ID, mọi dạng TXT quyền sai định dạng và hồi quy GUI/toàn ứng dụng; các Service còn dùng API cũ sẽ được refactor sau.

**Kết quả StaffService đã xác minh (27/11/2026):** Biên dịch C++17 với dependency cần thiết và chạy lần lượt `tests/test_staff_service.cpp` **32/32 PASS**, `tests/test_auth_service.cpp` **31/31 PASS**, `tests/test_movie_service.cpp` **41/41 PASS**, `tests/test_room_service.cpp` **43/43 PASS**. Hoàn thành StaffService trong phạm vi đã kiểm thử. Chưa kiểm thử lỗi ghi TXT và tích hợp nạp Invoice từ Repository thực; sáu lời gọi API `deactivateStaff()` trong test legacy còn chờ cập nhật.

**Kết quả MemberService đã xác minh (27/11/2026):** Biên dịch C++17 với dependency cần thiết và chạy lần lượt `tests/test_member_service.cpp` **45/45 PASS**, `tests/test_staff_service.cpp` **32/32 PASS**, `tests/test_auth_service.cpp` **31/31 PASS**, `tests/test_movie_service.cpp` **41/41 PASS**, `tests/test_room_service.cpp` **43/43 PASS**. Hoàn thành MemberService trong phạm vi đã kiểm thử. Chưa kiểm thử trực tiếp lỗi ghi TXT và chưa dùng công cụ dò rò rỉ bộ nhớ.

**Kết quả PricingService đã xác minh (28/11/2026):** Biên dịch C++17 với dependency cần thiết và chạy PricingService **67/67 PASS**, MemberService **45/45 PASS**, StaffService **32/32 PASS**, AuthService **31/31 PASS**, MovieService **41/41 PASS**, RoomService **43/43 PASS**; tổng **259 PASS, 0 FAIL**. Lỗi ghi TXT đã được tái hiện: update trả false, cấu hình RAM giữ nguyên. Hoàn thành PricingService trong phạm vi đã kiểm thử; chưa kiểm chứng tính nguyên tử TXT khi lỗi ghi giữa chừng và tích hợp BookingService.

Mỗi Service đi theo thứ tự rà soát → thiết kế → CHỐT MARKDOWN → code → test → cập nhật tiến độ; đối chiếu quy tắc nghiệp vụ và quy ước code trong `AGENTS.md`. Các phần phụ thuộc Repository chưa refactor, chưa kiểm thử được thực hiện sau khi dependency sẵn sàng, không đánh dấu hoàn thành trước.

### Giai đoạn 04 – Hoàn thiện Backend

- [ ] Rà soát các chức năng Backend chưa hoàn thiện.
- [ ] Hoàn thiện nghiệp vụ quản lý và hủy suất chiếu.
- [ ] Rà soát quy trình bán vé, hóa đơn và hủy vé.
- [ ] Rà soát nghiệp vụ giá vé, ưu đãi và điểm thành viên.
- [ ] Hoàn thiện báo cáo, thống kê và các nghiệp vụ còn thiếu.
- [ ] Kiểm thử các luồng nghiệp vụ hoàn chỉnh.

**Kết quả cần đạt:** Backend đáp ứng đầy đủ các chức năng trong phạm vi đồ án.

### Giai đoạn 05 – Tích hợp Backend và GUI

- [ ] Rà soát Business – UI Contract.
- [ ] Thống nhất API giữa Hoàng và Kiệt.
- [ ] Kết nối giao diện Qt với các Business Service.
- [ ] Đồng bộ trạng thái dữ liệu và thông báo lỗi.
- [ ] Kiểm tra phân quyền trên giao diện và tại Service.

**Kết quả cần đạt:** Giao diện sử dụng đúng API và không trực tiếp xử lý nghiệp vụ hoặc thao tác file dữ liệu.

### Giai đoạn 06 – Hoàn thiện giao diện

- [ ] Hoàn thiện các màn hình quản lý.
- [ ] Hoàn thiện luồng bán vé và thanh toán tại quầy.
- [ ] Hoàn thiện tra cứu, hủy vé và báo cáo.
- [ ] Xử lý các trạng thái lỗi và dữ liệu không hợp lệ.
- [ ] Kiểm tra trải nghiệm sử dụng và điều hướng.

**Kết quả cần đạt:** Các chức năng trong phạm vi đồ án có thể sử dụng thông qua giao diện Qt.

### Giai đoạn 07 – Kiểm thử toàn hệ thống

- [ ] Kiểm thử các Model, Repository và Service.
- [ ] Kiểm thử các luồng nghiệp vụ liên quan nhiều module.
- [ ] Kiểm thử quyền truy cập và các trường hợp bị từ chối.
- [ ] Kiểm thử đọc/ghi dữ liệu và tính toàn vẹn liên kết.
- [ ] Kiểm thử tích hợp giao diện Qt.
- [ ] Sửa lỗi và chạy lại các bài kiểm thử liên quan.

**Kết quả cần đạt:** Hệ thống hoạt động ổn định và các chức năng đã hoàn thành có kết quả kiểm thử xác nhận.

### Giai đoạn 08 – Hoàn thiện đồ án

- [ ] Rà soát tài liệu và mã nguồn.
- [ ] Hoàn thiện hướng dẫn cài đặt, biên dịch và chạy chương trình.
- [ ] Chuẩn bị dữ liệu minh họa.
- [ ] Đóng gói ứng dụng.
- [ ] Chuẩn bị báo cáo và demo.

**Kết quả cần đạt:** Dự án sẵn sàng để trình bày và bảo vệ.

## 6. Nguyên tắc cập nhật tiến độ

- Chỉ đánh dấu hoàn thành khi có bằng chứng từ code, tài liệu và kết quả kiểm thử.
- Không mặc định các chức năng của giai đoạn cũ đã hoàn thiện.
- Các thay đổi về thiết kế phải được cập nhật trong Markdown trước khi triển khai code.
- Không tự ý sửa các module ngoài phạm vi đang thực hiện.
- Hoàng và Kiệt cập nhật công việc tương ứng trong các file Task riêng.
- Hai người làm trên nhánh riêng từ mốc `5d663e3`, thống nhất trước khi chỉnh sửa cùng một module hoặc API chung.
- Các thay đổi lớn cần được review trước khi merge vào `main`.

## 7. Tài liệu liên quan

- `AGENTS.md`: Quy chuẩn chung của dự án.
- `docs/architecture/dependency_inversion.md`: Thiết kế DIP.
- `docs/architecture/PermissionDesign.md`: Thiết kế phân quyền.
- `docs/architecture/Business_UI_Contract.md`: Giao tiếp Backend và GUI.
- `docs/development/KeHoachRefactor.md`: Kế hoạch thực hiện chi tiết.
- `docs/development/PhanCongCongViec.md`: Phân công công việc.
- `docs/development/task/Task_Hoang.md`: Công việc của Hoàng.
- `docs/development/task/Task_Kiet.md`: Công việc của Kiệt.
- `docs/development/pre_refactor/`: Tài liệu phát triển giai đoạn cũ.
