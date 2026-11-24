# Repository<T>

## 1. Tổng quan

`Repository<T>` là template quản lý danh sách đối tượng và đọc/ghi TXT dùng chung trong Data Access. Khai báo và định nghĩa đều nằm trong `src/data_access/Repository.h` để trình biên dịch tạo phiên bản cho từng `T`.

**Trạng thái:** Đã rà soát mã nguồn hiện tại; kiểm thử gián tiếp thành công qua `tests/test_irepository.cpp` ngày 24/11/2026 (81 PASS, 0 FAIL). Tài liệu mô tả code hiện có, không đề xuất sửa thiết kế.

## 2. Kiến trúc và dữ liệu

```text
Service → IRepository<Movie> ← MovieRepository
                                │ chứa (composition)
                                ▼
                         Repository<Movie>
                                │ chứa
                                ▼
                     DoublyLinkedList<Movie*>
```

`Repository<T>` **không kế thừa `IRepository<T>`**. Concrete Repository kế thừa interface và chứa `Repository<T>` để dùng lại thao tác danh sách/file. Concrete Repository cung cấp các callback `getId(T*)`, `parse(const string&)`, `serialize(T*)`; nó xử lý validation, liên kết và ràng buộc riêng của Model. `Repository<T>` không có `update()`; chức năng cập nhật, nếu hỗ trợ, do Concrete Repository triển khai.

File TXT được `FileIO` đọc/ghi theo từng dòng. `Repository<T>` chỉ kiểm tra ID rỗng/trùng khi load và add; schema, định dạng ID và các trường Model do callback cùng Concrete Repository kiểm tra.

## 3. Thiết kế lớp

### 3.1. Thuộc tính private

| Thuộc tính | Kiểu | Vai trò |
|---|---|---|
| `filePath` | `string` | Đường dẫn TXT. |
| `items` | `DoublyLinkedList<T*>` | Danh sách con trỏ được Repository sở hữu. |
| `loaded` | `bool` | Trạng thái nạp; ban đầu `false`. |
| `getId` | `string (*)(T*)` | Lấy khóa của đối tượng. |
| `parse` | `T* (*)(const string&)` | Tạo đối tượng từ dòng TXT. |
| `serialize` | `string (*)(T*)` | Tạo dòng TXT từ đối tượng. |

### 3.2. Phương thức public

| Nhóm | Phương thức |
|---|---|
| Vòng đời | `Repository(...)`, `~Repository()`, `clear()` |
| Tệp | `load()`, `save()`, `getFilePath() const` |
| Danh sách | `add(T*)`, `removeById(const string&)`, `findById(const string&)`, `getAt(int)`, `count()`, `isLoaded()` |

## 4. Mã nguồn hiện tại

**Nguồn:** `src/data_access/Repository.h`. Khối dưới đây được chèn nguyên văn từ file đó, gồm cả comment và định dạng hiện tại.

