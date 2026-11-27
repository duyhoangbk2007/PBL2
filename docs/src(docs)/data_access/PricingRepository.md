# PricingRepository

**Trạng thái: CHỐT MARKDOWN.** Mã nguồn trong mục 5 là bản thiết kế đã chốt; chưa hoàn thành kiểm thử.

## 1. Tổng quan

`PricingRepository` là lớp thuộc tầng Data Access, chịu trách nhiệm quản lý bảng giá của hệ thống rạp chiếu phim.

Khác với các Repository quản lý danh sách Model, PricingRepository chỉ quản lý một đối tượng `Pricing`.

**File liên quan:**

- `src/data_access/PricingRepository.h`
- `src/data_access/PricingRepository.cpp`
- `src/models/Pricing.h`
- `src/models/Pricing.cpp`
- `data/pricing.txt`

**Trách nhiệm chính:**

- Nạp bảng giá từ file TXT.
- Kiểm tra tính hợp lệ của dữ liệu.
- Lưu và cập nhật bảng giá.
- Cung cấp bảng giá cho tầng Business.
- Quản lý trạng thái nạp dữ liệu.

PricingRepository không trực tiếp xử lý phân quyền, tính giá vé, giảm giá hoặc tính điểm tích lũy.

## 2. Kiến trúc

```text
GUI
 |
 v
PricingService
 |
 v
PricingRepository
 |
 +----> Pricing
 |
 v
FileIO
 |
 v
pricing.txt
```

PricingRepository sử dụng:

- `Pricing`: lưu trữ và kiểm tra tính hợp lệ của bảng giá.
- `Validator`: kiểm tra định dạng dữ liệu.
- `FileIO`: đọc và ghi file TXT.

PricingRepository sử dụng quan hệ thành phần (Composition) với Model Pricing.

```cpp
Pricing pricing;
```

Đối tượng Pricing được lưu trực tiếp bên trong Repository.

PricingRepository không kế thừa `IRepository<T>` và không sử dụng `Repository<T>` vì chỉ quản lý một đối tượng cấu hình, không quản lý danh sách Model.

## 3. Định dạng dữ liệu

File: `data/pricing.txt`

Mỗi dòng có định dạng:

```text
KEY|VALUE
```

Dữ liệu mặc định:

```text
STANDARD|70000
VIP|100000
COUPLE|160000
STUDENT_DISCOUNT|10
MEMBER_DISCOUNT|5
```

| Khóa | Kiểu dữ liệu | Ý nghĩa |
| --- | --- | --- |
| STANDARD | double | Giá ghế thường |
| VIP | double | Giá ghế VIP |
| COUPLE | double | Giá ghế đôi |
| STUDENT_DISCOUNT | double | Tỷ lệ giảm giá sinh viên |
| MEMBER_DISCOUNT | double | Tỷ lệ giảm giá thành viên |

### Quy tắc dữ liệu

- File phải có đúng năm dòng.
- Mỗi dòng có đúng một ký tự phân cách `|`.
- Khóa và giá trị không được để trống.
- Khóa phải thuộc danh mục năm khóa được hỗ trợ.
- Không được trùng khóa.
- Giá trị phải là số thập phân hợp lệ và hữu hạn.
- Giá ghế phải lớn hơn 0.
- Tỷ lệ giảm giá nằm trong đoạn [0, 100].
- Thứ tự các khóa khi nạp không bắt buộc cố định.
- Khi ghi file, các khóa được ghi theo thứ tự chuẩn.

Giá ghế sử dụng đơn vị VND.

Tỷ lệ giảm giá sử dụng đơn vị phần trăm.

Quy tắc tính điểm tích lũy không được lưu trong file này mà được xử lý tại PricingService.

## 4. Thiết kế lớp

### 4.1. Thuộc tính

| Thuộc tính | Kiểu dữ liệu | Ý nghĩa |
| --- | --- | --- |
| filePath | std::string | Đường dẫn file bảng giá |
| pricing | Pricing | Bảng giá hiện tại trong RAM |
| loaded | bool | Trạng thái nạp dữ liệu |

### 4.2. Các hàm private

| Hàm | Chức năng |
| --- | --- |
| writePricing() | Kiểm tra và ghi bảng giá xuống file |

### 4.3. Các hàm public

| Hàm | Chức năng |
| --- | --- |
| PricingRepository() | Khởi tạo Repository |
| load() | Nạp bảng giá |
| save() | Lưu bảng giá hiện tại |
| update() | Cập nhật và lưu bảng giá mới |
| getPricing() | Truy xuất bảng giá hiện tại |
| isLoaded() | Kiểm tra trạng thái nạp |

PricingRepository gồm 3 thuộc tính và 7 hàm.

## 5. Mã nguồn

### 5.1. PricingRepository.h

