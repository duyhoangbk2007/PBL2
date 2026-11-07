#ifndef PBL2_ID_GENERATOR_H
#define PBL2_ID_GENERATOR_H

#include <string>

class IdGenerator {
public:
    // Tạo mã và nội dung sequences mới, chưa ghi xuống file.
    // TransactionManager sẽ lưu updatedSequences cùng dữ liệu nghiệp vụ.
    // Mỗi lần gọi đọc lại file; chỉ cấp tiếp sau khi commit thành công.
    static bool generateNextId(const std::string& entity,
                               const std::string& sequencesPath,
                               std::string& id,
                               std::string& updatedSequences,
                               std::string& error);
};

#endif
