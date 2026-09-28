# Trie – Cây tiền tố

## 1. Tổng quan

Trie là cấu trúc dữ liệu dạng cây dùng để lưu và tìm kiếm chuỗi theo **tiền tố** (prefix). Những chuỗi có cùng phần đầu sẽ dùng chung một đoạn đường đi từ gốc, nhờ đó có thể tìm nhanh các tên bắt đầu bằng nội dung người dùng nhập.

Tài liệu này giải thích cách cài đặt trong [`core/data_structures/Trie.h`](../../core/data_structures/Trie.h). Bản trong `Test DS/Trie.h` có cấu trúc và API khác, không phải bản được mô tả ở đây.

Các thao tác được hỗ trợ:

| Hàm | Mục đích |
|---|---|
| `makeTrieNode` | Cấp phát và khởi tạo một nút |
| `trieIndex` | Chuyển một byte thành chỉ số nhánh, đổi chữ hoa ASCII thành chữ thường |
| `addString` | Thêm tên vào cây |
| `searchString` | Kiểm tra một tên đầy đủ có tồn tại không |
| `removeString` | Xóa tên bằng cách bỏ đánh dấu kết thúc |
| `collectWords` | Duyệt cây con để thu thập tên |
| `autoComplete` | Tìm các tên bắt đầu bằng tiền tố, có giới hạn số kết quả |
| `clearTrie` | Giải phóng toàn bộ cây |

Cách cài đặt dùng `struct` chứa dữ liệu, các hàm thao tác nằm bên ngoài, mảng con trỏ cố định và `new`/`delete`. Tên được lưu bằng `std::string`; không dùng STL container để quản lý các nhánh.

## 2. Nguyên lý hoạt động

### 2.1. Đường đi biểu diễn chuỗi

Nút gốc biểu diễn tiền tố rỗng. Mỗi cạnh tương ứng với một byte của chuỗi sau khi chuẩn hóa chữ hoa ASCII. Đi từ gốc qua các cạnh sẽ tạo thành một tiền tố; nút có `isEndOfWord = true` đánh dấu một tên đã được thêm đầy đủ.

Ví dụ thêm `An`, `Anh` và `Binh`:

```text
(root)
├── a
│   └── n  [kết thúc: "An"]
│       └── h  [kết thúc: "Anh"]
└── b
    └── i
        └── n
            └── h  [kết thúc: "Binh"]
```

`An` và `Anh` dùng chung đường đi `a → n`. Một nút có thể vừa kết thúc một tên, vừa có các nút con để biểu diễn tên dài hơn.

### 2.2. Tìm tên đầy đủ và tìm tiền tố

- `searchString(root, "A")` trả về `false`: đường đi tồn tại nhưng nút `a` chưa được đánh dấu là tên đầy đủ.
- `searchString(root, "AN")` trả về `true`: chữ hoa ASCII được đổi thành chữ thường khi duyệt cây.
- `autoComplete(root, "A", results, 5)` tìm đến nút `a`, rồi thu thập được `An` và `Anh` trong cây con.

Vì vậy, **đường đi tồn tại không đồng nghĩa với tên đầy đủ tồn tại**. Đây là lý do cần trường `isEndOfWord`.

## 3. Cấu trúc của một nút

```cpp
const int TRIE_ALPHABET_SIZE = 256;

struct TrieNode {
    TrieNode *child[TRIE_ALPHABET_SIZE];
    bool isEndOfWord;
    std::string originalName;
};
```

| Thành phần | Ý nghĩa |
|---|---|
| `child[256]` | Mỗi ô là con trỏ đến nút con ứng với một giá trị byte từ 0 đến 255; `nullptr` nghĩa là chưa có nhánh |
| `isEndOfWord` | Cho biết nút này có kết thúc một tên đang được lưu hay không |
| `originalName` | Lưu tên gốc tại nút kết thúc để trả kết quả đúng cách viết khi thêm |

