# Cấu trúc thư mục mục tiêu

Đây là cấu trúc dự kiến khi hoàn thiện, không phải danh sách file đã tồn tại. `.h/.cpp` là hai file; `.h/.cpp/.ui` là ba file. Tên file và tên thư mục dùng đúng chữ hoa/thường như cây dưới đây để include nhất quán trên mọi hệ điều hành.

```text
PBL2/
├── CMakeLists.txt                  # Build C++ và Qt (dự kiến)
├── main.cpp                       # QApplication và khởi tạo phụ thuộc
├── README.md
├── core/                          # C++ thuần, không phụ thuộc Qt
│   ├── models/                   # Định nghĩa thực thể và các trường dữ liệu
│   │   ├── User.h/.cpp             # Tài khoản, enum vai trò và trạng thái
│   │   ├── Movie.h/.cpp            # Phim, khoảng ngày chiếu
│   │   ├── Room.h/.cpp             # Thông tin phòng; sơ đồ ghế được dựng từ Seat
│   │   ├── Seat.h/.cpp             # Vị trí, loại; không có trạng thái bán
│   │   ├── Showtime.h/.cpp         # suất chiếu
│   │   ├── Ticket.h/.cpp           # vé
│   │   ├── Invoice.h/.cpp          # Hóa đơn
│   │   └── Member.h/.cpp           # Liên hệ và điểm; chưa có hạng thẻ
│   ├── data_structures/          # Cấu trúc dùng để lưu, tìm kiếm và hoàn tác
│   │   ├── DoublyLinkedList.h      # Danh sách thực thể (tự cài)
│   │   ├── HashTable.h             # Chỉ mục userId, username và invoiceId (tự cài)
│   │   ├── Stack.h                 # Undo chọn ghế trước thanh toán (tự cài)
│   │   ├── Trie.h                  # Gợi ý tên phim
│   │   ├── BST.h                   # Khóa (createdAt, invoiceId)
│   │   └── MaxHeap.h               # Top phim
│   ├── repositories/             # Đọc/ghi file, chuyển dòng dữ liệu thành thực thể
│   │   ├── AuthRepository.h/.cpp   # Đọc users.txt
│   │   ├── StaffRepository.h/.cpp  # Quản lý Staff trong users.txt, giữ Admin
│   │   ├── MovieRepository.h/.cpp
│   │   ├── RoomRepository.h/.cpp   # rooms.txt và seats.txt
│   │   ├── ShowtimeRepository.h/.cpp
│   │   ├── TicketRepository.h/.cpp
│   │   ├── InvoiceRepository.h/.cpp
│   │   └── MemberRepository.h/.cpp
│   ├── services/                 # Kiểm tra quyền, quy tắc nghiệp vụ và phối hợp repository
│   │   ├── AuthService.h/.cpp      # Xác thực, phân quyền
│   │   ├── StaffService.h/.cpp
│   │   ├── MovieService.h/.cpp
│   │   ├── RoomService.h/.cpp
│   │   ├── ShowtimeService.h/.cpp  # Lịch và ràng buộc sửa/hủy
│   │   ├── MemberService.h/.cpp    # Liên hệ và quy tắc điểm
│   │   ├── BookingService.h/.cpp   # Bán/hủy vé, điều phối giao dịch
│   │   └── ReportService.h/.cpp    # Lịch sử, thống kê, top phim
│   ├── utils/                    # Tiện ích dùng chung: file, kiểm tra đầu vào, thời gian, mã
│   │   ├── FileHandler.h/.cpp
│   │   ├── TransactionManager.h/.cpp # File tạm, backup, manifest, phục hồi
│   │   ├── Validator.h/.cpp
│   │   ├── DateTime.h/.cpp
│   │   └── IdGenerator.h/.cpp      # Mã tăng dần, không tái sử dụng
│   └── config/                   # Giá trị cấu hình dùng trong nghiệp vụ
│       ├── PricingConfig.h        # Giá, ưu đãi, tích điểm
│       └── SchedulingConfig.h     # Khoảng nghỉ giữa suất
├── ui/                          # Màn hình Qt: nhận thao tác, gọi service, hiển thị kết quả
│   ├── mainwindow.h/.cpp/.ui      # Điều hướng theo vai trò
│   ├── views/
│   │   ├── LoginView.h/.cpp/.ui
│   │   ├── AdminDashboard.h/.cpp/.ui
│   │   ├── StaffDashboard.h/.cpp/.ui
│   │   ├── MovieManagementView.h/.cpp/.ui
│   │   ├── RoomManagementView.h/.cpp/.ui
│   │   ├── ShowtimeManagementView.h/.cpp/.ui
│   │   ├── StaffManagementView.h/.cpp/.ui
│   │   ├── MemberManagementView.h/.cpp/.ui
│   │   ├── BookingView.h/.cpp/.ui  # Chọn vé, xem lại, xác nhận thu tiền
│   │   ├── SeatMapView.h/.cpp     # Widget sơ đồ theo suất
│   │   ├── InvoiceHistoryView.h/.cpp/.ui # Danh sách, chi tiết, hủy cho Admin
│   │   └── ReportView.h/.cpp/.ui
│   ├── viewmodels/               # Tùy chọn: chuyển dữ liệu cho QTableView
│   │   ├── MovieTableModel.h/.cpp
│   │   └── TicketTableModel.h/.cpp
│   └── resources/                # Giao diện, biểu tượng và tài nguyên Qt
│       ├── resources.qrc
│       ├── style.qss
│       └── icons/
├── data/                        # File lưu dữ liệu và vùng phục hồi giao dịch
│   ├── users.txt
│   ├── movies.txt
│   ├── rooms.txt
│   ├── seats.txt                  # Một dòng cho một vị trí ghế
│   ├── showtimes.txt
│   ├── tickets.txt
│   ├── invoices.txt
│   ├── members.txt
│   ├── sequences.txt              # Bộ đếm mã
│   └── transactions/              # Dữ liệu mới, backup, manifest
├── tests/                       # Kiểm tra cấu trúc dữ liệu, đọc/ghi và nghiệp vụ
│   ├── data_structures/            # Kiểm thử từng cấu trúc dữ liệu
│   ├── repositories/               # Parse, schema, tham chiếu và file lỗi
│   ├── services/                   # Quyền và quy tắc nghiệp vụ
│   └── fixtures/                   # Bộ dữ liệu mẫu cho kiểm thử
└── docs/                        # Tài liệu thiết kế và nhật ký phát triển
    ├── TongQuan/
    │   ├── TongQuanDeTai.md
    │   ├── cauTrucThuMuc.md
    │   ├── business_rules.md
    │   └── data_format.md
    ├── BookingService.md
    ├── DS/                        # Tài liệu cấu trúc dữ liệu hiện có
    │   ├── DLL.md
    │   ├── HashTable.md
    │   └── Stack.md
    └── development/               # Tổng quan tiến độ và nhật ký bước đã bắt đầu
        ├── TongQuanTienDo.md
        ├── Buoc01_KhoiTaoDuAn.md
        └── Buoc02_XayDungModels.md
```

