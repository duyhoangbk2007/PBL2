# UserRepository – Quản lý dữ liệu tài khoản

## 1. Tổng quan

`UserRepository` là lớp thuộc tầng Data Access, chịu trách nhiệm quản lý dữ liệu tài khoản người dùng trong hệ thống quản lý rạp chiếu phim.

Lớp này sử dụng một đối tượng `Repository<User>` để quản lý danh sách tài khoản, đọc dữ liệu từ file TXT và ghi dữ liệu xuống file.

Ngoài ra, `UserRepository` sử dụng hai `unordered_map` để hỗ trợ tìm kiếm tài khoản nhanh theo ID và username.

Các chức năng chính:

- Đọc và ghi dữ liệu tài khoản.
- Kiểm tra tính hợp lệ của dữ liệu User.
- Tìm kiếm tài khoản theo ID hoặc username.
- Thêm tài khoản mới.
- Cập nhật thông tin tài khoản.
- Kiểm tra username trùng lặp.
- Quản lý hai bảng chỉ mục tìm kiếm.

`UserRepository` không chịu trách nhiệm đăng nhập, đăng xuất hay phân quyền. Những nghiệp vụ này thuộc về `AuthService`.

---

## 2. Cấu trúc lớp UserRepository

### 2.1. Thuộc tính

```cpp
private:
    Repository<User> repo;

    unordered_map<string, User*> usersById;
    unordered_map<string, User*> usersByUsername;
```

**`Repository<User> repo`**

Đây là đối tượng thuộc lớp `Repository<T>` chung, với kiểu dữ liệu cụ thể là `User`.

Nó chịu trách nhiệm:

- Quản lý danh sách các con trỏ `User*`.
- Sở hữu và giải phóng bộ nhớ của các User.
- Đọc dữ liệu từ file.
- Ghi dữ liệu xuống file.
- Cung cấp các thao tác cơ bản như `add()`, `getAt()`, `count()`.

**`usersById`**

```cpp
unordered_map<string, User*> usersById;
```

Là bảng băm sử dụng `userId` làm khóa và con trỏ `User*` làm giá trị.

Ví dụ:

```cpp
usersById["USR0001"] = user1;
usersById["USR0002"] = user2;
```

Giúp tìm kiếm tài khoản theo ID với độ phức tạp trung bình O(1).

**`usersByUsername`**

```cpp
unordered_map<string, User*> usersByUsername;
```

Là bảng băm sử dụng username làm khóa.

Ví dụ:

```cpp
usersByUsername["admin"] = user1;
usersByUsername["hoang"] = user2;
```

Bảng này hỗ trợ tìm kiếm tài khoản khi đăng nhập và kiểm tra username trùng lặp.

**Lưu ý:** Hai map không tạo ra hoặc sở hữu đối tượng User. Chúng chỉ lưu các con trỏ đến đối tượng do `Repository<User>` quản lý.

---

## 3. Constructor – Khởi tạo UserRepository

```cpp
UserRepository::UserRepository(const string& filePath)
    : repo(filePath, getUserId, stringToUser, userToString)
{
}
```

### Chức năng

Khởi tạo đối tượng `UserRepository` và truyền các hàm xử lý dữ liệu User cho `Repository<User>`.

### Giải thích

Constructor nhận một tham số `filePath`, là đường dẫn đến file lưu dữ liệu tài khoản.

Danh sách khởi tạo:

```cpp
: repo(filePath, getUserId, stringToUser, userToString)
```

gọi constructor của `Repository<User>` với bốn tham số:

1. `filePath`: Đường dẫn file dữ liệu.
2. `getUserId`: Hàm lấy ID của User.
3. `stringToUser`: Hàm chuyển một dòng TXT thành đối tượng User.
4. `userToString`: Hàm chuyển đối tượng User thành chuỗi TXT.

Nhờ cơ chế truyền con trỏ hàm, `Repository<T>` không cần biết cấu trúc bên trong của User.

Ví dụ:

```cpp
UserRepository userRepo("data/users.txt");
```

Sau khi khởi tạo, dữ liệu chưa được đọc. Cần gọi `load()` trước khi sử dụng.

