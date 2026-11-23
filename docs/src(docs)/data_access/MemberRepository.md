# MemberRepository — mã nguồn và quy tắc dữ liệu

## 1. Tổng quan

`MemberRepository` chứa `Repository<Member>` sở hữu các `Member*` và hai `unordered_map` tra theo mã, số điện thoại. Bản đồ chỉ giữ con trỏ mượn; khi nạp/cập nhật phải đồng bộ danh sách và chỉ mục. File dữ liệu là `members.txt` theo đường dẫn constructor.

Repository kiểm tra ID, tên, số điện thoại và điểm hợp lệ; quyền người dùng và tích/thu hồi điểm khi bán/hủy vé thuộc Business. Hàm `isValidPhone()` là helper `static` ở phạm vi file; nó được tài liệu hóa cùng các hàm lớp vì tham gia trực tiếp vào kiểm tra dữ liệu.

Mã nguồn: `src/data_access/MemberRepository.cpp` và `src/data_access/MemberRepository.h`. Các đoạn code dưới đây giữ nguyên nội dung hàm, gồm dòng trống và comment trong hàm. Khi ghép các đoạn của một hàm theo thứ tự, ta thu được đúng định nghĩa gốc.

## 2. Chi tiết từng hàm

### isValidPhone() — helper phạm vi file

#### A. Tổng quan

- **Mục đích:** Kiểm tra số điện thoại đúng định dạng.
- **Tham số:** `const string& phone`.
- **Giá trị trả về:** `bool`; xem các nhánh `return` bên dưới.
- **Điều kiện quan trọng:** dữ liệu hợp lệ, nhánh trả `false`.

#### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Kiểm tra dữ liệu hợp lệ (không đạt: false)
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
static bool isValidPhone(const string& phone)
{
    if (phone.size() != 10 || phone[0] != '0')
    {
        return false;
    }
    for (char c : phone)
    {
        if (c < '0' || c > '9')
        {
            return false;
```

Điều kiện hợp lệ ngăn dữ liệu sai định dạng hoặc ngoài giới hạn đi vào danh sách đang quản lý. `return false` kết thúc ngay nhánh không đạt; các thao tác cập nhật phía sau không chạy. Vòng lặp xét từng bản ghi; điều kiện bên trong quyết định tiếp tục, bỏ qua hay dừng khi gặp dữ liệu sai.

**Bước 2 — Xác nhận xử lý thành công.**

```cpp
        }
    }
    return true;
}
```

Đoạn này chuẩn bị hoặc trả giá trị theo code. Nó chỉ đổi RAM hoặc TXT nếu có lời gọi cập nhật hay ghi dữ liệu tương ứng.

### MemberRepository::isValidMember()

#### A. Tổng quan

- **Mục đích:** Kiểm tra tính hợp lệ của dữ liệu đầu vào.
- **Tham số:** `const Member* member`.
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
bool MemberRepository::isValidMember(const Member* member)
{
    if (member == nullptr)
    {
        return false;
    }
    if (!Validator::isValidId(member->getMemberId(), "MEM"))
    {
        return false;
```

Điều kiện hợp lệ ngăn dữ liệu sai định dạng hoặc ngoài giới hạn đi vào danh sách đang quản lý. `return false` kết thúc ngay nhánh không đạt; các thao tác cập nhật phía sau không chạy.

**Bước 2 — Kiểm tra dữ liệu hợp lệ.**

```cpp
    }
    if (!Validator::isValidText(member->getFullName()))
    {
        return false;
    }
    if (!isValidPhone(member->getPhone()))
    {
        return false;
```

Điều kiện hợp lệ ngăn dữ liệu sai định dạng hoặc ngoài giới hạn đi vào danh sách đang quản lý. `return false` kết thúc ngay nhánh không đạt; các thao tác cập nhật phía sau không chạy.

**Bước 3 — Từ chối điều kiện không đạt.**

```cpp
    }
    if (member->getPoints() < 0)
    {
        return false;
    }
    return true;
}
```

`return false` kết thúc ngay nhánh không đạt; các thao tác cập nhật phía sau không chạy.

