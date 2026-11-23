# PBL2 – Hệ thống quản lý bán vé rạp chiếu phim

## 1. Giới thiệu

PBL2 là đồ án xây dựng **hệ thống quản lý bán vé rạp chiếu phim**, được phát triển bằng **C++17 và Qt Widgets**.

Ứng dụng hoạt động trên máy tính, mô phỏng quy trình quản lý và bán vé trực tiếp tại quầy của một rạp chiếu phim.

Hệ thống có hai vai trò người dùng:

- **Admin:** Quản lý các hoạt động của rạp chiếu phim.
- **Staff:** Thực hiện các nghiệp vụ bán vé và phục vụ khách hàng.

Khách hàng không cần tạo tài khoản đăng nhập. Nhân viên sẽ trực tiếp thao tác trên hệ thống thay cho khách hàng.

Dữ liệu được lưu trữ bằng các file văn bản `.txt` với mã hóa UTF-8, không sử dụng hệ quản trị cơ sở dữ liệu.

Ứng dụng được thiết kế để một phiên làm việc thao tác trên một bộ dữ liệu tại một thời điểm.

## 2. Chức năng chính

### 2.1. Đăng nhập và phân quyền

- Đăng nhập bằng tài khoản nhân viên hoặc quản trị viên.
- Xác thực thông tin tài khoản từ dữ liệu hệ thống.
- Phân quyền dựa trên các mã Permission.
- Mỗi chức năng chỉ được thực hiện khi tài khoản có Permission tương ứng.
- Việc kiểm tra quyền được thực hiện tại tầng Business Service, không chỉ tại giao diện.

### 2.2. Quản lý phim

- Thêm phim mới.
- Cập nhật thông tin phim.
- Tìm kiếm phim theo tên.
- Xem danh sách phim đang chiếu và sắp chiếu.
- Kích hoạt hoặc ngừng hoạt động phim.
- Xóa phim khi đáp ứng các điều kiện nghiệp vụ.

### 2.3. Quản lý phòng chiếu và ghế

- Thêm, sửa, xóa và tra cứu phòng chiếu.
- Quản lý sơ đồ ghế của từng phòng.
- Hỗ trợ ba loại ghế: STANDARD, VIP và COUPLE.
- Kiểm tra vị trí ghế hợp lệ và tránh trùng ghế.
- Bảo đảm các ràng buộc dữ liệu khi thay đổi phòng hoặc ghế.

### 2.4. Quản lý suất chiếu

- Tạo suất chiếu cho phim và phòng chiếu.
- Cập nhật thông tin suất chiếu.
- Xem và tìm kiếm lịch chiếu.
- Kiểm tra thời gian chiếu và lịch sử dụng phòng.
- Hủy hoặc xóa suất chiếu khi đáp ứng điều kiện nghiệp vụ.

### 2.5. Quản lý nhân viên

- Thêm tài khoản nhân viên.
- Cập nhật thông tin nhân viên.
- Tìm kiếm và tra cứu nhân viên.
- Kích hoạt hoặc vô hiệu hóa tài khoản.
- Quản lý quyền truy cập theo Permission.

### 2.6. Quản lý thành viên

- Đăng ký thành viên.
- Cập nhật thông tin liên hệ.
- Tìm kiếm và tra cứu thành viên.
- Quản lý điểm tích lũy.
- Sử dụng thông tin thành viên khi bán vé và áp dụng ưu đãi.

### 2.7. Bán vé

Quy trình bán vé tại quầy:

1. Chọn phim và suất chiếu.
2. Hiển thị danh sách ghế của phòng chiếu.
3. Kiểm tra trạng thái ghế theo suất chiếu.
4. Chọn ghế và loại vé.
5. Gắn thông tin thành viên nếu có.
6. Tính giá vé và áp dụng ưu đãi phù hợp.
7. Kiểm tra thông tin giao dịch và xác nhận thanh toán.
8. Tạo vé, hóa đơn và cập nhật điểm thành viên nếu có.
9. Lưu dữ liệu và thông báo kết quả giao dịch.

Hệ thống phải bảo đảm không bán trùng một ghế trong cùng suất chiếu.

### 2.8. Quản lý vé và hóa đơn

- Tra cứu thông tin vé và hóa đơn.
- Xem lịch sử giao dịch.
- Quản lý trạng thái vé.
- Hủy vé theo Permission và các điều kiện nghiệp vụ.
- Xử lý hoàn tiền và điều chỉnh điểm tích lũy khi hủy vé.
- Giữ lại lịch sử giao dịch để phục vụ tra cứu và thống kê.

### 2.9. Thống kê và báo cáo