---

## 4. Hàm getUserId()

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

### Chức năng

Lấy mã định danh của đối tượng User.

### Giải thích

Hàm nhận một con trỏ `User*`.

Nếu con trỏ bằng `nullptr`, hàm trả về chuỗi rỗng để tránh truy cập con trỏ không hợp lệ.

Nếu User tồn tại, hàm gọi:

```cpp
user->getUserId();
```

để lấy ID.

### Tại sao cần hàm này?

`Repository<T>` là lớp tổng quát nên không thể trực tiếp gọi `getUserId()`, vì mỗi Model có thể sử dụng tên hàm lấy ID khác nhau.

Ví dụ:

- Movie sử dụng `getMovieId()`.
- User sử dụng `getUserId()`.
- Room sử dụng `getRoomId()`.

Do đó, mỗi Repository cụ thể cung cấp một hàm lấy ID cho Repository chung.

---

## 5. Hàm isValidUser()

```cpp
bool UserRepository::isValidUser(const User* user)
```

### Chức năng

Kiểm tra dữ liệu của một đối tượng User có hợp lệ hay không.

### Các bước xử lý

**Bước 1: Kiểm tra con trỏ**

```cpp
if (user == nullptr)
{
    return false;
}
```

Không thể kiểm tra dữ liệu nếu đối tượng không tồn tại.

**Bước 2: Kiểm tra ID**

```cpp
if (!isValidUserId(user->getUserId()))
{
    return false;
}
```

ID phải có dạng `USRxxxx`, gồm tiền tố `USR` và bốn chữ số, khác `USR0000`.

Ví dụ:

- `USR0001`: Hợp lệ.
- `USR0123`: Hợp lệ.
- `MOV0001`: Không hợp lệ.
- `USR0000`: Không hợp lệ.

**Bước 3: Kiểm tra dữ liệu chuỗi**

```cpp
if (!Validator::isValidText(user->getUsername()) ||
    !Validator::isValidText(user->getPassword()) ||
    !Validator::isValidText(user->getFullName()))
{
    return false;
}
```

Sử dụng `Validator` chung để kiểm tra các trường văn bản theo quy tắc của dự án.

**Bước 4: Kiểm tra username**

```cpp
const string& username = user->getUsername();

if (username.front() == ' ' || username.back() == ' ')
{
    return false;
}
```

Không cho phép username bắt đầu hoặc kết thúc bằng dấu cách.

**Bước 5: Kiểm tra role**

```cpp
if (user->getRole() != UserRole::ADMIN &&
    user->getRole() != UserRole::STAFF)
{
    return false;
}
```

Chỉ chấp nhận hai loại tài khoản:

- `ADMIN`: Quản trị viên.
- `STAFF`: Nhân viên.

Nếu tất cả điều kiện đều hợp lệ, hàm trả về `true`.

### Lưu ý

`isValidUser()` chỉ kiểm tra tính hợp lệ của một User.

Hàm không kiểm tra username có trùng với tài khoản khác hay không. Việc này được xử lý bằng `usersByUsername` trong `load()`, `add()` và `update()`.

---

## 6. Hàm stringToUser()

```cpp
User* UserRepository::stringToUser(const string& line)
```

### Chức năng

Chuyển một dòng dữ liệu trong file TXT thành một đối tượng User.

Ví dụ:

```text
USR0001|admin|123456|Nguyen Van A|ADMIN|1
```

### Các bước xử lý

**Bước 1: Tách chuỗi**

```cpp
vector<string> fields;
size_t start = 0;
```

Sử dụng vòng lặp và `line.find('|', start)` để xác định vị trí dấu phân cách.

Mỗi phần dữ liệu được đưa vào `fields`.

Kết quả:

```cpp
fields[0] = "USR0001";
fields[1] = "admin";
fields[2] = "123456";
fields[3] = "Nguyen Van A";
fields[4] = "ADMIN";
fields[5] = "1";
```

**Bước 2: Kiểm tra số trường**

```cpp
if (fields.size() != 6)
{
    throw runtime_error("Sai so luong truong User");
}
```

Một User phải có đúng 6 trường.

