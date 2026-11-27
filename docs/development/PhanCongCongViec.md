# Phân công công việc PBL2 – Giai đoạn Refactor

## 1. Tổng quan

Tài liệu này quy định việc phân chia công việc giữa hai thành viên trong giai đoạn refactor và hoàn thiện hệ thống quản lý bán vé rạp chiếu phim PBL2.

**Thành viên:**
- Hoàng
- Kiệt

Trong giai đoạn trước, Hoàng chủ yếu phụ trách Backend và Kiệt chủ yếu phụ trách GUI.

Trong giai đoạn refactor, **cả hai thành viên cùng tham gia phát triển Backend, hoàn thiện chức năng, tích hợp GUI và kiểm thử hệ thống**.

Hai nhánh refactor lấy commit `5d663e3` làm mốc chung. Từ mốc này, Hoàng tiếp tục tầng Data Access, Kiệt refactor các Service đã tồn tại; mỗi người làm việc trên nhánh riêng và tránh sửa chồng chéo.

Mục tiêu là phân chia công việc tương đối đồng đều, hạn chế phụ thuộc giữa hai thành viên và bảo đảm mã nguồn được triển khai theo cùng một quy chuẩn.

## 2. Nguyên tắc phân công

- Mỗi module có một người chịu trách nhiệm chính.
- Cả hai thành viên đều tham gia thiết kế, viết Markdown, triển khai code và kiểm thử.
- Phân chia theo độ phức tạp và khối lượng công việc, không chỉ theo số lượng module.
- Tuân thủ kiến trúc DIP với duy nhất một interface chung `IRepository<T>`.
- Phân quyền theo Permission-only.
- Phân chia rõ trách nhiệm giữa Model, Repository, Service và GUI.
- Thực hiện quy trình rà soát → thiết kế → CHỐT MARKDOWN → code → test → cập nhật tiến độ.
- Không tự ý thay đổi API hoặc module do người còn lại phụ trách.
- Những thay đổi ảnh hưởng đến cả hai phải được thống nhất trước khi triển khai.

## 3. Công việc chung

Các công việc sau do cả Hoàng và Kiệt cùng thực hiện.

| Công việc | Hoàng | Kiệt |
|---|---|---|
| Thống nhất kiến trúc và quy chuẩn | Phối hợp | Phối hợp |
| Thiết kế `IRepository<T>` | Phối hợp; đã CHỐT MARKDOWN, triển khai và kiểm thử | Phối hợp; đã CHỐT MARKDOWN, triển khai và kiểm thử |
| Review Markdown | Phối hợp | Phối hợp |
| Rà soát Business – UI Contract | Phối hợp | Phối hợp |
| Tích hợp Backend và GUI | Phối hợp | Phối hợp |
| Kiểm thử tích hợp | Phối hợp | Phối hợp |
| Review Pull Request | Phối hợp | Phối hợp |
| Hoàn thiện tài liệu và demo | Phối hợp | Phối hợp |

Các công việc chung không thay thế trách nhiệm chính của từng thành viên đối với module được giao.

## 4. Phân chia module Backend

### 4.1. Hoàng phụ trách

| Nhóm | Module | Công việc |
|---|---|---|
| Phim | MovieRepository | Đã refactor theo DIP và validation, CHỐT MARKDOWN, kiểm thử qua `test_irepository.cpp` (81 PASS, 0 FAIL) |
| Suất chiếu | ShowtimeRepository | Đã CHỐT MARKDOWN, triển khai DIP, đối chiếu code khớp tài liệu và kiểm thử `tests/test_showtime_repository.cpp` ngày 26/11/2026: Passed 85, Failed 0, Total 85; nhánh lỗi nội bộ/rollback do ngoại lệ chưa kiểm thử |
| Ghế | SeatRepository | Đã CHỐT MARKDOWN, triển khai `IRepository<Seat>` và kiểm thử `tests/test_seat_repository.cpp` ngày 26/11/2026: Passed 81, Failed 0, Total 81 trong phạm vi bài test; nhánh lỗi nội bộ/rollback và hồi quy liên quan chưa kiểm thử |
| Hóa đơn | InvoiceRepository | Tiếp tục refactor và kiểm thử; hiện chưa triển khai `IRepository<Invoice>` |
| Vé | TicketRepository | Tiếp tục refactor và kiểm thử; hiện chưa triển khai `IRepository<Ticket>` |