```cpp
#ifndef PRICING_REPOSITORY_H
#define PRICING_REPOSITORY_H

#include <string>
#include "../models/Pricing.h"

class PricingRepository
{
private:
    std::string filePath;
    Pricing pricing;
    bool loaded;

    bool writePricing(const Pricing& pricing);

public:
    PricingRepository(const std::string& filePath);

    bool load();
    bool save();

    bool update(const Pricing& newPricing);

    const Pricing* getPricing() const;
    bool isLoaded() const;
};

#endif
```

### 5.2. PricingRepository.cpp

```cpp
#include "PricingRepository.h"
#include "../utils/FileIO.h"
#include "../utils/Validator.h"

#include <exception>
#include <stdexcept>
#include <vector>

using namespace std;

bool PricingRepository::writePricing(const Pricing& pricing)
{
    if (!pricing.isValid())
    {
        return false;
    }

    vector<string> lines =
    {
        "STANDARD|" + to_string(pricing.getStandardPrice()),
        "VIP|" + to_string(pricing.getVipPrice()),
        "COUPLE|" + to_string(pricing.getCouplePrice()),
        "STUDENT_DISCOUNT|" + to_string(pricing.getStudentDiscount()),
        "MEMBER_DISCOUNT|" + to_string(pricing.getMemberDiscount())
    };

    return FileIO::writeLines(filePath, lines);
}

PricingRepository::PricingRepository(const string& filePath)
    : filePath(filePath), loaded(false)
{
}

bool PricingRepository::load()
{
    // Khong su dung bang gia cu neu nap lai that bai
    loaded = false;

    vector<string> lines;

    if (!FileIO::readLines(filePath, lines))
    {
        return false;
    }

    if (lines.size() != 5)
    {
        throw runtime_error(filePath + ": Phai co dung 5 dong");
    }

    const string keys[5] =
    {
        "STANDARD",
        "VIP",
        "COUPLE",
        "STUDENT_DISCOUNT",
        "MEMBER_DISCOUNT"
    };

    double values[5] = {};
    bool found[5] = {};

    for (size_t i = 0; i < lines.size(); i++)
    {
        const string& line = lines[i];
        string error = filePath + ", dong " + to_string(i + 1) + ": ";

        size_t pos = line.find('|');

        if (pos == string::npos)
        {
            throw runtime_error(error + "Sai dinh dang");
        }

        string key = line.substr(0, pos);
        string value = line.substr(pos + 1);

        int index = -1;

        for (int j = 0; j < 5; j++)
        {
            if (key == keys[j])
            {
                index = j;
                break;
            }
        }

        if (index == -1 || found[index])
        {
            throw runtime_error(error + "Khoa sai hoac bi trung");
        }

        if (!Validator::isValidNumber(value))
        {
            throw runtime_error(error + "Gia tri khong hop le");
        }

        try
        {
            values[index] = stod(value);
        }
        catch (const exception&)
        {
            throw runtime_error(error + "Loi chuyen doi gia tri");
        }

        found[index] = true;
    }

    Pricing temp(values[0],
                 values[1],
                 values[2],
                 values[3],
                 values[4]);

    if (!temp.isValid())
    {
        throw runtime_error(filePath + ": Bang gia khong hop le");
    }

    // Chi cap nhat khi toan bo bang gia hop le
    pricing = temp;
    loaded = true;

    return true;
}

bool PricingRepository::save()
{
    if (!loaded)
    {
        return false;
    }

    return writePricing(pricing);
}

bool PricingRepository::update(const Pricing& newPricing)
{
    if (!loaded || !writePricing(newPricing))
    {
        return false;
    }

    // Chi cap nhat bo nho khi ghi file thanh cong
    pricing = newPricing;

    return true;
}

const Pricing* PricingRepository::getPricing() const
{
    if (!loaded)
    {
        return nullptr;
    }

    return &pricing;
}

bool PricingRepository::isLoaded() const
{
    return loaded;
}
```

## 6. Giải thích thư viện và từ khóa

### 6.1. string

Sử dụng để lưu đường dẫn file, tên khóa, giá trị và các dòng dữ liệu TXT.

### 6.2. vector

Sử dụng `vector<string>` để lưu toàn bộ các dòng dữ liệu khi đọc hoặc ghi file.

Vector tự quản lý bộ nhớ, không cần cấp phát hoặc giải phóng thủ công.

### 6.3. exception và stdexcept

- `<exception>` cung cấp lớp `std::exception`.
- `<stdexcept>` cung cấp lớp `std::runtime_error`.

`exception` được sử dụng để bắt ngoại lệ phát sinh khi chuyển đổi chuỗi thành số.

`runtime_error` được sử dụng để báo lỗi dữ liệu khi nạp bảng giá.

### 6.4. FileIO

Sử dụng hai hàm:

```cpp
FileIO::readLines(filePath, lines);
FileIO::writeLines(filePath, lines);
```

