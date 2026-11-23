# UserRepository — mã nguồn và quy tắc dữ liệu

## 1. Tổng quan

`UserRepository` chứa `Repository<User>` sở hữu User và hai `unordered_map` tra theo ID/username. Bản đồ chỉ giữ con trỏ mượn; cần cập nhật chỉ mục khi nạp, thêm hoặc đổi username. TXT lưu một tài khoản mỗi dòng theo định dạng được parse trong `stringToUser()`.

Repository kiểm tra ID, văn bản, vai trò, tính duy nhất của username và dữ liệu cập nhật. Đăng nhập, đăng xuất, quyền thao tác thuộc `AuthService`. Khi cập nhật, ID tài khoản giữ nguyên; con trỏ trả về từ tra cứu không được caller giải phóng.

Mã nguồn: `src/data_access/UserRepository.cpp` và `src/data_access/UserRepository.h`. Các đoạn code dưới đây giữ nguyên nội dung hàm, gồm dòng trống và comment trong hàm. Khi ghép các đoạn của một hàm theo thứ tự, ta thu được đúng định nghĩa gốc.

## 2. Chi tiết từng hàm

### UserRepository::isValidUser()

#### A. Tổng quan

- **Mục đích:** Kiểm tra tính hợp lệ của dữ liệu đầu vào.
- **Tham số:** `const User* user`.
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
Xác nhận xử lý thành công
    |
    v
Kết thúc
```

#### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1 — Kiểm tra dữ liệu hợp lệ.**

```cpp
bool UserRepository::isValidUser(const User* user)
{
    if (user == nullptr)
    {
        return false;
    }
    if (!Validator::isValidId(user->getUserId(), "USR"))
    {
        return false;
```

Điều kiện hợp lệ ngăn dữ liệu sai định dạng hoặc ngoài giới hạn đi vào danh sách đang quản lý. `return false` kết thúc ngay nhánh không đạt; các thao tác cập nhật phía sau không chạy.

**Bước 2 — Kiểm tra dữ liệu hợp lệ.**

```cpp
    }
    if (!Validator::isValidText(user->getUsername()) ||
        !Validator::isValidText(user->getPassword()) ||
        !Validator::isValidText(user->getFullName()))
    {
        return false;
    }
    const string& username = user->getUsername();
```

Điều kiện hợp lệ ngăn dữ liệu sai định dạng hoặc ngoài giới hạn đi vào danh sách đang quản lý. `return false` kết thúc ngay nhánh không đạt; các thao tác cập nhật phía sau không chạy.

**Bước 3 — Từ chối điều kiện không đạt.**

```cpp
    if (username.front() == ' ' || username.back() == ' ')
    {
        return false;
    }
    if (user->getRole() != UserRole::ADMIN &&
        user->getRole() != UserRole::STAFF)
    {
        return false;
```

`return false` kết thúc ngay nhánh không đạt; các thao tác cập nhật phía sau không chạy.

**Bước 4 — Xác nhận xử lý thành công.**

```cpp
    }
    return true;
}
```

Đoạn này chuẩn bị hoặc trả giá trị theo code. Nó chỉ đổi RAM hoặc TXT nếu có lời gọi cập nhật hay ghi dữ liệu tương ứng.

### UserRepository::UserRepository()

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
UserRepository::UserRepository(const string& filePath)
    : repo(filePath, getUserId, stringToUser, userToString)
{
}
```

Danh sách sau dấu `:` khởi tạo thành viên trước thân constructor; tham chiếu được giữ để dùng về sau, còn thân hàm không nạp file.

### UserRepository::getUserId()

#### A. Tổng quan

- **Mục đích:** Lấy ID của đối tượng làm khóa.
- **Tham số:** `User* user`.
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
string UserRepository::getUserId(User* user)
{
    if (user == nullptr)
    {
        return "";
    }
    return user->getUserId();
}
```

Đoạn này chuẩn bị hoặc trả giá trị theo code. Nó chỉ đổi RAM hoặc TXT nếu có lời gọi cập nhật hay ghi dữ liệu tương ứng.

### UserRepository::stringToUser()

#### A. Tổng quan

- **Mục đích:** Chuyển một dòng TXT thành đối tượng Model.
- **Tham số:** `const string& line`.
- **Giá trị trả về:** `User*`; xem các nhánh `return` bên dưới.
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
Kiểm tra điều kiện trước khi tiếp tục (lỗi: throw)
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
Kết thúc
```

#### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1 — Duyệt và đối chiếu từng phần tử.**

```cpp
User* UserRepository::stringToUser(const string& line)
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
        throw runtime_error("Sai so luong truong User");
```

`throw` báo lỗi bằng ngoại lệ cho bên gọi, khác với kết quả `false` thông thường.

**Bước 3 — Kiểm tra điều kiện trước khi tiếp tục.**

```cpp
    }

    if (fields[4] != "ADMIN" && fields[4] != "STAFF")
    {
        throw runtime_error("Role User khong hop le");
    }
    UserRole role;
    if (fields[4] == "ADMIN")
    {
        role = UserRole::ADMIN; // kiểu enum
    }
```

`throw` báo lỗi bằng ngoại lệ cho bên gọi, khác với kết quả `false` thông thường.

**Bước 4 — Kiểm tra điều kiện trước khi tiếp tục.**

```cpp
    else
    {
        role = UserRole::STAFF;
    }

    if (fields[5] != "0" && fields[5] != "1")
    {
        throw runtime_error("Trang thai User khong hop le");
```

`throw` báo lỗi bằng ngoại lệ cho bên gọi, khác với kết quả `false` thông thường.

**Bước 5 — Chuẩn bị giá trị cho bước kế tiếp.**

```cpp
    }

    bool active = (fields[5] == "1");

    User* user = new User(
        fields[0],
        fields[1],
        fields[2],
        fields[3],
        role,
        active
    );
```

`new` cấp phát Model trên heap. Cho đến khi Repository nhận thành công, nhánh lỗi hoặc caller vẫn phải chịu trách nhiệm giải phóng.

**Bước 6 — Kiểm tra dữ liệu hợp lệ.**

```cpp

    if (!isValidUser(user))
    {
        delete user;
        throw runtime_error("Du lieu User khong hop le");
    }
    return user;
}
```

Điều kiện hợp lệ ngăn dữ liệu sai định dạng hoặc ngoài giới hạn đi vào danh sách đang quản lý. `delete` giải phóng đối tượng; không được dùng lại con trỏ sau lệnh này. `throw` báo lỗi bằng ngoại lệ cho bên gọi, khác với kết quả `false` thông thường.

### UserRepository::userToString()

#### A. Tổng quan

- **Mục đích:** Chuyển đối tượng Model thành một dòng TXT.
- **Tham số:** `User* user`.
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
Trả kết quả cho bên gọi
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
string UserRepository::userToString(User* user)
{
    if (!isValidUser(user))
    {
        throw runtime_error("User khong hop le khi ghi file");
    }
    string role;
    if (user->getRole() == UserRole::ADMIN)
    {
        role = "ADMIN";
```

Điều kiện hợp lệ ngăn dữ liệu sai định dạng hoặc ngoài giới hạn đi vào danh sách đang quản lý. `throw` báo lỗi bằng ngoại lệ cho bên gọi, khác với kết quả `false` thông thường.

**Bước 2 — Trả kết quả cho bên gọi.**

```cpp
    }
    else
    {
        role = "STAFF";
    }
    return user->getUserId() + "|" +
           user->getUsername() + "|" +
           user->getPassword() + "|" +
           user->getFullName() + "|" +
           role + "|" +
           (user->isActive() ? "1" : "0");
```

Đoạn này chuẩn bị hoặc trả giá trị theo code. Nó chỉ đổi RAM hoặc TXT nếu có lời gọi cập nhật hay ghi dữ liệu tương ứng.

**Bước 3 — Chuẩn bị giá trị cho bước kế tiếp.**

```cpp
}
```

Đoạn này chuẩn bị hoặc trả giá trị theo code. Nó chỉ đổi RAM hoặc TXT nếu có lời gọi cập nhật hay ghi dữ liệu tương ứng.

### UserRepository::load()

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
bool UserRepository::load()
{
    if (repo.isLoaded()) // không load lại khi đã có dữ liệu
    {
        throw runtime_error("Du lieu User da duoc nap: " + repo.getFilePath());
    }
    if (!repo.load())
    {
        throw runtime_error("Khong the nap file User: " + repo.getFilePath());
```

