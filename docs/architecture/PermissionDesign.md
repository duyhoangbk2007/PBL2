# PermissionDesign — RBAC Permission-only

**Trạng thái:** API `AuthService::hasPermission()` và `requirePermission()` đã có; quy tắc Permission-only cho toàn bộ Service là **thiết kế dự kiến**, chưa được triển khai đầy đủ. Xem [DIP](dependency_inversion.md), [hợp đồng GUI](Business_UI_Contract.md), [nghiệp vụ](../overview/business_rules.md).

## 1. Nguồn quyền và API hiện tại

`data/role_permissions.txt` hiện có **31 mã Permission, 41 cặp**: 31 `ADMIN`, 10 `STAFF`. Mỗi dòng `role|permissionCode`, không header và không `#schema=1`; cặp `(role, permissionCode)` duy nhất. `RolePermissionRepository` hiện kiểm tra số trường, role `ADMIN/STAFF` và cặp trùng, rồi tra `hasPermission(role, code)`; **chưa đối chiếu `permissionCode` với danh mục 31 mã**. Đây là khoảng trống so với quy tắc dữ liệu mục tiêu, không được ghi là đã bảo đảm. Repository không có `save/grant/revoke`. `AuthService` giữ `currentUserId`, tra lại `UserRepository` và trạng thái `active`, ánh xạ `UserRole` sang role trong bảng. `hasPermission(code)` trả `false` nếu không có phiên hợp lệ, bảng chưa load hoặc không có cặp; `requirePermission(code)` gọi `requireLogin()` và ném `runtime_error` khi không có quyền, trả `const User&` khi được phép. Lỗi load bảng quyền phải từ chối truy cập; không cấp dự phòng.

`AuthService::requireAdmin()`, `isAdmin()`, `isStaff()` và `requireLogin()` **vẫn có trong code**. `requireLogin()` chỉ xác nhận phiên, không cấp quyền nghiệp vụ. `requireAdmin()`/role cứng không còn là cơ chế được phép dùng để cấp quyền trong thiết kế mục tiêu.

## 2. Ma trận quyền thực tế

| Permission | ADMIN | STAFF | Permission | ADMIN | STAFF |
| --- | :---: | :---: | --- | :---: | :---: |
| `MOVIE_VIEW` | Có | Có | `MOVIE_ADD` | Có | — |
| `MOVIE_UPDATE` | Có | — | `MOVIE_DELETE` | Có | — |
| `ROOM_VIEW` | Có | Có | `ROOM_ADD` | Có | — |
| `ROOM_UPDATE` | Có | — | `ROOM_DELETE` | Có | — |
| `SEAT_VIEW` | Có | Có | `SEAT_ADD` | Có | — |
| `SEAT_UPDATE` | Có | — | `SEAT_DELETE` | Có | — |
| `SHOWTIME_VIEW` | Có | Có | `SHOWTIME_ADD` | Có | — |
| `SHOWTIME_UPDATE` | Có | — | `SHOWTIME_CANCEL` | Có | — |
| `SHOWTIME_DELETE` | Có | — | `STAFF_VIEW` | Có | — |
| `STAFF_ADD` | Có | — | `STAFF_UPDATE` | Có | — |
| `STAFF_DEACTIVATE` | Có | — | `MEMBER_VIEW` | Có | Có |
| `MEMBER_ADD` | Có | Có | `MEMBER_UPDATE` | Có | Có |
| `TICKET_SELL` | Có | Có | `TICKET_CANCEL` | Có | — |
| `INVOICE_VIEW_OWN` | Có | Có | `INVOICE_VIEW_ALL` | Có | — |
| `REPORT_VIEW` | Có | — | `PRICING_VIEW` | Có | Có |
| `PRICING_UPDATE` | Có | — | | | |

Đây là cặp quyền **trong file hiện tại**, không phải bảo đảm nghiệp vụ tương ứng đã hoàn thành. File rỗng hợp lệ nhưng không cấp quyền. Không tự tạo mã mới hoặc giả định Admin có quyền ngoài cặp đã nạp.

## 3. Quy tắc đích và luồng kiểm tra

1. Tầng ghép nối nạp User và RolePermission thành công trước khi cho gọi nghiệp vụ. Thiếu/sai file quyền hoặc bảng nạp dở phải từ chối, không mặc định cho phép.
2. GUI dùng `authService.hasPermission("CODE")` để hiển thị, ẩn hoặc bật/tắt chức năng. Kết quả này chỉ phục vụ trải nghiệm; quyền có thể thay đổi trước lúc bấm.
3. **Mỗi entry point nghiệp vụ** của Service gọi `authService.requirePermission("CODE")` trước khi đọc/sửa dữ liệu được bảo vệ; Service tiếp tục kiểm tra quy tắc nghiệp vụ và phạm vi dữ liệu. Không dùng `requireAdmin()`, `isAdmin()`, `isStaff()` hay so role cứng để **cấp quyền**.
4. Repository chỉ validate dữ liệu, CRUD, tham chiếu, link/unlink và lưu trữ. Việc kiểm tra enum role của User hoặc seller để bảo vệ dữ liệu hợp lệ khác với cấp quyền thao tác; repository không nhận phiên người dùng để phân quyền.
5. Không có Permission thì từ chối thao tác; không trả `false` như một cách bỏ qua lỗi quyền nếu hợp đồng hiện tại ném exception. `load()/save()` là vòng đời dữ liệu nội bộ, không phải nút thao tác GUI; cách bảo vệ nếu công khai chúng cần chốt riêng.

