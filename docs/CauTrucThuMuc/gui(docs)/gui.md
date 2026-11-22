# `src/gui/` — Giao diện Qt Widgets (PBL2)

> **Phụ trách:** Kiệt (`task_kiet`)  
> **Phạm vi:** Bước 12 — phát triển GUI và tích hợp Backend.  
> **Trạng thái tài liệu:** Kế hoạch tổng quan; các màn hình và API phải được đối chiếu với mã nguồn thực tế trước khi triển khai.  
> **Công nghệ dự kiến:** C++17, Qt Widgets, CMake (phiên bản Qt và cấu hình build sẽ xác nhận khi thiết lập môi trường).

## 1. Mục tiêu

Xây dựng ứng dụng quản lý rạp chiếu phim có giao diện trực quan cho **Admin** và **Staff**, sử dụng các chức năng do tầng `src/business/` cung cấp. GUI chịu trách nhiệm nhận thao tác, kiểm tra dữ liệu nhập ở mức giao diện, gọi Service và hiển thị dữ liệu hoặc thông báo lỗi.

**Nguyên tắc kiến trúc:** `GUI → Service → Repository → dữ liệu TXT`. GUI **không** trực tiếp đọc/ghi TXT, không gọi Repository, không tự quyết định phân quyền, giá vé, ưu đãi hay điều kiện bán/hủy vé. Việc ẩn nút theo vai trò chỉ hỗ trợ trải nghiệm; Service vẫn phải kiểm tra quyền.

## 2. Phạm vi và trách nhiệm

| Kiệt (GUI) | Hoàng (Backend) |
| --- | --- |
| Thiết kế màn hình, bố cục, điều hướng, trạng thái giao diện | Cung cấp và kiểm thử API Service |
| Lấy dữ liệu từ form, gọi Service, hiển thị kết quả/lỗi | Kiểm tra quy tắc nghiệp vụ và phân quyền |
| Xử lý thông báo, xác nhận thao tác, làm mới bảng | Truy xuất/lưu trữ dữ liệu qua Repository |
| Tích hợp và kiểm thử luồng sử dụng trên Qt | Thống nhất hợp đồng API, dữ liệu trả về và lỗi |

**Ngoài phạm vi GUI:** tự sửa định dạng TXT, viết lại thuật toán tính giá, thao tác trực tiếp với Repository, thay đổi quy tắc nghiệp vụ khi chưa thống nhất với người phụ trách Backend.

## 3. Các màn hình dự kiến

| Màn hình | Trách nhiệm | Phụ thuộc |
| --- | --- | --- |
| `MainWindow` | Cửa sổ chính, điều hướng, trạng thái đăng nhập | Khung Qt, phiên đăng nhập |
| `LoginView` | Nhập tài khoản/mật khẩu, đăng nhập, báo lỗi | `AuthService` |
| `AdminDashboard` | Điều hướng các chức năng Admin | Quyền Admin từ Backend |
| `StaffDashboard` | Điều hướng các chức năng Staff | Quyền Staff từ Backend |
| `MovieManagementView` | Danh sách, form quản lý phim | `MovieService` |
| `RoomManagementView` | Danh sách, form quản lý phòng và thông tin ghế | `RoomService`, `SeatService` nếu có |
| `ShowtimeManagementView` | Danh sách/lịch và quản lý suất chiếu | `ShowtimeService` |
| `StaffManagementView` | Quản lý tài khoản nhân viên | `StaffService` |
| `MemberManagementView` | Tra cứu, quản lý thông tin thành viên | `MemberService` (cần xác nhận) |
| `BookingView` | Chọn suất, ghế, loại vé và xác nhận bán | `BookingService` (cần xác nhận) |
| `SeatMapView` | Sơ đồ và trạng thái ghế **theo suất chiếu** | API ghế/đặt vé (cần xác nhận) |
| `InvoiceHistoryView` | Tra cứu hóa đơn/vé, gửi yêu cầu hủy | API hóa đơn/vé (cần xác nhận) |
| `ReportView` | Hiển thị doanh thu, thống kê | `ReportService` (cần xác nhận) |

Danh sách là **mục tiêu thiết kế**, không khẳng định các file hoặc API đã tồn tại. Không tạo file rỗng chỉ để khớp sơ đồ. Có thể dùng Qt Designer và `.ui` khi thực sự cần.

## 4. Tổ chức thư mục đề xuất

