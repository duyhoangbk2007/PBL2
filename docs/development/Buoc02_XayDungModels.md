# Bước 02 — Xây dựng Models

## Checklist công việc

- [x] Khảo sát tài liệu, nhật ký và mã nguồn khi bắt đầu Bước 02 (2026-10-11).
- [x] Có đủ tám cặp `.h/.cpp` trong `core/models/`: `Movie`, `Seat`, `Room`, `Showtime`, `User`, `Member`, `Invoice`, `Ticket`.
- [x] Đối chiếu các trường chính của tám model với `docs/data_format.md`; mỗi model giữ dữ liệu, không trực tiếp đọc/ghi file hoặc quyết định quyền.
- [x] Kiểm tra cú pháp C++17 của cả tám file `.cpp` bằng `g++ -std=c++17 -Wall -Wextra -pedantic-errors -fsyntax-only ...` ngày 2026-10-11: đạt, không có cảnh báo.
- [x] Ghi tổng quan vai trò và ý nghĩa các thuộc tính của tám model theo mã nguồn hiện tại (2026-10-11).
- [x] Rà soát lại mã nguồn sau các chỉnh sửa của người học: `Member` không còn `setPoints()` và cả tám header hiện chưa có include guard; kiểm tra cú pháp lại đạt (2026-10-11).
- [x] Hoàn thành phần 2.1 (2026-10-17): thêm `#pragma once` vào cả tám header; thử include mỗi header hai lần trong một đơn vị biên dịch và kiểm tra cú pháp tám file `.cpp` bằng C++17, đều đạt.
- [x] Theo yêu cầu người học, đổi cả tám header sang `#ifndef`/`#define`/`#endif` với macro riêng cho từng file (2026-10-17); kiểm tra include lặp và biên dịch cú pháp lại đều đạt.
- [x] Bỏ comment sau `#endif` ở cả tám header theo yêu cầu người học (2026-10-17); biên dịch cú pháp lại tám `.cpp` đạt, không có cảnh báo.
- [x] Rà soát API sửa dữ liệu của `Member` và `Ticket` với quy tắc bán/hủy vé (2026-10-17); đã xác định rủi ro và phương án đề xuất bên dưới, chưa sửa code.
- [x] Người học chọn tự thêm `Member::setPoints()` và yêu cầu Codex thay năm setter hủy của `Ticket` bằng `markCancelled(...)` (2026-10-17).
- [x] Đã sửa `Ticket.h/.cpp`, kiểm tra cú pháp tám `.cpp` và liên kết/chạy một ví dụ nhỏ kiểm tra `markCancelled(...)`: đạt (2026-10-17).
- [ ] Kiểm tra `Member::setPoints()` sau khi người học thêm; xác định cách Service sẽ kiểm tra điểm và lưu cùng vé khi tới bước nghiệp vụ.
- [ ] Liên kết và chạy thử một chương trình nhỏ dùng constructor/getter của các model; ghi kết quả thực tế.
- [ ] Rà soát các điểm thiết kế còn mở bên dưới rồi mới xác nhận hoàn thành Bước 02.

**Tiến độ và quyết định đã chốt:** Model liên kết bằng mã (`Seat.roomId` → `Room.roomId`, `Ticket.invoiceId` → `Invoice.invoiceId`, `Ticket.showtimeId` → `Showtime.showtimeId`). `User` dùng một `enum class UserRole` cho ADMIN/STAFF. Trong thân hàm thành viên, truy cập thuộc tính qua `this->`; danh sách khởi tạo constructor dùng tên thuộc tính trực tiếp. Theo quyết định của người dùng, phiên bản môn học lưu `User.password` dạng văn bản và dùng `double` cho các khoản tiền trong `Invoice`/`Ticket`; điểm vẫn là `int`. Tiền giảm giữ phần thập phân. Các quyết định này đã được cập nhật trong `docs/data_format.md` và `docs/business_rules.md`; trước đó dự án từng dự kiến password hash và tiền nguyên 64-bit nhưng không còn áp dụng. `Ticket.h` có chú thích cho 14 thuộc tính.

