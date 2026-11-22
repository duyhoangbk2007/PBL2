# InvoiceRepository — mã nguồn và quy tắc dữ liệu

## 1. Tổng quan

`InvoiceRepository` quản lý `Invoice` qua `Repository<Invoice>` và tham chiếu đến Showtime, User, Member. `repo` sở hữu hóa đơn; các con trỏ liên kết đến Model khác là con trỏ mượn. Nạp các Repository tham chiếu trước Invoice; hủy Invoice trước những Repository đó để destructor tháo liên kết an toàn.

Mỗi bản ghi TXT chứa ID hóa đơn, các ID tham chiếu, thời gian, tổng tiền và điểm. Repository kiểm tra định dạng, tham chiếu và liên kết; việc bán vé, tính giá, phân quyền thuộc Business. `rollbackPending()` chỉ dùng cho hóa đơn mới đang tạo, không dùng xóa lịch sử đã lưu. `load()`/`save()` xử lý riêng một file, không tạo transaction đa file.

Mã nguồn: `src/data_access/InvoiceRepository.cpp` và `src/data_access/InvoiceRepository.h`. Các đoạn code dưới đây giữ nguyên nội dung hàm, gồm dòng trống và comment trong hàm. Khi ghép các đoạn của một hàm theo thứ tự, ta thu được đúng định nghĩa gốc.

## 2. Chi tiết từng hàm

### InvoiceRepository::InvoiceRepository()

#### A. Tổng quan

- **Mục đích:** Khởi tạo Repository và các tham chiếu/callback cần dùng.
- **Tham số:** `const string& filePath, ShowtimeRepository& showtimeRepo, UserRepository& userRepo, MemberRepository& memberRepo`.
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

Danh sách sau dấu `:` khởi tạo thành viên trước thân constructor; tham chiếu được giữ để dùng về sau, còn thân hàm không nạp file.

### InvoiceRepository::~InvoiceRepository()

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
Kết thúc
```

#### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1 — Tháo liên kết đối tượng.**

```cpp
InvoiceRepository::~InvoiceRepository()
{
    unlinkAll();
}
```

Liên kết được đổi theo thứ tự trong code; nếu bước sau thất bại, phần đã nối phải được tháo trước khi xóa đối tượng để tránh con trỏ treo.

### InvoiceRepository::unlinkAll()

#### A. Tổng quan

- **Mục đích:** Tháo các liên kết đang giữ.
- **Tham số:** không có.
- **Giá trị trả về:** `void`; xem các nhánh `return` bên dưới.
- **Điều kiện quan trọng:** con trỏ rỗng.

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
void InvoiceRepository::unlinkAll()
{
    for (int i = 0; i < repo.count(); i++)
    {
        Invoice* invoice = repo.getAt(i);
        if (invoice != nullptr)
        {
            invoice->unlinkReferences();
```

Liên kết được đổi theo thứ tự trong code; nếu bước sau thất bại, phần đã nối phải được tháo trước khi xóa đối tượng để tránh con trỏ treo. Vòng lặp xét từng bản ghi; điều kiện bên trong quyết định tiếp tục, bỏ qua hay dừng khi gặp dữ liệu sai.

**Bước 2 — Chuẩn bị giá trị cho bước kế tiếp.**

```cpp
        }
    }
}
```

Đoạn này chuẩn bị hoặc trả giá trị theo code. Nó chỉ đổi RAM hoặc TXT nếu có lời gọi cập nhật hay ghi dữ liệu tương ứng.

### InvoiceRepository::getInvoiceId()

#### A. Tổng quan

