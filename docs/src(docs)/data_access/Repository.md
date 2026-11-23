# Repository<T> — mã nguồn và quy tắc dữ liệu

## 1. Tổng quan

`Repository<T>` là template dùng chung, định nghĩa ngay trong `src/data_access/Repository.h` để trình biên dịch nhìn thấy mã khi tạo `Repository<Movie>`, `Repository<Room>` hoặc kiểu khác. Các Repository cụ thể **chứa** một `Repository<T>` và cung cấp ba callback `getId`, `parse`, `serialize` để xử lý Model và TXT riêng.

`items` là `DoublyLinkedList<T*>`; Repository sở hữu con trỏ thêm thành công và giải phóng qua `clear()`/destructor. Caller sở hữu con trỏ nếu `add()` thất bại. `loaded` bảo vệ thao tác cần dữ liệu đã nạp. `load()` dùng danh sách tạm để tránh thay dữ liệu đang dùng khi parse lỗi; `save()` tuần tự hóa rồi ghi một file. Callback là con trỏ hàm; `T*` là con trỏ tới Model, không phải bản sao.

Mã nguồn: `src/data_access/Repository.h`. Các đoạn code dưới đây giữ nguyên nội dung hàm, gồm dòng trống và comment trong hàm. Khi ghép các đoạn của một hàm theo thứ tự, ta thu được đúng định nghĩa gốc.

## 2. Chi tiết từng hàm

### Repository<T>::Repository()

#### A. Tổng quan

- **Mục đích:** Khởi tạo Repository và các tham chiếu/callback cần dùng.
- **Tham số:** `const string& filePath, string (*getId)(T*), T* (*parse)(const string&), string (*serialize)(T*)`.
- **Giá trị trả về:** không có (constructor/destructor).
- **Điều kiện quan trọng:** Callback và đường dẫn được giữ để dùng khi nạp/lưu; chưa đọc file.

#### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Đọc và parse TXT
    |
    v
Kết thúc
```

#### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1 — Đọc và parse TXT.**

```cpp
Repository<T>::Repository(const string& filePath, string (*getId)(T*), T* (*parse)(const string&), string (*serialize)(T*))
    : filePath(filePath), loaded(false), getId(getId), parse(parse), serialize(serialize)
{
}
```

Đọc TXT và dùng callback `parse` để tạo Model; dữ liệu lỗi phải đi theo nhánh xử lý lỗi trước khi thay danh sách đang dùng. Callback `serialize` tạo dòng TXT từ Model trước khi `FileIO` ghi file; kết quả ghi quyết định thao tác lưu thành công hay thất bại. Callback `getId` lấy khóa của Model để kiểm tra trùng hoặc tìm kiếm mà không cần biết kiểu `T` cụ thể.

### Repository<T>::~Repository()

#### A. Tổng quan

- **Mục đích:** Tháo liên kết hoặc giải phóng tài nguyên khi hủy Repository.
- **Tham số:** không có.
- **Giá trị trả về:** không có (constructor/destructor).
- **Điều kiện quan trọng:** Destructor gọi clear() để giải phóng các T* do Repository sở hữu.

#### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Giải phóng dữ liệu
    |
    v
Kết thúc
```

#### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1 — Giải phóng dữ liệu.**

```cpp
Repository<T>::~Repository()
{
    clear();
}
```

Dữ liệu do Repository sở hữu được giải phóng hoặc đặt lại; con trỏ đã xóa không được dùng tiếp.

### Repository<T>::clear()

#### A. Tổng quan

- **Mục đích:** Giải phóng toàn bộ đối tượng và đặt lại trạng thái.
- **Tham số:** không có.
- **Giá trị trả về:** `void`; xem các nhánh `return` bên dưới.
- **Điều kiện quan trọng:** trạng thái nạp.

#### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Giải phóng dữ liệu
    |
    v
Chuẩn bị dữ liệu
    |
    v
Kết thúc
```

#### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1 — Giải phóng dữ liệu.**

```cpp
void Repository<T>::clear()
{
    while (items.nodeCount() > 0)
    {
        T* item = items.getAt(0)->data;
        items.popFront(); //xóa khỏi danh sách
        delete item; //giải phóng đối tượng
    }
```

Dữ liệu do Repository sở hữu được giải phóng hoặc đặt lại; con trỏ đã xóa không được dùng tiếp. Vòng lặp xử lý từng con trỏ; điều kiện bên trong quyết định giữ, bỏ qua hoặc dừng.

**Bước 2 — Chuẩn bị dữ liệu.**

```cpp
    loaded = false;
}
```

Giá trị trong đoạn này được chuẩn bị hoặc trả về theo nhánh code; chỉ lời gọi cập nhật/ghi mới thay đổi danh sách hoặc TXT.

### Repository<T>::count()

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
Trả kết quả
    |
    v
Kết thúc
```

#### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1 — Trả kết quả.**

```cpp
int Repository<T>::count()
{
    return items.nodeCount();
}
```

Giá trị trong đoạn này được chuẩn bị hoặc trả về theo nhánh code; chỉ lời gọi cập nhật/ghi mới thay đổi danh sách hoặc TXT.

### Repository<T>::getAt()

#### A. Tổng quan

- **Mục đích:** Lấy phần tử theo chỉ số.
- **Tham số:** `int index`.
- **Giá trị trả về:** `T*`; xem các nhánh `return` bên dưới.
- **Điều kiện quan trọng:** con trỏ rỗng.

#### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Trả kết quả
    |
    v
Chuẩn bị dữ liệu
    |
    v
Kết thúc
```

#### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1 — Trả kết quả.**

```cpp
T* Repository<T>::getAt(int index)
{
    ListNode<T*>* node = items.getAt(index);
    if (node == nullptr)
    {
        return nullptr;
    }
    return node->data;
```

Giá trị trong đoạn này được chuẩn bị hoặc trả về theo nhánh code; chỉ lời gọi cập nhật/ghi mới thay đổi danh sách hoặc TXT.

**Bước 2 — Chuẩn bị dữ liệu.**

```cpp
}
```

Giá trị trong đoạn này được chuẩn bị hoặc trả về theo nhánh code; chỉ lời gọi cập nhật/ghi mới thay đổi danh sách hoặc TXT.

### Repository<T>::isLoaded()

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
Trả kết quả
    |
    v
Kết thúc
```

#### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1 — Trả kết quả.**

```cpp
bool Repository<T>::isLoaded()
{
    return loaded;
}
```

Giá trị trong đoạn này được chuẩn bị hoặc trả về theo nhánh code; chỉ lời gọi cập nhật/ghi mới thay đổi danh sách hoặc TXT.

### Repository<T>::findById()

#### A. Tổng quan

- **Mục đích:** Tra đối tượng theo ID.
- **Tham số:** `const string& id`.
- **Giá trị trả về:** `T*`; xem các nhánh `return` bên dưới.
- **Điều kiện quan trọng:** con trỏ rỗng.

#### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Duyệt danh sách
    |
    v
Trả kết quả
    |
    v
Kết thúc
```

#### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1 — Duyệt danh sách.**

```cpp
T* Repository<T>::findById(const string& id)
{
    for (int i = 0; i < count(); i++)
    {
        T* item = getAt(i);
        if (getId(item) == id)
        {
            return item;
```

Callback `getId` lấy khóa của Model để kiểm tra trùng hoặc tìm kiếm mà không cần biết kiểu `T` cụ thể. Vòng lặp xử lý từng con trỏ; điều kiện bên trong quyết định giữ, bỏ qua hoặc dừng.

**Bước 2 — Trả kết quả.**

```cpp
        }
    }
    return nullptr;
}
```

Giá trị trong đoạn này được chuẩn bị hoặc trả về theo nhánh code; chỉ lời gọi cập nhật/ghi mới thay đổi danh sách hoặc TXT.

### Repository<T>::add()

#### A. Tổng quan

- **Mục đích:** Thêm đối tượng hợp lệ vào Repository.
- **Tham số:** `T* item`.
- **Giá trị trả về:** `bool`; xem các nhánh `return` bên dưới.
- **Điều kiện quan trọng:** trạng thái nạp, con trỏ rỗng, nhánh trả `false`.

#### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Duyệt danh sách (không đạt: false)
    |
    v
Từ chối điều kiện sai (không đạt: false)
    |
    v
Trả kết quả
    |
    v
Kết thúc
```

#### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1 — Duyệt danh sách.**

```cpp
bool Repository<T>::add(T* item)
{
    if (!loaded || item == nullptr)
    {
        return false;
    }
    // kiểm tra trùng 
    for (int i = 0; i < count(); i++)
    {
        if (getAt(i) == item)
        {
            return false;
```

`false` kết thúc sớm; các cập nhật phía sau không diễn ra. Vòng lặp xử lý từng con trỏ; điều kiện bên trong quyết định giữ, bỏ qua hoặc dừng.