- Thống kê số lượng vé bán và vé hủy.
- Thống kê tổng tiền bán vé.
- Thống kê tiền hoàn và doanh thu thuần.
- Thống kê theo khoảng thời gian.
- Tổng hợp kết quả bán vé theo phim, suất chiếu hoặc nhân viên khi được hỗ trợ.

## 3. Quy tắc nghiệp vụ cơ bản

Một số quy tắc quan trọng của hệ thống:

- Một hóa đơn có thể chứa nhiều vé nhưng các vé phải thuộc cùng một suất chiếu.
- Mỗi vé tương ứng với một ghế trong một suất chiếu.
- Ghế đôi được quản lý như một vị trí ghế logic và bán bằng một vé dành cho hai người.
- Trạng thái ghế đã bán phụ thuộc vào suất chiếu, không lưu cố định trong sơ đồ phòng.
- Loại vé và loại ghế là hai thông tin độc lập.
- Các ưu đãi không cộng dồn; áp dụng mức giảm cao nhất đủ điều kiện.
- Không được bán vé cho suất chiếu đã bắt đầu.
- Hủy vé đã bán không đồng nghĩa với xóa bản ghi vé khỏi hệ thống.
- Các thao tác phải kiểm tra Permission và điều kiện nghiệp vụ tương ứng.

Quy tắc đầy đủ được mô tả trong `docs/overview/business_rules.md`.

## 4. Kiến trúc hệ thống

Dự án được tổ chức theo các tầng nhằm phân chia rõ trách nhiệm và giúp mã nguồn dễ bảo trì.

### 4.1. Các thành phần chính

**Models (`src/models/`)**

Chứa các lớp biểu diễn thực thể như User, Movie, Room, Seat, Showtime, Member, Ticket và Invoice.

**Data Structures (`src/data_structures/`)**

Chứa các cấu trúc dữ liệu tự cài đặt phục vụ quản lý dữ liệu.

**Data Access (`src/data_access/`)**

Chịu trách nhiệm quản lý, truy xuất, thêm, sửa, xóa, đọc và ghi dữ liệu.

**Business (`src/business/`)**

Chứa các Service xử lý nghiệp vụ, kiểm tra Permission và điều phối các thao tác với Repository.

**Utils (`src/utils/`)**

Chứa các tiện ích dùng chung như kiểm tra dữ liệu, xử lý ngày giờ, sinh ID và thao tác file.

**GUI (`src/gui/`)**

Giao diện được xây dựng bằng Qt Widgets, tiếp nhận thao tác người dùng và gọi các Business Service.

**Tests (`tests/`)**

Chứa các chương trình kiểm thử chức năng, nghiệp vụ và các thành phần của hệ thống.

### 4.2. Nguyên tắc thiết kế

Dự án áp dụng ba nguyên tắc chính:

**Dependency Inversion Principle (DIP)**

- Sử dụng một interface chung `IRepository<T>`.
- Các Concrete Repository triển khai interface này.
- Service phụ thuộc vào `IRepository<T>` thay vì phụ thuộc trực tiếp vào Concrete Repository.
- Lớp `Repository<T>` tiếp tục hỗ trợ quản lý và lưu trữ dữ liệu.

**Phân quyền bằng Permission (RBAC)**

- Hệ thống quản lý quyền truy cập bằng các mã Permission.
- Service sử dụng `AuthService` để kiểm tra Permission.
- Không sử dụng vai trò Admin/Staff được viết cứng trong các Service để quyết định quyền thực hiện nghiệp vụ.

**Phân chia trách nhiệm Service và Repository**

- Service kiểm tra Permission, xử lý điều kiện nghiệp vụ và điều phối.
- Repository thực hiện thao tác dữ liệu và bảo đảm tính toàn vẹn dữ liệu.
- GUI không trực tiếp xử lý nghiệp vụ hoặc đọc/ghi file dữ liệu.

Các nguyên tắc này là kiến trúc mục tiêu của đợt refactor và được triển khai theo từng module.

## 5. Cấu trúc dữ liệu

Dự án sử dụng ba cấu trúc dữ liệu chính:

| Cấu trúc dữ liệu | Nguồn | Mục đích |
|---|---|---|
| `DoublyLinkedList<T>` | Nhóm tự cài đặt | Quản lý danh sách đối tượng, đặc biệt trong `Repository<T>` |
| `std::vector<T>` | Thư viện chuẩn C++ | Lưu trữ và xử lý danh sách dữ liệu |
| `std::unordered_map<K, V>` | Thư viện chuẩn C++ | Lưu trữ dữ liệu theo cặp khóa–giá trị và hỗ trợ tra cứu nhanh |

