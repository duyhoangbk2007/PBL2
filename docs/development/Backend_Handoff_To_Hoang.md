# Bàn giao từ Kiệt sang Hoàng — tích hợp GUI PBL2

Ngày 21/11/2026. Hoàng phụ trách hợp đồng và bảo đảm nghiệp vụ trong Service. Kiệt phụ trách nối Qt với những hợp đồng đã thống nhất. Bản GUI hiện có thể dùng để duyệt luồng vận hành, quản lý danh mục trong RAM và lưu cấu hình giá. Cần xử lý các mục P0 trước khi nghiệm thu bán vé, hủy vé và tra cứu hóa đơn thật.

| Mốc đối chiếu | Giá trị |
|---|---|
| Nhánh GUI và HEAD | `feature/b12-kiet-GUI` · `3c22da9e0bda164f4318d5d5a0909809362b1448` |
| Bản GUI đã chạy | HEAD trên cùng working tree có các chỉnh sửa Giai đoạn 1–7 chưa commit. Chỉ checkout HEAD sẽ thiếu một phần GUI/test |
| Backend tham chiếu | `feature/b10-hoang-booking-management` · `b56bf562da1fac45eb8036543245d93f098c3eb8` đã có trong HEAD local |
| Mức ưu tiên | **17 vấn đề** gồm 9 P0, 7 P1, 1 P2 |
| Phân nhóm theo nguyên nhân chính | 4 thiếu API, 11 có API nhưng chưa đủ nghiệp vụ/an toàn, 2 đã có API chờ nối GUI. Chưa có mục nào đóng toàn bộ |
| Bằng chứng Windows | C++17, Qt Widgets 6.11.2, MinGW 13.1.0, CMake 3.30.5. Bốn ca CTest và bốn lượt chạy tương ứng bằng plugin Windows đạt |

Các đầu vào/đầu ra dưới đây là **đề nghị để thảo luận**. Tên hàm đã tồn tại, file nguồn, trạng thái từng View và bằng chứng của từng mã nằm trong [Backend Gap Tracker](Backend_Gap_Tracker.md).

## P0 — thứ tự xử lý theo đường đi của giao dịch