**Bước 2 — Từ chối điều kiện sai.**

```cpp
        }
    }
    string id = getId(item);
    if (id.empty())
    {
        return false;
    }
    if (findById(id) != nullptr)
    {
        return false;
```

Callback `getId` lấy khóa của Model để kiểm tra trùng hoặc tìm kiếm mà không cần biết kiểu `T` cụ thể. `false` kết thúc sớm; các cập nhật phía sau không diễn ra.

**Bước 3 — Trả kết quả.**

```cpp
    }
    items.pushBack(item);
    return true;
}
```

Giá trị trong đoạn này được chuẩn bị hoặc trả về theo nhánh code; chỉ lời gọi cập nhật/ghi mới thay đổi danh sách hoặc TXT.

### Repository<T>::removeById()

#### A. Tổng quan

- **Mục đích:** Xóa đối tượng theo khóa sau khi kiểm tra ràng buộc.
- **Tham số:** `const string& id`.
- **Giá trị trả về:** `bool`; xem các nhánh `return` bên dưới.
- **Điều kiện quan trọng:** trạng thái nạp, nhánh trả `false`.

#### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Duyệt danh sách (không đạt: false)
    |
    v
Giải phóng dữ liệu (không đạt: false)
    |
    v
Chuẩn bị dữ liệu
    |
    v
Kết thúc
```

#### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1 — Duyệt danh sách.**

```cpp
bool Repository<T>::removeById(const string& id)
{
    if (!loaded)
    {
        return false;
    }
    for (int i = 0; i < count(); i++)
    {
        T* item = getAt(i);
```

`false` kết thúc sớm; các cập nhật phía sau không diễn ra. Vòng lặp xử lý từng con trỏ; điều kiện bên trong quyết định giữ, bỏ qua hoặc dừng.

**Bước 2 — Giải phóng dữ liệu.**

```cpp
        if (getId(item) == id)
        {
            items.erase(i);
            delete item;
            return true;
        }
    }
    return false;
```

Dữ liệu do Repository sở hữu được giải phóng hoặc đặt lại; con trỏ đã xóa không được dùng tiếp. Callback `getId` lấy khóa của Model để kiểm tra trùng hoặc tìm kiếm mà không cần biết kiểu `T` cụ thể. `false` kết thúc sớm; các cập nhật phía sau không diễn ra.

**Bước 3 — Chuẩn bị dữ liệu.**

```cpp
}
```

Giá trị trong đoạn này được chuẩn bị hoặc trả về theo nhánh code; chỉ lời gọi cập nhật/ghi mới thay đổi danh sách hoặc TXT.

### Repository<T>::load()

#### A. Tổng quan

- **Mục đích:** Nạp và kiểm tra dữ liệu từ TXT vào RAM.
- **Tham số:** không có.
- **Giá trị trả về:** `bool`; xem các nhánh `return` bên dưới.
- **Điều kiện quan trọng:** trạng thái nạp, con trỏ rỗng, ngoại lệ, nhánh trả `false`.

#### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Đọc và parse TXT (không đạt: false)
    |
    v
Xử lý lỗi (lỗi: throw)
    |
    v
Xử lý lỗi (lỗi: throw)
    |
    v
Xử lý lỗi
    |
    v
Giải phóng dữ liệu
    |
    v
Chuẩn bị dữ liệu
    |
    v
Kết thúc
```

#### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1 — Đọc và parse TXT.**

```cpp
bool Repository<T>::load()
{
    if (loaded)
    {
        return false;
    }
    vector<string> lines;
    if (!FileIO::readLines(filePath, lines))
    {
        return false;
```

Đọc TXT và dùng callback `parse` để tạo Model; dữ liệu lỗi phải đi theo nhánh xử lý lỗi trước khi thay danh sách đang dùng. `false` kết thúc sớm; các cập nhật phía sau không diễn ra.

**Bước 2 — Xử lý lỗi.**

```cpp
    }
    DoublyLinkedList<T*> tempItems;
    try
    {
        for (int i = 0; i < (int)lines.size(); i++)
        {
            T* item = nullptr;
            try
            {
                item = parse(lines[i]);
                if (item == nullptr)
                {
                    throw runtime_error("Du lieu khong hop le");
                }
                string id = getId(item);
                if (id.empty())
                {
                    throw runtime_error("ID rong");
                }
                // Kiem tra ID trung trong danh sach tam
                for (int j = 0; j < tempItems.nodeCount(); j++)
                {
                    T* current = tempItems.getAt(j)->data;
                    if (getId(current) == id)
                    {
                        throw runtime_error("Trung ID");
```

