# Nghiệm thu GUI Qt Widgets — Giai đoạn 7

Ngày 21/11/2026, trên Windows. GUI đã chạy bằng Qt native với 11 trang nghiệp vụ và màn hình đăng nhập. Giá vé là cấu hình đã thử ghi qua GUI và đọc lại trong tiến trình mới. Sửa danh mục hiện có hiệu lực trong RAM. Mốc này chưa nghiệm thu giao dịch bán/hủy và báo cáo bằng dữ liệu thật.

[Bảng vấn đề backend](Backend_Gap_Tracker.md) · [Hồ sơ gửi Hoàng](Backend_Handoff_To_Hoang.md)

## Mốc mã nguồn và môi trường

| Hạng mục | Kết quả |
|---|---|
| Nhánh, HEAD local | `feature/b12-kiet-GUI` · `3c22da9e0bda164f4318d5d5a0909809362b1448`, ahead 5 so với upstream local lúc bắt đầu |
| Backend | `origin/feature/b10-hoang-booking-management` tại `b56bf562da1fac45eb8036543245d93f098c3eb8`, trùng parent thứ hai của HEAD |
| Working tree đầu kỳ | GĐ1–6 có file sửa/chưa track trong GUI, CMake, tests/GUI và tracker. Bản chạy gồm những file đó; HEAD riêng lẻ không tái tạo bản nghiệm thu. Không tạo commit mới |
| Wireframe | Đã kiểm tra `docs/ui/wireframe-v8.html`, đường dẫn này không tồn tại. Bản dùng để đối chiếu là [wireframe v8 thực tế](../CauTrucThuMuc/gui(docs)/pbl2_cinema_wireframe_v8.html) |
| Công cụ | C++17, CMake 3.30.5, Ninja, MinGW GCC 13.1.0, Qt Widgets/Test 6.11.2. CMake báo thiếu Vulkan headers tùy chọn; cấu hình, liên kết và chạy Qt Widgets thành công |
| Baseline | Configure thành công, Ninja báo không cần build lại, CTest 3/3 trước chỉnh sửa |
| Bản cuối | Build thư mục launcher và build Release từ cây build mới. Cả hai đạt 4/4 CTest. Chạy thêm cả bốn ca bằng `-platform windows` trên máy có Qt |

## Kết quả theo màn hình

| Màn hình | Ca đã kiểm tra / Service thật | Giới hạn khi nghiệm thu |
|---|---|---|
| Đăng nhập, shell | AuthService login/logout/hasPermission. Mở ở LoginView, sai mật khẩu, tài khoản khóa, Admin/Staff, logout rồi đổi tài khoản, role thiếu quyền và role không được cấp trang. Sidebar, header, trang đích và RBAC theo phiên | Bảng quyền lấy từ fixture test. Hủy vé có thêm điều kiện Admin của nghiệp vụ |
| Phim | MovieService đọc, tìm trong bảng, sửa, xóa bản ghi trống, từ chối xóa phim có suất, điều hướng theo movieId. Bật/tắt qua setMovieActive, từ chối tắt phim có suất tương lai | Thay đổi RAM. Thêm phim khóa |
| Phòng | RoomService đọc, sửa, từ chối xóa khi có liên kết, mở đúng ma trận theo roomId. Form khóa kích thước khi có suất; Service kiểm tra lại khi cập nhật | RAM. Thêm và bật/tắt khóa. Không dùng updateRoom để đổi active |
| Ghế | SeatService/RoomService. Kiểm tra hai ma trận, ba loại, ô trống, thêm/sửa/xóa có xác nhận, ID trùng ở hai phòng độc lập, từ chối ghế thứ hai cùng vị trí và Staff chỉ xem | RAM. View hạn chế sửa ở phòng có lịch. Ràng buộc vé lịch sử ở Service còn BG-008 |
| Suất chiếu | ShowtimeService đọc/sửa, bốn bộ lọc, reset, nhận phim từ MovieView. Test từ chối lịch quá khứ, suất CANCELLED, khoảng nghỉ 10 phút và chấp nhận đúng 15 phút | RAM. Form Thêm có thể xem; xác nhận Thêm, hủy, xóa còn khóa. Quy tắc suất từng có vé còn BG-013 |
| Nhân viên | StaffService đọc, tìm kiếm, sửa hồ sơ, vô hiệu hóa có xác nhận. Sau đổi username vẫn đăng nhập được với mật khẩu cũ. Danh sách và form sửa không hiển thị mật khẩu. Staff mặc định không thấy trang | RAM. Tạo Staff khóa. BG-016 theo dõi contract hồ sơ |
| Thành viên | MemberService đọc, tìm mã/tên/SĐT, sửa liên hệ, thử trùng SĐT, giữ nguyên mã và điểm. Staff có quyền mở màn hình và nút sửa | RAM. Tạo mới khóa. GUI chỉ đọc điểm |
| Giá vé & ưu đãi | PricingService cho xem 3 giá, 2 tỷ lệ và ghi từng trường. Admin cập nhật, Staff mặc định chỉ xem. Thử giá 0, giảm >100%, lỗi ghi, file hỏng, giá lớn và đọc lại sau restart | Ghi và đọc lại đạt trong miền chính xác 6 số lẻ của bản hiện tại. GUI khóa sửa khi dữ liệu nguồn vượt độ chính xác. Lỗi ghi dở/mất điện còn BG-014, BG-017 |
| Bán vé | Movie/Showtime/Room/Seat/Member/PricingService phục vụ chọn phim → suất → ghế → loại vé/thành viên → xem lại. Test căn giữa, đổi phim/suất, undo và bỏ hết, một vé COUPLE, tối đa 8 kết quả thành viên, ưu đãi cao nhất, điểm từng dòng và ba xác nhận tại quầy | DEMO cho trạng thái ghế và thao tác cuối. Không phát hành vé, hóa đơn, điểm. Không gọi createBooking |
| Lịch sử hóa đơn | AuthService kiểm tra OWN/ALL. Test mã, bốn trường lọc, biên ngày, chi tiết chỉ đọc và các trạng thái hóa đơn trong mẫu | Adapter thật trả NotImplemented. Test xác nhận Staff không nhận dữ liệu người khác khi nhập mã. Quy tắc hóa đơn thật theo ngày/người lập phải test lại ở Service |
| Hủy vé | TICKET_CANCEL và Admin. Test tìm hóa đơn/vé DEMO, lý do, vé SOLD trước giờ có dialog, vé CANCELLED và vé SOLD của suất đã qua không có nút hủy | Xác nhận khóa; bytes của vé, hóa đơn và thành viên trong thư mục tạm giữ nguyên |
| Báo cáo | REPORT_VIEW. Test kỳ, ngày cuối loại trừ 00:00 hôm sau, kỳ sai/rỗng, năm chỉ số, ngày thuần âm và tên phim dài. Tắt DEMO dọn bảng và thẻ | Ở chế độ thật hiện chờ Service. Các số DEMO là bộ minh họa, chưa đối soát nguồn giao dịch thật |