| Mục | Hợp đồng cần chốt với Kiệt | Lỗi phải phân biệt và tiêu chí nhận |
|---|---|---|
| BG-014 · ghi một file | Từ đường dẫn và toàn bộ nội dung mới, trả kết quả ghi/thay thế. Khi thất bại, bản trước phải còn đọc được | Thử không mở được file, lỗi giữa lần ghi, lỗi đóng/thay thế, mở lại sau sự cố. Trả được tên file lỗi mà không đưa nội dung nhạy cảm vào log |
| BG-017 · độ chính xác giá | Chốt miền số và độ chính xác chung khi đọc, sửa và ghi cả năm trường. Tất cả giá/tỷ lệ được chấp nhận phải đọc lại đúng | Test giá 0.0000001 và thay đổi một tỷ lệ khác, giá có hơn sáu số lẻ, mở lại và đối chiếu. GUI hiện khóa chỉnh sửa nếu bảng đã nạp có giá trị vượt độ chính xác của biểu mẫu |
| BG-006 · xác nhận và ghi nhiều file | Giao dịch nhận suất, danh sách ghế/loại vé, member tùy chọn và xác nhận tuổi, học sinh, tiền. Trả mã hóa đơn và kết quả từng vé chỉ sau khi vé, hóa đơn và điểm nhất quán. Chốt cơ chế khôi phục và thử lại | Từ chối thiếu xác nhận, ghế đã bán và giao dịch hết hiệu lực. Tiêm lỗi sau từng add/save, khởi động lại, thử cùng yêu cầu hai lần. Đối chiếu cả RAM lẫn ba file |
| BG-005 · điểm | Giá và điểm phải có trên từng dòng báo giá, vé đã ghi và kết quả sau hủy. Tổng điểm bằng tổng của từng dòng | Ví dụ hai vé thực trả 19.000 ₫ mỗi vé của thành viên được 1 + 1 = 2 điểm. Code tính trên tổng hiện cho 3 rồi dồn phần chênh vào vé cuối. Kiểm tra khách lẻ, ghế đôi, giới hạn số và thu hồi riêng vé cuối |
| BG-003 · tồn ghế | Truy vấn theo suất và phiên, trả danh sách ghế hoặc trạng thái còn chỗ/hết chỗ cùng lỗi riêng. Xác nhận lại khi chốt tiền | `getAvailableSeats` hiện trả rỗng cho cả phòng hết chỗ và dữ liệu chưa nạp. Test thiếu quyền, sai suất, phòng khóa, cùng seatId ở hai phòng/suất, vé đã hủy và hai lần mua cùng ghế |
| BG-004 · báo giá và bán | `calculateBooking` cần trả chi tiết gốc/giảm/thực trả/điểm của từng vé. `createBooking` đã kiểm tra TICKET_SELL. Kiểm tra lại quyền, giá và điều kiện khi xác nhận | Hết phiên, đủ quyền xem giá nhưng thiếu quyền bán, ghế trùng trong cùng lô, suất vừa bắt đầu, giá đổi sau báo giá. Repository có kiểm tra SOLD trong dữ liệu đang nạp; cần thử tranh chấp và xác định phạm vi hỗ trợ nhiều tiến trình |
| BG-013 · lịch suất đã phát sinh vé | Nhận suất và giờ mới. Service kiểm tra lịch sử vé trước khi cho thay đổi | Thử suất trống và suất từng có vé SOLD/CANCELLED, kể cả đã hoàn hết. Giữ nguyên giờ cũ nếu từ chối |
| BG-010 · quyền xem hóa đơn | Danh sách nhận bộ lọc và khoảng nửa mở. Chi tiết nhận đúng mã. Service lấy người dùng từ phiên, trả dữ liệu chỉ đọc của một suất, các dòng vé, tổng gốc, đã hoàn, còn lại và trạng thái suy ra | Hai nhân viên khác nhau, mã hóa đơn đoán được, hóa đơn cũ của chính mình, danh sách trong ngày, Admin có ALL, mốc 00:00. Cả danh sách và truy vấn chi tiết đều phải bảo vệ phạm vi |
| BG-011 · hủy từng vé | Tra theo vé/hóa đơn, trả điều kiện hủy và số tiền dự kiến. Xác nhận bằng mã vé, lý do và xác nhận hoàn tại quầy. Lấy người hủy/giờ hủy tại Service, hoàn đúng thực trả, thu hồi đúng điểm đã cộng | Yêu cầu Admin và TICKET_CANCEL. Từ chối khi đã hủy, suất vừa bắt đầu, thiếu điểm để thu hồi hoặc ghi lỗi. Thử lại sau sự cố, kiểm tra ghế được giải phóng đúng suất và hóa đơn gốc giữ nguyên tổng |

## P1 và P2

| Mục | Hợp đồng đề nghị | Tiêu chí nhận |
|---|---|---|
| BG-001 · P1 · form Thêm | Dữ liệu nhập không có ID → mã hệ thống cấp hoặc lỗi. Với MOV/ROM/SHW/USR/MEM, các hàm add hiện sao chép input, caller quản lý đối tượng đầu vào. Với Seat, repo nhận chính con trỏ khi add thành công | Test cả true/false/exception, trùng username/SĐT, ID sai/hết dải, lưu và đọc lại. GUI cần biết bản ghi vừa tạo mà không đoán “bản cuối” |
| BG-002 · P1 · lưu danh mục | Chốt Service nào chịu trách nhiệm quyền, mutation, save và phục hồi. Trả kết quả phân biệt thay đổi RAM, đã ghi đĩa và thất bại | Test sửa/xóa, khởi động lại, gọi khi thiếu quyền, lỗi ghi sau khi RAM đổi. Mã, liên kết và số bản ghi phải còn đúng |
| BG-007 · P1 · hủy/xóa suất | Mã suất → trạng thái mới hoặc lý do từ chối. Hủy khi chưa bắt đầu và không còn SOLD. Xóa khi chưa từng có vé | Cả hai hàm hiện có nhưng trả false. Test suất trống, còn SOLD, từng CANCELLED, sát giờ và thiếu quyền |
| BG-008 · P1 · sơ đồ ghế | Phòng, vị trí, loại và khóa ghép → ghế sau thay đổi hoặc lỗi tham chiếu. Chốt cách chuyển tọa độ thành mã như A01. Backend phải bảo vệ cả khi caller bỏ qua GUI | Cùng tọa độ, cùng seatId khác phòng, ghế từng có vé, suất tương lai có lịch sử và kích thước phòng. Kiểm tra liên kết sau khi add thất bại |
| BG-012 · P1 · báo cáo | `[from, until)` → tiền bán, hoàn, thuần, lượng vé bán/hủy, dòng tiền theo ngày và xếp hạng vé còn hiệu lực. Service kiểm tra REPORT_VIEW | Hoàn giao dịch kỳ trước, doanh thu ngày âm, biên tháng, COUPLE tính một vé. Top theo hóa đơn trong kỳ và vé còn SOLD, đồng hạng thì mã phim tăng dần |
| BG-015 · P1 · ID sau xóa/khôi phục | Chốt cách cấp mã không tái sử dụng và cách báo hết dải. Hiện max + 1 trên bản ghi đang nạp áp dụng cả INV/TKT, không có bộ đếm bền vững | Xóa bản ghi có mã cao nhất, mở lại, tạo mới, tạo nhiều vé một lần, thử lại sau giao dịch lỗi, hết 9999 |
| BG-016 · P1 · hồ sơ nhân viên | Mã, username, họ tên → hồ sơ mới hoặc lỗi. Service giữ password, role và active. Tránh buộc GUI truyền lại credential khi chỉ đổi tên | Test trùng username, thiếu quyền, sai ID. Đổi tên rồi đăng nhập lại bằng mật khẩu cũ, kiểm tra dữ liệu trả về và log |
| BG-009 · P2 · trạng thái phòng | Mã phòng và trạng thái đích → trạng thái mới hoặc lý do từ chối. Phần phim đã dùng setMovieActive trong RAM | Phòng còn suất tương lai, phòng chỉ có suất hủy, bật lại, thiếu quyền và đọc lại sau restart. Tái dùng test phim đã có |

