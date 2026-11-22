# RoomRepository — mã nguồn và quy tắc dữ liệu

## 1. Tổng quan

`RoomRepository` chứa `Repository<Room>` sở hữu các `Room*` và kiểm tra dữ liệu riêng của phòng. TXT mỗi dòng lưu ID, tên, kích thước và trạng thái. Mã phòng là khóa ổn định; việc sửa kích thước hoặc xóa phải xét các liên kết và ràng buộc được kiểm tra trong code.

Repository không phân quyền; `RoomService` xử lý quyền và nghiệp vụ tầng Business. Con trỏ Seat/Showtime liên kết với Room không thuộc quyền sở hữu của Room; không giải phóng chúng qua Room.

Mã nguồn: `src/data_access/RoomRepository.cpp` và `src/data_access/RoomRepository.h`. Các đoạn code dưới đây giữ nguyên nội dung hàm, gồm dòng trống và comment trong hàm. Khi ghép các đoạn của một hàm theo thứ tự, ta thu được đúng định nghĩa gốc.

## 2. Chi tiết từng hàm

### RoomRepository::RoomRepository()

#### A. Tổng quan

- **Mục đích:** Khởi tạo Repository và các tham chiếu/callback cần dùng.
- **Tham số:** `const string& filePath`.
- **Giá trị trả về:** không có (constructor/destructor).
- **Điều kiện quan trọng:** Các tham chiếu và callback truyền vào phải còn hợp lệ khi Repository sử dụng.

#### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Chuẩn bị giá trị cho bước kế tiếp
    |
    v
Kết thúc
```

#### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1 — Chuẩn bị giá trị cho bước kế tiếp.**

```cpp
RoomRepository::RoomRepository(const string& filePath)
    : repo(filePath, getRoomId, stringToRoom, roomToString)
{}
```

Danh sách sau dấu `:` khởi tạo thành viên trước thân constructor; tham chiếu được giữ để dùng về sau, còn thân hàm không nạp file.

### RoomRepository::getRoomId()

#### A. Tổng quan

- **Mục đích:** Lấy ID của đối tượng làm khóa.
- **Tham số:** `Room* room`.
- **Giá trị trả về:** `string`; xem các nhánh `return` bên dưới.
- **Điều kiện quan trọng:** con trỏ rỗng.

#### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Trả kết quả cho bên gọi
    |
    v
Kết thúc
```

#### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1 — Trả kết quả cho bên gọi.**

```cpp
string RoomRepository::getRoomId(Room* room)
{
    if (room == nullptr)
    {
        return "";
    }
    return room->getRoomId();
}
```

Cặp `roomId` và `seatId` xác định đúng ghế trong phòng; chỉ dùng `seatId` sẽ không đủ để phân biệt ghế ở các phòng khác nhau.

### RoomRepository::isValidRoom()

#### A. Tổng quan

- **Mục đích:** Kiểm tra tính hợp lệ của dữ liệu đầu vào.
- **Tham số:** `const Room* room`.
- **Giá trị trả về:** `bool`; xem các nhánh `return` bên dưới.
- **Điều kiện quan trọng:** con trỏ rỗng, dữ liệu hợp lệ, nhánh trả `false`.

#### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Kiểm tra dữ liệu hợp lệ (không đạt: false)
    |
    v
Kiểm tra dữ liệu hợp lệ (không đạt: false)
    |
    v
Từ chối điều kiện không đạt (không đạt: false)
    |
    v
