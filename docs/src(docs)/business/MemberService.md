# MemberService

> Trạng thái: CHỐT MARKDOWN — đã triển khai và hoàn thành trong phạm vi kiểm thử ngày 27/11/2026.

## 1. Tổng quan

MemberService thuộc tầng Business Logic, quản lý thông tin thành viên, phân quyền, sinh ID và truy xuất dữ liệu. Không trực tiếp đọc ghi TXT, không chỉnh điểm trực tiếp và không xóa thành viên.

File liên quan: `src/business/MemberService.h`, `src/business/MemberService.cpp`, `src/data_access/IRepository.h`, `src/models/Member.h`, `src/business/AuthService.h`.

## 2. Kiến trúc

```text
GUI -> MemberService -> IRepository<Member> <- MemberRepository
            |                                       |
            v                                       v
        AuthService                          Repository<Member>
```

DIP: phụ thuộc interface thay vì concrete repository. DI: truyền dependency qua constructor bằng tham chiếu, không sở hữu dependency. Permission-only: kiểm tra quyền, không kiểm tra role để cấp quyền.

## 3. Thuộc tính và API

| Thuộc tính | Kiểu | Vai trò |
|---|---|---|
| memberRepo | IRepository<Member>& | Truy xuất và lưu trữ thành viên |
| authService | AuthService& | Kiểm tra quyền |

| Hàm | Permission | Kết quả |
|---|---|---|
| load | Không | bool |
| save | Không | bool |
| addMember | MEMBER_ADD | bool |
| updateMember | MEMBER_UPDATE | bool |
| findMemberById | MEMBER_VIEW | const Member* |
| findMemberByPhone | MEMBER_VIEW | const Member* |
| getMemberAt | MEMBER_VIEW | const Member* |
| getMemberCount | MEMBER_VIEW | int |

2 thuộc tính, 1 constructor, 8 hàm public, 0 hàm private. Thiếu quyền: requirePermission ném exception trước khi thao tác dữ liệu. Con trỏ trả về chỉ mượn, không được delete. Không tự động save hoặc reload.

## 4. Mã nguồn đã chốt

### 4.1. MemberService.h

```cpp
#ifndef MEMBER_SERVICE_H
#define MEMBER_SERVICE_H

#include <string>
#include "../data_access/IRepository.h"
#include "../models/Member.h"
#include "AuthService.h"

class MemberService
{
private:
    IRepository<Member>& memberRepo;
    AuthService& authService;

public:
    MemberService(IRepository<Member>& memberRepo,
                  AuthService& authService);

    // Quan ly du lieu
    bool load();
    bool save();

    // Quan ly thanh vien
    bool addMember(Member* member);
    bool updateMember(const Member& newValues);

    // Tra cuu thanh vien
    const Member* findMemberById(const std::string& memberId);
    const Member* findMemberByPhone(const std::string& phone);
    const Member* getMemberAt(int index);
    int getMemberCount();
};

#endif
```

### 4.2. MemberService.cpp

```cpp
#include "MemberService.h"
#include "../utils/IdGenerator.h"
#include <vector>
using namespace std;

MemberService::MemberService(IRepository<Member>& memberRepo,
                             AuthService& authService)
    : memberRepo(memberRepo),
      authService(authService)
{
}

bool MemberService::load()
{
    return memberRepo.load();
}

bool MemberService::save()
{
    return memberRepo.save();
}

bool MemberService::addMember(Member* member)
{
    authService.requirePermission("MEMBER_ADD");

    if (!memberRepo.isLoaded() || member == nullptr)
    {
        return false;
    }

    vector<string> existingIds;

    for (int i = 0; i < memberRepo.count(); i++)
    {
        const Member* current = memberRepo.getAt(i);
        if (current == nullptr)
        {
            return false;
        }
        existingIds.push_back(current->getMemberId());
    }

    string memberId = IdGenerator::generateNextId("MEM", existingIds);
    Member* newMember = new Member(memberId,
                                   member->getFullName(),
                                   member->getPhone(),
                                   member->getPoints());

    try
    {
        if (!memberRepo.add(newMember))
        {
            delete newMember;
            return false;
        }
    }
    catch (...)
    {
        delete newMember;
        throw;
    }
    return true;
}

bool MemberService::updateMember(const Member& newValues)
{
    authService.requirePermission("MEMBER_UPDATE");
    return memberRepo.update(newValues);
}

const Member* MemberService::findMemberById(const string& memberId)
{
    authService.requirePermission("MEMBER_VIEW");
    return memberRepo.findById(memberId);
}

const Member* MemberService::findMemberByPhone(const string& phone)
{
    authService.requirePermission("MEMBER_VIEW");

    for (int i = 0; i < memberRepo.count(); i++)
    {
        const Member* member = memberRepo.getAt(i);
        if (member != nullptr && member->getPhone() == phone)
        {
            return member;
        }
    }
    return nullptr;
}

const Member* MemberService::getMemberAt(int index)
{
    authService.requirePermission("MEMBER_VIEW");
    return memberRepo.getAt(index);
}

int MemberService::getMemberCount()
{
    authService.requirePermission("MEMBER_VIEW");
    return memberRepo.count();
}
```

