# AGENTS.md — Quy chuẩn phát triển PBL2

Áp dụng cho Hoàng, Kiệt và Codex khi thiết kế, viết code, kiểm thử và cập nhật tài liệu. Các quy định dưới đây là kiến trúc và quy trình mục tiêu; không mặc định code cũ đã tuân thủ.

## 1. Nguồn đối chiếu và phạm vi

- Đọc yêu cầu, code, test và tài liệu liên quan trước khi thực hiện.
- Đối chiếu `README.md`, `docs/overview/business_rules.md` và `docs/overview/data_format.md`.
- Phân biệt code hiện tại với thiết kế dự kiến. Không tự ý thay đổi API, schema, quy tắc nghiệp vụ hoặc cấu trúc thư mục.
- Khi có mâu thuẫn, báo cáo và thống nhất trước khi sửa.
- Chỉ thay đổi những file thuộc phạm vi yêu cầu.
- Ưu tiên C++17 đơn giản, dễ đọc; tránh thêm lớp, thư viện hoặc kỹ thuật phức tạp không cần thiết.

## 2. Kiến trúc và trách nhiệm

- **Models:** Quản lý thuộc tính, trạng thái và quan hệ đối tượng; không đọc/ghi file hoặc phân quyền.
- **Data Structures:** Chứa cấu trúc dữ liệu tự cài đặt, đặc biệt `DoublyLinkedList<T>`.
- **Utils:** Cung cấp các tiện ích dùng chung như Validator, FileIO, IdGenerator và DateTime.
- **Data Access:** Thực hiện CRUD, load/save, kiểm tra dữ liệu cơ bản, liên kết đối tượng và bảo đảm tính toàn vẹn dữ liệu.
- **Business:** Service kiểm tra Permission, xử lý nghiệp vụ và điều phối Repository.
- **GUI:** Hiển thị, tiếp nhận thao tác và gọi Service; không trực tiếp xử lý nghiệp vụ hoặc đọc/ghi TXT.
- **Tests:** Kiểm thử các thành phần và chức năng hệ thống.

**Nguyên tắc bắt buộc:**

- **DIP:** Chỉ sử dụng một interface chung `IRepository<T>`. Concrete Repository triển khai interface này và tiếp tục sử dụng `Repository<T>` để quản lý dữ liệu. Service phụ thuộc `IRepository<T>` thông qua Dependency Injection, không phụ thuộc trực tiếp Concrete Repository.
- **RBAC:** Kiểm tra quyền bằng `AuthService::hasPermission()` hoặc `requirePermission()` với mã Permission tương ứng. Không dùng `requireAdmin()`, `isAdmin()`, `isStaff()` hoặc kiểm tra role cứng để cấp quyền. `requireLogin()` không thay thế kiểm tra Permission. Không tự ý nuốt exception hoặc thay đổi hợp đồng trả lỗi.
- **Phân chia trách nhiệm:** Service xử lý nghiệp vụ; Repository quản lý dữ liệu và tính toàn vẹn; Model quản lý trạng thái và quan hệ; GUI chỉ tương tác với Service.

Các thành phần chưa tuân thủ sẽ được refactor theo kế hoạch, không tự ý sửa ngoài phạm vi công việc.

## 3. Quy trình Markdown-first bắt buộc

Áp dụng cho cả Hoàng, Kiệt và Codex khi tạo hoặc sửa lớp/hàm C++.

1. **Phân tích:** Đọc code, yêu cầu, test và tài liệu liên quan.
2. **Thiết kế:** Xác định thuộc tính, API, constructor/destructor, các hàm, dependency, Permission, xử lý lỗi và quan hệ dữ liệu.
3. **Viết Markdown:** Ghi đầy đủ code `.h` và `.cpp` dự kiến vào Markdown trước khi sửa mã nguồn. Template được định nghĩa trong header.
4. **Giải thích:** Mỗi hàm, kể cả constructor, destructor và helper/private, phải có:
   - **Tổng quan:** Mục đích, tham số, giá trị trả về và điều kiện quan trọng.
   - **Luồng hoạt động chi tiết:** Sơ đồ ASCII thể hiện đúng thứ tự và các nhánh xử lý.
   - **Giải thích chi tiết từng đoạn code:** Giải thích đầy đủ các đoạn code liên tiếp, biến, cú pháp C++, điều kiện, con trỏ, exception, rollback và thay đổi trạng thái khi liên quan. Không bỏ sót dòng code hoặc lặp lại nguyên hàm.
