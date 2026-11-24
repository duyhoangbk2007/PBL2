# Quy tắc kiểm tra dữ liệu (Validation Rules)

## 1. Mục tiêu

- Phân chia rõ trách nhiệm kiểm tra dữ liệu giữa các tầng.
- Hạn chế kiểm tra trùng lặp.
- Đảm bảo tính hợp lệ và toàn vẹn dữ liệu.
- Đồng bộ cách triển khai Repository và Service.

## 2. Nguyên tắc chung

- Mỗi tầng chỉ kiểm tra những điều kiện thuộc trách nhiệm của mình.
- Tận dụng các kiểm tra đã được thực hiện ở tầng dưới.
- Không kiểm tra lại nếu tầng hiện tại không cần sử dụng điều kiện đó.
- Tập trung logic kiểm tra dùng chung vào các hàm hỗ trợ.
- Không loại bỏ kiểm tra nếu ảnh hưởng đến tính đúng đắn,
  toàn vẹn dữ liệu hoặc an toàn bộ nhớ.

## 3. Phân chia trách nhiệm

### 3.1. Repository<T>

Chịu trách nhiệm kiểm tra các điều kiện lưu trữ chung:

- Trạng thái loaded.
- Con trỏ nullptr khi thêm đối tượng.
- Trùng con trỏ.
- ID rỗng hoặc trùng ID chính.
- Sự tồn tại của đối tượng khi xóa.
- Vị trí truy cập trong danh sách.
- Lỗi đọc ghi file.

Không kiểm tra dữ liệu riêng của từng Model, liên kết
giữa các Model hoặc điều kiện nghiệp vụ.

### 3.2. Concrete Repository

Chịu trách nhiệm kiểm tra tính hợp lệ và toàn vẹn
dữ liệu của từng Model:

- Định dạng ID, chuỗi, ngày tháng.
- Giá trị và mối quan hệ giữa các thuộc tính.
- Trùng các thuộc tính duy nhất ngoài ID chính.
- Sự tồn tại của các đối tượng được tham chiếu.
- Ràng buộc liên kết giữa các Model.
- Điều kiện cập nhật và xóa dữ liệu.

Quy định:

- Tập trung kiểm tra thuộc tính vào hàm isValid...().
- Tái sử dụng các phương thức của Validator.
- Sử dụng lại các kiểm tra của Repository<T>.
- Đảm bảo dữ liệu hợp lệ ngay cả khi không thông qua Service.

### 3.3. Business Service

Chịu trách nhiệm kiểm tra và xử lý nghiệp vụ:

- Phân quyền người dùng.
- Điều kiện thực hiện chức năng.
- Điều kiện chuyển đổi trạng thái.
- Thời điểm thực hiện nghiệp vụ.
- Sinh ID mới.
- Điều phối các Repository và rollback giao dịch.

Không kiểm tra lại các điều kiện lưu trữ và tính hợp lệ
của Model đã được Repository đảm bảo, trừ khi cần thiết
để thực hiện nghiệp vụ an toàn.

### 3.4. Presentation (GUI)

- Kiểm tra dữ liệu nhập liệu.
- Kiểm tra khả năng chuyển đổi kiểu dữ liệu.
- Thông báo lỗi cho người dùng.
- Không thay thế các kiểm tra tại Service và Repository.
- Không tự triển khai lại logic nghiệp vụ.

## 4. Trách nhiệm theo từng phương thức

| Phương thức | Repository<T> | Concrete Repository | Service |
|---|---|---|---|
| load() | Đọc file, loaded, ID rỗng, trùng ID | Parse, validation, liên kết | Điều phối nếu cần |
| save() | Loaded, ghi file | Serialize, validation | Điều phối nếu cần |
| add() | Điều kiện lưu trữ chung | Validation, ràng buộc riêng | Phân quyền, sinh ID, nghiệp vụ |
| update() | Không triển khai | Validation, tìm kiếm, cập nhật | Phân quyền, nghiệp vụ |
| removeById() | Loaded, tìm kiếm, xóa | Ràng buộc liên kết, unlink | Phân quyền, nghiệp vụ |
| findById() | Tìm kiếm | Xử lý khóa riêng nếu cần | Phân quyền nếu cần |
| getAt() | Kiểm tra vị trí | Gọi phương thức chung | Phân quyền nếu cần |
| count() | Đếm đối tượng | Gọi phương thức chung | Phân quyền nếu cần |
| isLoaded() | Trả về trạng thái | Gọi phương thức chung | Sử dụng khi cần |

## 5. Các trường hợp được phép kiểm tra lại

Được phép kiểm tra lại điều kiện của tầng dưới khi:

- Cần kiểm tra nullptr trước khi truy cập đối tượng.
- Cần kiểm tra loaded trước khi xử lý dữ liệu.
- Cần sử dụng đối tượng để kiểm tra nghiệp vụ.
- Cần thay đổi dữ liệu trước khi gọi tầng dưới.
- Cần thực hiện link, unlink hoặc cập nhật chỉ mục phụ.
- Cần đảm bảo an toàn bộ nhớ và rollback.

Việc kiểm tra dữ liệu tại các thời điểm khác nhau
(load, add, update, save) không được xem là kiểm tra
trùng lặp không cần thiết.

## 6. Ví dụ áp dụng

### 6.1. Concrete Repository

```cpp
bool MovieRepository::add(Movie* movie)
{
    if (!isValidMovie(movie))
    {
        return false;
    }

    return repo.add(movie);
}
```

- isValidMovie() kiểm tra tính hợp lệ của Movie.
- repo.add() kiểm tra các điều kiện lưu trữ chung.
- Không kiểm tra lại loaded, trùng ID hoặc trùng con trỏ
  nếu không cần xử lý trước khi gọi repo.add().

### 6.2. Business Service

```cpp
bool MovieService::removeMovie(const string& movieId)
{
    authService.requirePermission("MOVIE_DELETE");

    return movieRepo.removeById(movieId);
}
```

- Service kiểm tra phân quyền.
- MovieRepository kiểm tra ràng buộc liên kết.
- Repository<T> kiểm tra điều kiện lưu trữ và xóa dữ liệu.

Service không cần kiểm tra lại sự tồn tại của Movie
vì không trực tiếp sử dụng đối tượng.

## 7. Xử lý lỗi và toàn vẹn dữ liệu

- Sử dụng false, nullptr và exception theo hợp đồng
  của từng phương thức.
- Chỉ sử dụng try-catch khi cần xử lý lỗi, giải phóng
  bộ nhớ, bổ sung thông tin lỗi hoặc rollback.
- Không loại bỏ các kiểm tra cần thiết cho link và unlink.
- Concrete Repository chịu trách nhiệm rollback
  các thay đổi nội bộ.
- Service chịu trách nhiệm rollback giao dịch
  liên quan đến nhiều Repository.
- Không làm thay đổi quyền sở hữu đối tượng khi refactor.

## 8. Quy định áp dụng

- Giữ nguyên Repository<T> làm lớp lưu trữ dùng chung.
- Sử dụng MovieRepository sau khi đồng bộ validation
  làm mẫu chuẩn cho các Concrete Repository.
- Áp dụng thống nhất cho tất cả Repository và Service.
- Không sao chép máy móc logic giữa các Model.
- Chạy lại các test liên quan sau khi thay đổi kiểm tra.
- Tuân thủ các quy tắc nghiệp vụ trong business_rules.md
  và định dạng dữ liệu trong data_format.md.