## 5. Giải thích từng hàm

Header dùng include guard để tránh khai báo lặp, include trực tiếp các kiểu sử dụng. `std::` trong header tránh đưa namespace vào file khác. CPP dùng `using namespace std;` sau include. `vector` lưu danh sách ID tạm để sinh ID.

### 5.1. Constructor

**Tổng quan:** nhận hai tham chiếu đến repository và AuthService; không có giá trị trả về. Dependency phải tồn tại lâu hơn Service.

**Luồng hoạt động:**
```text
Nhan dependency -> Khoi tao memberRepo -> Khoi tao authService -> Ket thuc
```

**Giải thích chi tiết:** `MemberService::MemberService` định nghĩa constructor ngoài lớp. Danh sách khởi tạo sau dấu `:` gắn hai thuộc tính tham chiếu với đối tượng được truyền vào. Thân rỗng vì không cần load dữ liệu hay tạo đối tượng mới.

### 5.2. load()

**Tổng quan:** không tham số, trả kết quả nạp của Repository; exception được truyền tiếp.

```text
load -> memberRepo.load()
         |-- true  -> true
         |-- false -> false
         `-- exception -> truyen tiep
```

**Giải thích chi tiết:** `return memberRepo.load();` gọi qua interface, không tự đọc file. Repository hiện từ chối lần load thứ hai nếu đã loaded. Không kiểm tra permission vì đây là thao tác khởi tạo dữ liệu.

### 5.3. save()

**Tổng quan:** không tham số; trả kết quả lưu dữ liệu, không tự sửa dữ liệu.

```text
save -> memberRepo.save()
         |-- true -> true
         |-- false -> false
         `-- exception -> truyen tiep
```

**Giải thích chi tiết:** `return memberRepo.save();` chuyển trách nhiệm serialize và ghi TXT cho Repository. Không kiểm tra loaded lặp lại, không kiểm tra permission ở hàm hạ tầng này.

### 5.4. addMember()

**Tổng quan:** nhận con trỏ Member đầu vào, sinh ID MEM mới và thêm bản sao. Không đổi ID hoặc quyền sở hữu đối tượng đầu vào. Thành viên mới phải có 0 điểm theo hợp đồng MemberRepository.

```text
Kiem tra MEMBER_ADD
 |-- Thieu quyen -> exception
 `-- Co quyen
      |-- Chua load / nullptr -> false
      `-- Duyet danh sach ID
           |-- Phan tu nullptr -> false
           `-- Sinh ID -> Tao Member moi -> repo.add
                |-- false -> delete -> false
                |-- exception -> delete -> throw
                `-- true -> Repository so huu -> true
```

**Giải thích chi tiết:** `requirePermission` chạy trước mọi thao tác. `!isLoaded()` và `member == nullptr` ngăn xử lý dữ liệu chưa sẵn sàng và truy cập con trỏ rỗng. Toán tử `||` dừng khi điều kiện trái đúng. `vector<string> existingIds` chứa ID đang có. Vòng `for` duyệt từ 0 đến count - 1; `const Member* current` chỉ đọc đối tượng; kiểm tra nullptr trước `->getMemberId()`. `push_back` thêm ID vào vector. `generateNextId("MEM", existingIds)` sinh ID theo quy tắc hiện hành. `new Member` cấp phát bản sao với ID mới, họ tên, số điện thoại và điểm đầu vào; không ép điểm về 0 để Repository từ chối dữ liệu sai. `try` bao quanh thao tác chuyển quyền sở hữu. Nếu add trả false, Service delete bản sao; nếu ném exception, `catch (...)` giải phóng rồi `throw;` ném lại lỗi gốc. Nếu add thành công, Repository sở hữu bản sao và Service trả true. Không tự save. Hợp đồng add phải đảm bảo không nhận quyền sở hữu khi thất bại.

