# Phân công công việc PBL2 – Giai đoạn Refactor

## 1. Tổng quan

Tài liệu này quy định việc phân chia công việc giữa hai thành viên trong giai đoạn refactor và hoàn thiện hệ thống quản lý bán vé rạp chiếu phim PBL2.

**Thành viên:**
- Hoàng
- Kiệt

Trong giai đoạn trước, Hoàng chủ yếu phụ trách Backend và Kiệt chủ yếu phụ trách GUI.

Trong giai đoạn refactor, **cả hai thành viên cùng tham gia phát triển Backend, hoàn thiện chức năng, tích hợp GUI và kiểm thử hệ thống**.

Mục tiêu là phân chia công việc tương đối đồng đều, hạn chế phụ thuộc giữa hai thành viên và bảo đảm mã nguồn được triển khai theo cùng một quy chuẩn.

## 2. Nguyên tắc phân công

- Mỗi module có một người chịu trách nhiệm chính.
- Cả hai thành viên đều tham gia thiết kế, viết Markdown, triển khai code và kiểm thử.
- Phân chia theo độ phức tạp và khối lượng công việc, không chỉ theo số lượng module.
- Tuân thủ kiến trúc DIP với duy nhất một interface chung `IRepository<T>`.
- Phân quyền theo Permission-only.
- Phân chia rõ trách nhiệm giữa Model, Repository, Service và GUI.
- Thực hiện quy trình Markdown-first trước khi sửa code.
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
| Phim | MovieRepository, MovieService | MovieRepository đã refactor theo DIP và validation, CHỐT MARKDOWN, kiểm thử qua `test_irepository.cpp` (81 PASS, 0 FAIL); MovieService còn theo kế hoạch |
| Suất chiếu | ShowtimeRepository, ShowtimeService | Refactor và hoàn thiện nghiệp vụ |
| Vé | TicketRepository, TicketService | Refactor và hoàn thiện nghiệp vụ |
| Hóa đơn | InvoiceRepository, InvoiceService | Refactor và hoàn thiện nghiệp vụ |
| Bán vé | BookingService và các thành phần liên quan | Hoàn thiện quy trình bán vé |
| Hủy vé | CancellationService và các thành phần liên quan | Hoàn thiện quy trình hủy vé, hoàn tiền |
| Báo cáo | ReportService và các thành phần liên quan | Hoàn thiện thống kê, báo cáo |

**Trọng tâm:** Các nghiệp vụ liên quan đến lịch chiếu, giao dịch bán vé, hóa đơn, hủy vé và báo cáo.

### 4.2. Kiệt phụ trách

| Nhóm | Module | Công việc |
|---|---|---|
| Phòng chiếu | RoomRepository, RoomService | Kiệt đã triển khai DIP cho RoomRepository; Hoàng đã rà soát, chuẩn hóa tài liệu và kiểm thử lại `test_room_repository.cpp` ngày 25/11/2026 (72 PASS, 0 FAIL). RoomService còn theo kế hoạch |
| Ghế | SeatRepository, SeatService | Refactor và hoàn thiện nghiệp vụ |
| Nhân viên | UserRepository, UserService | UserRepository đã CHỐT MARKDOWN, triển khai DIP, bỏ hai `unordered_map`, chuẩn hóa validation và kiểm thử `test_user_repository.cpp` ngày 25/11/2026 (69 PASS, 0 FAIL); UserService còn theo kế hoạch |
| Xác thực | AuthService và các thành phần liên quan | Chuẩn hóa xác thực, Permission |
| Phân quyền | RolePermissionRepository và các thành phần liên quan | Refactor và kiểm thử quyền |
| Thành viên | MemberRepository, MemberService | Refactor và hoàn thiện nghiệp vụ |
| Giá vé | PricingRepository, PricingService | Refactor và hoàn thiện nghiệp vụ |

**Trọng tâm:** Các nghiệp vụ quản lý dữ liệu nền tảng, người dùng, phân quyền, thành viên và giá vé.

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

- Hoàng thực hiện các Repository thuộc nhóm được giao.
- Kiệt thực hiện các Repository thuộc nhóm được giao.
- Cùng kiểm thử các liên kết dữ liệu giữa các Repository.
- Cả Hoàng và Kiệt dùng `MovieRepository.h/.cpp` và `MovieRepository.md` làm mẫu cho các Concrete Repository được giao, điều chỉnh logic theo từng Model và theo quy trình trong `AGENTS.md`.

### Giai đoạn 03 – Refactor Service

- Mỗi thành viên refactor các Service tương ứng.
- Chuẩn hóa Permission và xử lý nghiệp vụ.
- Kiểm thử từng Service.

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
- Đồng bộ với `main` thường xuyên để hạn chế xung đột.
- Không tự ý sửa module của người khác nếu chưa thống nhất.

Các thay đổi ảnh hưởng đến interface chung, API của Service hoặc định dạng dữ liệu phải được trao đổi trước khi merge.

## 9. Theo dõi tiến độ

Tiến độ tổng thể được ghi tại:

`docs/development/TongQuanTienDo.md`

Tiến độ cá nhân được ghi tại:

- `docs/development/tasks/Task_Hoang.md`
- `docs/development/tasks/Task_Kiet.md`

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