## Trách nhiệm các tầng

- UI chuyển đầu vào sang kiểu C++, gọi service và hiển thị kết quả; không đọc/ghi file nghiệp vụ.
- Service kiểm tra quyền và nghiệp vụ; repository truy xuất dữ liệu, không tự quyết định giá hoặc quyền.
- BookingService phối hợp TransactionManager để lưu vé, hóa đơn, điểm và bộ đếm như một giao dịch.
- File là nguồn dữ liệu bền vững. Các cấu trúc tra cứu trong bộ nhớ được dựng sau tải/phục hồi và cập nhật sau commit.
- Quan hệ thực thể dùng mã; vé tham chiếu hóa đơn qua invoiceId, không lưu lặp danh sách mã vé trong invoices.txt.
- Mỗi cấu trúc dữ liệu sở hữu kiểu node riêng; không dùng một `Node` chung cho stack, list, hash table, BST và trie.
- `User` dùng trường `role` thay vì tách lớp `Admin`/`Staff`; phân quyền là trách nhiệm của service.
- Quy tắc xem [business_rules.md](business_rules.md); schema và phục hồi xem [data_format.md](data_format.md).

## Trách nhiệm cụ thể của từng thư mục

Cây phía trên là **cấu trúc mục tiêu**. Hiện dự án mới có một phần mã trong `core/models/` và `core/data_structures/`; các thư mục khác có thể đang rỗng hoặc chưa tồn tại. Chỉ tạo file/thư mục khi triển khai đến phần tương ứng.

### Gốc dự án