### 5.5. updateMember()

**Tổng quan:** nhận `const Member& newValues`, trả bool; chỉ cập nhật thông tin hợp lệ, không đổi điểm trực tiếp.

```text
MEMBER_UPDATE?
 |-- Khong -> exception
 `-- Co -> repo.update(newValues) -> ket qua / exception
```

**Giải thích chi tiết:** tham chiếu const tránh sao chép và không sửa đầu vào. `requirePermission` kiểm tra quyền. `memberRepo.update(newValues)` dùng ID để tìm đối tượng và giao kiểm tra loaded, dữ liệu, số điện thoại duy nhất và điểm không đổi cho concrete Repository. Repository cập nhật tại chỗ để giữ liên kết Invoice. Không tự save.

### 5.6. findMemberById()

**Tổng quan:** nhận ID bằng tham chiếu const; trả con trỏ chỉ đọc hoặc nullptr.

```text
MEMBER_VIEW? -> Khong: exception
             -> Co: repo.findById -> Member* / nullptr
```

**Giải thích chi tiết:** kiểm tra quyền trước truy vấn. `findById` là API chung của interface. Chuyển `Member*` sang `const Member*` để bên gọi không sửa trực tiếp thuộc tính. Con trỏ không chuyển quyền sở hữu.

### 5.7. findMemberByPhone()

**Tổng quan:** nhận số điện thoại, tìm khớp chính xác, trả con trỏ chỉ đọc hoặc nullptr.

```text
MEMBER_VIEW?
 |-- Khong -> exception
 `-- Co -> i = 0
      |-- i >= count -> nullptr
      `-- getAt(i)
           |-- Khac nullptr va phone trung -> return member
           `-- Khong trung / nullptr -> i++ -> lap lai
```

**Giải thích chi tiết:** interface không có `findByPhone`, nên duyệt bằng count và getAt. `const Member* member` là con trỏ mượn. Toán tử `&&` đánh giá ngắn mạch: chỉ gọi getPhone khi con trỏ khác nullptr. `==` so sánh chuỗi chính xác, không chuẩn hóa số điện thoại. `return member` kết thúc ngay khi tìm thấy; `return nullptr` khi duyệt hết. Không tạo chỉ mục riêng trong Service. Với danh sách liên kết, getAt có thể phải duyệt nhiều node, nên chi phí cả vòng lặp có thể là O(n²), chậm hơn chỉ mục map của concrete Repository; chấp nhận để giữ interface chung trong phạm vi hiện tại.

### 5.8. getMemberAt()

**Tổng quan:** nhận index kiểu int; trả Member chỉ đọc hoặc nullptr nếu vị trí không hợp lệ.

```text
MEMBER_VIEW? -> Khong: exception
             -> Co: repo.getAt(index) -> Member* / nullptr
```

**Giải thích chi tiết:** kiểm tra quyền rồi chuyển index cho Repository. Không kiểm tra lại biên vì Repository chịu trách nhiệm. Không delete con trỏ trả về.

### 5.9. getMemberCount()

**Tổng quan:** không tham số, trả số thành viên kiểu int.

```text
MEMBER_VIEW? -> Khong: exception
             -> Co: repo.count() -> int
```

**Giải thích chi tiết:** kiểm tra quyền trước khi trả số lượng từ interface. Không tự duyệt lại danh sách hoặc thay đổi dữ liệu.

## 6. Nội dung thay đổi và điểm cần chốt

- Thay dependency MemberRepository bằng IRepository<Member> và include trực tiếp Member.h.
- findMemberByPhone duyệt danh sách thay vì gọi API riêng của concrete Repository.
- Dùng const Member* cho đối tượng chỉ đọc trong addMember; đồng bộ format và thứ tự hàm.
- Giữ nguyên 8 API public, permission, nghiệp vụ điểm, quy tắc ID và quyền sở hữu.
- Không thêm API xóa hoặc chỉnh điểm. BookingService chịu trách nhiệm cộng/thu hồi điểm.
- Không sửa MemberRepository: Repository<T>::load() từ chối khi loaded; khi kiểm tra trùng phone thất bại, concrete Repository xóa map và danh sách. Không thêm clear trước load.
- Việc duyệt theo phone có đánh đổi hiệu năng; không mở rộng interface ở bước này.
- Các điều kiện điểm và dữ liệu dựa trên hợp đồng concrete Repository; fake Repository dùng trong test cần mô phỏng hợp đồng tương ứng.