```text
src/
├── main.cpp
├── business/                  # Backend: Hoàng
├── data_access/               # Backend: Hoàng
└── gui/                       # GUI: Kiệt
    ├── MainWindow.h/.cpp
    ├── LoginView.h/.cpp
    ├── AdminDashboard.h/.cpp
    ├── StaffDashboard.h/.cpp
    ├── MovieManagementView.h/.cpp
    ├── RoomManagementView.h/.cpp
    ├── ShowtimeManagementView.h/.cpp
    ├── StaffManagementView.h/.cpp
    ├── MemberManagementView.h/.cpp
    ├── BookingView.h/.cpp
    ├── SeatMapView.h/.cpp
    ├── InvoiceHistoryView.h/.cpp
    └── ReportView.h/.cpp
```

Tổ chức thực tế có thể tách thêm thành `views/`, `widgets/` hoặc `dialogs/` khi số lượng file tăng, nhưng cần thống nhất với cấu trúc hiện hành trước khi đổi.

## 5. Quy tắc thiết kế GUI

1. **Một màn hình, một trách nhiệm chính:** màn hình quản lý tập trung vào danh sách, nhập liệu và thao tác của chức năng đó.
2. **Không chứa nghiệp vụ:** dữ liệu nhập có thể được kiểm tra rỗng/định dạng để phản hồi nhanh, nhưng Service là nơi quyết định thao tác hợp lệ.
3. **Xử lý lỗi đầy đủ:** kiểm tra kết quả trả về và bắt `std::exception`/`std::runtime_error` tại ranh giới gọi Service phù hợp; không để ngoại lệ làm đóng ứng dụng.
4. **Không giả định mọi API đều trả `bool`:** kiểm tra chữ ký thật của Service trước khi viết slot xử lý.
5. **Trạng thái rõ ràng:** phân biệt chưa chọn mục, đang nhập, thành công, thất bại; làm mới danh sách sau thao tác thành công.
6. **Xác nhận thao tác nhạy cảm:** xóa, hủy, thanh toán cần hộp thoại xác nhận phù hợp.
7. **Phân quyền hai lớp:** GUI chỉ hiển thị chức năng phù hợp; Backend vẫn xác thực quyền.
8. **Sơ đồ ghế theo suất chiếu:** không suy ra ghế còn trống chỉ từ cấu hình phòng.
9. **Không ghi trực tiếp vào dữ liệu:** mọi thao tác lưu phải đi qua Service.
10. **Thống nhất giao diện:** tên nút, thông báo, màu trạng thái và cách trình bày bảng dùng chung một quy ước.

## 6. Luồng hoạt động tổng quát

```text
Khởi động ứng dụng
    ↓
Khởi tạo Backend theo quy trình đã thống nhất
    ↓
LoginView → AuthService
    ↓
Đăng nhập thành công?
    ├── Không → Hiển thị lỗi, giữ LoginView
    └── Có → Điều hướng theo vai trò
                 ├── AdminDashboard
                 └── StaffDashboard
                          ↓
                  Chọn chức năng
                          ↓
                 View gọi Service
                          ↓
               Hiển thị kết quả/lỗi
                          ↓
                  Đăng xuất → LoginView
```

**Luồng mẫu quản lý phim:** nhập/tìm kiếm → gọi `MovieService` → nhận kết quả hoặc lỗi → cập nhật bảng/thông báo.

**Luồng mẫu bán vé:** chọn suất → lấy trạng thái ghế theo suất → chọn ghế → gọi API nghiệp vụ để xác nhận bán → cập nhật trạng thái; không tự tính tiền hoặc tự đánh dấu ghế đã bán trước khi Backend xác nhận.

## 7. Kế hoạch thực hiện `task_kiet`

### K1 — Thiết lập môi trường và khung GUI

- [ ] Xác minh phiên bản Qt, bộ biên dịch C++17 và CMake trên máy phát triển.
- [ ] Kiểm tra cấu trúc `src/`, điểm vào `main.cpp`, cấu hình build hiện tại.
- [ ] Cấu hình build Qt Widgets cùng Backend mà không phá các bài test hiện có.
- [ ] Tạo và chạy `MainWindow` tối thiểu.
- [ ] Thiết lập cơ chế chuyển màn hình.

**Hoàn thành khi:** build và chạy được cửa sổ Qt từ cấu hình dự án.

### K2 — Đăng nhập, đăng xuất và Dashboard

- [ ] Xây dựng `LoginView`.
- [ ] Đối chiếu và tích hợp `AuthService` thực tế.
- [ ] Hiển thị lỗi đăng nhập và lỗi khởi tạo dữ liệu.
- [ ] Tạo `AdminDashboard`, `StaffDashboard`.
- [ ] Chuyển màn hình đúng vai trò và đăng xuất về màn hình đăng nhập.

**Hoàn thành khi:** thử được luồng đăng nhập thành công/thất bại và phân vai trên giao diện.

### K3 — Các màn hình quản lý