Nếu số trường không hợp lệ, hàm ném ngoại lệ.

**Bước 3: Chuyển role**

Trong file TXT, role là chuỗi `"ADMIN"` hoặc `"STAFF"`.

Trong Model, role có kiểu `UserRole`.

Vì vậy, cần chuyển từ chuỗi sang enum.

**Bước 4: Chuyển trạng thái active**

```cpp
bool active = (fields[5] == "1");
```

Trong file:

- `"1"` tương ứng `true`.
- `"0"` tương ứng `false`.

Trước đó, hàm phải kiểm tra trường active chỉ nhận `"0"` hoặc `"1"`.

**Bước 5: Tạo đối tượng User**

```cpp
User* user = new User(
    fields[0],
    fields[1],
    fields[2],
    fields[3],
    role,
    active
);
```

Đối tượng được cấp phát động bằng `new`.

**Bước 6: Kiểm tra dữ liệu**

```cpp
if (!isValidUser(user))
{
    delete user;
    throw runtime_error("Du lieu User khong hop le");
}
```

Nếu dữ liệu không hợp lệ, phải giải phóng đối tượng vừa tạo để tránh rò rỉ bộ nhớ.

Nếu hợp lệ, trả về `User*`.

### Kết quả

Hàm trả về con trỏ đến đối tượng User đã được tạo.

Khi được gọi từ `Repository<User>::load()`, đối tượng này sẽ được Repository chung tiếp nhận và quản lý.

---

## 7. Hàm userToString()

```cpp
string UserRepository::userToString(User* user)
```

### Chức năng

Chuyển một đối tượng User thành một dòng dữ liệu TXT.

Đây là thao tác ngược lại của `stringToUser()`.

### Các bước xử lý

**Bước 1: Kiểm tra User**

```cpp
if (!isValidUser(user))
{
    throw runtime_error("User khong hop le khi ghi file");
}
```

Không cho phép ghi dữ liệu không hợp lệ.

**Bước 2: Chuyển role thành chuỗi**

```cpp
string role;

if (user->getRole() == UserRole::ADMIN)
{
    role = "ADMIN";
}
else
{
    role = "STAFF";
}
```

**Bước 3: Ghép các trường**

```cpp
return user->getUserId() + "|" +
       user->getUsername() + "|" +
       user->getPassword() + "|" +
       user->getFullName() + "|" +
       role + "|" +
       (user->isActive() ? "1" : "0");
```

### Kết quả

Ví dụ:

```text
USR0001|admin|123456|Nguyen Van A|ADMIN|1
```

Chuỗi này được `Repository<User>::save()` sử dụng để ghi xuống file.

---

## 8. Hàm load()

```cpp
bool UserRepository::load()
```

### Chức năng

Đọc dữ liệu tài khoản từ file TXT và xây dựng hai bảng chỉ mục tìm kiếm.

Đây là một trong những hàm quan trọng nhất của UserRepository.

### Các bước xử lý

**Bước 1: Kiểm tra trạng thái**

```cpp
if (repo.isLoaded())
{
    return false;
}
```

Nếu dữ liệu đã được load, không thực hiện load lại.

Việc này giúp tránh thay thế các đối tượng User đang được các thành phần khác sử dụng.

**Bước 2: Đọc dữ liệu**

```cpp
if (!repo.load())
{
    return false;
}
```

Gọi `Repository<User>::load()`.

Repository chung sẽ:

1. Đọc file TXT.
2. Chuyển từng dòng thành User bằng `stringToUser()`.
3. Kiểm tra ID trùng.
4. Lưu các User vào danh sách liên kết.
5. Đánh dấu trạng thái đã load khi thành công.

**Bước 3: Xóa chỉ mục cũ**

```cpp
usersById.clear();
usersByUsername.clear();
```

Đảm bảo hai map không chứa dữ liệu chỉ mục từ lần xử lý trước.

**Bước 4: Duyệt danh sách User**

```cpp
for (int i = 0; i < repo.count(); i++)
{
    User* user = repo.getAt(i);
}
```

Lấy từng User từ Repository chung.

**Bước 5: Kiểm tra trùng lặp**

