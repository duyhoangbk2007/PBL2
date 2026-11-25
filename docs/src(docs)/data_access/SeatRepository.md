# SeatRepository — mã nguồn và quy tắc dữ liệu

## 1. Tổng quan

`SeatRepository` chứa `Repository<Seat>` sở hữu ghế và giữ `RoomRepository&` không sở hữu để tra phòng, nối/tháo liên kết. Khóa ghế là cặp `(roomId, seatId)`, vị trí `(roomId, row, column)` không được trùng; một ghế đôi vẫn là một Seat.

TXT dùng `roomId|seatId|row|column|seatType`. `load()` cần Room đã nạp; khi nối lỗi phải gỡ các liên kết đã tạo theo code. Trạng thái đã bán không nằm trong Seat mà do Ticket theo suất chiếu quyết định. Con trỏ trả về từ tra cứu là con trỏ mượn; Repository không phân quyền.

Mã nguồn: `src/data_access/SeatRepository.cpp` và `src/data_access/SeatRepository.h`. Các đoạn code dưới đây giữ nguyên nội dung hàm, gồm dòng trống và comment trong hàm. Khi ghép các đoạn của một hàm theo thứ tự, ta thu được đúng định nghĩa gốc.

## 2. Chi tiết từng hàm

### SeatRepository::SeatRepository()

#### A. Tổng quan

- **Mục đích:** Khởi tạo Repository và các tham chiếu/callback cần dùng.
- **Tham số:** `const string& filePath, RoomRepository& roomRepo`.
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
SeatRepository::SeatRepository(const string& filePath, RoomRepository& roomRepo)
    : repo(filePath, getSeatKey, stringToSeat, seatToString), roomRepo(roomRepo)
{}
```

Danh sách sau dấu `:` khởi tạo thành viên trước thân constructor; tham chiếu được giữ để dùng về sau, còn thân hàm không nạp file.

### SeatRepository::~SeatRepository()

#### A. Tổng quan

- **Mục đích:** Tháo liên kết hoặc giải phóng tài nguyên khi hủy Repository.
- **Tham số:** không có.
- **Giá trị trả về:** không có (constructor/destructor).
- **Điều kiện quan trọng:** Các Repository được tham chiếu cần còn sống đến khi destructor tháo liên kết.

#### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Tháo liên kết đối tượng
    |
    v
Chuẩn bị giá trị cho bước kế tiếp
    |
    v
Kết thúc
```

#### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1 — Tháo liên kết đối tượng.**

```cpp
SeatRepository::~SeatRepository()
{
    for (int i = 0; i < repo.count(); i++)
    {
        Seat* seat = repo.getAt(i);
        if (seat != nullptr && seat->getRoom() != nullptr)
        {
            seat->getRoom()->unlinkSeat(seat);
```

Liên kết được đổi theo thứ tự trong code; nếu bước sau thất bại, phần đã nối phải được tháo trước khi xóa đối tượng để tránh con trỏ treo. Vòng lặp xét từng bản ghi; điều kiện bên trong quyết định tiếp tục, bỏ qua hay dừng khi gặp dữ liệu sai.

**Bước 2 — Chuẩn bị giá trị cho bước kế tiếp.**

```cpp
        }
    }
}
```

Destructor chạy khi Repository hết vòng đời; việc tháo liên kết hoặc `clear()` giữ các đối tượng khác không trỏ đến vùng nhớ đã giải phóng.

### SeatRepository::getSeatKey()

#### A. Tổng quan