PricingRepository không trực tiếp sử dụng ifstream hoặc ofstream.

FileIO chỉ chịu trách nhiệm đọc và ghi các dòng văn bản, không kiểm tra nghiệp vụ bảng giá.

### 6.5. Validator

Sử dụng:

```cpp
Validator::isValidNumber(value);
```

Để kiểm tra định dạng chuỗi số trước khi chuyển đổi sang double.

Ví dụ:

| Giá trị | Kết quả |
| --- | --- |
| "70000" | true |
| "10.5" | true |
| "-100" | true |
| "100abc" | false |
| "10.5.2" | false |
| "" | false |

Validator chỉ kiểm tra định dạng, không kiểm tra giới hạn giá ghế hoặc tỷ lệ giảm giá.

Việc kiểm tra các giới hạn này được thực hiện bởi `Pricing::isValid()`.

### 6.6. find() và substr()

Hàm find() tìm vị trí xuất hiện đầu tiên của một ký tự trong chuỗi.

Ví dụ:

```cpp
string line = "VIP|100000";

size_t pos = line.find('|');
```

Kết quả:

```text
pos = 3
```

Hàm substr() dùng để lấy một phần của chuỗi.

```cpp
string key = line.substr(0, pos);
string value = line.substr(pos + 1);
```

Kết quả:

```text
key   = VIP
value = 100000
```

### 6.7. stod()

Chuyển đổi chuỗi thành giá trị double.

```cpp
double price = stod("100000");
```

Kết quả:

```text
price = 100000.0
```

Hàm stod() có thể ném ngoại lệ nếu chuỗi không thể chuyển đổi hoặc giá trị vượt phạm vi của double.

PricingRepository sử dụng Validator để kiểm tra toàn bộ chuỗi trước khi gọi stod().

Không sử dụng thêm parsedLength trong thiết kế hiện tại nhằm hạn chế kiểm tra lặp lại.

Cách triển khai này phụ thuộc vào Validator::isValidNumber() hiện tại và giả định chương trình sử dụng locale số với dấu chấm thập phân.

Nếu chương trình thay đổi locale số, cần xem xét lại việc kiểm tra số ký tự đã chuyển đổi.

### 6.8. to_string()

Chuyển đổi giá trị số thành chuỗi để ghi file TXT.

```cpp
to_string(70000.0);
```

Kết quả theo C++17:

```text
70000.000000
```

Hàm có thể làm mất độ chính xác đối với các giá trị double có nhiều chữ số thập phân.

### 6.9. size_t

Kiểu số nguyên không dấu dùng để biểu diễn kích thước và vị trí trong string hoặc vector.

### 6.10. const

Tham số:

```cpp
const Pricing& newPricing
```

Cho phép truyền đối tượng bằng tham chiếu mà không thay đổi đối tượng được truyền vào.

Kiểu trả về:

```cpp
const Pricing*
```

Cho phép tầng ngoài truy cập nhưng không trực tiếp sửa đối tượng Pricing thông qua con trỏ.

Từ khóa const ở cuối phương thức getPricing() và isLoaded() cho biết hai hàm không thay đổi trạng thái Repository.

## 7. Giải thích các hàm private

### 7.1. writePricing()

```cpp
bool writePricing(const Pricing& pricing);
```

Chịu trách nhiệm kiểm tra và ghi một đối tượng Pricing xuống file TXT.

Hàm được sử dụng chung bởi save() và update() nhằm tránh lặp code.

#### Bước 1: Kiểm tra bảng giá

```cpp
if (!pricing.isValid())
{
    return false;
}
```

Tái sử dụng phương thức isValid() của Model Pricing.

Phương thức này kiểm tra:

- Tất cả giá trị phải hữu hạn.
- Giá ghế phải lớn hơn 0.
- Tỷ lệ giảm giá nằm trong đoạn [0, 100].

Không triển khai lại các điều kiện này trong Repository.

#### Bước 2: Chuyển đổi dữ liệu

```cpp
vector<string> lines =
{
    "STANDARD|" + to_string(pricing.getStandardPrice()),
    "VIP|" + to_string(pricing.getVipPrice()),
    "COUPLE|" + to_string(pricing.getCouplePrice()),
    "STUDENT_DISCOUNT|" + to_string(pricing.getStudentDiscount()),
    "MEMBER_DISCOUNT|" + to_string(pricing.getMemberDiscount())
};
```

Chuyển đổi năm thuộc tính của Pricing thành năm dòng theo định dạng KEY|VALUE.

Ví dụ:

```cpp
Pricing pricing(70000, 100000, 160000, 10, 5);
```

Dữ liệu được tạo:

```text
STANDARD|70000.000000
VIP|100000.000000
COUPLE|160000.000000
STUDENT_DISCOUNT|10.000000
MEMBER_DISCOUNT|5.000000
```

#### Bước 3: Ghi file

