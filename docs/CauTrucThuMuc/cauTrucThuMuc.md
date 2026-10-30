# Cấu trúc thư mục chi tiết PBL2

Tài liệu này là **bản thiết kế thư mục và danh mục file** cho ứng dụng quản lý rạp chiếu phim. Ký hiệu **[Có]** nghĩa là file đang có trong cây làm việc; **[Dự kiến]** nghĩa là vị trí và tên file dành cho bước triển khai sau, chưa có mã. Một dòng `Tên.h/.cpp` biểu thị **hai file riêng**: `.h` khai báo lớp/API, `.cpp` cài đặt hàm. Với Qt, `.ui` là file bố cục do Qt Designer tạo. Không tạo file rỗng chỉ để giống cây thiết kế.

## 1. Cây thư mục mục tiêu

```text
PBL2/
├── AGENTS.md
├── README.md
├── CMakeLists.txt
├── main.cpp
│
├── core/
│   ├── dal/                         # Data Access Layer
│   │   ├── models/                  # Các đối tượng dữ liệu
│   │   │   ├── Movie.h
│   │   │   ├── Movie.cpp
│   │   │   ├── Room.h
│   │   │   ├── Room.cpp
│   │   │   ├── Seat.h
│   │   │   ├── Seat.cpp
│   │   │   ├── Showtime.h
│   │   │   ├── Showtime.cpp
│   │   │   ├── User.h
│   │   │   ├── User.cpp
│   │   │   ├── Member.h
│   │   │   ├── Member.cpp
│   │   │   ├── Ticket.h
│   │   │   ├── Ticket.cpp
│   │   │   ├── Invoice.h
│   │   │   └── Invoice.cpp
│   │   │
│   │   └── dalc/                    # Data Access Logic Components
│   │       ├── MovieDALC.h
│   │       ├── MovieDALC.cpp
│   │       ├── RoomDALC.h
│   │       ├── RoomDALC.cpp
│   │       ├── SeatDALC.h
│   │       ├── SeatDALC.cpp
│   │       ├── ShowtimeDALC.h
│   │       ├── ShowtimeDALC.cpp
│   │       ├── UserDALC.h
│   │       ├── UserDALC.cpp
│   │       ├── MemberDALC.h
│   │       ├── MemberDALC.cpp
│   │       ├── TicketDALC.h
│   │       ├── TicketDALC.cpp
│   │       ├── InvoiceDALC.h
│   │       ├── InvoiceDALC.cpp
│   │       ├── SequenceDALC.h
│   │       ├── SequenceDALC.cpp
│   │       ├── BookingDALC.h
│   │       └── BookingDALC.cpp
│   │
│   ├── bll/                        # Business Logic Layer
│   │   ├── AuthService.h
│   │   ├── AuthService.cpp
│   │   ├── MovieManager.h
│   │   ├── MovieManager.cpp
│   │   ├── RoomManager.h
│   │   ├── RoomManager.cpp
│   │   ├── SeatManager.h
│   │   ├── SeatManager.cpp
│   │   ├── ShowtimeManager.h
│   │   ├── ShowtimeManager.cpp
│   │   ├── UserManager.h
│   │   ├── UserManager.cpp
│   │   ├── MemberManager.h
│   │   ├── MemberManager.cpp
│   │   ├── TicketManager.h
│   │   ├── TicketManager.cpp
│   │   ├── InvoiceManager.h
│   │   ├── InvoiceManager.cpp
│   │   ├── BookingService.h
│   │   ├── BookingService.cpp
│   │   ├── ReportService.h
│   │   └── ReportService.cpp
│   │
│   ├── data_structures/
│   │   └── DoublyLinkedList.h      # CTDL template tự viết
│   │
│   ├── utils/
│   │   ├── FileHandler.h
│   │   ├── FileHandler.cpp
│   │   ├── TransactionManager.h
│   │   ├── TransactionManager.cpp
│   │   ├── Validator.h
│   │   ├── Validator.cpp
│   │   ├── DateTime.h
│   │   ├── DateTime.cpp
│   │   ├── IdGenerator.h
│   │   └── IdGenerator.cpp
│   │
│   └── config/
│       ├── PricingConfig.h
│       └── SchedulingConfig.h
│
├── ui/                             # GUI - Qt Widgets
│   ├── mainwindow.h
│   ├── mainwindow.cpp
│   ├── mainwindow.ui
│   │
│   ├── views/
│   │   ├── LoginView.h
│   │   ├── LoginView.cpp
│   │   ├── LoginView.ui
│   │   ├── AdminDashboard.h
│   │   ├── AdminDashboard.cpp
│   │   ├── AdminDashboard.ui
│   │   ├── StaffDashboard.h
│   │   ├── StaffDashboard.cpp
│   │   ├── StaffDashboard.ui
│   │   ├── MovieManagementView.h
│   │   ├── MovieManagementView.cpp
│   │   ├── MovieManagementView.ui
│   │   ├── RoomManagementView.h
│   │   ├── RoomManagementView.cpp
│   │   ├── RoomManagementView.ui
│   │   ├── ShowtimeManagementView.h
│   │   ├── ShowtimeManagementView.cpp
│   │   ├── ShowtimeManagementView.ui
│   │   ├── StaffManagementView.h
│   │   ├── StaffManagementView.cpp
│   │   ├── StaffManagementView.ui
│   │   ├── MemberManagementView.h
│   │   ├── MemberManagementView.cpp
│   │   ├── MemberManagementView.ui
│   │   ├── BookingView.h
│   │   ├── BookingView.cpp
│   │   ├── BookingView.ui
│   │   ├── SeatMapView.h
│   │   ├── SeatMapView.cpp
│   │   ├── InvoiceHistoryView.h
│   │   ├── InvoiceHistoryView.cpp
│   │   ├── InvoiceHistoryView.ui
│   │   ├── ReportView.h
│   │   ├── ReportView.cpp
│   │   └── ReportView.ui
│   │
│   ├── viewmodels/
│   │   ├── MovieTableModel.h
│   │   ├── MovieTableModel.cpp
│   │   ├── TicketTableModel.h
│   │   └── TicketTableModel.cpp
│   │
│   └── resources/
│       ├── resources.qrc
│       ├── style.qss
│       └── icons/
│           ├── app.png
│           ├── movie.png
│           ├── room.png
│           ├── ticket.png
│           ├── member.png
│           └── report.png
│
├── data/                           # DB - Dữ liệu TXT
│   ├── users.txt
│   ├── movies.txt
│   ├── rooms.txt
│   ├── seats.txt
│   ├── showtimes.txt
│   ├── tickets.txt
│   ├── invoices.txt
│   ├── members.txt
│   ├── sequences.txt
│   └── transactions/
│
└── docs/                            Tài liệu thiết kế và nhật ký
    ├── TongQuan/
    │   ├── TongQuanDeTai.md         [Có] Phạm vi và luồng chức năng
    │   ├── business_rules.md        [Có] Quy tắc nghiệp vụ
    │   └── data_format.md           [Có] Schema TXT và phục hồi
    ├── CauTrucThuMuc/
    │   ├── cauTrucThuMuc.md         [Có] Cây thư mục chi tiết
    │   └── models.md                [Có] Đề xuất quan hệ Model hai chiều
    ├── DS/
    │   └── DLL.md                   [Có] Giải thích DLL hiện tại
    └── development/
        ├── TongQuanTienDo.md        [Có] Tiến độ 15 bước
        ├── PhanCongCongViec.md      [Có] Phân công hai thành viên
        ├── QuanHeModels.md          [Có] Rà soát quan hệ Model
        ├── Buoc01_KhoiTaoDuAn.md   [Có] Nhật ký bước 01
        └── Buoc02_XayDungModels.md  [Có] Nhật ký bước 02
```

