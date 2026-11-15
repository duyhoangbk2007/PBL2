# AGENTS.md — Quy ước cộng tác và viết code PBL2

> Áp dụng cho thành viên dự án (Hoàng, Kiệt), AI coding agents và người review. Đặt file này ở **gốc repository**. Đọc trước khi viết, sửa hoặc review code. Đây là quy ước phối hợp; không tự động thay đổi code hiện có.

## 1. Nguyên tắc ưu tiên

1. **Mã nguồn thực tế trên nhánh đích và API đã thống nhất** là căn cứ để tích hợp. Đọc file `.h`, `.cpp` và các test liên quan trước khi viết code.
2. `business_rules.md`, `data_format.md`, `README.md` mô tả nghiệp vụ và dữ liệu mục tiêu. Một số tài liệu cũ còn mô tả cấu trúc `core/repositories`, ID 6 chữ số hoặc `#schema=1`, **khác với cách triển khai hiện tại**. Không tự ý chuyển kiến trúc hoặc định dạng chỉ vì tài liệu cũ; ghi nhận khác biệt và thống nhất với nhóm trước.
3. Khi yêu cầu, tài liệu và code mâu thuẫn, **không âm thầm đoán hoặc sửa hàng loạt**. Nêu rõ điểm mâu thuẫn, đề xuất một phương án, chờ xác nhận trước khi thay đổi hợp đồng dữ liệu/API.
4. Ưu tiên code C++ dễ đọc, phù hợp trình độ môn học. Không thêm thư viện, template, lớp trừu tượng hoặc kiến trúc mới khi chưa cần.
5. Chỉ sửa đúng phạm vi task; không tự ý sửa file của người khác, đổi tên public API, định dạng TXT, hoặc refactor các module không liên quan.

## 2. Cấu trúc dự án đang sử dụng

- `src/models/`: thực thể (`User`, `Movie`, `Room`, `Seat`, `Showtime`, ...), thuộc tính, getter/setter, liên kết giữa thực thể.
- `src/data_structures/`: cấu trúc dữ liệu tự cài.
- `src/data_access/`: `Repository<T>` và repository cụ thể, parse/serialize, kiểm tra dữ liệu, CRUD.
- `src/business/`: service và nghiệp vụ, phân quyền qua `AuthService`.
- `src/utils/`: `Validator`, `FileIO` và tiện ích chung.
- `tests/`: các chương trình kiểm thử riêng.
- `docs/` hoặc các file `.md` hiện có: tài liệu; xác minh vị trí thực tế trước khi sửa.

**Không tạo lại `core/repositories/` hay `core/services/`** chỉ vì thấy trong tài liệu thiết kế cũ.

## 3. Phân chia trách nhiệm

### Model
- Lưu dữ liệu, cung cấp getter/setter và quan hệ đối tượng; không trực tiếp đọc/ghi TXT, không kiểm tra quyền người dùng.
- ID là khóa định danh; không tự ý thêm setter đổi ID.
- Quan hệ `Room`–`Seat`, `Room`–`Showtime` phải giữ nhất quán hai chiều; không liên kết trùng đối tượng, trùng vị trí hoặc sai phòng.

### Repository cụ thể
- Chứa `Repository<T> repo` theo mẫu `MovieRepository`, `UserRepository`, `RoomRepository`.
- Chịu trách nhiệm parse/serialize một dòng TXT, kiểm tra định dạng và tính toàn vẹn dữ liệu, CRUD, bảo vệ các ràng buộc dữ liệu cơ bản.
- Không kiểm tra quyền Admin/Staff, không xử lý đăng nhập, không gọi giao diện.
- Tận dụng `Repository<T>` cho `load`, `save`, `add`, `findById`, `getAt`, `count`, `removeById` khi phù hợp; không tự viết lại cơ chế đọc/ghi file.
- `load()` không nạp lại khi đã loaded, trừ khi nhóm chủ động thống nhất cơ chế reload.
- Không tự động `save()` sau từng thao tác nếu kiến trúc hiện tại yêu cầu gọi lưu riêng.

### Service
- Nhận repository và `AuthService` bằng **tham chiếu** trong constructor (theo `MovieService`/`RoomService`).
- Kiểm tra quyền **trước** khi thao tác; dùng `authService.requireAdmin()` cho thêm/sửa/xóa danh mục và `requireLogin()` cho truy vấn cần đăng nhập.
- `requireAdmin()` / `requireLogin()` có thể **ném exception**; không đổi thành `false` hoặc nuốt exception nếu chưa thống nhất.
- Kiểm tra quy tắc nghiệp vụ cần phối hợp dữ liệu, rồi gọi repository; không lặp lại mọi kiểm tra định dạng của repository.
- Hàm chỉ đọc ưu tiên trả `const T*` như `MovieService`, `RoomService`; không cấp quyền sửa Model trực tiếp qua service.
- Các quy tắc `active` và xóa/sửa dữ liệu đã tham chiếu phải nhất quán với service tương ứng; không tự tạo API kích hoạt mới khi chưa thống nhất.