Ví dụ thiết kế (chưa triển khai cho mọi hàm):

```cpp
const User& actor = authService.requirePermission("TICKET_CANCEL");
// Dùng actor.getUserId() để ghi nhận người hủy; sau đó kiểm tra điều kiện vé.
```

`requirePermission()` trả người dùng hiện tại để ghi audit; không cần `requireAdmin()` để lấy ID. Tài liệu này không thay chữ ký public hiện có.

## 4. Đối chiếu Service hiện tại và việc dự kiến

| Service | Hiện tại | Dự kiến |
| --- | --- | --- |
| Movie, Room, Seat, Showtime, Staff, Member | Các entry point nghiệp vụ chính đã gọi mã Permission tương ứng. `StaffService` lọc role `STAFF` để xác định đối tượng nhân viên. | Giữ Permission; role dùng làm ràng buộc loại đối tượng, không cấp quyền. Rà `load/save` nếu GUI gọi trực tiếp. |
| Pricing | Xem/sửa dùng `PRICING_VIEW/UPDATE`; `calculateDiscount`, `calculatePaidAmount`, `calculatePoints` chỉ `requireLogin()`. | Chốt liệu helper tính toán là nghiệp vụ công khai cần Permission nào hoặc chỉ gọi nội bộ sau entry point được bảo vệ. Không tự tạo mã mới. |
| Booking | `createBooking()` dùng `TICKET_SELL`; `cancelTicket()` gọi cả `requireAdmin()` và `TICKET_CANCEL`; `calculateBooking()`/`getAvailableSeats()` cần rà quyền khi là entry point. | Bỏ role gate của hủy sau khi chốt xung đột nghiệp vụ; dùng Permission phù hợp cho preview/ghế nếu công khai. |
| Invoice | Chọn `INVOICE_VIEW_ALL/OWN` theo role cứng; kiểm tra chủ hóa đơn và giới hạn ngày cho Staff. | Chọn phạm vi theo Permission: `VIEW_ALL` cho xem toàn bộ, `VIEW_OWN` cho dữ liệu của `currentUserId`; giữ kiểm tra owner/ngày độc lập. Quy tắc ưu tiên khi có cả hai quyền cần chốt. |
| Report | Gọi `requireAdmin()` rồi `REPORT_VIEW`. | Chỉ `requirePermission("REPORT_VIEW")`; vẫn validate dữ liệu/khoảng thời gian. |
| Auth | `requireAdmin()/isAdmin()/isStaff()` vẫn tồn tại; role dùng nội bộ để tra bảng quyền. | Không dùng các hàm role cứng để cấp quyền; quyết định có loại bỏ API cũ hay giữ cho hiển thị cần nhóm chốt. |

**Điểm mâu thuẫn:** [business_rules.md](../overview/business_rules.md) ghi hủy vé “chỉ Admin”. Permission-only cho phép bất kỳ role nào có `TICKET_CANCEL`; file hiện tại chỉ cấp Admin nên hành vi hôm nay giống nhau, nhưng hợp đồng sẽ khác nếu file đổi. Nhóm phải quyết định ràng buộc “chỉ Admin” có phải luật nghiệp vụ độc lập hay được thay bằng Permission, rồi mới bỏ `requireAdmin()` trong code. Không tự sửa tài liệu overview tại bước này.

## 5. Phạm vi dữ liệu, GUI và lỗi

`INVOICE_VIEW_OWN` chỉ cho hóa đơn do người đang đăng nhập lập; Service đối chiếu `invoice.staffId == currentUserId`, giới hạn lịch sử trong ngày nếu luật hiện hành yêu cầu, và không cho `staffId` tham số vượt phạm vi. `INVOICE_VIEW_ALL` cho truy vấn toàn bộ nếu có cặp quyền; GUI không tự lọc rồi coi là an toàn. Các quy tắc về vé SOLD, giờ suất, hoàn tiền, điểm và tham chiếu vẫn do Service kiểm tra dù đã có Permission. GUI bắt exception quyền/dữ liệu để hiển thị; không tự sửa TXT hay thử lại giao dịch lưu lỗi.

## 6. Kiểm thử và chờ xác nhận

- Test quyền được cấp/từ chối với cặp thực tế và bảng tùy biến, user inactive, chưa login, file quyền thiếu/sai/trùng/rỗng; test Service không vượt quyền dù GUI bật nút sai.
- Bổ sung test mã Permission không thuộc danh mục; hiện `RolePermissionRepository::load()` chưa từ chối mã như vậy. Cần chốt nguồn danh mục trước khi sửa parser.
- Test `INVOICE_VIEW_OWN` với owner khác, khoảng ngày và filter `staffId`; test role có `VIEW_ALL` hoặc `TICKET_CANCEL` được thêm trong file tạm để phát hiện gate cứng còn sót.
- **Chờ xác nhận:** xử lý xung đột “chỉ Admin” khi hủy vé; Permission cho các entry point preview/tính giá; ưu tiên `VIEW_ALL`/`VIEW_OWN`; public `load/save`; việc giữ hay loại API role cũ. Không thêm mã Permission trước quyết định nhóm.

Chưa có test mới hoặc thay đổi code trong lượt tài liệu này. Theo [AGENTS.md](../../AGENTS.md), thiết kế và code đầy đủ phải được **CHỐT MARKDOWN** trước refactor; `docs/src/` chỉ cập nhật sau khi test thành công.