### MemberRepository::MemberRepository()

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
MemberRepository::MemberRepository(const string& filePath)
    : repo(filePath, getMemberId, stringToMember, memberToString)
{
}
```

Danh sách sau dấu `:` khởi tạo thành viên trước thân constructor; tham chiếu được giữ để dùng về sau, còn thân hàm không nạp file.

### MemberRepository::getMemberId()

#### A. Tổng quan

- **Mục đích:** Lấy ID của đối tượng làm khóa.
- **Tham số:** `Member* member`.
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
string MemberRepository::getMemberId(Member* member)
{
    if (member == nullptr)
    {
        return "";
    }
    return member->getMemberId();
}
```

Đoạn này chuẩn bị hoặc trả giá trị theo code. Nó chỉ đổi RAM hoặc TXT nếu có lời gọi cập nhật hay ghi dữ liệu tương ứng.

### MemberRepository::stringToMember()

#### A. Tổng quan

- **Mục đích:** Chuyển một dòng TXT thành đối tượng Model.
- **Tham số:** `const string& line`.
- **Giá trị trả về:** `Member*`; xem các nhánh `return` bên dưới.
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
Duyệt và đối chiếu từng phần tử (lỗi: throw)
    |
    v
Chuẩn bị giá trị cho bước kế tiếp
    |
    v
Bắt lỗi và khôi phục trạng thái (lỗi: throw)
    |
    v
Kiểm tra dữ liệu hợp lệ
    |
    v
Trả kết quả cho bên gọi (lỗi: throw)
    |
    v
Kết thúc
```

#### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1 — Duyệt và đối chiếu từng phần tử.**

```cpp
Member* MemberRepository::stringToMember(const string& line)
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
    if (fields.size() != 4)
    {
        throw runtime_error("Sai so luong truong Member");
```

`throw` báo lỗi bằng ngoại lệ cho bên gọi, khác với kết quả `false` thông thường.

**Bước 3 — Duyệt và đối chiếu từng phần tử.**

```cpp
    }
    if (fields[3].empty())
    {
        throw runtime_error("Points Member khong hop le");
    }
    // kiểm tra points chỉ gồm chữ số
    for (char c : fields[3])
    {
        if (c < '0' || c > '9')
        {
            throw runtime_error("Points Member khong hop le");
```

`throw` báo lỗi bằng ngoại lệ cho bên gọi, khác với kết quả `false` thông thường. Vòng lặp xét từng bản ghi; điều kiện bên trong quyết định tiếp tục, bỏ qua hay dừng khi gặp dữ liệu sai.

**Bước 4 — Chuẩn bị giá trị cho bước kế tiếp.**

```cpp
        }
    }
    int points;
    //chuyển chuỗi points thành số nguyên
    try
    {
        size_t pos = 0;
        points = stoi(fields[3], &pos);
```

`try/catch` xử lý ngoại lệ trong phạm vi hiện tại; code dọn các thay đổi trước khi trả lỗi hoặc ném lại, nên thứ tự dọn có ý nghĩa. Phép parse đổi chuỗi TXT sang trường có kiểu; dữ liệu không chuyển được phải bị từ chối trước khi tạo đối tượng hợp lệ.

**Bước 5 — Bắt lỗi và khôi phục trạng thái.**

```cpp
        if (pos != fields[3].size())
        {
            throw runtime_error("Points Member khong hop le");
        }
    }
    catch (const exception&)
    {
        throw runtime_error("Points Member khong hop le");
```

`try/catch` xử lý ngoại lệ trong phạm vi hiện tại; code dọn các thay đổi trước khi trả lỗi hoặc ném lại, nên thứ tự dọn có ý nghĩa. `throw` báo lỗi bằng ngoại lệ cho bên gọi, khác với kết quả `false` thông thường.

**Bước 6 — Kiểm tra dữ liệu hợp lệ.**

```cpp
    }
    Member* member = new Member(
        fields[0],
        fields[1],
        fields[2],
        points
    );
    if (!isValidMember(member))
    {
        delete member;
```

Điều kiện hợp lệ ngăn dữ liệu sai định dạng hoặc ngoài giới hạn đi vào danh sách đang quản lý. `new` cấp phát Model trên heap. Cho đến khi Repository nhận thành công, nhánh lỗi hoặc caller vẫn phải chịu trách nhiệm giải phóng. `delete` giải phóng đối tượng; không được dùng lại con trỏ sau lệnh này.

**Bước 7 — Trả kết quả cho bên gọi.**

```cpp
        throw runtime_error("Du lieu Member khong hop le");
    }
    return member;
}
```

`throw` báo lỗi bằng ngoại lệ cho bên gọi, khác với kết quả `false` thông thường.

### MemberRepository::memberToString()

#### A. Tổng quan

- **Mục đích:** Chuyển đối tượng Model thành một dòng TXT.
- **Tham số:** `Member* member`.
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
string MemberRepository::memberToString(Member* member)
{
    if (!isValidMember(member))
    {
        throw runtime_error("Member khong hop le khi ghi file");
    }
    return member->getMemberId() + "|" +
           member->getFullName() + "|" +
           member->getPhone() + "|" +
           to_string(member->getPoints());
```

