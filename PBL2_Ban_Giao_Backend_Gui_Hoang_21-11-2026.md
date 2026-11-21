# PBL2 — BÀN GIAO YÊU CẦU HOÀN THIỆN BACKEND CHO HOÀNG

**Người gửi:** Kiệt (GUI Qt Widgets)  
**Người nhận:** Hoàng (Backend)  
**Ngày lập:** 21/11/2026  
**Mục đích:** Một hồ sơ thống nhất để xử lý những chỗ backend còn thiếu, chưa hoàn thiện, hoặc có API nhưng GUI chưa đủ điều kiện nối vào nghiệp vụ thật. Tài liệu này không phải xác nhận rằng backend đã được sửa.

## 1. Mốc mã nguồn và phạm vi kiểm tra

| Thành phần | Mốc đã kiểm tra |
|---|---|
| GUI đã push | [`feature/b12-kiet-GUI`](https://github.com/duyhoangbk2007/PBL2/tree/feature/b12-kiet-GUI), commit [`b4c63443f1d1dcc345f968825f0e254cde40d88c`](https://github.com/duyhoangbk2007/PBL2/commit/b4c63443f1d1dcc345f968825f0e254cde40d88c) (`done UI`) |
| Backend tham chiếu | [`feature/b10-hoang-booking-management`](https://github.com/duyhoangbk2007/PBL2/tree/feature/b10-hoang-booking-management), commit `b56bf562da1fac45eb8036543245d93f098c3eb8` |
| Báo cáo gốc | `docs/development/Backend_Gap_Tracker.md`; `docs/development/Backend_Handoff_To_Hoang.md`; `docs/development/GUI_Acceptance_Stage7.md` trên B12 |
| Mốc nghiệm thu GUI | Báo cáo Stage 7 được lập trên working tree ứng với HEAD `3c22da9`; **sau đó** B12 đã push commit `b4c6344`, trong đó có cả ba tài liệu và code GUI/test. Các mô tả mốc HEAD cũ **là lịch sử lúc chạy test**, không phải HEAD GitHub hiện hành. |
| Môi trường trong báo cáo | Windows; C++17, Qt Widgets 6.11.2, MinGW GCC 13.1.0, CMake 3.30.5; build launcher và Release từ build sạch; CTest **4/4** trên cả hai cấu hình, và chạy Qt native Windows đạt theo báo cáo Codex. |

**Ranh giới xác minh:** Ba tài liệu và commit B12 đã được mở trực tiếp trên GitHub; các file Backend then chốt trên B10 được đối chiếu trong quá trình làm GUI. Kết quả build/CTest là **bằng chứng được mô tả trong hồ sơ Stage 7**, không phải phép chạy lại độc lập của người tổng hợp. Không suy diễn rằng mọi API/điều kiện đã có đều được kiểm thử tích hợp thật.

**Tổng tồn đọng:** **17 vấn đề: 9 P0, 7 P1, 1 P2.** Phân loại theo nguyên nhân chính: **4 thiếu API** (BG-009 phần trạng thái phòng; BG-010, BG-011, BG-012), **11 API có nhưng chưa đủ bảo đảm/nghiệp vụ** (BG-003, 004, 005, 006, 007, 008, 013, 014, 015, 016, 017), **2 API đã có nhưng GUI cần hợp đồng tích hợp** (BG-001, BG-002). Không có BG nào đủ điều kiện đóng toàn bộ tại mốc kiểm tra. Một số thành phần bên trong BG-002 và BG-009 đã được chứng minh hoạt động.

## 2. Tình trạng GUI thực tế — tránh nhầm DEMO với chức năng thật

| Khu vực GUI | Đã gọi Service thật | Còn khóa / DEMO / hạn chế |
|---|---|---|
| Đăng nhập, phân quyền | `AuthService` login/logout/hasPermission; kiểm thử Admin/Staff, tài khoản vô hiệu, đổi phiên | Phân quyền lịch sử hóa đơn thật cần Service riêng ở BG-010 |
| Phim | Xem/tìm/sửa/xóa hợp lệ; `setMovieActive` bật/tắt và chặn tắt khi còn suất tương lai | Thêm phim khóa; thay đổi danh mục chưa chứng minh lưu sau restart |
| Phòng | Xem/sửa/xóa hợp lệ qua `RoomService` | Thêm phòng và bật/tắt phòng khóa; sửa chỉ RAM |
| Ghế | Hiển thị ma trận, thêm/sửa/xóa qua `SeatService` cho phòng đủ điều kiện | Ràng buộc ghế từng có vé cần thực thi tại backend; thay đổi chưa có bảo đảm lưu |
| Suất chiếu | Xem/lọc bốn điều kiện/sửa hợp lệ qua `ShowtimeService` | Thêm, hủy, xóa khóa; sửa thời gian suất từng có vé còn rủi ro |
| Nhân viên | Xem/tìm/sửa/vô hiệu hóa qua `StaffService` | Thêm khóa; sửa hồ sơ phải chuyển mật khẩu cũ; chưa lưu bền vững |
| Thành viên | Xem/tìm/sửa liên hệ qua `MemberService`; điểm chỉ đọc | Thêm khóa; sửa chưa lưu bền vững |
| Giá và ưu đãi | `PricingService` cập nhật từng trường; đã thử ghi TXT và đọc lại sau restart, quyền Admin/Staff | Nguy cơ truncate file khi lỗi giữa ghi; `std::to_string` làm mất số quá 6 chữ số thập phân; GUI tạm khóa sửa khi gặp giá nguồn không biểu diễn đủ |
| Bán vé tại quầy | Movie/Showtime/Room/Seat/Member/PricingService hỗ trợ chọn và **tạm tính**; undo, bỏ hết, ghế COUPLE, tối đa 8 kết quả thành viên | Trạng thái ghế `Unknown`, chọn thử gắn DEMO; **Hoàn tất giao dịch khóa**, không gọi `BookingService::createBooking()` |
| Lịch sử hóa đơn | UI bộ lọc, chi tiết chỉ đọc, kiểm tra quyền ở View | `SalesHistoryAdapter` trả `NotImplemented` khi truy vấn thật; DEMO riêng |
| Hủy vé/hoàn tiền | UI tìm kiếm, xem chi tiết và xác nhận DEMO | **Xác nhận hoàn tiền khóa**; không sửa Ticket/Invoice/Member thật |
| Báo cáo | UI kỳ, 5 chỉ số, dòng tiền và top phim | Service thật chưa có; mode thật dấu `—`, chỉ có số liệu DEMO được gắn nhãn |

## 3. Thứ tự ưu tiên đề nghị Hoàng thực hiện

1. **Nền tảng dữ liệu:** BG-014 (ghi an toàn một file) → BG-017 (độ chính xác giá) → BG-015 (ID không tái sử dụng), đồng thời chốt BG-002 (khi nào save, lỗi sau RAM mutation).
2. **Giao dịch bán vé:** BG-005 (điểm từng vé), BG-003 (ghế trống theo suất), BG-004 (quyền & báo giá), BG-006 (xác nhận và ghi đa file/khôi phục). Chỉ mở bán thật sau khi đạt chung các test.
3. **Bảo vệ lịch sử đã bán:** BG-013 (không sửa lịch suất đã từng có vé), BG-008 (không sửa/xóa ghế làm hỏng lịch sử), BG-007 (hủy/xóa suất có kiểm tra Ticket).
4. **Hóa đơn và hoàn tiền:** BG-010 (tra cứu đúng phạm vi) → BG-011 (hủy từng vé, thu hồi điểm).
5. **Báo cáo:** BG-012 sau khi dòng tiền bán/hoàn và lịch sử đã đáng tin cậy.
6. **Hoàn thiện danh mục:** BG-001 (form Thêm, contract ownership/ID), BG-009 (trạng thái phòng), BG-016 (cập nhật Staff không truyền credential). Có thể triển khai song song nếu quy tắc lưu đã rõ.

---

## 4. DANH SÁCH CHI TIẾT 17 BACKEND GAP

### BG-014 — P0 — Bảo toàn dữ liệu khi ghi một file (OPEN)

- **Hiện trạng:** `src/utils/FileIO.cpp::writeLines` mở `std::ofstream` trực tiếp trên file đích; thao tác truncate xảy ra trước khi nội dung mới ghi xong. `PricingRepository::update` và `Repository<T>::save` đều phụ thuộc cơ chế này.
- **Tác động:** Mất điện, lỗi ghi hoặc đóng tệp có thể khiến file cũ mất một phần nội dung; GUI có báo thất bại vẫn không bảo đảm file trước còn nguyên.
- **Hoàng cần làm:** Chốt cách ghi qua file tạm cùng volume, flush/close, thay thế an toàn trong môi trường Windows, xử lý lỗi và thông báo chính xác tệp bị lỗi. Giữ dữ liệu cũ đọc được nếu cập nhật không hoàn thành. Đây **không thay thế** xử lý đa file BG-006.
- **Test nghiệm thu:** lỗi trước khi mở, giữa ghi, lúc close/replace; so sánh dữ liệu cũ, khởi động lại, không mất dòng; quyền ghi và đường dẫn Unicode.

### BG-017 — P0 — Độ chính xác giá/tỷ lệ khi lưu (OPEN)

- **Hiện trạng:** `PricingRepository::load` chấp nhận số thập phân hơn 6 chữ số; `Pricing::isValid` cho phép số dương rất nhỏ; `PricingRepository::update/save` dùng `std::to_string` khi serialize cả năm trường. Ví dụ `0.0000001` có thể thành `0.000000` và gây hỏng dữ liệu khi đọc lại.
- **GUI:** Đã dùng `PricingService` thật và thử restart; có chặn lưu khi dữ liệu nguồn vượt mức biểu diễn an toàn trong form; không giải quyết được lời gọi từ nơi khác.
- **Hoàng cần làm:** Chốt miền số, số chữ số thập phân và phương pháp parse/serialize thống nhất; mọi giá trị được chấp nhận phải round-trip đúng hoặc bị từ chối **trước khi ghi**, không để thay đổi một trường phá các trường khác.
- **Test:** `0.0000001`, các giá cực lớn, >6 chữ số thập phân, đổi riêng discount nhưng giữ nguyên giá khác, restart và so sánh.

### BG-006 — P0 — Xác nhận tại quầy, ghi đa file và khôi phục giao dịch (BLOCKED)

- **Hiện trạng:** `BookingService::createBooking()` có thật, nhưng chưa nhận xác nhận đã thu tiền/độ tuổi/điều kiện STUDENT. Luồng thêm Invoice rồi từng Ticket, cộng điểm Member và `save()` lần lượt: nếu lỗi giữa chừng có nguy cơ RAM/TXT không nhất quán, retry có thể gây trùng giao dịch.
- **GUI:** Có ba dòng xác nhận; nút Hoàn tất bị khóa.
- **Hoàng cần làm:** Chốt input giao dịch (showtime, từng seat/ticketType, member tùy chọn, xác nhận điều kiện tại quầy); xác minh lại toàn bộ ngay lúc ghi. Thiết kế trạng thái lỗi rõ, khôi phục/rollback hoặc cơ chế phục hồi và retry không trùng. Chỉ trả kết quả thành công và invoiceId sau khi dữ liệu nhất quán.
- **Test:** thiếu từng xác nhận; lỗi sau từng `add/save`, tắt/mở lại; thử lại cùng giao dịch; 2 lần bán cùng ghế; đối chiếu Invoice/Ticket/Member ở cả RAM lẫn TXT.

### BG-005 — P0 — Tính và ghi điểm theo **từng vé** (OPEN)

- **Hiện trạng:** `PricingService::calculatePoints(paidAmount,isMember)` đã có. Nhưng `BookingService::calculateBooking` tính điểm trên **tổng hóa đơn**, và `createBooking` dồn chênh lệch vào vé cuối; trái business rule `sum(floor(paid_each_ticket / 10000))`.
- **Ví dụ xác định lỗi:** Hai vé mỗi vé thực trả **19.000 ₫**: quy tắc đúng là `1 + 1 = 2 điểm`, nhưng `floor(38.000 / 10.000) = 3 điểm`.
- **Hoàng cần làm:** Điểm từng vé được tính sau giảm giá, lưu đúng trên Ticket và cộng lại ở Invoice/Member. Không phân bổ điểm hóa đơn bằng cách dồn vào vé cuối. Giữ `pointsEarned` trên Ticket để thu hồi chính xác khi hủy riêng vé đó.
- **Test:** hai vé có phần dư, vé cuối, vé COUPLE, khách không có Member, số lớn, hủy một vé.

### BG-003 — P0 — Trạng thái ghế theo suất, quyền và lỗi tra cứu (BLOCKED)

- **Hiện trạng:** `BookingService::getAvailableSeats(showtimeId)` có thật, dựa trên Ticket đã SOLD; nhưng trả danh sách rỗng cho nhiều nguyên nhân (hết ghế, suất không hợp lệ, repository chưa nạp…), chưa kiểm tra quyền rõ ở đầu phương thức.
- **GUI:** `BookingAdapter::seatStatus` trả `Unknown`; chỉ chọn thử trong DEMO.
- **Hoàng cần làm:** Chốt Business API trả trạng thái/lý do riêng cho `(showtimeId,seatId)` hoặc danh sách khả dụng, có kiểm tra quyền và phiên; phân biệt ghế hết chỗ với lỗi load hoặc suất sai. Giao dịch chốt tiền luôn kiểm tra lại để ngăn bán trùng.
- **Test:** cùng seatId ở hai phòng/suất, ghế từng SOLD/CANCELLED, suất đã qua, phòng ngừng hoạt động, thiếu quyền, repository lỗi, hai giao dịch đồng thời trong phạm vi single-process.

### BG-004 — P0 — Báo giá và quyền tạo booking (BLOCKED)

- **Hiện trạng:** `BookingService::calculateBooking` và `createBooking` đã tồn tại. `createBooking` **đã có** `requirePermission("TICKET_SELL")`; không lặp lại nhận xét cũ rằng API hoàn toàn không có quyền. `calculateBooking` chưa có check quyền bán rõ ở đầu. `BookingSummary` hiện chỉ có danh sách tiền thực trả, tổng và điểm.
- **GUI:** Tạm tính qua PricingService thật; chưa gọi BookingService để giao dịch.
- **Hoàng cần làm:** Chốt quyền/phiên khi báo giá và bán; trả từng dòng **giá gốc – giảm – thực trả – điểm** và lỗi phân biệt; kiểm tra giá, loại vé, thành viên, ghế và suất tại thời điểm xác nhận, không tin báo giá cũ.
- **Test:** chưa đăng nhập, chỉ có PRICING_VIEW nhưng thiếu TICKET_SELL, ghế trùng trong lô, suất vừa bắt đầu, thành viên mất hiệu lực và giá đổi giữa báo giá/bán.

### BG-013 — P0 — Chặn sửa giờ suất đã từng có vé (BLOCKED)

- **Hiện trạng:** `ShowtimeService::updateShowtime`/`ShowtimeRepository::update` kiểm tra lịch và trạng thái nhưng chưa kiểm tra **lịch sử vé**. Suất từng có vé SOLD hoặc CANCELLED vẫn có nguy cơ bị đổi thời gian, làm sai điều kiện hủy và lịch sử.
- **GUI:** Sửa lịch đang gọi Service thật, trước mắt cập nhật RAM.
- **Hoàng cần làm:** Tra lịch sử vé ở tầng Business khi sửa, chặn mọi thay đổi phim/phòng/thời gian đã phát sinh vé theo quy tắc hiện hành; không chỉ xét vé còn SOLD.
- **Test:** chưa từng có vé thì sửa được, từng có SOLD/CANCELLED thì từ chối, giờ cũ giữ nguyên.

### BG-010 — P0 — Tra cứu hóa đơn có kiểm soát phạm vi (BLOCKED)

- **Hiện trạng:** Đã có Model/Repository Invoice và Ticket, **chưa có Business Service** để lấy danh sách/chi tiết theo đúng quyền. Màn hình thật đang trả `NotImplemented`.
- **Hoàng cần làm:** API danh sách lọc `[from, until)` và chi tiết theo invoiceId; Service tự lấy người dùng đang đăng nhập. Admin `INVOICE_VIEW_ALL` xem toàn bộ khi có quyền; Staff `INVOICE_VIEW_OWN` chỉ xem hóa đơn do mình lập trong ngày, hóa đơn cũ chỉ tra đúng mã và vẫn phải của mình. Phạm vi phải áp dụng cả danh sách lẫn xem chi tiết; không cho đoán mã để lộ hóa đơn người khác.
- **Trả về:** thông tin hóa đơn, đúng các vé thuộc suất, tổng gốc, đã hoàn, còn lại, trạng thái suy ra ACTIVE/PARTIALLY_CANCELLED/CANCELLED.
- **Test:** hai Staff, hôm nay/hôm qua, tra mã đoán được, Admin đủ/thiếu quyền, biên 00:00, nhiều vé một hóa đơn.

### BG-011 — P0 — Hủy từng vé và hoàn tiền/điểm (BLOCKED)

- **Hiện trạng:** `Ticket::markCancelled` và trường audit đã có ở Model, **chưa có Business Service** hủy vé thật, kiểm tra quyền và lưu giao dịch. GUI chỉ có dialog DEMO, xác nhận bị khóa.
- **Hoàng cần làm:** Tra vé/hóa đơn và điều kiện hủy; xác nhận hủy theo ticketId, lý do và việc hoàn tiền tại quầy. Kiểm tra **Admin và `TICKET_CANCEL`**, vé SOLD, suất chưa bắt đầu; hoàn đúng paidAmount, lưu thời gian/người hủy/lý do, giải phóng ghế theo suất, thu hồi đúng điểm đã cộng của vé; giữ nguyên tổng gốc hóa đơn. Bảo đảm xử lý lỗi ghi đa file và thử lại.
- **Test:** hủy một trong nhiều vé, hủy lại, suất vừa bắt đầu, Staff cố hủy, Member thiếu điểm, lỗi lưu từng bước và restart; trạng thái hóa đơn suy ra đúng.

### BG-001 — P1 — Form Thêm danh mục và hợp đồng cấp mã/ownership (OPEN)

- **Hiện trạng:** `addMovie`, `addRoom`, `addStaff`, `addMember`, `addShowtime` **đã sinh ID** bằng `IdGenerator` từ dữ liệu đang nạp, tạo bản sao mới rồi `add`; trả `bool` nên GUI chưa có hợp đồng rõ cách nhận ID vừa tạo. Input pointer ở năm hàm này vẫn thuộc caller. `SeatService::addSeat` đưa pointer vào Repository khi thành công.
- **GUI:** Năm form Thêm vẫn khóa; thêm ghế phòng đủ điều kiện chạy RAM.
- **Hoàng cần làm:** Chốt API nhập dữ liệu không cần ID và **trả ID/bản ghi được tạo** hoặc cơ chế tin cậy tương đương; quy tắc ownership, lỗi trùng/hết ID, phối hợp save. Không yêu cầu tạo lại chức năng cấp ID vốn đã có.
- **Test:** thêm liên tiếp, hết 9999, sai quyền, input lỗi, ownership với true/false/exception, save/restart; kiểm tra BG-015 để không tái dùng ID.

### BG-002 — P1 — Lưu bền vững thay đổi danh mục (OPEN)

- **Hiện trạng:** Các Service phim/phòng/ghế/suất/nhân viên/thành viên có `save()`, nhưng nhiều thao tác chỉnh sửa chỉ cập nhật RAM; UI không tự gọi save tùy tiện. Giá vé là ngoại lệ đã được ghi/đọc lại.
- **Hoàng cần làm:** Chốt trách nhiệm Service/application của mutation + save; trạng thái trả về phải phân biệt **RAM đổi**, **đã ghi TXT**, **lỗi ghi và phục hồi**. Xem lại quyền/lifecycle của `save()` gọi trực tiếp; không cho bỏ qua kiểm soát quyền qua API công khai.
- **Test:** sửa/thêm/xóa rồi restart, file không thể ghi, lỗi khi RAM đã đổi, quyền gọi save, dữ liệu tham chiếu không hỏng. Phối hợp với BG-014.

### BG-007 — P1 — Hủy/xóa suất chiếu (BLOCKED)

- **Hiện trạng:** `ShowtimeService::cancelShowtime` và `removeShowtime` có kiểm tra quyền và điều kiện ban đầu, nhưng vẫn `return false` vì chưa kiểm tra lịch sử vé.
- **GUI:** Hai nút khóa.
- **Hoàng cần làm:** Hủy suất trước giờ và không còn vé SOLD; xóa chỉ khi **chưa từng** có vé; không ngầm hủy hàng loạt vé. Trả nguyên nhân từ chối cho GUI.
- **Test:** suất trống; còn vé SOLD; chỉ còn CANCELLED; sát giờ; quyền không đủ; dữ liệu lưu sau restart.

### BG-008 — P1 — Bảo vệ sơ đồ ghế theo lịch sử vé (BLOCKED)

- **Hiện trạng:** `SeatService` có CRUD, nhưng chưa phối hợp đầy đủ lịch sử Ticket để ngăn sửa/xóa ghế từng có vé và bảo vệ sơ đồ của suất tương lai. GUI đang tự hạn chế thao tác phòng có lịch, không thay thế backend.
- **Hoàng cần làm:** Mọi ràng buộc về vị trí/seatId/loại ghế/lịch sử vé phải được kiểm tra tại tầng Service; dùng khóa ghép roomId + seatId, tránh nhầm ghế giữa phòng. Chốt cách gán mã tọa độ như A04.
- **Test:** tọa độ trùng, cùng seatId khác phòng, vé đã hủy, suất tương lai từng phát sinh vé, chỉnh sơ đồ sau bán, rollback khi add thất bại.

### BG-012 — P1 — ReportService doanh thu và top phim (BLOCKED)

- **Hiện trạng:** Chưa có `ReportService` truy vấn thật; GUI chỉ hiển thị số DEMO hoặc dấu `—`.
- **Hoàng cần làm:** API có `REPORT_VIEW`, khoảng thời gian `[from, until)`; tiền bán theo thời gian hóa đơn, tiền hoàn theo thời gian hủy, thuần = bán − hoàn (có thể âm), số vé bán/hủy tương ứng, số liệu theo ngày và top phim trong kỳ. Top phim chỉ tính vé thuộc hóa đơn lập trong kỳ **còn SOLD lúc xem**; sắp xếp giảm dần, hòa thì movieId tăng dần. COUPLE tính một vé, hai người nếu tính số người.
- **Test:** hoàn hóa đơn kỳ cũ, ngày âm, biên ngày/tháng, kỳ không dữ liệu, số tiền lớn, phim hòa hạng, quyền bị từ chối; phân biệt dòng tiền và top phim.

### BG-015 — P1 — Không cấp lại ID đã từng dùng (OPEN)

- **Hiện trạng:** `IdGenerator::generateNextId` lấy max từ **bản ghi còn đang nạp** rồi cộng 1, trần 9999. Xóa bản ghi mã lớn nhất có thể khiến lần tạo sau sử dụng lại danh tính đã cấp. Dùng cho MOV/ROM/SHW/USR/MEM và INV/TKT trong booking.
- **Hoàng cần làm:** Chốt chính sách ID bất biến, không tái sử dụng kể cả khi bản ghi cũ đã bị xóa; nguồn cấp mã bền vững và phục hồi sau lỗi giao dịch. Không kết luận chỉ từ việc hiện không dùng `sequences.txt` rằng bộ đếm đã an toàn.
- **Test:** xóa max, restart, tạo mới; nhiều vé trong lô; giao dịch thất bại rồi retry; hết 9999.

### BG-016 — P1 — API sửa Staff không phải truyền credential qua GUI (OPEN)

- **Hiện trạng:** `StaffService::updateStaff(const User&)` làm GUI phải đọc lại mật khẩu cũ và chuyển trong đối tượng User khi chỉ muốn thay tên/username. GUI không hiển thị/log mật khẩu; test đăng nhập lại bằng mật khẩu cũ đạt.
- **Hoàng cần làm:** API cập nhật profile theo userId, username, fullName; Service tự giữ password, role, active. Chặn đổi credential/role/active qua đường sửa profile và kiểm tra quyền.
- **Test:** sửa username/tên, trùng username, sai ID, thiếu quyền; đăng nhập bằng password cũ; bảo đảm không lộ thông tin nhạy cảm.

### BG-009 — P2 — Bật/tắt trạng thái phòng (OPEN)

- **Hiện trạng:** Phim đã có `MovieService::setMovieActive()` và GUI gọi thật. `RoomService` chưa có API đổi active; `updateRoom()` hiện từ chối đổi cờ trạng thái.
- **Hoàng cần làm:** Nếu giữ chức năng theo wireframe, chốt API `setRoomActive` (tên đề nghị, chưa phải API thật) với kiểm tra suất tương lai/chưa hủy, quyền và kết quả lưu.
- **Test:** phòng còn suất tương lai, chỉ có suất đã hủy, bật lại, thiếu quyền, restart.

---

## 5. Hợp đồng tích hợp đề nghị — không nhầm với API đã tồn tại

Các chữ ký dưới đây là **phạm vi input/output để hai bên thảo luận**, không phải lệnh sửa code hoặc cam kết tên hàm có sẵn:

| Nhu cầu | Input cần có | Output/lỗi cần có | Những BG liên quan |
|---|---|---|---|
| Tạo danh mục | Dữ liệu nghiệp vụ không gồm ID, user từ phiên | Record/ID được tạo; đã ghi hay chưa; mã lỗi validation/permission/save | 001, 002, 015 |
| Ghế còn trống | showtimeId và phiên | Ghế + status AVAILABLE/SOLD/UNKNOWN hoặc result phân biệt sai suất/hết chỗ/chưa load/thiếu quyền | 003, 004 |
| Báo giá | showtimeId, danh sách `{seatId,ticketType}`, member tùy chọn | Dòng vé gồm giá gốc/giảm/thực trả/điểm, tổng; lỗi/điều kiện hết hiệu lực | 004, 005 |
| Xác nhận bán | Dữ liệu báo giá + xác nhận tại quầy và phiên | invoiceId, ticketIds, trạng thái ghi, lỗi cụ thể, hành vi retry an toàn | 003–006, 014, 015, 017 |
| Danh sách/chi tiết hóa đơn | Khoảng `[from,until)`, bộ lọc/mã chính xác, phiên | Dữ liệu chỉ đọc đã giới hạn OWN/ALL, từng vé, hoàn tiền/trạng thái suy ra | 010 |
| Hủy từng vé | ticketId, lý do, xác nhận đã hoàn tiền, phiên | Trạng thái vé, tiền hoàn, điểm thu hồi, thời gian/người hủy, trạng thái ghi/retry | 005, 006, 011 |
| Báo cáo | `[from,until)`, phân nhóm ngày, tiêu chí top phim, phiên | Tiền bán/hoàn/thuần, số vé bán/hủy, chuỗi ngày, xếp hạng | 012 |
| Hủy/xóa suất và sửa sơ đồ | ID, điều kiện thời gian, trạng thái/lịch sử vé, phiên | Thành công hoặc lý do từ chối; không phá liên kết lịch sử | 007, 008, 013 |

## 6. Những việc Kiệt cần làm sau khi Hoàng hoàn thiện Backend

| File GUI / adapter | Việc cần nối lại |
|---|---|
| `src/gui/BookingAdapter.h`, `BookingView.cpp` | Đổi `Unknown` sang trạng thái Service; báo giá từ Business, validate lại tại chốt tiền; chỉ bật nút Hoàn tất khi nghiệp vụ bán an toàn; xử lý lỗi, reset, retry và hiển thị invoiceId thật. |
| `src/gui/SalesHistoryAdapter.h/.cpp`, `InvoiceHistoryView.cpp` | Thay `NotImplemented` bằng API danh sách/chi tiết, bảo vệ riêng OWN/ALL, test hai nhân viên và dữ liệu cũ theo mã. |
| `src/gui/TicketCancellationView.cpp` | Tìm vé thật, hiển thị quyền/đủ điều kiện, hủy từng vé, cập nhật ghế/điểm và các trạng thái hóa đơn sau khi Service trả thành công. |
| `src/gui/ReportView.cpp` | Lấy số liệu thật từ ReportService; giữ DEMO riêng, test ngày âm và top phim. |
| `MovieManagementView.cpp`, `RoomManagementView.cpp`, `ShowtimeManagementView.cpp`, `StaffManagementView.cpp`, `MemberManagementView.cpp` | Mở các nút Thêm khi có ID trả về + save đáng tin cậy, cập nhật/khôi phục theo kết quả ghi; nối trạng thái phòng và sửa profile Staff nếu API bổ sung. |
| `SeatMapView.cpp` | Bỏ việc tự quyết định thay Service về lịch sử vé, kiểm thử ràng buộc và kết quả lưu. |
| `PricingManagementView.cpp` | Sau sửa FileIO/serializer, bỏ workaround hạn chế độ chính xác khi backend bảo đảm round-trip; chạy lại ca lỗi ghi. |

## 7. Điều kiện chấp nhận trước khi bật giao dịch thật

**Không mở khóa bán vé** chỉ vì code đã có `BookingService`: cần đạt kiểm thử quyền, ghế theo suất, báo giá từng vé, điểm từng vé, xác nhận tại quầy, giao dịch đa file và thử lại sau lỗi. Sau một giao dịch thành công, khởi động lại ở tiến trình khác phải đọc thấy **cùng Invoice, Ticket, Member points**; sau giao dịch bị từ chối hoặc lỗi phải không phát sinh trạng thái bán trùng hoặc không nhất quán. Tương tự, không mở hủy vé khi chưa có Service quyền/hoàn/thu hồi điểm/khôi phục.

**Hồi quy GUI cần chạy sau mỗi đợt nối backend:** `gui_navigation`, `gui_booking`, `gui_restart`, `gui_history` cùng unit/integration tests của Service mới. Dùng fixture/tệp tạm, tuyệt đối không chạy kiểm thử phá hủy trên `data/` vận hành. Báo cáo Stage 7 chưa kiểm thử gói chạy độc lập trên máy Windows sạch, nên không ghi là đã đóng gói hoàn chỉnh.

## 8. Những phần đã kiểm chứng, không yêu cầu Hoàng làm lại

- Đăng nhập/đăng xuất và menu phân quyền bằng `AuthService` đã được kiểm thử ở cấp GUI (chưa thay thế test phân quyền truy vấn hóa đơn mới).
- `MovieService::setMovieActive` đã nối với GUI, có test bật/tắt và từ chối tắt phim còn suất tương lai (hiệu lực RAM, còn chờ lưu bền vững).
- `PricingService` có API xem/cập nhật giá và ưu đãi; GUI đã test ghi/đọc lại qua hai tiến trình và Staff thiếu quyền. **Chỉ** cần xử lý các lỗi FileIO/precision chưa giải quyết.
- `BookingService` bản đầu **đã tồn tại** trên B10, bao gồm `getAvailableSeats`, `calculateBooking`, `createBooking`; cần sửa/bổ sung và kiểm thử, không phải yêu cầu viết lại từ đầu.
- Năm Service thêm mới đã có cơ chế cấp ID dựa trên dữ liệu nạp; vẫn cần contract trả ID, không tái sử dụng mã và lưu dữ liệu.

## 9. Kết luận và yêu cầu phản hồi của Hoàng

Đề nghị Hoàng phản hồi theo từng BG: **đồng ý / điều chỉnh yêu cầu / đã sửa**, kèm tên API cuối cùng, commit chứa thay đổi, test đã chạy và kết quả. Kiệt sẽ thay các adapter `NotImplemented` và kích hoạt dần các chức năng sau khi contract được chốt và kiểm thử trên dữ liệu tạm. Có thể chia thành các đợt: **(1) độ an toàn dữ liệu, (2) bán vé, (3) hóa đơn/hủy, (4) báo cáo và danh mục**.

---

### Nguồn gốc tài liệu đã gộp

- [Backend Gap Tracker — B12](https://github.com/duyhoangbk2007/PBL2/blob/b4c63443f1d1dcc345f968825f0e254cde40d88c/docs/development/Backend_Gap_Tracker.md)
- [Backend Handoff to Hoàng — B12](https://github.com/duyhoangbk2007/PBL2/blob/b4c63443f1d1dcc345f968825f0e254cde40d88c/docs/development/Backend_Handoff_To_Hoang.md)
- [GUI Acceptance Stage 7 — B12](https://github.com/duyhoangbk2007/PBL2/blob/b4c63443f1d1dcc345f968825f0e254cde40d88c/docs/development/GUI_Acceptance_Stage7.md)
- [Backend B10 đã đối chiếu](https://github.com/duyhoangbk2007/PBL2/tree/b56bf562da1fac45eb8036543245d93f098c3eb8/src/business)
