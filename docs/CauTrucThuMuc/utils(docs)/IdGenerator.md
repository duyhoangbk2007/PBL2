# IdGenerator

Mã nguồn: `src/utils/IdGenerator.h/.cpp`. API hiện hành:

```cpp
static std::string generateNextId(const std::string& entity,
                                  const std::vector<std::string>& existingIds);
```

Hàm nhận các ID **đang có trong bộ nhớ**, kiểm tra tất cả ID đúng tiền tố và dạng ba chữ cái + bốn chữ số, lấy phần số lớn nhất rồi cộng một. Danh sách rỗng bắt đầu bằng `0001`. Các tiền tố hợp lệ là `USR`, `MOV`, `ROM`, `SHW`, `TKT`, `INV`, `MEM`; mã lớn nhất là `9999`. Hàm ném `std::runtime_error` nếu tiền tố/ID sai hoặc hết dải.

Hàm không đọc hay ghi `sequences.txt`. Nếu xóa bản ghi có ID lớn nhất, số đó có thể được cấp lại; các khoảng trống thấp hơn không được lấp. Các Service tạo MOV/ROM/SHW/USR/MEM và `BookingService` tạo INV/TKT hiện gọi hàm này. Lưu bản ghi TXT là thao tác riêng; xem [định dạng dữ liệu](../../TongQuan/data_format.md).

Các bài kiểm thử tiện ích và `test_auto_id_services_v2.cpp` kiểm tra cơ chế hiện tại; kết quả chạy được ghi ở tài liệu tiến độ.