- **Mục đích:** Tạo khóa ghép phòng và ghế.
- **Tham số:** `Seat* seat`.
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
string SeatRepository::getSeatKey(Seat* seat) //(roomId + seatId)
{
    if (seat == nullptr)
    {
        return "";
    }
    return seat->getRoomId() + "|" + seat->getSeatId();
}
```

Cặp `roomId` và `seatId` xác định đúng ghế trong phòng; chỉ dùng `seatId` sẽ không đủ để phân biệt ghế ở các phòng khác nhau.

### SeatRepository::isValidSeat()

#### A. Tổng quan

- **Mục đích:** Kiểm tra tính hợp lệ của dữ liệu đầu vào.
- **Tham số:** `const Seat* seat`.
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
Từ chối điều kiện không đạt (không đạt: false)
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

**Bước 1 — Kiểm tra dữ liệu hợp lệ.**

```cpp
bool SeatRepository::isValidSeat(const Seat* seat)
{
    if (seat == nullptr)
    {
        return false;
    }
    if (!Validator::isValidId(seat->getRoomId(), "ROM"))
    {
        return false;
```

Điều kiện hợp lệ ngăn dữ liệu sai định dạng hoặc ngoài giới hạn đi vào danh sách đang quản lý. Cặp `roomId` và `seatId` xác định đúng ghế trong phòng; chỉ dùng `seatId` sẽ không đủ để phân biệt ghế ở các phòng khác nhau. `return false` kết thúc ngay nhánh không đạt; các thao tác cập nhật phía sau không chạy.

**Bước 2 — Từ chối điều kiện không đạt.**

```cpp
    }
    if (seat->getSeatId().empty() || seat->getSeatId().find('|') != string::npos)
    {
        return false;
    }
    if (seat->getRow() <= 0 || seat->getColumn() <= 0)
    {
        return false;
```

Hàng và cột được so với kích thước phòng và các ghế khác để tránh vị trí ngoài sơ đồ hoặc trùng chỗ. Cặp `roomId` và `seatId` xác định đúng ghế trong phòng; chỉ dùng `seatId` sẽ không đủ để phân biệt ghế ở các phòng khác nhau. `return false` kết thúc ngay nhánh không đạt; các thao tác cập nhật phía sau không chạy.

**Bước 3 — Từ chối điều kiện không đạt.**

```cpp
    }
    SeatType type = seat->getSeatType();
    if (type != SeatType::STANDARD &&
        type != SeatType::VIP &&
        type != SeatType::COUPLE)
    {
        return false;
    }
```

`return false` kết thúc ngay nhánh không đạt; các thao tác cập nhật phía sau không chạy.

**Bước 4 — Xác nhận xử lý thành công.**

```cpp
    return true;
}
```

Đoạn này chuẩn bị hoặc trả giá trị theo code. Nó chỉ đổi RAM hoặc TXT nếu có lời gọi cập nhật hay ghi dữ liệu tương ứng.

### SeatRepository::stringToSeat()

#### A. Tổng quan

- **Mục đích:** Chuyển một dòng TXT thành đối tượng Model.
- **Tham số:** `const string& line`.
- **Giá trị trả về:** `Seat*`; xem các nhánh `return` bên dưới.
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
Kiểm tra điều kiện trước khi tiếp tục
    |
    v
Kiểm tra điều kiện trước khi tiếp tục (lỗi: throw)
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
Seat* SeatRepository::stringToSeat(const string& line)
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
        throw runtime_error("Sai so luong truong Seat");
```

`throw` báo lỗi bằng ngoại lệ cho bên gọi, khác với kết quả `false` thông thường.

**Bước 3 — Chuẩn bị giá trị cho bước kế tiếp.**

```cpp
    }
    int row;
    int column;
    try
    {
        size_t pos1 = 0;
        size_t pos2 = 0;
```

`try/catch` xử lý ngoại lệ trong phạm vi hiện tại; code dọn các thay đổi trước khi trả lỗi hoặc ném lại, nên thứ tự dọn có ý nghĩa.

**Bước 4 — Kiểm tra điều kiện trước khi tiếp tục.**

```cpp
        row = stoi(fields[2], &pos1);
        column = stoi(fields[3], &pos2);

        if (pos1 != fields[2].length() ||
            pos2 != fields[3].length())
        {
            throw runtime_error("Sai dinh dang so");
        }
```

`throw` báo lỗi bằng ngoại lệ cho bên gọi, khác với kết quả `false` thông thường. Phép parse đổi chuỗi TXT sang trường có kiểu; dữ liệu không chuyển được phải bị từ chối trước khi tạo đối tượng hợp lệ.