| Vị trí | Trách nhiệm |
| --- | --- |
| `CMakeLists.txt` | Khai báo các file nguồn cần biên dịch, chuẩn C++17, thư viện liên kết và mục tiêu build. Khi đến bước giao diện mới cấu hình thêm Qt. Hiện file này chưa có trong cây làm việc. |
| `main.cpp` | Điểm bắt đầu của chương trình. Về sau khởi tạo ứng dụng, nạp dữ liệu và các thành phần cần thiết rồi mở giao diện; không đặt toàn bộ nghiệp vụ vào đây. Hiện file còn rỗng. |
| `README.md` | Giới thiệu mục tiêu, phạm vi, yêu cầu môi trường và cách chạy sau khi đã kiểm chứng. |

### `core/` — chương trình C++ thuần

`core/` chứa dữ liệu và xử lý chính, độc lập với Qt để có thể kiểm tra bằng chương trình C++ thông thường. Các phần bên trong phối hợp theo chiều **Service → Repository → tiện ích lưu file**; model và cấu trúc dữ liệu là kiểu dữ liệu dùng chung, không phải tầng tự gọi ngược lên UI.

| Thư mục | Làm gì | Không làm gì |
| --- | --- | --- |
| `core/models/` | Mỗi class biểu diễn một đối tượng nghiệp vụ và các thuộc tính của nó: `Movie`, `Room`, `Seat`, `Showtime`, `User`, `Member`, `Invoice`, `Ticket`. Constructor tạo đối tượng; getter cho đọc dữ liệu; các thao tác đơn giản như `Ticket::markCancelled()` cập nhật trạng thái của chính đối tượng. Các mã như `Seat.roomId` liên kết đối tượng với nhau. | Không mở file `.txt`, không tự tra cứu đối tượng theo mã, không quyết định quyền Admin/Staff hay kiểm tra toàn bộ quy trình bán/hủy vé. `Room` hiện lưu kích thước phòng; sơ đồ ghế sẽ được dựng từ các `Seat`, chưa nằm trong `Room`. |
| `core/data_structures/` | Tự cài cấu trúc lưu và tra cứu trong bộ nhớ. `DoublyLinkedList` giữ dãy phần tử, `HashTable` tìm nhanh theo khóa, `Stack` hỗ trợ hoàn tác chọn ghế; các cấu trúc dự kiến khác phục vụ gợi ý, sắp xếp hoặc thống kê. | Không coi dữ liệu trong bộ nhớ là file lưu bền vững, không chứa quy tắc bán vé hay giao diện. Hiện mới có ba header đầu tiên và chưa kiểm thử đầy đủ. |
| `core/repositories/` | Tải bản ghi từ file thành model, quản lý tập dữ liệu trong bộ nhớ, tìm/thêm/cập nhật/xóa bản ghi và chuẩn bị dữ liệu để ghi lại. Ví dụ `MovieRepository` quản lý phim; `RoomRepository` phải làm việc với cả phòng và ghế. | Không quyết định ai được sửa phim, có được bán một ghế hay giá vé là bao nhiêu; các điều kiện đó thuộc Service. Hiện chưa có repository được triển khai. |
| `core/service/` hiện có; `core/services/` trong cây mục tiêu | Chứa các service điều phối một thao tác hoàn chỉnh. Ví dụ `BookingService` kiểm tra người thao tác, suất, ghế, giá và điểm; gọi repository, rồi phối hợp lưu hóa đơn/vé/điểm. `ShowtimeService` kiểm tra trùng lịch; `AuthService` kiểm tra đăng nhập và quyền. | Không trực tiếp vẽ giao diện hoặc tự parse từng dòng file. Hai tên thư mục đang khác nhau; khi triển khai service cần chọn một tên thống nhất rồi cập nhật đường dẫn build/include. |
| `core/utils/` | Cung cấp chức năng dùng lại cho nhiều nơi: đọc/ghi file, kiểm tra định dạng đầu vào, thời gian, cấp mã và lưu/phục hồi giao dịch nhiều file. `TransactionManager` dự kiến bảo đảm các file liên quan cùng được cập nhật hoặc cùng được khôi phục. | Không chứa quyết định nghiệp vụ riêng của một chức năng, chẳng hạn ai được hủy vé hoặc giảm giá bao nhiêu. Hiện chưa có tiện ích được triển khai. |
| `core/config/` | Giữ các giá trị cấu hình như giá ghế, mức giảm, quy tắc tích điểm và khoảng nghỉ giữa hai suất để service dùng nhất quán. | Không lưu dữ liệu thay đổi theo từng vé, hóa đơn hay thành viên. Thư mục này chưa được tạo. |