```cpp
if (usersById.find(user->getUserId()) != usersById.end() ||
    usersByUsername.find(user->getUsername()) != usersByUsername.end())
{
    throw runtime_error("Trung ID hoac username");
}
```

Nếu ID hoặc username đã tồn tại trong map, dữ liệu không hợp lệ.

Lưu ý: `Repository<T>` đã kiểm tra trùng ID, nhưng UserRepository vẫn cần kiểm tra trùng username.

**Bước 6: Xây dựng chỉ mục**

```cpp
usersById[user->getUserId()] = user;
usersByUsername[user->getUsername()] = user;
```

Hai map cùng lưu con trỏ đến User do Repository chung quản lý.

**Bước 7: Xử lý lỗi**

```cpp
catch (...)
{
    usersById.clear();
    usersByUsername.clear();
    repo.clear();
    throw;
}
```

Nếu phát sinh lỗi trong quá trình xây dựng chỉ mục:

1. Xóa hai map.
2. Xóa dữ liệu vừa load trong Repository.
3. Ném lại ngoại lệ cho lớp gọi xử lý.

### Kết quả

- Trả về `true` nếu đọc file và xây dựng chỉ mục thành công.
- Trả về `false` nếu đã load hoặc `repo.load()` trả về `false`.
- Có thể ném ngoại lệ nếu dữ liệu không hợp lệ.

### Điểm khác với MovieRepository

`MovieRepository::load()` chỉ cần gọi `repo.load()`.

`UserRepository::load()` phải thực hiện thêm việc xây dựng hai map và kiểm tra username trùng.

---

## 9. Hàm save()

```cpp
bool UserRepository::save()
{
    return repo.save();
}
```

### Chức năng

Ghi danh sách User hiện tại xuống file TXT.

### Giải thích

`UserRepository` không trực tiếp mở file hoặc duyệt danh sách.

Toàn bộ công việc được giao cho `Repository<User>::save()`.

Repository chung sẽ:

1. Kiểm tra trạng thái load.
2. Duyệt các User đang quản lý.
3. Gọi `userToString()` cho từng User.
4. Ghi các dòng dữ liệu xuống file.

### Kết quả

Trả về `true` nếu ghi thành công và `false` nếu không thể ghi hoặc chưa load.

Hàm không cần ghi hai map vì map chỉ là cấu trúc chỉ mục trong bộ nhớ.

---

## 10. Hàm findById()

```cpp
User* UserRepository::findById(const string& userId)
{
    auto it = usersById.find(userId);

    if (it == usersById.end())
    {
        return nullptr;
    }

    return it->second;
}
```

### Chức năng

Tìm kiếm User dựa trên ID.

### Giải thích

```cpp
auto it = usersById.find(userId);
```

`find()` là hàm có sẵn của `unordered_map`.

Nó tìm phần tử có khóa bằng `userId` và trả về một iterator.

`auto` cho phép trình biên dịch tự suy ra kiểu iterator.

Nếu không tìm thấy:

```cpp
it == usersById.end()
```

thì trả về `nullptr`.

Nếu tìm thấy:

```cpp
return it->second;
```

trả về giá trị của phần tử trong map, tức con trỏ `User*`.

Trong một phần tử của `unordered_map`:

- `it->first`: Khóa.
- `it->second`: Giá trị.

### Độ phức tạp

O(1) trung bình, O(n) trong trường hợp xấu nhất.

---

## 11. Hàm findByUsername()

```cpp
User* UserRepository::findByUsername(const string& username)
{
    auto it = usersByUsername.find(username);

    if (it == usersByUsername.end())
    {
        return nullptr;
    }

    return it->second;
}
```

### Chức năng

Tìm kiếm User theo username.

### Giải thích

Hàm hoạt động tương tự `findById()`, nhưng sử dụng map `usersByUsername`.

Đây là hàm đặc biệt quan trọng đối với `AuthService`.

Ví dụ:

```cpp
User* user = userRepo.findByUsername("admin");
```

`AuthService` có thể sử dụng User tìm được để kiểm tra mật khẩu, trạng thái tài khoản và vai trò.