**Bước 5 — Bắt lỗi và khôi phục trạng thái.**

```cpp
    }
    catch (const exception&)
    {
        throw runtime_error("Hang hoac cot Seat khong hop le");
    }

    SeatType seatType;
```

`try/catch` xử lý ngoại lệ trong phạm vi hiện tại; code dọn các thay đổi trước khi trả lỗi hoặc ném lại, nên thứ tự dọn có ý nghĩa. `throw` báo lỗi bằng ngoại lệ cho bên gọi, khác với kết quả `false` thông thường.

**Bước 6 — Kiểm tra điều kiện trước khi tiếp tục.**

```cpp
    if (fields[4] == "STANDARD")
    {
        seatType = SeatType::STANDARD;
    }
    else if (fields[4] == "VIP")
    {
        seatType = SeatType::VIP;
    }
```

Đoạn này chuẩn bị hoặc trả giá trị theo code. Nó chỉ đổi RAM hoặc TXT nếu có lời gọi cập nhật hay ghi dữ liệu tương ứng.

**Bước 7 — Kiểm tra điều kiện trước khi tiếp tục.**

```cpp
    else if (fields[4] == "COUPLE")
    {
        seatType = SeatType::COUPLE;
    }
    else
    {
        throw runtime_error("Loai ghe khong hop le");
    }
```

`throw` báo lỗi bằng ngoại lệ cho bên gọi, khác với kết quả `false` thông thường.

**Bước 8 — Chuẩn bị giá trị cho bước kế tiếp.**

```cpp

    Seat* seat = new Seat(
        fields[0],
        fields[1],
        row,
        column,
        seatType
    );
```

`new` cấp phát Model trên heap. Cho đến khi Repository nhận thành công, nhánh lỗi hoặc caller vẫn phải chịu trách nhiệm giải phóng.

**Bước 9 — Kiểm tra dữ liệu hợp lệ.**

```cpp

    if (!isValidSeat(seat))
    {
        delete seat;
        throw runtime_error("Du lieu Seat khong hop le");
    }

    return seat;
```

Điều kiện hợp lệ ngăn dữ liệu sai định dạng hoặc ngoài giới hạn đi vào danh sách đang quản lý. `delete` giải phóng đối tượng; không được dùng lại con trỏ sau lệnh này. `throw` báo lỗi bằng ngoại lệ cho bên gọi, khác với kết quả `false` thông thường.

**Bước 10 — Chuẩn bị giá trị cho bước kế tiếp.**

```cpp
}
```

Đoạn này chuẩn bị hoặc trả giá trị theo code. Nó chỉ đổi RAM hoặc TXT nếu có lời gọi cập nhật hay ghi dữ liệu tương ứng.

### SeatRepository::seatToString()

#### A. Tổng quan

- **Mục đích:** Chuyển đối tượng Model thành một dòng TXT.
- **Tham số:** `Seat* seat`.
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
Trả kết quả cho bên gọi (lỗi: throw)
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
string SeatRepository::seatToString(Seat* seat)
{
    if (!isValidSeat(seat))
    {
        throw runtime_error("Seat khong hop le khi ghi file");
    }
    string type;
    switch (seat->getSeatType())
    {
        case SeatType::STANDARD:
            type = "STANDARD";
```

Điều kiện hợp lệ ngăn dữ liệu sai định dạng hoặc ngoài giới hạn đi vào danh sách đang quản lý. `throw` báo lỗi bằng ngoại lệ cho bên gọi, khác với kết quả `false` thông thường.

**Bước 2 — Chuẩn bị giá trị cho bước kế tiếp.**

```cpp
            break;

        case SeatType::VIP:
            type = "VIP";
            break;

        case SeatType::COUPLE:
            type = "COUPLE";
```

Đoạn này chuẩn bị hoặc trả giá trị theo code. Nó chỉ đổi RAM hoặc TXT nếu có lời gọi cập nhật hay ghi dữ liệu tương ứng.

**Bước 3 — Trả kết quả cho bên gọi.**

```cpp
            break;
        default:
            throw runtime_error("Loai ghe khong hop le");
    }
    return seat->getRoomId() + "|" +
           seat->getSeatId() + "|" +
           to_string(seat->getRow()) + "|" +
           to_string(seat->getColumn()) + "|" +
           type;
```

Hàng và cột được so với kích thước phòng và các ghế khác để tránh vị trí ngoài sơ đồ hoặc trùng chỗ. Cặp `roomId` và `seatId` xác định đúng ghế trong phòng; chỉ dùng `seatId` sẽ không đủ để phân biệt ghế ở các phòng khác nhau. `throw` báo lỗi bằng ngoại lệ cho bên gọi, khác với kết quả `false` thông thường.

**Bước 4 — Chuẩn bị giá trị cho bước kế tiếp.**

```cpp
}
```

Đoạn này chuẩn bị hoặc trả giá trị theo code. Nó chỉ đổi RAM hoặc TXT nếu có lời gọi cập nhật hay ghi dữ liệu tương ứng.

### SeatRepository::save()

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
bool SeatRepository::save()
{
    return repo.save();
}
```

