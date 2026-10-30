# Tổng quan tiến độ PBL2 — hai thành viên

**Rà soát 2026-10-30:** Đã chuyển tám cặp Model sang `core/dal/models/` và sửa include tương đối, không đổi thuật toán hay quan hệ Model. Hiện chỉ có `DoublyLinkedList.h`; `HashTable.h`, `Stack.h`, CMake, GUI, DALC, BLL và file dữ liệu không có trong cây làm việc. Bước 03 vẫn chưa nghiệm thu. Các mô tả về header hoặc working tree trước đây ở phần dưới là ghi chép lịch sử.

Theo [phân công](PhanCongCongViec.md), Hoàng phụ trách quản lý rạp và Kiệt phụ trách tài khoản/bán vé. Bảng ghi trạng thái **thực tế**; nhiệm vụ được giao chưa phải bằng chứng đã làm. Đối chiếu code, nhật ký và kiểm thử trước khi đổi trạng thái.

| Bước | Nội dung | Hoàng | Kiệt | Trạng thái chung | Nhật ký |
| --- | --- | --- | --- | --- | --- |
| 01 | Khởi tạo dự án | Cấu trúc thư mục, `CMakeLists.txt`, `main.cpp`, build C++ | `README.md`, `.gitignore`, tài liệu, quy trình GitHub | Đang thực hiện: đã có khung thư mục và Git; `main.cpp` rỗng, chưa có `CMakeLists.txt` hoặc bản build được xác minh. Chưa nghiệm thu riêng Hoàng/Kiệt. | [Bước 01](Buoc01_KhoiTaoDuAn.md) |
| 02 | Xây dựng Models | `Movie`, `Room`, `Seat`, `Showtime` | `User`, `Member`, `Ticket`, `Invoice` | Đang thực hiện: đủ 8 cặp `.h/.cpp`; đã kiểm tra cú pháp và thử riêng Ticket/Showtime, chưa thử tích hợp cả 8 model hoặc vòng đời con trỏ/DLL. | [Bước 02](Buoc02_XayDungModels.md) |
| 03 | Cấu trúc dữ liệu | `DoublyLinkedList<T>` tự cài | CTDL khác dùng STL khi phù hợp | Chưa bắt đầu bước: hiện có DLL; `HashTable.h` và `Stack.h` không có trong cây làm việc; chưa kiểm thử hành vi/ nghiệm thu Bước 03. | Chưa tạo |
| 04 | Hạ tầng lưu trữ | `FileHandler`, `Validator`, định dạng dữ liệu | `TransactionManager`, `IdGenerator`, `DateTime`, giao dịch | Chưa bắt đầu; mới là thiết kế. | Chưa tạo |
| 05 | Repositories | `MovieRepository`, `RoomRepository`, `ShowtimeRepository` | `AuthRepository`, `StaffRepository`, `MemberRepository`, `TicketRepository`, `InvoiceRepository` | Chưa bắt đầu; mới là thiết kế. | Chưa tạo |
| 06 | Đăng nhập và phân quyền | Quy tắc phân quyền Admin/Staff, kiểm thử và tích hợp | `AuthService`, `StaffService`, đăng nhập và nhân viên | Chưa bắt đầu; mới là thiết kế. | Chưa tạo |
| 07 | Quản lý phim và thành viên | `MovieService`, thêm/sửa/xóa/tìm phim | `MemberService`, thành viên và điểm | Chưa bắt đầu; mới là thiết kế. | Chưa tạo |
| 08 | Quản lý phòng và ghế | `RoomService`, cấu trúc phòng và loại ghế | Sơ đồ ghế hai chiều, test dữ liệu phòng/ghế | Chưa bắt đầu; mới là thiết kế. | Chưa tạo |
| 09 | Suất chiếu và giá vé | `ShowtimeService`, trùng lịch và ràng buộc | `PricingConfig`, quy tắc và kiểm thử giá vé | Chưa bắt đầu; mới là thiết kế. | Chưa tạo |
| 10 | Chọn ghế, đặt vé | Dữ liệu trạng thái ghế theo suất, kiểm thử khả dụng | `BookingService`, chọn ghế, kiểm tra vé đã bán, Undo | Chưa bắt đầu; mới là thiết kế. | Chưa tạo |
| 11 | Thanh toán và lưu vé | Kiểm thử tích hợp lưu vé/hóa đơn, phục hồi lỗi giao dịch | `BookingService`, tạo `Ticket`/`Invoice`, lưu giao dịch | Chưa bắt đầu; mới là thiết kế. | Chưa tạo |
| 12 | Hủy vé, lịch sử, báo cáo | `ReportService`, thống kê doanh thu, vé và phim | Hủy vé, truy vấn lịch sử vé/hóa đơn | Chưa bắt đầu; mới là thiết kế. | Chưa tạo |
| 13 | Xây dựng nền tảng Qt | `MainWindow`, `AdminDashboard`, tài nguyên và bố cục chung | `LoginView`, `StaffDashboard`, luồng đăng nhập | Chưa bắt đầu; mới là thiết kế. | Chưa tạo |
| 14 | Giao diện chức năng | View phim/phòng/suất/báo cáo, `MovieTableModel` | View nhân viên/thành viên/đặt vé/sơ đồ ghế/lịch sử, `TicketTableModel` | Chưa bắt đầu; mới là thiết kế. | Chưa tạo |
| 15 | Kiểm thử, đóng gói, báo cáo | Kiểm thử quản trị, build/đóng gói, kiến trúc | Kiểm thử bán vé/giao dịch, hướng dẫn sử dụng, báo cáo | Chưa bắt đầu; mới là thiết kế. | Chưa tạo |

## Bằng chứng và điểm còn mở khi đổi lộ trình

- Bước 01: [nhật ký](Buoc01_KhoiTaoDuAn.md) ghi đã khảo sát thư mục và tài liệu CTDL; chưa có build. Git có `main` theo dõi `origin/main`, nhưng quy trình hai người chưa được nghiệm thu.
- Bước 02: [nhật ký](Buoc02_XayDungModels.md) giữ các mốc kiểm tra cũ. Code hiện đã có `Member::setPoints(const int&)` và con trỏ quan hệ ở Room, Showtime, Invoice, Ticket; đoạn nhật ký nói chưa có là lịch sử, không phản ánh code mới nhất. Chưa có kiểm thử tích hợp đầy đủ. Xem [QuanHeModels.md](QuanHeModels.md).
- Bước 03: DLL, HashTable, Stack đã tồn tại; include guard được thử trong Bước 02, chưa có kiểm thử hành vi. Trie, BST, MaxHeap chưa có. Chưa tạo nhật ký Bước 03.
- `docs/BookingService.md` đang bị xóa trong working tree; `QuanHeModels.md` đang được chuyển từ `docs/TongQuan/` sang `docs/development/`. Một số liên kết trong tài liệu cũ có thể sai; lần cập nhật này không phục hồi hoặc di chuyển chúng.

Chỉ tạo một nhật ký cho bước khi thực sự bắt đầu; khi quay lại, cập nhật file đã có. Nhật ký ghi việc Hoàng, việc Kiệt, tích hợp, checklist nghiệm thu, lỗi/cách khắc phục/kết quả thử và phần việc tiếp theo; lưu cả kiến thức, thay đổi code và bằng chứng để khôi phục ngữ cảnh.
