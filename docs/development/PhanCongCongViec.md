# Phân công phát triển cho hai thành viên

## 1. Mục tiêu

Hai người dùng chung một repository. Hoàng phụ trách quản lý rạp; Kiệt phụ trách tài khoản và bán vé. Mỗi module có một người chịu trách nhiệm chính, người còn lại review phần giao nhau. Đây là phân công cho công việc tiếp theo; nhật ký cũ không ghi tác giả từng file nên không suy diễn ai đã làm phần nào.

## 2. Lộ trình 15 bước

| Bước | Nội dung | Hoàng | Kiệt |
| --- | --- | --- | --- |
| 01 | Khởi tạo dự án | Thiết lập cấu trúc thư mục, `CMakeLists.txt`, `main.cpp`, kiểm tra build C++ | Chuẩn bị `README.md`, `.gitignore`, tài liệu, thiết lập quy trình GitHub |
| 02 | Xây dựng Models | `Movie`, `Room`, `Seat`, `Showtime` | `User`, `Member`, `Ticket`, `Invoice` |
| 03 | Cấu trúc dữ liệu | `DoublyLinkedList`, `Trie`, `MaxHeap` | `HashTable`, `Stack`, `BST` |
| 04 | Hạ tầng lưu trữ | `FileHandler`, `Validator`, quy tắc định dạng dữ liệu | `TransactionManager`, `IdGenerator`, `DateTime`, quy tắc giao dịch |
| 05 | Repositories | `MovieRepository`, `RoomRepository`, `ShowtimeRepository` | `AuthRepository`, `StaffRepository`, `MemberRepository`, `TicketRepository`, `InvoiceRepository` |
| 06 | Đăng nhập và phân quyền | Xây dựng, kiểm thử quy tắc phân quyền Admin/Staff và tích hợp quyền vào nghiệp vụ | `AuthService`, `StaffService`, đăng nhập và quản lý nhân viên |
| 07 | Quản lý phim và thành viên | `MovieService`, thêm/sửa/xóa/tìm kiếm phim | `MemberService`, quản lý thành viên và điểm tích lũy |
| 08 | Quản lý phòng và ghế | `RoomService`, quản lý cấu trúc phòng, loại ghế | Xử lý biểu diễn sơ đồ ghế hai chiều, viết test kiểm tra dữ liệu phòng/ghế |
| 09 | Suất chiếu và giá vé | `ShowtimeService`, kiểm tra trùng lịch và ràng buộc suất chiếu | Xây dựng quy tắc tính giá, `PricingConfig`, kiểm thử giá vé |
| 10 | Chọn ghế, đặt vé | Cung cấp dữ liệu trạng thái ghế theo suất chiếu, kiểm thử khả dụng ghế | `BookingService`, chọn ghế, kiểm tra ghế đã bán, Undo trước thanh toán |
| 11 | Thanh toán và lưu vé | Kiểm thử tích hợp việc lưu vé, hóa đơn và phục hồi khi lỗi giao dịch | Hoàn thiện `BookingService`, tạo `Ticket`, `Invoice`, lưu giao dịch an toàn |
| 12 | Hủy vé, lịch sử, báo cáo | `ReportService`, thống kê doanh thu, số vé, phim | Xử lý hủy vé, truy vấn lịch sử vé và hóa đơn |
| 13 | Xây dựng nền tảng Qt | `MainWindow`, `AdminDashboard`, tài nguyên và bố cục giao diện chung | `LoginView`, `StaffDashboard`, tích hợp luồng đăng nhập |
| 14 | Giao diện chức năng | Giao diện quản lý phim, phòng, suất chiếu, báo cáo; `MovieTableModel` | Giao diện nhân viên, thành viên, đặt vé, sơ đồ ghế, lịch sử hóa đơn; `TicketTableModel` |
| 15 | Kiểm thử, đóng gói, báo cáo | Kiểm thử phần quản trị, cấu hình build/đóng gói, tổng hợp kiến trúc | Kiểm thử nghiệp vụ bán vé, giao dịch, hoàn thiện hướng dẫn sử dụng và nội dung báo cáo |

## 3. Nhiệm vụ và file/module chịu trách nhiệm