```cpp
#ifndef REPOSITORY_H
#define REPOSITORY_H
#include <vector>
#include <stdexcept>
#include "../utils/FileIO.h"
#include <string>
#include "../data_structures/DoublyLinkedList.h"

using namespace std;

template <typename T>
class Repository
{
private:
    string filePath;
    DoublyLinkedList<T*> items;
    bool loaded; // đánh dấu repo đã nạp dữ liệu chưa
    
    string (*getId)(T*); //con trỏ hàm - getId là 1 con trỏ trỏ đến hàm nhận tham số T* và trả về string
    T* (*parse)(const string&); //chuyển txt sang đối tượng cụ thể
    string (*serialize)(T*); //chuyển đối tượng sang txt
public:
    Repository(const string& filePath, string (*getId)(T*), T* (*parse)(const string&), string (*serialize)(T*));
    ~Repository();

    bool load();
    bool save();

    bool add(T* item);
    bool removeById(const string& id);
    
    T* findById(const string& id);
    T* getAt(int index);
    int count();
    bool isLoaded();
    void clear();
    const string& getFilePath() const;
};

//----

template <typename T>
Repository<T>::Repository(const string& filePath, string (*getId)(T*), T* (*parse)(const string&), string (*serialize)(T*))
    : filePath(filePath), loaded(false), getId(getId), parse(parse), serialize(serialize)
{
}

template <typename T>
Repository<T>::~Repository()
{
    clear();
}

template <typename T>
void Repository<T>::clear()
{
    while (items.nodeCount() > 0)
    {
        T* item = items.getAt(0)->data;
        items.popFront(); //xóa khỏi danh sách
        delete item; //giải phóng đối tượng
    }
    loaded = false;
}
template <typename T>
int Repository<T>::count()
{
    return items.nodeCount();
}
template <typename T>
T* Repository<T>::getAt(int index)
{
    ListNode<T*>* node = items.getAt(index);
    if (node == nullptr)
    {
        return nullptr;
    }
    return node->data;
}
template <typename T>
bool Repository<T>::isLoaded()
{
    return loaded;
}
template <typename T>
T* Repository<T>::findById(const string& id)
{
    for (int i = 0; i < count(); i++)
    {
        T* item = getAt(i);
        if (getId(item) == id)
        {
            return item;
        }
    }
    return nullptr;
}
template <typename T>
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
    }
    items.pushBack(item);
    return true;
}

template <typename T>
bool Repository<T>::removeById(const string& id)
{
    if (!loaded)
    {
        return false;
    }
    for (int i = 0; i < count(); i++)
    {
        T* item = getAt(i);
        if (getId(item) == id)
        {
            items.erase(i);
            delete item;
            return true;
        }
    }
    return false;
}

template <typename T>
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
            tempItems.popFront();
            delete item;
        }
        throw;
    }
    items.swap(tempItems);
    loaded = true;
    return true;
}

template <typename T>
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
        string line = serialize(item);
        lines.push_back(line);
    }
    return FileIO::writeLines(filePath, lines);
}
template <typename T>
const string& Repository<T>::getFilePath() const
{
    return filePath;
}

#endif
```

## 5. Giải thích thư viện và cú pháp

| Thành phần | Cách dùng |
|---|---|
| `<vector>` | `vector<string>` giữ các dòng đọc hoặc chuẩn bị ghi. |
| `<stdexcept>` | `runtime_error` cho dữ liệu load không hợp lệ. |
| `<string>` | `string`, `to_string`, đường dẫn và ID. |
| `FileIO.h` | `readLines`, `writeLines` cho TXT. |
| `DoublyLinkedList.h` | Lưu `T*`; hỗ trợ `nodeCount`, `getAt`, `pushBack`, `erase`, `popFront`, `swap`. |
| `template <typename T>` | Định nghĩa một lớp dùng cho nhiều Model; định nghĩa phải thấy được tại nơi tạo template. |
| Con trỏ hàm | Ba callback truyền logic riêng của Model vào lớp chung. |
| `const string&` | Nhận hoặc trả chuỗi bằng tham chiếu, tránh sao chép. |
| `try`/`catch`/`throw` | Dọn dữ liệu tạm và chuyển lỗi ra caller. |

## 6. Giải thích từng phương thức

### 6.1. Constructor

**Tổng quan:** Nhận đường dẫn và ba callback; không trả giá trị, không đọc file. Callback phải hợp lệ khi được gọi; constructor hiện không kiểm tra null.

```text
Nhận tham số → khởi tạo filePath, loaded=false, getId, parse, serialize → kết thúc
```

**Giải thích code:** Danh sách khởi tạo sau dấu `:` gán từng thành viên trước khi vào thân `{}` rỗng. Chưa có `T*` nào trong `items` và chưa gọi FileIO.

### 6.2. Destructor

**Tổng quan:** Không tham số/giá trị trả về; giải phóng đối tượng đang sở hữu.

```text
Hủy Repository → gọi clear() → kết thúc
```

**Giải thích code:** `~Repository()` chỉ gọi `clear()`. Không tự lưu file; con trỏ mượn tới các đối tượng đã xóa trở nên không hợp lệ.

