# InvoiceRepository

## 1. Tổng quan

`InvoiceRepository` là lớp thuộc tầng **Data Access**, chịu trách nhiệm quản lý dữ liệu hóa đơn (`Invoice`) của hệ thống bán vé rạp chiếu phim.

Lớp sử dụng `Repository<Invoice>` để thực hiện các thao tác lưu trữ cơ bản, đồng thời phối hợp với các Repository khác để kiểm tra và thiết lập quan hệ giữa hóa đơn với suất chiếu, nhân viên và thành viên.

**File liên quan:**

- `src/data_access/InvoiceRepository.h`
- `src/data_access/InvoiceRepository.cpp`
- `src/data_access/Repository.h`
- `src/models/Invoice.h`
- `src/models/Invoice.cpp`
- `src/data_access/ShowtimeRepository.h`
- `src/data_access/UserRepository.h`
- `src/data_access/MemberRepository.h`
- `tests/test_invoice_ticket_repository.cpp`

**Trách nhiệm chính:**

1. Đọc dữ liệu hóa đơn từ file TXT.
2. Chuyển đổi dữ liệu giữa chuỗi TXT và đối tượng `Invoice`.
3. Kiểm tra tính hợp lệ của dữ liệu hóa đơn.
4. Kiểm tra các tham chiếu đến suất chiếu, nhân viên và thành viên.
5. Thiết lập và tháo các liên kết giữa những đối tượng.
6. Thêm hóa đơn mới vào bộ nhớ.
7. Tìm kiếm, truy xuất và lọc hóa đơn.
8. Ghi dữ liệu hóa đơn xuống file TXT.
9. Đảm bảo không để lại liên kết sai khi quá trình nạp dữ liệu thất bại.

`InvoiceRepository` không trực tiếp xử lý nghiệp vụ bán vé, tính giá vé, giảm giá hoặc phân quyền thao tác. Những trách nhiệm đó thuộc tầng Business Logic.

---

## 2. Các thư viện sử dụng

### 2.1. `<string>`

Cung cấp kiểu dữ liệu `std::string`, dùng để lưu trữ và xử lý chuỗi ký tự.

Ví dụ:

```cpp
string invoiceId = "INV0001";
```

Một số hàm thường dùng:

| Hàm | Ý nghĩa |
|---|---|
| `find()` | Tìm vị trí xuất hiện của chuỗi hoặc ký tự |
| `substr()` | Lấy một phần của chuỗi |
| `size()` | Trả về độ dài chuỗi |
| `empty()` | Kiểm tra chuỗi có rỗng không |
| `to_string()` | Chuyển số thành chuỗi |

### 2.2. `<vector>`

Cung cấp `std::vector`, một cấu trúc dữ liệu dạng mảng có khả năng thay đổi kích thước.

Ví dụ:

```cpp
vector<Invoice*> result;
result.push_back(invoice);
```

`vector<Invoice*>` lưu các con trỏ đến hóa đơn, không tự tạo bản sao của những đối tượng hóa đơn.

Các hàm quan trọng:

- `push_back(value)`: thêm phần tử vào cuối vector.
- `size()`: trả về số phần tử.
- `empty()`: kiểm tra vector rỗng.
- `operator[]`: truy cập phần tử theo chỉ số.

**Vì sao dùng `vector` để trả kết quả lọc?**

Các hàm như `getByStaff()` có thể trả về nhiều hóa đơn. `vector` giúp lưu danh sách kết quả và trả về thuận tiện mà không phải sao chép các đối tượng `Invoice`.

### 2.3. `<stdexcept>`

Cung cấp các lớp ngoại lệ tiêu chuẩn, chẳng hạn `std::runtime_error`.

Ví dụ:

```cpp
throw runtime_error("Du lieu Invoice khong hop le");
```

`throw` dùng để phát sinh ngoại lệ khi chương trình gặp tình huống không thể tiếp tục xử lý bình thường.

Ngoại lệ có thể được bắt bằng `try` và `catch`.

