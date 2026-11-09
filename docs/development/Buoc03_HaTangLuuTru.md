# Bước 03 – Hạ tầng lưu trữ dữ liệu của Kiệt

Ngày cập nhật: 2026-11-07. Branch: `feature/b3-Kiet-storage`.
Theo [Task_Kiet.md](tasks/Task_Kiet.md) và [schema/phục hồi](../TongQuan/data_format.md).

## Phần đã triển khai

`TransactionManager` nhận danh sách tên file và toàn bộ nội dung mới. Ví dụ bán vé cần lưu vé, hóa đơn, điểm thành viên và bộ đếm cùng một giao dịch; khi có lỗi trước commit, tất cả trở về dữ liệu cũ.

Mỗi giao dịch dùng `data/transactions/txn-.../`. Trong đó có các bản `.new`, `.bak` và `manifest.txt`. Ghi PREPARED trước khi thay file chính, ghi COMMITTED sau khi thay đủ. `FlushFileBuffers` và `MoveFileExW` với WRITE_THROUGH thực hiện ghi/thay file trên Windows. Bản mới và backup được giữ nguyên cho đến khi commit hoặc rollback hoàn tất.

Khi đã xử lý xong, đổi tên thư mục thành `done-...` trước khi dọn. Nếu dọn bị gián đoạn thì lần phục hồi sau có thể tiếp tục dọn mà không cần đọc lại các file đã xóa. Nếu giao dịch đã commit nhưng chưa đổi tên được, vẫn trả thành công và chặn giao dịch mới cho đến khi `recover()` xử lý xong.

## Cách gọi

Sau khi ứng dụng giữ khóa độc quyền bộ dữ liệu, gọi `recover()` trước khi tải các file nghiệp vụ. Khóa bộ dữ liệu thuộc luồng khởi động, chưa được cài trong lớp này. Chỉ cập nhật dữ liệu trong bộ nhớ và báo thành công sau khi `commit()` trả true.

```cpp
TransactionManager transactions("data");
std::string error;
if (!transactions.recover(error)) {
    // Hiển thị error và dừng tải dữ liệu.
    return;
}

// Tải dữ liệu, kiểm tra schema và chuẩn bị nội dung mới.
std::string movieId, newSequences;
if (!IdGenerator::taoMaTiepTheo("MOV", "data/sequences.txt",
                                movieId, newSequences, error)) {
    return;
}

// newMovies là TOÀN BỘ movies.txt mới, có dòng #schema=1.
if (!transactions.commit({{"movies.txt", newMovies},
                          {"sequences.txt", newSequences}}, error)) {
    // Chưa cập nhật bộ nhớ, hiển thị error.
    return;
}
// Cập nhật bộ nhớ và báo thành công.
```

`IdGenerator` chưa ghi file. Mã chỉ được sử dụng cho dữ liệu nghiệp vụ sau khi giao dịch lưu thành công. Khi cần cấp nhiều mã cùng loại trong một giao dịch, `SequenceDALC` ở Bước 04 cần quản lý bộ đếm trong bộ nhớ; gọi lại hàm hiện tại trước khi lưu sẽ đọc lại số cũ.

## Giải thích các hàm của TransactionManager

| Hàm | Tác dụng |
|---|---|
| Constructor | Nhận thư mục dữ liệu và lưu đường dẫn tuyệt đối. |
| `recover` | Dọn phần chuẩn bị chưa thay dữ liệu, rollback PREPARED, kiểm tra/giữ COMMITTED; chặn ghi khi lỗi. |
| `commit` | Kiểm tra danh sách file, lưu bản mới/backup, ghi manifest và thay các file chính; gọi phục hồi khi lỗi trước commit. |
| `validName` | Chỉ chấp nhận chín tên file TXT của dự án, tránh ghi ra ngoài bộ dữ liệu. |
| `readFile` | Đọc toàn bộ nội dung; báo lỗi nếu file thiếu hoặc không hợp lệ. |
| `writeFile` | Ghi toàn bộ nội dung và gọi API Windows để đẩy dữ liệu xuống đĩa. |
| `replaceFile` | Ghi nội dung qua file tạm rồi thay file đích; dùng chung cho dữ liệu và manifest. |
| `readManifest` | Kiểm tra manifest và các bản cần phục hồi trước khi thay dữ liệu. |
| `cleanup` | Đánh dấu thư mục đã xử lý bằng tên `done-...`, sau đó dọn dữ liệu tạm. |

