# PermissionDesign — Thiết kế phân quyền RBAC (PBL2)

**Trạng thái:** Đặc tả thiết kế, chưa triển khai code.  
**Nguồn dữ liệu đã chốt:** `role_permissions.txt` (không có dòng schema).  
**Phạm vi:** Chỉ đọc và kiểm tra quyền theo vai trò; **chưa** có API hay giao diện cấp/thu hồi quyền.

## 1. Mục tiêu

- Hai vai trò `ADMIN`, `STAFF` dùng trường `role` của `User` trong `users.txt`.
- Quyền thực hiện nghiệp vụ phụ thuộc các bản ghi trong `role_permissions.txt`, không phụ thuộc kiểm tra vai trò hardcode trong từng Service.
- Backend kiểm tra quyền ở thời điểm thực hiện thao tác; GUI chỉ dùng thông tin quyền để điều chỉnh hiển thị.
- Không quản lý quyền riêng từng tài khoản, không thêm vai trò mới, không cấp/thu hồi quyền khi ứng dụng đang chạy ở phiên bản này.

## 2. File dữ liệu

Đường dẫn dự kiến: `data/role_permissions.txt` (đường dẫn thực tế truyền qua constructor/cấu hình).

```text
role|permissionCode
```

Dòng trên **chỉ là mô tả cấu trúc, không phải header trong file**. File thực tế chứa đúng một cặp trên mỗi dòng, ví dụ:

```text
ADMIN|MOVIE_VIEW
ADMIN|MOVIE_ADD
STAFF|MOVIE_VIEW
```

- UTF-8; mỗi dòng một bản ghi; **không có `#schema=1`**; không có tên cột.
- Role chỉ được là `ADMIN` hoặc `STAFF`.
- `permissionCode` phải nằm trong danh sách 29 mã được hỗ trợ bên dưới; phân biệt chữ hoa/chữ thường.
- Khóa duy nhất là `(role, permissionCode)`; không chấp nhận dòng trùng, dòng thiếu/thừa trường, role hoặc permission không hợp lệ.
- Không có cặp `(role, permissionCode)` nghĩa là **không có quyền**.
- Không được âm thầm bỏ qua dòng lỗi; khi nạp lỗi, báo lỗi và không cho phép thực hiện nghiệp vụ được bảo vệ.
- File rỗng hợp lệ về mặt cú pháp nhưng không cấp bất kỳ quyền nào; việc không có quyền đăng nhập hoặc vận hành phải được xử lý rõ, không tự cấp quyền dự phòng.

## 3. Danh sách quyền đã chốt

Tổng cộng **29 mã quyền** và **38 bản ghi** trong file hiện tại (29 Admin, 9 Staff).

| Permission | ADMIN mặc định | STAFF mặc định |
|---|---|---|
| `MOVIE_VIEW` | Có | Có |
| `MOVIE_ADD` | Có | Không |
| `MOVIE_UPDATE` | Có | Không |
| `MOVIE_DELETE` | Có | Không |
| `ROOM_VIEW` | Có | Có |
| `ROOM_ADD` | Có | Không |
| `ROOM_UPDATE` | Có | Không |
| `ROOM_DELETE` | Có | Không |
| `SEAT_VIEW` | Có | Có |
| `SEAT_ADD` | Có | Không |
| `SEAT_UPDATE` | Có | Không |
| `SEAT_DELETE` | Có | Không |
| `SHOWTIME_VIEW` | Có | Có |
| `SHOWTIME_ADD` | Có | Không |
| `SHOWTIME_UPDATE` | Có | Không |
| `SHOWTIME_CANCEL` | Có | Không |
| `SHOWTIME_DELETE` | Có | Không |
| `STAFF_VIEW` | Có | Không |
| `STAFF_ADD` | Có | Không |
| `STAFF_UPDATE` | Có | Không |
| `STAFF_DEACTIVATE` | Có | Không |
| `MEMBER_VIEW` | Có | Có |
| `MEMBER_ADD` | Có | Có |
| `MEMBER_UPDATE` | Có | Có |
| `TICKET_SELL` | Có | Có |
| `TICKET_CANCEL` | Có | Không |
| `INVOICE_VIEW_OWN` | Có | Có |
| `INVOICE_VIEW_ALL` | Có | Không |
| `REPORT_VIEW` | Có | Không |

**Lưu ý triển khai:** Những mã cho Member, Ticket, Invoice, Report và hai thao tác hủy/xóa suất chiếu có thể chưa có nghiệp vụ hoàn chỉnh trên `main`. Khai báo quyền không có nghĩa là chức năng đã sẵn sàng.

## 4. Ánh xạ quyền và nghiệp vụ

