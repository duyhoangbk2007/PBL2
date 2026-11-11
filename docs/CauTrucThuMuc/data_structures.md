# `src/data_structures/` — cấu trúc dữ liệu tự cài đặt

**Hiện trạng đến hết Bước 03:** thư mục có `DoublyLinkedList.h`. Đây là template C++ nên khai báo và định nghĩa nằm cùng file header, không có `DoublyLinkedList.cpp`. Chỉ danh sách liên kết đôi là cấu trúc bắt buộc tự cài; các cấu trúc khác có thể dùng STL khi chức năng cần.

## Ý nghĩa và trách nhiệm

`DoublyLinkedList<T>` giữ các node trong RAM. Mỗi `ListNode<T>` có `data`, `prev`, `next`; danh sách giữ `head`. Các thao tác đã có gồm `pushFront`, `pushBack`, `insert`, `popFront`, `popBack`, `erase`, `update`, `search`, `getAt`, `nodeCount`, `clear` và `show`. Danh sách không biết định dạng TXT, quyền người dùng hoặc quy tắc bán vé.

## Sườn code hiện có

Đây là hình dạng rút gọn của header, **không phải API mới**:

```cpp
template <typename T>
struct ListNode {
    T data;
    ListNode<T>* prev;
    ListNode<T>* next;
    // ...
};

template <typename T>
class DoublyLinkedList {
private:
    ListNode<T>* head;

public:
    DoublyLinkedList();
    ~DoublyLinkedList();
    void pushBack(const T& x);
    ListNode<T>* getAt(int k);
    int nodeCount() const;
    // ... các thao tác còn lại trong header
};
```

Danh sách sở hữu **node** nhưng không mặc nhiên sở hữu đối tượng được giữ qua `T*`. Vì lớp chưa định nghĩa cách sao chép an toàn, tránh sao chép danh sách theo giá trị. Chỉ số `getAt(k)` bắt đầu từ 0 và trả `nullptr` khi ngoài phạm vi. Xem [DLL.md](../DS/DLL.md) để biết các thao tác và độ phức tạp.

## Ranh giới sau Bước 03

Model hiện dùng danh sách để giữ một số quan hệ trong RAM. Việc Repository quản lý tập Model và tra cứu theo ID thuộc các bước chức năng sau; tài liệu này không xác nhận đã có kiểm thử hành vi đầy đủ cho DLL hoặc đã có mọi cấu trúc dữ liệu dự kiến.