```cpp
try
{
    // Thuc hien thao tac
}
catch (const exception&)
{
    // Xu ly loi
}
```

`const exception&` có nghĩa là bắt ngoại lệ thông qua tham chiếu hằng, tránh sao chép đối tượng ngoại lệ.

### 2.4. `<cmath>`

Cung cấp các hàm toán học.

Trong `InvoiceRepository`, hàm quan trọng là:

```cpp
isfinite(totalAmount)
```

Hàm `std::isfinite()` kiểm tra một số thực có phải giá trị hữu hạn hay không.

Ví dụ:

- `100000.0`: hữu hạn.
- `0.0`: hữu hạn.
- `NaN`: không hữu hạn.
- `Infinity`: không hữu hạn.

Việc kiểm tra này giúp tránh lưu các giá trị tiền không hợp lệ.

### 2.5. Các thư viện nội bộ

| File | Mục đích |
|---|---|
| `Repository.h` | Cung cấp lớp Repository tổng quát |
| `Invoice.h` | Định nghĩa Model hóa đơn |
| `ShowtimeRepository.h` | Truy xuất suất chiếu |
| `UserRepository.h` | Truy xuất nhân viên |
| `MemberRepository.h` | Truy xuất thành viên |
| `Validator.h` | Kiểm tra ID, ngày giờ và định dạng số |
| `DateTime.h` | Hỗ trợ so sánh thời điểm |

---

## 3. Cấu trúc lớp

### 3.1. Thuộc tính

```cpp
Repository<Invoice> repo;

ShowtimeRepository& showtimeRepo;
UserRepository& userRepo;
MemberRepository& memberRepo;
```

**`Repository<Invoice> repo`**

Đây là đối tượng thuộc lớp template `Repository<T>`, với `T` được thay bằng `Invoice`.

Nhờ đó, `InvoiceRepository` có thể tái sử dụng những chức năng chung như:

- `load()`
- `save()`
- `add()`
- `findById()`
- `getAt()`
- `count()`
- `clear()`
- `isLoaded()`

**Ba thuộc tính tham chiếu Repository**

Dấu `&` thể hiện tham chiếu đến một đối tượng đã tồn tại.

Ví dụ:

```cpp
UserRepository& userRepo;
```

`InvoiceRepository` không tạo bản sao của `UserRepository`, mà sử dụng chính đối tượng được truyền vào constructor.

Điều này giúp các Repository làm việc trên cùng một tập dữ liệu trong bộ nhớ.

**Lưu ý về vòng đời:** Những Repository được tham chiếu phải tồn tại lâu hơn `InvoiceRepository`.

### 3.2. Hàm static

```cpp
static string getInvoiceId(Invoice* invoice);
static Invoice* stringToInvoice(const string& line);
static string invoiceToString(Invoice* invoice);
static bool isValidInvoice(const Invoice* invoice);
```

Từ khóa `static` ở đây cho biết hàm thuộc về lớp và không cần truy cập một đối tượng `InvoiceRepository` cụ thể.

Các hàm này chủ yếu xử lý dữ liệu đầu vào, đầu ra và kiểm tra cấu trúc hóa đơn.

Nhờ không phụ thuộc vào trạng thái của một đối tượng Repository cụ thể, chúng phù hợp để truyền làm callback cho `Repository<Invoice>`.

---

## 4. Constructor và Destructor

### 4.1. Constructor

```cpp
InvoiceRepository::InvoiceRepository(const string& filePath,
                                     ShowtimeRepository& showtimeRepo,
                                     UserRepository& userRepo,
                                     MemberRepository& memberRepo)
    : repo(filePath, getInvoiceId, stringToInvoice, invoiceToString),
      showtimeRepo(showtimeRepo),
      userRepo(userRepo),
      memberRepo(memberRepo)
{
}
```

Constructor nhận đường dẫn file hóa đơn và ba Repository phụ thuộc.

**Giải thích initializer list**

Phần sau dấu `:` được gọi là *member initializer list*, dùng để khởi tạo các thuộc tính trước khi thân constructor được thực thi.