**File liên quan:** `core/models/{Movie,Seat,Room,Showtime,User,Member,Invoice,Ticket}.h/.cpp`; `docs/data_format.md`; `docs/business_rules.md`. Trong Bước 02, người học và Codex đã cùng tạo, rà soát và chỉnh các model; nhật ký này tóm tắt trạng thái hiện hành để dễ khôi phục ngữ cảnh.

**Cập nhật sau chỉnh sửa của người học (2026-10-11):** `Member.h/.cpp` không còn `setPoints()`, nên mã gọi bên ngoài không thể sửa trực tiếp điểm qua setter này. Cả tám header hiện không có `#pragma once`/include guard, kể cả `Invoice.h` và `Ticket.h` trước đây từng có. Nội dung thuộc tính và kiểu dữ liệu vẫn khớp phần tổng quan dưới đây. Đã chạy lại `g++ -std=c++17 -Wall -Wextra -pedantic-errors -fsyntax-only` cho tám file `.cpp`: đạt, không có cảnh báo. Chưa liên kết hoặc chạy chương trình.

**Cập nhật phần 2.1 (2026-10-17):** Đã thêm `#pragma once` ở đầu `Movie.h`, `Seat.h`, `Room.h`, `Showtime.h`, `User.h`, `Member.h`, `Invoice.h`, `Ticket.h`; không thay đổi thuộc tính hoặc hàm. Kiến thức: khi nhiều đường `#include` dẫn tới cùng một header, class/enum trong header có thể bị định nghĩa lại trong một đơn vị biên dịch. `#pragma once` yêu cầu trình biên dịch xử lý nội dung header đó một lần trong đơn vị biên dịch này. Đã thử include từng header hai lần trong cùng một nguồn đưa qua stdin cho `g++ -std=c++17 -Wall -Wextra -pedantic-errors -x c++ -fsyntax-only -`: đạt. Biên dịch cú pháp lại cả tám `.cpp` với cùng tùy chọn C++17: đạt, không có cảnh báo. Chưa liên kết hoặc chạy chương trình.

**Điều chỉnh cách bảo vệ header (2026-10-17):** Người học chọn cú pháp chuẩn `#ifndef`/`#define`/`#endif`, nên đã thay `#pragma once` trong cả tám file `.h`. Mỗi file dùng macro riêng dạng `PBL2_CORE_MODELS_<TÊN_MODEL>_H`; `#endif` đặt sau toàn bộ class và enum của file. Không đổi thuộc tính, hàm hoặc `.cpp`. Kiến thức: lần include đầu định nghĩa macro; các lần include sau bỏ qua nội dung trong cặp điều kiện, tránh định nghĩa lại class/enum. Thử include mỗi header hai lần trong cùng một đơn vị biên dịch bằng `g++ -std=c++17 -Wall -Wextra -pedantic-errors -x c++ -fsyntax-only -`: đạt. Kiểm tra cú pháp lại cả tám `.cpp` với C++17 và các tùy chọn cảnh báo trên: đạt, không có cảnh báo. Chưa liên kết/chạy chương trình.

**Chỉnh cách trình bày (2026-10-17):** Bỏ phần comment `// PBL2_CORE_MODELS_<TÊN_MODEL>_H` sau `#endif` ở cả tám header; giữ nguyên các macro `#ifndef`/`#define`. Kiểm tra cú pháp lại tám file `.cpp` bằng `g++ -std=c++17 -Wall -Wextra -pedantic-errors -fsyntax-only`: đạt, không có cảnh báo. Chưa liên kết/chạy chương trình.

**Rà soát mục 2.2 (2026-10-17, chỉ đọc và phân tích, chưa sửa code):**

