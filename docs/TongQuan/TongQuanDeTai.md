# Tổng quan đề tài và luồng chức năng

Tài liệu này giúp nhìn toàn bộ **hệ thống quản lý bán vé rạp chiếu phim tại quầy** trước khi đi vào từng class. Đây là **thiết kế mục tiêu**, không có nghĩa các màn hình, Service hoặc Repository đã được lập trình. Hiện dự án mới có các model và một số cấu trúc dữ liệu; đối chiếu tiến độ thực tế trong [Tổng quan tiến độ](../development/TongQuanTienDo.md).

## 1. Hệ thống giải quyết việc gì?

Rạp quản lý phim, phòng/ghế và suất chiếu; nhân viên dùng ứng dụng để bán vé trực tiếp tại quầy. Hệ thống lưu vé, hóa đơn, thành viên và điểm; Admin xem lịch sử, hủy vé hợp lệ và thống kê. Ứng dụng chạy offline, dùng C++17, giao diện Qt Widgets dự kiến và dữ liệu `.txt`. **Khách hàng không có tài khoản đăng nhập và không tự đặt vé trực tuyến.**

Một ví dụ xuyên suốt: Admin tạo phim → tạo phòng và ghế → xếp suất chiếu. Staff đăng nhập → chọn suất/ghế → xác nhận đã thu tiền → hệ thống lưu vé và hóa đơn. Nếu cần hủy vé đã bán, Admin thực hiện nghiệp vụ hủy riêng; Undo trong lúc chọn ghế chỉ dùng **trước khi thu tiền**.

## 2. Ba lớp ứng dụng và DB

Kiến trúc mục tiêu là **GUI → BLL → DAL → DB** và kết quả trả về theo chiều ngược lại. GUI Qt Widgets chỉ gọi BLL, không gọi DAL hoặc đọc TXT. BLL kiểm tra dữ liệu, quyền và nghiệp vụ rồi gọi DALC, không tự đọc/ghi TXT. DAL gồm `core/dal/models/` (dữ liệu trao đổi, không tự truy cập DB) và `core/dal/dalc/` (quản lý tập Model, tìm kiếm, chuyển đổi và đọc/ghi TXT). DB là các file TXT trong `data/`. Các lớp Repository/Service nhắc bên dưới là tên dự kiến trong DALC/BLL, chưa có mã; không tạo chức năng mới trong lần đổi thư mục này.

Chỉ `DoublyLinkedList<T>` cần tự cài. Hash table, stack, queue, map, heap và mảng động có thể dùng STL. Quan hệ Model có mã định danh và một số con trỏ hiện có; lần này không thay đổi con trỏ hoặc vòng đời đối tượng.

### Ghi chú về bảng thiết kế cũ

Bảng bên dưới ghi luồng thiết kế cũ để đối chiếu; đường đi thực thi mới phải tuân theo GUI → BLL → DAL → DB.

| Tầng | Thư mục | Câu hỏi tầng đó trả lời |
| --- | --- | --- |
| UI | `ui/` (dự kiến) | Người dùng nhìn thấy gì và vừa bấm gì? Hiển thị form, bảng, sơ đồ ghế và kết quả. |
| Service | Cây mục tiêu: `core/bll/`; thư mục hiện có: `core/bll/` | Thao tác có được phép không, dữ liệu có hợp lệ không và phải phối hợp những bước nào? |
| Repository | `core/dal/dalc/` | Tìm, thêm, sửa, xóa hoặc chuẩn bị lưu bản ghi nào? |
| File Storage | `data/*.txt`, qua `core/utils/` | Bản ghi được lưu bền vững và phục hồi khi ghi lỗi như thế nào? |

`core/dal/models/` chứa các **đối tượng dữ liệu** (`Movie`, `Room`, `Seat`, `Showtime`, `User`, `Member`, `Ticket`, `Invoice`) đi qua các tầng. `core/data_structures/` chứa cách giữ/tra cứu dữ liệu **trong bộ nhớ** như danh sách liên kết đôi, bảng băm và stack. Hai thư mục này hỗ trợ các tầng trên, không thay thế Service hay Repository.

Ví dụ Admin thêm phim:

```text
MovieManagementView (UI)
    → MovieService: kiểm tra Admin, dữ liệu phim, quy tắc thêm phim
    → MovieRepository: thêm bản ghi Movie và chuẩn bị lưu
    → FileHandler/TransactionManager khi cần: ghi movies.txt và sequences.txt
    → trả kết quả về UI
```