Ví dụ:

```cpp
userRepo(userRepo)
```

- `userRepo` bên trái: thuộc tính của lớp.
- `userRepo` bên phải: tham số truyền vào constructor.

Với thuộc tính kiểu tham chiếu, việc khởi tạo bằng initializer list là bắt buộc.

**Giải thích callback**

```cpp
repo(filePath, getInvoiceId, stringToInvoice, invoiceToString)
```

Bốn tham số lần lượt là:

1. Đường dẫn file TXT.
2. Hàm lấy ID của một Invoice.
3. Hàm chuyển dòng TXT thành Invoice.
4. Hàm chuyển Invoice thành dòng TXT.

`Repository<Invoice>` sẽ gọi những hàm này khi cần, thay vì tự biết cấu trúc riêng của hóa đơn.

Đây là kỹ thuật truyền hàm làm tham số, thường gọi là **callback**.

### 4.2. Destructor

```cpp
InvoiceRepository::~InvoiceRepository()
{
    unlinkAll();
}
```

Destructor được gọi khi đối tượng `InvoiceRepository` bị hủy.

Hàm `unlinkAll()` tháo liên kết của các hóa đơn khỏi những đối tượng liên quan trước khi `Repository<Invoice>` giải phóng dữ liệu do nó sở hữu.

Điều này giúp tránh việc `User` hoặc `Member` tiếp tục giữ con trỏ đến hóa đơn đã bị xóa.

---

## 5. Các hàm hỗ trợ chuyển đổi dữ liệu

### 5.1. `getInvoiceId()`

```cpp
string InvoiceRepository::getInvoiceId(Invoice* invoice)
{
    if (invoice == nullptr)
    {
        return "";
    }

    return invoice->getInvoiceId();
}
```

**Mục đích:** Lấy mã hóa đơn từ đối tượng `Invoice`.

Nếu con trỏ bằng `nullptr`, hàm trả về chuỗi rỗng để tránh truy cập con trỏ không hợp lệ.

Toán tử `->` dùng để truy cập thành viên thông qua con trỏ.

Ví dụ:

```cpp
invoice->getInvoiceId();
```

Tương đương:

```cpp
(*invoice).getInvoiceId();
```

### 5.2. `stringToInvoice()`

**Mục đích:** Chuyển một dòng TXT thành đối tượng `Invoice`.

Định dạng dữ liệu:

```text
invoiceId|showtimeId|staffId|memberId|createdAt|totalAmount
```

Ví dụ:

```text
INV0001|SHW0001|USR0001|MEM0001|2027-01-10T09:00:00|150000
```

Quy trình:

1. Tách dòng TXT thành sáu trường.
2. Kiểm tra số lượng trường.
3. Chuyển trường `totalAmount` thành `double`.
4. Tạo đối tượng `Invoice`.
5. Kiểm tra tính hợp lệ của đối tượng.
6. Trả về con trỏ Invoice nếu hợp lệ.

**Giải thích `size_t`**

```cpp
size_t start = 0;
```

`size_t` là kiểu số nguyên không dấu thường được dùng để biểu diễn kích thước hoặc vị trí trong bộ nhớ và các container.

Các hàm như `string::find()` và `string::size()` sử dụng kiểu này.

**Giải thích `string::npos`**

```cpp
size_t pos = line.find('|', start);

if (pos == string::npos)
{
    // Khong tim thay dau |
}
```

`string::npos` là giá trị đặc biệt biểu thị việc không tìm thấy vị trí cần tìm.

**Giải thích `substr()`**

```cpp
line.substr(start, pos - start)
```

Hàm lấy chuỗi con bắt đầu từ `start`, với độ dài `pos - start`.

**Giải thích `stod()`**

```cpp
totalAmount = stod(fields[5], &pos);
```

`std::stod()` chuyển một chuỗi thành số thực kiểu `double`.

Tham số `&pos` nhận số ký tự mà hàm đã xử lý thành công.

