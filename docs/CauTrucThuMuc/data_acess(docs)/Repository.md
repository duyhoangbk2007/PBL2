## 2. Lớp `Repository<T>` dùng chung

**File:** `src/data_access/Repository.h`

`Repository<T>` là một lớp template được xây dựng nhằm tái sử dụng những chức năng giống nhau giữa các loại Repository.

Ví dụ, `MovieRepository`, `RoomRepository` và `ShowtimeRepository` đều cần đọc file, lưu file, tìm kiếm theo ID và quản lý danh sách đối tượng.

Thay vì viết lại các chức năng này nhiều lần, ta sử dụng `Repository<T>`.

```cpp
Repository<Movie>
Repository<Room>
Repository<Showtime>
```

Trong đó `T` là kiểu Model mà Repository quản lý.

### 2.1. Vì sao sử dụng template?

Vì mỗi loại Model có kiểu dữ liệu khác nhau, nhưng cách quản lý danh sách và thao tác file có nhiều điểm chung.

Template cho phép viết một lớp chung và sử dụng lại với nhiều kiểu Model mà không cần sao chép code.

### 2.2. Vì sao dùng composition thay vì kế thừa?

Thiết kế dự kiến của dự án là các Repository cụ thể **chứa một đối tượng `Repository<T>`** thay vì kế thừa từ nó.

Ví dụ:

```cpp
class MovieRepository
{
private:
    Repository<Movie> repo;
};
```

Khi cần tìm phim theo ID, `MovieRepository` có thể gọi:

```cpp
repo.findById(movieId);
```

Cách này giúp:

- Tái sử dụng các hàm chung.
- Tách chức năng quản lý dữ liệu khỏi chức năng đặc thù của từng Model.
- Giảm việc viết lại `load()`, `save()`, `findById()`, ...
- Giữ code đơn giản, dễ đọc và dễ bảo trì.

**Lưu ý:** Đây là kiến trúc dự kiến. Phiên bản `MovieRepository` hiện có trên nhánh Bước 04 chưa được chuyển sang sử dụng `Repository<Movie>` theo cách này.

---

## 3. Các thuộc tính của `Repository<T>`

```cpp
private:
    string filePath;
    DoublyLinkedList<T*> items;
    bool loaded;

    string (*getId)(T*);
    T* (*parse)(const string&);
    string (*serialize)(T*);
```

### 3.1. `filePath`

```cpp
string filePath;
```

Lưu đường dẫn đến file TXT mà Repository quản lý.

Ví dụ:

```text
data/movies.txt
```

Mỗi đối tượng Repository được khởi tạo với một đường dẫn file cụ thể.

### 3.2. `items`

```cpp
DoublyLinkedList<T*> items;
```

Danh sách liên kết đôi chứa các con trỏ đến đối tượng Model đang được quản lý trong RAM.

Ví dụ với `Repository<Movie>`:

```text
items
  |
  +-- Movie*
  +-- Movie*
  +-- Movie*
```

Sử dụng `T*` giúp Repository quản lý các đối tượng thông qua con trỏ.

**Quan trọng:** Trong thiết kế hiện tại, Repository sở hữu các đối tượng được lưu trong `items` và chịu trách nhiệm giải phóng chúng.

### 3.3. `loaded`

```cpp
bool loaded;
```

Đánh dấu Repository đã nạp dữ liệu thành công hay chưa.

- `false`: Chưa nạp thành công hoặc đã gọi `clear()`.
- `true`: Đã nạp dữ liệu thành công bằng `load()`.

Giá trị ban đầu là `false`.

Thuộc tính này giúp `save()` tránh ghi file khi Repository chưa nạp dữ liệu.

### 3.4. `getId`

```cpp
string (*getId)(T*);
```

Con trỏ hàm nhận một đối tượng `T*` và trả về ID của đối tượng đó.

Repository không thể tự biết mỗi Model sử dụng tên hàm lấy ID nào.

Ví dụ:

```cpp
string getMovieId(Movie* movie)
{
    return movie->getMovieId();
}
```

Khi sử dụng `Repository<Movie>`, ta truyền hàm này vào constructor.

Nhờ vậy, Repository có thể tìm kiếm và kiểm tra trùng ID mà không cần biết chi tiết của lớp `Movie`.