**Trọng tâm hiện tại:** Hoàn thiện tầng Data Access và các liên kết dữ liệu; các Service liên quan đến bán vé, hóa đơn, hủy vé và báo cáo thuộc giai đoạn sau, chưa ghi nhận đã refactor.

### 4.2. Kiệt phụ trách

| Nhóm | Module | Công việc |
|---|---|---|
| Phim | MovieService | Đã CHỐT MARKDOWN, refactor DIP/DI qua `IRepository<Movie>&`, Permission-only, giữ 12 hàm public; `.h/.cpp` khớp Markdown 100%. `tests/test_movie_service.cpp` ngày 26/11/2026 biên dịch C++17 và chạy đạt Passed: 41, Failed: 0, Total: 41; hoàn thành trong phạm vi bài test |
| Phòng chiếu | RoomService | Đã CHỐT MARKDOWN, refactor DIP/DI qua `IRepository<Room>&`, Permission-only; có 9 hàm public gồm `setRoomActive()`, `.h/.cpp` khớp Markdown 100%. `Room::getShowtimeAt()` đã bổ sung. `tests/test_room_service.cpp` ngày 27/11/2026 biên dịch C++17 và chạy đạt Passed: 43, Failed: 0, Total: 43; hoàn thành trong phạm vi bài test |
| Xác thực | AuthService | Đã CHỐT MARKDOWN và hoàn thành trong phạm vi kiểm thử: DIP qua `IRepository<User>&`, DI, Permission-only; giữ `RolePermissionRepository&` đặc thù, 7 hàm public/2 helper private, bỏ `isAdmin()`, `isStaff()`, `requireAdmin()`. `.h/.cpp` khớp Markdown 100%; ngày 27/11/2026 biên dịch C++17, AuthService 31/31 PASS, MovieService 41/41 PASS, RoomService 43/43 PASS |
| Nhân viên | StaffService | Đã CHỐT MARKDOWN, refactor DIP/DI qua `IRepository<User>&`, Permission-only; có 9 hàm public, không có helper private. `setStaffActive()` thay `deactivateStaff()` và dùng `STAFF_DEACTIVATE` cho cả bật/tắt; `.h/.cpp` khớp Markdown 100%. Ngày 27/11/2026 biên dịch C++17, StaffService 32/32 PASS, AuthService 31/31 PASS, MovieService 41/41 PASS, RoomService 43/43 PASS |
| Thành viên | MemberService | Đã CHỐT MARKDOWN, refactor DIP/DI qua `IRepository<Member>&`, Permission-only; giữ 8 hàm public, không có hàm private. Tìm phone qua `getAt()`, giữ nghiệp vụ thành viên/điểm; `.h/.cpp` khớp Markdown 100%. Ngày 27/11/2026 biên dịch C++17, MemberService 45/45 PASS, StaffService 32/32 PASS, AuthService 31/31 PASS, MovieService 41/41 PASS, RoomService 43/43 PASS; hoàn thành trong phạm vi đã kiểm thử |
| Suất chiếu | ShowtimeService | Ưu tiên refactor theo `IRepository<Showtime>`, `IRepository<Movie>`, `IRepository<Room>`; kiểm tra Ticket/Invoice khi hủy hoặc xóa suất phải chờ các Repository đó được Hoàng refactor và kiểm thử |
| Ghế | SeatService | Cần kiểm tra lịch sử Ticket, kể cả vé đã hủy, trước khi xóa ghế |
| Nhân viên | UserService | UserRepository đã kiểm thử 69 PASS, 0 FAIL; phần UserService ngoài StaffService còn theo kế hoạch |
| Giá vé | PricingService | PricingRepository vừa được cập nhật trên nhánh Hoàng; Service còn theo kế hoạch |

**Trọng tâm hiện tại:** Chỉ refactor Service đã tồn tại, không triển khai Service mới trong giai đoạn này. Rà soát dependency thực tế trước khi thiết kế từng Service; phần phụ thuộc Repository chưa sẵn sàng được ghi rõ và thực hiện sau.