## Kiểm thử lưu và kiến trúc

- `gui_booking` cập nhật giá qua nút Lưu trên bản sao fixture, kiểm tra giá trong RAM và các dòng pricing.txt. Khi thay đường dẫn file bằng thư mục trong sandbox test, thao tác ghi bị từ chối, giá trong RAM/form giữ giá trước. Gọi Service với Staff thiếu quyền cũng bị từ chối.
- `gui_restart` sao chép dữ liệu thử vào thư mục có dấu và khoảng trắng. Một tiến trình mở MainWindow, đổi giá STANDARD thành 123.456.789.012,5 ₫ và giảm học sinh thành 17,125%, đóng cửa sổ rồi thoát. Tiến trình mới đăng nhập, đọc đúng hai giá trị; file còn đủ năm dòng và không bị viết lại khi chỉ xem. Các file danh mục còn lại được so sánh với fixture. Hai lần chạy riêng kiểm tra file trùng khóa/thiếu dòng và giá 0.0000001. Cả hai trường hợp giữ nguyên file đầu vào.
- `gui_navigation` và `gui_history` kiểm tra điều hướng, dialog, trạng thái rỗng, quyền và dữ liệu mẫu. Các test Qt thao tác trên QTemporaryDir. Launcher thử tay trỏ thẳng đến `tests/GUI/data/`; nếu tự bấm Lưu bảng giá trong launcher này, giá thử sẽ thay đổi ở đó.
- Rà mã trong `src/gui/`: ApplicationContext khởi tạo repository và nạp tài khoản/quyền khi khởi động. Các View thực hiện CRUD/truy vấn qua Service. QFile trong MainWindow đọc tài nguyên QSS. Không thấy View mở TXT nghiệp vụ, truy cập TicketRepository/InvoiceRepository, dùng sequences.txt, gọi markCancelled hoặc tự cộng/trừ điểm.
- Ma trận giữ giá trị hiển thị và khóa phòng/ghế. SeatMapView tạo nhãn vị trí như A04 theo tọa độ, không cấp ID tuần tự. Khi thêm Seat, unique_ptr chỉ release sau khi add thành công. Năm hàm add danh mục trên B10 tạo bản sao và giữ caller sở hữu input. Không thấy View giữ con trỏ Model qua lần đổi phiên/reload; không thực hiện phân tích heap bằng sanitizer.
- Các handler CRUD dùng kết quả bool/nullptr và khối xử lý ngoại lệ ở CatalogUi hoặc ở View. Service kiểm tra quyền độc lập; đóng phiên hủy cây widget của phiên cũ. Thử nghiệm xác nhận không có quyền bán/không có trang trước đăng nhập và Staff không kế thừa thao tác của Admin. Các điều kiện thật của BookingService và quyền trên save danh mục được theo dõi riêng.
- Tính bền vững của danh mục, ghi nhiều file/khôi phục, chặn bán trùng giữa tiến trình, hoàn và thu hồi điểm thật, quyền xem hóa đơn thật và số liệu doanh thu thật **chưa nghiệm thu**. Xem điều kiện và test cần bổ sung theo từng BG.