Đã rút gọn: bỏ `Entry`, `writeManifest`, `installFile` và `rollback` riêng; dùng `map` lưu trạng thái tồn tại của file, `replaceFile` dùng chung cho thao tác thay file, rollback nằm trong `recover`. Mỗi nội dung ghi được giới hạn tối đa 4 GB, phù hợp các file TXT của dự án.

## Bằng chứng kiểm thử

Biên dịch bằng g++ trên Windows, C++17 với `-Wall -Wextra -pedantic`, không có cảnh báo. Chạy ngày 2026-11-07 trả `All transaction tests passed`.

```powershell
g++ -std=c++17 -Wall -Wextra -pedantic tests/transaction_manager_test.cpp core/utils/TransactionManager.cpp core/utils/IdGenerator.cpp -o "$env:TEMP/pbl2_transaction_manager_test.exe"
& "$env:TEMP/pbl2_transaction_manager_test.exe"
```

Các ca gồm: lưu nhiều file; khóa file thứ hai để gây lỗi sau khi đã thay file thứ nhất và kiểm tra rollback; từ chối đường dẫn lạ/tên trùng; dọn vùng chưa PREPARED; phục hồi PREPARED tại các điểm trước, giữa và sau hai lần thay file; xóa file vốn chưa tồn tại; rollback bị khóa rồi thử lại; thiếu backup; manifest hỏng; giữ dữ liệu COMMITTED; tiếp tục dọn; cấp mã và lưu bộ đếm cùng dữ liệu.

Các ca gián đoạn mô phỏng trạng thái file còn lại khi tiến trình dừng; chưa thử mất điện thực tế. Bộ kiểm thử chỉ dùng thư mục riêng dưới thư mục tạm.

## IdGenerator — cập nhật ngày 2026-11-09

Viết lại theo cách dùng mảng, vòng lặp và chuỗi cơ bản. `taoMaTiepTheo` thực hiện bốn bước: mở file bộ đếm, đọc từng dòng, tạo mã rồi tăng bộ đếm, ghép nội dung mới. Tên hàm và biến dùng tiếng Việt không dấu.

- `timViTriLoaiMa`: tìm vị trí của loại mã trong mảng 7 loại được hỗ trợ.
- `docSoDuong`: đọc từng chữ số, từ chối ký tự lạ, số 0 và số vượt giới hạn `long long`.
- `boDem[i]` lưu số tiếp theo của `CAC_LOAI_MA[i]`; số 0 đánh dấu loại chưa có trong file và giúp phát hiện dòng trùng.

Chỉ dùng thư viện chuẩn `string`, `fstream`. Hằng số `GIOI_HAN_BO_DEM = 9223372036854775807LL` dùng để kiểm tra giới hạn số trước khi nhân, cộng và tăng bộ đếm. Hàm trả mã và nội dung mới để `TransactionManager` lưu; không tự ghi file.

Kiểm thử phiên bản này bằng C++17 với `-Wall -Wextra -pedantic` đạt, không có cảnh báo. Các ca gồm định dạng mã, tăng bộ đếm sau khi lưu, không tự lưu file, CRLF, dòng trống, số sai định dạng, dòng trùng, loại mã lạ, thiếu bộ đếm và giới hạn `long long`.

```powershell
g++ -std=c++17 -Wall -Wextra -pedantic tests/id_generator_test.cpp core/utils/IdGenerator.cpp -o "$env:TEMP/pbl2_id_generator_test.exe"
& "$env:TEMP/pbl2_id_generator_test.exe" "$env:TEMP/pbl2_id_generator_fixture.txt"
```