- Movie: `MOVIE_VIEW` cho tra cứu; `MOVIE_ADD`, `MOVIE_UPDATE`, `MOVIE_DELETE` cho thêm/sửa/xóa.
- Room: `ROOM_VIEW`, `ROOM_ADD`, `ROOM_UPDATE`, `ROOM_DELETE`.
- Seat: `SEAT_VIEW`, `SEAT_ADD`, `SEAT_UPDATE`, `SEAT_DELETE`.
- Showtime: `SHOWTIME_VIEW`, `SHOWTIME_ADD`, `SHOWTIME_UPDATE`, `SHOWTIME_CANCEL`, `SHOWTIME_DELETE`.
- Staff: `STAFF_VIEW`, `STAFF_ADD`, `STAFF_UPDATE`, `STAFF_DEACTIVATE`.
- Member: `MEMBER_VIEW`, `MEMBER_ADD`, `MEMBER_UPDATE` (API cụ thể đối chiếu khi triển khai Service).
- Ticket: `TICKET_SELL`, `TICKET_CANCEL`.
- Invoice: `INVOICE_VIEW_OWN` (phải kiểm tra người lập và điều kiện ngày/mã theo business rules), `INVOICE_VIEW_ALL`.
- Report: `REPORT_VIEW`.
- `load()`/`save()` là thao tác vòng đời dữ liệu nội bộ, **không** là quyền thao tác GUI.

## 5. Luồng kiểm tra quyền

1. Ứng dụng khởi tạo và nạp `UserRepository`, `RolePermissionRepository` trước khi cung cấp chức năng đăng nhập/nghiệp vụ. Lỗi nạp bảng quyền phải được xử lý theo hướng từ chối truy cập, không mặc định cho phép.
2. `AuthService` xác định tài khoản đang đăng nhập bằng `currentUserId`, tra lại `UserRepository` và kiểm tra `active`.
3. Lấy `role` hiện tại của User; tra `(role, permissionCode)` trong `RolePermissionRepository`.
4. Có bản ghi hợp lệ thì cho phép; không có thì từ chối. Service phải kiểm tra lại mỗi lần gọi nghiệp vụ.
5. `hasPermission(...)` dự kiến trả `bool`; `requirePermission(...)` dự kiến ném ngoại lệ khi chưa đăng nhập/không đủ quyền, tương tự quy ước hiện tại của AuthService. **Chữ ký, loại ngoại lệ và thông điệp lỗi sẽ chốt khi viết contract/code.**

## 6. Ranh giới trách nhiệm

- `RolePermissionRepository`: đọc file, xác thực bản ghi, lưu bảng quyền trong RAM, tra cứu quyền; **chưa cần** `grantPermission`/`revokePermission` hoặc `save()` nếu không có nghiệp vụ ghi.
- `AuthService`: kết hợp phiên đăng nhập, trạng thái User và bảng quyền để kiểm tra quyền hiện hành.
- Các Service: yêu cầu mã quyền theo thao tác; vẫn kiểm tra quy tắc nghiệp vụ và phạm vi dữ liệu riêng.
- Qt GUI: có thể hỏi `hasPermission` để bật/tắt chức năng, nhưng không được coi đó là hàng rào bảo mật.

## 7. Quy tắc an toàn

- Không tự cấp quyền `ADMIN` nếu thiếu bản ghi trong file; quyền của Admin cũng được xác định bởi dữ liệu.
- Không có chức năng Admin cấp quyền cho Staff trong phiên bản này; sửa file ngoài ứng dụng chỉ có hiệu lực sau khi nạp lại/khởi động lại.
- `INVOICE_VIEW_OWN` không cho phép xem hóa đơn người khác; kiểm tra phạm vi dữ liệu vẫn bắt buộc.
- `TICKET_CANCEL` phải đồng thời tuân thủ quy tắc hủy vé trong `business_rules.md` (hiện tài liệu quy định chỉ Admin); không thể dùng việc thêm một dòng quyền để bỏ qua quy tắc nghiệp vụ mà chưa cập nhật đặc tả.
- Không bỏ qua lỗi định dạng hoặc dùng bảng quyền nạp dở.

## 8. Tác động đến tài liệu hiện hành

- `data_format.md` hiện ghi **mọi file** phải bắt đầu bằng `#schema=1`; cần bổ sung **ngoại lệ rõ ràng** cho `role_permissions.txt`, không xóa quy tắc schema của các file khác.
- `business_rules.md` hiện mô tả quyền theo vai trò; cần ghi rõ quyền thao tác được tra từ bảng quyền, trong khi các giới hạn nghiệp vụ như hủy vé và xem hóa đơn của mình vẫn có hiệu lực.
- Backend–Frontend Contract cần ghi mã permission của từng hàm, kết quả khi không có quyền, và API kiểm tra quyền của người dùng hiện tại.

## 9. Tiêu chí nghiệm thu giai đoạn thiết kế

- [x] Có danh sách 29 permission, đúng với file dữ liệu đã chốt.
- [x] Có ma trận quyền mặc định Admin/Staff.
- [x] Quy định file không có dòng schema.
- [x] Có quy tắc từ chối truy cập khi dữ liệu quyền lỗi hoặc thiếu.
- [x] Không đưa chức năng cấp/thu hồi quyền vào phạm vi hiện tại.
- [ ] Đồng bộ `data_format.md` và `business_rules.md` trong repository.
- [ ] Xác nhận API/ngoại lệ cụ thể khi bắt đầu giai đoạn code.