Ở đây `MovieService` **quyết định có được thêm**, còn `MovieRepository` **quản lý bản ghi phim**. Không đặt việc kiểm tra quyền vào mỗi nút UI, vì nút có thể bị gọi từ nơi khác.

## 3. Admin làm gì?

Admin quản lý dữ liệu nền của rạp, nhân viên và báo cáo; đồng thời **cũng có thể bán vé**. Bảng dưới là luồng dự kiến, không phải tên hàm đã tồn tại.

| Chức năng | UI → Service → Repository | File dữ liệu chính |
| --- | --- | --- |
| Đăng nhập | `LoginView` → `AuthService` → `AuthRepository` | `users.txt` |
| Thêm/sửa/tìm/ngừng phim; xóa khi đủ điều kiện | `MovieManagementView` → `MovieService` → `MovieRepository` | `movies.txt`, `sequences.txt` khi thêm |
| Quản lý phòng và sơ đồ ghế | `RoomManagementView`/`SeatMapView` → `RoomService` → `RoomRepository` | `rooms.txt`, `seats.txt`, `sequences.txt` khi thêm phòng |
| Tạo/sửa/hủy và xem lịch suất chiếu | `ShowtimeManagementView` → `ShowtimeService` → `ShowtimeRepository`; service còn tra phim/phòng/vé để kiểm tra điều kiện | `showtimes.txt`, `sequences.txt` khi tạo |
| Thêm/sửa/tìm/vô hiệu hóa Staff | `StaffManagementView` → `StaffService` → `StaffRepository` | `users.txt`, `sequences.txt` khi thêm |
| Xem tất cả hóa đơn, lọc lịch sử | `InvoiceHistoryView` → `ReportService` → `InvoiceRepository`, `TicketRepository` và các repository tra cứu liên quan | Chủ yếu đọc `invoices.txt`, `tickets.txt` |
| Hủy **vé đã bán** trước giờ chiếu | `InvoiceHistoryView` → `BookingService` (luồng hủy) → `TicketRepository`, `InvoiceRepository`, `MemberRepository` và các repository kiểm tra tham chiếu | Ghi `tickets.txt`, `members.txt` nếu có thành viên; giữ hóa đơn lịch sử |
| Xem doanh thu, vé bán/hủy, top phim | `ReportView` → `ReportService` → các repository hóa đơn, vé, phim/suất | Chủ yếu đọc `invoices.txt`, `tickets.txt`, `movies.txt`, `showtimes.txt` |
| Bán vé tại quầy | `BookingView`/`SeatMapView` → `BookingService` → các repository của suất, phòng/ghế, vé, hóa đơn, thành viên | Xem luồng bán vé ở Mục 5 |

Admin không được tùy ý xóa lịch sử vé/hóa đơn. Hủy vé là cập nhật trạng thái `SOLD` → `CANCELLED`, ghi người hủy/lý do/tiền hoàn và thu hồi điểm cùng giao dịch; chỉ thực hiện khi các điều kiện trong [quy tắc nghiệp vụ](business_rules.md) thỏa mãn.

## 4. Staff làm gì?

Staff tập trung vào tra cứu thông tin, phục vụ khách và bán vé. Service vẫn kiểm tra quyền Staff ở từng nghiệp vụ; chỉ ẩn nút trên UI là chưa đủ.

| Chức năng | UI → Service → Repository | File dữ liệu chính |
| --- | --- | --- |
| Đăng nhập | `LoginView` → `AuthService` → `AuthRepository` | `users.txt` |
| Xem/tìm phim sắp chiếu, đang chiếu; xem suất | `StaffDashboard`/`BookingView` → `MovieService`, `ShowtimeService` → `MovieRepository`, `ShowtimeRepository` | Đọc `movies.txt`, `showtimes.txt` |
| Xem sơ đồ ghế và ghế còn trống của suất | `SeatMapView`/`BookingView` → `BookingService` → `RoomRepository`, `TicketRepository`, `ShowtimeRepository` | Đọc `rooms.txt`, `seats.txt`, `tickets.txt`, `showtimes.txt` |
| Tạo, tìm, sửa **thông tin liên hệ** thành viên | `MemberManagementView` → `MemberService` → `MemberRepository` | `members.txt`, `sequences.txt` khi tạo |
| Chọn ghế, Undo/bỏ giỏ trước thu tiền | `BookingView` → `BookingService` và `Stack` trong bộ nhớ | **Chưa ghi file** |
| Tính giá, xác nhận đã thu tiền, bán vé | `BookingView` → `BookingService` → các repository vé, hóa đơn, thành viên cùng các nguồn kiểm tra suất/ghế | Ghi `invoices.txt`, `tickets.txt`, `members.txt` nếu có thành viên, `sequences.txt` |
| Xem hóa đơn của mình trong ngày hoặc tra cứu hóa đơn cũ của mình theo mã | `StaffDashboard`/`InvoiceHistoryView` → `ReportService` → `InvoiceRepository`, `TicketRepository` | Đọc `invoices.txt`, `tickets.txt` |

