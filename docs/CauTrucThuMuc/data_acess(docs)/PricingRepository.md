# PricingRepository — mã nguồn và quy tắc dữ liệu

## 1. Tổng quan

`PricingRepository` giữ **một** đối tượng `Pricing`, đường dẫn TXT và cờ `loaded`; lớp không dùng `Repository<T>`. File có năm dòng `KEY|VALUE`: `STANDARD`, `VIP`, `COUPLE`, `STUDENT_DISCOUNT`, `MEMBER_DISCOUNT`. Giá phải lớn hơn 0, tỷ lệ trong `[0,100]`, mọi số phải hữu hạn.

`load()` kiểm tra đủ năm khóa rồi mới thay cấu hình RAM; nạp lỗi giữ cấu hình đã có. `update()` ghi file trước khi gán cấu hình mới trong RAM; nếu ghi dở, code không bảo đảm file đích còn nguyên. `save()` ghi cấu hình hiện tại. Repository không kiểm tra quyền, tính ưu đãi hay cộng điểm; `getPricing()` trả con trỏ mượn.

Mã nguồn: `src/data_access/PricingRepository.cpp` và `src/data_access/PricingRepository.h`. Các đoạn code dưới đây giữ nguyên nội dung hàm, gồm dòng trống và comment trong hàm. Khi ghép các đoạn của một hàm theo thứ tự, ta thu được đúng định nghĩa gốc.

## 2. Chi tiết từng hàm

### PricingRepository::PricingRepository()

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
PricingRepository::PricingRepository(const string& filePath)
    : filePath(filePath), loaded(false)
{
}
```

Danh sách sau dấu `:` khởi tạo thành viên trước thân constructor; tham chiếu được giữ để dùng về sau, còn thân hàm không nạp file.

### PricingRepository::load()

#### A. Tổng quan

- **Mục đích:** Nạp và kiểm tra dữ liệu từ TXT vào RAM.
- **Tham số:** không có.
- **Giá trị trả về:** `bool`; xem các nhánh `return` bên dưới.
- **Điều kiện quan trọng:** trạng thái nạp, dữ liệu hợp lệ, nhánh trả `false`.

#### B. Quy trình thực hiện

```text
Bắt đầu
    |
    v
Đọc dữ liệu TXT (không đạt: false)
    |
    v
Chuẩn bị giá trị cho bước kế tiếp
    |
    v
Duyệt và đối chiếu từng phần tử (không đạt: false)
    |
    v
Duyệt và đối chiếu từng phần tử
    |
    v
Kiểm tra dữ liệu hợp lệ (không đạt: false)
    |
    v
Bắt lỗi và khôi phục trạng thái (không đạt: false)
    |
    v
Từ chối điều kiện không đạt (không đạt: false)
    |
    v
Duyệt và đối chiếu từng phần tử (không đạt: false)
    |
    v
Chuẩn bị giá trị cho bước kế tiếp
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

**Bước 1 — Đọc dữ liệu TXT.**

```cpp
bool PricingRepository::load()
{
    vector<string> lines;
    if (!FileIO::readLines(filePath, lines))
    {
        return false;
    }
    if (lines.size() != 5)
    {
        return false;
```

Lời gọi nạp đọc TXT vào bộ nhớ. Nếu trả `false`, hàm dừng ở nhánh lỗi; dữ liệu chỉ được xem là dùng được sau khi các bước kiểm tra tiếp theo hoàn tất. `return false` kết thúc ngay nhánh không đạt; các thao tác cập nhật phía sau không chạy.

**Bước 2 — Chuẩn bị giá trị cho bước kế tiếp.**

```cpp
    }
    double values[5] = {};
    bool found[5] = {};
    const string keys[5] = {
        "STANDARD",
        "VIP",
        "COUPLE",
        "STUDENT_DISCOUNT",
        "MEMBER_DISCOUNT"
    };
```

Đoạn này chuẩn bị hoặc trả giá trị theo code. Nó chỉ đổi RAM hoặc TXT nếu có lời gọi cập nhật hay ghi dữ liệu tương ứng.

**Bước 3 — Duyệt và đối chiếu từng phần tử.**

```cpp
    for (const string& line : lines)
    {
        size_t pos = line.find('|');
        // Kiem tra dinh dang KEY|VALUE
        if (pos == string::npos ||
            pos == 0 ||
            pos == line.size() - 1 ||
            line.find('|', pos + 1) != string::npos)
        {
            return false;
```