Lời gọi ghi chuyển trạng thái hiện tại ra TXT. Kết quả ghi quyết định giá trị trả về; một lời gọi này không bảo đảm các file Repository khác cùng được cập nhật.

### SeatRepository::load()

#### A. Tổng quan

- **Mục đích:** Nạp và kiểm tra dữ liệu từ TXT vào RAM.
- **Tham số:** không có.
- **Giá trị trả về:** `bool`; xem các nhánh `return` bên dưới.
- **Điều kiện quan trọng:** trạng thái nạp, con trỏ rỗng, nhánh trả `false`.

#### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Đọc dữ liệu TXT (không đạt: false)
    |
    v
Xóa dữ liệu nạp dở khi kiểm tra thất bại
    |
    v
Xóa dữ liệu nạp dở khi kiểm tra thất bại (không đạt: false)
    |
    v
Duyệt và đối chiếu từng phần tử
    |
    v
Xóa dữ liệu nạp dở khi kiểm tra thất bại (không đạt: false)
    |
    v
Tra cứu đối tượng theo khóa
    |
    v
Xóa dữ liệu nạp dở khi kiểm tra thất bại (không đạt: false)
    |
    v
Bắt lỗi và khôi phục trạng thái
    |
    v
Xóa dữ liệu nạp dở khi kiểm tra thất bại
    |
    v
Xác nhận xử lý thành công
    |
    v