## 2. Kiến trúc và quy tắc hoạt động

```text
GUI  →  BLL  →  DAL  →  DB
GUI  ←  BLL  ←  DAL  ←  DB
```

| Thành phần | Thư mục | Nhiệm vụ |
| --- | --- | --- |
| GUI | `ui/` | Nhận thao tác và hiển thị dữ liệu. |
| BLL | `core/bll/` | Xử lý nghiệp vụ, CRUD và phân quyền. |
| DAL – Models | `core/dal/models/` | Biểu diễn các đối tượng dữ liệu. |
| DAL – DALC | `core/dal/dalc/` | Quản lý danh sách, truy xuất và lưu dữ liệu. |
| DB | `data/` | Lưu trữ dữ liệu TXT. |

`data_structures/`, `utils/` và `config/` là thành phần hỗ trợ, không phải tầng thứ tư. Chỉ DALC dùng tiện ích ghi file nghiệp vụ để tương tác DB; BLL không đọc hoặc ghi TXT trực tiếp. Cây ở Mục 1 là thiết kế mục tiêu; các DALC, Manager, GUI và dữ liệu chưa có trong mã hiện tại. Phần kiểm thử sẽ được bổ sung khi bước tương ứng bắt đầu.

## 3. Ý nghĩa từng thư mục và file