```cpp
return FileIO::writeLines(filePath, lines);
```

Ghi toàn bộ bảng giá xuống file TXT.

Hàm trả về:

- true: bảng giá hợp lệ và ghi file thành công.
- false: bảng giá không hợp lệ hoặc ghi file thất bại.

Hàm không thay đổi đối tượng Pricing đang được Repository quản lý.

Không kiểm tra loaded trong writePricing() vì điều kiện này đã được kiểm tra tại save() và update().

## 8. Giải thích các hàm public

### 8.1. Constructor

```cpp
PricingRepository(const std::string& filePath);
```

Khởi tạo đường dẫn file và trạng thái loaded = false.

```cpp
PricingRepository::PricingRepository(const string& filePath)
    : filePath(filePath), loaded(false)
{
}
```

Đối tượng pricing được tự động khởi tạo thông qua constructor mặc định của Model Pricing.

```cpp
Pricing::Pricing()
    : standardPrice(70000.0),
      vipPrice(100000.0),
      couplePrice(160000.0),
      studentDiscount(10.0),
      memberDiscount(5.0)
{
}
```

Ví dụ:

```cpp
PricingRepository pricingRepo("data/pricing.txt");
```

Trạng thái ban đầu:

```text
filePath = data/pricing.txt

pricing:
    standardPrice  = 70000
    vipPrice       = 100000
    couplePrice    = 160000
    studentDiscount = 10
    memberDiscount  = 5

loaded = false
```

Mặc dù Pricing đã có các giá trị mặc định, Repository vẫn chưa được xem là đã nạp dữ liệu.

Constructor không trực tiếp đọc hoặc tạo file.

### 8.2. load()

```cpp
bool load();
```

Nạp toàn bộ bảng giá từ file TXT, kiểm tra dữ liệu và cập nhật đối tượng Pricing.

#### Bước 1: Đặt lại trạng thái nạp

```cpp
loaded = false;
```

PricingRepository cho phép nạp lại bảng giá.

Nếu lần nạp mới thất bại, Repository không tiếp tục cung cấp bảng giá cũ thông qua getPricing().

Đối tượng pricing cũ vẫn tồn tại trong bộ nhớ nhưng không được xem là bảng giá đã nạp hợp lệ.

#### Bước 2: Đọc file

```cpp
vector<string> lines;

if (!FileIO::readLines(filePath, lines))
{
    return false;
}
```

Đọc toàn bộ dữ liệu từ file TXT vào vector lines.

Ví dụ:

```text
lines[0] = "STANDARD|70000"
lines[1] = "VIP|100000"
lines[2] = "COUPLE|160000"
lines[3] = "STUDENT_DISCOUNT|10"
lines[4] = "MEMBER_DISCOUNT|5"
```

Nếu không thể đọc file, trả về false.

Không tự động tạo file hoặc sử dụng bảng giá mặc định thay thế.

#### Bước 3: Kiểm tra số dòng

```cpp
if (lines.size() != 5)
{
    throw runtime_error(filePath + ": Phai co dung 5 dong");
}
```

Bảng giá phải có đúng năm dòng tương ứng với năm thuộc tính của Pricing.

Nếu không đúng số lượng, ném ngoại lệ.

#### Bước 4: Khởi tạo các mảng hỗ trợ

```cpp
const string keys[5] =
{
    "STANDARD",
    "VIP",
    "COUPLE",
    "STUDENT_DISCOUNT",
    "MEMBER_DISCOUNT"
};

double values[5] = {};
bool found[5] = {};
```

Ba mảng có các vị trí tương ứng với nhau.

| Index | keys | values | found |
| --- | --- | --- | --- |
| 0 | STANDARD | 0 | false |
| 1 | VIP | 0 | false |
| 2 | COUPLE | 0 | false |
| 3 | STUDENT_DISCOUNT | 0 | false |
| 4 | MEMBER_DISCOUNT | 0 | false |

Trong đó:

- keys chứa danh sách khóa hợp lệ.
- values lưu giá trị tương ứng với từng khóa.
- found xác định khóa đã xuất hiện hay chưa.

Mảng found được sử dụng để phát hiện khóa trùng lặp.

Ba mảng giúp chương trình đọc đúng dữ liệu mà không phụ thuộc vào thứ tự các dòng trong file.

#### Bước 5: Duyệt từng dòng

```cpp
for (size_t i = 0; i < lines.size(); i++)
{
    const string& line = lines[i];
    string error = filePath + ", dong " + to_string(i + 1) + ": ";
}
```

Duyệt lần lượt các dòng dữ liệu.

Biến error chứa đường dẫn và số dòng để sử dụng khi báo lỗi.

Sử dụng i + 1 vì chỉ số vector bắt đầu từ 0, còn số dòng trong file bắt đầu từ 1.

#### Bước 6: Tách khóa và giá trị