- **Mục đích:** Lấy ID của đối tượng làm khóa.
- **Tham số:** `Invoice* invoice`.
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
string InvoiceRepository::getInvoiceId(Invoice* invoice)
{
    if (invoice == nullptr)
    {
        return "";
    }
    return invoice->getInvoiceId();
}
```

Đoạn này chuẩn bị hoặc trả giá trị theo code. Nó chỉ đổi RAM hoặc TXT nếu có lời gọi cập nhật hay ghi dữ liệu tương ứng.

### InvoiceRepository::isValidInvoice()

#### A. Tổng quan

- **Mục đích:** Kiểm tra tính hợp lệ của dữ liệu đầu vào.
- **Tham số:** `const Invoice* invoice`.
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
bool InvoiceRepository::isValidInvoice(const Invoice* invoice)
{
    if (invoice == nullptr)
    {
        return false;
    }
    if (!Validator::isValidId(invoice->getInvoiceId(), "INV") ||
        !Validator::isValidId(invoice->getShowtimeId(), "SHW") ||
        !Validator::isValidId(invoice->getStaffId(), "USR"))
    {
        return false;
```

Điều kiện hợp lệ ngăn dữ liệu sai định dạng hoặc ngoài giới hạn đi vào danh sách đang quản lý. `return false` kết thúc ngay nhánh không đạt; các thao tác cập nhật phía sau không chạy.

**Bước 2 — Kiểm tra dữ liệu hợp lệ.**

```cpp
    }
    if (!invoice->getMemberId().empty() &&
        !Validator::isValidId(invoice->getMemberId(), "MEM"))
    {
        return false;
    }
    if (!Validator::isValidDateTime(invoice->getCreatedAt()))
    {
        return false;
```

Điều kiện hợp lệ ngăn dữ liệu sai định dạng hoặc ngoài giới hạn đi vào danh sách đang quản lý. `return false` kết thúc ngay nhánh không đạt; các thao tác cập nhật phía sau không chạy.

**Bước 3 — Từ chối điều kiện không đạt.**

```cpp
    }
    double totalAmount = invoice->getTotalAmount();
    if (!isfinite(totalAmount) || totalAmount < 0)
    {
        return false;
    }
    return true;
}
```

`return false` kết thúc ngay nhánh không đạt; các thao tác cập nhật phía sau không chạy.

### InvoiceRepository::stringToInvoice()

#### A. Tổng quan

- **Mục đích:** Chuyển một dòng TXT thành đối tượng Model.
- **Tham số:** `const string& line`.
- **Giá trị trả về:** `Invoice*`; xem các nhánh `return` bên dưới.
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
Kiểm tra dữ liệu hợp lệ (lỗi: throw)
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
Kết thúc
```

#### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1 — Duyệt và đối chiếu từng phần tử.**

```cpp
Invoice* InvoiceRepository::stringToInvoice(const string& line)
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

    if (fields.size() != 6)
    {
        throw runtime_error("Sai so luong truong Invoice");
```

`throw` báo lỗi bằng ngoại lệ cho bên gọi, khác với kết quả `false` thông thường.

**Bước 3 — Kiểm tra dữ liệu hợp lệ.**

```cpp
    }
    double totalAmount = 0;
    try
    {
        if (!Validator::isValidNumber(fields[5]))
        {
            throw runtime_error("Sai dinh dang tong tien");
        }
```

Điều kiện hợp lệ ngăn dữ liệu sai định dạng hoặc ngoài giới hạn đi vào danh sách đang quản lý. `try/catch` xử lý ngoại lệ trong phạm vi hiện tại; code dọn các thay đổi trước khi trả lỗi hoặc ném lại, nên thứ tự dọn có ý nghĩa. `throw` báo lỗi bằng ngoại lệ cho bên gọi, khác với kết quả `false` thông thường.

**Bước 4 — Bắt lỗi và khôi phục trạng thái.**

```cpp
        size_t pos = 0;
        totalAmount = stod(fields[5], &pos);
        if (pos != fields[5].size())
        {
            throw runtime_error("Sai dinh dang tong tien");
        }
    }
    catch (const exception&)
    {
        throw runtime_error("Tong tien Invoice khong hop le");
```

`try/catch` xử lý ngoại lệ trong phạm vi hiện tại; code dọn các thay đổi trước khi trả lỗi hoặc ném lại, nên thứ tự dọn có ý nghĩa. `throw` báo lỗi bằng ngoại lệ cho bên gọi, khác với kết quả `false` thông thường. Phép parse đổi chuỗi TXT sang trường có kiểu; dữ liệu không chuyển được phải bị từ chối trước khi tạo đối tượng hợp lệ.

**Bước 5 — Chuẩn bị giá trị cho bước kế tiếp.**

```cpp
    }

    Invoice* invoice = new Invoice(fields[0],
                                   fields[1],
                                   fields[2],
                                   fields[3],
                                   fields[4],
                                   totalAmount);
