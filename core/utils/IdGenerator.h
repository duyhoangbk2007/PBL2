#ifndef PBL2_ID_GENERATOR_H
#define PBL2_ID_GENERATOR_H

#include <string>

class IdGenerator {
private:
    // Trả về vị trí trong mảng loại mã, hoặc -1 nếu không tìm thấy.
    static int timViTriLoaiMa(const std::string& loaiMa);
    static bool docSoDuong(const std::string& chuoiSo, long long& ketQua);

public:
    // Tạo mã và nội dung bộ đếm mới, chưa ghi xuống file.
    // Trả true khi thành công; trả false và ghi thông báo vào loi khi thất bại.
    // TransactionManager sẽ lưu noiDungBoDemMoi cùng dữ liệu nghiệp vụ.
    // Mỗi lần gọi đọc lại file; chỉ cấp tiếp sau khi commit thành công.
    static bool taoMaTiepTheo(const std::string& loaiMa,
                             const std::string& duongDanBoDem,
                             std::string& maMoi,
                             std::string& noiDungBoDemMoi,
                             std::string& loi);
};

#endif