Nút không cần lưu riêng byte của nó: chỉ số trong mảng `child` của nút cha đã xác định byte trên cạnh đi tới nó.

### 3.1. Khởi tạo nút và con trỏ gốc

`makeTrieNode()` cấp phát một `TrieNode`, đặt toàn bộ con trỏ con thành `nullptr`, `isEndOfWord` thành `false` và `originalName` thành chuỗi rỗng.

Người gọi có thể bắt đầu bằng:

```cpp
TrieNode *root = nullptr;
```

`addString(&root, name)` tự tạo gốc khi thêm tên không rỗng đầu tiên. Hàm nhận `TrieNode **` để có thể thay đổi chính con trỏ `root` của người gọi. Tương tự, `clearTrie(&root)` giải phóng cây rồi đặt `root = nullptr`.

Khi gọi hai hàm này, phải truyền địa chỉ của một biến con trỏ hợp lệ, chẳng hạn `&root`; không truyền trực tiếp `nullptr` làm đối số `TrieNode **`.

### 3.2. Ánh xạ byte và xử lý UTF-8

```cpp
int trieIndex(unsigned char c) {
    if (c >= 'A' && c <= 'Z') return c + ('a' - 'A');
    return c;
}
```

Mỗi chuỗi được duyệt bằng `unsigned char`, giúp chỉ số luôn nằm trong khoảng `0..255`, tránh chỉ số âm khi gặp byte có giá trị lớn hơn 127.

Cây lưu được các byte UTF-8, nhưng **mỗi mức tương ứng với một byte, không nhất thiết là một ký tự tiếng Việt**. Một ký tự UTF-8 gồm nhiều byte sẽ tạo nhiều cạnh liên tiếp.

Việc không phân biệt hoa/thường chỉ áp dụng cho `A..Z` và `a..z`. Mã hiện tại không chuyển chữ hoa Unicode như `Đ` thành `đ`, không bỏ dấu và không chuẩn hóa Unicode. Hai cách mã hóa Unicode khác nhau dù hiển thị giống nhau vẫn có thể tạo đường đi khác nhau.

## 4. Giải thích các thao tác

### 4.1. Thêm tên – `addString`

```cpp
void addString(TrieNode **root, const std::string &name);
```

1. Nếu tên rỗng, kết thúc ngay.
2. Nếu chưa có gốc, tạo nút gốc.
3. Với mỗi byte của tên, tính chỉ số bằng `trieIndex`.
4. Nếu nhánh tương ứng chưa tồn tại, tạo nút con; sau đó đi xuống nút con.
5. Ở nút cuối, đặt `isEndOfWord = true` và lưu `originalName = name`.

Thêm `Anh` sau `An` chỉ cần tạo thêm nhánh `h`. Thêm lại cùng tên không tạo một bản ghi riêng. Nếu thêm `An` rồi `AN`, cả hai dùng chung đường đi và tên trả về sau đó là `AN`, do `originalName` được ghi đè bằng lần thêm cuối.

### 4.2. Tìm chính xác – `searchString`

```cpp
bool searchString(TrieNode *root, const std::string &name);
```

Hàm trả về `false` nếu cây chưa có gốc hoặc tên rỗng. Nếu đầu vào hợp lệ, hàm đi theo từng byte; thiếu bất kỳ nhánh nào thì trả về `false`. Khi đi hết tên, kết quả là giá trị `isEndOfWord` của nút cuối.

Ví dụ cây chỉ có `Anh`: tìm `An` vẫn trả về `false`, dù đường đi `a → n` tồn tại.

### 4.3. Xóa tên – `removeString`

```cpp
bool removeString(TrieNode *root, const std::string &name);
```

Hàm tìm đến nút cuối giống `searchString`. Nếu không có tên đầy đủ tương ứng, trả về `false`. Nếu có, hàm đặt:

```cpp
temp->isEndOfWord = false;
temp->originalName = "";
```

