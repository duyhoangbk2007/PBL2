# Tổng quan Doubly Linked List

Nguồn đối chiếu: `core/data_structures/DoublyLinkedList.h`. Tài liệu mô tả mã hiện có, chưa phải kết quả kiểm thử.

## Ý tưởng và cấu tạo

Danh sách liên kết đôi lưu các phần tử trong những node tách rời. `ListNode<T>` chứa `data`, con trỏ `next` tới node sau và `prev` tới node trước. Lớp `DoublyLinkedList<T>` giữ con trỏ `head` tới node đầu; danh sách rỗng khi `head == nullptr`. Mã dùng `new` để tạo node và `delete` khi xóa; destructor gọi `clear()` để giải phóng các node. Vì là template, phần định nghĩa hàm nằm cùng file header.

Ví dụ sau `pushBack(10)`, `pushBack(20)`, `pushFront(5)`:

```text
head → [5] ⇄ [10] ⇄ [20] → nullptr
```

## Các thao tác đã viết

| Hàm | Việc thực hiện | Độ phức tạp theo mã hiện tại |
| --- | --- | --- |
| `pushFront(x)` | Thêm node vào đầu, cập nhật `prev` của đầu cũ. | O(1) |
| `pushBack(x)` | Đi từ `head` tới cuối rồi thêm node. | O(n) |
| `insert(x, k)` | Chèn trước vị trí `k`; cho phép `k` từ 0 đến số node. | O(n) |
| `popFront()`, `popBack()` | Xóa node đầu/cuối; danh sách rỗng thì không làm gì. | O(1), O(n) |
| `erase(k)`, `update(k, x)` | Xóa hoặc đổi dữ liệu tại vị trí `k`; vị trí không hợp lệ thì không làm gì. | O(n) |
| `search(x)`, `getAt(k)` | Tìm theo giá trị hoặc vị trí; trả con trỏ node, không thấy thì trả `nullptr`. | O(n) |
| `nodeCount()`, `show()`, `clear()` | Đếm, in và xóa toàn bộ node. | O(n) |

Chỉ số vị trí bắt đầu từ 0. `search(x)` dùng toán tử `==` của `T`; `show()` cần `T` in được bằng `std::cout`. Con trỏ trả về từ `search()`/`getAt()` sẽ không còn hợp lệ sau khi node tương ứng bị xóa hoặc danh sách bị hủy.

## Vai trò dự kiến và điểm cần kiểm tra

Theo thiết kế, danh sách có thể lưu những tập thực thể trong bộ nhớ. Việc chọn thực thể cụ thể thuộc các bước sau; header hiện chưa được nối với model hay repository.

- Lớp tự quản lý bộ nhớ động nhưng chưa định nghĩa hoặc vô hiệu hóa copy constructor/copy assignment. Sao chép mặc định sẽ sao chép `head`, có nguy cơ hai danh sách cùng sở hữu node và giải phóng lặp. Cần xử lý trước khi sao chép đối tượng.
- Chưa có kiểm thử thực tế trong lần ghi tài liệu này. Nên kiểm tra danh sách rỗng, một node, chèn/xóa đầu–giữa–cuối và chỉ số ngoài phạm vi khi đến bước CTDL.