Ví dụ, với chuỗi `"150000abc"`, `stod()` có thể đọc được phần `150000` nhưng chưa đọc hết chuỗi.

Vì vậy cần kiểm tra:

```cpp
if (pos != fields[5].size())
{
    throw runtime_error("Sai dinh dang tong tien");
}
```

Điều này ngăn các giá trị có phần ký tự thừa được chấp nhận.

**Quản lý bộ nhớ**

Đối tượng được tạo bằng:

```cpp
Invoice* invoice = new Invoice(...);
```

Nếu dữ liệu không hợp lệ, phải gọi `delete invoice` trước khi ném ngoại lệ để tránh rò rỉ bộ nhớ.

### 5.3. `invoiceToString()`

**Mục đích:** Chuyển đối tượng `Invoice` thành chuỗi theo đúng định dạng TXT.

Hàm kiểm tra dữ liệu trước khi chuyển đổi.

Sau đó, các trường được ghép bằng ký tự `|`.

```cpp
to_string(invoice->getTotalAmount())
```

`std::to_string()` chuyển giá trị số thành chuỗi.

Ví dụ:

```cpp
to_string(150000.0)
```

Thường tạo ra:

```text
150000.000000
```

Định dạng này vẫn biểu diễn giá trị số hợp lệ trong TXT.

---

## 6. Kiểm tra dữ liệu và tham chiếu

### 6.1. `isValidInvoice()`

Hàm kiểm tra dữ liệu cơ bản của một hóa đơn, không cần truy cập các Repository khác.

Các điều kiện gồm:

| Trường | Điều kiện |
|---|---|
| `invoiceId` | ID hợp lệ, tiền tố `INV` |
| `showtimeId` | ID hợp lệ, tiền tố `SHW` |
| `staffId` | ID hợp lệ, tiền tố `USR` |
| `memberId` | Rỗng hoặc ID hợp lệ với tiền tố `MEM` |
| `createdAt` | Đúng định dạng ngày giờ |
| `totalAmount` | Số hữu hạn, không âm |

**Phân biệt:** ID đúng định dạng không đồng nghĩa với đối tượng được tham chiếu thực sự tồn tại.

Ví dụ, `SHW9999` có thể đúng định dạng nhưng không có trong dữ liệu suất chiếu.

### 6.2. `isValidReference()`

Hàm kiểm tra các quan hệ giữa Invoice và những đối tượng khác.

Quy trình:

1. Gọi `isValidInvoice()` để kiểm tra dữ liệu cơ bản.
2. Kiểm tra các Repository phụ thuộc đã load.
3. Tìm suất chiếu theo `showtimeId`.
4. Tìm nhân viên theo `staffId`.
5. Kiểm tra vai trò nhân viên là `STAFF` hoặc `ADMIN`.
6. Kiểm tra thời điểm tạo hóa đơn trước thời điểm bắt đầu suất chiếu.
7. Nếu có `memberId`, kiểm tra thành viên tồn tại.

**Vì sao phải tách hai hàm kiểm tra?**

`isValidInvoice()` chỉ phụ thuộc vào dữ liệu của hóa đơn.

`isValidReference()` phụ thuộc vào các đối tượng khác trong hệ thống.

Việc tách riêng giúp code rõ ràng, dễ tái sử dụng và dễ kiểm thử.

**Lưu ý:** Điều kiện `createdAt` phải trước `startAt` là điều kiện đang được áp dụng trong bản triển khai Repository. Cần giữ đồng bộ với quy tắc nghiệp vụ được chốt cho hệ thống.

---

## 7. Nạp và lưu dữ liệu

### 7.1. `load()`

Đây là một trong những hàm quan trọng nhất của lớp.

**Mục đích:** Đọc dữ liệu hóa đơn từ TXT và thiết lập các liên kết trong bộ nhớ.

Quá trình gồm ba giai đoạn.

**Giai đoạn 1: Kiểm tra điều kiện ban đầu**

- Không cho phép load lại khi Repository đã load.
- Yêu cầu `ShowtimeRepository`, `UserRepository` và `MemberRepository` đã load.