## 7. Kiểm thử

Ngày 27/11/2026, `tests/test_member_service.cpp` biên dịch C++17 và chạy với TXT tạm riêng, Repository thực và `IRepository<Member>` giả lập: **Passed: 45, Failed: 0, Total: 45**. Hồi quy lần lượt: **StaffService 32/32 PASS, AuthService 31/31 PASS, MovieService 41/41 PASS, RoomService 43/43 PASS**. `.h/.cpp` khớp hai khối code Markdown 100%. Refactor giữ 8 hàm public, không có hàm private; DIP/DI qua `IRepository<Member>&`, Permission-only, không đổi nghiệp vụ quản lý thành viên và điểm. `findMemberByPhone()` duyệt qua `getAt()` của interface.

- [x] DIP/DI với Repository giả lập; gọi đủ 8 hàm public.
- [x] Load thành công, load lần hai trả false và chỉ mục phone vẫn đúng; file trùng phone ném exception, rollback danh sách/chỉ mục, sửa file rồi load lại thành công.
- [x] Save thành công và save khi chưa load trả false.
- [x] Add thành viên 0 điểm, sinh MEM theo ID lớn nhất, không đổi đầu vào; nullptr, chưa load, trùng phone, dữ liệu sai, điểm khác 0.
- [x] Mock `add()` trả false hoặc ném exception; không chuyển quyền sở hữu đầu vào khi thao tác thất bại.
- [x] Update hợp lệ, ID không tồn tại, trùng phone, dữ liệu sai, thay đổi điểm bị từ chối; giữ ID, địa chỉ và liên kết Invoice được nối trực tiếp.
- [x] Tìm theo ID/phone, phone không tồn tại, cập nhật phone rồi tìm lại, index âm/vượt biên và count có dữ liệu.
- [x] Chưa đăng nhập, ADMIN thiếu quyền, STAFF được cấp quyền riêng và tài khoản inactive không đăng nhập được; thiếu quyền không sửa dữ liệu.
- [x] Hồi quy StaffService, AuthService, MovieService, RoomService.
- [ ] Chưa kiểm thử trực tiếp lỗi ghi TXT; chưa dùng công cụ dò rò rỉ bộ nhớ để xác nhận giải phóng allocation nội bộ trên nhánh add thất bại/exception.

Dùng TXT tạm riêng. Biên dịch C++17 với một test main và các dependency cần thiết; không lấy các Service chưa refactor. Ví dụ PowerShell (điều chỉnh danh sách dependency theo source thực tế):

```powershell
$src = @(Get-ChildItem src/models,src/utils,src/data_access -Filter *.cpp -Recurse | ForEach-Object FullName)
g++ -std=c++17 tests/test_member_service.cpp src/business/MemberService.cpp src/business/AuthService.cpp $src -o test_member_service.exe
if ($LASTEXITCODE -eq 0) { .\test_member_service.exe }
```

Đã biên dịch C++17 và chạy bài test với các dependency cần thiết; các Service hồi quy cũng được biên dịch và chạy riêng. Chưa xác nhận biên dịch toàn ứng dụng hoặc hồi quy GUI.

## 8. Bàn giao

- [x] Rà soát mã nguồn tại effc5e8 và đối chiếu MemberRepository/Repository tại 7ab0bee.
- [x] Soạn thiết kế Markdown.
- [x] Review và CHỐT MARKDOWN.
- [x] Triển khai .h/.cpp khớp 100% Markdown.
- [x] Biên dịch C++17 và chạy `tests/test_member_service.cpp`: 45/45 PASS; hoàn thành MemberService trong phạm vi đã kiểm thử.
- [x] Hồi quy StaffService 32/32, AuthService 31/31, MovieService 41/41, RoomService 43/43 PASS.
- [x] Ghi nhận giới hạn kiểm thử và cập nhật tài liệu tiến độ.
- [ ] Commit/push do người thực hiện quyết định.

Không thay đổi mã nguồn, schema, nghiệp vụ hoặc các Service khác ngoài phạm vi refactor đã chốt.