### `ui/` — giao diện người dùng

`ui/` dự kiến chứa Qt Widgets. `mainwindow` điều hướng màn hình; `views/` hiển thị form, bảng, sơ đồ ghế và nhận thao tác; `viewmodels/` (nếu cần) chuyển dữ liệu sang dạng bảng Qt; `resources/` chứa biểu tượng, kiểu hiển thị và tài nguyên giao diện. UI lấy dữ liệu hoặc gửi yêu cầu qua Service, hiển thị kết quả/lỗi cho người dùng. UI không tự đọc `data/*.txt`, tính giá vé, cộng điểm hoặc quyết định quyền. Hiện chưa triển khai Qt.

### `data/` — dữ liệu lưu lâu dài

Các file như `movies.txt`, `users.txt`, `showtimes.txt`, `tickets.txt` và `members.txt` là nguồn dữ liệu sau khi tắt chương trình; `sequences.txt` giữ bộ đếm mã. `data/transactions/` dự kiến giữ file mới, bản sao và manifest trong lúc lưu nhiều file để có thể phục hồi khi bị gián đoạn. Cấu trúc từng dòng và quy tắc phục hồi nằm trong [data_format.md](data_format.md). Thư mục `data/` hiện có, nhưng các file dữ liệu này chưa được tạo trong dự án.

### `tests/` — kiểm chứng chương trình

`tests/data_structures/` kiểm tra các cấu trúc tự cài; `tests/repositories/` kiểm tra đọc/ghi, dữ liệu lỗi và tham chiếu; `tests/services/` kiểm tra quyền và quy tắc nghiệp vụ; `tests/fixtures/` giữ dữ liệu mẫu riêng cho kiểm thử. Kết quả thử cần được ghi trong nhật ký bước tương ứng. Thư mục `tests/` hiện chưa có; sự tồn tại của code hoặc tài liệu chưa chứng minh chức năng chạy đúng.

### `docs/` — tài liệu và nhật ký học tập

`docs/TongQuan/business_rules.md` mô tả quyền và quy tắc nghiệp vụ; `docs/TongQuan/data_format.md` quy định cách lưu dữ liệu; file này mô tả nơi đặt và trách nhiệm của mã. `docs/BookingService.md` giải thích luồng bán vé dự kiến. `docs/DS/` giải thích các cấu trúc dữ liệu đã có. `docs/development/` chứa `TongQuanTienDo.md` và nhật ký của **những bước đã bắt đầu**; không tạo sẵn nhật ký cho cả 15 bước. Tài liệu là căn cứ thiết kế, cần đối chiếu mã và kết quả kiểm thử trước khi đánh dấu một phần hoàn thành.

**Ví dụ luồng bán một vé khi các phần liên quan đã được triển khai:** nhân viên chọn ghế trong `ui/` → `BookingService` kiểm tra quyền, suất chiếu, ghế và tính giá → repository lấy/tạo các model `Ticket`, `Invoice`, `Member` → `TransactionManager` lưu các file liên quan trong `data/` → Service báo kết quả để UI hiển thị. Đây là luồng dự kiến, chưa phải chức năng hiện có.

## Ý nghĩa từng file theo thư mục

Phần này là bảng tra cứu nhanh cho **các file trong cây mục tiêu**. Các cặp `Tên.h/.cpp` nghĩa là: `.h` khai báo class và các hàm có thể dùng, `.cpp` cài đặt các hàm đó. Các file ghi **đã có** tồn tại trong dự án; các file ghi **dự kiến** mới là kế hoạch, chưa có nghĩa chức năng đã chạy. Riêng `core/service/` là tên thư mục hiện có; cây mục tiêu dùng `core/services/` và cần thống nhất tên khi bắt đầu viết Service.

### Gốc dự án

| File | Ý nghĩa | Hiện trạng |
| --- | --- | --- |
| `AGENTS.md` | Quy tắc làm việc, kiến trúc, cách hướng dẫn và theo dõi 15 bước. | Đã có |
| `README.md` | Giới thiệu hệ thống, phạm vi chức năng và kiến trúc tổng quan. | Đã có |
| `main.cpp` | Điểm chương trình bắt đầu chạy; sau này khởi tạo các thành phần và giao diện. | Đã có, còn rỗng |
| `CMakeLists.txt` | Cho CMake biết cần biên dịch file nào và tạo chương trình ra sao. | Chưa có trong cây làm việc |