### Gốc dự án

| File | Ý nghĩa |
| --- | --- |
| `AGENTS.md` | Quy ước cộng tác, ranh giới tầng và cách cập nhật tiến độ; chỉ tạo khi nhóm thống nhất nội dung. |
| `CMakeLists.txt` | Liệt kê nguồn C++17, target, phụ thuộc Qt và bài kiểm thử tương ứng từng giai đoạn. |
| `main.cpp` | Khởi tạo ứng dụng và nối các thành phần; không chứa toàn bộ nghiệp vụ hoặc trực tiếp xử lý bản ghi TXT. |
| `README.md` | Giới thiệu hệ thống, kiến trúc, phạm vi và hướng dẫn build/chạy khi có cấu hình thật. |

### `core/` và `core/dal/`

`core/` chứa mã C++ không phụ thuộc Qt. `core/dal/` là tầng DAL, gồm `models/` và `dalc/`. Luồng yêu cầu là **GUI → BLL → DAL → DB**; kết quả trả về **DB → DAL → BLL → GUI**. Không bỏ qua tầng trung gian. Model là dữ liệu trao đổi, không tự truy cập file; DALC là nơi quản lý tập bản ghi và tương tác TXT. Tiện ích file trong `utils/` chỉ được DALC dùng khi đọc/ghi dữ liệu.

### `core/dal/models/` — đối tượng dữ liệu

Mỗi cặp `.h/.cpp` hiện có. Model giữ thuộc tính, constructor, getter/setter và hành vi nội tại; không mở TXT, phân quyền hoặc quản lý CRUD cho toàn bộ danh sách. Mã định danh và các con trỏ quan hệ hiện có được giữ nguyên.

| Cặp file | Ý nghĩa |
| --- | --- |
| `Movie.h/.cpp` | Một phim: mã, tên, thể loại, thời lượng, giới hạn tuổi, khoảng ngày chiếu và trạng thái. |
| `Room.h/.cpp` | Một phòng: mã, tên, kích thước và trạng thái; hiện có danh sách con trỏ ghế liên quan. |
| `Seat.h/.cpp` | Một vị trí ghế: phòng, mã ghế, hàng/cột, loại ghế; trạng thái đã bán phụ thuộc suất chiếu, không thuộc ghế cố định. |
| `Showtime.h/.cpp` | Một suất chiếu: phim, phòng, thời gian và trạng thái; hiện có liên kết đối tượng ở bộ nhớ. |
| `User.h/.cpp` | Tài khoản Admin/Staff: định danh, đăng nhập, vai trò và trạng thái. |
| `Member.h/.cpp` | Thành viên: định danh, thông tin liên hệ và số điểm. |
| `Ticket.h/.cpp` | Một vé cho một ghế trong một suất: loại vé, số tiền, điểm và thông tin hủy; không xóa vật lý vé đã giao dịch. |
| `Invoice.h/.cpp` | Một hóa đơn: suất chiếu, người bán, thành viên tùy chọn, thời điểm và tổng tiền; hiện có danh sách con trỏ vé. |

### `core/dal/dalc/` — truy xuất dữ liệu

DALC quản lý tập Model, đọc/ghi TXT và dùng tiện ích file; không quyết định quyền hoặc giá vé. Các cặp file sau đều dự kiến.