Kết thúc
```

#### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1 — Kiểm tra dữ liệu hợp lệ.**

```cpp
bool RoomRepository::isValidRoom(const Room* room)
{
    if (room == nullptr)
    {
        return false;
    }
    if (!Validator::isValidId(room->getRoomId(), "ROM"))
    {
        return false;
```

Điều kiện hợp lệ ngăn dữ liệu sai định dạng hoặc ngoài giới hạn đi vào danh sách đang quản lý. Cặp `roomId` và `seatId` xác định đúng ghế trong phòng; chỉ dùng `seatId` sẽ không đủ để phân biệt ghế ở các phòng khác nhau. `return false` kết thúc ngay nhánh không đạt; các thao tác cập nhật phía sau không chạy.

**Bước 2 — Kiểm tra dữ liệu hợp lệ.**

```cpp
    }
    if (!Validator::isValidText(room->getName()))
    {
        return false;
    }
    if (room->getRowCount() <= 0)
    {
        return false;
```

Điều kiện hợp lệ ngăn dữ liệu sai định dạng hoặc ngoài giới hạn đi vào danh sách đang quản lý. `return false` kết thúc ngay nhánh không đạt; các thao tác cập nhật phía sau không chạy.

**Bước 3 — Từ chối điều kiện không đạt.**

```cpp
    }
    if (room->getColumnCount() <= 0)
    {
        return false;
    }
    return true;
}
```

`return false` kết thúc ngay nhánh không đạt; các thao tác cập nhật phía sau không chạy.

### RoomRepository::stringToRoom()

#### A. Tổng quan

- **Mục đích:** Chuyển một dòng TXT thành đối tượng Model.
- **Tham số:** `const string& line`.
- **Giá trị trả về:** `Room*`; xem các nhánh `return` bên dưới.
- **Điều kiện quan trọng:** dữ liệu hợp lệ, ngoại lệ.

#### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Duyệt và đối chiếu từng phần tử
    |
    v
Kiểm tra điều kiện trước khi tiếp tục (lỗi: throw)
    |
    v
Chuẩn bị giá trị cho bước kế tiếp
    |
    v
Kiểm tra điều kiện trước khi tiếp tục (lỗi: throw)
    |
    v
Bắt lỗi và khôi phục trạng thái (lỗi: throw)
    |
    v
Chuẩn bị giá trị cho bước kế tiếp
    |
    v
Kiểm tra dữ liệu hợp lệ (lỗi: throw)
    |
    v
Chuẩn bị giá trị cho bước kế tiếp
    |
    v
Kết thúc
```

#### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1 — Duyệt và đối chiếu từng phần tử.**

```cpp
Room* RoomRepository::stringToRoom(const string& line)
{
    vector<string> fields;
    size_t start = 0;
    while (true)
    {
        size_t pos = line.find('|', start);
        if (pos == string::npos)
        {
            fields.push_back(line.substr(start));
```

Vòng lặp xét từng bản ghi; điều kiện bên trong quyết định tiếp tục, bỏ qua hay dừng khi gặp dữ liệu sai.

**Bước 2 — Kiểm tra điều kiện trước khi tiếp tục.**

```cpp
            break;
        }
        fields.push_back(line.substr(start, pos - start));
        start = pos + 1;
    }
    if (fields.size() != 5)
    {
        throw runtime_error("Sai so luong truong Room");
```

`throw` báo lỗi bằng ngoại lệ cho bên gọi, khác với kết quả `false` thông thường.

**Bước 3 — Chuẩn bị giá trị cho bước kế tiếp.**

```cpp
    }
    int rowCount;
    int columnCount;
    try
    {
        size_t pos1 = 0;
        size_t pos2 = 0;

```

`try/catch` xử lý ngoại lệ trong phạm vi hiện tại; code dọn các thay đổi trước khi trả lỗi hoặc ném lại, nên thứ tự dọn có ý nghĩa.

**Bước 4 — Kiểm tra điều kiện trước khi tiếp tục.**

```cpp
        rowCount = stoi(fields[2], &pos1);
        columnCount = stoi(fields[3], &pos2);

        if (pos1 != fields[2].length() || pos2 != fields[3].length())
        {
            throw runtime_error("Sai dinh dang so");
        }
    }
```