### Tại sao không duyệt Repository?

Nếu duyệt danh sách liên kết, thời gian tìm kiếm là O(n).

Với `unordered_map`, thời gian tìm kiếm trung bình là O(1).

---

## 12. Hàm getAt()

```cpp
User* UserRepository::getAt(int index)
{
    return repo.getAt(index);
}
```

### Chức năng

Lấy User theo vị trí trong danh sách.

### Giải thích

Hàm gọi `Repository<User>::getAt()`.

Ví dụ:

```cpp
User* user = userRepo.getAt(0);
```

Trả về User ở vị trí đầu tiên nếu tồn tại.

Nếu chỉ số không hợp lệ, `Repository<T>::getAt()` trả về `nullptr`.

Hàm này sử dụng Repository chung thay vì map vì `unordered_map` không hỗ trợ truy cập theo vị trí chỉ số.

---

## 13. Hàm count()

```cpp
int UserRepository::count()
{
    return repo.count();
}
```

### Chức năng

Trả về tổng số User đang được quản lý.

### Giải thích

Hàm gọi `Repository<User>::count()`.

Repository chung lấy số phần tử trong danh sách liên kết.

Ví dụ:

```cpp
int total = userRepo.count();
```

Nếu có 5 tài khoản, kết quả là `5`.

Không cần sử dụng hai map để đếm vì Repository chung là nguồn dữ liệu chính.

---

## 14. Hàm isLoaded()

```cpp
bool UserRepository::isLoaded()
{
    return repo.isLoaded();
}
```

### Chức năng

Kiểm tra dữ liệu User đã được nạp từ file hay chưa.

### Giải thích

Hàm trả về trạng thái `loaded` được quản lý bên trong `Repository<User>`.

- `true`: Dữ liệu đã load thành công.
- `false`: Chưa load hoặc dữ liệu đã được xóa khỏi Repository.

Ví dụ:

```cpp
if (!userRepo.isLoaded())
{
    userRepo.load();
}
```

---

## 15. Hàm add()

```cpp
bool UserRepository::add(User* user)
```

### Chức năng

Thêm một tài khoản mới vào hệ thống.

Hàm phải bảo đảm User hợp lệ, ID không trùng, username không trùng và dữ liệu trong hai map được đồng bộ với Repository.

### Các bước xử lý

**Bước 1: Kiểm tra trạng thái và dữ liệu**

```cpp
if (!repo.isLoaded() || !isValidUser(user))
{
    return false;
}
```

Không cho phép thêm tài khoản khi chưa load hoặc dữ liệu không hợp lệ.

**Bước 2: Kiểm tra ID trùng**

```cpp
if (findById(user->getUserId()) != nullptr)
{
    return false;
}
```

Nếu ID đã tồn tại, không thêm.

**Bước 3: Kiểm tra username trùng**

```cpp
if (findByUsername(user->getUsername()) != nullptr)
{
    return false;
}
```

Username phải duy nhất trong toàn bộ hệ thống.

**Bước 4: Kiểm tra con trỏ trùng**

```cpp
for (int i = 0; i < repo.count(); i++)
{
    if (repo.getAt(i) == user)
    {
        return false;
    }
}
```

Không được thêm cùng một con trỏ User nhiều lần.

Vì Repository chung sở hữu các đối tượng, việc lưu cùng một con trỏ hai lần có thể gây giải phóng bộ nhớ lặp.

**Bước 5: Thêm vào hai map**

```cpp
usersById[userId] = user;
usersByUsername[username] = user;
```

Tạo chỉ mục tìm kiếm cho User mới.

**Bước 6: Thêm vào Repository**

```cpp
if (!repo.add(user))
{
    usersById.erase(userId);
    usersByUsername.erase(username);
    return false;
}
```

Nếu Repository không thêm được User, hai chỉ mục vừa tạo phải được xóa để tránh dữ liệu không đồng bộ.

**Bước 7: Xử lý ngoại lệ**

Nếu việc thêm vào map hoặc Repository phát sinh ngoại lệ, các chỉ mục đã tạo được hoàn tác rồi ngoại lệ được ném tiếp.