5. **Review:** Chỉnh sửa code trong Markdown đến khi nhóm xác nhận **CHỐT MARKDOWN**.
6. **Triển khai:** Sao chép code từ Markdown sang `.h/.cpp` giống **100%**, không tự ý thay đổi.
7. **Kiểm thử:** Biên dịch, chạy test và đối chiếu code với Markdown.
8. **Xử lý lỗi:** Nếu phát hiện lỗi, sửa và chốt lại Markdown trước khi cập nhật code.

**Quy định tài liệu:**

- Markdown thiết kế trước triển khai đặt trong `docs/architecture/` hoặc `docs/development/` tùy nội dung.
- `docs/src/` ghi nhận code đã triển khai và kiểm thử thành công.
- Phân biệt rõ trạng thái: dự kiến, đã chốt, đã triển khai và kiểm thử.
- Không đánh dấu hoàn thành khi chưa có kết quả kiểm thử thực tế.

## 4. Quy chuẩn tài liệu

- `docs/architecture/`: Kiến trúc, sơ đồ và quyết định thiết kế.
- `docs/overview/`: Tổng quan, quy tắc nghiệp vụ và định dạng dữ liệu.
- `docs/development/`: Kế hoạch, phân công, tiến độ, nhiệm vụ và Markdown thiết kế.
- `docs/src/`: Tài liệu code chi tiết theo module, cập nhật sau khi code được kiểm thử thành công.

**Yêu cầu:**

- Mỗi file có tiêu đề, bố cục rõ ràng và liên kết chính xác.
- Không lặp nội dung hoặc tạo nhiều phiên bản code chuẩn khác nhau.
- Tài liệu chi tiết phải bao phủ mọi hàm đã triển khai, kể cả private/helper và template.
- Code trong tài liệu phải khớp 100% với mã nguồn tương ứng.
- Không mô tả chức năng chưa triển khai như đã hoàn thành.
- Khi cập nhật tài liệu cũ, giữ nội dung còn đúng và chỉnh sửa tại chỗ.

## 5. Quy ước C++ và API

- Sử dụng C++17, ưu tiên cách viết đơn giản, dễ hiểu.
- Tên lớp và file: `PascalCase`; tên hàm và biến: `camelCase`.
- Getter: `getXxx()`; hàm kiểm tra boolean: `isXxx()`.
- Service ưu tiên `addXxx()`, `updateXxx()`, `removeXxx()`, `findXxxById()`, `getXxxAt()`, `getXxxCount()`.
- Repository ưu tiên `load()`, `save()`, `add()`, `update()`, `removeById()`, `findById()`, `getAt()`, `count()`, `isLoaded()` khi phù hợp.
- Khai báo hàm trong `.h`, định nghĩa trong `.cpp`; template định nghĩa trong header.
- Ưu tiên `const string&` cho chuỗi đầu vào, `const T&` khi phù hợp; hạn chế cấp quyền sửa Model trực tiếp từ Service.
- Kiểm tra `nullptr` trước khi truy cập con trỏ.
- Sử dụng ngoặc nhọn cho `if`, `for`, `while`; thụt lề 4 dấu cách.
- Comment ngắn gọn, giải thích mục đích hoặc lý do xử lý.
- Chỉ `#include` thư viện cần thiết; tránh `using namespace std;` trong header mới.
- Tái sử dụng Validator, `Repository<T>` và các tiện ích hiện có; không viết lại chức năng trùng lặp.
- Không tự ý thêm hoặc thay đổi public API, quy tắc xóa/sửa, định dạng dữ liệu hay ID.

## 6. Dữ liệu, con trỏ và tính toàn vẹn

**Định dạng dữ liệu**