Kết thúc
```

#### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1 — Đọc dữ liệu TXT.**

```cpp
bool SeatRepository::load()
{
    if (repo.isLoaded() || !roomRepo.isLoaded())
    {
        return false;
    }
    if (!repo.load())
    {
        return false;
```

Lời gọi nạp đọc TXT vào bộ nhớ. Nếu trả `false`, hàm dừng ở nhánh lỗi; dữ liệu chỉ được xem là dùng được sau khi các bước kiểm tra tiếp theo hoàn tất. Trạng thái `loaded` quyết định Repository đã sẵn sàng chưa; nhánh chưa nạp dừng trước khi thao tác dựa trên dữ liệu đó. `return false` kết thúc ngay nhánh không đạt; các thao tác cập nhật phía sau không chạy.

**Bước 2 — Xóa dữ liệu nạp dở khi kiểm tra thất bại.**

```cpp
    }
    // Buoc 1: Kiem tra toan bo du lieu truoc khi lien ket
    for (int i = 0; i < repo.count(); i++)
    {
        Seat* seat = repo.getAt(i);
        Room* room = roomRepo.findById(seat->getRoomId());
        if (room == nullptr ||
            seat->getRoom() != nullptr ||
            seat->getRow() > room->getRowCount() ||
            seat->getColumn() > room->getColumnCount())
        {
            repo.clear();
```

`repo.clear()` giải phóng các đối tượng vừa nạp và đặt lại trạng thái; vì dữ liệu chưa qua hết kiểm tra, không được để chúng tiếp tục xuất hiện như bản ghi hợp lệ. Tra Room bằng `roomId` để bảo đảm đối tượng liên kết còn tồn tại; `nullptr` nghĩa là tham chiếu sai và không thể nối Seat/Showtime vào phòng. Hàng và cột được so với kích thước phòng và các ghế khác để tránh vị trí ngoài sơ đồ hoặc trùng chỗ.

**Bước 3 — Xóa dữ liệu nạp dở khi kiểm tra thất bại.**

```cpp
            return false;
        }
        // kiểm tra trùng ghế với những ghế có trong phòng
        for (int j = 0; j < room->getSeatCount(); j++)
        {
            Seat* current = room->getSeatAt(j);
            if (current == nullptr ||
                current->getSeatId() == seat->getSeatId() ||
                (current->getRow() == seat->getRow() &&
                 current->getColumn() == seat->getColumn()))
            {
                repo.clear();
                return false;
            }
```

`repo.clear()` giải phóng các đối tượng vừa nạp và đặt lại trạng thái; vì dữ liệu chưa qua hết kiểm tra, không được để chúng tiếp tục xuất hiện như bản ghi hợp lệ. Hàng và cột được so với kích thước phòng và các ghế khác để tránh vị trí ngoài sơ đồ hoặc trùng chỗ. Cặp `roomId` và `seatId` xác định đúng ghế trong phòng; chỉ dùng `seatId` sẽ không đủ để phân biệt ghế ở các phòng khác nhau.

**Bước 4 — Duyệt và đối chiếu từng phần tử.**

```cpp
        }

        // Kiem tra trung giua cac Seat vua load
        for (int j = 0; j < i; j++)
        {
            Seat* other = repo.getAt(j);
            if (other->getRoomId() != seat->getRoomId())
            {
                continue;
            }
```

Cặp `roomId` và `seatId` xác định đúng ghế trong phòng; chỉ dùng `seatId` sẽ không đủ để phân biệt ghế ở các phòng khác nhau. Vòng lặp xét từng bản ghi; điều kiện bên trong quyết định tiếp tục, bỏ qua hay dừng khi gặp dữ liệu sai.

**Bước 5 — Xóa dữ liệu nạp dở khi kiểm tra thất bại.**

```cpp
            if (other->getSeatId() == seat->getSeatId() ||
                (other->getRow() == seat->getRow() &&
                 other->getColumn() == seat->getColumn()))
            {
                repo.clear();
                return false;
            }
        }
```

`repo.clear()` giải phóng các đối tượng vừa nạp và đặt lại trạng thái; vì dữ liệu chưa qua hết kiểm tra, không được để chúng tiếp tục xuất hiện như bản ghi hợp lệ. Hàng và cột được so với kích thước phòng và các ghế khác để tránh vị trí ngoài sơ đồ hoặc trùng chỗ. Cặp `roomId` và `seatId` xác định đúng ghế trong phòng; chỉ dùng `seatId` sẽ không đủ để phân biệt ghế ở các phòng khác nhau.

**Bước 6 — Tra cứu đối tượng theo khóa.**

```cpp
    }
    // Buoc 2: Lien ket Seat voi Room
    try
    {
        for (int i = 0; i < repo.count(); i++)
        {
            Seat* seat = repo.getAt(i);
            Room* room = roomRepo.findById(seat->getRoomId());
```

Tra Room bằng `roomId` để bảo đảm đối tượng liên kết còn tồn tại; `nullptr` nghĩa là tham chiếu sai và không thể nối Seat/Showtime vào phòng. Cặp `roomId` và `seatId` xác định đúng ghế trong phòng; chỉ dùng `seatId` sẽ không đủ để phân biệt ghế ở các phòng khác nhau. `try/catch` xử lý ngoại lệ trong phạm vi hiện tại; code dọn các thay đổi trước khi trả lỗi hoặc ném lại, nên thứ tự dọn có ý nghĩa.

**Bước 7 — Xóa dữ liệu nạp dở khi kiểm tra thất bại.**

```cpp
            if (!seat->linkRoom(room))
            {
                // Thao tat ca lien ket da tao
                for (int j = 0; j < repo.count(); j++)
                {
                    Seat* current = repo.getAt(j);
                    if (current->getRoom() != nullptr)
                    {
                        current->getRoom()->unlinkSeat(current);
                    }
                }
                repo.clear();
                return false;
            }
```

`repo.clear()` giải phóng các đối tượng vừa nạp và đặt lại trạng thái; vì dữ liệu chưa qua hết kiểm tra, không được để chúng tiếp tục xuất hiện như bản ghi hợp lệ. Liên kết được đổi theo thứ tự trong code; nếu bước sau thất bại, phần đã nối phải được tháo trước khi xóa đối tượng để tránh con trỏ treo. `return false` kết thúc ngay nhánh không đạt; các thao tác cập nhật phía sau không chạy.

**Bước 8 — Bắt lỗi và khôi phục trạng thái.**

```cpp
        }
    }
    catch (...)
    {
        // rollback
        for (int i = 0; i < repo.count(); i++)
        {
            Seat* seat = repo.getAt(i);
```

`try/catch` xử lý ngoại lệ trong phạm vi hiện tại; code dọn các thay đổi trước khi trả lỗi hoặc ném lại, nên thứ tự dọn có ý nghĩa. Vòng lặp xét từng bản ghi; điều kiện bên trong quyết định tiếp tục, bỏ qua hay dừng khi gặp dữ liệu sai.

**Bước 9 — Xóa dữ liệu nạp dở khi kiểm tra thất bại.**

```cpp
            if (seat->getRoom() != nullptr)
            {
                seat->getRoom()->unlinkSeat(seat);
            }
        }
        repo.clear();
        throw;
    }
```

`repo.clear()` giải phóng các đối tượng vừa nạp và đặt lại trạng thái; vì dữ liệu chưa qua hết kiểm tra, không được để chúng tiếp tục xuất hiện như bản ghi hợp lệ. Liên kết được đổi theo thứ tự trong code; nếu bước sau thất bại, phần đã nối phải được tháo trước khi xóa đối tượng để tránh con trỏ treo.

**Bước 10 — Xác nhận xử lý thành công.**

```cpp

    return true;
}
```

Đoạn này chuẩn bị hoặc trả giá trị theo code. Nó chỉ đổi RAM hoặc TXT nếu có lời gọi cập nhật hay ghi dữ liệu tương ứng.

### SeatRepository::add()

#### A. Tổng quan

- **Mục đích:** Thêm đối tượng hợp lệ vào Repository.
- **Tham số:** `Seat* seat`.
- **Giá trị trả về:** `bool`; xem các nhánh `return` bên dưới.
- **Điều kiện quan trọng:** trạng thái nạp, con trỏ rỗng, dữ liệu hợp lệ, nhánh trả `false`.

#### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Kiểm tra dữ liệu hợp lệ (không đạt: false)
    |
    v
Tra cứu đối tượng theo khóa (không đạt: false)
    |
    v
Nối quan hệ giữa các Model (không đạt: false)
    |
    v
Tháo liên kết đối tượng (không đạt: false)
    |
    v
Bắt lỗi và khôi phục trạng thái
    |
    v
Kết thúc
```

#### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1 — Kiểm tra dữ liệu hợp lệ.**

```cpp
bool SeatRepository::add(Seat* seat)
{
    if (!repo.isLoaded() ||
        !roomRepo.isLoaded() ||
        !isValidSeat(seat))
    {
        return false;
    }
```

Trạng thái `loaded` quyết định Repository đã sẵn sàng chưa; nhánh chưa nạp dừng trước khi thao tác dựa trên dữ liệu đó. Điều kiện hợp lệ ngăn dữ liệu sai định dạng hoặc ngoài giới hạn đi vào danh sách đang quản lý. `return false` kết thúc ngay nhánh không đạt; các thao tác cập nhật phía sau không chạy.

**Bước 2 — Tra cứu đối tượng theo khóa.**

```cpp
    Room* room = roomRepo.findById(seat->getRoomId());
    if (room == nullptr)
    {
        return false;
    }
    // Khong them lai con tro da duoc quan ly
    for (int i = 0; i < repo.count(); i++)
    {
        if (repo.getAt(i) == seat)
        {
            return false;
```

Tra Room bằng `roomId` để bảo đảm đối tượng liên kết còn tồn tại; `nullptr` nghĩa là tham chiếu sai và không thể nối Seat/Showtime vào phòng. Cặp `roomId` và `seatId` xác định đúng ghế trong phòng; chỉ dùng `seatId` sẽ không đủ để phân biệt ghế ở các phòng khác nhau. `return false` kết thúc ngay nhánh không đạt; các thao tác cập nhật phía sau không chạy.