## Những nơi Kiệt cần nối lại

| GUI hiện tại | Khi có hợp đồng đã kiểm thử |
|---|---|
| `BookingView.cpp`, `BookingAdapter.h` chọn ghế thử, báo giá qua PricingService; trạng thái ghế Unknown và nút hoàn tất khóa | Nối truy vấn tồn ghế, báo giá Service và giao dịch ghi. Chỉ xóa lựa chọn sau khi có kết quả đã ghi, xử lý báo giá thay đổi và thử lại an toàn |
| `InvoiceHistoryView.cpp`, `TicketCancellationView.cpp`, `ReportView.cpp`, `SalesHistoryAdapter.cpp` trả NotImplemented ở chế độ thật. Preview dùng mẫu riêng, xác nhận hoàn tiền khóa | Thay từng phương thức của adapter, giữ kiểm tra quyền ở Service, bổ sung test có hai Staff và dữ liệu giao dịch trên thư mục tạm |
| `MovieManagementView.cpp`, `RoomManagementView.cpp`, `StaffManagementView.cpp`, `MemberManagementView.cpp`, `ShowtimeManagementView.cpp` khóa tạo mới. Hủy/xóa suất và trạng thái phòng còn khóa | Nối hợp đồng add/save, thống nhất thông báo sau ghi. Nối cập nhật hồ sơ Staff, hủy/xóa suất và trạng thái phòng theo từng quyền |
| `SeatMapView.cpp` giới hạn sửa vào phòng chưa có lịch, dùng khóa ghép; `ApplicationContext` là nơi lắp repository/service | Chuyển quyết định dựa trên lịch sử vé về Service, kiểm tra lại quyền và kết quả khi thực hiện |
| `PricingManagementView.cpp` ghi từng trường theo PricingService; ghi thành công rồi đọc lại đã được kiểm thử | Sau thay đổi FileIO chạy lại test ghi lỗi, mất điện và khôi phục. Giữ nguyên việc báo rõ khi mới ghi được một phần các trường |

**Trình tự nghiệm thu đề nghị.** Chốt lưu, độ chính xác và khôi phục (014, 017, 006, 015), giá/điểm/quyền/ghế (003–005), bảo vệ dữ liệu đã bán (007, 008, 013), sau đó lịch sử và hoàn tiền (010, 011). Mở báo cáo sau khi sự kiện bán và hoàn đáng tin cậy (012). Hoàn thiện danh mục, trạng thái và hồ sơ (001, 002, 009, 016) có thể tiến hành song song khi hợp đồng lưu đã chốt.

Mỗi lần thay adapter, chạy lại `gui_navigation`, `gui_booking`, `gui_restart`, `gui_history` cùng test Service mới. Mốc nghiệm thu giao dịch thật cần bao gồm đọc lại ở tiến trình mới sau ca thành công, ca bị từ chối và ca lỗi ghi. [Bảng nghiệm thu GUI và cách chạy Windows](GUI_Acceptance_Stage7.md)