```

`new` cấp phát Model trên heap. Cho đến khi Repository nhận thành công, nhánh lỗi hoặc caller vẫn phải chịu trách nhiệm giải phóng.

**Bước 6 — Kiểm tra dữ liệu hợp lệ.**

```cpp

    if (!isValidInvoice(invoice))
    {
        delete invoice;
        throw runtime_error("Du lieu Invoice khong hop le");
    }
    return invoice;
}
```

Điều kiện hợp lệ ngăn dữ liệu sai định dạng hoặc ngoài giới hạn đi vào danh sách đang quản lý. `delete` giải phóng đối tượng; không được dùng lại con trỏ sau lệnh này. `throw` báo lỗi bằng ngoại lệ cho bên gọi, khác với kết quả `false` thông thường.

### InvoiceRepository::invoiceToString()

#### A. Tổng quan

- **Mục đích:** Chuyển đối tượng Model thành một dòng TXT.
- **Tham số:** `Invoice* invoice`.
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
string InvoiceRepository::invoiceToString(Invoice* invoice)
{
    if (!isValidInvoice(invoice))
    {
        throw runtime_error("Invoice khong hop le khi ghi file");
    }

    return invoice->getInvoiceId() + "|" +
           invoice->getShowtimeId() + "|" +
           invoice->getStaffId() + "|" +
           invoice->getMemberId() + "|" +
           invoice->getCreatedAt() + "|" +
           to_string(invoice->getTotalAmount());
```

Điều kiện hợp lệ ngăn dữ liệu sai định dạng hoặc ngoài giới hạn đi vào danh sách đang quản lý. `throw` báo lỗi bằng ngoại lệ cho bên gọi, khác với kết quả `false` thông thường.

**Bước 2 — Chuẩn bị giá trị cho bước kế tiếp.**

```cpp
}
```

Đoạn này chuẩn bị hoặc trả giá trị theo code. Nó chỉ đổi RAM hoặc TXT nếu có lời gọi cập nhật hay ghi dữ liệu tương ứng.

### InvoiceRepository::isValidReference()

#### A. Tổng quan

- **Mục đích:** Kiểm tra các ID tham chiếu còn tồn tại.
- **Tham số:** `const Invoice* invoice`.
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
Tra cứu đối tượng theo khóa (không đạt: false)
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
bool InvoiceRepository::isValidReference(const Invoice* invoice)
{
    if (!isValidInvoice(invoice))
    {
        return false;
    }
    if (!showtimeRepo.isLoaded() ||
        !userRepo.isLoaded() ||
        !memberRepo.isLoaded())
    {
        return false;
```

Trạng thái `loaded` quyết định Repository đã sẵn sàng chưa; nhánh chưa nạp dừng trước khi thao tác dựa trên dữ liệu đó. Điều kiện hợp lệ ngăn dữ liệu sai định dạng hoặc ngoài giới hạn đi vào danh sách đang quản lý. `return false` kết thúc ngay nhánh không đạt; các thao tác cập nhật phía sau không chạy.

**Bước 2 — Tra cứu đối tượng theo khóa.**

```cpp
    }
    Showtime* showtime = showtimeRepo.findById(invoice->getShowtimeId());
    User* seller = userRepo.findById(invoice->getStaffId());
    if (showtime == nullptr || seller == nullptr)
    {
        return false;
    }
    if (seller->getRole() != UserRole::STAFF &&
        seller->getRole() != UserRole::ADMIN)
    {
        return false;
```

Tra User theo ID để kiểm tra nhân viên liên quan có trong dữ liệu đã nạp. `return false` kết thúc ngay nhánh không đạt; các thao tác cập nhật phía sau không chạy.

**Bước 3 — Tra cứu đối tượng theo khóa.**

```cpp
    }
    if (!DateTime::isBefore(invoice->getCreatedAt(),
                            showtime->getStartAt()))
    {
        return false;
    }
    if (!invoice->getMemberId().empty())
    {
        Member* member = memberRepo.findById(invoice->getMemberId());
```

Tra Member theo ID; kết quả rỗng chỉ được chấp nhận nếu nhánh nghiệp vụ cho phép hóa đơn không có thành viên. `return false` kết thúc ngay nhánh không đạt; các thao tác cập nhật phía sau không chạy.

**Bước 4 — Từ chối điều kiện không đạt.**

```cpp
        if (member == nullptr)
        {
            return false;
        }
    }

    return true;
}
```

`return false` kết thúc ngay nhánh không đạt; các thao tác cập nhật phía sau không chạy.

### InvoiceRepository::load()

#### A. Tổng quan

- **Mục đích:** Nạp và kiểm tra dữ liệu từ TXT vào RAM.
- **Tham số:** không có.
- **Giá trị trả về:** `bool`; xem các nhánh `return` bên dưới.
- **Điều kiện quan trọng:** trạng thái nạp, con trỏ rỗng, dữ liệu hợp lệ, ngoại lệ, nhánh trả `false`.

#### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Từ chối điều kiện không đạt (không đạt: false)
    |
    v
Đọc dữ liệu TXT (không đạt: false)
    |
    v
Kiểm tra dữ liệu hợp lệ (lỗi: throw)
    |
    v
Tra cứu đối tượng theo khóa
    |
    v
Nối quan hệ giữa các Model (lỗi: throw)
    |
    v
Xóa dữ liệu nạp dở khi kiểm tra thất bại
    |
    v
Kết thúc
```

#### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1 — Từ chối điều kiện không đạt.**

```cpp
bool InvoiceRepository::load()
{
    if (repo.isLoaded() ||
        !showtimeRepo.isLoaded() ||
        !userRepo.isLoaded() ||
        !memberRepo.isLoaded())
    {
        return false;
```

Trạng thái `loaded` quyết định Repository đã sẵn sàng chưa; nhánh chưa nạp dừng trước khi thao tác dựa trên dữ liệu đó. `return false` kết thúc ngay nhánh không đạt; các thao tác cập nhật phía sau không chạy.

**Bước 2 — Đọc dữ liệu TXT.**

```cpp
    }
    if (!repo.load())
    {
        return false;
    }
    try
    {
        // Giai doan 1: Kiem tra tham chieu
        for (int i = 0; i < repo.count(); i++)
        {
            Invoice* invoice = repo.getAt(i);
```

Lời gọi nạp đọc TXT vào bộ nhớ. Nếu trả `false`, hàm dừng ở nhánh lỗi; dữ liệu chỉ được xem là dùng được sau khi các bước kiểm tra tiếp theo hoàn tất. `try/catch` xử lý ngoại lệ trong phạm vi hiện tại; code dọn các thay đổi trước khi trả lỗi hoặc ném lại, nên thứ tự dọn có ý nghĩa. `return false` kết thúc ngay nhánh không đạt; các thao tác cập nhật phía sau không chạy.

**Bước 3 — Kiểm tra dữ liệu hợp lệ.**

```cpp
            if (!isValidReference(invoice))
            {
                throw runtime_error("Invoice co tham chieu khong hop le");
            }
        }
        // Giai doan 2: Lien ket doi tuong
        for (int i = 0; i < repo.count(); i++)
        {
            Invoice* invoice = repo.getAt(i);
```

Điều kiện hợp lệ ngăn dữ liệu sai định dạng hoặc ngoài giới hạn đi vào danh sách đang quản lý. `throw` báo lỗi bằng ngoại lệ cho bên gọi, khác với kết quả `false` thông thường. Vòng lặp xét từng bản ghi; điều kiện bên trong quyết định tiếp tục, bỏ qua hay dừng khi gặp dữ liệu sai.

**Bước 4 — Tra cứu đối tượng theo khóa.**

```cpp
            Showtime* showtime = showtimeRepo.findById(invoice->getShowtimeId());
            User* seller = userRepo.findById(invoice->getStaffId());
            Member* member = nullptr;

            if (!invoice->getMemberId().empty())
            {
                member = memberRepo.findById(invoice->getMemberId());
            }
```