`throw` báo lỗi bằng ngoại lệ cho bên gọi, khác với kết quả `false` thông thường. Phép parse đổi chuỗi TXT sang trường có kiểu; dữ liệu không chuyển được phải bị từ chối trước khi tạo đối tượng hợp lệ.

**Bước 5 — Bắt lỗi và khôi phục trạng thái.**

```cpp
    catch (const exception&)
    {
        throw runtime_error("So hang hoac so cot khong hop le");
    }
    // Kiem tra trang thai
    if (fields[4] != "0" && fields[4] != "1")
    {
        throw runtime_error("Trang thai Room khong hop le");
```

`try/catch` xử lý ngoại lệ trong phạm vi hiện tại; code dọn các thay đổi trước khi trả lỗi hoặc ném lại, nên thứ tự dọn có ý nghĩa. `throw` báo lỗi bằng ngoại lệ cho bên gọi, khác với kết quả `false` thông thường.

**Bước 6 — Chuẩn bị giá trị cho bước kế tiếp.**

```cpp
    }
    bool active = (fields[4] == "1");

    // Tao doi tuong Room
    Room* room = new Room(
        fields[0],
        fields[1],
        rowCount,
        columnCount,
        active
    );
```

`new` cấp phát Model trên heap. Cho đến khi Repository nhận thành công, nhánh lỗi hoặc caller vẫn phải chịu trách nhiệm giải phóng.

**Bước 7 — Kiểm tra dữ liệu hợp lệ.**

```cpp

    // Kiem tra du lieu Room
    if (!isValidRoom(room))
    {
        delete room;
        throw runtime_error("Du lieu Room khong hop le");
    }
    return room;
```

Điều kiện hợp lệ ngăn dữ liệu sai định dạng hoặc ngoài giới hạn đi vào danh sách đang quản lý. `delete` giải phóng đối tượng; không được dùng lại con trỏ sau lệnh này. `throw` báo lỗi bằng ngoại lệ cho bên gọi, khác với kết quả `false` thông thường.

**Bước 8 — Chuẩn bị giá trị cho bước kế tiếp.**

```cpp
}
```

Đoạn này chuẩn bị hoặc trả giá trị theo code. Nó chỉ đổi RAM hoặc TXT nếu có lời gọi cập nhật hay ghi dữ liệu tương ứng.

### RoomRepository::roomToString()

#### A. Tổng quan

- **Mục đích:** Chuyển đối tượng Model thành một dòng TXT.
- **Tham số:** `Room* room`.
- **Giá trị trả về:** `string`; xem các nhánh `return` bên dưới.
- **Điều kiện quan trọng:** dữ liệu hợp lệ, ngoại lệ.

#### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Kiểm tra dữ liệu hợp lệ (lỗi: throw)
    |
    v
Chuẩn bị giá trị cho bước kế tiếp
    |
    v
Kết thúc
```

#### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1 — Kiểm tra dữ liệu hợp lệ.**

```cpp
string RoomRepository::roomToString(Room* room)
{
    if (!isValidRoom(room))
    {
        throw runtime_error("Room khong hop le khi ghi file");
    }
    return room->getRoomId() + "|" +
           room->getName() + "|" +
           to_string(room->getRowCount()) + "|" +
           to_string(room->getColumnCount()) + "|" +
           (room->isActive() ? "1" : "0");
```

Điều kiện hợp lệ ngăn dữ liệu sai định dạng hoặc ngoài giới hạn đi vào danh sách đang quản lý. Cặp `roomId` và `seatId` xác định đúng ghế trong phòng; chỉ dùng `seatId` sẽ không đủ để phân biệt ghế ở các phòng khác nhau. `throw` báo lỗi bằng ngoại lệ cho bên gọi, khác với kết quả `false` thông thường.

**Bước 2 — Chuẩn bị giá trị cho bước kế tiếp.**

```cpp
}
```

Đoạn này chuẩn bị hoặc trả giá trị theo code. Nó chỉ đổi RAM hoặc TXT nếu có lời gọi cập nhật hay ghi dữ liệu tương ứng.

### RoomRepository::save()

#### A. Tổng quan

- **Mục đích:** Ghi trạng thái hiện tại ra TXT.
- **Tham số:** không có.
- **Giá trị trả về:** `bool`; xem các nhánh `return` bên dưới.
- **Điều kiện quan trọng:** không có điều kiện đặc biệt ngoài các câu lệnh bên dưới.

#### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Ghi dữ liệu TXT
    |
    v
Kết thúc
```

#### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1 — Ghi dữ liệu TXT.**

```cpp
bool RoomRepository::save()
{
    return repo.save();
}
```

Lời gọi ghi chuyển trạng thái hiện tại ra TXT. Kết quả ghi quyết định giá trị trả về; một lời gọi này không bảo đảm các file Repository khác cùng được cập nhật.

### RoomRepository::load()

#### A. Tổng quan

- **Mục đích:** Nạp và kiểm tra dữ liệu từ TXT vào RAM.
- **Tham số:** không có.
- **Giá trị trả về:** `bool`; xem các nhánh `return` bên dưới.
- **Điều kiện quan trọng:** trạng thái nạp, nhánh trả `false`.

#### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Đọc dữ liệu TXT (không đạt: false)
    |
    v
Kết thúc
```

#### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1 — Đọc dữ liệu TXT.**

```cpp
bool RoomRepository::load()
{
    if (repo.isLoaded())
    {
        return false;
    }
    return repo.load();
}
```

Lời gọi nạp đọc TXT vào bộ nhớ. Nếu trả `false`, hàm dừng ở nhánh lỗi; dữ liệu chỉ được xem là dùng được sau khi các bước kiểm tra tiếp theo hoàn tất. Trạng thái `loaded` quyết định Repository đã sẵn sàng chưa; nhánh chưa nạp dừng trước khi thao tác dựa trên dữ liệu đó. `return false` kết thúc ngay nhánh không đạt; các thao tác cập nhật phía sau không chạy.

### RoomRepository::add()

#### A. Tổng quan

- **Mục đích:** Thêm đối tượng hợp lệ vào Repository.
- **Tham số:** `Room* room`.
- **Giá trị trả về:** `bool`; xem các nhánh `return` bên dưới.
- **Điều kiện quan trọng:** trạng thái nạp, dữ liệu hợp lệ, nhánh trả `false`.

#### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Kiểm tra dữ liệu hợp lệ (không đạt: false)
    |
    v
Trả kết quả cho bên gọi
    |
    v
Kết thúc
```

#### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1 — Kiểm tra dữ liệu hợp lệ.**

```cpp
bool RoomRepository::add(Room* room)
{
    if (!repo.isLoaded() || !isValidRoom(room))
    {
        return false;
    }
    for (int i = 0; i < repo.count(); i++)
    {
        if (repo.getAt(i) == room)
        {
            return false;
```

Trạng thái `loaded` quyết định Repository đã sẵn sàng chưa; nhánh chưa nạp dừng trước khi thao tác dựa trên dữ liệu đó. Điều kiện hợp lệ ngăn dữ liệu sai định dạng hoặc ngoài giới hạn đi vào danh sách đang quản lý. `return false` kết thúc ngay nhánh không đạt; các thao tác cập nhật phía sau không chạy.

**Bước 2 — Trả kết quả cho bên gọi.**

```cpp
        }
    }
    return repo.add(room);
}
```

Đoạn này chuẩn bị hoặc trả giá trị theo code. Nó chỉ đổi RAM hoặc TXT nếu có lời gọi cập nhật hay ghi dữ liệu tương ứng.

### RoomRepository::update()

#### A. Tổng quan

- **Mục đích:** Cập nhật đối tượng đang quản lý.
- **Tham số:** `const Room& newValues`.
- **Giá trị trả về:** `bool`; xem các nhánh `return` bên dưới.
- **Điều kiện quan trọng:** trạng thái nạp, con trỏ rỗng, dữ liệu hợp lệ, nhánh trả `false`.

#### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Tra cứu đối tượng theo khóa (không đạt: false)
    |
    v
Duyệt và đối chiếu từng phần tử (không đạt: false)
    |
    v
Từ chối điều kiện không đạt (không đạt: false)
    |
    v
Xác nhận xử lý thành công
    |
    v
Kết thúc
```

#### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1 — Tra cứu đối tượng theo khóa.**

```cpp
bool RoomRepository::update(const Room& newValues)
{
    if (!repo.isLoaded() || !isValidRoom(&newValues))
    {
        return false;
    }
    Room* room = repo.findById(newValues.getRoomId());

```

Trạng thái `loaded` quyết định Repository đã sẵn sàng chưa; nhánh chưa nạp dừng trước khi thao tác dựa trên dữ liệu đó. Điều kiện hợp lệ ngăn dữ liệu sai định dạng hoặc ngoài giới hạn đi vào danh sách đang quản lý. Cặp `roomId` và `seatId` xác định đúng ghế trong phòng; chỉ dùng `seatId` sẽ không đủ để phân biệt ghế ở các phòng khác nhau.

**Bước 2 — Duyệt và đối chiếu từng phần tử.**

```cpp
    if (room == nullptr)
    {
        return false;
    }
    // Khong cho thu nho phong neu co ghe nam ngoai kich thuoc moi
    for (int i = 0; i < room->getSeatCount(); i++)
    {
        Seat* seat = room->getSeatAt(i);
```

`return false` kết thúc ngay nhánh không đạt; các thao tác cập nhật phía sau không chạy. Vòng lặp xét từng bản ghi; điều kiện bên trong quyết định tiếp tục, bỏ qua hay dừng khi gặp dữ liệu sai.

**Bước 3 — Từ chối điều kiện không đạt.**

```cpp
        if (seat == nullptr)
        {
            return false;
        }
        if (seat->getRow() > newValues.getRowCount() || seat->getColumn() > newValues.getColumnCount())
        {
            return false;
        }
```

Hàng và cột được so với kích thước phòng và các ghế khác để tránh vị trí ngoài sơ đồ hoặc trùng chỗ. `return false` kết thúc ngay nhánh không đạt; các thao tác cập nhật phía sau không chạy.

**Bước 4 — Xác nhận xử lý thành công.**

```cpp
    }
    room->setName(newValues.getName());
    room->setRowCount(newValues.getRowCount());
    room->setColumnCount(newValues.getColumnCount());
    room->setActive(newValues.isActive());
    return true;
}
```

Đoạn này chuẩn bị hoặc trả giá trị theo code. Nó chỉ đổi RAM hoặc TXT nếu có lời gọi cập nhật hay ghi dữ liệu tương ứng.

### RoomRepository::removeById()

#### A. Tổng quan

- **Mục đích:** Xóa đối tượng theo khóa sau khi kiểm tra ràng buộc.
- **Tham số:** `const string& roomId`.
- **Giá trị trả về:** `bool`; xem các nhánh `return` bên dưới.
- **Điều kiện quan trọng:** trạng thái nạp, con trỏ rỗng, nhánh trả `false`.

#### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Tra cứu đối tượng theo khóa (không đạt: false)
    |
    v
Từ chối điều kiện không đạt (không đạt: false)
    |
    v
Trả kết quả cho bên gọi
    |
    v
Kết thúc
```

#### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1 — Tra cứu đối tượng theo khóa.**

```cpp
bool RoomRepository::removeById(const string& roomId)
{
    if (!repo.isLoaded())
    {
        return false;
    }
    Room* room = repo.findById(roomId);

```

Trạng thái `loaded` quyết định Repository đã sẵn sàng chưa; nhánh chưa nạp dừng trước khi thao tác dựa trên dữ liệu đó. `return false` kết thúc ngay nhánh không đạt; các thao tác cập nhật phía sau không chạy.