Lời gọi nạp đọc TXT vào bộ nhớ. Nếu trả `false`, hàm dừng ở nhánh lỗi; dữ liệu chỉ được xem là dùng được sau khi các bước kiểm tra tiếp theo hoàn tất. Trạng thái `loaded` quyết định Repository đã sẵn sàng chưa; nhánh chưa nạp dừng trước khi thao tác dựa trên dữ liệu đó. `throw` báo lỗi bằng ngoại lệ cho bên gọi, khác với kết quả `false` thông thường.

**Bước 2 — Duyệt và đối chiếu từng phần tử.**

```cpp
    }
    // Xoa chi muc cu
    usersById.clear();
    usersByUsername.clear();
    // Dua du lieu User vao hai map
    try
    {
        for (int i = 0; i < repo.count(); i++)
        {
            User* user = repo.getAt(i);
```

`try/catch` xử lý ngoại lệ trong phạm vi hiện tại; code dọn các thay đổi trước khi trả lỗi hoặc ném lại, nên thứ tự dọn có ý nghĩa. Vòng lặp xét từng bản ghi; điều kiện bên trong quyết định tiếp tục, bỏ qua hay dừng khi gặp dữ liệu sai.

**Bước 3 — Kiểm tra điều kiện trước khi tiếp tục.**

```cpp
            // Kiem tra trung ID va username
            if (usersById.find(user->getUserId()) != usersById.end() ||
                usersByUsername.find(user->getUsername()) != usersByUsername.end())
            {
                throw runtime_error(
                    "Trung ID hoac username trong file " + repo.getFilePath() + " tai dong " + to_string(i + 1));            
            }

```

`throw` báo lỗi bằng ngoại lệ cho bên gọi, khác với kết quả `false` thông thường.

**Bước 4 — Bắt lỗi và khôi phục trạng thái.**

```cpp
            usersById[user->getUserId()] = user;
            usersByUsername[user->getUsername()] = user;
        }
    }
    catch (...)
    {
        usersById.clear();
        usersByUsername.clear();
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

### UserRepository::save()

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
bool UserRepository::save()
{
    if (!repo.isLoaded())
    {
        throw runtime_error("Chua nap du lieu User");
    }
    if (!repo.save())
    {
        throw runtime_error(
            "Khong the ghi file User: " + repo.getFilePath()
        );
```

Lời gọi ghi chuyển trạng thái hiện tại ra TXT. Kết quả ghi quyết định giá trị trả về; một lời gọi này không bảo đảm các file Repository khác cùng được cập nhật. Trạng thái `loaded` quyết định Repository đã sẵn sàng chưa; nhánh chưa nạp dừng trước khi thao tác dựa trên dữ liệu đó. `throw` báo lỗi bằng ngoại lệ cho bên gọi, khác với kết quả `false` thông thường.

**Bước 2 — Xác nhận xử lý thành công.**

```cpp
    }
    return true;
}
```

Đoạn này chuẩn bị hoặc trả giá trị theo code. Nó chỉ đổi RAM hoặc TXT nếu có lời gọi cập nhật hay ghi dữ liệu tương ứng.

### UserRepository::findById()

#### A. Tổng quan

