# Cấu trúc thư mục chi tiết PBL2

Tài liệu này liệt kê **từng file trong cây `src/` hiện tại và phần còn dự kiến**. Các file ghi `[Dự kiến]` chỉ tạo khi bước tương ứng cần đến; tên màn hình Qt và cách tách Service còn có thể điều chỉnh khi lập trình. Không tạo file rỗng chỉ để khớp sơ đồ. Xem [tiến độ thực tế](../development/TongQuanTienDo.md).

## 1. Cây thư mục mục tiêu

```text
PBL2/
├── .gitignore
├── README.md
├── CMakeLists.txt                         [Dự kiến]
├── src/
│   ├── main.cpp                           [Đã chuyển; vẫn là chương trình thử]
│   ├── gui/                               [Dự kiến: Qt Widgets]
│   │   ├── MainWindow.h                   [Dự kiến]
│   │   ├── MainWindow.cpp                 [Dự kiến]
│   │   ├── MainWindow.ui                  [Dự kiến nếu dùng Qt Designer]
│   │   ├── LoginView.h                    [Dự kiến]
│   │   ├── LoginView.cpp                  [Dự kiến]
│   │   ├── LoginView.ui                   [Dự kiến nếu dùng Qt Designer]
│   │   ├── AdminDashboard.h              [Dự kiến]
│   │   ├── AdminDashboard.cpp            [Dự kiến]
│   │   ├── StaffDashboard.h              [Dự kiến]
│   │   ├── StaffDashboard.cpp            [Dự kiến]
│   │   ├── MovieManagementView.h         [Dự kiến]
│   │   ├── MovieManagementView.cpp       [Dự kiến]
│   │   ├── RoomManagementView.h          [Dự kiến]
│   │   ├── RoomManagementView.cpp        [Dự kiến]
│   │   ├── ShowtimeManagementView.h      [Dự kiến]
│   │   ├── ShowtimeManagementView.cpp    [Dự kiến]
│   │   ├── StaffManagementView.h         [Dự kiến]
│   │   ├── StaffManagementView.cpp       [Dự kiến]
│   │   ├── MemberManagementView.h        [Dự kiến]
│   │   ├── MemberManagementView.cpp      [Dự kiến]
│   │   ├── BookingView.h                 [Dự kiến]
│   │   ├── BookingView.cpp               [Dự kiến]
│   │   ├── SeatMapView.h                 [Dự kiến]
│   │   ├── SeatMapView.cpp               [Dự kiến]
│   │   ├── InvoiceHistoryView.h          [Dự kiến]
│   │   ├── InvoiceHistoryView.cpp        [Dự kiến]
│   │   ├── ReportView.h                  [Dự kiến]
│   │   └── ReportView.cpp                [Dự kiến]
│   ├── business/                          [Dự kiến: xử lý nghiệp vụ]
│   │   ├── AuthService.h                 [Dự kiến]
│   │   ├── AuthService.cpp               [Dự kiến]
│   │   ├── MovieService.h                [Dự kiến]
│   │   ├── MovieService.cpp              [Dự kiến]
│   │   ├── RoomService.h                 [Dự kiến]
│   │   ├── RoomService.cpp               [Dự kiến]
│   │   ├── ShowtimeService.h             [Dự kiến]
│   │   ├── ShowtimeService.cpp           [Dự kiến]
│   │   ├── MemberService.h               [Dự kiến]
│   │   ├── MemberService.cpp             [Dự kiến]
│   │   ├── StaffService.h                [Dự kiến]
│   │   ├── StaffService.cpp              [Dự kiến]
│   │   ├── BookingService.h              [Dự kiến]
│   │   ├── BookingService.cpp            [Dự kiến]
│   │   ├── ReportService.h               [Dự kiến]
│   │   └── ReportService.cpp             [Dự kiến]
│   ├── data_access/                       [Dự kiến: truy xuất TXT]
│   │   ├── MovieRepository.h             [Dự kiến viết lại ở Bước 04]
│   │   ├── MovieRepository.cpp           [Dự kiến viết lại ở Bước 04]
│   │   ├── RoomRepository.h              [Dự kiến]
│   │   ├── RoomRepository.cpp            [Dự kiến]
│   │   ├── SeatRepository.h              [Dự kiến]
│   │   ├── SeatRepository.cpp            [Dự kiến]
│   │   ├── ShowtimeRepository.h          [Dự kiến]
│   │   ├── ShowtimeRepository.cpp        [Dự kiến]
│   │   ├── UserRepository.h              [Dự kiến]
│   │   ├── UserRepository.cpp            [Dự kiến]
│   │   ├── MemberRepository.h            [Dự kiến]
│   │   ├── MemberRepository.cpp          [Dự kiến]
│   │   ├── TicketRepository.h            [Dự kiến]
│   │   ├── TicketRepository.cpp          [Dự kiến]
│   │   ├── InvoiceRepository.h           [Dự kiến]
│   │   └── InvoiceRepository.cpp         [Dự kiến]
│   ├── models/                            [Đã chuyển tám cặp Model]
│   │   ├── Movie.h
│   │   ├── Movie.cpp
│   │   ├── Room.h
│   │   ├── Room.cpp
│   │   ├── Seat.h
│   │   ├── Seat.cpp
│   │   ├── Showtime.h
│   │   ├── Showtime.cpp
│   │   ├── User.h
│   │   ├── User.cpp
│   │   ├── Member.h
│   │   ├── Member.cpp
│   │   ├── Ticket.h
│   │   ├── Ticket.cpp
│   │   ├── Invoice.h
│   │   └── Invoice.cpp
│   ├── data_structures/                   [Đã chuyển]
│   │   └── DoublyLinkedList.h
│   └── utils/
│       ├── FileIO.h                      [Đã chuyển từ FileHandler]
│       ├── FileIO.cpp                    [Đã chuyển từ FileHandler]
│       ├── DateTime.h                    [Đã chuyển]
│       ├── DateTime.cpp
│       ├── IdGenerator.h                 [Đã chuyển]
│       └── IdGenerator.cpp
├── data/                                  [Hiện trống; TXT dưới đây dự kiến]
│   ├── users.txt
│   ├── movies.txt
│   ├── rooms.txt
│   ├── seats.txt
│   ├── showtimes.txt
│   ├── members.txt
│   ├── tickets.txt
│   ├── invoices.txt
│   └── sequences.txt
├── tests/
│   ├── test_all_utils.cpp               [Đã có; kiểm tra FileIO và tiện ích]
│   └── legacy/                          [Lưu mã TransactionManager cũ để đối chiếu]
│       ├── TransactionManager.h
│       └── TransactionManager.cpp
└── docs/                                 [Các file dưới đây đã có]
    ├── TongQuan/
    │   ├── TongQuanDeTai.md
    │   ├── business_rules.md
    │   └── data_format.md
    ├── CauTrucThuMuc/
    │   ├── cauTrucThuMuc.md
    │   ├── src.md
    │   ├── gui.md
    │   ├── business.md
    │   ├── data_access.md
    │   ├── models.md
    │   ├── data_structures.md
    │   ├── utils.md
    │   └── utils(docs)/
    │       ├── FileIO.md
    │       ├── DateTime.md
    │       └── IdGenerator.md
    ├── DS/
    │   └── DLL.md
    └── development/
        ├── TongQuanTienDo.md
        ├── PhanCongCongViec.md
        ├── Buoc01_KhoiTaoDuAn.md
        ├── Buoc02_XayDungModels.md
        ├── Buoc03_HaTangLuuTruDuLieu.md
        ├── Buoc04_QuanLyPhim.md
        └── tasks/
            ├── Task_Hoang.md
            └── Task_Kiet.md
```