## DateTime

`DateTime` có bốn hàm: `chuoiThanhNgay`/`chuoiThanhNgayGio` chuyển chuỗi thành `std::tm`; `ngayThanhChuoi`/`ngayGioThanhChuoi` chuyển ngược về chuỗi. Hai hàm phụ `matches` kiểm tra chữ số/dấu phân cách và `validDate` kiểm tra ngày thực tế/năm nhuận. Parse lỗi trả false và giữ nguyên kết quả; format lỗi trả chuỗi rỗng.

Ngày dùng `YYYY-MM-DD`, thời điểm dùng `YYYY-MM-DDTHH:mm:ss`, năm 0001–9999. Các trường biểu diễn giờ Việt Nam UTC+07:00, không chuyển đổi múi giờ hay dùng `mktime`. Trong `std::tm`, `tm_year` là năm trừ 1900, `tm_mon` từ 0 đến 11; phần parse chỉ điền các trường ngày/giờ, không tính thứ trong tuần/ngày thứ mấy trong năm.

Kiểm thử ngày 2026-11-07 trả `All datetime tests passed`: ngày nhuận/thế kỷ, ngày không tồn tại, định dạng sai, giờ vượt giới hạn, parse/format hai chiều và giữ nguyên kết quả khi parse lỗi. Biên dịch C++17 không có cảnh báo.

```powershell
g++ -std=c++17 -Wall -Wextra -pedantic tests/datetime_test.cpp core/utils/DateTime.cpp -o "$env:TEMP/pbl2_datetime_test.exe"
& "$env:TEMP/pbl2_datetime_test.exe"
```

## Phần còn lại

Review API với `FileHandler`/`Validator` của Hoàng và tích hợp DALC chưa hoàn thành. Chưa đánh dấu Bước 03 chung hoàn thành.

## Rà soát toàn bộ Bước 03 ngày 2026-11-07

- `IdGenerator`: dùng `from_chars` đọc số đúng định dạng, từ chối dấu `+`, khoảng trắng, số không dương, tràn và entity trùng. Báo file/dòng khi một dòng dữ liệu sai. Bỏ `iomanip`/`sstream`, dùng chuỗi để thêm số 0 và dựng nội dung mới.
- `DateTime`: cố định locale khi định dạng để năm 2026 không bị ghi thành `2.026` trên máy có phân nhóm chữ số.
- `TransactionManager`: PREPARED chỉ cần đủ backup để rollback; COMMITTED chỉ kiểm tra dữ liệu mới và không yêu cầu backup thừa. Chụp danh sách thư mục trước khi dọn. File vốn không tồn tại được chuyển ra khỏi bộ dữ liệu bằng thao tác WRITE_THROUGH khi rollback, rồi mới dọn.
- Bổ sung kiểm thử riêng cho mã, kiểm thử locale, rollback gián đoạn, kiểm tra đủ backup trước khi phục hồi và lỗi dữ liệu COMMITTED. Cả ba bộ kiểm thử chạy đạt với C++17, `-Wall -Wextra -pedantic`, không có cảnh báo.

Lệnh chạy kiểm thử mã bổ sung:

```powershell
g++ -std=c++17 -Wall -Wextra -pedantic tests/id_generator_test.cpp core/utils/IdGenerator.cpp -o "$env:TEMP/pbl2_id_generator_test.exe"
& "$env:TEMP/pbl2_id_generator_test.exe"
```

Ranh giới trách nhiệm: `Validator`/DALC kiểm tra dữ liệu nghiệp vụ, bộ đếm lớn hơn mọi mã hiện có và file bắt buộc không bị thiếu; `TransactionManager` chỉ lưu/phục hồi nhóm file. Khóa độc quyền bộ dữ liệu nằm ở luồng khởi động. Chưa có `FileHandler`/`Validator` trong cây mã hiện tại để review tích hợp; không đánh dấu mục đó hoàn thành.