`MovieService.md` là mẫu chuẩn cho Service tiếp theo về cấu trúc, DIP/DI, Permission-only và cách trình bày. Điều chỉnh quy tắc nghiệp vụ theo từng Service; kết quả MovieService chưa bao phủ hồi quy GUI/caller, lỗi ghi file và tích hợp load Showtime thực tế.

Kết quả RoomService chưa bao phủ `startAt` bằng đúng thời điểm hiện tại, lỗi ghi file, tích hợp nạp Showtime thực tế và hồi quy GUI/caller.

Kết quả AuthService chưa bao phủ tái sử dụng User ID, mọi dạng TXT quyền sai định dạng và hồi quy GUI/toàn ứng dụng. BookingService, ReportService và các Service còn gọi API AuthService cũ sẽ được xử lý khi đến lượt refactor.

Kết quả StaffService chưa bao phủ lỗi ghi TXT và tích hợp nạp Invoice từ Repository thực. Sáu lời gọi `deactivateStaff()` trong `tests/legacy_test/test_staff_service.cpp` vẫn chờ cập nhật; không sửa test legacy chỉ để tạo kết quả PASS.

Kết quả MemberService đã kiểm thử rollback khi load trùng số điện thoại và load lại sau khi sửa TXT. Chưa kiểm thử trực tiếp lỗi ghi TXT và chưa dùng công cụ dò rò rỉ bộ nhớ.

### 4.3. Các thành phần dùng chung

Những thành phần sau được cả hai thành viên phối hợp xử lý:

- `IRepository<T>`
- `Repository<T>`
- `IRepository<T>` đã hoàn thành; `Repository<T>` đã được rà soát và đồng bộ tài liệu, kiểm thử gián tiếp qua `test_irepository.cpp` (81 PASS, 0 FAIL), chưa xác nhận mọi Model.
- Các Models và quan hệ giữa Models
- `DoublyLinkedList<T>`
- Các tiện ích dùng chung trong `src/utils/`
- Cơ chế đọc/ghi file
- Cấu hình khởi tạo và liên kết các Service
- Business – UI Contract

**Quy định:** Không tự ý thay đổi các thành phần dùng chung khi chưa đánh giá ảnh hưởng đến module của người còn lại.

## 5. Trách nhiệm đối với mỗi module

Người được giao module phải thực hiện đầy đủ các bước:

1. Đọc và phân tích code hiện tại.
2. Xác định những điểm chưa phù hợp với kiến trúc mới.
3. Thiết kế lại module trong Markdown.
4. Viết đầy đủ code `.h` và `.cpp` dự kiến trong Markdown.
5. Giải thích từng hàm và luồng hoạt động chi tiết.
6. Gửi người còn lại review khi có thay đổi ảnh hưởng đến API chung.
7. Chốt Markdown trước khi triển khai.
8. Triển khai code đúng theo Markdown.
9. Viết và chạy các bài kiểm thử.
10. Đối chiếu code với tài liệu và cập nhật tiến độ.

Mỗi module chỉ được đánh dấu hoàn thành sau khi đáp ứng các yêu cầu thiết kế, biên dịch và kiểm thử.

## 6. Phân công GUI và tích hợp

Do Kiệt đã phát triển giao diện Qt trong giai đoạn trước, nhóm ưu tiên tận dụng những thành phần giao diện hiện có.

Tuy nhiên, giai đoạn mới không mặc định toàn bộ GUI là trách nhiệm riêng của Kiệt.

**Kiệt:**
- Rà soát và duy trì các thành phần GUI hiện có.
- Phối hợp cập nhật giao diện khi API thay đổi.
- Hỗ trợ tích hợp các chức năng quản lý.

**Hoàng:**
- Phối hợp kết nối các nghiệp vụ bán vé, hủy vé, hóa đơn và báo cáo với GUI.
- Kiểm tra luồng dữ liệu giữa GUI và Service.
- Hỗ trợ hoàn thiện các màn hình liên quan đến module phụ trách.

Hai thành viên thống nhất cách chia màn hình cụ thể trước khi bắt đầu giai đoạn tích hợp, dựa trên khối lượng công việc thực tế.