Đọc TXT và dùng callback `parse` để tạo Model; dữ liệu lỗi phải đi theo nhánh xử lý lỗi trước khi thay danh sách đang dùng. Callback `getId` lấy khóa của Model để kiểm tra trùng hoặc tìm kiếm mà không cần biết kiểu `T` cụ thể. Ngoại lệ chuyển lỗi cho caller sau khi dọn tài nguyên ở nhánh tương ứng.

**Bước 3 — Xử lý lỗi.**

```cpp
                    }
                }
                tempItems.pushBack(item);
                // Tu day tempItems quan ly item
                item = nullptr;
            }
            catch (const exception& e)
            {
                // giải phóng đối tượng bị lỗi
                delete item;
                throw runtime_error(
                    "Loi doc file " + filePath +
                    " tai dong " + to_string(i + 1) +
                    ": " + e.what()
                );
            }
```

Dữ liệu do Repository sở hữu được giải phóng hoặc đặt lại; con trỏ đã xóa không được dùng tiếp. Ngoại lệ chuyển lỗi cho caller sau khi dọn tài nguyên ở nhánh tương ứng.

**Bước 4 — Xử lý lỗi.**

```cpp
            catch (...)
            {
                delete item;
                throw;
            }
        }
    }
    catch (...)
    {
        // Giải phóng các đối tượng còn lại
        while (tempItems.nodeCount() > 0)
        {
            T* item = tempItems.getAt(0)->data;
```

Dữ liệu do Repository sở hữu được giải phóng hoặc đặt lại; con trỏ đã xóa không được dùng tiếp. Vòng lặp xử lý từng con trỏ; điều kiện bên trong quyết định giữ, bỏ qua hoặc dừng.

**Bước 5 — Giải phóng dữ liệu.**

```cpp
            tempItems.popFront();
            delete item;
        }
        throw;
    }
    items.swap(tempItems);
    loaded = true;
    return true;
```

Dữ liệu do Repository sở hữu được giải phóng hoặc đặt lại; con trỏ đã xóa không được dùng tiếp.

**Bước 6 — Chuẩn bị dữ liệu.**

```cpp
}
```

Giá trị trong đoạn này được chuẩn bị hoặc trả về theo nhánh code; chỉ lời gọi cập nhật/ghi mới thay đổi danh sách hoặc TXT.

### Repository<T>::save()

#### A. Tổng quan

- **Mục đích:** Ghi trạng thái hiện tại ra TXT.
- **Tham số:** không có.
- **Giá trị trả về:** `bool`; xem các nhánh `return` bên dưới.
- **Điều kiện quan trọng:** trạng thái nạp, nhánh trả `false`.

#### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Duyệt danh sách (không đạt: false)
    |
    v
Tuần tự hóa và ghi TXT
    |
    v
Kết thúc
```

#### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1 — Duyệt danh sách.**

```cpp
bool Repository<T>::save()
{
    if (!loaded)
    {
        return false;
    }
    vector<string> lines;
    for (int i = 0; i < count(); i++)
    {
        T* item = getAt(i);
```

`false` kết thúc sớm; các cập nhật phía sau không diễn ra. Vòng lặp xử lý từng con trỏ; điều kiện bên trong quyết định giữ, bỏ qua hoặc dừng.

**Bước 2 — Tuần tự hóa và ghi TXT.**

```cpp
        string line = serialize(item);
        lines.push_back(line);
    }
    return FileIO::writeLines(filePath, lines);
}
```

Callback `serialize` tạo dòng TXT từ Model trước khi `FileIO` ghi file; kết quả ghi quyết định thao tác lưu thành công hay thất bại.

### Repository<T>::getFilePath()

#### A. Tổng quan

- **Mục đích:** Trả đường dẫn TXT.
- **Tham số:** không có.
- **Giá trị trả về:** `const string&`; xem các nhánh `return` bên dưới.
- **Điều kiện quan trọng:** không có điều kiện đặc biệt ngoài các câu lệnh bên dưới.

#### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Trả kết quả
    |
    v
Kết thúc
```

#### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1 — Trả kết quả.**

```cpp
const string& Repository<T>::getFilePath() const
{
    return filePath;
}
```

Giá trị trong đoạn này được chuẩn bị hoặc trả về theo nhánh code; chỉ lời gọi cập nhật/ghi mới thay đổi danh sách hoặc TXT.