**Giai đoạn 2: Đọc và kiểm tra dữ liệu**

```cpp
repo.load();
```

`Repository<Invoice>` đọc file và sử dụng callback `stringToInvoice()` để tạo đối tượng.

Sau đó, `InvoiceRepository` duyệt từng hóa đơn để kiểm tra tham chiếu.

**Giai đoạn 3: Liên kết đối tượng**

Với mỗi hóa đơn:

```cpp
invoice->linkShowtime(showtime);
invoice->linkSeller(seller);
invoice->linkMember(member);
```

Các hàm này thiết lập quan hệ giữa hóa đơn và những đối tượng tương ứng.

**Giải thích rollback**

Rollback có nghĩa là hoàn tác những thay đổi đã thực hiện khi một thao tác thất bại.

Ví dụ:

- Hóa đơn thứ nhất đã liên kết thành công.
- Hóa đơn thứ hai liên kết thất bại.
- Không thể giữ trạng thái mà một số hóa đơn đã liên kết, một số chưa.

Khi đó:

```cpp
unlinkAll();
repo.clear();
```

- `unlinkAll()`: tháo các liên kết đã tạo.
- `repo.clear()`: xóa các hóa đơn đã nạp và đưa Repository về trạng thái chưa load.

**Giải thích `catch (...)`**

```cpp
catch (...)
{
    unlinkAll();
    repo.clear();
    throw;
}
```

`catch (...)` bắt mọi ngoại lệ C++ có thể được xử lý bởi cơ chế exception.

`throw;` không có đối số sẽ ném lại ngoại lệ hiện tại, giữ nguyên loại ngoại lệ ban đầu.

Nhờ vậy, hàm có thể dọn dẹp dữ liệu trước khi chuyển lỗi cho tầng gọi bên ngoài.

### 7.2. `save()`

```cpp
bool InvoiceRepository::save()
{
    return repo.save();
}
```

Hàm ủy quyền việc ghi dữ liệu cho `Repository<Invoice>`.

Khi ghi, Repository sử dụng callback `invoiceToString()` để chuyển từng hóa đơn thành chuỗi TXT.

`save()` không tự kiểm tra lại toàn bộ quan hệ hóa đơn – vé.

---

## 8. Thêm hóa đơn

### `add(Invoice* invoice)`

**Mục đích:** Thêm một hóa đơn mới vào Repository.

Các bước:

1. Kiểm tra Repository đã load.
2. Kiểm tra dữ liệu và tham chiếu hợp lệ.
3. Kiểm tra không trùng `invoiceId`.
4. Tìm các đối tượng liên quan.
5. Liên kết hóa đơn với suất chiếu, nhân viên và thành viên.
6. Gọi `repo.add(invoice)` để thêm hóa đơn vào bộ nhớ.
7. Nếu thất bại, tháo những liên kết vừa tạo.

**Quy tắc sở hữu con trỏ**

Khi `repo.add(invoice)` thành công, Repository nhận trách nhiệm quản lý vòng đời đối tượng Invoice.

Khi `add()` trả về `false`, bên gọi vẫn chịu trách nhiệm giải phóng đối tượng chưa được thêm.

Không gọi `delete` một hóa đơn đã được Repository nhận quản lý.

**Lưu ý:** Hàm `add()` dành cho hóa đơn mới chưa được liên kết từ trước. Việc gọi với đối tượng đã có quan hệ khác cần được ngăn chặn hoặc xử lý riêng để tránh làm thay đổi liên kết ngoài ý muốn.

---

## 9. Các hàm truy vấn

### 9.1. `findById()`

```cpp
Invoice* InvoiceRepository::findById(const string& invoiceId)
{
    return repo.findById(invoiceId);
}
```

Tìm hóa đơn theo ID.

Trả về:

- Con trỏ `Invoice*` nếu tìm thấy.
- `nullptr` nếu không tìm thấy.

### 9.2. `getAt()`

```cpp
Invoice* InvoiceRepository::getAt(int index)
{
    return repo.getAt(index);
}
```