- [ ] `MovieManagementView`.
- [ ] `RoomManagementView` và phần quản lý ghế liên quan.
- [ ] `ShowtimeManagementView`.
- [ ] `StaffManagementView`.
- [ ] `MemberManagementView` khi API sẵn sàng.
- [ ] Kiểm thử danh sách, thêm/sửa/xóa hoặc đổi trạng thái **theo đúng chức năng Service hỗ trợ**.

**Hoàn thành khi:** các thao tác đã được Backend hỗ trợ có thể thực hiện và nhận phản hồi qua GUI.

### K4 — Bán vé, hóa đơn

- [ ] Xác nhận hợp đồng API đặt vé, trạng thái ghế, hóa đơn và hủy vé.
- [ ] Tạo `BookingView` và `SeatMapView`.
- [ ] Tạo `InvoiceHistoryView`.
- [ ] Kiểm thử ghế trống/đã chọn/không khả dụng và lỗi thao tác.
- [ ] Đảm bảo chỉ cập nhật trạng thái bán vé sau khi Service xác nhận thành công.

**Hoàn thành khi:** luồng bán vé/hóa đơn được tích hợp theo API thực tế, không xử lý nghiệp vụ trong GUI.

### K5 — Thống kê, kiểm thử, bàn giao

- [ ] Tạo `ReportView` khi `ReportService` sẵn sàng.
- [ ] Kiểm thử luồng Admin và Staff, gồm cả trường hợp không đủ quyền.
- [ ] Kiểm thử form rỗng, dữ liệu sai, dữ liệu không tồn tại, lỗi Service.
- [ ] Kiểm thử điều hướng, đăng xuất/đăng nhập lại và cập nhật danh sách.
- [ ] Viết hướng dẫn build/chạy và ghi nhận kết quả kiểm thử thực tế.

**Hoàn thành khi:** ứng dụng chạy ổn định trong các luồng đã triển khai và có hướng dẫn tái hiện.

## 8. Quy trình làm việc và phối hợp

- Trước mỗi màn hình, đọc API Service tương ứng: tham số, kiểu trả về, điều kiện gọi, ngoại lệ và quyền.
- Nếu API chưa có hoặc chưa thống nhất, ghi rõ **đang chờ Backend**, không tự tạo logic thay thế trong GUI.
- Thực hiện theo vòng lặp nhỏ: dựng bố cục → nối signal/slot → gọi Service → kiểm thử → commit.
- Ưu tiên commit theo chức năng nhỏ, ví dụ `gui: add login view`, `gui: integrate movie management`.
- Trước khi merge, kiểm tra build, kiểm thử luồng liên quan và tránh sửa mã Backend ngoài phạm vi được thống nhất.

## 9. Rủi ro và điểm cần xác nhận

| Vấn đề | Cách xử lý |
| --- | --- |
| Qt/CMake chưa cấu hình đồng nhất | Kiểm tra môi trường và build mẫu trước khi viết nhiều màn hình |
| API Service có thể thay đổi | Đọc header/implementation hiện tại, thống nhất hợp đồng với Hoàng |
| Service có thể ném ngoại lệ | Bắt và hiển thị thông báo phù hợp tại điểm gọi |
| Một số Service nghiệp vụ chưa hoàn tất | Ưu tiên màn hình có API ổn định; đánh dấu phụ thuộc |
| Trạng thái ghế thay đổi theo suất | Luôn lấy dữ liệu trạng thái từ Backend theo suất chiếu |
| Khác biệt quyền Admin/Staff | Kiểm thử cả giao diện và kết quả từ Service |

## 10. Tiêu chí hoàn thành chung

- [ ] Build được GUI với Backend trong môi trường đã thống nhất.
- [ ] Có luồng đăng nhập, điều hướng theo vai trò, đăng xuất.
- [ ] Các màn hình đã triển khai gọi Service thay vì Repository/TXT.
- [ ] Thông báo lỗi không làm ứng dụng thoát bất thường.
- [ ] Không có nghiệp vụ tính giá, quyền, bán/hủy vé trong GUI.
- [ ] Có bằng chứng kiểm thử và hướng dẫn chạy.

## 11. Nhật ký tiến độ

| Ngày | Công việc | Kết quả kiểm chứng | Vướng mắc / tiếp theo |
| --- | --- | --- | --- |
| Chưa bắt đầu | Lập kế hoạch GUI tổng quan | Chưa build hoặc kiểm thử | Bắt đầu K1, đối chiếu mã nguồn và môi trường |

---

**Tài liệu liên quan:** `business.md`, `src.md`, `cauTrucThuMuc.md` và tài liệu phân công/tiến độ trong `docs/development/`.

Tài liệu này là kế hoạch GUI chung; trạng thái triển khai chỉ được đánh dấu hoàn thành sau khi có mã và kết quả kiểm thử thực tế.