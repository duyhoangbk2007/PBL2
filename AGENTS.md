# Quy tắc làm việc PBL2

- Dự án dùng C++17, OOP, Qt Widgets; dữ liệu lưu file `.txt`.
- Giữ luồng UI → Service → Repository → lưu file. Model giữ dữ liệu và liên kết object, không tự lưu file hay xử lý nghiệp vụ. Core không phụ thuộc Qt.
- Trong hàm thành viên, dùng `this->` khi truy cập thành viên. Header dùng `#ifndef`, `#define`, `#endif` với macro riêng.
- Model giữ ID để lưu file; con trỏ giữa các model là tham chiếu không sở hữu, trừ khi thiết kế thống nhất quy định khác.
- Tuân thủ [quy tắc nghiệp vụ](docs/TongQuan/business_rules.md), [định dạng dữ liệu](docs/TongQuan/data_format.md), [quan hệ model](docs/development/QuanHeModels.md).
- Trước nhiệm vụ mới, đọc [phân công](docs/development/PhanCongCongViec.md), [tiến độ](docs/development/TongQuanTienDo.md), rồi đối chiếu code và nhật ký liên quan.
- Mỗi file/module có một người chịu trách nhiệm chính. Thống nhất interface dùng chung trước khi sửa; người còn lại review và kiểm thử tích hợp.
- Chỉ tạo nhật ký khi thực sự bắt đầu bước. Nhật ký ghi việc Hoàng, việc Kiệt, tích hợp, checklist nghiệm thu, lỗi/cách khắc phục/kết quả thử, phần tiếp theo; cập nhật file cũ khi quay lại.
- Chỉ đánh dấu hoàn thành khi có bằng chứng code và kiểm thử. Không tự chuyển bước, commit, push hoặc sửa code ngoài phạm vi người dùng yêu cầu.
