# Tổng quan Hash Table

Nguồn đối chiếu: `core/data_structures/HashTable.h`. Tài liệu mô tả mã hiện có, chưa phải kết quả kiểm thử.

## Ý tưởng và cấu tạo

`HashTable<T>` lưu cặp `string key` và `T value` trong mảng các bucket. Hàm `hashFunction()` duyệt từng ký tự khóa, tính giá trị lũy tiến với hệ số 31 rồi lấy phần dư cho `capacity`. Khi nhiều khóa rơi vào cùng bucket, các `HashNode<T>` được nối bằng con trỏ `next` (separate chaining). `table` là mảng con trỏ node; `count` theo dõi số khóa. Nếu truyền `capacity <= 0`, constructor dùng 10.

```text
table[0] → nullptr
table[1] → [key A, value] → [key B, value] → nullptr
```

## Các thao tác đã viết

| Hàm | Việc thực hiện |
| --- | --- |
| `insert(key, value)` | Nếu khóa đã có, cập nhật giá trị; nếu chưa có, tạo node và thêm vào đầu bucket, tăng `count`. |
| `search(key)` | Duyệt bucket tương ứng và trả con trỏ node hoặc `nullptr`. |
| `remove(key)` | Bỏ node có khóa tương ứng và giảm `count`; khóa không có thì giữ nguyên. |
| `isEmpty()`, `size()` | Kiểm tra rỗng và trả số khóa đang lưu. |
| `show()` | In các bucket và các cặp khóa–giá trị bằng `std::cout`. |
| `clear()` | Xóa mọi node trong từng bucket và đặt `count = 0`; destructor còn giải phóng mảng bucket. |

Tra cứu, thêm và xóa có thời gian kỳ vọng gần O(1) khi khóa phân bố đều và chuỗi va chạm ngắn; trường hợp nhiều khóa vào cùng bucket là O(n). Mã hiện không tự tăng kích thước bảng hoặc băm lại, nên hiệu năng có thể giảm khi `count` lớn hơn nhiều so với `capacity`. `show()` yêu cầu `T` in được; cập nhật khóa cũ yêu cầu `T` gán được.

## Vai trò dự kiến và điểm cần kiểm tra

Theo thiết kế, hash table phục vụ tra cứu `userId`, `username`, `invoiceId` trong bộ nhớ. Header hiện chưa kết nối với model, đăng nhập hoặc hóa đơn. Hàm băm này chỉ để phân bucket, không dùng để bảo vệ mật khẩu.

- Lớp sở hữu mảng và node cấp phát động nhưng chưa định nghĩa hoặc vô hiệu hóa copy constructor/copy assignment; sao chép mặc định có nguy cơ giải phóng bộ nhớ lặp.
- `search()` trả con trỏ tới node nội bộ; con trỏ mất hiệu lực khi xóa khóa, `clear()` hoặc hủy bảng.
- Chưa kiểm thử thực tế. Nên thử khóa trùng, khóa không có, va chạm, `capacity <= 0`, `clear()` rồi thêm lại khi đến bước CTDL.