`src/` chứa mã nguồn ứng dụng C++; `data/` chứa TXT; `docs/` chứa tài liệu; `tests/` chứa kiểm thử và mã cũ để đối chiếu. Cây mã mới không đặt thêm `core/`, `dal/`, `dalc/`, `services/` hoặc `repositories/` làm thư mục trung gian. Tên tài liệu hiện hành dùng `data_access.md`, `FileIO.md` và `Buoc04_QuanLyPhim.md`; lịch sử DALC và FileHandler được giữ trong nội dung nhật ký. Tài liệu riêng `TransactionManager.md` không còn trong cây tài liệu; nhật ký Bước 03 vẫn giữ lịch sử liên quan.

## 2. Vai trò từng file mã nguồn

Với các cặp `.h` và `.cpp` trong bảng, `.h` khai báo class và hàm; `.cpp` cài đặt chúng. Tám cặp Model, `FileIO`, `DateTime` và `IdGenerator` đã có ở `src/`; các cặp còn lại vẫn là đề xuất. Hai file `MovieRepository` cũ đã bị xóa khỏi cây làm việc; Hoàng sẽ viết lại từ đầu trong Bước 04. API cụ thể phải đối chiếu mã khi triển khai.

### `src/gui/` — giao diện

| File/cặp file | Vai trò dự kiến |
| --- | --- |
| `MainWindow.h/.cpp` | Cửa sổ chính và điều hướng theo vai trò. |
| `MainWindow.ui` | Bố cục cửa sổ chính nếu nhóm dùng Qt Designer. |
| `LoginView.h/.cpp`, `LoginView.ui` | Nhận thông tin đăng nhập, hiển thị kết quả từ `AuthService`; `.ui` chỉ cần khi dùng Qt Designer. |
| `AdminDashboard.h/.cpp` | Lối vào chức năng Admin. |
| `StaffDashboard.h/.cpp` | Lối vào chức năng Staff. |
| `MovieManagementView.h/.cpp` | Form và bảng quản lý phim; gọi `MovieService`. |
| `RoomManagementView.h/.cpp` | Form và sơ đồ quản lý phòng/ghế; gọi Service liên quan. |
| `ShowtimeManagementView.h/.cpp` | Lịch suất chiếu và thao tác tạo/sửa/hủy. |
| `StaffManagementView.h/.cpp` | Màn hình quản lý tài khoản nhân viên. |
| `MemberManagementView.h/.cpp` | Tra cứu và sửa thông tin liên hệ thành viên. |
| `BookingView.h/.cpp` | Chọn suất, ghế, loại vé; hiển thị giá và xác nhận thu tiền. |
| `SeatMapView.h/.cpp` | Hiển thị sơ đồ và trạng thái ghế **theo suất chiếu**. |
| `InvoiceHistoryView.h/.cpp` | Tra hóa đơn/vé và gửi yêu cầu hủy nếu có quyền. |
| `ReportView.h/.cpp` | Hiển thị doanh thu và thống kê do `ReportService` trả về. |

