# RolePermissionRepository — mã nguồn và quy tắc dữ liệu

## 1. Tổng quan

`RolePermissionRepository` đọc các dòng `ROLE|PERMISSION` vào `vector<string>`, không dùng `Repository<T>`. `loaded` cho biết bảng quyền hợp lệ; nếu nạp lỗi, `hasPermission()` trả `false`. Repository không đăng nhập người dùng hoặc tự cấp quyền.

Code chỉ chấp nhận vai trò `ADMIN` và `STAFF`, dòng đúng một dấu `|`, không rỗng, không trùng. Nó chưa đối chiếu mã `PERMISSION` với danh sách quyền thiết kế. Dữ liệu được thêm dần khi nạp; nếu lỗi ở dòng sau, vector có thể còn phần đã đọc nhưng `loaded = false` ngăn sử dụng.

Mã nguồn: `src/data_access/RolePermissionRepository.cpp` và `src/data_access/RolePermissionRepository.h`. Các đoạn code dưới đây giữ nguyên nội dung hàm, gồm dòng trống và comment trong hàm. Khi ghép các đoạn của một hàm theo thứ tự, ta thu được đúng định nghĩa gốc.

## 2. Chi tiết từng hàm

### RolePermissionRepository::RolePermissionRepository()

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
RolePermissionRepository::RolePermissionRepository(const string& filePath)
    : filePath(filePath), loaded(false)
{
}
```

Danh sách sau dấu `:` khởi tạo thành viên trước thân constructor; tham chiếu được giữ để dùng về sau, còn thân hàm không nạp file.

### RolePermissionRepository::load()

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
Duyệt và đối chiếu từng phần tử (không đạt: false)
    |
    v
Duyệt và đối chiếu từng phần tử (không đạt: false)
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
bool RolePermissionRepository::load()
{
    loaded = false;
    permissions.clear();
    vector<string> lines;
    if (!FileIO::readLines(filePath, lines))
    {
        return false;
```

Lời gọi nạp đọc TXT vào bộ nhớ. Nếu trả `false`, hàm dừng ở nhánh lỗi; dữ liệu chỉ được xem là dùng được sau khi các bước kiểm tra tiếp theo hoàn tất. Trạng thái `loaded` quyết định Repository đã sẵn sàng chưa; nhánh chưa nạp dừng trước khi thao tác dựa trên dữ liệu đó. `return false` kết thúc ngay nhánh không đạt; các thao tác cập nhật phía sau không chạy.

**Bước 2 — Duyệt và đối chiếu từng phần tử.**

```cpp
    }
    for (const string& line : lines)
    {
        size_t pos = line.find('|');
        if (pos == string::npos ||
            pos == 0 ||
            pos == line.size() - 1 ||
            line.find('|', pos + 1) != string::npos)
        {
            return false;
```

`return false` kết thúc ngay nhánh không đạt; các thao tác cập nhật phía sau không chạy. Vòng lặp xét từng bản ghi; điều kiện bên trong quyết định tiếp tục, bỏ qua hay dừng khi gặp dữ liệu sai.

**Bước 3 — Duyệt và đối chiếu từng phần tử.**

```cpp
        }
        string role = line.substr(0, pos);
        if (role != "ADMIN" && role != "STAFF")
        {
            return false;
        }
        for (const string& item : permissions)
        {
            if (item == line)
            {
                return false;
            }
```

`return false` kết thúc ngay nhánh không đạt; các thao tác cập nhật phía sau không chạy. Vòng lặp xét từng bản ghi; điều kiện bên trong quyết định tiếp tục, bỏ qua hay dừng khi gặp dữ liệu sai.

**Bước 4 — Xác nhận xử lý thành công.**

```cpp
        }
        permissions.push_back(line);
    }
    loaded = true;
    return true;
}
```

Trạng thái `loaded` quyết định Repository đã sẵn sàng chưa; nhánh chưa nạp dừng trước khi thao tác dựa trên dữ liệu đó.

### RolePermissionRepository::isLoaded()

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
bool RolePermissionRepository::isLoaded() const
{
    return loaded;
}
```

Trạng thái `loaded` quyết định Repository đã sẵn sàng chưa; nhánh chưa nạp dừng trước khi thao tác dựa trên dữ liệu đó.

### RolePermissionRepository::hasPermission()

#### A. Tổng quan

- **Mục đích:** Kiểm tra một vai trò có mã quyền.
- **Tham số:** `const string& role, const string& permission`.
- **Giá trị trả về:** `bool`; xem các nhánh `return` bên dưới.
- **Điều kiện quan trọng:** trạng thái nạp, nhánh trả `false`.

#### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Duyệt và đối chiếu từng phần tử (không đạt: false)
    |
    v
Từ chối điều kiện không đạt (không đạt: false)
    |
    v
Kết thúc
```

#### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1 — Duyệt và đối chiếu từng phần tử.**

```cpp
bool RolePermissionRepository::hasPermission(const string& role, const string& permission) const
{
    if (!loaded)
    {
        return false;
    }
    string key = role + "|" + permission;
    for (const string& item : permissions)
    {
        if (item == key)
        {
            return true;
```

Trạng thái `loaded` quyết định Repository đã sẵn sàng chưa; nhánh chưa nạp dừng trước khi thao tác dựa trên dữ liệu đó. `return false` kết thúc ngay nhánh không đạt; các thao tác cập nhật phía sau không chạy. Vòng lặp xét từng bản ghi; điều kiện bên trong quyết định tiếp tục, bỏ qua hay dừng khi gặp dữ liệu sai.

**Bước 2 — Từ chối điều kiện không đạt.**

```cpp
        }
    }
    return false;
}
```

`return false` kết thúc ngay nhánh không đạt; các thao tác cập nhật phía sau không chạy.


