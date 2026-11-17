# Tổng quan tiến độ PBL2 — Hoàng và Kiệt

Nhóm xác nhận **Bước 01–03 đã hoàn thành**. Xác nhận này là trạng thái kế hoạch hiện hành; các [nhật ký lịch sử](Buoc01_KhoiTaoDuAn.md) vẫn ghi đúng bằng chứng và giới hạn tại thời điểm thực hiện. Từ Bước 04, nhóm phát triển theo **chức năng**, mỗi chức năng gồm Repository → Service → kiểm thử C++ → cập nhật tài liệu. GUI Qt tập trung ở Bước 12. Xem [phân công](PhanCongCongViec.md), [Task Hoàng](tasks/Task_Hoang.md) và [Task Kiệt](tasks/Task_Kiet.md).

| Bước | Nội dung | Phụ trách | Trạng thái | Nhật ký |
| --- | --- | --- | --- | --- |
| 01 | Khởi tạo dự án | Hoàng — Backend; Kiệt tham gia theo lịch sử | **Đã hoàn thành theo xác nhận nhóm**; giữ bằng chứng và ghi chú lịch sử. | [Bước 01](Buoc01_KhoiTaoDuAn.md) |
| 02 | Xây dựng Models và quan hệ Model | Hoàng — Backend; Kiệt tham gia theo lịch sử | **Đã hoàn thành theo xác nhận nhóm**; giữ bằng chứng và ghi chú lịch sử. | [Bước 02](Buoc02_XayDungModels.md) |
| 03 | Xây dựng tiện ích lưu trữ dữ liệu ban đầu | Hoàng — Backend; Kiệt tham gia theo lịch sử | **Đã hoàn thành theo xác nhận nhóm**; kết quả kiểm thử lịch sử giữ nguyên. Thiết kế mới dùng `FileIO`, `DateTime`, `IdGenerator`. | [Bước 03](Buoc03_HaTangLuuTruDuLieu.md) |
| 04 | Quản lý phim | Hoàng | **Đang thực hiện, tạm dừng để làm Bước 06**: `MovieRepository` đạt 40/40, `MovieService` đạt 45/45 trong phạm vi test hiện có; nghiệp vụ đổi trạng thái phim còn thiếu. | [Bước 04](Buoc04_QuanLyPhim.md) |
| 05 | Đăng nhập và phân quyền | Hoàng | **Đang thực hiện, chưa nghiệm thu**: `UserRepository` và `AuthService` đã có; `test_auth.cpp` đạt 9/14, còn 5 ca lỗi. | [Bước 05](Buoc05_DangNhapVaPhanQuyen.md) |
| 06 | Quản lý phòng, ghế và suất chiếu | Hoàng | **Đang thực hiện**: RoomRepository/Service đạt 78/78 và 54/54; SeatRepository/Service đạt 85/85 và 59/59; ShowtimeRepository đạt 89/89 trong các test đã ghi. `ShowtimeService` có mã tạo/sửa/tra cứu và kiểm tra quyền nhưng hủy/xóa chưa hoàn thiện, chưa có test riêng; toàn bước chưa nghiệm thu. | [Bước 06](Buoc06_QuanLyPhongVaGhe.md) |
| 07 | Quản lý nhân viên | Hoàng | Chưa bắt đầu. | Chưa tạo |
| 08 | Quản lý thành viên | Hoàng | Chưa bắt đầu. | Chưa tạo |
| 09 | Giá vé, ưu đãi và tích điểm | Hoàng | Chưa bắt đầu. | Chưa tạo |
| 10 | Bán vé tại quầy | Hoàng | Chưa bắt đầu; `Ticket`/`Invoice` Model không đồng nghĩa bán vé đã có. | Chưa tạo |
| 11 | Hủy vé, lịch sử hóa đơn và thống kê | Hoàng | Chưa bắt đầu. | Chưa tạo |
| 12 | Xây dựng giao diện Qt | Kiệt | Chưa bắt đầu. | Chưa tạo |
| 13 | Kiểm thử tích hợp và sửa lỗi | Kiệt — GUI; Hoàng — Backend | Chưa bắt đầu. | Chưa tạo |
| 14 | Đóng gói, hoàn thiện tài liệu và bàn giao | Kiệt — GUI/bàn giao; Hoàng — hỗ trợ Backend | Chưa bắt đầu. | Chưa tạo |

**Phân công hiện hành:** Hoàng chịu trách nhiệm toàn bộ Backend Bước 01–11; Kiệt chịu trách nhiệm GUI và tích hợp giao diện từ Bước 12. Đóng góp Backend trước đây của Kiệt ở Bước 01–03 vẫn là lịch sử, không thay đổi trạng thái hoàn thành.

## Hiện trạng mã và cách đọc lịch sử

Mã hiện có tám Model ở `src/models/`, `DoublyLinkedList.h` ở `src/data_structures/`, `FileIO`, `DateTime`, `IdGenerator` ở `src/utils/` và `src/main.cpp` thử nghiệm. Đã có `MovieRepository` và `MovieService`, mỗi lớp có test riêng đạt nhưng Service còn thiếu nghiệp vụ đổi trạng thái; đã có Repository/Service phòng và ghế cùng `ShowtimeRepository` với test riêng đạt; `ShowtimeService` mới triển khai một phần, chưa có test riêng; chưa có GUI Qt hoặc CMake. `TransactionManager` cũ ở `tests/legacy/` chỉ để đối chiếu và giữ bài thử lịch sử.

Nhật ký Bước 01–03 được viết trước quyết định lộ trình mới nên có thể ghi trạng thái và tên thư mục cũ. Không sửa lịch sử để suy ra những lần build, test hoặc merge chưa xảy ra. Quy tắc TXT hiện hành: mọi file, kể cả `sequences.txt`, không có dòng schema hoặc tiêu đề; xem [định dạng dữ liệu](../TongQuan/data_format.md). Bước 04 chỉ hoàn thành khi cả `MovieRepository` và `MovieService` xử lý nghiệp vụ phim, có kiểm thử C++ và tài liệu tương ứng.