### 3.5. `parse`

```cpp
T* (*parse)(const string&);
```

Con trỏ hàm dùng để chuyển một dòng TXT thành một đối tượng Model.

Ví dụ:

```text
MOV0001|Avatar|Sci-Fi|162|13|2026-01-01|2026-12-31|1
```

Sau khi `parse()` thành công, ta nhận được một đối tượng `Movie*` chứa các trường tương ứng.

Hàm `parse()` do Repository cụ thể cung cấp vì mỗi Model có cấu trúc dữ liệu khác nhau.

### 3.6. `serialize`

```cpp
string (*serialize)(T*);
```

Con trỏ hàm dùng để chuyển một đối tượng Model thành chuỗi TXT.

Đây là thao tác ngược lại với `parse()`.

Ví dụ:

```text
Movie*
   |
   | serialize()
   v
MOV0001|Avatar|Sci-Fi|162|13|2026-01-01|2026-12-31|1
```

`Repository<T>` chỉ gọi hàm được truyền vào, không tự quyết định thứ tự hoặc định dạng các trường.

---

## 4. Constructor

```cpp
Repository(
    const string& filePath,
    string (*getId)(T*),
    T* (*parse)(const string&),
    string (*serialize)(T*)
);
```

### Chức năng

Khởi tạo Repository với các thông tin cần thiết.

Constructor nhận bốn tham số:

1. `filePath`: Đường dẫn file TXT.
2. `getId`: Hàm lấy ID từ đối tượng Model.
3. `parse`: Hàm chuyển dòng TXT thành Model.
4. `serialize`: Hàm chuyển Model thành dòng TXT.

### Cách hoạt động

```cpp
: filePath(filePath),
  loaded(false),
  getId(getId),
  parse(parse),
  serialize(serialize)
{
}
```

Constructor gán các tham số cho thuộc tính tương ứng và đặt `loaded = false`.

Constructor **không tự đọc file**.

Muốn nạp dữ liệu, cần gọi `load()`.

---

## 5. Destructor — `~Repository()`

```cpp
Repository<T>::~Repository()
{
    clear();
}
```

### Chức năng

Giải phóng dữ liệu khi đối tượng Repository bị hủy.

Destructor gọi `clear()` để xóa các đối tượng Model đang được quản lý trong RAM.

Điều này giúp hạn chế rò rỉ bộ nhớ.

**Lưu ý về quyền sở hữu:** Không được tự ý `delete` một đối tượng đang thuộc quyền quản lý của Repository. Ngoài ra, lớp hiện chưa vô hiệu hóa thao tác sao chép; việc sao chép Repository có thể khiến nhiều đối tượng cùng giữ các con trỏ và gây lỗi giải phóng bộ nhớ hai lần.

---

## 6. Hàm `clear()`

```cpp
void clear();
```

### Chức năng

Xóa toàn bộ đối tượng Model trong Repository và giải phóng bộ nhớ.

### Cách hoạt động

Hàm thực hiện vòng lặp khi danh sách vẫn còn phần tử:

```cpp
while (items.nodeCount() > 0)
{
    ListNode<T*>* node = items.getAt(0);
    delete node->data;
    items.popFront();
}
loaded = false;
```

Giải thích:

1. `items.nodeCount() > 0`: Kiểm tra danh sách còn phần tử.
2. `items.getAt(0)`: Lấy node đầu tiên.
3. `delete node->data`: Giải phóng đối tượng Model.
4. `items.popFront()`: Xóa node đầu tiên khỏi danh sách.
5. Lặp lại đến khi danh sách rỗng.
6. Đặt `loaded = false`.

### Vì sao phải đặt `loaded = false`?

Vì sau khi `clear()`, Repository không còn dữ liệu đã được nạp.

Nếu giữ `loaded = true`, chương trình có thể gọi `save()` và ghi danh sách rỗng xuống file, làm mất dữ liệu cũ.

---

## 7. Hàm `count()`

```cpp
int count();
```

### Chức năng

Trả về số lượng đối tượng hiện có trong Repository.

### Cách hoạt động

```cpp
return items.nodeCount();
```

Hàm sử dụng `nodeCount()` của `DoublyLinkedList`.

Ví dụ:

```cpp
int total = repo.count();
```

Nếu danh sách có 5 đối tượng thì kết quả là `5`.