Sau đó trả về `true`. Đây là **xóa logic**: các nút và nhánh vẫn được giữ lại, kể cả nhánh không còn tên nào sử dụng. Bộ nhớ của các nút chỉ được giải phóng khi gọi `clearTrie`.

Ví dụ xóa `An` khỏi cây có `An` và `Anh` sẽ bỏ đánh dấu tại nút `n`; tên `Anh` vẫn tồn tại. Thêm lại `An` có thể tái sử dụng các nút cũ.

### 4.4. Thu thập tên – `collectWords`

```cpp
void collectWords(TrieNode *root, std::string results[],
                  int limit, int &count);
```

Hàm duyệt sâu (DFS) từ nút được truyền vào:

1. Dừng nếu nút hoặc mảng kết quả là `nullptr`, hoặc `count >= limit`.
2. Nếu nút kết thúc một tên, chép `originalName` vào `results[count]`, rồi tăng `count`.
3. Duyệt các nhánh từ chỉ số 0 đến 255 và đệ quy xuống nhánh tồn tại, dừng khi đủ kết quả.

`count` được truyền bằng tham chiếu để tất cả lần gọi đệ quy cùng cập nhật một số lượng kết quả. Khi gọi trực tiếp, người gọi cần khởi tạo `count` hợp lệ, thường là 0, và bảo đảm mảng có đủ chỗ.

Kết quả theo thứ tự byte của đường đi đã chuẩn hóa ASCII, với tên tại nút hiện tại được lấy trước các tên dài hơn. Đây không phải thứ tự thêm vào, thứ tự theo độ phổ biến hay thứ tự từ điển tiếng Việt.

### 4.5. Gợi ý theo tiền tố – `autoComplete`

```cpp
int autoComplete(TrieNode *root, const std::string &prefix,
                 std::string results[], int limit);
```

Hàm gồm hai giai đoạn:

1. Đi từ gốc theo tiền tố. Nếu thiếu nhánh, trả về 0.
2. Khởi tạo `count = 0`, gọi `collectWords` tại nút cuối của tiền tố và trả về số tên đã ghi.

Các trường hợp cần chú ý:

- Gốc hoặc `results` là `nullptr`, hoặc `limit <= 0`: trả về 0.
- Tiền tố rỗng: thu thập từ gốc, tức có thể gợi ý mọi tên còn được đánh dấu trong cây, tối đa `limit` tên.
- Tiền tố cũng là tên đầy đủ: tên đó được đưa vào kết quả trước các tên dài hơn.
- `results` phải có ít nhất `limit` phần tử. Chỉ các phần tử từ 0 đến số lượng trả về trừ 1 là kết quả của lần gọi này; phần còn lại không được hàm xóa.

### 4.6. Giải phóng cây – `clearTrie`

```cpp
void clearTrie(TrieNode **root);
```

Hàm đệ quy giải phóng từng cây con, sau đó `delete` nút hiện tại và đặt con trỏ tương ứng thành `nullptr`. Cách duyệt con trước, cha sau bảo đảm vẫn truy cập được các nhánh để giải phóng chúng.

Sau `clearTrie(&root)`, biến `root` của người gọi là `nullptr`. Những con trỏ khác trỏ tới các nút cũ không còn hợp lệ.

## 5. Độ phức tạp

Quy ước:

- `L`: độ dài tên tính bằng byte; `P`: độ dài tiền tố tính bằng byte.
- `N`: tổng số nút đã cấp phát, kể cả các nút còn lại sau xóa logic.
- `V`: số nút thực sự được thăm trong giai đoạn thu thập kết quả.
- `S`: tổng số byte của các tên được chép vào mảng kết quả.
- `H`: chiều cao cây hoặc cây con được duyệt, tính theo số cạnh.

