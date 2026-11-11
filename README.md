# PBL2 – Hệ thống quản lý bán vé rạp chiếu phim

## 1. Giới thiệu

Ứng dụng desktop C++/Qt mô phỏng bán vé offline tại quầy. Nhân viên thao tác thay khách; không có tài khoản khách hàng hoặc đặt vé online. Hai vai trò sử dụng là Admin và Staff. Dữ liệu lưu bằng file văn bản UTF-8.

Tài liệu mô tả thiết kế mục tiêu, không khẳng định các chức năng bên dưới đã triển khai. Hiện dự án có tám Model, `DoublyLinkedList<T>` và các tiện ích lưu trữ/ngày giờ/cấp mã; chưa có ứng dụng bán vé hoàn chỉnh. `main.cpp` chỉ là chương trình thử, chưa phải điểm khởi chạy giao diện. Bản PBL2 dự kiến chỉ cho phép một phiên ứng dụng thao tác trên một bộ dữ liệu tại một thời điểm.

## 2. Chức năng

### 2.1. Đăng nhập và phân quyền

- Xác thực tài khoản đang hoạt động từ `users.txt`, xác định Admin/Staff.
- Service kiểm tra quyền, không chỉ ẩn chức năng trên giao diện.

### 2.2. Admin

- Quản lý phim: thêm, sửa, tìm kiếm, ngừng hoạt động; xóa khi chưa được tham chiếu.
- Quản lý phòng và sơ đồ ghế thường/VIP/đôi.
- Quản lý suất chiếu: tạo, sửa, hủy, xem lịch và kiểm tra trùng lịch cùng phòng.
- Quản lý nhân viên: thêm, sửa, tìm kiếm, vô hiệu hóa; xóa khi chưa có dữ liệu tham chiếu.
- Xem toàn bộ hóa đơn; lọc theo thời gian, nhân viên, phim hoặc suất chiếu.
- Hủy vé trước giờ chiếu, hoàn tiền và thu hồi điểm đã cộng.
- Thống kê tiền bán, tiền hoàn, doanh thu thuần, số vé bán/hủy và top phim.
- Được thực hiện nghiệp vụ bán vé; hóa đơn ghi tài khoản thực sự thao tác.

### 2.3. Staff

- Xem phim sắp chiếu/đang chiếu, tìm kiếm và nhận gợi ý tên phim.
- Tạo, tra cứu, cập nhật thông tin liên hệ thành viên; không sửa điểm trực tiếp.
- Bán vé theo quy trình:
  1. Chọn phim và suất chiếu chưa bắt đầu.
  2. Xem ghế trống/đã bán của suất đó.
  3. Chọn ghế, loại vé; có thể hoàn tác chọn ghế bằng stack.
  4. Gắn thành viên nếu có, áp dụng ưu đãi và tính tiền.
  5. Xem lại giao dịch, xác nhận điều kiện độ tuổi và đã thu tiền tại quầy.
  6. Kiểm tra lại ghế, tạo vé/hóa đơn, cộng điểm và lưu dữ liệu.
  7. Chỉ thông báo thành công khi lưu đầy đủ giao dịch.
- Bỏ giao dịch trước thanh toán không tạo vé, hóa đơn hoặc điểm.
- Xem hóa đơn của mình trong ngày; tra cứu hóa đơn cũ của mình theo mã.

### 2.4. Phạm vi thống nhất

- Một hóa đơn chứa một hoặc nhiều vé của cùng một suất chiếu, lưu `showtimeId` và gắn tối đa một thành viên. Mọi vé trong hóa đơn phải tham chiếu đúng suất này.
- Một vé ứng với một vị trí ghế. Ghế đôi bán nguyên cặp bằng một vé dành cho hai người.
- Ghế đã bán xác định theo **mã suất chiếu + mã ghế**, không lưu trong sơ đồ phòng.
- Loại vé thường/học sinh độc lập với loại ghế và thành viên. Ưu đãi không cộng dồn; lấy mức giảm cao nhất đủ điều kiện.
- Stack chỉ hoàn tác chọn ghế trước thanh toán. Hủy vé đã thanh toán là nghiệp vụ riêng của Admin, giữ lịch sử.
- Chưa hỗ trợ quản lý ca, hạng thẻ, đổi điểm hoặc thanh toán trực tuyến.

