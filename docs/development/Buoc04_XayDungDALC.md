# Nhật ký Bước 04 — Quản lý phim

> Giữ tên file `Buoc04_XayDungDALC.md` vì đó là tên nhật ký lịch sử và nhiều liên kết đang trỏ tới. **Mục tiêu Bước 04 hiện hành là chức năng quản lý phim hoàn chỉnh**, gồm `MovieRepository`, `MovieService`, kiểm thử C++ và cập nhật tài liệu; không phải hoàn thiện toàn bộ tầng Repository.

## Mốc lịch sử 2026-11-10

Khi chuẩn bị Bước 04 theo kế hoạch cũ, nhóm dùng tên DALC và đã có mã nháp `core/dal/dalc/MovieDALC.h/.cpp`. Hạ tầng lúc đó dùng `FileHandler` và `TransactionManager`; chưa có bằng chứng nghiệm thu hoặc kiểm thử tích hợp chức năng phim. Sau quyết định cấu trúc mới, mã nháp được chuyển và đổi tên thành `src/data_access/MovieRepository.h/.cpp`, đọc/ghi qua `FileIO`. Việc chuyển tên/thư mục **không chứng minh Repository đã hoàn thành**. Phần sửa `load()` đọc từ dòng đầu được giữ; mọi TXT không có schema hoặc tiêu đề.

## Mục tiêu hiện hành và phân công

**Hoàng phụ trách chính.** Thực hiện theo thứ tự: Repository → Service → kiểm thử C++ → cập nhật tài liệu. Kiệt có thể làm Bước 05 đăng nhập song song sau khi thống nhất phần API dùng chung. Qt Widgets chưa thuộc Bước 04; giao diện phim nằm ở Bước 13.

| Thành phần | Công việc |
| --- | --- |
| `MovieRepository` | Quản lý tập `Movie` trong RAM; đọc/ghi `movies.txt` qua `FileIO`; tra cứu, kiểm tra mã trùng và định dạng TXT. |
| `MovieService` | Kiểm tra quyền Admin, dữ liệu và điều kiện thêm/sửa/xóa/ngừng phim; gọi Repository, không đọc TXT trực tiếp. |
| Kiểm thử C++ | Thử CRUD, tìm kiếm, trạng thái, mã trùng, bản ghi sai, lưu/nạp lại và lỗi ghi. |
| Tài liệu | Ghi API, quyết định và lệnh/kết quả kiểm thử thực tế trong nhật ký và Task. |

### Checklist

- [x] Chuyển bản nháp `MovieDALC` thành `MovieRepository` ở `src/data_access/`, đổi nơi gọi sang `FileIO`; kiểm tra cú pháp C++17. Đây là việc chuyển cấu trúc, chưa nghiệm thu chức năng.
- [ ] Đối chiếu API Model `Movie`, định dạng `movies.txt`, quyền sở hữu object và quan hệ với suất chiếu.
- [ ] Hoàn thiện `MovieRepository` để nạp, tra cứu, sửa và ghi toàn bộ file; báo lỗi file/dòng khi dữ liệu không hợp lệ.
- [ ] Cài `MovieService` cho nghiệp vụ phim và kiểm tra quyền; chỉ thêm API thực sự cần.
- [ ] Kiểm thử chức năng phim bằng C++ và ghi bằng chứng; kiểm tra kết quả lưu/nạp lại, không bỏ dòng đầu.
- [ ] Cập nhật tài liệu theo hành vi đã được kiểm chứng và tích hợp với `AuthService` khi API Kiệt có sẵn.

`IdGenerator` hiện trả mã và `updatedSequences` nhưng không tự ghi `sequences.txt`; khi thêm phim phải giữ quy tắc cấp mã hiện hành. Ghi nhiều file tuần tự không nguyên tử: phải kiểm tra kết quả từng lần ghi, báo lỗi và không âm thầm ghi đè dữ liệu sai.

## Bằng chứng hiện có và điều kiện hoàn thành

Sau khi chuyển thư mục, mã ứng dụng C++17 biên dịch và liên kết được; `tests/test_all_utils.cpp` chạy **4 PASS, 0 FAILED** cho `FileIO`, `DateTime`, `IdGenerator` và bài thử giao dịch lịch sử. Đây **không phải** kiểm thử chức năng `MovieRepository`/`MovieService`. Hiện `MovieService` chưa có mã, nên Bước 04 vẫn **đang thực hiện**.

Bước 04 chỉ hoàn thành khi chức năng phim qua Repository và Service chạy đúng quy tắc, có kiểm thử C++ và tài liệu tương ứng. Sau đó tiếp tục các chức năng theo [lộ trình 15 bước](PhanCongCongViec.md); không cần đợi tất cả Repository mới bắt đầu Service.