- `Member.h/.cpp`: chỉ có setter cho họ tên và số điện thoại; `points` không có setter. Điều này phù hợp với quy tắc không cho sửa điểm tùy ý. Tuy vậy, Service bán/hủy vé vẫn phải tạo ra số dư mới và Repository phải lưu số dư ấy. Phương án đơn giản cho dự án hiện tại: Service kiểm tra giao dịch, tính `newPoints = oldPoints + pointsEarned` khi bán hoặc `oldPoints - pointsEarned` khi hủy, chặn số âm/tràn số; sau đó tạo một đối tượng `Member` mới giữ nguyên mã/tên/điện thoại và mang `newPoints`. Repository lưu bản cập nhật cùng vé/hóa đơn theo một giao dịch. Constructor phục vụ việc khôi phục dữ liệu và tạo bản cập nhật, không thay thế kiểm tra nghiệp vụ của Service.
- `Ticket.h/.cpp`: năm setter `setStatus`, `setCancelledAt`, `setCancelledBy`, `setCancelReason`, `setRefundAmount` đang công khai. Có thể gọi riêng `setStatus(CANCELLED)` và để trống thông tin hủy, trái với `docs/data_format.md`. Phương án đề xuất: sau khi Service xác minh Admin, vé còn SOLD, suất chưa bắt đầu và điểm đủ thu hồi, gọi **một thao tác model** như `markCancelled(cancelledAt, cancelledBy, cancelReason)` để đặt đồng thời `status = CANCELLED`, ba trường hủy và `refundAmount = paidAmount`; bỏ năm setter riêng nếu không còn nơi dùng. Constructor đầy đủ vẫn cần cho việc đọc bản ghi lịch sử từ file. Model chỉ giữ trạng thái nội tại nhất quán; kiểm tra quyền, thời gian, tham chiếu và lưu nhiều file là trách nhiệm của Service/Repository.
- Theo `docs/business_rules.md`, bán vé phải lưu hóa đơn, vé, điểm và bộ đếm mã cùng giao dịch; hủy vé phải lưu trạng thái vé và số điểm thu hồi cùng giao dịch. Không cập nhật `Member.points` hay `Ticket.status` trong dữ liệu đang dùng trước khi giao dịch lưu thành công. Bản PBL2 chưa có đổi điểm; điểm bằng tổng `pointsEarned` của các vé SOLD thuộc thành viên.
- Giới hạn cần hiểu: getter/setter/constructor của model không phải cơ chế phân quyền bảo mật. C++ vẫn cho phép thay cả đối tượng qua phép gán mặc định hoặc tạo đối tượng mới; chỉ luồng Service/Repository và việc kiểm tra lúc đọc dữ liệu mới bảo đảm quy tắc nghiệp vụ. Chưa có Service/Repository nên phương án trên chưa được triển khai hoặc kiểm thử.

**Triển khai theo lựa chọn của người học (2026-10-17):** Người học sẽ tự thêm `Member::setPoints()`, nên phương án tạo lại `Member` ở đoạn rà soát trên là đề xuất lịch sử, không còn là hướng dự kiến. Codex đã thay năm setter hủy riêng trong `Ticket.h/.cpp` bằng `markCancelled(cancelledAt, cancelledBy, cancelReason)`. Hàm đặt đồng thời `status = CANCELLED`, ba thông tin hủy và `refundAmount = paidAmount`; giữ nguyên `pointsEarned` và giá vé ban đầu. Kiến thức: gom các trường liên quan vào một thao tác giúp tránh gọi thiếu setter; model vẫn không tự kiểm tra quyền Admin, thời điểm hủy, vé đã hủy trước đó, dữ liệu đầu vào hoặc giao dịch lưu nhiều file. Service phải kiểm tra các điều kiện này trước khi gọi, cập nhật điểm thành viên và chỉ công bố trạng thái mới sau khi lưu giao dịch thành công. Kiểm tra thực tế: `g++ -std=c++17 -Wall -Wextra -pedantic-errors -fsyntax-only` trên cả tám `.cpp` đạt, không có cảnh báo; liên kết `Ticket.cpp` với một chương trình tạm tạo vé SOLD, gọi `markCancelled(...)` rồi kiểm tra status, ba trường hủy, refund, paidAmount và pointsEarned: chạy thoát mã 0. Chương trình tạm không được giữ trong dự án. Chưa kiểm tra liên kết/chạy cả tám model cùng nhau hoặc luồng Service/Repository.