**Bước 3 — Nối quan hệ giữa các Model.**

```cpp
        }
    }
    if (seat->getRoom() != nullptr)
    {
        return false;
    }
    // Lien ket Seat voi Room
    if (!seat->linkRoom(room))
    {
        return false;
```

Liên kết được đổi theo thứ tự trong code; nếu bước sau thất bại, phần đã nối phải được tháo trước khi xóa đối tượng để tránh con trỏ treo. `return false` kết thúc ngay nhánh không đạt; các thao tác cập nhật phía sau không chạy.

**Bước 4 — Tháo liên kết đối tượng.**

```cpp
    }
    try
    {
        if (!repo.add(seat))
        {
            room->unlinkSeat(seat);
            return false;
        }
```

Liên kết được đổi theo thứ tự trong code; nếu bước sau thất bại, phần đã nối phải được tháo trước khi xóa đối tượng để tránh con trỏ treo. `try/catch` xử lý ngoại lệ trong phạm vi hiện tại; code dọn các thay đổi trước khi trả lỗi hoặc ném lại, nên thứ tự dọn có ý nghĩa. `return false` kết thúc ngay nhánh không đạt; các thao tác cập nhật phía sau không chạy.

**Bước 5 — Bắt lỗi và khôi phục trạng thái.**

```cpp
    }
    catch (...)
    {
        room->unlinkSeat(seat);
        throw;
    }
    return true;
}
```

Liên kết được đổi theo thứ tự trong code; nếu bước sau thất bại, phần đã nối phải được tháo trước khi xóa đối tượng để tránh con trỏ treo. `try/catch` xử lý ngoại lệ trong phạm vi hiện tại; code dọn các thay đổi trước khi trả lỗi hoặc ném lại, nên thứ tự dọn có ý nghĩa.

### SeatRepository::update()

#### A. Tổng quan

- **Mục đích:** Cập nhật đối tượng đang quản lý.
- **Tham số:** `const Seat& newValues`.
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
Từ chối điều kiện không đạt (không đạt: false)
    |
    v
Chuẩn bị giá trị cho bước kế tiếp
    |
    v
Kết thúc
```

#### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1 — Tra cứu đối tượng theo khóa.**

```cpp
bool SeatRepository::update(const Seat& newValues)
{
    if (!repo.isLoaded() || !isValidSeat(&newValues))
    {
        return false;
    }
    Seat* seat = findById(newValues.getRoomId(), newValues.getSeatId());
    if (seat == nullptr)
    {
        return false;
```

Trạng thái `loaded` quyết định Repository đã sẵn sàng chưa; nhánh chưa nạp dừng trước khi thao tác dựa trên dữ liệu đó. Điều kiện hợp lệ ngăn dữ liệu sai định dạng hoặc ngoài giới hạn đi vào danh sách đang quản lý. Cặp `roomId` và `seatId` xác định đúng ghế trong phòng; chỉ dùng `seatId` sẽ không đủ để phân biệt ghế ở các phòng khác nhau.

**Bước 2 — Từ chối điều kiện không đạt.**

```cpp
    }
    // không được thay đổi vị trí ghế
    if (seat->getRow() != newValues.getRow() || seat->getColumn() != newValues.getColumn())
    {
        return false;
    }
    seat->setSeatType(newValues.getSeatType());
    return true;
```

Hàng và cột được so với kích thước phòng và các ghế khác để tránh vị trí ngoài sơ đồ hoặc trùng chỗ. `return false` kết thúc ngay nhánh không đạt; các thao tác cập nhật phía sau không chạy.

**Bước 3 — Chuẩn bị giá trị cho bước kế tiếp.**

```cpp
}
```

Đoạn này chuẩn bị hoặc trả giá trị theo code. Nó chỉ đổi RAM hoặc TXT nếu có lời gọi cập nhật hay ghi dữ liệu tương ứng.

### SeatRepository::removeById()

#### A. Tổng quan

- **Mục đích:** Xóa đối tượng theo khóa sau khi kiểm tra ràng buộc.
- **Tham số:** `const string& roomId, const string& seatId`.
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
Tháo liên kết đối tượng (không đạt: false)
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
bool SeatRepository::removeById(const string& roomId, const string& seatId)
{
    if (!repo.isLoaded())
    {
        return false;
    }
    Seat* seat = findById(roomId, seatId);
    if (seat == nullptr)
    {
        return false;
```