| Thao tác | Thời gian | Bộ nhớ phụ / cấp phát |
|---|---|---|
| `makeTrieNode` | O(256), xem là O(1) với bảng chữ cái cố định | Một nút có 256 con trỏ |
| `addString` | O(L) với bảng chữ cái cố định | Tối đa L nút mới, cộng nút gốc nếu chưa có; lưu bản sao tên ở nút cuối |
| `searchString` | O(L) | O(1) |
| `removeString` | O(L) | O(1); không thu hồi nút |
| `collectWords` | O(256V + S), tương đương O(V + S) khi xem 256 là hằng số | O(H + 1) ngăn xếp đệ quy, ngoài mảng kết quả |
| `autoComplete` | O(P + 256V + S) | Ngăn xếp của `collectWords`, ngoài mảng kết quả |
| `clearTrie` | O(256N), tương đương O(N) với bảng chữ cái cố định | O(H + 1) ngăn xếp đệ quy |

Không thể khẳng định gợi ý luôn chỉ tốn O(P + số kết quả): hàm còn phải duyệt các nút trung gian và có thể đi qua nhiều nhánh đã bị xóa logic trước khi tìm đủ tên.

Tổng bộ nhớ lưu cây gồm mảng 256 con trỏ ở mỗi nút, cờ kết thúc, đối tượng chuỗi và vùng nhớ chứa tên gốc. Riêng mảng con trỏ chiếm 2048 byte mỗi nút nếu một con trỏ có kích thước 8 byte. Vì vậy, dù truy cập nhánh nhanh, cách cài đặt này tốn bộ nhớ khi phần lớn ô `child` là `nullptr`.

## 6. Ví dụ sử dụng

```cpp
#include <iostream>
#include "core/data_structures/Trie.h"

int main() {
    TrieNode *root = nullptr;

    addString(&root, "An");
    addString(&root, "Anh");
    addString(&root, "Binh");

    std::cout << std::boolalpha;
    std::cout << searchString(root, "AN") << '\n'; // true
    std::cout << searchString(root, "A") << '\n';  // false

    const int LIMIT = 5;
    std::string results[LIMIT];
    int count = autoComplete(root, "a", results, LIMIT);
    for (int i = 0; i < count; i++) {
        std::cout << results[i] << '\n'; // An, sau do Anh
    }

    removeString(root, "An");
    std::cout << searchString(root, "An") << '\n';  // false
    std::cout << searchString(root, "Anh") << '\n'; // true

    clearTrie(&root);
    return 0;
}
```

Đường dẫn `#include` trong ví dụ giả định file chương trình được đặt ở thư mục gốc dự án.

## 7. Ưu điểm và giới hạn của bản cài đặt

- Tìm tên theo độ dài chuỗi, không phải so sánh lần lượt với mọi tên trong danh sách.
- Các tên có tiền tố chung dùng chung nút; gợi ý chỉ cần duyệt cây con của tiền tố.
- Giữ cách viết gốc để hiển thị, nhưng chỉ lưu một tên cho mỗi đường đi; không có danh sách ID để phân biệt nhiều đối tượng trùng tên.
- Mỗi nút có 256 con trỏ nên chi phí bộ nhớ lớn. Xóa logic nhiều lần không làm giảm số nút đã cấp phát.
- Hỗ trợ lưu byte UTF-8 nhưng chưa hỗ trợ tìm không dấu, chuẩn hóa Unicode hoặc chuyển hoa/thường đầy đủ cho tiếng Việt.
- Gợi ý chỉ khớp phần đầu chuỗi; không tìm chuỗi con ở giữa và không sửa lỗi gõ.
- Thu thập tên và giải phóng cây dùng đệ quy, nên chuỗi quá dài làm tăng độ sâu ngăn xếp.

Trie phù hợp khi cần tìm tên chính xác và gợi ý theo tiền tố. Khi cần tra cứu đối tượng cụ thể từ tên, ứng dụng cần thêm cơ chế ánh xạ tên sang dữ liệu hoặc ID vì nút hiện tại chỉ lưu `originalName`.
