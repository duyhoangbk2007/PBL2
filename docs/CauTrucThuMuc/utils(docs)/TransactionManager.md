# TransactionManager

**Mã nguồn:** `core/utils/TransactionManager.h`, `core/utils/TransactionManager.cpp`. Lớp này phối hợp `FileHandler` và `std::filesystem` để thay nhiều file TXT trong một lần gọi, giữ bản sao cũ để thử rollback và phục hồi khi khởi động. Đây là **cách cài đặt hiện tại**, chưa đạt toàn bộ yêu cầu bền vững trong [thiết kế định dạng dữ liệu](../../TongQuan/data_format.md).

## Dữ liệu vào và API công khai

```cpp
struct TransactionFile {
    std::string path;
    std::vector<std::string> newLines;
};

static void execute(const std::vector<TransactionFile>& files);
static void recover(const std::string& transactionDir = "data/transactions");
```

Mỗi `TransactionFile` chỉ định file gốc và **toàn bộ nội dung mới** theo từng dòng. `execute` không tự tính phần thay đổi, không kiểm tra schema hay quy tắc nghiệp vụ. Đường dẫn tương đối được hiểu theo thư mục làm việc của tiến trình. Các hàm trả về `void`; lỗi được báo bằng exception, chủ yếu là `std::runtime_error` hoặc exception của `std::filesystem`.

### `execute(files)`

1. Từ chối danh sách rỗng; tạo `data/transactions/txn_1`, `txn_2`... bằng tên thư mục đầu tiên chưa tồn tại.
2. Gọi `prepareFiles`: tạo `new/` và `backup/`; ghi bản mới bằng `FileHandler::writeLines` và sao chép file gốc sang `backup/` nếu file gốc tồn tại.
3. Gọi `writeManifest(..., "PREPARED")`: ghi trạng thái, số lượng file và mỗi dòng `đường_dẫn_gốc|1` hoặc `|0` tùy file gốc có tồn tại lúc ghi manifest.
4. Gọi `replaceFiles`: kiểm tra file mới và backup cần thiết, rồi sao chép từng file mới đè lên file gốc. Khi biên dịch với `TEST_ROLLBACK`, mã cố ý ném lỗi sau khi thay file đầu tiên để thử rollback.
5. Gọi `writeManifest(..., "COMMITTED")`: đọc manifest `PREPARED`, thay dòng trạng thái thành `COMMITTED`, ghi `manifest.tmp` rồi đổi tên thành `manifest.txt`.
6. Dọn thư mục giao dịch sau khi hoàn tất. Lỗi dọn được nhận qua `std::error_code` nhưng hiện không được báo ra ngoài.

Nếu phát sinh exception sau khi `PREPARED` được ghi và cờ `prepared` đã bật, `execute` gọi `rollback`, thử dọn thư mục rồi ném lại lỗi. Nếu rollback thất bại, hàm ném lỗi “Rollback that bai, can kiem tra du lieu” và giữ thư mục giao dịch để kiểm tra/phục hồi. Lỗi trước thời điểm đó chỉ được thử dọn thư mục tạm.

### `recover(transactionDir)`

Hàm duyệt các thư mục con của `transactionDir`; nếu thư mục không tồn tại thì kết thúc. Mỗi thư mục phải có `manifest.txt` đọc được và có ít nhất hai dòng:

- `PREPARED`: gọi `rollback` để chép backup về file đã tồn tại trước giao dịch, hoặc xóa file mới nếu trước đó chưa tồn tại.
- `COMMITTED`: giữ các file hiện tại; không chép backup trở lại.
- Trạng thái khác hoặc thiếu manifest: ném lỗi và dừng phục hồi.

Sau khi xử lý một thư mục, hàm thử xóa nó. `recover` in tiến trình ra `std::cout`. Ứng dụng dự kiến gọi phục hồi **trước khi nạp dữ liệu nghiệp vụ**; phần lấy khóa độc quyền bộ dữ liệu chưa được cài ở lớp này.

## Các hàm `private`

| Hàm | Vai trò thực tế |
| --- | --- |
| `prepareFiles(files, transactionPath)` | Tạo thư mục `new/`, `backup/`; ghi nội dung mới và sao lưu từng file gốc đang tồn tại. |
| `writeManifest(files, transactionPath, status)` | Với `PREPARED`, tạo manifest; với `COMMITTED`, đọc manifest cũ, đổi trạng thái và đổi tên file tạm. Từ chối trạng thái khác. |
| `replaceFiles(files, transactionPath)` | Kiểm tra file tạm và backup, rồi `copy_file(..., overwrite_existing)` từng file đích. |
| `rollback(transactionPath)` | Đọc manifest `PREPARED`; phục hồi từ backup cho file cũ, xóa file mới với cờ `0`. |

## Giới hạn cần biết trước khi tích hợp

- `replaceFiles` dùng sao chép đè từng file, nên việc thay file **không nguyên tử**. Mã không gọi `fsync`/flush bền vững cho file và thư mục; chưa có bảo đảm khi mất điện đột ngột.
- `new/` và `backup/` đặt tên theo **tên file cuối đường dẫn**. Hai đường dẫn khác nhau cùng tên trong một giao dịch sẽ đụng nhau; bên gọi hiện phải tránh trường hợp này.
- Nếu tiến trình dừng sau khi tạo thư mục giao dịch nhưng trước khi có manifest, `recover` hiện ném lỗi “Thiếu manifest” thay vì tự nhận diện và dọn vùng chuẩn bị dở.
- Cờ “file gốc đã tồn tại” được xác định lúc ghi manifest, sau bước sao lưu. Mã chưa khóa bộ dữ liệu; thay đổi file từ tiến trình khác trong khoảng này có thể làm backup và cờ không khớp.
- Nếu một lỗi xảy ra **sau khi manifest đã thành `COMMITTED`**, nhánh xử lý lỗi của `execute` vẫn thử `rollback`, nhưng `rollback` chỉ chấp nhận `PREPARED`; lúc đó cần kiểm tra trạng thái dữ liệu trước khi tiếp tục. Trên một số hệ thống, thao tác đổi tên `manifest.tmp` đè lên `manifest.txt` cũng có thể thất bại.
- Việc dọn thất bại không được báo. Manifest `COMMITTED` được `recover` dọn mà chưa đối chiếu nội dung các file đích. Lớp cũng chưa xác thực đầy đủ số dòng manifest, đường dẫn hoặc backup hỏng.

Vì vậy, khi ghép vào DALC/BookingService, chỉ báo nghiệp vụ thành công sau khi lưu đạt yêu cầu của hệ thống và xử lý rõ exception; không suy ra rằng lớp hiện tại đã bảo đảm giao dịch bền vững trong mọi sự cố.