`return false` kết thúc ngay nhánh không đạt; các thao tác cập nhật phía sau không chạy. Vòng lặp xét từng bản ghi; điều kiện bên trong quyết định tiếp tục, bỏ qua hay dừng khi gặp dữ liệu sai.

**Bước 4 — Duyệt và đối chiếu từng phần tử.**

```cpp
        }
        string key = line.substr(0, pos);
        string value = line.substr(pos + 1);
        int index = -1;
        for (int i = 0; i < 5; i++)
        {
            if (key == keys[i])
            {
                index = i;
                break;
            }
```

Vòng lặp xét từng bản ghi; điều kiện bên trong quyết định tiếp tục, bỏ qua hay dừng khi gặp dữ liệu sai.

**Bước 5 — Kiểm tra dữ liệu hợp lệ.**

```cpp
        }
        // Kiem tra khoa khong ton tai hoac bi trung
        if (index == -1 || found[index])
        {
            return false;
        }
        if (!Validator::isValidNumber(value))
        {
            return false;
```

Điều kiện hợp lệ ngăn dữ liệu sai định dạng hoặc ngoài giới hạn đi vào danh sách đang quản lý. `return false` kết thúc ngay nhánh không đạt; các thao tác cập nhật phía sau không chạy.

**Bước 6 — Bắt lỗi và khôi phục trạng thái.**

```cpp
        }
        try
        {
            values[index] = stod(value); //string to double
        }
        catch (...)
        {
            return false;
```

`try/catch` xử lý ngoại lệ trong phạm vi hiện tại; code dọn các thay đổi trước khi trả lỗi hoặc ném lại, nên thứ tự dọn có ý nghĩa. `return false` kết thúc ngay nhánh không đạt; các thao tác cập nhật phía sau không chạy. Phép parse đổi chuỗi TXT sang trường có kiểu; dữ liệu không chuyển được phải bị từ chối trước khi tạo đối tượng hợp lệ.

**Bước 7 — Từ chối điều kiện không đạt.**

```cpp
        }

        if (!std::isfinite(values[index])) //kiểm tra có hữu hạn hay k
        {
            return false;
        }

        found[index] = true;
```

`return false` kết thúc ngay nhánh không đạt; các thao tác cập nhật phía sau không chạy.

**Bước 8 — Duyệt và đối chiếu từng phần tử.**

```cpp
    }

    // Kiem tra du 5 khoa
    for (int i = 0; i < 5; i++)
    {
        if (!found[i])
        {
            return false;
```

`return false` kết thúc ngay nhánh không đạt; các thao tác cập nhật phía sau không chạy. Vòng lặp xét từng bản ghi; điều kiện bên trong quyết định tiếp tục, bỏ qua hay dừng khi gặp dữ liệu sai.

**Bước 9 — Chuẩn bị giá trị cho bước kế tiếp.**

```cpp
        }
    }

    Pricing temp(
        values[0],
        values[1],
        values[2],
        values[3],
        values[4]
    );
```

Đoạn này chuẩn bị hoặc trả giá trị theo code. Nó chỉ đổi RAM hoặc TXT nếu có lời gọi cập nhật hay ghi dữ liệu tương ứng.

**Bước 10 — Kiểm tra dữ liệu hợp lệ.**

```cpp

    if (!temp.isValid())
    {
        return false;
    }
    // Chi cap nhat khi toan bo du lieu hop le
    pricing = temp;
    loaded = true;
```

Trạng thái `loaded` quyết định Repository đã sẵn sàng chưa; nhánh chưa nạp dừng trước khi thao tác dựa trên dữ liệu đó. Điều kiện hợp lệ ngăn dữ liệu sai định dạng hoặc ngoài giới hạn đi vào danh sách đang quản lý. `return false` kết thúc ngay nhánh không đạt; các thao tác cập nhật phía sau không chạy.

**Bước 11 — Xác nhận xử lý thành công.**

```cpp
    return true;
}
```

Đoạn này chuẩn bị hoặc trả giá trị theo code. Nó chỉ đổi RAM hoặc TXT nếu có lời gọi cập nhật hay ghi dữ liệu tương ứng.

### PricingRepository::save()

#### A. Tổng quan

- **Mục đích:** Ghi trạng thái hiện tại ra TXT.
- **Tham số:** không có.
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
Ghi dữ liệu TXT
    |
    v