### Quyền sở hữu bộ nhớ

Khi `add()` thành công, `Repository<User>` nhận quyền sở hữu con trỏ User.

Khi `add()` trả về `false`, Repository không nhận quyền sở hữu; bên gọi vẫn phải quản lý con trỏ đó.

### Lưu ý

`add()` chỉ cập nhật dữ liệu trong bộ nhớ.

Muốn lưu thay đổi xuống file TXT, phải gọi `save()`.

---

## 16. Hàm update()

```cpp
bool UserRepository::update(const User& newValues)
```

### Chức năng

Cập nhật thông tin của một tài khoản đã tồn tại.

Hàm cho phép thay đổi:

- Username.
- Password.
- Full name.
- Trạng thái active.

Không cho phép thay đổi:

- User ID.
- Role thông qua hàm update này.

### Các bước xử lý

**Bước 1: Kiểm tra dữ liệu**

```cpp
if (!repo.isLoaded() || !isValidUser(&newValues))
{
    return false;
}
```

**Bước 2: Tìm User cần cập nhật**

```cpp
User* user = findById(newValues.getUserId());

if (user == nullptr)
{
    return false;
}
```

Sử dụng ID để xác định tài khoản cần cập nhật.

**Bước 3: Kiểm tra role**

```cpp
if (user->getRole() != newValues.getRole())
{
    return false;
}
```

Không cho phép thay đổi vai trò thông qua hàm này.

**Bước 4: Kiểm tra username trùng**

```cpp
User* existingUser = findByUsername(newValues.getUsername());

if (existingUser != nullptr && existingUser != user)
{
    return false;
}
```

Điều kiện này có nghĩa:

- Nếu username chưa tồn tại: Cho phép.
- Nếu username đang thuộc chính User cần cập nhật: Cho phép.
- Nếu username thuộc User khác: Từ chối.

**Bước 5: Lưu username cũ và mới**

```cpp
string oldUsername = user->getUsername();
string newUsername = newValues.getUsername();
```

Hai biến này phục vụ việc cập nhật chỉ mục username.

**Bước 6: Thêm chỉ mục username mới**

```cpp
if (oldUsername != newUsername)
{
    usersByUsername[newUsername] = user;
}
```

Nếu username thay đổi, tạo khóa mới trong map trước khi xóa khóa cũ.

**Bước 7: Cập nhật thuộc tính**

```cpp
user->setPassword(newValues.getPassword());
user->setFullName(newValues.getFullName());
user->setUsername(newUsername);
user->setActive(newValues.isActive());
```

Chỉ cập nhật những trường được phép thay đổi.

Không gán toàn bộ đối tượng User bằng `*user = newValues`.

Điều này giúp giữ nguyên địa chỉ của đối tượng và các dữ liệu liên kết không thuộc phạm vi cập nhật.

**Bước 8: Xóa username cũ**

```cpp
if (oldUsername != newUsername)
{
    usersByUsername.erase(oldUsername);
}
```

Sau khi cập nhật thành công, xóa khóa cũ khỏi map.

### Ví dụ

Trước khi cập nhật:

```text
USR0002|hoang|123456|Nguyen Van Hoang|STAFF|1
```

Sau khi cập nhật:

```text
USR0002|hoang123|654321|Nguyen Van Hoang|STAFF|1
```

Trong `usersByUsername`:

```text
Trước: "hoang"    -> User*
Sau:   "hoang123" -> User*
```

`usersById` không cần thay đổi vì ID vẫn là `USR0002`.

### Xử lý ngoại lệ

Trong phiên bản hiện tại, nếu một setter ném ngoại lệ, hàm sẽ xóa khóa username mới vừa thêm rồi ném lại ngoại lệ.

Tuy nhiên, cách này chưa bảo đảm hoàn tác các thuộc tính đã được setter trước đó cập nhật.

Vì vậy, `update()` hiện tại chưa có bảo đảm cập nhật nguyên tử hoàn toàn khi phát sinh ngoại lệ.

### Kết quả

Trả về `true` khi cập nhật thành công, `false` khi không đáp ứng điều kiện nghiệp vụ.

