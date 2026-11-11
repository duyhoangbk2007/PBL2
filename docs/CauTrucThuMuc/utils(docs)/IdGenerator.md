# IdGenerator

**Mã nguồn hiện có:** `src/utils/IdGenerator.h`, `src/utils/IdGenerator.cpp`; đã chuyển từ `core/utils/`, giữ nguyên quy tắc cấp mã.

`IdGenerator` đọc bộ đếm trong `sequences.txt` và **tính** mã kế tiếp cùng nội dung bộ đếm mới. File này chỉ chứa các dòng `entity|nextValue`, không có `#schema=1` hoặc dòng tiêu đề, giống các file TXT dữ liệu khác. Hàm hiện tại không tự ghi `sequences.txt`: bên gọi phải lưu nội dung được trả về khi tạo bản ghi mới. Nếu chỉ lấy mã mà không lưu bộ đếm, lần gọi sau có thể cấp lại mã đó.

## `generateNextId(entity, sequencesPath, id, updatedSequences)`

```cpp
static void generateNextId(const std::string& entity,
                           const std::string& sequencesPath,
                           std::string& id,
                           std::string& updatedSequences);
```

| Tham số | Ý nghĩa |
| --- | --- |
| `entity` | Tiền tố loại đối tượng: `USR`, `MOV`, `ROM`, `SHW`, `TKT`, `INV` hoặc `MEM`. |
| `sequencesPath` | Đường dẫn file bộ đếm hiện có; đường dẫn tương đối tính từ thư mục làm việc của tiến trình. |
| `id` | Đầu ra: mã mới ở dạng tiền tố + bốn chữ số, ví dụ `MOV0007`. |
| `updatedSequences` | Đầu ra: toàn bộ nội dung bộ đếm sau khi tăng dòng tương ứng, chỉ gồm các dòng `entity|nextValue` và có `\n` cuối mỗi dòng được xử lý; không có schema. |

Hàm xóa rỗng hai tham số đầu ra ngay khi bắt đầu. Nếu xử lý thành công, nó gán đồng thời mã và nội dung bộ đếm mới; nếu ném lỗi trước đó, hai tham số vẫn rỗng.

### Trình tự xử lý

1. Kiểm tra `entity` thuộc bảy tiền tố được hỗ trợ và mở `sequencesPath` bằng `std::ifstream`.
2. Đọc từng dòng, bỏ `\r` cuối dòng nếu có và bỏ qua dòng rỗng. Mỗi dòng còn lại phải có dấu `|`, phần trước là loại mã hợp lệ, phần sau chỉ gồm chữ số thập phân.
3. Chuyển phần số thành `int`, yêu cầu nằm trong khoảng `1..10000`, và từ chối loại mã lặp lại trong file.
4. Với dòng khớp `entity`, dùng giá trị hiện tại làm số của mã mới, sau đó tăng số đó lên một trong nội dung bộ đếm trả về. Giá trị `10000` nghĩa là đã hết mã cho loại này.
5. Sau khi đọc hết file, báo lỗi nếu luồng đọc bị lỗi hoặc không tìm thấy dòng của `entity`.

Ví dụ, nếu file chứa `MOV|7`, gọi với `entity = "MOV"` sẽ cho `id = "MOV0007"` và dòng tương ứng trong `updatedSequences` là `MOV|8\n`. Mã lớn nhất có thể cấp là `MOV9999`; bộ đếm sau đó là `10000`.

### Hai hàm hỗ trợ nội bộ

- `isValidEntity(entity)` trả `true` cho đúng bảy tiền tố trên. Đây là hàm `private`, không phải API cho bên ngoài.
- `formatId(entity, number)` kiểm tra `number` trong khoảng `1..9999`, thêm số 0 ở đầu để đủ bốn chữ số và ghép với tiền tố. Hàm `private` này ném `std::runtime_error` nếu số nằm ngoài phạm vi.

### Lỗi và giới hạn

Hàm ném `std::runtime_error` khi loại mã hoặc file không hợp lệ, thiếu bộ đếm, dữ liệu sai định dạng, dòng trùng, số vượt phạm vi hoặc hết mã. Nó không kiểm tra mã vừa cấp có trùng với bản ghi đã lưu hay không; tính đúng đắn phụ thuộc bộ đếm và việc lưu bộ đếm cùng dữ liệu. Nó cũng không khóa file hay tự chống hai phiên gọi cấp mã đồng thời. Quy tắc khóa bộ dữ liệu thuộc tầng ứng dụng.

Xem [quy ước `sequences.txt`](../../TongQuan/data_format.md) và [FileIO](FileHandler.md) khi tích hợp việc tạo đối tượng. Thiết kế mục tiêu ghi từng file tuần tự, không dùng `TransactionManager`.
