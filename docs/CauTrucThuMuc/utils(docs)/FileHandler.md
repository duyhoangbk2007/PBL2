# FileHandler

**Mã nguồn:** `core/utils/FileHandler.h`, `core/utils/FileHandler.cpp`.

`FileHandler` là tiện ích I/O đơn giản cho file văn bản. Hai hàm đều là `static`, trả về `bool` để báo thành công hoặc thất bại; chúng không biết định dạng bản ghi nghiệp vụ, không tạo thư mục cha và không quản lý giao dịch nhiều file. Các lớp DALC dự kiến gọi tiện ích này khi đọc/ghi dữ liệu.

## `readlines(path, lines)`

```cpp
static bool readlines(const std::string& path,
                      std::vector<std::string>& lines);
```

- `path`: đường dẫn file cần đọc. Đường dẫn tương đối được hiểu theo thư mục làm việc của tiến trình.
- `lines`: tham số đầu ra. Hàm mở file bằng `std::ifstream`, đọc lần lượt bằng `std::getline` vào một vector tạm. Ký tự xuống dòng không nằm trong các chuỗi trả về.
- Trả `true` khi mở và đọc xong; khi đó `lines` được **thay toàn bộ** bằng nội dung vừa đọc. File rỗng cho kết quả là vector rỗng.
- Trả `false` nếu không mở được file hoặc luồng đọc báo lỗi `bad()`. Trong hai trường hợp này, `lines` giữ nguyên nội dung cũ vì dữ liệu chỉ được `swap` sau khi đọc thành công.

Hàm không tách trường theo dấu `|`, không kiểm tra schema và không tự coi file thiếu là dữ liệu rỗng. Bên gọi phải tự phân biệt lỗi đọc với một file rỗng hợp lệ.

## `writeLines(path, lines)`

```cpp
static bool writeLines(const std::string& path,
                       const std::vector<std::string>& lines);
```

- `path`: file đích; thư mục cha phải tồn tại và có quyền ghi.
- `lines`: các dòng cần ghi. Hàm mở bằng `std::ofstream` ở chế độ mặc định, tức tạo file mới hoặc **xóa nội dung file cũ** trước khi ghi.
- Mỗi chuỗi được ghi kèm `\n`, kể cả chuỗi cuối. Vector rỗng tạo file rỗng.
- Trả `false` nếu mở file, ghi một dòng hoặc đóng file thất bại; trả `true` sau khi đóng thành công.

Nếu ghi lỗi giữa chừng, file đích có thể đã bị cắt ngắn hoặc chỉ chứa một phần dữ liệu. Vì vậy, không gọi `writeLines` trực tiếp lên file nghiệp vụ khi thao tác cần cập nhật nhiều file nhất quán; luồng đó cần `TransactionManager` và vẫn phải xét các giới hạn của lớp này. Hàm không thực hiện `fsync` hay xác nhận dữ liệu đã bền vững sau mất điện.

## Ví dụ sử dụng

```cpp
std::vector<std::string> lines;
if (!FileHandler::readlines("data/movies.txt", lines)) {
    // Báo lỗi đọc; không sử dụng lines như dữ liệu mới.
}

if (!FileHandler::writeLines("data/example.txt", {"MOV0001|Ten phim"})) {
    // Báo lỗi ghi; file đích có thể đã bị thay đổi một phần.
}
```

Ví dụ chỉ minh họa API. Nội dung bản ghi và đường dẫn nghiệp vụ phải tuân theo [định dạng dữ liệu](../../TongQuan/data_format.md).
