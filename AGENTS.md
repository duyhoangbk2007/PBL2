# AGENT.md — Quy ước làm việc chung dự án PBL2

> Dành cho Hoàng, Kiệt và trợ lý lập trình (Codex/AI). Đặt file này ở **thư mục gốc repository**. Đây là hướng dẫn phối hợp; nếu tài liệu và code mâu thuẫn, phải kiểm tra mã hiện tại và hỏi nhóm trước khi thay đổi quy tắc nghiệp vụ.

## 1. Bối cảnh và nguyên tắc ưu tiên

- Dự án: hệ thống bán vé rạp chiếu phim **offline tại quầy**, C++17, giao diện Qt Widgets dự kiến; dữ liệu TXT UTF-8. Hai vai trò **ADMIN** và **STAFF**; không có tài khoản khách hàng hay đặt vé online.
- Ưu tiên đọc theo thứ tự: (1) code thực tế trên **nhánh đang làm**, (2) `docs/TongQuan/data_format.md` và `docs/TongQuan/business_rules.md`, (3) `docs/development/TongQuanTienDo.md`, nhật ký bước và task, (4) các tài liệu thiết kế tổng quan. Tài liệu lịch sử có thể chứa đường dẫn/thiết kế cũ; **không sao chép máy móc**.
- **Không tự ý sửa code, tài liệu, xóa file, đổi kiến trúc, commit, push, merge hay tạo PR** khi chưa được người phụ trách yêu cầu rõ ràng. Khi được yêu cầu phân tích hoặc đề xuất, chỉ giải thích và đưa bản nháp/diff.
- Không tuyên bố “đã test”, “đã push”, “đã hoàn thành” nếu chưa có bằng chứng thực tế. Phân biệt **đã viết code**, **đã biên dịch**, **đã chạy test**, **đã tích hợp**.
- Làm theo từng phần nhỏ khi người thực hiện đang học; giải thích lý do và tác động trước những thay đổi không hiển nhiên. Không đưa thêm thư viện, kỹ thuật hoặc abstraction phức tạp nếu chưa cần.

## 2. Cấu trúc hiện hành và trách nhiệm các tầng

```text
src/
  models/             # Movie, Room, Seat, Showtime, User, Member, Ticket, Invoice
  data_structures/    # DoublyLinkedList và cấu trúc dữ liệu tự cài
  data_access/        # Repository<T>, các *Repository.h/.cpp
  services/           # Các *Service.h/.cpp (kiểm tra tên thư mục thực tế)
  utils/              # FileIO, Validator, DateTime, IdGenerator...
tests/                # test_*.cpp và các bài test hiện có
data/                 # các file TXT nghiệp vụ
docs/
  TongQuan/           # data_format.md, business_rules.md và thiết kế
  development/        # TongQuanTienDo.md, BuocXX_*.md, PhanCongCongViec.md
    tasks/            # Task_Hoang.md, Task_Kiet.md
```

**Luồng thiết kế:** UI → Service → Repository → FileIO/TXT. Model chứa dữ liệu và quan hệ đối tượng; không đặt đọc/ghi TXT hoặc kiểm tra quyền vào Model. Repository quản lý bản ghi, parse/serialize, tra cứu, dữ liệu hợp lệ và ràng buộc lưu trữ. Service chịu trách nhiệm nghiệp vụ, phân quyền, phối hợp nhiều Repository và các điều kiện theo ngữ cảnh. UI không truy cập file TXT trực tiếp.

**Lưu ý:** Một số tài liệu cũ dùng `core/`, `repositories/` hoặc `DALC`; ưu tiên cấu trúc `src/` và `data_access/` trong code hiện tại. Không tự đổi tên toàn dự án để khớp tài liệu cũ.

## 3. Form code C++ hiện tại

