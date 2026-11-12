# AGENTS.md — hướng dẫn làm việc với PBL2

## Mục đích và phạm vi

PBL2 là đồ án C++17 xây dựng ứng dụng desktop offline quản lý bán vé tại quầy cho rạp chiếu phim. Nhân viên thao tác thay khách; không có tài khoản khách hàng hay đặt vé trực tuyến. Dữ liệu đích lưu bằng các file TXT UTF-8. Hai vai trò dự kiến là Admin và Staff.

Đây là dự án đang triển khai, chưa phải ứng dụng hoàn chỉnh. Tài liệu mô tả cả thiết kế/lộ trình chưa có code. Luôn phân biệt rõ **đã triển khai** với **dự kiến**; xác minh trong cây mã trước khi khẳng định một chức năng tồn tại.

## Trạng thái code hiện có

- `src/models/`: tám model `Movie`, `Room`, `Seat`, `Showtime`, `User`, `Member`, `Ticket`, `Invoice` và quan hệ liên kết giữa một số model.
- `src/data_structures/DoublyLinkedList.h`: cấu trúc danh sách liên kết đôi tự cài đặt.
- `src/utils/`: `FileIO`, `DateTime`, `IdGenerator`.
- `src/data_access/MovieRepository.*` hiện có trong working tree; rà soát API/triển khai thực tế trước khi sửa vì tài liệu tiến độ có thể đã cũ.
- `src/data_access/UserRepository.*` chứa `Repository<User>` qua composition, dùng hash table theo ID/username. Đọc API/quyền sở hữu ở `docs/development/Buoc05_DangNhapVaPhanQuyen.md` trước khi tích hợp. Đã kiểm thử tích hợp Bước 05 qua `tests/test_auth.cpp`: 14 nhóm đạt, 0 lỗi ngày 2026-11-12.
- `src/business/AuthService.*` dùng `UserRepository&` để đăng nhập/đăng xuất và kiểm tra quyền. Dùng chung một AuthService trong phiên; Service quản trị gọi `requireAdmin()`, chức năng cho cả hai vai trò gọi `requireLogin()` và kiểm tra nghiệp vụ riêng. Đã kiểm thử Bước 05; chưa tích hợp GUI/các Service nghiệp vụ tiếp theo.
- `src/main.cpp` hiện chỉ là chương trình thử in thông báo; chưa phải ứng dụng GUI.
- Chưa có `src/gui/`, hệ thống Repository/Service đầy đủ, dữ liệu chạy mẫu hoặc `CMakeLists.txt` trong cây được rà soát. Không tự tạo chức năng hoặc khẳng định các phần này đã tồn tại.
- `tests/legacy/TransactionManager.*` là code lưu trữ lịch sử, không phải kiến trúc lưu hiện hành. `tests/test_all_utils.cpp` bao gồm cả test util và TransactionManager lịch sử.

## Kiến trúc mục tiêu

Luồng phụ thuộc: **GUI Qt Widgets → Business/Service → Data Access/Repository → file TXT**. Kết quả đi ngược lên UI.

- GUI nhận thao tác và trình bày kết quả; không chứa quy tắc phân quyền/nghiệp vụ.
- Service xác thực quyền, đầu vào, ràng buộc và phối hợp nghiệp vụ.
- Repository quản lý model trong RAM, phân tích/tuần tự hóa dữ liệu và gọi `FileIO`.
- Model là đối tượng dữ liệu, không tự đọc/ghi file.
- `FileIO` chỉ đọc/ghi các dòng; kiểm tra cấu trúc, khóa, dữ liệu và tham chiếu thuộc Repository/Service thích hợp.
- Chỉ `DoublyLinkedList<T>` bắt buộc tự cài theo đề tài. Với cấu trúc khác, ưu tiên STL nếu phù hợp; không tạo cấu trúc tự cài mới nếu không có yêu cầu rõ.

Không để UI gọi Repository trực tiếp khi Service tương ứng đã có. Không đưa kiểm tra quyền chỉ vào nút/ẩn màn hình; Service phải bảo vệ thao tác.

## Quy tắc nghiệp vụ cần giữ