Staff **không** quản lý phim/phòng/suất/nhân viên, không sửa điểm thành viên tùy ý, không xem toàn bộ hóa đơn của người khác và không hủy vé đã bán. Với vé học sinh, nhân viên kiểm tra giấy tờ; với phim giới hạn tuổi, nhân viên xác nhận điều kiện tại quầy trước khi bán.

## 5. Vì sao BookingService khác MovieService?

`MovieService` xử lý **một nhóm nghiệp vụ phim**. Khi thêm phim, nó kiểm tra quyền và dữ liệu rồi yêu cầu `MovieRepository` lưu phim. Khi tìm phim, nó lấy dữ liệu từ repository và trả kết quả phù hợp.

`BookingService` xử lý **một quy trình đi qua nhiều loại dữ liệu**. Trước thu tiền, nó giữ giỏ ghế và stack Undo trong bộ nhớ. Khi xác nhận bán, nó kiểm tra lại suất/ghế/quyền, tính giá từng vé và điểm, tạo `Invoice` cùng các `Ticket`, rồi phối hợp lưu nhiều file. Một hóa đơn có ít nhất một vé, mọi vé cùng suất, có tối đa một thành viên. Admin hoặc Staff bán đều ghi đúng `userId` người thao tác vào `Invoice.staffId`.

```text
BookingView → BookingService
                 ├─ ShowtimeRepository + MovieRepository: suất/phim còn hợp lệ?
                 ├─ RoomRepository + TicketRepository: ghế thuộc phòng và chưa bán?
                 ├─ MemberRepository: thành viên có tồn tại, điểm mới là bao nhiêu?
                 ├─ InvoiceRepository + TicketRepository: chuẩn bị hóa đơn và vé
                 └─ TransactionManager: lưu cùng giao dịch, rồi cập nhật bộ nhớ
```

`TicketRepository` chỉ quản lý bản ghi vé; nó **không tự quyết định** Staff có được bán ghế đó hay không, mức giảm giá nào áp dụng hoặc khi nào phải cộng điểm. Những quyết định này thuộc `BookingService`. Tương tự, `MemberRepository` lưu điểm đã được Service tính và kiểm tra, không cho UI tự nhập điểm để tăng/giảm.

Xem [thiết kế BookingService](../BookingService.md) để đọc từng bước chọn ghế, Undo, tính giá, xác nhận và xử lý lỗi.

## 6. Cách đọc sơ đồ khi tự triển khai

1. **Bắt đầu từ hành động người dùng:** ví dụ “Admin thêm phim” hoặc “Staff bán vé”.
2. **Tìm màn hình UI:** màn hình lấy dữ liệu nhập và hiển thị kết quả.
3. **Tìm Service cùng nghiệp vụ:** nơi kiểm tra quyền, điều kiện và điều phối.
4. **Tìm Repository theo loại dữ liệu:** nơi đọc/tìm/chuẩn bị cập nhật các bản ghi.
5. **Xác định file dữ liệu bị ảnh hưởng:** một thao tác có thể đọc nhiều file nhưng chỉ ghi một số file. Nếu ghi nhiều file, cần cơ chế giao dịch như mô tả trong [định dạng dữ liệu](data_format.md).

Các tên file dự kiến nằm trong [cấu trúc thư mục](cauTrucThuMuc.md). Hiện `core/dal/dalc/` và `core/bll/` chưa có mã; các bảng trên dùng để **định hướng lúc xây dựng**, chưa xác nhận chức năng hoàn thành.
