# Bước 01 — Khởi tạo Project

## Checklist công việc

- [x] Khảo sát lại toàn bộ file, thư mục, mã nguồn và tài liệu hiện có (2026-10-11).
- [x] Tạo nhật ký Bước 01 và xác định việc còn làm.
- [x] Ghi vai trò các thư mục hiện có và đối chiếu với cấu trúc mục tiêu.
- [x] Ghi tổng quan ba CTDL đã có vào `docs/DS/`; chưa kiểm thử mã CTDL.
- [ ] Xác minh bộ công cụ C++17 và CMake dùng trên Windows.
- [ ] Thống nhất và hoàn thiện cấu trúc thư mục tối thiểu cho bước khởi tạo.
- [ ] Viết chương trình C++17 tối thiểu trong `main.cpp` và cấu hình build trong `CMakeLists.txt`.
- [ ] Build, chạy thử và ghi lại kết quả thực tế.
- [ ] Bổ sung hướng dẫn khởi động/build sau khi đã kiểm chứng.

**Hiện trạng khi bắt đầu khảo sát:** `main.cpp` và `CMakeLists.txt` đều rỗng. Có `core/data_structures/DoublyLinkedList.h`, `HashTable.h`, `Stack.h`; đây là mã hiện có, chưa được kiểm thử trong Bước 01. Có `core/models/`, `core/repositories/`, `core/service/`, `core/utils/`, `data/`, `ui/`; chưa có file triển khai trong các thư mục đó. Ba file tài liệu trong `docs/DS/` lúc đầu đều rỗng và đã được bổ sung tổng quan ở lần cập nhật này. Tài liệu thiết kế mô tả cấu trúc mục tiêu và nghiệp vụ dự kiến, không xác nhận chức năng đã hoàn thành. Git có nhiều thay đổi sẵn; chưa sửa mã nguồn trong lần khảo sát này.

**Vai trò các thư mục** (vai trò dự kiến theo kiến trúc; thư mục tồn tại chưa có nghĩa chức năng đã được lập trình):

| Thư mục | Trách nhiệm | Hiện trạng |
| --- | --- | --- |
| `core/` | Chứa phần xử lý C++ thuần, không phụ thuộc Qt. UI sẽ gọi các service trong core thay vì tự xử lý nghiệp vụ. | Đã có các thư mục con, chưa có luồng ứng dụng hoàn chỉnh. |
| `core/models/` | Định nghĩa đối tượng và dữ liệu của chúng, như phim, phòng, suất chiếu, vé, hóa đơn, tài khoản. Model không tự đọc file hay quyết định quyền. | Có thư mục, chưa có file model. |
| `core/data_structures/` | Chứa cấu trúc dữ liệu trong bộ nhớ để lưu, tra cứu hoặc hoàn tác thao tác. | Có ba header: `DoublyLinkedList.h`, `HashTable.h`, `Stack.h`. Chưa xác minh bằng kiểm thử. |
| `core/repositories/` | Truy xuất dữ liệu: đọc/ghi qua tiện ích lưu trữ, chuyển bản ghi thành model, thêm/sửa/xóa/tìm. Quy tắc nghiệp vụ thuộc service. | Có thư mục, chưa có file repository. |
| `core/service/` | Thư mục đang có nhưng chưa có mã. Theo thiết kế, tầng service sẽ kiểm tra đầu vào, quyền và quy tắc nghiệp vụ, rồi điều phối repository. | Tên hiện tại là số ít; tài liệu dùng `core/services/`. Chưa quyết định đổi tên. |
| `core/utils/` | Các tiện ích dùng chung như thao tác file, ngày giờ, kiểm tra dữ liệu và hỗ trợ giao dịch lưu nhiều file. | Có thư mục, chưa có file tiện ích. |
| `core/config/` | Nơi dự kiến đặt hằng số cấu hình giá vé, ưu đãi và khoảng nghỉ giữa suất; tránh rải giá trị trong nhiều service. | Chưa có thư mục. |
| `ui/` | Giao diện Qt Widgets dự kiến: nhận thao tác, hiển thị kết quả và gọi service; không trực tiếp đọc/ghi file nghiệp vụ. | Có thư mục, chưa có file giao diện; Qt chưa được thiết lập. |
| `data/` | Dữ liệu văn bản `.txt` của ứng dụng và vùng giao dịch/phục hồi sau này. Định dạng cụ thể theo `docs/data_format.md`. | Có thư mục, chưa có file dữ liệu. |
| `docs/` | Tài liệu thiết kế, quy tắc và ghi chép học tập; không phải mã chạy của ứng dụng. | Có tài liệu thiết kế, `DS/` và `development/`. |
| `docs/DS/` | Giải thích cấu trúc dữ liệu đã làm để dễ học lại và đối chiếu với mã. | Đã bổ sung tổng quan DLL, HashTable và Stack trong lần này. |
| `docs/development/` | Theo dõi trạng thái 15 bước và nhật ký của bước đã thực sự bắt đầu; giúp khôi phục ngữ cảnh. | Có file tổng quan và nhật ký Bước 01. |
| `tests/` | Nơi dự kiến đặt bài kiểm tra cấu trúc dữ liệu, repository, service và dữ liệu mẫu. | Chưa có thư mục; chưa chạy kiểm thử. |