---

## 8. Hàm `getAt(int index)`

```cpp
T* getAt(int index);
```

### Chức năng

Lấy con trỏ đến đối tượng Model tại vị trí `index`.

### Cách hoạt động

```cpp
ListNode<T*>* node = items.getAt(index);

if (node == nullptr)
{
    return nullptr;
}

return node->data;
```

Giải thích:

1. Tìm node tại vị trí `index`.
2. Nếu không tìm thấy node, trả về `nullptr`.
3. Nếu tìm thấy, trả về con trỏ Model được lưu trong node.

Ví dụ:

```cpp
Movie* movie = repo.getAt(0);
```

Kết quả là con trỏ đến phim đầu tiên, nếu vị trí đó tồn tại.

**Lưu ý:** Hàm trả về con trỏ trực tiếp, không tạo bản sao. Việc thay đổi đối tượng qua con trỏ sẽ thay đổi dữ liệu đang được Repository quản lý.

---

## 9. Hàm `isLoaded()`

```cpp
bool isLoaded();
```

### Chức năng

Kiểm tra Repository đã nạp dữ liệu thành công hay chưa.

### Cách hoạt động

```cpp
return loaded;
```

Ví dụ:

```cpp
if (!repo.isLoaded())
{
    repo.load();
}
```

Hàm chỉ kiểm tra trạng thái `loaded`, không kiểm tra file trên ổ đĩa.

---

## 10. Hàm `findById(const string& id)`

```cpp
T* findById(const string& id);
```

### Chức năng

Tìm một đối tượng Model dựa trên ID.

### Cách hoạt động

Hàm duyệt qua từng đối tượng trong danh sách:

```cpp
for (int i = 0; i < count(); i++)
{
    T* item = getAt(i);

    if (getId(item) == id)
    {
        return item;
    }
}

return nullptr;
```

Giải thích:

1. Duyệt danh sách từ phần tử đầu tiên.
2. Lấy đối tượng tại vị trí hiện tại.
3. Gọi `getId(item)` để lấy ID.
4. So sánh với ID cần tìm.
5. Nếu trùng, trả về con trỏ đối tượng.
6. Nếu duyệt hết mà không tìm thấy, trả về `nullptr`.

Ví dụ:

```cpp
Movie* movie = repo.findById("MOV0001");
```

### Độ phức tạp

Hàm sử dụng tìm kiếm tuyến tính và gọi `getAt(i)` ở mỗi vòng lặp. Do đó, hiệu năng thực tế còn phụ thuộc cách `DoublyLinkedList::getAt()` được triển khai; với danh sách liên kết phải duyệt đến vị trí `i`, tổng chi phí có thể là `O(n²)`.

---

## 11. Hàm `add(T* item)`

```cpp
bool add(T* item);
```

### Chức năng

Thêm một đối tượng Model vào danh sách đang quản lý.

### Cách hoạt động

**Bước 1:** Kiểm tra con trỏ có hợp lệ không.

```cpp
if (item == nullptr)
{
    return false;
}
```

**Bước 2:** Lấy ID của đối tượng.

```cpp
string id = getId(item);
```

**Bước 3:** Không cho phép ID rỗng.

```cpp
if (id.empty())
{
    return false;
}
```

**Bước 4:** Kiểm tra ID đã tồn tại hay chưa.

```cpp
if (findById(id) != nullptr)
{
    return false;
}
```

**Bước 5:** Thêm đối tượng vào cuối danh sách.

```cpp
items.pushBack(item);
return true;
```

### Giá trị trả về

- `true`: Thêm thành công.
- `false`: Con trỏ null, ID rỗng hoặc ID đã tồn tại.

### Lưu ý

- `add()` chỉ thêm dữ liệu trong RAM, **không tự ghi xuống file**.
- Phiên bản hiện tại chưa kiểm tra `loaded` trong `add()`.
- Repository không kiểm tra các quy tắc riêng của Model như thời lượng phim hoặc ngày phát hành.
- Khi thêm thành công, Repository nhận quyền sở hữu con trỏ và sẽ giải phóng đối tượng khi cần.
- Khi trả về `false`, hàm không giải phóng con trỏ được truyền vào.

---

## 12. Hàm `load()`

```cpp
bool load();
```

