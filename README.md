# PBL2 – Hệ thống quản lý bán vé rạp chiếu phim

## 1. Giới thiệu

Ứng dụng desktop C++/Qt mô phỏng bán vé offline tại quầy. Nhân viên thao tác thay khách; không có tài khoản khách hàng hoặc đặt vé online. Hai vai trò sử dụng là Admin và Staff. Dữ liệu lưu bằng file văn bản UTF-8.

Tài liệu mô tả thiết kế mục tiêu, không khẳng định chức năng đã triển khai. Dự án hiện có khung thư mục và một số file cấu trúc dữ liệu. Bản PBL2 chỉ cho phép một phiên ứng dụng thao tác trên một bộ dữ liệu tại một thời điểm.

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

Luồng chính: **UI → Services → Repositories → file dữ liệu**.

- `core/models/`: thực thể dữ liệu C++.
- `core/data_structures/`: cấu trúc tự cài đặt, header-only.
- `core/services/`: nghiệp vụ, phân quyền, điều phối giao dịch.
- `core/repositories/`: đọc/ghi và tra cứu dữ liệu.
- `core/utils/`: file, ngày giờ, kiểm tra đầu vào và phục hồi giao dịch.
- `core/config/`: giá vé, ưu đãi, tích điểm và khoảng nghỉ giữa suất.
- `ui/`: Qt Widgets; gọi service, không đọc/ghi file nghiệp vụ trực tiếp.
- `data/`: dữ liệu bền vững và vùng phục hồi giao dịch.

Core là C++ thuần, không phụ thuộc Qt. Dự kiến build bằng CMake; chưa có cấu hình và hướng dẫn build hoàn chỉnh ở giai đoạn hiện tại.

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

Chỉ cần cài đặt 2 đến 3 cấu trúc dữ liệu, còn lại có thể dùng thư viện. Quy ước dự án cho phép `std::string`, stream đọc/ghi file và tiện ích chuẩn; cần đối chiếu quy định giảng viên trước khi triển khai. Kiểu và container Qt chỉ dùng ở tầng UI.

## 5. Tài liệu

- [Tổng quan đề tài và luồng chức năng](docs/TongQuan/TongQuanDeTai.md)
- [Cấu trúc thư mục mục tiêu](docs/TongQuan/cauTrucThuMuc.md)
- [Quy tắc nghiệp vụ](docs/TongQuan/business_rules.md)
- [Định dạng dữ liệu và phục hồi](docs/TongQuan/data_format.md)
- `docs/DS/`: tài liệu cấu trúc dữ liệu hiện có.