## 4. Quy ước đặt tên và viết code

- Lớp/file: `PascalCase` (`SeatRepository.h`, `SeatService.cpp`).
- Hàm/biến: `camelCase` (`findSeatById`, `roomRepo`, `newValues`).
- Getter: `getXxx()`; boolean: `isXxx()`; thao tác: `addXxx`, `updateXxx`, `removeXxx`, `findXxxById`, `getXxxAt`, `getXxxCount` ở tầng Service.
- Repository: `load`, `save`, `add`, `update`, `removeById`, `findById`, `getAt`, `count`, `isLoaded` khi nghiệp vụ cho phép.
- Hàm chuyển đổi TXT theo mẫu đang dùng: `stringToSeat(const string&)`, `seatToString(Seat*)`; callback lấy ID đặt tên rõ nghĩa. **Lưu ý:** Seat có khóa ghép, xem mục 6.
- Định nghĩa hàm không-template ở `.cpp`, khai báo ở `.h`; template `Repository<T>` giữ định nghĩa trong header theo code hiện tại.
- Sử dụng `const string&` cho chuỗi đầu vào; `const T& newValues` khi cập nhật; kiểm tra `nullptr` trước khi truy cập.
- Giữ ngoặc nhọn cho `if`, `for`, `while`; thụt lề 4 dấu cách; thông báo lỗi và comment ngắn, giải thích *vì sao* thay vì nhắc lại code.
- Dùng `Validator::isValidId`, `Validator::isValidText`, `Validator::isValidDate` khi phù hợp; không sao chép lại thuật toán validate đã có.
- Chỉ thêm `#include` thực sự cần thiết; không đưa `using namespace std;` vào header mới nếu có thể tránh.

## 5. Quy tắc dữ liệu và sở hữu con trỏ

- TXT: một bản ghi mỗi dòng, các trường phân tách bằng `|`; parse phải kiểm tra **đúng số trường**, số nguyên phải parse hết chuỗi, boolean chỉ `0`/`1`, enum phải đúng giá trị được định nghĩa.
- Khi dữ liệu TXT không hợp lệ, parse báo lỗi rõ ràng; không giữ lại đối tượng cấp phát lỗi.
- Repository đang sở hữu các đối tượng `T*` được **thêm thành công** và giải phóng khi xóa/hủy; caller chịu trách nhiệm giải phóng con trỏ nếu `add()` thất bại.
- `Room` chỉ giữ các con trỏ liên kết đến `Seat`/`Showtime`, không mặc định sở hữu chúng. Không `delete` đối tượng liên kết qua `Room`.
- Không để con trỏ liên kết trỏ đến đối tượng đã bị xóa/reload. Trước khi xóa hoặc reload đối tượng có quan hệ, phải có chiến lược kiểm tra, tháo liên kết hoặc chặn thao tác; không âm thầm bỏ qua.
- Khi `load()` nhiều repository, liên kết dữ liệu **sau khi các đối tượng liên quan đã được nạp**; xử lý ID tham chiếu không tồn tại một cách rõ ràng.
- Không thay đổi quy ước ID hiện hành (ví dụ `ROM0001`, `MOV0001`, `USR0001`) hoặc tự đưa `#schema=1` vào file khi chưa có quyết định thống nhất về migration.

## 6. Hợp đồng bắt buộc khi làm Seat (B06)

### Model và định dạng
- `Seat` hiện có: `roomId`, `seatId`, `row`, `column`, `seatType`, con trỏ `Room* room`.
- `SeatType`: `STANDARD`, `VIP`, `COUPLE`.
- Định dạng mục tiêu `seats.txt`: `roomId|seatId|row|column|seatType`.
- `row`/`column` bắt đầu từ 1; ghế phải nằm trong kích thước phòng.
- Không lưu trạng thái ghế đã bán trong `Seat`: trạng thái bán phụ thuộc **suất chiếu và vé**.
- Ghế đôi (`COUPLE`) là **một seatId, một vị trí logic**; không tách thành hai ghế/ID.

### Khóa và ràng buộc
- `seatId` chỉ duy nhất **trong một phòng**, không bắt buộc duy nhất toàn hệ thống.
- Khóa ghế là **(`roomId`, `seatId`)**. Vị trí (`roomId`, `row`, `column`) cũng phải duy nhất.
- **Không dùng `seatId` đơn lẻ làm ID toàn cục trong `Repository<Seat>`**. Trước khi viết `SeatRepository`, thống nhất cách biểu diễn khóa ghép (ví dụ callback `getId` tạo khóa không nhập nhằng) và chữ ký `findById`/`removeById`; không tự quyết định API một phía.
- Khi thêm ghế, cần xác minh phòng tồn tại và tọa độ hợp lệ; khi đổi loại ghế, không thay đổi khóa hoặc vị trí nếu API không cho phép.
- Không xóa hoặc thay đổi mã ghế đã từng xuất hiện trong vé; chặn thay đổi sơ đồ khi có suất tương lai chưa hủy từng phát sinh vé, theo `business_rules.md`. Nếu các repository vé/suất chưa hoàn thiện, ghi rõ quy tắc **chưa thể kiểm chứng đầy đủ**, không giả vờ đã bảo đảm.
- Các thao tác liên kết `Seat::linkRoom()` và `Room::linkSeat()` phải thống nhất; tránh thêm vào danh sách hai lần và tránh liên kết nửa chừng.