### 6.3. `clear()`

**Tổng quan:** Không tham số, trả `void`; xóa toàn bộ `T*`, đặt `loaded=false`.

```text
items còn node? → có: lấy node đầu → popFront → delete T* → lặp
             └→ không: loaded=false → kết thúc
```

**Giải thích code:** `while (items.nodeCount() > 0)` bảo đảm `items.getAt(0)` tồn tại; `->data` lấy con trỏ. `popFront()` bỏ node trước khi `delete item` giải phóng Model. `loaded=false` cho phép gọi load lần sau. Code không tự hủy các liên kết từ Repository khác tới Model.

### 6.4. `count()`

**Tổng quan:** Không tham số; trả số node hiện có.

```text
items.nodeCount() → trả int
```

**Giải thích code:** `return items.nodeCount();` không kiểm tra `loaded`, không sửa danh sách.

### 6.5. `getAt(int index)`

**Tổng quan:** Trả `T*` mượn tại chỉ số hoặc `nullptr` nếu không có node.

```text
items.getAt(index) → node null? → có: nullptr; không: node->data
```

**Giải thích code:** Biến `ListNode<T*>* node` giữ node; kiểm tra null trước khi truy cập `node->data`. Không chuyển quyền sở hữu cho caller.

### 6.6. `isLoaded()`

**Tổng quan:** Không tham số; trả `bool` trạng thái nạp.

```text
đọc loaded → trả bool
```

**Giải thích code:** `return loaded;`; `load()` thành công đặt true, `clear()` đặt false.

### 6.7. `findById(const string& id)`

**Tổng quan:** Nhận ID; trả `T*` mượn đầu tiên khớp hoặc `nullptr`.

```text
duyệt i từ 0 đến count()-1 → getId(item)==id? → có: trả item
                                      └→ không: tiếp tục
hết danh sách → nullptr
```

**Giải thích code:** Vòng `for` gọi `getAt(i)` và callback `getId(item)` để so chuỗi. Không kiểm tra `loaded`; danh sách rỗng trả null. Callback phải xử lý được con trỏ đang lưu.

### 6.8. `add(T* item)`

**Tổng quan:** Trả true khi thêm; false nếu chưa load, null, trùng địa chỉ, ID rỗng hoặc trùng ID. Khi thất bại, đối tượng mới vẫn thuộc caller.

```text
!loaded hoặc item null? → false
trùng địa chỉ trong items? → false
getId(item) rỗng? → false
findById(id) có kết quả? → false
pushBack(item) → true
```

**Giải thích code:** Điều kiện đầu chặn thao tác sai trạng thái. Vòng `for` so `getAt(i) == item` tránh thêm lại cùng con trỏ. `string id = getId(item)` lấy khóa; hai nhánh tiếp theo chặn ID rỗng/trùng. `items.pushBack(item)` là điểm nhận quyền sở hữu. Hàm không validate trường Model hoặc tự ghi file.

### 6.9. `removeById(const string& id)`

**Tổng quan:** Trả true nếu xóa được; false nếu chưa load/không thấy. Không tự kiểm tra liên kết của Model.

```text
!loaded? → false
duyệt items → getId(item)==id? → erase(i) → delete item → true
hết danh sách → false
```

**Giải thích code:** Vòng `for` lấy từng `T*`; nếu ID khớp, `items.erase(i)` bỏ node rồi `delete item` giải phóng đối tượng. Concrete Repository phải kiểm tra tham chiếu trước khi gọi. Con trỏ mượn tới đối tượng đã xóa không còn hợp lệ.

### 6.10. `load()`

**Tổng quan:** Trả false nếu đã load hoặc đọc file thất bại; true sau khi parse toàn bộ. Dữ liệu lỗi ném exception và dọn danh sách tạm.

```text
loaded? → false; readLines thất bại? → false
tạo tempItems → với từng dòng: parse → null/ID rỗng/trùng? → xóa item, ném lỗi có dòng
                         └→ hợp lệ: pushBack vào tempItems, item=nullptr
exception → xóa mọi T* tạm → ném tiếp
hết dòng → items.swap(tempItems) → loaded=true → true
```