- Lưu trữ bằng TXT UTF-8, theo schema của từng file.
- Kiểm tra số trường, kiểu dữ liệu, boolean `0/1` và enum hợp lệ khi parse.
- Dữ liệu sai phải được xử lý rõ ràng; không âm thầm bỏ qua hoặc ghi đè dữ liệu lỗi.
- Tuân thủ quy tắc ID hiện hành; không tự ý thay đổi số chữ số, khóa hoặc thêm `#schema=1`.

**Quản lý con trỏ**

- Repository sở hữu đối tượng `T*` sau khi thêm thành công và chịu trách nhiệm giải phóng khi thích hợp.
- Nếu `add()` thất bại, caller vẫn chịu trách nhiệm giải phóng con trỏ.
- Các con trỏ liên kết không mặc định đồng nghĩa với quyền sở hữu.
- Trước khi xóa, clear hoặc reload, phải bảo đảm không tạo con trỏ treo.

**Tính toàn vẹn dữ liệu**

- Link/unlink phải nhất quán hai chiều, không trùng hoặc liên kết sai.
- Khi thao tác thất bại, phải xử lý rollback phù hợp.
- Load các Repository theo đúng thứ tự phụ thuộc; xử lý rõ tham chiếu bị thiếu.
- Không tự động reload hoặc save nếu hợp đồng hiện tại không yêu cầu.
- Kiểm tra kết quả đọc/ghi file; không coi nhiều lần ghi TXT là transaction nguyên tử.
- Không giả định mọi đối tượng đều có một ID đơn duy nhất; các khóa ghép phải được thiết kế và thống nhất.
- Bảo vệ dữ liệu đã được tham chiếu và lịch sử giao dịch theo quy tắc nghiệp vụ.
- Nếu chưa bảo đảm được một ràng buộc, phải ghi rõ thay vì tự nhận đã hoàn thành.

## 7. Biên dịch, kiểm thử và bàn giao

- Mỗi file test có `main()` riêng; chỉ biên dịch một file test tại một thời điểm.
- Khi biên dịch test, lấy toàn bộ `.cpp` trong `src/`, trừ `main.cpp` và các file không phù hợp với cấu hình test.

Ví dụ trong Git Bash:

```bash
g++ -std=c++17 -I src tests/test_room_service.cpp $(find src -name "*.cpp" ! -name "main.cpp") -o test_room_service.exe

./test_room_service.exe
```

**Yêu cầu kiểm thử:**

- Kiểm tra trường hợp hợp lệ, không hợp lệ và các trường hợp biên.
- Kiểm tra dữ liệu trùng, chưa load, `nullptr`, exception và rollback khi liên quan.
- Kiểm tra Permission được cấp, bị từ chối và trạng thái chưa đăng nhập.
- Kiểm tra load/save, link/unlink và tính toàn vẹn dữ liệu.
- Dùng dữ liệu test riêng, không ghi đè dữ liệu thật.
- Không xóa hoặc sửa test chỉ để tạo kết quả PASS.
- Ghi nhận kết quả chạy thực tế và những phần chưa kiểm chứng.

Chỉ bàn giao khi code khớp Markdown đã chốt, biên dịch thành công, test liên quan đạt yêu cầu và tài liệu được cập nhật.

## 8. Git và cộng tác Hoàng–Kiệt–Codex

- Hoàng và Kiệt cùng tuân thủ quy trình Markdown-first.
- Làm việc trên nhánh riêng, không trực tiếp sửa `main` khi chưa thống nhất.
- Đồng bộ code mới nhất trước khi bắt đầu công việc.
- Chỉ sửa module và file thuộc phạm vi được giao.
- Trước khi thay đổi API, interface, schema, Permission hoặc quan hệ Model, phải thống nhất với người còn lại.
- Tạo Pull Request sau khi hoàn thành công việc.
- PR cần nêu nội dung thay đổi, Markdown đã chốt, kết quả test và vấn đề còn tồn tại.
- Người còn lại review code, tài liệu và khả năng tương thích trước khi merge.
- Kiểm tra xung đột, biên dịch và chạy test liên quan trước khi merge vào `main`.
- Codex không tự ý commit, push, merge hoặc sửa ngoài phạm vi khi chưa được yêu cầu.