## 7. Thứ tự phối hợp thực hiện

### Giai đoạn 01 – Chuẩn bị kiến trúc

Cả hai thành viên:

- Thống nhất thiết kế `IRepository<T>`.
- Rà soát `Repository<T>` hiện tại.
- Thống nhất cách Dependency Injection vào Service.
- Thống nhất quy tắc Permission-only.
- Rà soát những thay đổi ảnh hưởng đến nhiều module.

### Giai đoạn 02 – Refactor Repository

- Hoàng tiếp tục `SeatRepository`, `InvoiceRepository`, `TicketRepository` để hoàn thiện Data Access.
- Kiệt đối chiếu dependency của các Service được giao với Repository đã refactor; không sửa Repository do Hoàng phụ trách.
- Cùng kiểm thử các liên kết dữ liệu giữa các Repository.
- Cả Hoàng và Kiệt dùng `MovieRepository.h/.cpp` và `MovieRepository.md` làm mẫu cho các Concrete Repository được giao, điều chỉnh logic theo từng Model và theo quy trình trong `AGENTS.md`.

### Giai đoạn 03 – Refactor Service

- Kiệt đã hoàn thành `MovieService`, `RoomService`, `AuthService`, `StaffService` và `MemberService` trong phạm vi kiểm thử; tiếp tục ưu tiên `ShowtimeService` đã tồn tại. Hoàng tiếp tục Data Access song song.
- Rà soát dependency thực tế; phần cần Repository chưa refactor xong được ghi rõ để thực hiện sau.
- Service tuân thủ DIP qua `IRepository<T>`, Permission-only, quy tắc nghiệp vụ và quy ước code trong `AGENTS.md`; kiểm thử từng Service trước khi đánh dấu hoàn thành.

### Giai đoạn 04 – Hoàn thiện Backend

- Hoàn thiện các chức năng còn thiếu.
- Kiểm thử các nghiệp vụ liên quan nhiều module.
- Phối hợp giải quyết các phụ thuộc giữa hai nhóm.

### Giai đoạn 05 – GUI và kiểm thử

- Tích hợp Backend với Qt.
- Hoàn thiện giao diện.
- Kiểm thử toàn bộ các luồng sử dụng.
- Sửa lỗi và rà soát kết quả.

## 8. Quy trình làm việc với GitHub

- Mỗi thành viên làm việc trên nhánh riêng.
- Không sửa trực tiếp trên `main`.
- Commit theo từng phần công việc rõ ràng.
- Khi hoàn thành một phần, tạo Pull Request.
- Người còn lại review trước khi merge.
- Hai nhánh refactor dùng commit `5d663e3` làm mốc chung; đồng bộ có kiểm tra để hạn chế xung đột.
- Không tự ý sửa module của người khác nếu chưa thống nhất.

Các thay đổi ảnh hưởng đến interface chung, API của Service hoặc định dạng dữ liệu phải được trao đổi trước khi merge.

## 9. Theo dõi tiến độ

Tiến độ tổng thể được ghi tại:

`docs/development/TongQuanTienDo.md`

Tiến độ cá nhân được ghi tại:

- `docs/development/task/Task_Hoang.md`
- `docs/development/task/Task_Kiet.md`

Mỗi công việc cần thể hiện:

- Nội dung cần thực hiện.
- Trạng thái hiện tại.
- Tài liệu thiết kế liên quan.
- Kết quả biên dịch và kiểm thử.
- Các lỗi hoặc vấn đề chưa giải quyết.

Không đánh dấu hoàn thành chỉ vì đã viết xong code.

## 10. Mục tiêu cuối cùng

Sau khi hoàn thành giai đoạn refactor:

- Backend tuân thủ kiến trúc đã thống nhất.
- Các chức năng nghiệp vụ được hoàn thiện và kiểm thử.
- GUI sử dụng đúng Business Service.
- Tài liệu và code được đồng bộ.
- Cả Hoàng và Kiệt đều nắm được kiến trúc, cách hoạt động và các nghiệp vụ chính của hệ thống.
- Dự án sẵn sàng cho việc đóng gói, báo cáo và bảo vệ.