- Một hóa đơn có một hoặc nhiều vé thuộc cùng một suất chiếu, tối đa một thành viên; lưu `showtimeId`. Mỗi vé tham chiếu đúng hóa đơn và suất đó.
- Một vé ứng với một vị trí ghế. Ghế đôi được bán nguyên cặp bằng một vé dành cho hai người.
- Ghế đã bán được xác định bằng cặp **mã suất chiếu + mã ghế**, không phải cờ bán trong sơ đồ phòng.
- Loại vé (thường/học sinh), loại ghế và tư cách thành viên là khái niệm riêng. Ưu đãi không cộng dồn; áp dụng mức giảm cao nhất đủ điều kiện.
- Undo chỉ hoàn tác lựa chọn ghế trước khi xác nhận bán; hủy vé đã thanh toán là nghiệp vụ riêng, giữ lịch sử.
- Staff chỉ sửa thông tin liên hệ thành viên qua luồng hợp lệ, không tự sửa điểm. Mọi thao tác bán (kể cả Admin bán) ghi đúng `userId` người thao tác vào hóa đơn.
- Trước khi xác nhận bán phải kiểm tra lại suất/ghế để tránh dữ liệu cũ hoặc ghế vừa bị bán. Không báo thành công nếu ghi các file cần thiết thất bại.
- Hủy vé (dự kiến chỉ Admin, trước giờ chiếu) cập nhật trạng thái và lưu lịch sử/hoàn tiền/thu hồi điểm theo quy tắc nghiệp vụ; không xóa lịch sử hóa đơn.
- Các file TXT có định dạng được quy định trong `docs/TongQuan/data_format.md`; quy tắc cụ thể ở `docs/TongQuan/business_rules.md`. Đọc tài liệu đó trước khi thay đổi schema hoặc nghiệp vụ.

## Quy ước khi thay đổi dự án

1. Trước khi sửa, đọc file liên quan và kiểm tra `README.md`, tài liệu chức năng tương ứng trong `docs/`, cùng `docs/development/TongQuanTienDo.md`. Tài liệu task cá nhân/nhật ký có thể ghi trạng thái cũ; đối chiếu code và ghi ngày/ngữ cảnh khi cần.
2. Giữ thay đổi trong đúng tầng và đúng phạm vi bước hiện tại. Không triển khai thay đổi schema, tầng mới hoặc chức năng ngoài yêu cầu chỉ vì tài liệu mục tiêu có nhắc tới.
3. Giữ tương thích với C++17 và phong cách tên hiện có. Header dùng include guard; include header trực tiếp cần thiết; giữ khai báo/định nghĩa nhất quán giữa `.h` và `.cpp`.
4. Các Model có con trỏ liên kết hai chiều. Khi sửa quan hệ, giữ nhất quán hai phía; kiểm tra `nullptr`, mã tham chiếu không khớp, liên kết lặp, mã trùng và đối tượng đã liên kết nơi khác. Con trỏ liên kết không đồng nghĩa với quyền sở hữu; đừng tự ý `delete` hoặc thay đổi vòng đời/copy-move nếu chưa theo thiết kế DAL.
5. Khi lưu TXT, kiểm tra mọi kết quả đọc/ghi và báo lỗi có ngữ cảnh. `IdGenerator` sinh mã từ `sequences.txt` nhưng trả dữ liệu sequence cập nhật cho bên gọi; không tự ghi file sequence trừ khi API/luồng đã yêu cầu. Ghi nhiều file thông thường không nguyên tử; không tuyên bố giao dịch an toàn nếu không có cơ chế được triển khai và kiểm tra.
6. Nếu thay đổi ảnh hưởng hành vi, schema, API hoặc trạng thái tiến độ, cập nhật tài liệu liên quan cùng thay đổi. Không sửa checklist lịch sử thành bằng chứng mới; chỉ đánh dấu hoàn tất khi đã có bằng chứng tương ứng.
7. Không tự thêm dependency, Qt setup, build system hoặc file dữ liệu production nếu yêu cầu không cần.

## Kiểm tra

Không tự chạy hoặc thêm test trừ khi người dùng yêu cầu kiểm tra/xác minh. Khi được yêu cầu, xem lệnh hiện có trong `tests/test_all_utils.cpp` và chạy trên dữ liệu tạm/cô lập; không sửa dữ liệu người dùng. Báo chính xác lệnh và kết quả thực tế, không suy diễn rằng toàn ứng dụng đã build từ một bài test tiện ích.

Riêng đăng nhập/phân quyền dùng `tests/test_auth.cpp`; lệnh build/chạy trong `docs/development/Buoc05_DangNhapVaPhanQuyen.md`. Runner tự tạo và dọn thư mục tạm; không dùng tài khoản thật. Kết quả 14 nhóm đạt ở trên là bằng chứng tại ngày ghi, không thay thế lần kiểm tra mới sau thay đổi liên quan.

## Tài liệu tham chiếu

- `README.md`: phạm vi, trạng thái, kiến trúc và lộ trình tổng quan.
- `docs/TongQuan/TongQuanDeTai.md`: luồng và vai trò.
- `docs/TongQuan/business_rules.md`: quy tắc nghiệp vụ.
- `docs/TongQuan/data_format.md`: định dạng file dữ liệu.
- `docs/CauTrucThuMuc/`: vai trò thư mục/model/API dự kiến.
- `docs/DS/DLL.md`: danh sách liên kết đôi.
- `docs/development/TongQuanTienDo.md`, `PhanCongCongViec.md` và nhật ký bước: trạng thái/lịch sử, cần đối chiếu working tree vì có thể lỗi thời.
