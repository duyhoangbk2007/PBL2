# Cấu trúc thư mục mục tiêu

Đây là cấu trúc dự kiến khi hoàn thiện, không phải danh sách file đã tồn tại. `.h/.cpp` là hai file; `.h/.cpp/.ui` là ba file. File hiện có `doublyLinkedList.h` cần thống nhất thành `DoublyLinkedList.h` và cập nhật include khi triển khai.

```text
PBL2/
├── CMakeLists.txt                  # Build C++ và Qt (dự kiến)
├── main.cpp                       # QApplication và khởi tạo phụ thuộc
├── README.md
├── core/                          # C++ thuần, không phụ thuộc Qt
│   ├── models/
│   │   ├── User.h/.cpp             # Tài khoản, vai trò, trạng thái
│   │   ├── Admin.h/.cpp            # Kế thừa User
│   │   ├── Staff.h/.cpp            # Kế thừa User
│   │   ├── Movie.h/.cpp            # Phim, khoảng ngày chiếu
│   │   ├── Room.h/.cpp             # Phòng, mảng ghế hai chiều
│   │   ├── Seat.h/.cpp             # Vị trí, loại; không có trạng thái bán
│   │   ├── Showtime.h/.cpp         # Phim, phòng, bắt đầu/kết thúc, trạng thái
│   │   ├── Ticket.h/.cpp           # Ghế theo suất, giá chốt, điểm, thông tin hủy
│   │   ├── Invoice.h/.cpp          # Người bán, thành viên, thời điểm, tổng tiền
│   │   └── Member.h/.cpp           # Liên hệ và điểm; chưa có hạng thẻ
│   ├── data_structures/            # Tự cài đặt, header-only
│   │   ├── Node.h
│   │   ├── DoublyLinkedList.h      # Danh sách thực thể
│   │   ├── HashTable.h             # ID tài khoản (userId), mã hóa đơn (invoiceId)
│   │   ├── Stack.h                 # Undo chọn ghế trước thanh toán
│   │   ├── Trie.h                  # Gợi ý tên phim
│   │   ├── BST.h                   # Khóa (createdAt, invoiceId)
│   │   └── MaxHeap.h               # Top phim
│   ├── repositories/
│   │   ├── AuthRepository.h/.cpp   # Đọc users.txt
│   │   ├── StaffRepository.h/.cpp  # Quản lý Staff trong users.txt, giữ Admin
│   │   ├── MovieRepository.h/.cpp
│   │   ├── RoomRepository.h/.cpp   # rooms.txt và seats.txt
│   │   ├── ShowtimeRepository.h/.cpp
│   │   ├── TicketRepository.h/.cpp
│   │   ├── InvoiceRepository.h/.cpp
│   │   └── MemberRepository.h/.cpp
│   ├── services/
│   │   ├── AuthService.h/.cpp      # Xác thực, phân quyền
│   │   ├── StaffService.h/.cpp
│   │   ├── MovieService.h/.cpp
│   │   ├── RoomService.h/.cpp
│   │   ├── ShowtimeService.h/.cpp  # Lịch và ràng buộc sửa/hủy
│   │   ├── MemberService.h/.cpp    # Liên hệ và quy tắc điểm
│   │   ├── BookingService.h/.cpp   # Bán/hủy vé, điều phối giao dịch
│   │   └── ReportService.h/.cpp    # Lịch sử, thống kê, top phim
│   ├── utils/
│   │   ├── FileHandler.h/.cpp
│   │   ├── TransactionManager.h/.cpp # File tạm, backup, manifest, phục hồi
│   │   ├── Validator.h/.cpp
│   │   ├── DateTime.h/.cpp
│   │   └── IdGenerator.h/.cpp      # Mã tăng dần, không tái sử dụng
│   └── config/
│       ├── PricingConfig.h        # Giá, ưu đãi, tích điểm
│       └── SchedulingConfig.h     # Khoảng nghỉ giữa suất
├── ui/
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
│   ├── viewmodels/                # Tùy chọn: adapter QTableView
│   │   ├── MovieTableModel.h/.cpp
│   │   └── TicketTableModel.h/.cpp
│   └── resources/
│       ├── resources.qrc
│       ├── style.qss
│       └── icons/
├── data/
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
└── docs/
    ├── cauTrucThuMuc.md
    ├── business_rules.md
    ├── data_format.md
    └── DS/                        # Tài liệu cấu trúc dữ liệu hiện có
```

## Trách nhiệm các tầng

- UI chuyển đầu vào sang kiểu C++, gọi service và hiển thị kết quả; không đọc/ghi file nghiệp vụ.
- Service kiểm tra quyền và nghiệp vụ; repository truy xuất dữ liệu, không tự quyết định giá hoặc quyền.
- BookingService phối hợp TransactionManager để lưu vé, hóa đơn, điểm và bộ đếm như một giao dịch.
- File là nguồn dữ liệu bền vững. Các cấu trúc tra cứu trong bộ nhớ được dựng sau tải/phục hồi và cập nhật sau commit.
- Quan hệ thực thể dùng mã; vé tham chiếu hóa đơn qua invoiceId, không lưu lặp danh sách mã vé trong invoices.txt.
- Quy tắc xem [business_rules.md](business_rules.md); schema và phục hồi xem [data_format.md](data_format.md).