Trạng thái `loaded` quyết định Repository đã sẵn sàng chưa; nhánh chưa nạp dừng trước khi thao tác dựa trên dữ liệu đó. `return false` kết thúc ngay nhánh không đạt; các thao tác cập nhật phía sau không chạy.

**Bước 2 — Tháo liên kết đối tượng.**

```cpp
    }
    Room* room = seat->getRoom();
    if (room == nullptr)
    {
        return false;
    }
    // Thao lien ket truoc khi xoa Seat
    if (!room->unlinkSeat(seat))
    {
        return false;
```

Liên kết được đổi theo thứ tự trong code; nếu bước sau thất bại, phần đã nối phải được tháo trước khi xóa đối tượng để tránh con trỏ treo. `return false` kết thúc ngay nhánh không đạt; các thao tác cập nhật phía sau không chạy.

**Bước 3 — Trả kết quả cho bên gọi.**

```cpp
    }
    // Xoa Seat khoi Repository va giai phong bo nho
    return repo.removeById(roomId + "|" + seatId);
}
```

Đoạn này chuẩn bị hoặc trả giá trị theo code. Nó chỉ đổi RAM hoặc TXT nếu có lời gọi cập nhật hay ghi dữ liệu tương ứng.

### SeatRepository::findById()

#### A. Tổng quan

- **Mục đích:** Tra đối tượng theo ID.
- **Tham số:** `const string& roomId, const string& seatId`.
- **Giá trị trả về:** `Seat*`; xem các nhánh `return` bên dưới.
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
Seat* SeatRepository::findById(const string& roomId, const string& seatId)
{
    return repo.findById(roomId + "|" + seatId);
}
```

Đoạn này chuẩn bị hoặc trả giá trị theo code. Nó chỉ đổi RAM hoặc TXT nếu có lời gọi cập nhật hay ghi dữ liệu tương ứng.

### SeatRepository::getAt()

#### A. Tổng quan

- **Mục đích:** Lấy phần tử theo chỉ số.
- **Tham số:** `int index`.
- **Giá trị trả về:** `Seat*`; xem các nhánh `return` bên dưới.
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
Seat* SeatRepository::getAt(int index)
{
    return repo.getAt(index);
}
```

Đoạn này chuẩn bị hoặc trả giá trị theo code. Nó chỉ đổi RAM hoặc TXT nếu có lời gọi cập nhật hay ghi dữ liệu tương ứng.

### SeatRepository::count()

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
int SeatRepository::count()
{
    return repo.count();
}
```

Đoạn này chuẩn bị hoặc trả giá trị theo code. Nó chỉ đổi RAM hoặc TXT nếu có lời gọi cập nhật hay ghi dữ liệu tương ứng.

### SeatRepository::isLoaded()

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
bool SeatRepository::isLoaded()
{
    return repo.isLoaded();
}
```

Trạng thái `loaded` quyết định Repository đã sẵn sàng chưa; nhánh chưa nạp dừng trước khi thao tác dựa trên dữ liệu đó.