| Cặp file | Ý nghĩa |
| --- | --- |
| `MovieDALC.h/.cpp` | Đọc/ghi và tìm phim trong `movies.txt`. |
| `RoomDALC.h/.cpp` | Quản lý phòng trong `rooms.txt`. |
| `SeatDALC.h/.cpp` | Quản lý ghế trong `seats.txt`, tra theo phòng và vị trí. |
| `ShowtimeDALC.h/.cpp` | Quản lý suất chiếu trong `showtimes.txt`. |
| `UserDALC.h/.cpp` | Tra `userId`, `username` và lưu `users.txt`. |
| `MemberDALC.h/.cpp` | Quản lý thành viên và điểm trong `members.txt`. |
| `TicketDALC.h/.cpp` | Lưu và tra vé, giữ lịch sử vé hủy trong `tickets.txt`. |
| `InvoiceDALC.h/.cpp` | Lưu và tra hóa đơn trong `invoices.txt`. |
| `SequenceDALC.h/.cpp` | Quản lý bộ đếm `sequences.txt`, tránh tái sử dụng mã. |
| `BookingDALC.h/.cpp` | Phối hợp ghi vé, hóa đơn, điểm và bộ đếm trong giao dịch. |

### `core/bll/` — nghiệp vụ

BLL kiểm tra dữ liệu, quyền và quy tắc nghiệp vụ rồi gọi DALC; không đọc/ghi TXT trực tiếp. Các cặp file sau đều dự kiến. Có thể gộp Manager khi một class khác đã quản lý hợp lý nghiệp vụ đó; cần ghi rõ lý do.

| Cặp file | Ý nghĩa |
| --- | --- |
| `AuthService.h/.cpp` | Đăng nhập, kiểm tra tài khoản hoạt động và quyền. |
| `MovieManager.h/.cpp` | CRUD phim theo quyền và điều kiện tham chiếu. |
| `RoomManager.h/.cpp` | Quản lý phòng và giới hạn chỉnh sửa. |
| `SeatManager.h/.cpp` | Quản lý vị trí, loại ghế và sơ đồ phòng. |
| `ShowtimeManager.h/.cpp` | Quản lý lịch, trùng suất, tạo/sửa/hủy. |
| `UserManager.h/.cpp` | Quản lý nhân viên và phân quyền. |
| `MemberManager.h/.cpp` | Quản lý thành viên và quy tắc điểm. |
| `TicketManager.h/.cpp` | Tra cứu vé và bảo vệ lịch sử. |
| `InvoiceManager.h/.cpp` | Tra cứu và kiểm tra hóa đơn. |
| `BookingService.h/.cpp` | Chọn ghế, Undo, giá/điểm, bán và hủy vé. |
| `ReportService.h/.cpp` | Doanh thu, hoàn tiền và thống kê. |

### `core/data_structures/`

| File | Ý nghĩa |
| --- | --- |
| `DoublyLinkedList.h` | Template danh sách liên kết đôi tự cài đặt, có thể dùng với nhiều kiểu Model. Thuật toán hiện tại được giữ nguyên; chưa coi việc biên dịch Model là kiểm thử hành vi DLL. |

Chỉ `DoublyLinkedList<T>` bắt buộc tự cài. Khi phù hợp, có thể dùng `std::unordered_map`, `std::stack`, `std::queue`, `std::map`, `std::priority_queue`, `std::vector`; tìm tên phim theo tiền tố không bắt buộc viết Trie. Không tạo trước `HashTable.h`, `Stack.h`, `Trie.h`, `BST.h`, `MaxHeap.h`.

### `core/utils/` và `core/config/`

Các file đều **dự kiến**. `utils/` chứa công cụ kỹ thuật dùng chung; `config/` chứa giá trị cấu hình. Đường đọc/ghi dữ liệu của hệ thống vẫn do DALC điều phối, không để BLL gọi thẳng `FileHandler` hoặc `TransactionManager`.