1. C++17; class khai báo trong `.h`, định nghĩa các hàm không phải template trong `.cpp`. Header dùng `#ifndef`/`#define`/`#endif` với macro duy nhất; tránh viết toàn bộ logic trong header. Riêng template `Repository<T>` hiện triển khai trong `Repository.h`.
2. Giữ phong cách dễ đọc: `std::string`/`string`, `bool` cho kết quả thao tác, con trỏ `T*` khi tìm đối tượng (không có thì `nullptr`), `const string&` cho tham số chuỗi, `const` ở getter khi phù hợp với API hiện có. Không tự chuyển toàn bộ sang `shared_ptr`, `unique_ptr`, `optional`, framework mới.
3. Tên lớp PascalCase (`RoomRepository`), hàm camelCase (`findById`, `isValidRoom`), file theo tên lớp (`RoomRepository.h/.cpp`). Đặt dấu `{}` nhất quán, có chú thích ngắn ở các đoạn kiểm tra nghiệp vụ hoặc quyền sở hữu khó hiểu; tránh comment giải thích hiển nhiên.
4. Dùng các tiện ích đã có: `Validator::isValidText`, `Validator::isValidDate`, `Validator::isValidId` **nếu đã tồn tại trên nhánh hiện tại**; `FileIO::readLines/writeLines`, `IdGenerator`, `DateTime`. Kiểm tra khai báo thực tế trước khi gọi, không tự giả định API.
5. Đối với Repository cụ thể, ưu tiên **composition** với `Repository<T> repo;` như `MovieRepository`, `RoomRepository`, thay vì lặp lại logic đọc/ghi, danh sách và giải phóng bộ nhớ. Constructor truyền các hàm tĩnh `getId`, `parse`, `serialize` phù hợp chữ ký `Repository<T>`.
6. Form Repository thường có: constructor; `getId`, `parse`, `serialize`, `isValid...` (private static); `load`, `save`, `add`, `update`, `removeById`, `findById`, `getAt`, `count`, `isLoaded` (public). **Đây là mẫu tham khảo, không bắt buộc** nếu thực thể có khóa hoặc quan hệ khác.
7. `parse` kiểm tra **đúng số trường**, số nguyên phải được chuyển hết chuỗi (`stoi` + vị trí), enum/boolean phải thuộc tập giá trị cho phép; dữ liệu không hợp lệ ném `runtime_error`. Nếu đã `new` rồi phát hiện sai, phải `delete` trước khi ném lỗi. `serialize` kiểm tra dữ liệu trước khi ghi. `add/update/remove` trả `false` khi thao tác không được phép theo hợp đồng hiện có.
8. Không lặp kiểm tra đã có trong `Repository<T>` trừ khi cần ràng buộc riêng. `Repository<T>` hiện **sở hữu** các con trỏ đã thêm/nạp thành công; con trỏ liên kết giữa các Model **không sở hữu** đối tượng đích. Không thêm cùng một con trỏ hai lần; không xóa/nạp lại khi còn tham chiếu có thể bị treo.
9. Không thay đổi chữ ký hàm, định dạng file, cấu trúc dữ liệu chung hoặc cơ chế quản lý bộ nhớ chỉ để sửa một lỗi cục bộ nếu chưa phân tích tác động và thống nhất với người phụ trách.

### Ví dụ form một Repository

```cpp
// ExampleRepository.h (minh hoa, khong phai API can tao)
#ifndef EXAMPLE_REPOSITORY_H
#define EXAMPLE_REPOSITORY_H

#include <string>
#include "Repository.h"
#include "../models/Example.h"

class ExampleRepository
{
private:
    Repository<Example> repo;
    static std::string getId(Example* item);
    static Example* parse(const std::string& line);
    static std::string serialize(Example* item);
    static bool isValid(const Example* item);

public:
    ExampleRepository(const std::string& filePath);
    bool load();
    bool save();
    bool add(Example* item);
    bool update(const Example& values);
    bool removeById(const std::string& id);
    Example* findById(const std::string& id);
    Example* getAt(int index);
    int count();
    bool isLoaded();
};
#endif
```

## 4. Định dạng dữ liệu và quan hệ