Tra Member theo ID; kết quả rỗng chỉ được chấp nhận nếu nhánh nghiệp vụ cho phép hóa đơn không có thành viên. Tra User theo ID để kiểm tra nhân viên liên quan có trong dữ liệu đã nạp.

**Bước 5 — Nối quan hệ giữa các Model.**

```cpp
            if (!invoice->linkShowtime(showtime) ||
                !invoice->linkSeller(seller) ||
                !invoice->linkMember(member))
            {
                throw runtime_error("Khong the lien ket Invoice");
            }
        }
    }
```

Liên kết được đổi theo thứ tự trong code; nếu bước sau thất bại, phần đã nối phải được tháo trước khi xóa đối tượng để tránh con trỏ treo. `throw` báo lỗi bằng ngoại lệ cho bên gọi, khác với kết quả `false` thông thường.

**Bước 6 — Xóa dữ liệu nạp dở khi kiểm tra thất bại.**

```cpp
    catch (...)
    {
        unlinkAll();
        repo.clear();
        throw;
    }
    return true;
}
```

`repo.clear()` giải phóng các đối tượng vừa nạp và đặt lại trạng thái; vì dữ liệu chưa qua hết kiểm tra, không được để chúng tiếp tục xuất hiện như bản ghi hợp lệ. Liên kết được đổi theo thứ tự trong code; nếu bước sau thất bại, phần đã nối phải được tháo trước khi xóa đối tượng để tránh con trỏ treo. `try/catch` xử lý ngoại lệ trong phạm vi hiện tại; code dọn các thay đổi trước khi trả lỗi hoặc ném lại, nên thứ tự dọn có ý nghĩa.

### InvoiceRepository::save()

#### A. Tổng quan

- **Mục đích:** Ghi trạng thái hiện tại ra TXT.
- **Tham số:** không có.
- **Giá trị trả về:** `bool`; xem các nhánh `return` bên dưới.
- **Điều kiện quan trọng:** nhánh trả `false`.

#### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Ghi dữ liệu TXT (không đạt: false)
    |
    v
Chuẩn bị giá trị cho bước kế tiếp
    |
    v
Kết thúc
```

#### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1 — Ghi dữ liệu TXT.**

```cpp
bool InvoiceRepository::save()
{
    if (!repo.save())
    {
        return false;
    }
    pendingIds.clear();
    return true;
```

Lời gọi ghi chuyển trạng thái hiện tại ra TXT. Kết quả ghi quyết định giá trị trả về; một lời gọi này không bảo đảm các file Repository khác cùng được cập nhật. `return false` kết thúc ngay nhánh không đạt; các thao tác cập nhật phía sau không chạy.

**Bước 2 — Chuẩn bị giá trị cho bước kế tiếp.**

```cpp
}
```

Đoạn này chuẩn bị hoặc trả giá trị theo code. Nó chỉ đổi RAM hoặc TXT nếu có lời gọi cập nhật hay ghi dữ liệu tương ứng.

### InvoiceRepository::add()

#### A. Tổng quan

- **Mục đích:** Thêm đối tượng hợp lệ vào Repository.
- **Tham số:** `Invoice* invoice`.
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
Tra cứu đối tượng theo khóa
    |
    v
Tháo liên kết đối tượng
    |
    v
Tháo liên kết đối tượng (không đạt: false)
    |
    v
Bắt lỗi và khôi phục trạng thái (không đạt: false)
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
bool InvoiceRepository::add(Invoice* invoice)
{
    if (!repo.isLoaded() || !isValidReference(invoice))
    {
        return false;
    }

    if (repo.findById(invoice->getInvoiceId()) != nullptr)
    {
        return false;
```

Trạng thái `loaded` quyết định Repository đã sẵn sàng chưa; nhánh chưa nạp dừng trước khi thao tác dựa trên dữ liệu đó. Điều kiện hợp lệ ngăn dữ liệu sai định dạng hoặc ngoài giới hạn đi vào danh sách đang quản lý. `return false` kết thúc ngay nhánh không đạt; các thao tác cập nhật phía sau không chạy.