### Chức năng

Đọc dữ liệu từ file TXT, chuyển thành các đối tượng Model và đưa vào danh sách trong RAM.

Đây là một trong những hàm quan trọng nhất của Repository.

### Bước 1: Đọc dữ liệu từ file

```cpp
vector<string> lines;

if (!FileIO::readLines(filePath, lines))
{
    return false;
}
```

`FileIO::readLines()` đọc các dòng dữ liệu từ file vào `vector<string>`.

Nếu không đọc được file, hàm trả về `false`.

### Bước 2: Tạo danh sách tạm

```cpp
DoublyLinkedList<T*> tempItems;
```

Danh sách tạm được sử dụng để tránh làm mất dữ liệu cũ nếu quá trình đọc hoặc chuyển đổi gặp lỗi.

Ví dụ:

```text
items (dữ liệu cũ)
  |
  +-- Movie A
  +-- Movie B

tempItems (đang đọc dữ liệu mới)
  |
  +-- Movie C
  +-- Movie D
```

Trong quá trình đọc, `items` vẫn được giữ nguyên.

### Bước 3: Chuyển từng dòng thành Model

```cpp
T* item = parse(lines[i]);
```

Hàm gọi `parse()` được truyền vào constructor.

Nếu `parse()` trả về `nullptr`, Repository ném ngoại lệ vì dữ liệu không hợp lệ.

### Bước 4: Kiểm tra ID

Repository kiểm tra:

- ID không được rỗng.
- ID không được trùng với các đối tượng đã đọc vào danh sách tạm.

Nếu ID không hợp lệ, đối tượng vừa tạo được giải phóng và quá trình nạp dữ liệu bị hủy.

### Bước 5: Thêm vào danh sách tạm

```cpp
tempItems.pushBack(item);
```

Đối tượng hợp lệ được thêm vào danh sách tạm.

### Bước 6: Xử lý khi có lỗi

Nếu xảy ra ngoại lệ, hàm giải phóng các đối tượng trong `tempItems`, sau đó ném lại ngoại lệ.

Danh sách `items` cũ chưa bị xóa.

### Bước 7: Thay thế dữ liệu cũ

Chỉ khi toàn bộ quá trình đọc và kiểm tra thành công, Repository mới gọi:

```cpp
clear();
```

Sau đó chuyển các con trỏ từ `tempItems` sang `items`.

Cuối cùng:

```cpp
loaded = true;
return true;
```

### Kết quả

- Đọc thành công: Dữ liệu mới thay thế dữ liệu cũ, `loaded = true`.
- `FileIO::readLines()` thất bại: Trả về `false`, giữ nguyên dữ liệu cũ.
- Parse hoặc kiểm tra ID thất bại: Giải phóng danh sách tạm và ném ngoại lệ.

**Lưu ý kỹ thuật:** Cơ chế danh sách tạm được thiết kế để bảo vệ dữ liệu cũ khi đọc lỗi. Tuy nhiên, quá trình chuyển dữ liệu từ danh sách tạm sang danh sách chính chưa có cơ chế rollback riêng nếu thao tác thêm node phát sinh lỗi.

---

## 13. Hàm `save()`

```cpp
bool save();
```

### Chức năng

Chuyển toàn bộ đối tượng Model trong RAM thành các dòng TXT và ghi xuống file.

### Bước 1: Kiểm tra trạng thái

```cpp
if (!loaded)
{
    return false;
}
```

Nếu Repository chưa nạp dữ liệu thành công, hàm không cho phép ghi file.

### Bước 2: Tạo danh sách dòng TXT

```cpp
vector<string> lines;
```

### Bước 3: Chuyển Model thành chuỗi

```cpp
for (int i = 0; i < count(); i++)
{
    T* item = getAt(i);
    string line = serialize(item);
    lines.push_back(line);
}
```

Hàm duyệt toàn bộ danh sách, gọi `serialize()` cho từng đối tượng và đưa kết quả vào `lines`.

### Bước 4: Ghi dữ liệu xuống file

```cpp
return FileIO::writeLines(filePath, lines);
```

`FileIO` chịu trách nhiệm thực hiện thao tác ghi file.

### Giá trị trả về

- `true`: `FileIO::writeLines()` báo ghi thành công.
- `false`: Repository chưa load hoặc `FileIO::writeLines()` báo thất bại.