- TXT UTF-8, mỗi dòng một bản ghi, phân tách `|`, **không có dòng tiêu đề/schema**; boolean `0/1`, enum chữ hoa, giữ cả trường rỗng cuối dòng. Không cho phép ký tự `|`, xuống dòng, ký tự điều khiển trong text.
- ID thực thể theo tiền tố ba chữ và bốn chữ số, số thứ tự `0001`–`9999` (ví dụ `MOV0001`, `ROM0001`, `USR0001`). **Ghế khác:** khóa `(roomId, seatId)`, không được dùng riêng `seatId` như khóa toàn cục.
- `rooms.txt`: `roomId|name|rowCount|columnCount|active`; `seats.txt`: `roomId|seatId|row|column|seatType`. Vị trí `(roomId,row,column)` phải duy nhất; ghế nằm trong kích thước phòng. `STANDARD/VIP/COUPLE`; ghế đôi là **một vị trí logic**.
- Trạng thái ghế đã bán thuộc **từng suất chiếu**, không ghi trong `seats.txt`. Không xóa/sửa đối tượng gây hỏng liên kết với suất chiếu, vé, hóa đơn. Đặc biệt phải xác định thứ tự `load` và liên kết lại các con trỏ sau khi nạp.
- `Repository<T>::load()` hiện có thể xóa các đối tượng cũ sau khi parse thành công; phải xét nguy cơ con trỏ treo trước khi cho phép reload. Không áp dụng máy móc hành vi `MovieRepository` cho `RoomRepository`, `SeatRepository` hoặc giao dịch.
- Các nghiệp vụ bán/hủy vé, tích điểm, phân quyền phải theo `docs/TongQuan/business_rules.md`; không tự suy diễn từ tên setter/model.

## 5. Quy trình thực hiện mỗi bước

**Trước khi code**
1. `git status`, `git branch --show-current`, `git pull` đúng nhánh khi phù hợp; không làm mất thay đổi chưa commit.
2. Đọc `docs/development/TongQuanTienDo.md`, `docs/development/PhanCongCongViec.md`, nhật ký `BuocXX_*.md`, task của **người được phân công**, đặc tả dữ liệu/nghiệp vụ và code liên quan.
3. Kiểm tra tên file, API, kiểu dữ liệu, quyền sở hữu và các liên kết đang có. Nếu tài liệu lệch code, ghi rõ chỗ lệch và hỏi trước khi sửa.
4. Nêu phạm vi nhỏ sẽ làm, file dự kiến thay đổi, điều kiện hoàn thành; không tự mở rộng sang bước khác.

**Khi code**
5. Theo thứ tự phù hợp: Repository → Service → kiểm thử C++ → tài liệu; mỗi lớp `.h` trước, `.cpp` sau, triển khai từng hàm dễ kiểm tra. Không sao chép nguyên lớp khác nếu khác khóa/ràng buộc.
6. Giữ thay đổi nhỏ và đúng phạm vi; tránh refactor không liên quan. Phải kiểm tra cả trường hợp hợp lệ, dữ liệu sai, trùng, không tìm thấy, chưa load, lỗi đọc/ghi và ràng buộc tham chiếu.
7. Viết test độc lập trong `tests/test_<module>.cpp`, dùng **file TXT tạm**, không ghi đè `data/*.txt` thật. Mẫu test: `check()` in `[PASS]/[FAIL]`, thống kê `Passed/Failed/Total`, `main()` trả `1` nếu có test lỗi.

**Trước khi push/PR**
8. Biên dịch với `g++ -std=c++17 -Wall -Wextra ...` và chạy executable; ghi **lệnh thực tế**, số PASS/FAIL và giới hạn test. Không coi “biên dịch thành công” là “đã test”.
9. `git diff`, `git status`: kiểm tra chỉ có file cần thiết; không commit `.exe`, file test tạm, dữ liệu thật vô tình thay đổi hoặc bí mật.
10. Cập nhật tài liệu theo Mục 6, commit có ý nghĩa, push nhánh cá nhân, mở PR và yêu cầu review. **Không tự merge** khi chưa được nhóm đồng ý; nếu thay đổi liên quan phần của người kia, phải thông báo và kiểm tra tích hợp.

## 6. Quy tắc cập nhật tài liệu (bắt buộc)

Sau **mỗi phần việc thực sự hoàn thành**, cập nhật đồng bộ:

| File | Cần ghi gì |
| --- | --- |
| `docs/development/tasks/Task_Hoang.md` **hoặc** `Task_Kiet.md` | Chỉ cập nhật task của người làm; checkbox hoàn thành, nhánh, ghi chú, test, PR nếu có. Không tự sửa task của người khác. |
| `docs/development/BuocXX_*.md` | Nhật ký bước: thay đổi file/API, quyết định thiết kế, quy tắc dữ liệu, lệnh biên dịch/test, kết quả thực tế, vấn đề còn mở, việc tiếp theo. Tạo file nhật ký nếu bước chưa có. |
| `docs/development/TongQuanTienDo.md` | Cập nhật **trạng thái thực tế** của bước; chỉ đánh dấu hoàn thành khi Repository + Service + kiểm thử + tích hợp/tài liệu đáp ứng điều kiện bước. |
| `docs/TongQuan/data_format.md` | Chỉ sửa khi nhóm **đã thống nhất** thay đổi định dạng TXT, ID, enum hoặc ràng buộc lưu trữ. |
| `docs/TongQuan/business_rules.md` | Chỉ sửa khi nhóm **đã thống nhất** thay đổi nghiệp vụ, quyền hoặc điều kiện giao dịch. |
| `README.md`, tài liệu kiến trúc/cấu trúc thư mục | Cập nhật nếu thay đổi hướng dẫn chạy, kiến trúc hoặc đường dẫn thực tế; tránh sửa lịch sử vô cớ. |