```cpp
size_t pos = line.find('|');

if (pos == string::npos)
{
    throw runtime_error(error + "Sai dinh dang");
}

string key = line.substr(0, pos);
string value = line.substr(pos + 1);
```

Tìm vị trí ký tự phân cách và tách dòng thành hai phần.

Ví dụ:

```text
VIP|100000
```

Kết quả:

```text
pos   = 3
key   = VIP
value = 100000
```

Chỉ cần kiểm tra trường hợp không có ký tự phân cách tại bước này.

Các trường hợp còn lại được phát hiện ở những bước validation phía sau.

| Dữ liệu | Nơi phát hiện lỗi |
| --- | --- |
| VIP100000 | Không tìm thấy ký tự phân cách |
| \|100000 | Khóa không hợp lệ |
| VIP\| | Validator::isValidNumber() |
| VIP\|100000\|200 | Validator::isValidNumber() |

Không cần kiểm tra lại các điều kiện đã được đảm nhiệm ở bước sau.

#### Bước 7: Xác định vị trí của khóa

```cpp
int index = -1;

for (int j = 0; j < 5; j++)
{
    if (key == keys[j])
    {
        index = j;
        break;
    }
}
```

Duyệt mảng keys để xác định vị trí tương ứng với khóa vừa đọc.

Ví dụ:

```text
key = VIP
```

Quá trình tìm kiếm:

```text
j = 0: VIP != STANDARD
j = 1: VIP == VIP
```

Kết quả:

```text
index = 1
```

Nếu không tìm thấy khóa, index vẫn bằng -1.

#### Bước 8: Kiểm tra khóa không hợp lệ hoặc trùng lặp

```cpp
if (index == -1 || found[index])
{
    throw runtime_error(error + "Khoa sai hoac bi trung");
}
```

Có hai trường hợp không hợp lệ:

- index == -1: khóa không thuộc danh mục.
- found[index] == true: khóa đã xuất hiện trước đó.

Toán tử || sử dụng cơ chế short-circuit.

Nếu index == -1 thì found[index] không được truy cập, tránh truy cập mảng ngoài phạm vi.

#### Bước 9: Kiểm tra và chuyển đổi giá trị

```cpp
if (!Validator::isValidNumber(value))
{
    throw runtime_error(error + "Gia tri khong hop le");
}
```

Kiểm tra toàn bộ chuỗi có định dạng số hợp lệ hay không.

Sau đó chuyển đổi sang double:

```cpp
try
{
    values[index] = stod(value);
}
catch (const exception&)
{
    throw runtime_error(error + "Loi chuyen doi gia tri");
}
```

Nếu quá trình chuyển đổi thất bại, ném runtime_error kèm thông tin file và số dòng.

Sau khi chuyển đổi thành công:

```cpp
found[index] = true;
```

Ví dụ với dòng:

```text
VIP|100000
```

Kết quả:

```text
index     = 1
values[1] = 100000
found[1]  = true
```

Nếu khóa VIP xuất hiện lần nữa, found[1] đã bằng true nên dữ liệu bị từ chối.

#### Bước 10: Kết quả sau khi đọc toàn bộ file

Giả sử file có thứ tự:

```text
VIP|100000
MEMBER_DISCOUNT|5
STANDARD|70000
STUDENT_DISCOUNT|10
COUPLE|160000
```

Sau khi duyệt:

| Index | keys | values | found |
| --- | --- | --- | --- |
| 0 | STANDARD | 70000 | true |
| 1 | VIP | 100000 | true |
| 2 | COUPLE | 160000 | true |
| 3 | STUDENT_DISCOUNT | 10 | true |
| 4 | MEMBER_DISCOUNT | 5 | true |

Dù thứ tự các dòng khác nhau, giá trị vẫn được đưa vào đúng vị trí trong mảng values.

Do đã kiểm tra:

- File có đúng năm dòng.
- Chỉ có năm khóa hợp lệ.
- Không có khóa trùng lặp.

Nên chắc chắn cả năm khóa đều xuất hiện.

Không cần duyệt lại mảng found để kiểm tra thiếu khóa.

#### Bước 11: Tạo đối tượng Pricing tạm

```cpp
Pricing temp(values[0],
             values[1],
             values[2],
             values[3],
             values[4]);
```

Tạo đối tượng Pricing từ các giá trị vừa đọc.

Tương đương:

```cpp
Pricing temp(70000, 100000, 160000, 10, 5);
```

Sử dụng đối tượng tạm để tránh cập nhật bảng giá chính thức trước khi kiểm tra toàn bộ dữ liệu.

#### Bước 12: Kiểm tra tính hợp lệ của bảng giá

```cpp
if (!temp.isValid())
{
    throw runtime_error(filePath + ": Bang gia khong hop le");
}
```

Tái sử dụng Pricing::isValid() để kiểm tra:

- Tất cả giá trị phải hữu hạn.
- Giá ghế phải lớn hơn 0.
- Tỷ lệ giảm giá phải nằm trong đoạn [0, 100].

Ví dụ:

```text
STUDENT_DISCOUNT|150
```

Giá trị "150" vượt qua Validator::isValidNumber() vì có định dạng số hợp lệ.

Tuy nhiên, Pricing::isValid() sẽ trả về false vì tỷ lệ giảm giá vượt quá 100%.

Không cần kiểm tra isfinite() riêng trong vòng lặp vì Model Pricing đã thực hiện kiểm tra này.

#### Bước 13: Cập nhật bảng giá

```cpp
pricing = temp;
loaded = true;

return true;
```

Chỉ cập nhật bảng giá chính thức khi toàn bộ dữ liệu hợp lệ.

Nếu bất kỳ bước nào thất bại, loaded vẫn bằng false.

#### Luồng xử lý tổng quát

```text
pricing.txt
     |
     v
vector<string> lines
     |
     v
Tach KEY va VALUE
     |
     v
Kiem tra khoa va gia tri
     |
     v
keys, values, found
     |
     v
Pricing temp
     |
     v
temp.isValid()
     |
     v
pricing = temp
loaded = true
```

#### Kết quả của load()

| Trường hợp | Kết quả | loaded |
| --- | --- | --- |
| File hợp lệ | true | true |
| Không đọc được file | false | false |
| File rỗng | runtime_error | false |
| Sai số dòng | runtime_error | false |
| Sai định dạng | runtime_error | false |
| Khóa không hợp lệ | runtime_error | false |
| Khóa trùng lặp | runtime_error | false |
| Giá trị sai định dạng | runtime_error | false |
| Lỗi chuyển đổi giá trị | runtime_error | false |
| Bảng giá không hợp lệ | runtime_error | false |
| Nạp lại thành công | true | true |
| Nạp lại thất bại | false hoặc ngoại lệ | false |

### 8.3. save()

```cpp
bool save();
```

Lưu bảng giá hiện tại xuống file TXT.

```cpp
if (!loaded)
{
    return false;
}

return writePricing(pricing);
```

Kiểm tra Repository đã được nạp, sau đó sử dụng writePricing() để kiểm tra và ghi bảng giá.

Hàm không thay đổi đối tượng Pricing trong RAM.

Nếu ghi file thất bại, trạng thái loaded vẫn được giữ nguyên.

### 8.4. update()

```cpp
bool update(const Pricing& newPricing);
```

Cập nhật bảng giá mới và lưu ngay xuống file TXT.

#### Bước 1: Kiểm tra và ghi bảng giá mới

```cpp
if (!loaded || !writePricing(newPricing))
{
    return false;
}
```

Nếu Repository chưa load, writePricing() không được gọi do cơ chế short-circuit.

Nếu đã load, writePricing() thực hiện kiểm tra và ghi bảng giá mới.

Nếu validation hoặc ghi file thất bại, trả về false.

#### Bước 2: Cập nhật bộ nhớ

```cpp
pricing = newPricing;

return true;
```

Chỉ cập nhật bảng giá trong RAM khi ghi file thành công.

#### Ví dụ

Bảng giá hiện tại:

```cpp
Pricing pricing(70000, 100000, 160000, 10, 5);
```

Thay đổi giá VIP thành 120000:

```cpp
Pricing newPricing(70000, 120000, 160000, 10, 5);

pricingRepo.update(newPricing);
```

Quá trình thực hiện:

```text
Kiem tra loaded
       |
       v
writePricing(newPricing)
       |
       v
Kiem tra newPricing.isValid()
       |
       v
Ghi bang gia moi xuong TXT
       |
       v
pricing = newPricing
       |
       v
return true
```

Khác với các Repository quản lý danh sách Model, PricingRepository::update() thực hiện lưu file ngay.

Cơ chế này phù hợp với cách PricingService hiện tại cập nhật bảng giá.

Nếu ghi file thất bại, bảng giá trong RAM không thay đổi.

Tuy nhiên, FileIO chưa hỗ trợ ghi file nguyên tử nên không đảm bảo nội dung file cũ vẫn nguyên vẹn nếu lỗi xảy ra trong quá trình ghi.

### 8.5. getPricing()

```cpp
const Pricing* getPricing() const;
```

Trả về con trỏ đến bảng giá hiện tại.

```cpp
if (!loaded)
{
    return nullptr;
}

return &pricing;
```

Hàm trả về:

- nullptr: Repository chưa được nạp thành công.
- const Pricing*: bảng giá hiện tại.

Sử dụng const Pricing* để tầng ngoài không trực tiếp sửa đối tượng Pricing.

Con trỏ không chuyển quyền sở hữu cho tầng gọi và không được giải phóng bằng delete.