| Người | File/module chính | Nhiệm vụ |
| --- | --- | --- |
| Hoàng | `core/models/{Movie,Room,Seat,Showtime}.h/.cpp`; `core/data_structures/{DoublyLinkedList,Trie,MaxHeap}.h`; `FileHandler`, `Validator`; Movie/Room/Showtime Repository và Service; `ReportService`; `MainWindow`, `AdminDashboard`, `MovieTableModel` và các giao diện quản lý | Quản lý phim, phòng/ghế, lịch suất; cung cấp trạng thái ghế; thống kê và kiểm thử phần quản trị. Chịu trách nhiệm chính `CMakeLists.txt`, `main.cpp`, build và đóng gói. |
| Kiệt | `core/models/{User,Member,Ticket,Invoice}.h/.cpp`; `core/data_structures/{HashTable,Stack,BST}.h`; `TransactionManager`, `IdGenerator`, `DateTime`; Auth/Staff/Member/Ticket/Invoice Repository; Auth/Staff/Member/Booking Service; `PricingConfig`, `LoginView`, `StaffDashboard`, `TicketTableModel` và các giao diện bán vé | Quản lý tài khoản, thành viên, giá, chọn ghế và Undo, bán/hủy vé, hóa đơn; biểu diễn sơ đồ ghế hai chiều và kiểm thử dữ liệu phòng/ghế. Chịu trách nhiệm chính `README.md`, `.gitignore`, quy trình Git và hướng dẫn sử dụng. |

Các tên Repository, Service, View và CTDL chưa có là **kế hoạch**, theo [cấu trúc mục tiêu](../TongQuan/cauTrucThuMuc.md). Kiệt cài `BST` để phục vụ truy vấn lịch sử; Hoàng cài `MaxHeap` cho thống kê. Hoàng quản lý `RoomService`, Kiệt phụ trách biểu diễn sơ đồ ghế hai chiều; hai người phải thống nhất dữ liệu đầu vào và đầu ra. Tài liệu và các file cấu hình chung cần cả hai review.

## 4. Phụ thuộc và file dùng chung

| Phụ thuộc | Cần thống nhất trước | Cách làm song song |
| --- | --- | --- |
| `CMakeLists.txt`, `main.cpp`, `README.md`, `.gitignore`, tài liệu và quy ước tên | Hoàng giữ build; Kiệt giữ README, Git và tài liệu hướng dẫn. Thống nhất target, tên thư mục `core/service/` hiện có hay `core/services/` theo thiết kế. | PR nhỏ; người kia review thay đổi chung. |
| Model và `DoublyLinkedList.h` | ID là dữ liệu lưu file; con trỏ không sở hữu. Chốt nơi giữ object, địa chỉ ổn định, cách sao chép DLL, API `Room`–`Seat`, `Showtime`–`Movie`/`Room`, `Invoice`–`Ticket`. | Ghi chữ ký API trong [QuanHeModels.md](QuanHeModels.md) trước; mỗi người sửa model mình giữ. |
| FileHandler/Validator của Hoàng; TransactionManager/IdGenerator/DateTime của Kiệt; schema `.txt` | Thứ tự đọc, lỗi tham chiếu, cấp mã, định dạng thời gian, ghi/rollback nhiều file theo `data_format.md`. | Chốt interface nhỏ trước Bước 04; phát triển với dữ liệu kiểm thử riêng rồi ghép. |
| Ghế trống và BookingService | Hoàng cung cấp suất/phòng/ghế; Kiệt cung cấp vé SOLD và quy trình bán. | Chốt API đọc tối thiểu, thử bằng dữ liệu mẫu; tích hợp từng phần trước Bước 10–11. |
| Sơ đồ ghế, báo cáo và Qt | Kiệt cần phòng/ghế của Hoàng để biểu diễn sơ đồ; Hoàng cần vé/hóa đơn và BST lịch sử của Kiệt cho báo cáo. UI chỉ gọi Service. | Chốt dữ liệu trả về và quyền sở hữu `SeatMapView`; làm View bằng dữ liệu mẫu trong lúc Service hoàn thiện. |

Chưa được tự thay đổi interface dùng chung. Những điểm còn cần quyết định: chủ sở hữu object và vòng đời con trỏ, nhất là Bước 02/10/11; copy/move của các CTDL cấp phát động; hợp đồng giữa FileHandler–TransactionManager; tên thư mục Service; vị trí xử lý sơ đồ ghế hai chiều; định dạng dữ liệu Service trả cho UI. Quyết định cần ghi trong tài liệu hoặc PR trước khi triển khai phụ thuộc.

