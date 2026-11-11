# Tổng quan tiến độ PBL2 — Hoàng và Kiệt

Nhóm xác nhận **Bước 01–03 đã hoàn thành**. Xác nhận này là trạng thái kế hoạch hiện hành; các [nhật ký lịch sử](Buoc01_KhoiTaoDuAn.md) vẫn ghi đúng bằng chứng và giới hạn tại thời điểm thực hiện. Từ Bước 04, nhóm phát triển theo **chức năng**, mỗi chức năng gồm Repository → Service → kiểm thử C++ → cập nhật tài liệu. GUI Qt tập trung ở Bước 13. Xem [phân công](PhanCongCongViec.md), [Task Hoàng](tasks/Task_Hoang.md) và [Task Kiệt](tasks/Task_Kiet.md).

| Bước | Nội dung | Phụ trách | Trạng thái | Nhật ký |
| --- | --- | --- | --- | --- |
| 01 | Khởi tạo dự án | Hoàng và Kiệt | **Đã hoàn thành theo xác nhận nhóm**; giữ bằng chứng và ghi chú lịch sử. | [Bước 01](Buoc01_KhoiTaoDuAn.md) |
| 02 | Xây dựng Models và quan hệ Model | Hoàng và Kiệt | **Đã hoàn thành theo xác nhận nhóm**; giữ bằng chứng và ghi chú lịch sử. | [Bước 02](Buoc02_XayDungModels.md) |
| 03 | Xây dựng tiện ích lưu trữ dữ liệu ban đầu | Hoàng và Kiệt | **Đã hoàn thành theo xác nhận nhóm**; kết quả kiểm thử lịch sử giữ nguyên. Thiết kế mới dùng `FileIO`, `DateTime`, `IdGenerator`. | [Bước 03](Buoc03_HaTangLuuTruDuLieu.md) |
| 04 | Quản lý phim | Hoàng | **Đang thực hiện**: có mã nháp `MovieDALC` theo kiến trúc cũ, hiện chuyển thành bản nháp `src/data_access/MovieRepository.h/.cpp`; chưa có `MovieService` hoặc bằng chứng kiểm thử chức năng. | [Bước 04](Buoc04_XayDungDALC.md) |
| 05 | Đăng nhập và phân quyền | Kiệt | Chưa bắt đầu; `User` Model không đồng nghĩa `UserRepository`/`AuthService` đã có. | Chưa tạo |
| 06 | Quản lý phòng và ghế | Hoàng | Chưa bắt đầu. | Chưa tạo |
| 07 | Quản lý nhân viên | Kiệt | Chưa bắt đầu. | Chưa tạo |
| 08 | Quản lý suất chiếu | Hoàng | Chưa bắt đầu. | Chưa tạo |
| 09 | Quản lý thành viên | Kiệt | Chưa bắt đầu. | Chưa tạo |
| 10 | Giá vé, ưu đãi và tích điểm | Kiệt chính, Hoàng hỗ trợ | Chưa bắt đầu. | Chưa tạo |
| 11 | Bán vé tại quầy | Kiệt chính, Hoàng hỗ trợ tích hợp | Chưa bắt đầu; `Ticket`/`Invoice` Model không đồng nghĩa bán vé đã có. | Chưa tạo |
| 12 | Hủy vé, lịch sử hóa đơn và thống kê | Cả hai, chia theo nghiệp vụ | Chưa bắt đầu. | Chưa tạo |
| 13 | Xây dựng giao diện Qt | Cả hai | Chưa bắt đầu. | Chưa tạo |
| 14 | Kiểm thử tích hợp và sửa lỗi | Cả hai | Chưa bắt đầu. | Chưa tạo |
| 15 | Đóng gói, hoàn thiện tài liệu và bàn giao | Cả hai | Chưa bắt đầu. | Chưa tạo |

## Hiện trạng mã và cách đọc lịch sử

Mã hiện có tám Model ở `src/models/`, `DoublyLinkedList.h` ở `src/data_structures/`, `FileIO`, `DateTime`, `IdGenerator` ở `src/utils/`, bản nháp `MovieRepository` ở `src/data_access/` và `src/main.cpp` thử nghiệm. Chưa có Service, GUI Qt, các Repository còn lại, CMake hoặc dữ liệu TXT nghiệp vụ. `TransactionManager` cũ ở `tests/legacy/` chỉ để đối chiếu và giữ bài thử lịch sử.

Nhật ký Bước 01–03 được viết trước quyết định lộ trình mới nên có thể ghi trạng thái và tên thư mục cũ. Không sửa lịch sử để suy ra những lần build, test hoặc merge chưa xảy ra. Quy tắc TXT hiện hành: mọi file, kể cả `sequences.txt`, không có dòng schema hoặc tiêu đề; xem [định dạng dữ liệu](../TongQuan/data_format.md). Bước 04 chỉ hoàn thành khi cả `MovieRepository` và `MovieService` xử lý nghiệp vụ phim, có kiểm thử C++ và tài liệu tương ứng.