Hàm không tự ghi file. Cần gọi `save()` để lưu thay đổi.

---

## 17. Quan hệ giữa UserRepository và Repository&lt;T&gt;

`Repository<User>` xử lý các chức năng chung, còn `UserRepository` bổ sung các quy tắc riêng của tài khoản.

| Chức năng | Repository&lt;User&gt; | UserRepository |
|---|---|---|
| Đọc file TXT | Thực hiện | Gọi và xây dựng map |
| Ghi file TXT | Thực hiện | Gọi lại |
| Quản lý bộ nhớ User | Thực hiện | Sử dụng |
| Kiểm tra trùng ID | Có | Kiểm tra thêm |
| Kiểm tra trùng username | Không | Có |
| Tìm theo ID | Duyệt danh sách | Sử dụng map |
| Tìm theo username | Không hỗ trợ trực tiếp | Sử dụng map |
| Kiểm tra dữ liệu User | Không biết quy tắc cụ thể | Thực hiện |
| Cập nhật thuộc tính User | Không hỗ trợ trực tiếp | Thực hiện |

### Nguyên tắc thiết kế

`Repository<T>` chịu trách nhiệm quản lý dữ liệu tổng quát.

`UserRepository` chịu trách nhiệm áp dụng các quy tắc dữ liệu dành riêng cho tài khoản.

`AuthService` chịu trách nhiệm xử lý nghiệp vụ đăng nhập, đăng xuất và kiểm tra quyền.

Cách phân chia này giúp các lớp có trách nhiệm rõ ràng, dễ bảo trì và mở rộng.

---

## 18. Những lưu ý quan trọng

**1. Phải đồng bộ Repository và hai map**

Khi thêm User, phải thêm chỉ mục.

Khi đổi username, phải cập nhật chỉ mục username.

Khi xóa hoặc load lại dữ liệu trong Repository, phải xử lý các chỉ mục tương ứng để tránh con trỏ không hợp lệ.

**2. Không tự ý giải phóng User đã thuộc Repository**

Khi `repo.add(user)` thành công, Repository chung chịu trách nhiệm giải phóng bộ nhớ.

Không được `delete user` từ bên ngoài nếu User vẫn đang được Repository quản lý.

**3. Không tự ý sửa username qua con trỏ**

Vì các hàm tìm kiếm hiện trả về `User*`, lớp gọi có thể truy cập các setter.

Nếu sửa username trực tiếp mà không thông qua `UserRepository::update()`, map `usersByUsername` sẽ không được cập nhật.

Do đó, các thao tác thay đổi dữ liệu cần đi qua UserRepository.

**4. Chưa có chức năng remove**

Phiên bản UserRepository hiện tại chỉ hỗ trợ thêm và cập nhật.

Chưa triển khai xóa tài khoản, nhằm tránh ảnh hưởng đến những dữ liệu khác có thể đang tham chiếu đến User.

**5. Không tự động lưu file**

`add()` và `update()` chỉ thay đổi dữ liệu trong bộ nhớ.

Muốn lưu dữ liệu, cần gọi `save()`.

**6. Mật khẩu hiện được lưu dưới dạng văn bản thuần**

Đây là giới hạn của thiết kế TXT hiện tại. Nếu phát triển thành hệ thống thực tế, cần sử dụng cơ chế băm mật khẩu an toàn thay vì lưu mật khẩu trực tiếp.

---

## 19. Tổng kết

`UserRepository` là lớp quản lý dữ liệu tài khoản được xây dựng dựa trên `Repository<User>` chung.

Lớp này sử dụng kết hợp:

- `Repository<User>` để quản lý dữ liệu và bộ nhớ.
- `unordered_map` để tìm kiếm nhanh.
- `Validator` để kiểm tra dữ liệu văn bản.
- Các hàm chuyển đổi để đọc và ghi định dạng TXT.
- Các hàm nghiệp vụ dữ liệu để kiểm tra trùng lặp và cập nhật tài khoản.

Nhờ đó, UserRepository vừa tái sử dụng được các chức năng của Repository chung, vừa đáp ứng các yêu cầu đặc thù của tài khoản người dùng trong hệ thống.