## 3. Kiến trúc

Luồng mục tiêu: **GUI → Business Logic → Data Access → DB**; kết quả trả về theo chiều ngược lại.

- `src/gui/`: Qt Widgets, nhận thao tác và hiển thị kết quả.
- `src/business/`: các class `*Service` kiểm tra quyền và quy tắc nghiệp vụ.
- `src/data_access/`: các class `*Repository` quản lý Model trong RAM, chuyển đổi và đọc/ghi TXT.
- `src/models/`: tám Model, không tự đọc/ghi file.
- `src/data_structures/`: cấu trúc tự cài đặt như `DoublyLinkedList<T>`.
- `src/utils/`: `FileIO`, `DateTime`, `IdGenerator`. `FileIO` chỉ đọc/ghi dòng TXT.
- `data/`: các file TXT; `tests/`: kiểm thử; `docs/`: tài liệu.

**Cây `src/` đã được tạo một phần.** Tám cặp Model ở `src/models/`, `DoublyLinkedList.h` ở `src/data_structures/`, `FileIO`, `IdGenerator`, `DateTime` ở `src/utils/` và bản nháp `MovieRepository.h/.cpp` ở `src/data_access/`. `src/main.cpp` vẫn là chương trình thử; chưa có GUI Qt, các Service/Repository còn lại hoặc CMake. Mã `TransactionManager` cũ được lưu ở `tests/legacy/` cho bài thử lịch sử; ứng dụng mục tiêu không dùng. Xem [tổng quan tiến độ](docs/development/TongQuanTienDo.md).

**Lộ trình 15 bước:** nhóm xác nhận Bước 01–03 đã hoàn thành. Bước 04–12 triển khai theo từng chức năng: Repository → Service → kiểm thử C++ → tài liệu; không chờ viết xong tất cả Repository mới làm Service. Bước 04 hiện là **quản lý phim** do Hoàng phụ trách; bản nháp `MovieRepository` chưa có `MovieService` hoặc kiểm thử chức năng. Kiệt có thể bắt đầu Bước 05 đăng nhập song song. Bước 13 xây dựng Qt, Bước 14 kiểm thử tích hợp, Bước 15 đóng gói và bàn giao. Xem [phân công 15 bước](docs/development/PhanCongCongViec.md).

## 4. Cấu trúc dữ liệu

| Cấu trúc | Vai trò |
| --- | --- |
| Mảng hai chiều | Sơ đồ ghế phòng chiếu |
| Danh sách liên kết đôi | Danh sách phim, suất chiếu, vé, hóa đơn và các tập thực thể cần thiết |
| Hash table | Tra cứu tài khoản theo `userId`, đăng nhập theo `username` và hóa đơn theo `invoiceId` |
| Stack | Hoàn tác trước thanh toán |
| Trie | Gợi ý tên phim theo tiền tố |
| BST | Chỉ mục hóa đơn theo khóa `(thời điểm lập, mã hóa đơn)` |
| Max heap | Top phim theo doanh thu hoặc số vé hợp lệ |

Chỉ bắt buộc tự cài `DoublyLinkedList<T>`. Các CTDL khác có thể dùng STL (`std::unordered_map`, `std::stack`, `std::queue`, `std::map`, `std::priority_queue`, `std::vector`) khi phù hợp. `HashTable.h` và `Stack.h` hiện không có trong cây làm việc. Các quan hệ Model hiện có và đề xuất bổ sung được ghi trong [tài liệu Model](docs/CauTrucThuMuc/models.md).

## 5. Tài liệu

- [Tổng quan đề tài và luồng chức năng](docs/TongQuan/TongQuanDeTai.md)
- [Cấu trúc thư mục mục tiêu](docs/CauTrucThuMuc/cauTrucThuMuc.md)
- [Quy tắc nghiệp vụ](docs/TongQuan/business_rules.md)
- [Định dạng dữ liệu TXT](docs/TongQuan/data_format.md)
- `docs/DS/`: tài liệu cấu trúc dữ liệu hiện có.