**Bước 2 — Duyệt và đối chiếu từng phần tử.**

```cpp
    }
    // không thêm trùng
    for (int i = 0; i < repo.count(); i++)
    {
        if (repo.getAt(i) == invoice)
        {
            return false;
        }
```

`return false` kết thúc ngay nhánh không đạt; các thao tác cập nhật phía sau không chạy. Vòng lặp xét từng bản ghi; điều kiện bên trong quyết định tiếp tục, bỏ qua hay dừng khi gặp dữ liệu sai.

**Bước 3 — Từ chối điều kiện không đạt.**

```cpp
    }
    // Khong them Invoice da duoc lien ket
    if (invoice->getShowtime() != nullptr ||
        invoice->getSeller() != nullptr ||
        invoice->getMember() != nullptr)
    {
        return false;
    }
```

`return false` kết thúc ngay nhánh không đạt; các thao tác cập nhật phía sau không chạy.

**Bước 4 — Tra cứu đối tượng theo khóa.**

```cpp
    Showtime* showtime = showtimeRepo.findById(invoice->getShowtimeId());
    User* seller = userRepo.findById(invoice->getStaffId());
    Member* member = nullptr;

    if (!invoice->getMemberId().empty())
    {
        member = memberRepo.findById(invoice->getMemberId());
    }
```

Tra Member theo ID; kết quả rỗng chỉ được chấp nhận nếu nhánh nghiệp vụ cho phép hóa đơn không có thành viên. Tra User theo ID để kiểm tra nhân viên liên quan có trong dữ liệu đã nạp.

**Bước 5 — Tháo liên kết đối tượng.**

```cpp
    pendingIds.push_back(invoice->getInvoiceId());
    try
    {
        if (!invoice->linkShowtime(showtime) ||
            !invoice->linkSeller(seller) ||
            !invoice->linkMember(member))
        {
            invoice->unlinkReferences();
```

Liên kết được đổi theo thứ tự trong code; nếu bước sau thất bại, phần đã nối phải được tháo trước khi xóa đối tượng để tránh con trỏ treo. `try/catch` xử lý ngoại lệ trong phạm vi hiện tại; code dọn các thay đổi trước khi trả lỗi hoặc ném lại, nên thứ tự dọn có ý nghĩa.

**Bước 6 — Tháo liên kết đối tượng.**

```cpp
            pendingIds.pop_back();
            return false;
        }

        if (!repo.add(invoice))
        {
            invoice->unlinkReferences();
            pendingIds.pop_back();
```

Liên kết được đổi theo thứ tự trong code; nếu bước sau thất bại, phần đã nối phải được tháo trước khi xóa đối tượng để tránh con trỏ treo. `return false` kết thúc ngay nhánh không đạt; các thao tác cập nhật phía sau không chạy.

**Bước 7 — Bắt lỗi và khôi phục trạng thái.**

```cpp
            return false;
        }
    }
    catch (...)
    {
        invoice->unlinkReferences();
        pendingIds.pop_back();
        throw;
```

Liên kết được đổi theo thứ tự trong code; nếu bước sau thất bại, phần đã nối phải được tháo trước khi xóa đối tượng để tránh con trỏ treo. `try/catch` xử lý ngoại lệ trong phạm vi hiện tại; code dọn các thay đổi trước khi trả lỗi hoặc ném lại, nên thứ tự dọn có ý nghĩa. `return false` kết thúc ngay nhánh không đạt; các thao tác cập nhật phía sau không chạy.

**Bước 8 — Xác nhận xử lý thành công.**

```cpp
    }

    return true;
}
```

Đoạn này chuẩn bị hoặc trả giá trị theo code. Nó chỉ đổi RAM hoặc TXT nếu có lời gọi cập nhật hay ghi dữ liệu tương ứng.

### InvoiceRepository::rollbackPending()

#### A. Tổng quan

- **Mục đích:** Hoàn tác đối tượng mới tạo còn ở trạng thái chờ.
- **Tham số:** `const string& invoiceId`.
- **Giá trị trả về:** `bool`; xem các nhánh `return` bên dưới.
- **Điều kiện quan trọng:** con trỏ rỗng, nhánh trả `false`.

#### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Kiểm tra đối tượng chờ hoàn tác (không đạt: false)
    |
    v
Tháo liên kết đối tượng (không đạt: false)
    |
    v
Chuẩn bị giá trị cho bước kế tiếp
    |
    v
Kết thúc
```

#### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1 — Kiểm tra đối tượng chờ hoàn tác.**

```cpp
bool InvoiceRepository::rollbackPending(const string& invoiceId)
{
    if (pendingIds.empty() || pendingIds.back() != invoiceId)
    {
        return false;
    }
    Invoice* invoice = repo.findById(invoiceId);
    if (invoice == nullptr || invoice->getTicketCount() != 0)
    {
        return false;
```

`return false` kết thúc ngay nhánh không đạt; các thao tác cập nhật phía sau không chạy.

**Bước 2 — Tháo liên kết đối tượng.**

```cpp
    }
    invoice->unlinkReferences();
    if (!repo.removeById(invoiceId))
    {
        return false;
    }
    pendingIds.pop_back();
    return true;
```

Liên kết được đổi theo thứ tự trong code; nếu bước sau thất bại, phần đã nối phải được tháo trước khi xóa đối tượng để tránh con trỏ treo. `return false` kết thúc ngay nhánh không đạt; các thao tác cập nhật phía sau không chạy.

**Bước 3 — Chuẩn bị giá trị cho bước kế tiếp.**

```cpp
}
```

Đoạn này chuẩn bị hoặc trả giá trị theo code. Nó chỉ đổi RAM hoặc TXT nếu có lời gọi cập nhật hay ghi dữ liệu tương ứng.

### InvoiceRepository::findById()

#### A. Tổng quan

- **Mục đích:** Tra đối tượng theo ID.
- **Tham số:** `const string& invoiceId`.
- **Giá trị trả về:** `Invoice*`; xem các nhánh `return` bên dưới.
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
Invoice* InvoiceRepository::findById(const string& invoiceId)
{
    return repo.findById(invoiceId);
}
```

Đoạn này chuẩn bị hoặc trả giá trị theo code. Nó chỉ đổi RAM hoặc TXT nếu có lời gọi cập nhật hay ghi dữ liệu tương ứng.

### InvoiceRepository::getAt()

#### A. Tổng quan

- **Mục đích:** Lấy phần tử theo chỉ số.
- **Tham số:** `int index`.
- **Giá trị trả về:** `Invoice*`; xem các nhánh `return` bên dưới.
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
Invoice* InvoiceRepository::getAt(int index)
{
    return repo.getAt(index);
}
```

Đoạn này chuẩn bị hoặc trả giá trị theo code. Nó chỉ đổi RAM hoặc TXT nếu có lời gọi cập nhật hay ghi dữ liệu tương ứng.

### InvoiceRepository::count()

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
int InvoiceRepository::count()
{
    return repo.count();
}
```

Đoạn này chuẩn bị hoặc trả giá trị theo code. Nó chỉ đổi RAM hoặc TXT nếu có lời gọi cập nhật hay ghi dữ liệu tương ứng.

### InvoiceRepository::isLoaded()

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
bool InvoiceRepository::isLoaded()
{
    return repo.isLoaded();
}
```

Trạng thái `loaded` quyết định Repository đã sẵn sàng chưa; nhánh chưa nạp dừng trước khi thao tác dựa trên dữ liệu đó.

### InvoiceRepository::getByShowtime()

#### A. Tổng quan

- **Mục đích:** Lọc theo suất chiếu.
- **Tham số:** `const string& showtimeId`.
- **Giá trị trả về:** `vector<Invoice*>`; xem các nhánh `return` bên dưới.
- **Điều kiện quan trọng:** trạng thái nạp.

#### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Trả kết quả cho bên gọi
    |
    v
Duyệt và đối chiếu từng phần tử
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
vector<Invoice*> InvoiceRepository::getByShowtime(const string& showtimeId)
{
    vector<Invoice*> result;

    if (!repo.isLoaded())
    {
        return result;
    }
```

