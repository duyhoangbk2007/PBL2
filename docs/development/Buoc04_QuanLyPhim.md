# Nhật ký Bước 04 — Quản lý phim

## Rà soát kiểm thử 2026-11-17

Codex biên dịch C++17 và chạy test trong thư mục tạm: `tests/test_movie_repository.cpp` **40/40 PASS**, `tests/test_movie_service.cpp` **45/45 PASS**. `MovieService` hiện có thêm, sửa, xóa và tra cứu với kiểm tra quyền. `updateMovie()` từ chối đổi `active`, nên nghiệp vụ bật/tắt phim chưa hoàn thiện; test đạt không đồng nghĩa toàn Bước 04 hoàn thành. Các mô tả trước ngày này bên dưới giữ giá trị lịch sử.

## Rà soát 2026-11-12

Nhánh `feature/b04-hoang-movie-management`, commit `73ebf68`: `MovieRepository` đã có CRUD, tra cứu, đọc/ghi qua `FileIO` và kiểm tra dữ liệu. Biên dịch và chạy `tests/test_movie_repository.cpp` bằng g++ C++17: 40 PASS, 0 FAILED; có kiểm tra lưu/nạp lại. `MovieService.cpp` qua kiểm tra cú pháp C++17, hiện hỗ trợ thêm, sửa và tra cứu. Service chưa có API đổi trạng thái, xóa, kiểm tra quyền hoặc test riêng. Repository chưa báo tên file/số dòng cho mọi lỗi nạp, và test chưa kiểm tra lỗi ghi. Vì vậy Bước 04 vẫn đang thực hiện. Các mô tả trạng thái bên dưới là lịch sử trước lần rà soát này.

**Mục tiêu hiện hành:** Hoàng làm lại chức năng quản lý phim từ đầu, gồm `MovieRepository`, `MovieService`, kiểm thử C++ và cập nhật tài liệu. Tên DALC chỉ còn trong phần ghi chép lịch sử bên dưới.

## Mốc lịch sử 2026-11-10

Khi chuẩn bị Bước 04 theo kế hoạch cũ, nhóm dùng tên DALC và đã có mã nháp `core/dal/dalc/MovieDALC.h/.cpp`. Hạ tầng lúc đó dùng `FileHandler` và `TransactionManager`; chưa có bằng chứng nghiệm thu hoặc kiểm thử tích hợp chức năng phim. Sau quyết định cấu trúc mới, mã nháp từng được chuyển và đổi tên thành `src/data_access/MovieRepository.h/.cpp`, đọc/ghi qua `FileIO`. Hai file Repository hiện **không còn trong cây làm việc**; code cũ chưa được nghiệm thu và Hoàng sẽ viết lại từ đầu theo yêu cầu Bước 04 hiện hành. Phần sửa `load()` đọc từ dòng đầu là ghi nhận của lần chuyển cấu trúc, không phải kết quả nghiệm thu. Mọi TXT không có schema hoặc tiêu đề.

## Mục tiêu hiện hành và phân công

**Hoàng phụ trách chính.** Thực hiện theo thứ tự: Repository → Service → kiểm thử C++ → cập nhật tài liệu. Hoàng cũng phụ trách Backend đăng nhập/phân quyền ở Bước 05. Kiệt phụ trách GUI và tích hợp từ Bước 12, có thể thống nhất trước hợp đồng dữ liệu màn hình phim. Qt Widgets chưa thuộc Bước 04.

| Thành phần | Công việc |
| --- | --- |
| `MovieRepository` | Quản lý tập `Movie` trong RAM; đọc/ghi `movies.txt` qua `FileIO`; tra cứu, kiểm tra mã trùng và định dạng TXT. |
| `MovieService` | Kiểm tra quyền Admin, dữ liệu và điều kiện thêm/sửa/xóa/ngừng phim; gọi Repository, không đọc TXT trực tiếp. |
| Kiểm thử C++ | Thử CRUD, tìm kiếm, trạng thái, mã trùng, bản ghi sai, lưu/nạp lại và lỗi ghi. |
| Tài liệu | Ghi API, quyết định và lệnh/kết quả kiểm thử thực tế trong nhật ký và Task. |

### Checklist

- [x] Chuyển bản nháp `MovieDALC` thành `MovieRepository` ở `src/data_access/`, đổi nơi gọi sang `FileIO`; kiểm tra cú pháp C++17. Đây là việc chuyển cấu trúc, chưa nghiệm thu chức năng.
- [ ] Đối chiếu API Model `Movie`, định dạng `movies.txt`, quyền sở hữu object và quan hệ với suất chiếu.
- [ ] Viết lại `MovieRepository` từ đầu để nạp, tra cứu, thêm/sửa/xóa và ghi toàn bộ `movies.txt`; báo lỗi file/dòng khi dữ liệu không hợp lệ. Không đánh dấu hoàn thành dựa trên code cũ.
- [ ] Viết `MovieService` cho nghiệp vụ phim và kiểm tra quyền; chỉ thêm API thực sự cần.
- [ ] Kiểm thử chức năng phim bằng C++ và ghi bằng chứng; kiểm tra kết quả lưu/nạp lại, không bỏ dòng đầu.
- [ ] Cập nhật tài liệu theo hành vi đã được kiểm chứng và tích hợp với `AuthService` trong phần Backend của Hoàng khi API đủ điều kiện.

`IdGenerator` hiện trả mã và `updatedSequences` nhưng không tự ghi `sequences.txt`; khi thêm phim phải giữ quy tắc cấp mã hiện hành. Ghi nhiều file tuần tự không nguyên tử: phải kiểm tra kết quả từng lần ghi, báo lỗi và không âm thầm ghi đè dữ liệu sai.

## Bằng chứng hiện có và điều kiện hoàn thành

Tại mốc chuyển thư mục trước đây, mã ứng dụng C++17 từng biên dịch và liên kết được; `tests/test_all_utils.cpp` từng chạy **4 PASS, 0 FAILED** cho `FileIO`, `DateTime`, `IdGenerator` và bài thử giao dịch lịch sử. Kết quả lịch sử đó **không phải** kiểm thử chức năng `MovieRepository`/`MovieService` và không chứng minh trạng thái build hiện nay. Ở mốc lịch sử đó, hai file Repository cũ đã bị xóa, `MovieService` chưa có mã và chưa có kiểm thử chức năng phim. Hiện `MovieRepository` đã đạt 40/40 kiểm thử riêng, `MovieService` đã có mã bước đầu nhưng chưa có test riêng; Bước 04 vẫn chưa hoàn thành.

Bước 04 chỉ hoàn thành khi chức năng phim qua Repository và Service chạy đúng quy tắc, có kiểm thử C++ và tài liệu tương ứng. Sau đó tiếp tục các chức năng theo [lộ trình 14 bước](PhanCongCongViec.md); không cần đợi tất cả Repository mới bắt đầu Service.