### Phân quyền dự kiến
- Admin quản lý phòng/sơ đồ ghế; Staff chỉ xem dữ liệu cần cho bán vé.
- Service kiểm tra quyền; repository không phân quyền.
- Trước khi code `SeatService`, thống nhất API chính thức và các trường hợp xóa/sửa ghế có vé tham chiếu.

## 7. Mẫu API và quy trình tạo module mới

1. Đọc Model, repository/service tương tự và tài liệu nghiệp vụ.
2. Đề xuất `.h` **trước**: constructor, thuộc tính, chữ ký hàm, quyền, sở hữu con trỏ, khóa định danh.
3. Đối chiếu API với module đang dùng; trao đổi với thành viên còn lại **trước khi** code `.cpp`.
4. Triển khai lần lượt: helper validate → parse/serialize → load/save → CRUD → service/permissions → test.
5. Không tạo API hoặc helper trùng với `Validator`/`Repository<T>` khi đã có cách dùng chung.
6. Test trường hợp đúng, dữ liệu sai, trùng khóa/vị trí, chưa load, quyền Admin/Staff/chưa đăng nhập, lưu/đọc lại, và quan hệ liên kết.
7. Cập nhật tài liệu mô tả **đúng code đã triển khai**, không ghi chức năng tương lai là đã hoàn thành.

## 8. Biên dịch và kiểm thử

- Mỗi file test có `main()` riêng; **không** biên dịch nhiều file test cùng lúc.
- Trong Git Bash, biên dịch toàn bộ `.cpp` trong `src` nhưng loại `main.cpp` để tránh `multiple definition of main`:

```bash
g++ -std=c++17 -I src tests/test_room_service.cpp $(find src -name "*.cpp" ! -name "main.cpp") -o test_room_service.exe
./test_room_service.exe
```

- Khi test Seat, thay đúng tên file test và tên file `.exe`.
- Không kết luận PASS chỉ từ việc code trông đúng; cần kết quả chạy thực tế.
- Test dùng file TXT tạm, không ghi đè dữ liệu thật; dọn file sau test.
- Test phân quyền phải kiểm tra exception từ `requireAdmin`/`requireLogin`.

## 9. Quy trình Git và cộng tác Hoàng–Kiệt

- Mỗi người làm trên nhánh riêng theo quy ước đang sử dụng; không commit trực tiếp vào `main` nếu chưa được thống nhất.
- Trước khi bắt đầu: cập nhật nhánh, đọc `.h` và thay đổi mới của người kia; không code dựa trên API cũ.
- Khi thay public API, schema TXT, quan hệ Model hoặc quy tắc nghiệp vụ: thông báo cho người kia và thống nhất trước.
- PR ghi rõ: file đã thay đổi, API mới/đổi, ràng buộc, test đã chạy, điểm còn thiếu.
- Không tự ý sửa code ngoài task, không thay đổi file của người khác chỉ để format, không xóa test cũ để làm test mới PASS.
- Khi review: ưu tiên lỗi thực tế và tính tương thích; nêu file/hàm, nguyên nhân, cách sửa tối thiểu. Không biến vấn đề phong cách thành lỗi bắt buộc.
- Trước khi merge: biên dịch, chạy test liên quan, kiểm tra xung đột, xác minh tài liệu và không làm hỏng các module đã hoàn thành.

## 10. Checklist trước khi bàn giao SeatRepository/SeatService

- [ ] API `.h` đã được hai thành viên thống nhất.
- [ ] Khóa ghép `(roomId, seatId)` hoạt động đúng.
- [ ] Không trùng vị trí `(roomId, row, column)`.
- [ ] Kiểm tra phòng tồn tại, giới hạn hàng/cột và loại ghế hợp lệ.
- [ ] Parse/serialize đúng `seats.txt`; load/save theo mẫu Repository.
- [ ] Liên kết hai chiều Room–Seat nhất quán, không tạo con trỏ treo.
- [ ] Quyền Admin/Staff nằm trong Service.
- [ ] Các ràng buộc cần Ticket/Showtime được kiểm chứng hoặc ghi rõ chưa hoàn thiện.
- [ ] Test compile và PASS thực tế; không ghi đè dữ liệu thật.
- [ ] PR mô tả thay đổi và tài liệu được cập nhật.