### `core/models/` — một đối tượng dữ liệu trong mỗi cặp file

| File `.h/.cpp` | Đại diện cho gì? |
| --- | --- |
| `Movie` | Một phim: mã, tên, thể loại, thời lượng, giới hạn tuổi, khoảng ngày chiếu và trạng thái hoạt động. |
| `Room` | Một phòng: mã, tên, số hàng/cột và trạng thái hoạt động; không tự chứa danh sách ghế. |
| `Seat` | Một vị trí ghế của phòng: mã phòng, mã ghế, hàng/cột và loại ghế. |
| `Showtime` | Một suất chiếu: chiếu phim nào, tại phòng nào, lúc nào và trạng thái lịch. |
| `User` | Tài khoản Admin/Staff: thông tin đăng nhập, vai trò và trạng thái hoạt động. |
| `Member` | Thành viên của rạp: mã, tên, số điện thoại và điểm hiện có. |
| `Ticket` | Một vé của một ghế trong một suất: giá, loại vé, điểm và thông tin hủy nếu có. |
| `Invoice` | Một lần bán: mã hóa đơn, suất, người bán, thành viên tùy chọn, thời điểm và tổng tiền ban đầu. |

Cả tám cặp file model **đã có**. Model giữ dữ liệu; mã liên kết đối tượng với nhau, chẳng hạn `Ticket.invoiceId` dẫn tới `Invoice`. Việc kiểm tra quyền và ghi file thuộc các tầng khác.

### `core/data_structures/` — cách tổ chức dữ liệu trong bộ nhớ

| File `.h` | Ý nghĩa | Hiện trạng |
| --- | --- | --- |
| `DoublyLinkedList.h` | Danh sách liên kết đôi để giữ và duyệt các tập model như phim, vé, hóa đơn. | Đã có; chưa kiểm thử đầy đủ |
| `HashTable.h` | Bảng băm để tìm nhanh theo khóa như `userId`, `username`, `invoiceId`. | Đã có; chưa kiểm thử đầy đủ |
| `Stack.h` | Ngăn xếp nhớ thứ tự ghế vừa chọn để Undo trước khi thu tiền. | Đã có; chưa kiểm thử đầy đủ |
| `Trie.h` | Cấu trúc tìm tên phim theo tiền tố để gợi ý khi gõ. | Dự kiến |
| `BST.h` | Chỉ mục hóa đơn theo thời điểm lập và mã hóa đơn để duyệt theo thứ tự. | Dự kiến |
| `MaxHeap.h` | Lấy các phim đứng đầu theo chỉ tiêu thống kê. | Dự kiến |

Ba file đã có là template nên phần cài đặt nằm trong header. Vai trò cụ thể của chúng được giải thích thêm trong `docs/DS/`.

### `core/repositories/` — lấy, giữ và lưu bản ghi

Các cặp file dưới đây **đều dự kiến**; thư mục hiện chưa có mã. Repository quản lý dữ liệu, còn Service quyết định một thao tác có được phép hay không.

| File `.h/.cpp` | Dữ liệu phụ trách |
| --- | --- |
| `AuthRepository` | Tìm tài khoản trong `users.txt` theo mã hoặc tên đăng nhập để hỗ trợ xác thực. |
| `StaffRepository` | Thêm, cập nhật, tra cứu tài khoản Staff trong cùng `users.txt`, giữ nguyên tài khoản Admin. |
| `MovieRepository` | Đọc/ghi và tìm các bản ghi phim trong `movies.txt`. |
| `RoomRepository` | Quản lý phòng trong `rooms.txt` và các vị trí ghế trong `seats.txt`. |
| `ShowtimeRepository` | Quản lý các suất trong `showtimes.txt`. |
| `TicketRepository` | Lưu và tìm vé trong `tickets.txt`, kể cả vé đã hủy để giữ lịch sử. |
| `InvoiceRepository` | Lưu và tìm hóa đơn trong `invoices.txt`; vé liên kết qua `invoiceId`. |
| `MemberRepository` | Quản lý thành viên và điểm lưu trong `members.txt`. |

