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
| 02 | Xây dựng `IRepository<T>` và chuẩn hóa tầng Data Access | Đang thực hiện: IRepository, Repository, MovieRepository, RoomRepository, UserRepository, MemberRepository và ShowtimeRepository hoàn thành trong phạm vi đã ghi nhận; các Repository khác chưa xác nhận |
| 03 | Refactor các Business Service theo DIP và Permission-only | Chưa bắt đầu |
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
- [ ] Chuẩn hóa các Concrete Repository theo interface chung.
- [ ] Bảo đảm tính toàn vẹn dữ liệu và các liên kết giữa đối tượng.
- [ ] Kiểm thử các Repository sau khi refactor.

**Kết quả cần đạt:** Các Repository phù hợp với kiến trúc DIP và giữ nguyên hành vi nghiệp vụ đã chốt.

**Kết quả đã xác minh (24/11/2026):** Sau điều chỉnh validation, `tests/test_irepository.cpp` biên dịch bằng C++17 cùng các `.cpp` trong `src/` (trừ `main.cpp`) và chạy đạt 81 PASS, 0 FAIL. Kết quả xác nhận interface, MovieRepository và gián tiếp `Repository<Movie>` trong phạm vi bài test; chưa xác nhận toàn bộ tầng Data Access.

**Kết quả đã xác minh (25/11/2026):** `tests/test_room_repository.cpp` biên dịch bằng C++17 cùng các `.cpp` trong `src/` (trừ `main.cpp`) và chạy đạt 72 PASS, 0 FAIL. Kết quả xác nhận `RoomRepository` trong phạm vi bài test; `RoomService` và các trường hợp chưa được bài test bao phủ vẫn theo kế hoạch.

**Kết quả UserRepository đã xác minh (25/11/2026):** `tests/test_user_repository.cpp` biên dịch bằng C++17 cùng các `.cpp` trong `src/` (trừ `main.cpp`) và chạy đạt 69 PASS, 0 FAIL. Kết quả chỉ xác nhận các trường hợp trong bài test, chưa xác nhận toàn bộ Service hoặc các lỗi ghi file.

**Kết quả MemberRepository đã xác minh (25/11/2026):** `tests/test_member_repository.cpp` biên dịch bằng C++17 cùng các `.cpp` trong `src/` (trừ `main.cpp`) và chạy đạt 204 PASS, 0 FAIL. Kết quả xác nhận các ca trong bài test, gồm `Validator::isValidPhone()`; liên kết Invoice và lỗi setter chưa được kiểm thử.

**Kết quả ShowtimeRepository đã xác minh (26/11/2026):** `tests/test_showtime_repository.cpp` biên dịch bằng C++17 cùng các `.cpp` trong `src/` (trừ `main.cpp`) và chạy đạt **Passed: 85, Failed: 0, Total: 85**. Kết quả xác nhận chín hàm qua `IRepository<Showtime>`, load/save, CRUD, lịch và liên kết trong phạm vi bài test. Chưa kích hoạt các nhánh rollback do ngoại lệ ở add/update; bảo vệ lịch sử Ticket và Invoice sẽ hoàn thiện tại ShowtimeService.

### Giai đoạn 03 – Refactor Business Service

- [ ] Rà soát các Service hiện tại.
- [ ] Chuyển phụ thuộc Repository sang `IRepository<T>` tại các vị trí phù hợp.
- [ ] Chuẩn hóa kiểm tra Permission thông qua `AuthService`.
- [ ] Chuyển các điều kiện nghiệp vụ về đúng tầng Service.
- [ ] Kiểm thử từng Service.

**Kết quả cần đạt:** Các Service tuân thủ DIP, Permission-only và phân chia trách nhiệm rõ ràng.

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
- Các thay đổi lớn cần được review trước khi merge vào `main`.

## 7. Tài liệu liên quan

- `AGENTS.md`: Quy chuẩn chung của dự án.
- `docs/architecture/dependency_inversion.md`: Thiết kế DIP.
- `docs/architecture/PermissionDesign.md`: Thiết kế phân quyền.
- `docs/architecture/Business_UI_Contract.md`: Giao tiếp Backend và GUI.
- `docs/development/KeHoachRefactor.md`: Kế hoạch thực hiện chi tiết.
- `docs/development/PhanCongCongViec.md`: Phân công công việc.
- `docs/development/tasks/Task_Hoang.md`: Công việc của Hoàng.
- `docs/development/tasks/Task_Kiet.md`: Công việc của Kiệt.
- `docs/development/pre_refactor/`: Tài liệu phát triển giai đoạn cũ.