## Đối chiếu giao diện và Windows

Giữ Sidebar navy, nhấn xanh, header tài khoản/đăng xuất, Segoe UI, panel bo góc, trạng thái hover/selected/disabled và dialog native. Đã điều hướng qua 11 trang ở 960×580, 1280×820, 1366×768 (đơn vị logic Qt); có ảnh native ở kích thước lớn hơn. Nội dung có thể cuộn khi không đủ chỗ. Bảng rộng dùng thanh cuộn, ô văn bản có tooltip, tên phim dài trong biểu đồ được rút gọn và có giá trị đầy đủ khi trỏ chuột.

Các khác biệt chủ ý so với HTML v8 gồm ma trận ô Qt native, trang bán vé có nút bỏ chọn tất cả, báo cáo dùng 5 thẻ riêng và thanh ngang hỗ trợ giá trị âm. Nhãn DEMO và nút khóa phản ánh khả năng thật của từng Service. Ảnh mẫu có dữ liệu sinh trong thư mục thử, không đại diện doanh thu thực tế.

Đã chạy `tests/GUI/Chay-Thu.cmd` từ dự án, thấy LoginView. Đã sao chép launcher, executable và fixture vào đường dẫn Windows có khoảng trắng và tiếng Việt rồi đăng nhập Admin thành công. Quan sát tiến trình nạp Qt6Core.dll, Qt6Gui.dll, Qt6Widgets.dll, qwindows.dll và runtime MinGW (libgcc_s_seh-1.dll, libstdc++-6.dll, libwinpthread-1.dll). Đây là môi trường phát triển có Qt và plugin; chưa có kết quả kiểm thử gói độc lập trên máy sạch.

### Chạy lại

Sau khi build, nhấp đúp [Chay-Thu.cmd](../../tests/GUI/Chay-Thu.cmd). Tài khoản thử nằm trong `tests/GUI/data/users.txt`. Một số suất trong fixture tay có ngày cố định; test bán vé tự tạo suất tương lai trong bản sao tạm để tiếp tục chạy ở các ngày khác.

Trong PowerShell tại gốc repository, dùng đường dẫn cài đặt thực tế trên máy nghiệm thu.

```powershell
$env:Path = "C:\Qt\6.11.2\mingw_64\bin;C:\Qt\Tools\mingw1310_64\bin;C:\Qt\Tools\Ninja;C:\Qt\Tools\CMake_64\bin;$env:Path"
cmake -S . -B build/qt-phase1 -G Ninja -DCMAKE_PREFIX_PATH=C:/Qt/6.11.2/mingw_64 -DCMAKE_CXX_COMPILER=C:/Qt/Tools/mingw1310_64/bin/g++.exe -DBUILD_TESTING=ON
cmake --build build/qt-phase1 -j 6
ctest --test-dir build/qt-phase1 --output-on-failure
.\tests\GUI\Chay-Thu.cmd
```

Để tái tạo ảnh, đặt `PBL2_HISTORY_SCREENSHOTS` và `PBL2_CATALOG_SCREENSHOTS` thành đường dẫn tuyệt đối tới thư mục đã tạo, chạy lần lượt `test_gui_navigation.exe -platform windows`, cùng file đó với `--history`, rồi `test_gui_booking.exe -platform windows`. Ca restart dùng `test_gui_booking.exe -platform windows --restart`. Các executable nằm trong `build/qt-phase1/`. Ảnh nghiệm thu trên máy nằm trong `build/qt-phase1/nghiem-thu-7/`.

## File thuộc đợt nghiệm thu

- Cấu hình và khởi động: `CMakeLists.txt`, `src/gui/main.cpp`. Test điều hướng dùng fixture quyền; thêm gui_restart. Qt chọn thư mục dữ liệu Unicode trước khi backend mở tên file tương đối.
- GUI: `src/gui/MainWindow.cpp`, `MovieManagementView.cpp`, `RoomManagementView.cpp`, `CatalogView.cpp`, `PricingManagementView.cpp` (cùng thư mục). Sửa công cụ trạng thái phim, thông báo form Thêm, tooltip, cuộn trang, header ở cửa sổ nhỏ và bảo vệ cấu hình giá khi lưu.
- Test: `tests/GUI/test_booking.cpp`, `test_catalog.cpp`, `test_navigation.cpp`, `test_history.cpp`. Bổ sung ca restart/hỏng file, trạng thái phim, từ chối ghế trùng, ma trận hiển thị sau đổi phòng, quyền ghi, kích thước thật của cửa sổ, suất đã bắt đầu và ảnh LoginView.
- Tài liệu: `docs/development/Backend_Gap_Tracker.md`, `Backend_Handoff_To_Hoang.md`, `GUI_Acceptance_Stage7.md` (cùng thư mục). Các sửa đổi backend và dữ liệu vận hành không thuộc đợt này.