### Tổng quan tám model và thuộc tính

Các model giữ dữ liệu trong bộ nhớ. Mã như `movieId`, `roomId`, `invoiceId` là cách liên kết đối tượng; bản thân model chưa tự tìm đối tượng liên quan, đọc file hay kiểm tra toàn bộ quy tắc nghiệp vụ.

**`Movie` — thông tin một phim** (`movies.txt`):

- `movieId`: mã phim bất biến; `title`: tên phim; `genre`: thể loại.
- `durationMinutes`: thời lượng tính bằng phút; dùng để tính giờ kết thúc suất chiếu lúc tạo/sửa suất. `minAge`: tuổi tối thiểu của người xem.
- `releaseDate`, `endDate`: ngày đầu và cuối của khoảng phim được chiếu, dạng `YYYY-MM-DD`; khác với giờ bắt đầu/kết thúc của từng suất.
- `active`: phim còn được quản lý là đang hoạt động hay đã ngừng hoạt động.

**`Seat` — một vị trí ghế thuộc phòng** (`seats.txt`):

- `roomId`: mã phòng chứa ghế; phải tìm được `Room` cùng mã. `seatId`: mã ghế trong phòng, ví dụ `A01`; cặp `(roomId, seatId)` xác định ghế.
- `row`, `column`: hàng và cột của vị trí trong sơ đồ phòng, bắt đầu từ 1.
- `seatType`: loại ghế `STANDARD`, `VIP` hoặc `COUPLE`. Ghế đôi là một vị trí logic cho hai người. `Seat` không lưu trạng thái đã bán vì trạng thái đó phụ thuộc từng suất chiếu.

**`Room` — thông tin một phòng chiếu** (`rooms.txt`):

- `roomId`: mã phòng bất biến; `name`: tên hiển thị của phòng.
- `rowCount`, `columnCount`: kích thước lưới sơ đồ ghế. Tích của hai số là số ô tối đa, không nhất thiết là số ghế thực tế vì có thể có ô trống.
- `active`: phòng có đang hoạt động hay không. Danh sách ghế nằm ở dữ liệu `Seat`, không lưu lặp số ghế trong `Room`.

**`Showtime` — một suất chiếu cụ thể** (`showtimes.txt`):

- `showtimeId`: mã suất; `movieId`: mã phim được chiếu; `roomId`: mã phòng sử dụng.
- `startAt`, `endAt`: thời điểm bắt đầu/kết thúc dạng `YYYY-MM-DDTHH:mm:ss`. `endAt` được chốt theo thời lượng phim khi tạo/sửa suất.
- `status`: `SCHEDULED` (đã lên lịch) hoặc `CANCELLED` (đã hủy). Suất đã kết thúc được suy ra từ thời gian, không có enum riêng.

**`User` — tài khoản người dùng hệ thống** (`users.txt`):

- `userId`: mã tài khoản bất biến; `username`: tên dùng để đăng nhập, phải duy nhất.
- `password`: mật khẩu văn bản theo quyết định hiện tại của dự án môn học; chưa có cơ chế hash.
- `fullName`: họ tên người dùng; `role`: `ADMIN` hoặc `STAFF`, quyết định quyền ở tầng Service; `active`: tài khoản còn được phép đăng nhập hay không.

**`Member` — khách có thông tin thành viên** (`members.txt`):

- `memberId`: mã thành viên bất biến; `fullName`: họ tên; `phone`: số điện thoại giữ dưới dạng chuỗi để không mất số `0` đầu và phải duy nhất.
- `points`: số điểm hiện có. Điểm phát sinh từ vé bán/hủy, không phải số người dùng được tự sửa tùy ý.