**Giải thích code:** `vector<string> lines` nhận file; `DoublyLinkedList<T*> tempItems` giữ kết quả tạm. Trong vòng `for`, `item` bắt đầu null; `parse(lines[i])` tạo Model, rồi kiểm tra null, ID rỗng và trùng trong `tempItems`. Sau `pushBack`, gán `item=nullptr` để tránh xóa hai lần. `catch (const exception& e)` xóa item chưa chuyển sở hữu và ném `runtime_error` có đường dẫn, số dòng `i+1`, thông điệp gốc; `catch (...)` dọn item và ném lại lỗi khác. `catch (...)` bên ngoài dọn toàn bộ `tempItems` rồi ném lại. Chỉ sau khi duyệt hết, `swap` chuyển danh sách và đặt `loaded=true`.

### 6.11. `save()`

**Tổng quan:** Trả false nếu chưa load; nếu đã load, trả kết quả `FileIO::writeLines`.

```text
!loaded? → false
duyệt items → serialize(item) → lines.push_back(line)
writeLines(filePath, lines) → trả bool
```

**Giải thích code:** `vector<string> lines` tập hợp chuỗi; vòng `for` lấy từng item, gọi callback `serialize`, đưa dòng vào vector. `FileIO::writeLines` được gọi một lần sau vòng lặp. Hàm không bắt exception từ callback, không tự lưu sau add/remove và không tạo transaction nhiều file.

### 6.12. `getFilePath() const`

**Tổng quan:** Không tham số; trả `const string&` tới đường dẫn.

```text
đọc filePath → trả tham chiếu const
```

**Giải thích code:** `return filePath;` tránh sao chép; tham chiếu chỉ hợp lệ khi Repository còn sống. `const` cuối chữ ký cấm hàm sửa đối tượng.

## 7. Quản lý bộ nhớ và lỗi

- `add()` thành công hoặc `load()` thành công chuyển quyền sở hữu `T*` sang Repository. `removeById()`, `clear()` và destructor giải phóng chúng.
- `add()` thất bại không xóa con trỏ mới. `findById()` và `getAt()` chỉ trả con trỏ mượn.
- `load()` trả false cho lỗi đọc hoặc nạp lại; dữ liệu dòng sai gây exception. Danh sách tạm được dọn trước khi ném tiếp.
- Concrete Repository quản lý validation và ràng buộc liên kết trước khi xóa/clear để tránh con trỏ treo.

## 8. Kế hoạch kiểm thử

| Nội dung | Trạng thái |
|---|---|
| Load/save, add/remove, find/getAt/count/isLoaded qua MovieRepository | Đã kiểm thử gián tiếp bằng `test_irepository.cpp`. |
| Trạng thái trước load, null, trùng ID, dữ liệu load lỗi và dọn tạm | Đã kiểm thử gián tiếp trong phạm vi bài test. |
| `clear()` trực tiếp, `getFilePath()`, callback null, lỗi ghi file, các Model khác | Chưa kiểm thử trực tiếp trong lần rà soát này. |

**Kết quả thực tế (24/11/2026):** `tests/test_irepository.cpp` biên dịch bằng C++17 với các `.cpp` trong `src/` trừ `main.cpp`, chạy đạt **81 PASS, 0 FAIL**. Đây là kiểm thử `Repository<Movie>` qua `MovieRepository`/`IRepository<Movie>`, không xác nhận mọi kiểu `Repository<T>`.

## 9. Tiến độ

- [x] Rà soát `Repository.h` và cập nhật tài liệu theo bố cục mẫu `MovieRepository.md`.
- [x] Ghi nhận kiểm thử gián tiếp qua `test_irepository.cpp`.
- [ ] Kiểm thử trực tiếp các API và trường hợp chưa được bài test bao phủ.

`Repository<T>` tiếp tục được dùng bằng composition; không thêm `update()` hoặc thay đổi code đã chốt.