**Bước 2 — Từ chối điều kiện không đạt.**

```cpp
    if (room == nullptr)
    {
        return false;
    }
    // Khong xoa phong khi con ghe hoac suat chieu
    if (room->getSeatCount() > 0 ||
        room->getShowtimeCount() > 0)
    {
        return false;
```

`return false` kết thúc ngay nhánh không đạt; các thao tác cập nhật phía sau không chạy.

**Bước 3 — Trả kết quả cho bên gọi.**

```cpp
    }
    return repo.removeById(roomId);
}
```

Đoạn này chuẩn bị hoặc trả giá trị theo code. Nó chỉ đổi RAM hoặc TXT nếu có lời gọi cập nhật hay ghi dữ liệu tương ứng.

### RoomRepository::findById()

#### A. Tổng quan

- **Mục đích:** Tra đối tượng theo ID.
- **Tham số:** `const string& roomId`.
- **Giá trị trả về:** `Room*`; xem các nhánh `return` bên dưới.
- **Điều kiện quan trọng:** không có điều kiện đặc biệt ngoài các câu lệnh bên dưới.

#### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Tra cứu đối tượng theo khóa
    |
    v
Kết thúc
```

#### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1 — Tra cứu đối tượng theo khóa.**

```cpp
Room* RoomRepository::findById(const string& roomId)
{
    return repo.findById(roomId);
}
```

Đoạn này chuẩn bị hoặc trả giá trị theo code. Nó chỉ đổi RAM hoặc TXT nếu có lời gọi cập nhật hay ghi dữ liệu tương ứng.

### RoomRepository::getAt()

#### A. Tổng quan

- **Mục đích:** Lấy phần tử theo chỉ số.
- **Tham số:** `int index`.
- **Giá trị trả về:** `Room*`; xem các nhánh `return` bên dưới.
- **Điều kiện quan trọng:** không có điều kiện đặc biệt ngoài các câu lệnh bên dưới.

#### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Trả kết quả cho bên gọi
    |
    v
Kết thúc
```

#### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1 — Trả kết quả cho bên gọi.**

```cpp
Room* RoomRepository::getAt(int index)
{
    return repo.getAt(index);
}
```

Đoạn này chuẩn bị hoặc trả giá trị theo code. Nó chỉ đổi RAM hoặc TXT nếu có lời gọi cập nhật hay ghi dữ liệu tương ứng.

### RoomRepository::count()

#### A. Tổng quan

- **Mục đích:** Trả số phần tử đang quản lý.
- **Tham số:** không có.
- **Giá trị trả về:** `int`; xem các nhánh `return` bên dưới.
- **Điều kiện quan trọng:** không có điều kiện đặc biệt ngoài các câu lệnh bên dưới.

#### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Trả kết quả cho bên gọi
    |
    v
Kết thúc
```

#### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1 — Trả kết quả cho bên gọi.**

```cpp
int RoomRepository::count()
{
    return repo.count();
}
```

Đoạn này chuẩn bị hoặc trả giá trị theo code. Nó chỉ đổi RAM hoặc TXT nếu có lời gọi cập nhật hay ghi dữ liệu tương ứng.

### RoomRepository::isLoaded()

#### A. Tổng quan

- **Mục đích:** Trả trạng thái đã nạp.
- **Tham số:** không có.
- **Giá trị trả về:** `bool`; xem các nhánh `return` bên dưới.
- **Điều kiện quan trọng:** trạng thái nạp.

#### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Trả kết quả cho bên gọi
    |
    v
Kết thúc
```

#### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1 — Trả kết quả cho bên gọi.**

```cpp
bool RoomRepository::isLoaded()
{
    return repo.isLoaded();
}
```

Trạng thái `loaded` quyết định Repository đã sẵn sàng chưa; nhánh chưa nạp dừng trước khi thao tác dựa trên dữ liệu đó.