## 5. Git branch và Pull Request

`main` giữ bản đã tích hợp. Tên nhánh tính năng theo mẫu `feature/bXX-hoang-chuc-nang` hoặc `feature/bXX-kiet-chuc-nang`, trong đó `XX` là số bước gồm hai chữ số và `chuc-nang` là nhiệm vụ thực tế, viết chữ thường và nối bằng dấu `-`.

| Bước | Nhánh ví dụ của Hoàng | Nhánh ví dụ của Kiệt |
| --- | --- | --- |
| 01 | `feature/b01-hoang-setup` | `feature/b01-kiet-docs` |
| 02 | `feature/b02-hoang-models` | `feature/b02-kiet-models` |
| 03 | `feature/b03-hoang-structures` | `feature/b03-kiet-structures` |
| 04 | `feature/b04-hoang-file-storage` | `feature/b04-kiet-transactions` |
| 05 | `feature/b05-hoang-repositories` | `feature/b05-kiet-repositories` |

Bước 06–15 đặt tên tương tự theo việc thực tế. Một bước có thể tách thành nhiều nhánh/PR nhỏ với phần `chuc-nang` khác nhau. **Không tạo sẵn 30 nhánh**: khi bắt đầu phần việc của một bước, mỗi người mới tạo nhánh tương ứng từ `main` mới nhất.

1. Trước khi chuyển nhánh, xem `git status` và bảo vệ thay đổi chưa commit. Mỗi PR tập trung một việc; nếu sửa file người kia hoặc API chung thì thống nhất trước.
2. Sau khi tự kiểm tra, người làm push nhánh và mở Pull Request vào `main`, ghi file đổi, lệnh/kết quả kiểm thử và phụ thuộc còn mở. Người kia review và thử phần giao nhau rồi mới merge.
3. Sau merge, cả hai cập nhật `main` và kiểm tra tích hợp; có thể xóa nhánh tính năng đã gộp. Tránh push trực tiếp hoặc force push `main`. Trợ lý không tự commit/push/merge khi người dùng chưa yêu cầu.

## 6. Nghiệm thu mỗi bước và nhật ký

Một bước chỉ hoàn thành khi phần Hoàng, phần Kiệt và tích hợp chung có bằng chứng từ code, biên dịch/chạy hoặc kiểm thử phù hợp, đối chiếu quy tắc nghiệp vụ và schema. Kiểm tra cú pháp một `.cpp` không thay cho liên kết/chạy cả phần cần nghiệm thu. Nếu một bên xong trước, ghi phần đã xác minh nhưng trạng thái chung vẫn đang thực hiện. Bước 05 và 14 có nhiều module, nên chia thành các Pull Request nhỏ để dễ review và tích hợp.

Chỉ tạo `BuocNN_*.md` khi thực sự bắt đầu bước NN. Nhật ký gồm: công việc Hoàng, công việc Kiệt, tích hợp chung, checklist nghiệm thu, lỗi/cách khắc phục/kết quả kiểm thử và phần việc tiếp theo. Ghi thêm kiến thức, thay đổi code và lệnh thử để khôi phục ngữ cảnh. Nhật ký Bước 01/02 đã có: khi quay lại thì bổ sung các mục Hoàng/Kiệt/tích hợp vào file cũ, giữ nguyên lịch sử.

## 7. Tích hợp và kiểm thử hai phần

1. Đọc tiến độ, nhật ký, tài liệu giao diện; chốt đầu vào/đầu ra và người giữ module.
2. Hai người làm trên nhánh riêng, dùng dữ liệu mẫu hoặc stub cho phần chưa có; tự kiểm tra module mình.
3. Ghép theo phụ thuộc: model/CTDL → hạ tầng file → Repository → Service → UI. Với booking, ghép tra cứu suất/phòng/ghế của Hoàng với vé/BookingService của Kiệt.
4. Review PR, biên dịch/liên kết, chạy ca thành công và ca lỗi; với giao dịch nhiều file, thử lỗi ghi và phục hồi. Ghi lệnh, kết quả và cách sửa trong nhật ký.
5. Sau nghiệm thu mới cập nhật tiến độ; nếu còn lỗi thì ghi người xử lý và không đánh dấu hoàn thành.