**`Invoice` — một lần bán tại quầy** (`invoices.txt`):

- `invoiceId`: mã hóa đơn; `showtimeId`: suất chung của mọi vé trong hóa đơn.
- `staffId`: `userId` của người thực sự bán, có thể là Staff hoặc Admin; `memberId`: mã thành viên gắn với hóa đơn, để rỗng nếu không có.
- `createdAt`: thời điểm lập hóa đơn; `totalAmount`: tổng `paidAmount` ban đầu của các vé, dùng `double` và không đổi khi một vé về sau bị hủy. Vé tham chiếu hóa đơn qua `invoiceId`, hóa đơn không lưu lặp danh sách mã vé.

**`Ticket` — một vé cho một vị trí ghế trong suất chiếu** (`tickets.txt`):

- `ticketId`: mã vé; `invoiceId`: mã hóa đơn chứa vé; `showtimeId`: mã suất, phải trùng suất của hóa đơn; `seatId`: ghế trong phòng của suất đó. Phòng được suy ra qua `Showtime.roomId`.
- `ticketType`: `REGULAR` hoặc `STUDENT`, độc lập với `Seat.seatType`.
- `basePrice`: giá gốc; `discountAmount`: tiền giảm; `paidAmount`: tiền thực trả = giá gốc − tiền giảm. Ba khoản dùng `double` và được giữ theo giá lúc bán.
- `pointsEarned`: điểm cộng ban đầu của vé, giữ nguyên trong lịch sử kể cả khi hủy; bằng 0 nếu hóa đơn không gắn thành viên.
- `status`: `SOLD` hoặc `CANCELLED`. `cancelledAt`: thời điểm hủy; `cancelledBy`: `userId` của Admin hủy; `cancelReason`: lý do hủy. Ba chuỗi hủy để rỗng khi vé còn `SOLD`.
- `refundAmount`: tiền hoàn dùng `double`; bằng 0 khi `SOLD`, bằng `paidAmount` khi hủy hợp lệ.

## Lỗi và vướng mắc

- Chưa thấy lỗi cú pháp C++17 ở tám file `.cpp`, nhưng **chưa liên kết hoặc chạy kiểm thử hành vi**. `main.cpp` hiện rỗng; `CMakeLists.txt` hiện không có trong cây làm việc, nên chưa có bản build dự án được xác nhận.
- Vướng mắc include lặp từng được ghi nhận ngày 2026-10-11 đã được xử lý ngày 2026-10-17; header hiện dùng `#ifndef`/`#define`/`#endif` và phép thử include lặp đã đạt.
- `Member::setPoints()` hiện chưa có trong mã; người học dự định tự thêm. Khi triển khai bán/hủy vé, Service phải kiểm tra điểm mới không âm/không tràn và lưu cùng giao dịch. Không cho UI sửa điểm tùy ý.
- `Ticket::markCancelled(...)` đã thay năm setter hủy, nhưng hàm hiện giả định Service đã xác minh vé SOLD, người hủy, thời gian và thông tin hủy hợp lệ. Gọi trực tiếp với dữ liệu sai hoặc gọi hủy lần hai vẫn có thể tạo dữ liệu không hợp lệ; cần chặn ở Service khi triển khai nghiệp vụ.
- `Showtime::isEndAfterStart()` so sánh chuỗi, chỉ đáng tin khi thời điểm đúng dạng `YYYY-MM-DDTHH:mm:ss`; chưa có kiểm tra ngày giờ hợp lệ. Điều kiện trùng lịch, tham chiếu mã, quyền và dữ liệu duy nhất thuộc các bước Service/Repository.
- Với tiền `double`, chưa chốt cách ghi/đọc đủ chính xác và cách so sánh tổng tiền khi triển khai Repository/Service. Không nên dùng `==` cho các tổng tiền tính toán.

## Các phần tiếp theo cần làm