Sau khi nạp lại thất bại, tầng gọi không được tiếp tục sử dụng con trỏ đã lấy trước đó.

Cần gọi lại getPricing() khi thực hiện nghiệp vụ mới.

### 8.6. isLoaded()

```cpp
bool isLoaded() const;
```

Trả về trạng thái nạp dữ liệu.

- true: bảng giá đã được nạp thành công.
- false: chưa nạp hoặc lần nạp gần nhất thất bại.

## 9. Quản lý bộ nhớ

PricingRepository lưu trực tiếp đối tượng Pricing:

```cpp
Pricing pricing;
```

Không sử dụng new hoặc delete.

Đối tượng Pricing tồn tại trong suốt vòng đời của PricingRepository.

Các biến tạm, mảng và vector được tự động giải phóng khi hàm kết thúc.

Không cần tự định nghĩa destructor.

Con trỏ do getPricing() trả về không sở hữu đối tượng.

## 10. DIP và phân chia trách nhiệm

PricingRepository không kế thừa IRepository<T> vì không quản lý danh sách Model theo CRUD.

Không cần triển khai các hàm add(), removeById(), findById(), getAt() hoặc count().

Dependency giữa PricingService và PricingRepository sẽ được xem xét khi refactor tầng Business.

| Thành phần | Trách nhiệm |
| --- | --- |
| Pricing | Lưu trữ và kiểm tra bảng giá |
| Validator | Kiểm tra định dạng dữ liệu |
| PricingRepository | Nạp, lưu và cập nhật bảng giá |
| PricingService | Phân quyền và xử lý nghiệp vụ giá vé |
| FileIO | Đọc và ghi file TXT |
| GUI | Nhập liệu, hiển thị và thông báo lỗi |

PricingRepository không trực tiếp kiểm tra quyền PRICING_VIEW hoặc PRICING_UPDATE.

Việc phân quyền được thực hiện tại PricingService.

## 11. Các thay đổi so với phiên bản cũ

- Giữ nguyên ba thuộc tính và sáu hàm public.
- Bổ sung hàm private writePricing().
- Loại bỏ logic ghi file trùng lặp giữa save() và update().
- Đặt loaded = false trước khi nạp lại.
- Đơn giản hóa kiểm tra định dạng dòng.
- Gộp kiểm tra khóa không hợp lệ và khóa trùng.
- Sử dụng runtime_error khi dữ liệu file không hợp lệ.
- Bổ sung thông tin file và số dòng khi báo lỗi.
- Bỏ vòng lặp kiểm tra đủ khóa ở cuối load().
- Bỏ kiểm tra isfinite() trùng với Pricing::isValid().
- Giữ nguyên cơ chế ghi file trước khi cập nhật RAM.
- Chuẩn hóa cấu trúc, định dạng và comment.
- Không thay đổi định dạng pricing.txt.

### Giới hạn hiện tại

- FileIO chưa hỗ trợ ghi file nguyên tử.
- to_string() có thể làm mất độ chính xác của double.
- Giá trị trong RAM và giá trị nạp lại từ file có thể khác nhau do làm tròn khi ghi.
- Giá dương quá nhỏ có thể bị ghi thành 0.000000 và không hợp lệ khi nạp lại.
- Việc sử dụng stod() không kiểm tra parsedLength phụ thuộc vào Validator hiện tại và locale số của chương trình.
- Chưa hỗ trợ nhiều tiến trình cùng cập nhật bảng giá.

Các giới hạn trên được ghi nhận, chưa mở rộng phạm vi refactor sang FileIO, Validator hoặc Model Pricing.

## 12. Kế hoạch kiểm thử

File kiểm thử dự kiến:

`tests/test_pricing_repository.cpp`

### 12.1. Trạng thái ban đầu

- [ ] isLoaded() trả về false.
- [ ] getPricing() trả về nullptr.
- [ ] save() trước khi load.
- [ ] update() trước khi load.

### 12.2. Load dữ liệu hợp lệ

- [ ] File bảng giá mặc định.
- [ ] File có thứ tự khóa khác nhau.
- [ ] Giá ghế có phần thập phân.
- [ ] Tỷ lệ giảm giá bằng 0.
- [ ] Tỷ lệ giảm giá bằng 100.
- [ ] Kiểm tra đầy đủ năm thuộc tính.

### 12.3. Load dữ liệu không hợp lệ