GUI nhận thao tác và hiển thị dữ liệu; không mở TXT, tự tính giá hay quyết định quyền. Các file `.ui` ngoài hai ví dụ trong cây chỉ thêm khi thiết kế giao diện cần; không bắt buộc tạo cho từng màn hình.

### `src/business/` — nghiệp vụ

| Cặp file | Vai trò dự kiến |
| --- | --- |
| `AuthService.h/.cpp` | Đăng nhập, kiểm tra tài khoản đang hoạt động và vai trò Admin/Staff. |
| `MovieService.h/.cpp` | Quyền và điều kiện thêm/sửa/ngừng/xóa phim. |
| `RoomService.h/.cpp` | Điều kiện quản lý phòng, vị trí và loại ghế; có thể tách `SeatService` nếu chức năng thực tế cần. |
| `ShowtimeService.h/.cpp` | Kiểm tra lịch, phòng, phim và điều kiện tạo/sửa/hủy suất. |
| `MemberService.h/.cpp` | Thông tin thành viên và quy tắc điểm. |
| `StaffService.h/.cpp` | Quản lý nhân viên, dùng lại `UserRepository` và kiểm tra quyền Admin. |
| `BookingService.h/.cpp` | Chọn ghế, Undo trước thanh toán, tính giá/ưu đãi, bán/hủy vé và điều phối lưu. |
| `ReportService.h/.cpp` | Thống kê tiền bán, tiền hoàn, doanh thu và vé. |

`StaffService` thuộc Bước 07 và dùng lại `UserRepository` của Bước 05; không mặc định thêm `StaffRepository`. `PricingService` chỉ tạo ở Bước 10 nếu có lý do rõ. Chưa mặc định thêm Service cho từng Model. Business gọi Repository, không đọc hoặc ghi TXT trực tiếp.

### `src/data_access/` — truy xuất dữ liệu

| Cặp file | Vai trò dự kiến |
| --- | --- |
| `MovieRepository.h/.cpp` | Dự kiến viết lại trong Bước 04 để quản lý tập `Movie`, đọc/ghi `movies.txt`, tra phim theo mã; code cũ chưa nghiệm thu đã bị xóa khỏi cây làm việc. |
| `RoomRepository.h/.cpp` | Quản lý `Room`, đọc/ghi `rooms.txt`, kiểm tra mã phòng. |
| `SeatRepository.h/.cpp` | Quản lý `Seat`, đọc/ghi `seats.txt`, kiểm tra phòng và vị trí ghế. |
| `ShowtimeRepository.h/.cpp` | Quản lý `Showtime`, đọc/ghi `showtimes.txt`, kiểm tra tham chiếu phim/phòng. |
| `UserRepository.h/.cpp` | Quản lý `User`, đọc/ghi `users.txt`, tra ID và tên đăng nhập. |
| `MemberRepository.h/.cpp` | Quản lý `Member`, đọc/ghi `members.txt`, lưu điểm đã được Service tính. |
| `TicketRepository.h/.cpp` | Quản lý `Ticket`, đọc/ghi `tickets.txt`, giữ vé đã hủy để tra lịch sử. |
| `InvoiceRepository.h/.cpp` | Quản lý `Invoice`, đọc/ghi `invoices.txt`, giữ liên kết hóa đơn–vé. |

