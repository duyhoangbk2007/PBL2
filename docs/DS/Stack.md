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

## Vai trò cụ thể trong dự án rạp chiếu phim

Theo `business_rules.md`, stack dự kiến lưu **mã ghế vừa chọn trong giao dịch bán vé đang diễn ra**, để hoàn tác lựa chọn gần nhất trước khi thanh toán. Ví dụ nhân viên lần lượt chọn `A01`, `A02`, `B01`: các mã được `push()` theo thứ tự đó; khi bấm Undo, lấy `top()->data` là `B01` để bỏ ghế ấy khỏi danh sách đang chọn, rồi `pop()`. Lần Undo tiếp theo bỏ `A02`. Đây là quy tắc vào sau, ra trước (LIFO).

Stack chỉ lưu **lịch sử thao tác tạm thời**; ghế đã chọn còn phải được quản lý trong trạng thái đặt vé hiện tại để hiển thị và tính tiền. Đổi suất hoặc bỏ giao dịch thì xóa các lựa chọn tạm và làm rỗng stack. `Stack<T>` hiện không có `clear()`: có thể gọi `pop()` tới khi `isEmpty()`. Sức chứa nên dựa trên số vị trí ghế có thể chọn; vì `push()` trả `void` và bỏ qua khi đầy, nơi gọi cần kiểm tra `isFull()` để không hiển thị một lựa chọn chưa được ghi vào lịch sử Undo.

**Undo chọn ghế khác với hủy vé đã bán:** hủy vé là nghiệp vụ của Admin, phải lưu lịch sử và hoàn tiền/thu hồi điểm theo quy tắc. Stack này chưa được nối với màn hình bán vé hay Service và chưa được kiểm thử thực tế.