- [ ] File không tồn tại.
- [ ] File rỗng.
- [ ] Thiếu dòng dữ liệu.
- [ ] Thừa dòng dữ liệu.
- [ ] Thiếu ký tự phân cách.
- [ ] Thừa ký tự phân cách.
- [ ] Khóa rỗng.
- [ ] Giá trị rỗng.
- [ ] Khóa không hợp lệ.
- [ ] Khóa trùng lặp.
- [ ] Khóa sai chữ hoa hoặc chữ thường.
- [ ] Giá trị không phải số.
- [ ] Giá trị có khoảng trắng.
- [ ] Giá trị có nhiều dấu chấm.
- [ ] Giá trị vượt phạm vi double.
- [ ] Giá trị NaN hoặc infinity.
- [ ] Giá ghế bằng 0.
- [ ] Giá ghế âm.
- [ ] Tỷ lệ giảm giá âm.
- [ ] Tỷ lệ giảm giá lớn hơn 100.
- [ ] Kiểm tra thông tin ngoại lệ.
- [ ] Không công bố bảng giá khi load thất bại.

### 12.4. Save

- [ ] Lưu bảng giá hợp lệ.
- [ ] Kiểm tra định dạng file sau khi lưu.
- [ ] Kiểm tra thứ tự năm khóa.
- [ ] Lưu thất bại khi không thể ghi file.
- [ ] Nạp lại bảng giá sau khi lưu.

### 12.5. Update

- [ ] Cập nhật giá ghế thường.
- [ ] Cập nhật giá ghế VIP.
- [ ] Cập nhật giá ghế đôi.
- [ ] Cập nhật giảm giá sinh viên.
- [ ] Cập nhật giảm giá thành viên.
- [ ] Cập nhật đồng thời nhiều thuộc tính.
- [ ] Cập nhật bảng giá không hợp lệ.
- [ ] Cập nhật giá trị NaN hoặc infinity.
- [ ] Ghi file ngay khi update thành công.
- [ ] Bảng giá trong RAM không đổi khi validation lỗi.
- [ ] Bảng giá trong RAM không đổi khi ghi file lỗi.
- [ ] Nạp lại bảng giá sau khi update.

### 12.6. Nạp lại

- [ ] Nạp lại file hợp lệ.
- [ ] Nạp lại sau khi thay đổi bảng giá.
- [ ] Nạp lại file không tồn tại.
- [ ] Nạp lại file có dữ liệu không hợp lệ.
- [ ] isLoaded() trả về false khi nạp lại thất bại.
- [ ] getPricing() trả về nullptr khi nạp lại thất bại.
- [ ] Không cho phép save hoặc update sau khi nạp lỗi.
- [ ] Nạp lại thành công sau lần nạp lỗi.

### 12.7. Phạm vi kiểm thử

Kiểm thử thông qua các phương thức public của PricingRepository.

Hàm writePricing() được kiểm thử gián tiếp thông qua save() và update().

Sử dụng file dữ liệu test riêng, không thay đổi data/pricing.txt.

Chưa kiểm thử phân quyền, tính giá vé, giảm giá hoặc tính điểm tích lũy tại PricingService.

Các giới hạn về độ chính xác khi ghi số thực và locale cần được ghi nhận riêng, không đánh dấu đã xử lý khi chưa có thay đổi tương ứng.

## 13. Tiến độ triển khai

`PricingService::load()` chuyển tiếp ngoại lệ từ Repository lên tầng gọi. Hiện chưa có nơi gọi hàm này trong `src/`. Hai fixture legacy tại `test_booking_service.cpp` và `test_cancel_ticket.cpp` gọi trực tiếp `prices.load()` trong điều kiện boolean; khi dữ liệu sai, ngoại lệ thoát khỏi điều kiện và được `main()` bắt ở mức tổng quát. Các trường hợp dữ liệu sai trong `test_pricing.cpp` vẫn mong `false`, cần thay khi xây dựng test mới. Không sửa các nơi gọi hoặc test trong đợt này.

- [x] Rà soát mã nguồn hiện tại.
- [x] Xác định nội dung cần refactor.
- [x] Xây dựng thiết kế Markdown.
- [x] Đơn giản hóa hàm load().
- [x] Thống nhất cơ chế save() và update().
- [x] Rà soát Markdown lần cuối.
- [x] Kiểm tra tương thích tại các nơi gọi load().
- [x] CHỐT MARKDOWN.
- [x] Triển khai PricingRepository.h.
- [x] Triển khai PricingRepository.cpp.
- [x] Biên dịch cú pháp PricingRepository.cpp bằng C++17.
- [ ] Kiểm thử.
- [ ] Cập nhật kết quả kiểm thử và tiến độ.

## 14. Kết luận

PricingRepository chịu trách nhiệm quản lý bảng giá của hệ thống rạp chiếu phim.

Lớp sử dụng Model Pricing để lưu trữ và kiểm tra dữ liệu, không cần sử dụng Repository<T> hoặc IRepository<T>.

Hàm load() sử dụng ba mảng keys, values và found để đọc bảng giá mà không phụ thuộc vào thứ tự các khóa trong file.

Hàm writePricing() giúp tái sử dụng logic kiểm tra và ghi file giữa save() và update().

Các nghiệp vụ liên quan đến phân quyền, tính giá vé, giảm giá và tích điểm tiếp tục được xử lý tại PricingService.