### `core/services/` — kiểm tra quy tắc và điều phối công việc

Các cặp file dưới đây **đều dự kiến**; thư mục thực tế hiện tên `core/service/` và chưa có mã.

| File `.h/.cpp` | Nghiệp vụ phụ trách |
| --- | --- |
| `AuthService` | Kiểm tra đăng nhập, tài khoản đang hoạt động và quyền Admin/Staff. |
| `StaffService` | Kiểm tra quyền khi quản lý nhân viên và các ràng buộc với tài khoản. |
| `MovieService` | Kiểm tra dữ liệu và quy tắc khi thêm, sửa, ngừng hoặc xóa phim. |
| `RoomService` | Kiểm tra sơ đồ ghế, phòng hoạt động và điều kiện được sửa phòng. |
| `ShowtimeService` | Kiểm tra thời gian, trùng lịch, điều kiện tạo/sửa/hủy suất. |
| `MemberService` | Kiểm tra thông tin thành viên và quy tắc liên quan đến điểm. |
| `BookingService` | Giữ ghế chọn tạm, Undo, tính giá/điểm, bán vé và điều phối hủy vé theo quy tắc riêng. Xem [tài liệu quy trình](../BookingService.md). |
| `ReportService` | Lọc lịch sử hóa đơn, tính số vé/tiền và top phim. |

### `core/utils/` và `core/config/` — công cụ dùng chung

Các file dưới đây **đều dự kiến**. `core/utils/` hiện rỗng; `core/config/` chưa được tạo.

| File `.h/.cpp` hoặc `.h` | Ý nghĩa |
| --- | --- |
| `FileHandler` | Đọc/ghi file văn bản và xử lý lỗi truy cập file ở mức kỹ thuật. |
| `TransactionManager` | Lưu đồng bộ nhiều file; dùng dữ liệu mới, backup và manifest để phục hồi khi ghi lỗi. |
| `Validator` | Kiểm tra định dạng đầu vào dùng chung như chuỗi, số và trường văn bản. |
| `DateTime` | Kiểm tra, tính toán và so sánh ngày giờ theo định dạng đã thống nhất. |
| `IdGenerator` | Tạo mã mới từ `sequences.txt`, không cấp lại mã đã dùng. |
| `PricingConfig.h` | Các mức giá ghế, giảm giá và quy tắc tích điểm mặc định. |
| `SchedulingConfig.h` | Khoảng nghỉ tối thiểu giữa các suất cùng phòng. |

`Validator` kiểm tra **định dạng**; Service vẫn phải kiểm tra quy tắc nghiệp vụ cụ thể. `TransactionManager` không tự quyết định bán vé hay hủy vé.

### `ui/` — các màn hình Qt dự kiến

Chưa có file giao diện. Với các màn hình `.h/.cpp/.ui`: `.h` khai báo lớp, `.cpp` xử lý tương tác và gọi Service, `.ui` mô tả bố cục do Qt Designer tạo. `SeatMapView` trong cây mục tiêu chỉ có `.h/.cpp` vì sơ đồ ghế có thể được vẽ bằng code.

| File | Người dùng thấy hoặc làm gì? |
| --- | --- |
| `mainwindow.h/.cpp/.ui` | Cửa sổ chính, mở các màn hình phù hợp vai trò. |
| `views/LoginView.h/.cpp/.ui` | Nhập tên đăng nhập/mật khẩu và xem kết quả đăng nhập. |
| `views/AdminDashboard.h/.cpp/.ui` | Các lối vào chức năng của Admin. |
| `views/StaffDashboard.h/.cpp/.ui` | Các lối vào chức năng của Staff. |
| `views/MovieManagementView.h/.cpp/.ui` | Xem và quản lý phim. |
| `views/RoomManagementView.h/.cpp/.ui` | Xem phòng và sơ đồ ghế. |
| `views/ShowtimeManagementView.h/.cpp/.ui` | Xem lịch, tạo/sửa/hủy suất. |
| `views/StaffManagementView.h/.cpp/.ui` | Quản lý tài khoản nhân viên. |
| `views/MemberManagementView.h/.cpp/.ui` | Tra cứu và sửa thông tin liên hệ thành viên. |
| `views/BookingView.h/.cpp/.ui` | Chọn ghế/loại vé, xem tiền, Undo và xác nhận thu tiền. |
| `views/SeatMapView.h/.cpp` | Hiển thị các vị trí ghế theo phòng/suất và trạng thái chọn/bán. |
| `views/InvoiceHistoryView.h/.cpp/.ui` | Xem hóa đơn, vé và thao tác hủy khi có quyền. |
| `views/ReportView.h/.cpp/.ui` | Hiển thị các thống kê và top phim. |
| `viewmodels/MovieTableModel.h/.cpp` | Chuyển danh sách phim thành dữ liệu cho bảng Qt; chỉ cần tạo nếu UI dùng kiểu bảng này. |
| `viewmodels/TicketTableModel.h/.cpp` | Chuyển danh sách vé thành dữ liệu cho bảng Qt; cũng là phần tùy chọn. |
| `resources/resources.qrc` | Khai báo tài nguyên được đóng gói vào ứng dụng Qt. |
| `resources/style.qss` | Quy định màu sắc, phông chữ và cách hiển thị widget. |
| `resources/icons/` | Nơi đặt các file biểu tượng nếu giao diện cần. |