| File | Ý nghĩa |
| --- | --- |
| `FileHandler.h/.cpp` | Mở, đọc, ghi file TXT và báo lỗi I/O ở mức kỹ thuật; không quyết định nghiệp vụ. |
| `TransactionManager.h/.cpp` | Ghi/khôi phục nhóm file liên quan bằng dữ liệu tạm, bản sao và trạng thái giao dịch theo `data_format.md`. |
| `Validator.h/.cpp` | Kiểm tra định dạng chung của chuỗi, số, trường văn bản; quy tắc nghiệp vụ vẫn thuộc BLL. |
| `DateTime.h/.cpp` | Parse, định dạng và so sánh ngày giờ theo quy ước dự án. |
| `IdGenerator.h/.cpp` | Cấp mã từ `sequences.txt` theo quy tắc hiện có, không tái sử dụng mã đã cấp. |
| `PricingConfig.h` | Giá ghế, mức giảm và quy tắc điểm mặc định để BLL sử dụng nhất quán. |
| `SchedulingConfig.h` | Khoảng nghỉ tối thiểu giữa hai suất cùng phòng. |

### `ui/` — Qt Widgets

Toàn bộ file dưới đây **dự kiến**. `ui/` chỉ nhận thao tác, gọi BLL và trình bày kết quả; không gọi DALC, đọc TXT, tính giá hoặc quyết định quyền. `views/` chứa màn hình; `viewmodels/` chỉ cần khi dùng bảng Qt; `resources/` chứa tài nguyên giao diện.

| File | Ý nghĩa |
| --- | --- |
| `mainwindow.h/.cpp/.ui` | Cửa sổ chính, điều hướng tới màn hình theo vai trò. |
| `views/LoginView.h/.cpp/.ui` | Nhập thông tin đăng nhập, hiển thị kết quả từ AuthService. |
| `views/AdminDashboard.h/.cpp/.ui` | Các lối vào chức năng quản trị. |
| `views/StaffDashboard.h/.cpp/.ui` | Các lối vào chức năng bán vé và tra cứu cho Staff. |
| `views/MovieManagementView.h/.cpp/.ui` | Hiển thị, tìm và gửi yêu cầu quản lý phim tới BLL. |
| `views/RoomManagementView.h/.cpp/.ui` | Hiển thị phòng/ghế và gửi yêu cầu quản lý phòng. |
| `views/ShowtimeManagementView.h/.cpp/.ui` | Hiển thị lịch, tạo/sửa/hủy suất qua BLL. |
| `views/StaffManagementView.h/.cpp/.ui` | Quản lý tài khoản nhân viên qua BLL. |
| `views/MemberManagementView.h/.cpp/.ui` | Tra cứu và cập nhật thông tin liên hệ thành viên qua BLL. |
| `views/BookingView.h/.cpp/.ui` | Chọn suất/ghế/loại vé, xem giá BLL trả về, xác nhận thu tiền. |
| `views/SeatMapView.h/.cpp` | Vẽ vị trí và trạng thái ghế theo dữ liệu BLL trả về. |
| `views/InvoiceHistoryView.h/.cpp/.ui` | Hiển thị hóa đơn/vé và gửi yêu cầu hủy khi chức năng cho phép. |
| `views/ReportView.h/.cpp/.ui` | Hiển thị các chỉ số thống kê từ BLL. |
| `viewmodels/MovieTableModel.h/.cpp` | Chuyển kết quả phim thành dữ liệu bảng Qt, nếu màn hình dùng Qt Model/View. |
| `viewmodels/TicketTableModel.h/.cpp` | Chuyển kết quả vé thành dữ liệu bảng Qt, nếu cần. |
| `resources/resources.qrc` | Khai báo tài nguyên để Qt đóng gói vào ứng dụng. |
| `resources/style.qss` | Quy định kiểu hiển thị các widget. |
| `resources/icons/` | Thư mục ảnh biểu tượng; tên file cụ thể xác định khi có giao diện. |

### `data/` — DB dạng TXT

Các file đều **dự kiến**; chưa có dữ liệu thật trong cây làm việc. Tên trường, thứ tự, mã và quy trình ghi theo [data_format.md](../TongQuan/data_format.md), không tự đổi schema hoặc tạo dữ liệu giả vào DB thật.

