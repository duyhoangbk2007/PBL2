# Tổng quan Stack

Nguồn đối chiếu: `core/data_structures/Stack.h`. Tài liệu mô tả mã hiện có, chưa phải kết quả kiểm thử.

## Ý tưởng và cấu tạo

Stack hoạt động theo nguyên tắc **vào sau, ra trước** (LIFO). `StackNode<T>` chứa `data` và con trỏ `next`; `Stack<T>` giữ `topNode`, `capacity` và `count`. Mỗi `push()` thêm node ở đỉnh, `pop()` bỏ node ở đỉnh. Constructor biến sức chứa âm thành 0; destructor gọi `pop()` cho đến khi rỗng.

```text
push(A), push(B): top → [B] → [A] → nullptr
pop():          top → [A] → nullptr
```

## Các thao tác đã viết

| Hàm | Việc thực hiện | Độ phức tạp |
| --- | --- | --- |
| `push(x)` | Thêm `x` nếu chưa đầy; nếu đầy thì không làm gì. | O(1) |
| `pop()` | Xóa phần tử trên cùng; nếu rỗng thì không làm gì. | O(1) |
| `top()` | Trả con trỏ node trên cùng hoặc `nullptr` nếu rỗng; không xóa node. | O(1) |
| `isEmpty()`, `isFull()`, `size()` | Kiểm tra trạng thái và trả số phần tử. | O(1) |
| `show()` | In từ đỉnh xuống đáy bằng `std::cout`. | O(n) |

`capacity = 0` làm stack luôn đầy, nên `push()` không thêm được phần tử. `top()` trả con trỏ nội bộ; sau `pop()` hoặc hủy stack, không được tiếp tục dùng con trỏ đó. `show()` cần `T` in được.

## Vai trò dự kiến và điểm cần kiểm tra

Theo thiết kế, stack sẽ lưu lịch sử chọn ghế để hoàn tác thao tác gần nhất trước thanh toán. Header hiện chỉ là cấu trúc dữ liệu tổng quát; chưa có chức năng bán vé hay undo được nối vào.

- Lớp sở hữu node cấp phát động nhưng chưa định nghĩa hoặc vô hiệu hóa copy constructor/copy assignment; sao chép mặc định có nguy cơ giải phóng node lặp.
- `push()` và `pop()` không trả trạng thái thành công/thất bại; nơi gọi phải kiểm tra `isFull()`/`isEmpty()` nếu cần thông báo cho người dùng.
- Chưa kiểm thử thực tế. Nên thử stack rỗng, đầy, `capacity = 0`, thứ tự LIFO và thao tác sau khi `clear` tương đương bằng cách `pop()` nhiều lần khi đến bước CTDL.