### `data/` — mỗi file là một loại bản ghi

Các file sau **chưa có trong cây làm việc**; tên và thứ tự trường phải theo [data_format.md](data_format.md). Các file `.txt` là dữ liệu của ứng dụng, không phải mã C++.

| File | Dữ liệu lưu |
| --- | --- |
| `users.txt` | Tài khoản Admin/Staff. |
| `movies.txt` | Danh mục phim. |
| `rooms.txt` | Thông tin phòng. |
| `seats.txt` | Từng vị trí ghế thuộc một phòng. |
| `showtimes.txt` | Suất chiếu và thời gian. |
| `tickets.txt` | Vé đã bán và lịch sử vé đã hủy. |
| `invoices.txt` | Hóa đơn bán vé; không lưu lặp danh sách vé. |
| `members.txt` | Thông tin và điểm thành viên. |
| `sequences.txt` | Số tiếp theo để cấp mã cho từng loại đối tượng. |
| `transactions/` | Thư mục tạm cho dữ liệu mới, backup và manifest khi ghi nhiều file. |

### `tests/` và `docs/` — kiểm thử và tài liệu

`tests/` **chưa được tạo**. `tests/data_structures/` dự kiến kiểm tra danh sách, bảng băm, stack; `tests/repositories/` thử đọc/ghi và dữ liệu lỗi; `tests/services/` thử quyền, giá và các luồng nghiệp vụ; `tests/fixtures/` giữ bộ dữ liệu mẫu. Tên từng file kiểm thử sẽ đặt khi viết bài kiểm tra thật, chưa có danh sách cố định.

| File tài liệu hiện có | Dùng để xem gì? |
| --- | --- |
| `docs/TongQuan/TongQuanDeTai.md` | Tổng quan đề tài, quyền Admin/Staff và luồng UI → Service → Repository của từng chức năng. |
| `docs/TongQuan/cauTrucThuMuc.md` | Cây thư mục mục tiêu và ý nghĩa các file; chính là tài liệu này. |
| `docs/TongQuan/business_rules.md` | Ai được làm gì và các điều kiện nghiệp vụ khi quản lý/bán/hủy vé. |
| `docs/TongQuan/data_format.md` | Cách ghi từng file dữ liệu, kiểm tra dữ liệu và phục hồi giao dịch. |
| `docs/BookingService.md` | Diễn giải tuần tự quy trình bán vé và Undo trước thu tiền. |
| `docs/DS/DLL.md` | Giải thích danh sách liên kết đôi hiện có và vai trò dự kiến. |
| `docs/DS/HashTable.md` | Giải thích bảng băm hiện có và cách tra cứu theo khóa. |
| `docs/DS/Stack.md` | Giải thích stack hiện có và cách Undo chọn ghế. |
| `docs/development/TongQuanTienDo.md` | Trạng thái của cả 15 bước, kèm liên kết nhật ký đã tạo. |
| `docs/development/Buoc01_KhoiTaoDuAn.md` | Công việc, vướng mắc và việc tiếp theo của Bước 01. |
| `docs/development/Buoc02_XayDungModels.md` | Công việc, vướng mắc và việc tiếp theo của Bước 02. |

Các file nhật ký Bước 03–15 **chưa được tạo sẵn**; chỉ tạo khi thực sự bắt đầu bước tương ứng.