Điều kiện hợp lệ ngăn dữ liệu sai định dạng hoặc ngoài giới hạn đi vào danh sách đang quản lý. `throw` báo lỗi bằng ngoại lệ cho bên gọi, khác với kết quả `false` thông thường.

**Bước 2 — Chuẩn bị giá trị cho bước kế tiếp.**

```cpp
}
```

Đoạn này chuẩn bị hoặc trả giá trị theo code. Nó chỉ đổi RAM hoặc TXT nếu có lời gọi cập nhật hay ghi dữ liệu tương ứng.

### MemberRepository::load()

#### A. Tổng quan

- **Mục đích:** Nạp và kiểm tra dữ liệu từ TXT vào RAM.
- **Tham số:** không có.
- **Giá trị trả về:** `bool`; xem các nhánh `return` bên dưới.
- **Điều kiện quan trọng:** trạng thái nạp, ngoại lệ.

#### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Đọc dữ liệu TXT (lỗi: throw)
    |
    v
Duyệt và đối chiếu từng phần tử
    |
    v
Kiểm tra điều kiện trước khi tiếp tục (lỗi: throw)
    |
    v
Bắt lỗi và khôi phục trạng thái
    |
    v
Xóa dữ liệu nạp dở khi kiểm tra thất bại
    |
    v
Kết thúc
```

#### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1 — Đọc dữ liệu TXT.**

```cpp
bool MemberRepository::load()
{
    if (repo.isLoaded())
    {
        throw runtime_error("Du lieu Member da duoc nap: " + repo.getFilePath());
    }
    if (!repo.load())
    {
        throw runtime_error("Khong the nap file Member: " + repo.getFilePath());
```

Lời gọi nạp đọc TXT vào bộ nhớ. Nếu trả `false`, hàm dừng ở nhánh lỗi; dữ liệu chỉ được xem là dùng được sau khi các bước kiểm tra tiếp theo hoàn tất. Trạng thái `loaded` quyết định Repository đã sẵn sàng chưa; nhánh chưa nạp dừng trước khi thao tác dựa trên dữ liệu đó. `throw` báo lỗi bằng ngoại lệ cho bên gọi, khác với kết quả `false` thông thường.

**Bước 2 — Duyệt và đối chiếu từng phần tử.**

```cpp
    }
    // xây dựng 2 map
    membersById.clear();
    membersByPhone.clear();
    try
    {
        for (int i = 0; i < repo.count(); i++)
        {
            Member* member = repo.getAt(i);
```

`try/catch` xử lý ngoại lệ trong phạm vi hiện tại; code dọn các thay đổi trước khi trả lỗi hoặc ném lại, nên thứ tự dọn có ý nghĩa. Vòng lặp xét từng bản ghi; điều kiện bên trong quyết định tiếp tục, bỏ qua hay dừng khi gặp dữ liệu sai.

**Bước 3 — Kiểm tra điều kiện trước khi tiếp tục.**

```cpp
            if (membersById.find(member->getMemberId()) != membersById.end() ||
                membersByPhone.find(member->getPhone()) != membersByPhone.end())
            {
                throw runtime_error(
                    "Trung ID hoac phone trong file " +
                    repo.getFilePath() +
                    " tai dong " + to_string(i + 1)
                );
            }
```

`throw` báo lỗi bằng ngoại lệ cho bên gọi, khác với kết quả `false` thông thường.

**Bước 4 — Bắt lỗi và khôi phục trạng thái.**

```cpp
            membersById[member->getMemberId()] = member;
            membersByPhone[member->getPhone()] = member;
        }
    }
    catch (...)
    {
        membersById.clear();
        membersByPhone.clear();
```

`try/catch` xử lý ngoại lệ trong phạm vi hiện tại; code dọn các thay đổi trước khi trả lỗi hoặc ném lại, nên thứ tự dọn có ý nghĩa.

**Bước 5 — Xóa dữ liệu nạp dở khi kiểm tra thất bại.**

```cpp
        repo.clear();
        throw;
    }
    return true;
}
```

`repo.clear()` giải phóng các đối tượng vừa nạp và đặt lại trạng thái; vì dữ liệu chưa qua hết kiểm tra, không được để chúng tiếp tục xuất hiện như bản ghi hợp lệ.

### MemberRepository::save()

#### A. Tổng quan

- **Mục đích:** Ghi trạng thái hiện tại ra TXT.
- **Tham số:** không có.
- **Giá trị trả về:** `bool`; xem các nhánh `return` bên dưới.
- **Điều kiện quan trọng:** trạng thái nạp, ngoại lệ.

#### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Ghi dữ liệu TXT (lỗi: throw)
    |
    v
Xác nhận xử lý thành công
    |
    v
Kết thúc
```

#### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1 — Ghi dữ liệu TXT.**

```cpp
bool MemberRepository::save()
{
    if (!repo.isLoaded())
    {
        throw runtime_error("Chua nap du lieu Member");
    }
    if (!repo.save())
    {
        throw runtime_error("Khong the ghi file Member: " + repo.getFilePath());
```

Lời gọi ghi chuyển trạng thái hiện tại ra TXT. Kết quả ghi quyết định giá trị trả về; một lời gọi này không bảo đảm các file Repository khác cùng được cập nhật. Trạng thái `loaded` quyết định Repository đã sẵn sàng chưa; nhánh chưa nạp dừng trước khi thao tác dựa trên dữ liệu đó. `throw` báo lỗi bằng ngoại lệ cho bên gọi, khác với kết quả `false` thông thường.

**Bước 2 — Xác nhận xử lý thành công.**

```cpp
    }
    return true;
}
```

Đoạn này chuẩn bị hoặc trả giá trị theo code. Nó chỉ đổi RAM hoặc TXT nếu có lời gọi cập nhật hay ghi dữ liệu tương ứng.

### MemberRepository::findById()

#### A. Tổng quan

- **Mục đích:** Tra đối tượng theo ID.
- **Tham số:** `const string& memberId`.
- **Giá trị trả về:** `Member*`; xem các nhánh `return` bên dưới.
- **Điều kiện quan trọng:** con trỏ rỗng.

#### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Tra cứu đối tượng theo khóa
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
Member* MemberRepository::findById(const string& memberId)
{
    auto it = membersById.find(memberId);
    if (it == membersById.end())
    {
        return nullptr;
    }
    return it->second;
```

Đoạn này chuẩn bị hoặc trả giá trị theo code. Nó chỉ đổi RAM hoặc TXT nếu có lời gọi cập nhật hay ghi dữ liệu tương ứng.

**Bước 2 — Chuẩn bị giá trị cho bước kế tiếp.**

```cpp
}
```

Đoạn này chuẩn bị hoặc trả giá trị theo code. Nó chỉ đổi RAM hoặc TXT nếu có lời gọi cập nhật hay ghi dữ liệu tương ứng.

### MemberRepository::findByPhone()

#### A. Tổng quan

- **Mục đích:** Tra thành viên theo số điện thoại.
- **Tham số:** `const string& phone`.
- **Giá trị trả về:** `Member*`; xem các nhánh `return` bên dưới.
- **Điều kiện quan trọng:** con trỏ rỗng.

#### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Tra cứu đối tượng theo khóa
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
Member* MemberRepository::findByPhone(const string& phone)
{
    auto it = membersByPhone.find(phone);
    if (it == membersByPhone.end())
    {
        return nullptr;
    }
    return it->second;
```

Đoạn này chuẩn bị hoặc trả giá trị theo code. Nó chỉ đổi RAM hoặc TXT nếu có lời gọi cập nhật hay ghi dữ liệu tương ứng.

**Bước 2 — Chuẩn bị giá trị cho bước kế tiếp.**

```cpp
}
```

Đoạn này chuẩn bị hoặc trả giá trị theo code. Nó chỉ đổi RAM hoặc TXT nếu có lời gọi cập nhật hay ghi dữ liệu tương ứng.