**Cập nhật 2026-10-11:** Đã viết tài liệu tổng quan trong `docs/DS/DLL.md`, `HashTable.md`, `Stack.md` dựa trên ba header hiện có. Không sửa `.h`, `.cpp`, `main.cpp` hoặc `CMakeLists.txt`. Kiến thức ghi nhận: thư mục thể hiện trách nhiệm của từng tầng; template CTDL được cài trong header để trình biên dịch thấy phần định nghĩa khi dùng kiểu cụ thể. Chưa build hoặc chạy test nên chưa xác nhận tính đúng của ba CTDL.

## Lỗi và vướng mắc

- Chưa có lỗi build được xác nhận vì chưa có cấu hình hoặc chương trình để build.
- `g++` tìm thấy tại `C:\msys64\ucrt64\bin\g++.exe`, phiên bản 15.1.0. Chưa tìm thấy lệnh `cmake` trong PATH; cần xác minh cài đặt hoặc cấu hình môi trường.
- `core/service/` khác tên `core/services/` trong tài liệu. Cần kiểm tra ảnh hưởng trước khi điều chỉnh.
- Chưa có kết quả chạy thử chương trình. Đã đọc ba header ở mức tĩnh nhưng chưa biên dịch hoặc kiểm thử; cả ba lớp sở hữu con trỏ cấp phát động mà chưa kiểm soát thao tác sao chép, nên cần xem xét nguy cơ giải phóng lặp ở bước CTDL.

## Các phần tiếp theo cần làm

1. **Công cụ:** kiểm tra `g++`, CMake và cách dùng chúng trong terminal VS Code; ghi kết quả. Chờ người dùng xác nhận trước khi bắt đầu.
2. **Thư mục:** đối chiếu cấu trúc hiện có với kiến trúc mục tiêu, chỉ tạo/sửa thư mục cần cho Bước 01.
3. **Project tối thiểu:** hướng dẫn từng phần của `main.cpp` và `CMakeLists.txt` để hiểu điểm vào chương trình và CMake.
4. **Kiểm chứng:** cấu hình, build, chạy chương trình C++17; ghi lệnh, kết quả, lỗi và cách khắc phục; cập nhật hướng dẫn build.

Sau mỗi phần, cập nhật checklist, kiến thức học được, thay đổi code, lỗi, kết quả kiểm thử và việc tiếp theo ngay trong ba mục trên; giữ lịch sử những lần cập nhật. Không chuyển phần hoặc bước khi chưa được yêu cầu.