Repository gọi `FileIO` để đọc dòng, kiểm tra số trường/kiểu/khóa/tham chiếu rồi dựng Model trong RAM. Khi nghiệp vụ yêu cầu lưu, Repository chuyển Model thành dòng và ghi **toàn bộ** file. Từ Bước 04, từng chức năng làm Repository rồi Service rồi kiểm thử C++; không chờ hoàn thành mọi Repository. Chưa mặc định tạo `SequenceRepository` hoặc `BookingRepository`: `IdGenerator` xử lý quy tắc mã, còn `BookingService` điều phối bán vé. Xem [thiết kế Repository](data_access.md) và [lộ trình](../development/PhanCongCongViec.md).

### `src/models/` — đối tượng dữ liệu

| Cặp file | Đối tượng biểu diễn |
| --- | --- |
| `Movie.h/.cpp` | Phim: mã, tên, thời lượng, độ tuổi, trạng thái và thông tin chiếu. |
| `Room.h/.cpp` | Phòng: mã, tên, kích thước, trạng thái; liên hệ với ghế. |
| `Seat.h/.cpp` | Vị trí ghế trong phòng, hàng/cột và loại ghế. Ghế đã bán được xác định theo suất, không lưu cố định trong phòng. |
| `Showtime.h/.cpp` | Suất chiếu: phim, phòng, thời gian và trạng thái. |
| `User.h/.cpp` | Tài khoản Admin/Staff và trạng thái hoạt động. |
| `Member.h/.cpp` | Thành viên, thông tin liên hệ và điểm. |
| `Ticket.h/.cpp` | Vé cho một ghế trong một suất, giá, trạng thái và thông tin hủy. |
| `Invoice.h/.cpp` | Hóa đơn của một suất, người bán, thành viên tùy chọn, nhiều vé và tổng tiền. |

Tám cặp này **đã có mã ở `src/models/`**. Model không tự đọc/ghi file hoặc quyết định nghiệp vụ. Xem [quan hệ Model](models.md).

### `src/data_structures/` và `src/utils/`

| File/cặp file | Vai trò |
| --- | --- |
| `DoublyLinkedList.h` | Danh sách liên kết đôi tự cài đặt, hiện ở `src/data_structures/`; không có `.cpp` vì là template. |
| `FileIO.h/.cpp` | Tiện ích đã có ở `src/utils/` với `readLines(...)`, `writeLines(...)` để đọc/ghi dòng TXT. |
| `DateTime.h/.cpp` | Hỗ trợ ngày giờ; mã hiện ở `src/utils/` mới cung cấp thời điểm hiện tại, chưa có API parse đầy đủ. |
| `IdGenerator.h/.cpp` | Đọc `sequences.txt`, tính mã và nội dung bộ đếm mới; mã hiện ở `src/utils/`, không tự ghi file. |

Mã `TransactionManager` cũ được lưu ở `tests/legacy/` để đối chiếu và chạy bài thử lịch sử; ứng dụng trong `src/` không dùng lớp này. Không có `data/transactions/` trong thiết kế mục tiêu. Không tự thêm `Validator`, factory hoặc các class tiện ích chưa cần thiết.

## 3. File ở gốc, `data/`, `tests/` và `docs/`

| Vị trí | Vai trò và trạng thái |
| --- | --- |
| `.gitignore` | Quy tắc bỏ qua file sinh ra; đã có. |
| `README.md` | Giới thiệu, phạm vi và hướng dẫn dự án; đã có. |
| `CMakeLists.txt` | Cấu hình build C++17/Qt và kiểm thử khi triển khai; chưa có. |
| `src/main.cpp` | Đã chuyển vào `src/`; hiện vẫn là chương trình thử, chưa khởi chạy Qt. |
| `data/users.txt` | Tài khoản Admin/Staff. |
| `data/movies.txt` | Danh mục phim. |
| `data/rooms.txt` | Danh mục phòng. |
| `data/seats.txt` | Vị trí và loại ghế theo phòng. |
| `data/showtimes.txt` | Lịch và trạng thái suất chiếu. |
| `data/members.txt` | Thông tin thành viên và điểm. |
| `data/tickets.txt` | Vé bán và lịch sử vé hủy. |
| `data/invoices.txt` | Hóa đơn; vé liên kết bằng mã hóa đơn. |
| `data/sequences.txt` | Các dòng `entity|nextValue` dùng cấp mã. |
| `tests/test_all_utils.cpp` | Bài thử đã cập nhật đường dẫn `src/utils/`; vẫn thử cả mã giao dịch lịch sử trong `tests/legacy/`. |