### MemberRepository::getAt()

#### A. Tổng quan

- **Mục đích:** Lấy phần tử theo chỉ số.
- **Tham số:** `int index`.
- **Giá trị trả về:** `Member*`; xem các nhánh `return` bên dưới.
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
Member* MemberRepository::getAt(int index)
{
    return repo.getAt(index);
}
```

Đoạn này chuẩn bị hoặc trả giá trị theo code. Nó chỉ đổi RAM hoặc TXT nếu có lời gọi cập nhật hay ghi dữ liệu tương ứng.

### MemberRepository::count()

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
int MemberRepository::count()
{
    return repo.count();
}
```

Đoạn này chuẩn bị hoặc trả giá trị theo code. Nó chỉ đổi RAM hoặc TXT nếu có lời gọi cập nhật hay ghi dữ liệu tương ứng.

### MemberRepository::isLoaded()

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
bool MemberRepository::isLoaded()
{
    return repo.isLoaded();
}
```

Trạng thái `loaded` quyết định Repository đã sẵn sàng chưa; nhánh chưa nạp dừng trước khi thao tác dựa trên dữ liệu đó.

### MemberRepository::add()

#### A. Tổng quan

- **Mục đích:** Thêm đối tượng hợp lệ vào Repository.
- **Tham số:** `Member* member`.
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
Duyệt và đối chiếu từng phần tử (không đạt: false)
    |
    v
Chuẩn bị giá trị cho bước kế tiếp
    |
    v
Bắt lỗi và khôi phục trạng thái
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

**Bước 1 — Kiểm tra dữ liệu hợp lệ.**

```cpp
bool MemberRepository::add(Member* member)
{
    if (!repo.isLoaded() || !isValidMember(member))
    {
        return false;
    }
    if (member->getPoints() != 0)
    {
        return false;
```

Trạng thái `loaded` quyết định Repository đã sẵn sàng chưa; nhánh chưa nạp dừng trước khi thao tác dựa trên dữ liệu đó. Điều kiện hợp lệ ngăn dữ liệu sai định dạng hoặc ngoài giới hạn đi vào danh sách đang quản lý. `return false` kết thúc ngay nhánh không đạt; các thao tác cập nhật phía sau không chạy.

**Bước 2 — Tra cứu đối tượng theo khóa.**

```cpp
    }
    if (findById(member->getMemberId()) != nullptr)
    {
        return false;
    }
    if (findByPhone(member->getPhone()) != nullptr)
    {
        return false;
```

`return false` kết thúc ngay nhánh không đạt; các thao tác cập nhật phía sau không chạy.

**Bước 3 — Duyệt và đối chiếu từng phần tử.**

```cpp
    }
    // kiểm tra nếu có thành viên rồi thì không thêm nữa
    for (int i = 0; i < repo.count(); i++)
    {
        if (repo.getAt(i) == member)
        {
            return false;
        }
```

`return false` kết thúc ngay nhánh không đạt; các thao tác cập nhật phía sau không chạy. Vòng lặp xét từng bản ghi; điều kiện bên trong quyết định tiếp tục, bỏ qua hay dừng khi gặp dữ liệu sai.

**Bước 4 — Chuẩn bị giá trị cho bước kế tiếp.**

```cpp
    }
    string memberId = member->getMemberId();
    string phone = member->getPhone();
    // Thêm vào 2 map
    membersById[memberId] = member;
    try
    {
        membersByPhone[phone] = member;
```

`try/catch` xử lý ngoại lệ trong phạm vi hiện tại; code dọn các thay đổi trước khi trả lỗi hoặc ném lại, nên thứ tự dọn có ý nghĩa.

**Bước 5 — Bắt lỗi và khôi phục trạng thái.**

```cpp
    }
    catch (...)
    {
        membersById.erase(memberId);
        throw;
    }

    try
    {
        if (!repo.add(member))
        {
            membersById.erase(memberId);
```

`try/catch` xử lý ngoại lệ trong phạm vi hiện tại; code dọn các thay đổi trước khi trả lỗi hoặc ném lại, nên thứ tự dọn có ý nghĩa.

**Bước 6 — Bắt lỗi và khôi phục trạng thái.**

```cpp
            membersByPhone.erase(phone);
            return false;
        }
    }
    catch (...)
    {
        membersById.erase(memberId);
        membersByPhone.erase(phone);
```

`try/catch` xử lý ngoại lệ trong phạm vi hiện tại; code dọn các thay đổi trước khi trả lỗi hoặc ném lại, nên thứ tự dọn có ý nghĩa. `return false` kết thúc ngay nhánh không đạt; các thao tác cập nhật phía sau không chạy.

**Bước 7 — Xác nhận xử lý thành công.**

```cpp
        throw;
    }
    return true;
}
```

Đoạn này chuẩn bị hoặc trả giá trị theo code. Nó chỉ đổi RAM hoặc TXT nếu có lời gọi cập nhật hay ghi dữ liệu tương ứng.

### MemberRepository::update()

#### A. Tổng quan

- **Mục đích:** Cập nhật đối tượng đang quản lý.
- **Tham số:** `const Member& newValues`.
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
Tra cứu đối tượng theo khóa (không đạt: false)
    |
    v
Kiểm tra điều kiện trước khi tiếp tục
    |
    v
Bắt lỗi và khôi phục trạng thái
    |
    v
Kiểm tra điều kiện trước khi tiếp tục
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
bool MemberRepository::update(const Member& newValues)
{
    if (!repo.isLoaded() || !isValidMember(&newValues))
    {
        return false;
    }

    Member* member = findById(newValues.getMemberId());
```