1. **Hoàn thiện header — đã xong 2026-10-17:** cả tám header hiện dùng `#ifndef`/`#define`/`#endif`; phép thử include lặp và kiểm tra cú pháp `.cpp` đều đạt.
2. **Rà soát quyền sửa dữ liệu — đã triển khai phần Ticket:** `Ticket` hiện có `markCancelled(...)` thay năm setter hủy. Người học sẽ tự thêm `Member::setPoints()`; khi có, rà soát chữ ký và việc dùng setter qua Service. Chưa viết Service/Repository trong Bước 02.
3. **Kiểm chứng:** viết/chạy ví dụ nhỏ để liên kết tám file `.cpp`, tạo model và đọc một số trường đại diện; ghi lệnh, kết quả và lỗi thực tế. Việc này không thay thế bản build CMake của Bước 01.
4. **Kết thúc Bước 02:** đối chiếu lại schema và kiểu dữ liệu hiện hành, cập nhật checklist và `TongQuanTienDo.md` nếu trạng thái thay đổi. Chờ người dùng yêu cầu trước khi chuyển sang Bước 03.

**Hướng dẫn cụ thể để tiếp tục (2026-10-17):** Nên làm lần lượt các phần nhỏ dưới đây. Sau mỗi phần, kiểm tra kết quả rồi cập nhật nhật ký; chưa coi Bước 02 hoàn thành chỉ vì các file model đã tồn tại.

- **2.1 — Bảo vệ header.** Tìm hiểu mục đích của `#pragma once` hoặc cặp `#ifndef`/`#define`. Thêm một trong hai cách vào từng file `core/models/{Movie,Seat,Room,Showtime,User,Member,Invoice,Ticket}.h`. Với mỗi header, thử include nó hai lần trong một file kiểm tra tạm: không được báo lỗi định nghĩa lại class/enum. Sau đó biên dịch cú pháp tám `.cpp` với C++17. Đây là phần nên làm tiếp theo vì header sẽ được nhiều file khác sử dụng.
- **2.2 — Chốt cách sửa dữ liệu nhạy cảm.** Xem lại `Member.h/.cpp` và `Ticket.h/.cpp`: điểm chỉ thay đổi do nghiệp vụ bán/hủy vé; các trường hủy của vé phải nhất quán với `status`. Ghi rõ thao tác nào thuộc Service ở các bước sau và model cần cung cấp thao tác tối thiểu nào. Chưa cần viết Service hay chức năng bán vé trong Bước 02.
- **2.3 — Chạy thử model.** Tạo một file kiểm tra nhỏ trong vị trí phù hợp (không dùng `main.cpp` đang dành cho Bước 01 nếu chưa thống nhất). Khởi tạo ít nhất một đối tượng của mỗi model, gọi vài getter và kiểm tra giá trị nhận được; đặc biệt thử mã liên kết giữa `Room`–`Seat` và `Showtime`–`Ticket`–`Invoice`. Biên dịch và **liên kết** file kiểm tra với cả tám `.cpp`, chạy chương trình, rồi ghi lệnh và kết quả thực tế. Kiểm tra cú pháp đơn lẻ chưa xác nhận được bước này.
- **2.4 — Đối chiếu cuối bước.** So thuộc tính, kiểu dữ liệu và enum với `docs/data_format.md`/`docs/business_rules.md`; ghi rõ các giới hạn còn để cho Repository/Service (tham chiếu mã, ngày giờ, số tiền). Chỉ khi phần kiểm chứng đạt và các quyết định cần cho model đã rõ mới cập nhật trạng thái Bước 02 trong `TongQuanTienDo.md`.

**Cập nhật 2026-10-17:** Đã hoàn thành 2.1 và thay API hủy của `Ticket` theo mục 2.2; đang chờ người học thêm `Member::setPoints()` để rà soát tiếp. Sau đó là kiểm chứng liên kết/chạy cả tám model ở 2.3 và đối chiếu cuối bước ở 2.4.
