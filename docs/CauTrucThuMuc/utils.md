# `src/utils/` — tiện ích dùng chung

**Hiện trạng đến hết Bước 03:** đã có ba cặp file `FileIO.h/.cpp`, `DateTime.h/.cpp`, `IdGenerator.h/.cpp`. Chúng hỗ trợ kỹ thuật cho chương trình, không phải tầng nghiệp vụ và không tự quyết định quyền, giá vé hay điều kiện bán/hủy.

| File | Ý nghĩa và API hiện có |
| --- | --- |
| `FileIO.h/.cpp` | `readLines(path, lines)` đọc các dòng TXT; `writeLines(path, lines)` ghi toàn bộ dòng; hai hàm trả `bool`. Không parse Model. |
| `DateTime.h/.cpp` | `getCurrentDateTime()` trả chuỗi thời điểm hiện tại; chưa có API parse/so sánh ngày giờ đầy đủ. |
| `IdGenerator.h/.cpp` | `generateNextId(entity, sequencesPath, id, updatedSequences)` tính mã mới và nội dung bộ đếm mới; **không tự ghi** `sequences.txt`. |

## Sườn API đang có

```cpp
class FileIO {
public:
    static bool readLines(const std::string& path,
                          std::vector<std::string>& lines);
    static bool writeLines(const std::string& path,
                           const std::vector<std::string>& lines);
};

class DateTime {
public:
    static std::string getCurrentDateTime();
};

class IdGenerator {
public:
    static void generateNextId(const std::string& entity,
                               const std::string& sequencesPath,
                               std::string& id,
                               std::string& updatedSequences);
};
```

Đây là chữ ký rút từ các header hiện có; phần `#include` được lược bỏ để tập trung vào vai trò. Khi đọc dữ liệu, lớp truy xuất dữ liệu sẽ gọi `FileIO`, tự kiểm tra số trường/kiểu/khóa/tham chiếu và tạo Model. Khi cần lưu, lớp đó chuẩn bị các dòng rồi gọi `FileIO`; lỗi phải được báo rõ. Ghi nhiều file tuần tự **không nguyên tử**.

Mọi TXT, kể cả `sequences.txt`, không có `#schema=1` hoặc dòng tiêu đề. Không đổi quy tắc ID hoặc định dạng TXT khi chỉ tổ chức lại thư mục. Thiết kế mới không dùng `TransactionManager`; mã cũ được lưu tại `tests/legacy/` để đối chiếu lịch sử Bước 03.

Chi tiết từng tiện ích: [FileIO](<utils(docs)/FileHandler.md>) (tên file tài liệu cũ được giữ), [DateTime](<utils(docs)/DateTime.md>), [IdGenerator](<utils(docs)/IdGenerator.md>). Xem thêm [định dạng dữ liệu](../TongQuan/data_format.md) và [nhật ký Bước 03](../development/Buoc03_HaTangLuuTruDuLieu.md).