- Không xóa/viết lại bằng chứng lịch sử chỉ vì kiến trúc mới đã thay đổi. Thêm mục **“Cập nhật ngày YYYY-MM-DD”** để phân biệt tình trạng mới với ghi chú cũ.
- Không tự đánh dấu toàn bộ bước “xong” khi mới hoàn thành một Repository; không ghi kết quả test chưa chạy. Nếu chưa biết PR/commit, ghi **“chưa xác nhận”**.
- Nếu công việc chạm nhiều bước, ghi rõ phụ thuộc và file liên quan; không cập nhật bừa tất cả task.

## 7. Quy trình Git phối hợp Hoàng – Kiệt

- Mỗi bước/người có nhánh riêng, mẫu: `feature/bXX-<ten>-<mo-ta-kebab-case>`. Ví dụ: `feature/b06-hoang-room-seat-management`.
- Tạo nhánh từ base đã được nhóm thống nhất (thường `main` sau khi tích hợp). **Kiểm tra nhánh gốc** trước khi tạo để không vô tình kéo theo commit chưa merge của bước trước.
- Commit nhỏ, rõ nội dung; ví dụ `feat(b06): implement RoomRepository and tests`, `test(b05): cover authentication failures`, `docs(b06): update progress and test evidence`.
- Trước PR: kiểm tra diff, build/test, xung đột với code của người còn lại, cập nhật tài liệu. PR ghi mục tiêu, file thay đổi, cách chạy test, kết quả, giới hạn và các quyết định cần review.
- Không force-push, rebase nhánh của người khác, xóa nhánh hoặc merge PR nếu chưa được người phụ trách đồng ý.

## 8. Checklist tự rà soát trước khi báo hoàn thành

- [ ] Đã đọc code thực tế và đúng tài liệu hiện hành.
- [ ] Thay đổi đúng file, đúng trách nhiệm Model/Repository/Service/UI.
- [ ] Không dùng API chưa tồn tại; `.h` và `.cpp` khớp chữ ký.
- [ ] Đã xét dữ liệu sai, ID/khóa trùng, con trỏ `nullptr`, ownership và liên kết tham chiếu.
- [ ] Đã biên dịch và chạy test, ghi rõ kết quả thực tế.
- [ ] Không làm thay đổi dữ liệu nghiệp vụ thật khi test.
- [ ] Đã cập nhật task cá nhân, nhật ký bước và tiến độ chung ở mức phù hợp.
- [ ] Đã kiểm tra `git diff`/`git status`, commit/PR theo đúng yêu cầu.

## 9. Cách trợ lý/AI phải phản hồi

- Khi được hỏi **“đọc commit/kiểm tra code”**: đọc nhánh/commit mới nhất được chỉ định, trích chính xác đường dẫn và hàm; không khẳng định đã đọc nếu chưa truy cập được.
- Khi được hỏi **“hướng dẫn code”**: đưa code đặt đúng file, giải thích từng phần, ưu tiên cú pháp C++ đơn giản và tương thích code hiện có; nếu người học yêu cầu từng phần thì không đổ toàn bộ lớp trong một lượt.
- Khi được hỏi **“sửa”**: trước tiên chỉ rõ lỗi, nguyên nhân, file và thay đổi tối thiểu. Chỉ trực tiếp sửa repository khi được yêu cầu rõ.
- Khi được hỏi **“cập nhật docs”**: rà soát code/test/commit thực tế, sửa đúng những file ở Mục 6; không tự thay đổi quy tắc nghiệp vụ hoặc tiến độ lịch sử.
- Khi chưa chắc: **hỏi một câu cụ thể**, không tự đoán và viết code theo giả định chưa kiểm chứng.