Trạng thái `loaded` quyết định Repository đã sẵn sàng chưa; nhánh chưa nạp dừng trước khi thao tác dựa trên dữ liệu đó. Điều kiện hợp lệ ngăn dữ liệu sai định dạng hoặc ngoài giới hạn đi vào danh sách đang quản lý. `return false` kết thúc ngay nhánh không đạt; các thao tác cập nhật phía sau không chạy.

**Bước 2 — Từ chối điều kiện không đạt.**

```cpp

    if (member == nullptr)
    {
        return false;
    }

    // Khong cho phep thay doi diem truc tiep
    if (member->getPoints() != newValues.getPoints())
    {
        return false;
```

`return false` kết thúc ngay nhánh không đạt; các thao tác cập nhật phía sau không chạy.

**Bước 3 — Tra cứu đối tượng theo khóa.**

```cpp
    }

    // Kiem tra trung so dien thoai
    Member* existingMember = findByPhone(newValues.getPhone());

    if (existingMember != nullptr && existingMember != member)
    {
        return false;
```

`return false` kết thúc ngay nhánh không đạt; các thao tác cập nhật phía sau không chạy.

**Bước 4 — Kiểm tra điều kiện trước khi tiếp tục.**

```cpp
    }

    string oldPhone = member->getPhone();
    string newPhone = newValues.getPhone();

    // Them khoa moi vao map truoc
    if (oldPhone != newPhone)
    {
        membersByPhone[newPhone] = member;
```

Đoạn này chuẩn bị hoặc trả giá trị theo code. Nó chỉ đổi RAM hoặc TXT nếu có lời gọi cập nhật hay ghi dữ liệu tương ứng.

**Bước 5 — Bắt lỗi và khôi phục trạng thái.**

```cpp
    }

    try
    {
        member->setFullName(newValues.getFullName());
        member->setPhone(newPhone);
    }
    catch (...)
    {
        if (oldPhone != newPhone)
        {
            membersByPhone.erase(newPhone);
```

`try/catch` xử lý ngoại lệ trong phạm vi hiện tại; code dọn các thay đổi trước khi trả lỗi hoặc ném lại, nên thứ tự dọn có ý nghĩa.

**Bước 6 — Kiểm tra điều kiện trước khi tiếp tục.**

```cpp
        }
        throw;
    }

    // Xoa khoa cu sau khi cap nhat thanh cong
    if (oldPhone != newPhone)
    {
        membersByPhone.erase(oldPhone);
```

Đoạn này chuẩn bị hoặc trả giá trị theo code. Nó chỉ đổi RAM hoặc TXT nếu có lời gọi cập nhật hay ghi dữ liệu tương ứng.

**Bước 7 — Xác nhận xử lý thành công.**

```cpp
    }

    return true;
}
```

Đoạn này chuẩn bị hoặc trả giá trị theo code. Nó chỉ đổi RAM hoặc TXT nếu có lời gọi cập nhật hay ghi dữ liệu tương ứng.