Nếu `serialize()` ném ngoại lệ, `save()` hiện không bắt ngoại lệ đó.

### Lưu ý

`save()` ghi toàn bộ danh sách hiện tại, không chỉ ghi các đối tượng vừa được thêm hoặc sửa.

Vì vậy, trước khi lưu cần bảo đảm dữ liệu trong RAM hợp lệ.

---

## 14. Cách sử dụng `Repository<T>` trong Repository cụ thể

Mỗi Repository cụ thể cần cung cấp ba hàm để `Repository<T>` có thể làm việc với Model tương ứng.

| Hàm | Trách nhiệm |
|---|---|
| `getId` | Lấy ID của Model |
| `parse` | Chuyển chuỗi TXT thành Model |
| `serialize` | Chuyển Model thành chuỗi TXT |

Ví dụ minh họa với `Movie`:

```cpp
string getMovieId(Movie* movie)
{
    return movie->getMovieId();
}
```

Sau khi đã xây dựng đầy đủ các hàm chuyển đổi tương thích với chữ ký yêu cầu, có thể khởi tạo:

```cpp
Repository<Movie> repo(
    "data/movies.txt",
    getMovieId,
    parseMovie,
    serializeMovie
);
```

Trong đó `parseMovie` và `serializeMovie` là các hàm cần được triển khai riêng.

Ví dụ sử dụng:

```cpp
repo.load();

Movie* movie = repo.findById("MOV0001");

int total = repo.count();

repo.save();
```

**Quan trọng:** Đây là ví dụ minh họa cách tích hợp, không phải khẳng định `parseMovie` và `serializeMovie` đã tồn tại trong code hiện tại.

---

## 15. Những nguyên tắc cần nhớ

### 15.1. Repository quản lý bộ nhớ

Các đối tượng được thêm thành công vào Repository sẽ do Repository quản lý và giải phóng.

Không tự ý xóa các con trỏ mà Repository đang sở hữu.

### 15.2. `load()` và `save()` là hai thao tác riêng biệt

- `load()` đọc dữ liệu từ file vào RAM.
- `save()` ghi dữ liệu từ RAM xuống file.
- `add()` chỉ thay đổi dữ liệu trong RAM.

Không phải cứ thêm một đối tượng là file TXT được cập nhật ngay.

### 15.3. Repository chung không xử lý mọi quy tắc nghiệp vụ

`Repository<T>` chỉ kiểm tra các điều kiện chung như con trỏ null, ID rỗng và ID trùng.

Các điều kiện đặc thù của Model phải được xử lý ở lớp phù hợp.

Các quy tắc nghiệp vụ liên quan đến nhiều Model hoặc quyền người dùng thuộc Service.

### 15.4. Không tự ý thay đổi định dạng TXT

Hàm `parse()` và `serialize()` phải thống nhất với định dạng dữ liệu được quy định trong `docs/TongQuan/data_format.md`.

### 15.5. Không sao chép Repository tùy ý

Do Repository đang sở hữu các con trỏ thô (`T*`) và destructor giải phóng chúng, việc sao chép mặc định có nguy cơ dẫn đến lỗi bộ nhớ.

Cần xử lý vấn đề quyền sở hữu và sao chép trước khi sử dụng Repository trong các tình huống có sao chép đối tượng.

---

## 16. Tổng kết các hàm

| Hàm | Chức năng | Giá trị trả về |
|---|---|---|
| `Repository(...)` | Khởi tạo Repository | Không có |
| `~Repository()` | Giải phóng dữ liệu | Không có |
| `clear()` | Xóa toàn bộ dữ liệu trong RAM | `void` |
| `count()` | Đếm số đối tượng | `int` |
| `getAt(index)` | Lấy đối tượng theo vị trí | `T*` hoặc `nullptr` |
| `isLoaded()` | Kiểm tra trạng thái load | `bool` |
| `findById(id)` | Tìm đối tượng theo ID | `T*` hoặc `nullptr` |
| `add(item)` | Thêm đối tượng vào RAM | `bool` |
| `load()` | Đọc file và nạp Model vào RAM | `bool`, có thể ném ngoại lệ |
| `save()` | Ghi Model từ RAM xuống file | `bool`, có thể ném ngoại lệ |