Trả về hóa đơn tại vị trí `index` trong danh sách.

Vị trí bắt đầu từ `0`.

### 9.3. `count()`

```cpp
int InvoiceRepository::count()
{
    return repo.count();
}
```

Trả về số lượng hóa đơn hiện có trong Repository.

### 9.4. `isLoaded()`

```cpp
bool InvoiceRepository::isLoaded()
{
    return repo.isLoaded();
}
```

Kiểm tra trạng thái nạp dữ liệu.

Hàm trả về `true` khi Repository đã được nạp thành công.

---

## 10. Các hàm lọc hóa đơn

### 10.1. `getByShowtime()`

Trả về danh sách hóa đơn thuộc một suất chiếu.

Ví dụ:

```cpp
vector<Invoice*> invoices =
    invoiceRepo.getByShowtime("SHW0001");
```

Hàm duyệt danh sách hóa đơn và so sánh `showtimeId`.

### 10.2. `getByStaff()`

Trả về danh sách hóa đơn do một nhân viên bán.

Ví dụ:

```cpp
vector<Invoice*> invoices =
    invoiceRepo.getByStaff("USR0001");
```

Hàm hỗ trợ các chức năng như xem lịch sử bán vé của nhân viên.

### 10.3. `getByMember()`

Trả về danh sách hóa đơn của một thành viên.

Ví dụ:

```cpp
vector<Invoice*> invoices =
    invoiceRepo.getByMember("MEM0001");
```

Nếu `memberId` rỗng hoặc Repository chưa load, hàm trả về vector rỗng.

**Độ phức tạp:** Cả ba hàm đều duyệt qua danh sách hóa đơn nên có độ phức tạp tuyến tính theo số hóa đơn, chưa tính chi phí truy cập từng phần tử của cấu trúc danh sách bên dưới.

---

## 11. Quản lý liên kết và bộ nhớ

### 11.1. `unlinkAll()`

```cpp
void InvoiceRepository::unlinkAll()
{
    for (int i = 0; i < repo.count(); i++)
    {
        Invoice* invoice = repo.getAt(i);

        if (invoice != nullptr)
        {
            invoice->unlinkReferences();
        }
    }
}
```

**Mục đích:** Tháo liên kết của tất cả hóa đơn đang được Repository quản lý.

Hàm này không trực tiếp xóa Invoice mà gọi:

```cpp
invoice->unlinkReferences();
```

`Invoice::unlinkReferences()` chịu trách nhiệm tháo quan hệ với User, Member và Showtime.

### 11.2. Phân biệt unlink và delete

**Unlink** là loại bỏ quan hệ giữa các đối tượng.

**Delete** là giải phóng vùng nhớ của đối tượng được tạo bằng `new`.

Hai thao tác này khác nhau.

Ví dụ, một nhân viên có thể giữ con trỏ đến hóa đơn trong danh sách lịch sử bán vé.

Nếu hóa đơn bị xóa nhưng con trỏ vẫn nằm trong danh sách của nhân viên, con trỏ đó trở thành **dangling pointer** (con trỏ treo).

Truy cập con trỏ treo có thể dẫn đến hành vi không xác định.

Vì vậy, phải tháo liên kết trước khi giải phóng hóa đơn.

### 11.3. Thứ tự hủy Repository

`InvoiceRepository` phụ thuộc vào `ShowtimeRepository`, `UserRepository` và `MemberRepository`.

Ngoài ra, `TicketRepository` có thể giữ con trỏ đến Invoice.

Do đó, thứ tự hủy phải bảo đảm:

- Các liên kết từ Ticket đến Invoice đã được xử lý trước khi Invoice bị xóa.
- Các Repository được Invoice tham chiếu vẫn còn tồn tại khi `InvoiceRepository` thực hiện destructor.

---

## 12. Thứ tự load dữ liệu

Đối với phần quản lý hóa đơn và vé, các quan hệ phụ thuộc cần được thiết lập theo thứ tự.