Kết thúc
```

#### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1 — Kiểm tra dữ liệu hợp lệ.**

```cpp
bool PricingRepository::save()
{
    if (!loaded || !pricing.isValid())
    {
        return false;
    }
    vector<string> lines = {
        "STANDARD|" + to_string(pricing.getStandardPrice()),
        "VIP|" + to_string(pricing.getVipPrice()),
        "COUPLE|" + to_string(pricing.getCouplePrice()),
        "STUDENT_DISCOUNT|" + to_string(pricing.getStudentDiscount()),
        "MEMBER_DISCOUNT|" + to_string(pricing.getMemberDiscount())
    };
```

Trạng thái `loaded` quyết định Repository đã sẵn sàng chưa; nhánh chưa nạp dừng trước khi thao tác dựa trên dữ liệu đó. Điều kiện hợp lệ ngăn dữ liệu sai định dạng hoặc ngoài giới hạn đi vào danh sách đang quản lý. `return false` kết thúc ngay nhánh không đạt; các thao tác cập nhật phía sau không chạy.

**Bước 2 — Ghi dữ liệu TXT.**

```cpp
    return FileIO::writeLines(filePath, lines);
}
```

Lời gọi ghi chuyển trạng thái hiện tại ra TXT. Kết quả ghi quyết định giá trị trả về; một lời gọi này không bảo đảm các file Repository khác cùng được cập nhật.

### PricingRepository::update()

#### A. Tổng quan

- **Mục đích:** Cập nhật đối tượng đang quản lý.
- **Tham số:** `const Pricing& newPricing`.
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
Ghi dữ liệu TXT (không đạt: false)
    |
    v
Kết thúc
```

#### C. Giải thích chi tiết từng bước kèm code C++

**Bước 1 — Kiểm tra dữ liệu hợp lệ.**

```cpp
bool PricingRepository::update(const Pricing& newPricing)
{
    if (!loaded || !newPricing.isValid())
    {
        return false;
    }
    // Luu cau hinh moi xuong file truoc
    vector<string> lines = {
        "STANDARD|" + to_string(newPricing.getStandardPrice()),
        "VIP|" + to_string(newPricing.getVipPrice()),
        "COUPLE|" + to_string(newPricing.getCouplePrice()),
        "STUDENT_DISCOUNT|" + to_string(newPricing.getStudentDiscount()),
        "MEMBER_DISCOUNT|" + to_string(newPricing.getMemberDiscount())
    };
```

Trạng thái `loaded` quyết định Repository đã sẵn sàng chưa; nhánh chưa nạp dừng trước khi thao tác dựa trên dữ liệu đó. Điều kiện hợp lệ ngăn dữ liệu sai định dạng hoặc ngoài giới hạn đi vào danh sách đang quản lý. `return false` kết thúc ngay nhánh không đạt; các thao tác cập nhật phía sau không chạy.

**Bước 2 — Ghi dữ liệu TXT.**

```cpp
    if (!FileIO::writeLines(filePath, lines))
    {
        return false;
    }
    // Chi cap nhat bo nho khi ghi thanh cong
    pricing = newPricing;
    return true;
}
```

Lời gọi ghi chuyển trạng thái hiện tại ra TXT. Kết quả ghi quyết định giá trị trả về; một lời gọi này không bảo đảm các file Repository khác cùng được cập nhật. `return false` kết thúc ngay nhánh không đạt; các thao tác cập nhật phía sau không chạy.

### PricingRepository::getPricing()

#### A. Tổng quan

- **Mục đích:** Trả cấu hình giá hiện tại.
- **Tham số:** không có.
- **Giá trị trả về:** `const Pricing*`; xem các nhánh `return` bên dưới.
- **Điều kiện quan trọng:** trạng thái nạp, con trỏ rỗng.

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
const Pricing* PricingRepository::getPricing() const
{
    if (!loaded)
    {
        return nullptr;
    }
    return &pricing;
}
```

Trạng thái `loaded` quyết định Repository đã sẵn sàng chưa; nhánh chưa nạp dừng trước khi thao tác dựa trên dữ liệu đó.

### PricingRepository::isLoaded()

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
bool PricingRepository::isLoaded() const
{
    return loaded;
}
```

Trạng thái `loaded` quyết định Repository đã sẵn sàng chưa; nhánh chưa nạp dừng trước khi thao tác dựa trên dữ liệu đó.