| File/thư mục | Ý nghĩa |
| --- | --- |
| `users.txt` | Tài khoản Admin/Staff. |
| `movies.txt` | Danh mục phim. |
| `rooms.txt` | Thông tin phòng. |
| `seats.txt` | Các vị trí ghế thuộc phòng. |
| `showtimes.txt` | Lịch và trạng thái suất chiếu. |
| `members.txt` | Thông tin liên hệ và điểm thành viên. |
| `tickets.txt` | Vé bán và lịch sử vé hủy. |
| `invoices.txt` | Hóa đơn; vé liên kết qua mã hóa đơn, không lưu lặp danh sách vé. |
| `sequences.txt` | Bộ đếm cấp mã cho từng loại đối tượng. |
| `transactions/` | Vùng dữ liệu tạm/backup/manifest khi lưu giao dịch nhiều file; tên file con do cơ chế lưu quyết định. |

## 4. `docs/` — ý nghĩa từng file

| File | Ý nghĩa |
| --- | --- |
| `TongQuan/TongQuanDeTai.md` | Tổng quan hệ thống và chức năng. |
| `CauTrucThuMuc/cauTrucThuMuc.md` | Cây thư mục và trách nhiệm; file này. |
| `CauTrucThuMuc/models.md` | Đề xuất quan hệ Model hai chiều. |
| `TongQuan/business_rules.md` | Quy tắc nghiệp vụ. |
| `TongQuan/data_format.md` | Schema TXT và quy tắc lưu. |
| `DS/DLL.md` | Giải thích Doubly Linked List. |
| `development/QuanHeModels.md` | Rà soát quan hệ Model trước đây. |
| `development/PhanCongCongViec.md` | Phân công và phối hợp. |
| `development/TongQuanTienDo.md` | Tiến độ 15 bước. |
| `development/Buoc01_KhoiTaoDuAn.md` | Nhật ký Bước 01. |
| `development/Buoc02_XayDungModels.md` | Nhật ký Bước 02. |

Các file trên hiện có. `BookingService.md`, `DS/HashTable.md` và `DS/Stack.md` được nhắc trong thiết kế cũ nhưng hiện không có trong cây làm việc. Nhật ký Bước 03–15 chỉ tạo khi bước thật sự bắt đầu.

## 5. Ví dụ luồng xử lý theo mô hình 3 lớp

Các class trong ví dụ đều dự kiến. Tiện ích file chỉ được DALC gọi.

### 5.1. Admin thêm phim

```text
MovieManagementView (GUI)
  → MovieManager (BLL): kiểm tra quyền Admin, dữ liệu phim
  → MovieDALC (DAL): quản lý danh sách và chuẩn bị ghi
  → FileHandler / TransactionManager (tiện ích của DALC)
  → movies.txt + sequences.txt (DB)
```

Kết quả trả về `DB → DALC → MovieManager → MovieManagementView`.

### 5.2. Nhân viên bán vé

```text
BookingView (GUI)
  → BookingService (BLL): kiểm tra quyền, suất, ghế, giá, giảm giá, điểm
  → BookingDALC (DAL): phối hợp InvoiceDALC, TicketDALC, MemberDALC, SequenceDALC
  → TransactionManager (tiện ích của DALC)
  → invoices.txt + tickets.txt + members.txt + sequences.txt (DB)
```

Chỉ báo bán vé thành công sau khi giao dịch được lưu thành công.

## 6. Chuyển đổi từ cấu trúc cũ

| Cấu trúc cũ | Cấu trúc mới / xử lý |
| --- | --- |
| `core/models/` | `core/dal/models/` — tám cặp Model đã được chuyển. |
| `core/repositories/` | `core/dal/dalc/` — vị trí dự kiến, chưa có mã để chuyển. |
| `core/service/` hoặc `core/services/` | `core/bll/` — vị trí dự kiến, chưa có mã để chuyển. |
| `core/data_structures/DoublyLinkedList.h` | Giữ nguyên. |
| `core/data_structures/HashTable.h` | Không còn bắt buộc tự cài; hiện không có file. |
| `core/data_structures/Stack.h` | Không còn bắt buộc tự cài; hiện không có file. |