Trong đó:

- **Doubly Linked List:** Danh sách liên kết đôi, mỗi nút có liên kết đến nút trước và nút sau. Đây là cấu trúc dữ liệu do nhóm tự xây dựng.
- **Vector:** Mảng động, có thể thay đổi kích thước và truy cập phần tử theo chỉ số.
- **Unordered Map:** Cấu trúc bảng băm, giúp tìm kiếm dữ liệu theo khóa hiệu quả trong trường hợp thông thường.

Việc lựa chọn cấu trúc dữ liệu phụ thuộc vào mục đích sử dụng của từng thành phần.

## 6. Lưu trữ dữ liệu

Hệ thống sử dụng các file `.txt` để lưu trữ dữ liệu.

Mỗi dòng biểu diễn một bản ghi, các trường được phân tách bằng ký tự `|`.

Ví dụ:

```text
MOV0001|Ten phim|Hanh dong|120|13|2026-01-01|2026-12-31|1
```

Các thao tác đọc, ghi và chuyển đổi dữ liệu được thực hiện tại tầng Data Access và các tiện ích liên quan.

Hệ thống phải bảo đảm:

- Kiểm tra dữ liệu trước khi thêm hoặc cập nhật.
- Không tạo ID trùng.
- Duy trì tính nhất quán giữa các đối tượng có liên kết.
- Xử lý lỗi khi đọc hoặc ghi file.
- Không làm mất dữ liệu hoặc tạo liên kết không hợp lệ khi thao tác thất bại.

Chi tiết định dạng được mô tả trong `docs/overview/data_format.md`.

## 7. Quy trình phát triển

Nhóm áp dụng quy trình **Markdown-first** để thống nhất giữa thiết kế, mã nguồn và tài liệu.

1. Đọc yêu cầu, code và tài liệu liên quan.
2. Thiết kế API, thuộc tính, constructor và các hàm.
3. Viết đầy đủ mã nguồn dự kiến trong file Markdown.
4. Viết tổng quan, luồng hoạt động và giải thích chi tiết từng đoạn code.
5. Review, chỉnh sửa và chốt Markdown.
6. Triển khai `.h` và `.cpp` đúng 100% theo mã nguồn đã chốt trong Markdown.
7. Biên dịch, kiểm thử và xử lý lỗi.
8. Đối chiếu code với Markdown, cập nhật kết quả và hoàn thành công việc.

Nếu phát hiện lỗi trong quá trình triển khai, nhóm cập nhật và chốt lại Markdown trước khi sửa code tương ứng.

Quy chuẩn chi tiết được mô tả trong `AGENTS.md`.

## 8. Công nghệ sử dụng

- **Ngôn ngữ:** C++17.
- **Giao diện:** Qt Widgets.
- **Lưu trữ:** File TXT UTF-8.
- **Cấu trúc dữ liệu:** Doubly Linked List tự cài đặt, `std::vector`, `std::unordered_map`.
- **Quản lý mã nguồn:** Git và GitHub.
- **Kiểm thử:** Các chương trình test C++.

## 9. Tài liệu dự án

### Tổng quan

- [Tổng quan đề tài](docs/overview/TongQuanDeTai.md)
- [Quy tắc nghiệp vụ](docs/overview/business_rules.md)
- [Định dạng dữ liệu](docs/overview/data_format.md)

### Kiến trúc

- [Cấu trúc thư mục](docs/architecture/cauTrucThuMuc.md)
- [Dependency Inversion Principle](docs/architecture/dependency_inversion.md)
- [Thiết kế phân quyền](docs/architecture/PermissionDesign.md)
- [Business – UI Contract](docs/architecture/Business_UI_Contract.md)

### Phát triển

- [Tổng quan tiến độ](docs/development/TongQuanTienDo.md)
- [Phân công công việc](docs/development/PhanCongCongViec.md)
- [Công việc của Hoàng](docs/development/tasks/Task_Hoang.md)
- [Công việc của Kiệt](docs/development/tasks/Task_Kiet.md)

### Quy chuẩn chung

- [AGENTS.md](AGENTS.md) – Quy định kiến trúc, viết code, viết tài liệu, kiểm thử và cộng tác.

## 10. Thành viên thực hiện

Dự án được thực hiện bởi hai thành viên:

- **Hoàng:** Phát triển Backend và xử lý nghiệp vụ.
- **Kiệt:** Phát triển giao diện Qt và phối hợp tích hợp hệ thống.

Cả hai thành viên tuân theo cùng một quy chuẩn thiết kế, phát triển, kiểm thử và quản lý mã nguồn.