- **Mục đích:** Tra đối tượng theo ID.
- **Tham số:** `const string& userId`.
- **Giá trị trả về:** `User*`; xem các nhánh `return` bên dưới.
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
User* UserRepository::findById(const string& userId)
{
    auto it = usersById.find(userId);
    if (it == usersById.end())
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

### UserRepository::findByUsername()

#### A. Tổng quan

- **Mục đích:** Tra tài khoản theo username.
- **Tham số:** `const string& username`.
- **Giá trị trả về:** `User*`; xem các nhánh `return` bên dưới.
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
User* UserRepository::findByUsername(const string& username)
{
    auto it = usersByUsername.find(username);
    if (it == usersByUsername.end())
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

### UserRepository::getAt()

#### A. Tổng quan

- **Mục đích:** Lấy phần tử theo chỉ số.
- **Tham số:** `int index`.
- **Giá trị trả về:** `User*`; xem các nhánh `return` bên dưới.
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
User* UserRepository::getAt(int index)
{
    return repo.getAt(index);
}
```

Đoạn này chuẩn bị hoặc trả giá trị theo code. Nó chỉ đổi RAM hoặc TXT nếu có lời gọi cập nhật hay ghi dữ liệu tương ứng.

### UserRepository::count()

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
int UserRepository::count()
{
    return repo.count();
}
```

Đoạn này chuẩn bị hoặc trả giá trị theo code. Nó chỉ đổi RAM hoặc TXT nếu có lời gọi cập nhật hay ghi dữ liệu tương ứng.

### UserRepository::isLoaded()

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
bool UserRepository::isLoaded()
{
    return repo.isLoaded();
}
```

Trạng thái `loaded` quyết định Repository đã sẵn sàng chưa; nhánh chưa nạp dừng trước khi thao tác dựa trên dữ liệu đó.

### UserRepository::add()

#### A. Tổng quan

- **Mục đích:** Thêm đối tượng hợp lệ vào Repository.
- **Tham số:** `User* user`.
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
Tra cứu đối tượng theo khóa (không đạt: false)
    |
    v
Chuẩn bị giá trị cho bước kế tiếp
    |
    v
Bắt lỗi và khôi phục trạng thái
    |
    v
Kiểm tra điều kiện trước khi tiếp tục
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
bool UserRepository::add(User* user)
{
    if (!repo.isLoaded() || !isValidUser(user))
    {
        return false;
    }
    // Kiem tra trung ID
    if (findById(user->getUserId()) != nullptr)
    {
        return false;
```

Trạng thái `loaded` quyết định Repository đã sẵn sàng chưa; nhánh chưa nạp dừng trước khi thao tác dựa trên dữ liệu đó. Điều kiện hợp lệ ngăn dữ liệu sai định dạng hoặc ngoài giới hạn đi vào danh sách đang quản lý. `return false` kết thúc ngay nhánh không đạt; các thao tác cập nhật phía sau không chạy.

**Bước 2 — Tra cứu đối tượng theo khóa.**

```cpp
    }
    // Kiem tra trung username
    if (findByUsername(user->getUsername()) != nullptr)
    {
        return false;
    }
    // Khong them lai con tro da duoc quan ly
    for (int i = 0; i < repo.count(); i++)
    {
        if (repo.getAt(i) == user)
        {
            return false;
```

`return false` kết thúc ngay nhánh không đạt; các thao tác cập nhật phía sau không chạy. Vòng lặp xét từng bản ghi; điều kiện bên trong quyết định tiếp tục, bỏ qua hay dừng khi gặp dữ liệu sai.

**Bước 3 — Chuẩn bị giá trị cho bước kế tiếp.**

```cpp
        }
    }
    // Them User vao hai map truoc
    string userId = user->getUserId();
    string username = user->getUsername();

    usersById[userId] = user;

```

Đoạn này chuẩn bị hoặc trả giá trị theo code. Nó chỉ đổi RAM hoặc TXT nếu có lời gọi cập nhật hay ghi dữ liệu tương ứng.

**Bước 4 — Bắt lỗi và khôi phục trạng thái.**

```cpp
    try
    {
        usersByUsername[username] = user;
    }
    catch (...)
    {
        usersById.erase(userId);
        throw;
```

`try/catch` xử lý ngoại lệ trong phạm vi hiện tại; code dọn các thay đổi trước khi trả lỗi hoặc ném lại, nên thứ tự dọn có ý nghĩa.

**Bước 5 — Kiểm tra điều kiện trước khi tiếp tục.**

```cpp
    }

    // Them User vao Repository
    try
    {
        if (!repo.add(user))
        {
            usersById.erase(userId);
```

`try/catch` xử lý ngoại lệ trong phạm vi hiện tại; code dọn các thay đổi trước khi trả lỗi hoặc ném lại, nên thứ tự dọn có ý nghĩa.

**Bước 6 — Bắt lỗi và khôi phục trạng thái.**

```cpp
            usersByUsername.erase(username);
            return false;
        }
    }
    catch (...)
    {
        usersById.erase(userId);
        usersByUsername.erase(username);
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

### UserRepository::update()

#### A. Tổng quan

- **Mục đích:** Cập nhật đối tượng đang quản lý.
- **Tham số:** `const User& newValues`.
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
Tra cứu đối tượng theo khóa (không đạt: false)
    |
    v
Từ chối điều kiện không đạt (không đạt: false)
    |
    v
Chuẩn bị giá trị cho bước kế tiếp
    |
    v
Bắt lỗi và khôi phục trạng thái
    |
    v
Xác nhận xử lý thành công
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
bool UserRepository::update(const User& newValues)
{
    if (!repo.isLoaded() || !isValidUser(&newValues))
    {
        return false;
    }
    User* user = findById(newValues.getUserId());
    if (user == nullptr)
    {
        return false;
```

Trạng thái `loaded` quyết định Repository đã sẵn sàng chưa; nhánh chưa nạp dừng trước khi thao tác dựa trên dữ liệu đó. Điều kiện hợp lệ ngăn dữ liệu sai định dạng hoặc ngoài giới hạn đi vào danh sách đang quản lý. `return false` kết thúc ngay nhánh không đạt; các thao tác cập nhật phía sau không chạy.

**Bước 2 — Tra cứu đối tượng theo khóa.**

```cpp
    }
    // Khong cho phep thay doi role
    if (user->getRole() != newValues.getRole())
    {
        return false;
    }
    // Kiem tra username moi co bi trung khong
    User* existingUser = findByUsername(newValues.getUsername());
```

`return false` kết thúc ngay nhánh không đạt; các thao tác cập nhật phía sau không chạy.

**Bước 3 — Từ chối điều kiện không đạt.**

```cpp
    if (existingUser != nullptr && existingUser != user)
    {
        return false;
    }
    string oldUsername = user->getUsername();
    string newUsername = newValues.getUsername();
    // Neu username thay doi, them khoa moi vao map
    if (oldUsername != newUsername)
    {
        usersByUsername[newUsername] = user;
```

`return false` kết thúc ngay nhánh không đạt; các thao tác cập nhật phía sau không chạy.

**Bước 4 — Chuẩn bị giá trị cho bước kế tiếp.**

```cpp
    }
    // Cap nhat thong tin User
    try
    {
        user->setPassword(newValues.getPassword());
        user->setFullName(newValues.getFullName());
        user->setUsername(newUsername);
        user->setActive(newValues.isActive());
```

`try/catch` xử lý ngoại lệ trong phạm vi hiện tại; code dọn các thay đổi trước khi trả lỗi hoặc ném lại, nên thứ tự dọn có ý nghĩa.

**Bước 5 — Bắt lỗi và khôi phục trạng thái.**

```cpp
    }
    catch (...)
    {
        if (oldUsername != newUsername)
        {
            usersByUsername.erase(newUsername);
        }
        throw;
```

`try/catch` xử lý ngoại lệ trong phạm vi hiện tại; code dọn các thay đổi trước khi trả lỗi hoặc ném lại, nên thứ tự dọn có ý nghĩa.

**Bước 6 — Xác nhận xử lý thành công.**

```cpp
    }

    // Xoa username cu khoi map
    if (oldUsername != newUsername)
    {
        usersByUsername.erase(oldUsername);
    }
    return true;
```

Đoạn này chuẩn bị hoặc trả giá trị theo code. Nó chỉ đổi RAM hoặc TXT nếu có lời gọi cập nhật hay ghi dữ liệu tương ứng.

**Bước 7 — Chuẩn bị giá trị cho bước kế tiếp.**

```cpp
}
```

Đoạn này chuẩn bị hoặc trả giá trị theo code. Nó chỉ đổi RAM hoặc TXT nếu có lời gọi cập nhật hay ghi dữ liệu tương ứng.