Trạng thái `loaded` quyết định Repository đã sẵn sàng chưa; nhánh chưa nạp dừng trước khi thao tác dựa trên dữ liệu đó.

**Bước 2 — Duyệt và đối chiếu từng phần tử.**

```cpp

    for (int i = 0; i < repo.count(); i++)
    {
        Invoice* invoice = repo.getAt(i);

        if (invoice->getShowtimeId() == showtimeId)
        {
            result.push_back(invoice);
```

Vòng lặp xét từng bản ghi; điều kiện bên trong quyết định tiếp tục, bỏ qua hay dừng khi gặp dữ liệu sai.

**Bước 3 — Trả kết quả cho bên gọi.**

```cpp
        }
    }

    return result;
}
```

Đoạn này chuẩn bị hoặc trả giá trị theo code. Nó chỉ đổi RAM hoặc TXT nếu có lời gọi cập nhật hay ghi dữ liệu tương ứng.

### InvoiceRepository::getByStaff()

#### A. Tổng quan

- **Mục đích:** Lọc theo nhân viên.
- **Tham số:** `const string& staffId`.
- **Giá trị trả về:** `vector<Invoice*>`; xem các nhánh `return` bên dưới.
- **Điều kiện quan trọng:** trạng thái nạp.

#### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Trả kết quả cho bên gọi
    |
    v
Duyệt và đối chiếu từng phần tử
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
vector<Invoice*> InvoiceRepository::getByStaff(const string& staffId)
{
    vector<Invoice*> result;

    if (!repo.isLoaded())
    {
        return result;
    }
```

Trạng thái `loaded` quyết định Repository đã sẵn sàng chưa; nhánh chưa nạp dừng trước khi thao tác dựa trên dữ liệu đó.

**Bước 2 — Duyệt và đối chiếu từng phần tử.**

```cpp

    for (int i = 0; i < repo.count(); i++)
    {
        Invoice* invoice = repo.getAt(i);

        if (invoice->getStaffId() == staffId)
        {
            result.push_back(invoice);
```

Vòng lặp xét từng bản ghi; điều kiện bên trong quyết định tiếp tục, bỏ qua hay dừng khi gặp dữ liệu sai.

**Bước 3 — Trả kết quả cho bên gọi.**

```cpp
        }
    }

    return result;
}
```

Đoạn này chuẩn bị hoặc trả giá trị theo code. Nó chỉ đổi RAM hoặc TXT nếu có lời gọi cập nhật hay ghi dữ liệu tương ứng.

### InvoiceRepository::getByMember()

#### A. Tổng quan

- **Mục đích:** Lọc theo thành viên.
- **Tham số:** `const string& memberId`.
- **Giá trị trả về:** `vector<Invoice*>`; xem các nhánh `return` bên dưới.
- **Điều kiện quan trọng:** trạng thái nạp.

#### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Trả kết quả cho bên gọi
    |
    v
Duyệt và đối chiếu từng phần tử
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
vector<Invoice*> InvoiceRepository::getByMember(const string& memberId)
{
    vector<Invoice*> result;

    if (!repo.isLoaded() || memberId.empty())
    {
        return result;
    }
```

Trạng thái `loaded` quyết định Repository đã sẵn sàng chưa; nhánh chưa nạp dừng trước khi thao tác dựa trên dữ liệu đó.

**Bước 2 — Duyệt và đối chiếu từng phần tử.**

```cpp

    for (int i = 0; i < repo.count(); i++)
    {
        Invoice* invoice = repo.getAt(i);

        if (invoice->getMemberId() == memberId)
        {
            result.push_back(invoice);
```

Vòng lặp xét từng bản ghi; điều kiện bên trong quyết định tiếp tục, bỏ qua hay dừng khi gặp dữ liệu sai.

**Bước 3 — Trả kết quả cho bên gọi.**

```cpp
        }
    }

    return result;
}
```

Đoạn này chuẩn bị hoặc trả giá trị theo code. Nó chỉ đổi RAM hoặc TXT nếu có lời gọi cập nhật hay ghi dữ liệu tương ứng.