Một thứ tự phù hợp là:

1. `MovieRepository`
2. `RoomRepository`
3. `SeatRepository`
4. `ShowtimeRepository`
5. `UserRepository`
6. `MemberRepository`
7. `InvoiceRepository`
8. `TicketRepository`

Trong đó, các Repository độc lập có thể được nạp theo thứ tự khác, miễn là mọi phụ thuộc cần thiết đã được nạp trước khi liên kết.

**Quan trọng:** `InvoiceRepository` phải được load trước `TicketRepository`, vì vé cần tham chiếu đến hóa đơn đã tồn tại.

Sau khi cả hai Repository được nạp, hệ thống cần kiểm tra các quy tắc liên quan đến dữ liệu tổng hợp như số vé trong hóa đơn và tổng tiền hóa đơn.

---

## 13. Quy tắc nghiệp vụ liên quan

Theo định dạng dữ liệu hóa đơn và vé của dự án:

- Mỗi hóa đơn thuộc một suất chiếu.
- Mỗi hóa đơn có một người bán là Staff hoặc Admin.
- Thành viên có thể có hoặc không.
- Mỗi hóa đơn phải có ít nhất một vé.
- Các vé thuộc cùng hóa đơn phải có cùng `showtimeId` với hóa đơn.
- `totalAmount` phải bằng tổng `paidAmount` ban đầu của các vé, kể cả vé đã hủy.
- Khi so sánh số thực `double`, cần xét đến sai số tính toán.

Một số quy tắc trên cần được kiểm tra sau khi `TicketRepository` đã load, vì tại thời điểm `InvoiceRepository::load()`, danh sách vé có thể chưa tồn tại trong bộ nhớ.

Không nên từ chối hóa đơn chỉ vì chưa có vé ở giai đoạn đang nạp riêng dữ liệu hóa đơn.

---

## 14. Kiểm thử

**File test:**

`tests/test_invoice_ticket_repository.cpp`

Các nội dung cần kiểm tra gồm:

- Load file rỗng và file hợp lệ.
- Không cho phép load sai thứ tự phụ thuộc.
- Thêm hóa đơn hợp lệ.
- Từ chối hóa đơn trùng ID.
- Từ chối dữ liệu sai định dạng.
- Từ chối tham chiếu không tồn tại.
- Kiểm tra liên kết đến User, Member và Showtime.
- Kiểm tra rollback khi load thất bại.
- Kiểm tra các hàm truy vấn và lọc.
- Kiểm tra save và reload.
- Kiểm tra tương tác với `TicketRepository`.

**Kết quả hiện tại:** Theo kết quả chạy test được báo cáo, các test của InvoiceRepository và TicketRepository đã pass. Chưa ghi số lượng cụ thể vì chưa có bảng tổng kết Passed/Failed trong tài liệu này.

---

## 15. Tổng kết

`InvoiceRepository` là lớp quản lý dữ liệu hóa đơn, kết hợp khả năng lưu trữ tổng quát của `Repository<Invoice>` với những quy tắc kiểm tra tham chiếu riêng của hóa đơn.

Các điểm quan trọng nhất cần nắm:

1. **Template:** `Repository<Invoice>` tái sử dụng logic lưu trữ chung.
2. **Callback:** Cho phép Repository tổng quát xử lý ID và định dạng dữ liệu của Invoice.
3. **Reference:** Các Repository liên quan được truyền bằng tham chiếu để dùng chung dữ liệu.
4. **Link/Unlink:** Quản lý quan hệ giữa các đối tượng, tránh con trỏ treo.
5. **Rollback:** Hoàn tác trạng thái khi nạp hoặc liên kết thất bại.
6. **Exception:** Báo lỗi và bảo đảm dọn dẹp khi có ngoại lệ.
7. **Ownership:** Repository quản lý vòng đời những Invoice đã được thêm thành công.

Việc hiểu rõ các cơ chế này sẽ giúp triển khai `TicketRepository` và các Service nghiệp vụ bán vé an toàn, đồng bộ hơn.