`data/` hiện **trống**; chín file TXT trong bảng là dự kiến. Mọi file TXT, kể cả `sequences.txt`, không có dòng `#schema=1` hoặc tiêu đề. Số trường, thứ tự và kiểu dữ liệu giữ theo [data_format.md](../TongQuan/data_format.md). Không tạo `data/transactions/` trong thiết kế mục tiêu.

Các file dưới `docs/` đã được liệt kê đúng đường dẫn trong cây ở Mục 1: `TongQuan/` giải thích phạm vi, nghiệp vụ và TXT; `CauTrucThuMuc/` mô tả cấu trúc, Model, Repository và lịch sử tiện ích; `DS/DLL.md` giải thích danh sách liên kết đôi; `development/` chứa tiến độ, phân công, nhật ký Bước 01–04 và hai checklist cá nhân. Chưa có nhật ký các bước sau Bước 04.

## 4. Luồng giữa các thư mục

```text
src/gui/ → src/business/ → src/data_access/ → data/*.txt
             Service             Repository       DB
                                     ↓
                            src/utils/FileIO
```

`src/models/` và `src/data_structures/` hỗ trợ việc giữ dữ liệu trong RAM; `src/utils/` hỗ trợ kỹ thuật, không phải tầng nghiệp vụ thứ tư. Ví dụ Admin thêm phim: `MovieManagementView` gọi `MovieService`; Service kiểm tra quyền và dữ liệu; `MovieRepository` cập nhật tập phim và dùng `FileIO` lưu `movies.txt` khi cần. Khi cấp mã, luồng tạo còn phải lưu nội dung `updatedSequences` của `IdGenerator` vào `sequences.txt`.

Ví dụ bán vé (Bước 11; GUI triển khai ở Bước 13): `BookingView` gọi `BookingService`; Service kiểm tra suất, ghế, giá, thành viên và quyền; các Repository liên quan chuẩn bị vé, hóa đơn, điểm và bộ đếm rồi ghi các file TXT tương ứng. **Ghi nhiều file tuần tự không nguyên tử**: lỗi hoặc gián đoạn có thể khiến dữ liệu không nhất quán. Phải kiểm tra từng lần ghi, báo lỗi và không âm thầm bỏ dữ liệu sai.

## 5. Đường dẫn hiện có và việc chuyển thư mục

Đây là **bản đồ các file vừa được chuyển**; các class chưa được lập trình vẫn để `[Dự kiến]` trong cây ở Mục 1:

| Đường dẫn hiện có | Vị trí mục tiêu | Ghi chú |
| --- | --- | --- |
| `main.cpp` | `src/main.cpp` | Đã chuyển; hiện vẫn là chương trình thử, chưa phải GUI. |
| `core/dal/models/*.h`, `*.cpp` | `src/models/` | Đã chuyển tám Model và giữ quan hệ/định dạng hiện hành. |
| `core/dal/dalc/MovieDALC.h/.cpp` | `src/data_access/MovieRepository.h/.cpp` | Từng chuyển và đổi tên; hai file Repository cũ hiện đã bị xóa, chưa nghiệm thu; Bước 04 sẽ viết lại từ đầu. |
| `core/data_structures/DoublyLinkedList.h` | `src/data_structures/` | Đã chuyển cấu trúc dữ liệu tự cài đặt. |
| `core/utils/FileHandler.h/.cpp` | `src/utils/FileIO.h/.cpp` | Đã đổi tên class và phương thức đọc thành `readLines`. |
| `core/utils/DateTime.h/.cpp` | `src/utils/` | Đã chuyển, chưa đổi API. |
| `core/utils/IdGenerator.h/.cpp` | `src/utils/` | Đã chuyển, giữ quy tắc cấp mã và `sequences.txt`. |
| `core/utils/TransactionManager.h/.cpp` | `tests/legacy/TransactionManager.h/.cpp` | Lưu mã lịch sử để bài thử cũ tiếp tục đối chiếu; ứng dụng không dùng. |

Hiện `src/` đã có tám Model, `DoublyLinkedList`, `FileIO`, `DateTime`, `IdGenerator` và `main.cpp` thử nghiệm. Hai file `MovieRepository` cũ đã bị xóa khỏi cây làm việc; Bước 04 sẽ viết lại từ đầu. Repository hiện hành, Service, GUI Qt và `CMakeLists.txt` chưa có. `data/` trống; một số `.exe` thử nghiệm còn ở gốc cây làm việc và không thuộc cây mã nguồn mục tiêu.
