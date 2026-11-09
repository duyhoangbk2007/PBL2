#include "../core/utils/IdGenerator.h"

#include <cassert>
#include <climits>
#include <fstream>
#include <iostream>
#include <string>

void ghiFile(const std::string& duongDan, const std::string& noiDung) {
    std::ofstream tep(duongDan, std::ios::binary);
    tep << noiDung;
    tep.close();
    assert(tep.good());
}

void kiemTraLoi(const std::string& duongDan, const std::string& noiDung,
               const std::string& loaiMa) {
    ghiFile(duongDan, noiDung);
    std::string ma = "cu", noiDungMoi = "cu", loi;
    bool thanhCong = IdGenerator::taoMaTiepTheo(loaiMa, duongDan, ma, noiDungMoi, loi);
    assert(!thanhCong);
    assert(ma.empty() && noiDungMoi.empty() && !loi.empty());
}

int main(int argc, char* argv[]) {
    // Chạy với đường dẫn file tạm riêng, không dùng file dữ liệu của ứng dụng.
    assert(argc == 2);
    std::string duongDan = argv[1];
    std::string ma, noiDungMoi, loi;
    const std::string duLieu = "#schema=1\r\nUSR|7\r\n\r\nMOV|12\r\n";
    ghiFile(duongDan, duLieu);

    bool thanhCong = IdGenerator::taoMaTiepTheo("MOV", duongDan, ma, noiDungMoi, loi);
    assert(thanhCong && loi.empty());
    assert(ma == "MOV000012");
    assert(noiDungMoi == "#schema=1\nMOV|13\nUSR|7\n");

    // Hàm không tự ghi file: gọi lại vẫn lấy số 12.
    thanhCong = IdGenerator::taoMaTiepTheo("MOV", duongDan, ma, noiDungMoi, loi);
    assert(thanhCong && ma == "MOV000012");
    ghiFile(duongDan, noiDungMoi);
    thanhCong = IdGenerator::taoMaTiepTheo("MOV", duongDan, ma, noiDungMoi, loi);
    assert(thanhCong && ma == "MOV000013");

    const std::string soSai[] = {"", "0", "-1", "+1", " 1", "1 ", "1x", "1|2",
                                std::to_string(LLONG_MAX) + "0"};
    for (const std::string& so : soSai) {
        kiemTraLoi(duongDan, "#schema=1\nMOV|" + so + "\n", "MOV");
        assert(!IdGenerator::taoMaTiepTheo("MOV", duongDan, ma, noiDungMoi, loi));
        assert(loi.find(duongDan + ":2:") != std::string::npos);
    }
    kiemTraLoi(duongDan, "MOV|1\n", "MOV");
    kiemTraLoi(duongDan, "#schema=1\nMOV\n", "MOV");
    kiemTraLoi(duongDan, "#schema=1\nMOV|1\nMOV|2\n", "MOV");
    kiemTraLoi(duongDan, "#schema=1\nMOV|1\nABC|2\n", "MOV");
    kiemTraLoi(duongDan, "#schema=1\nUSR|1\n", "MOV");
    kiemTraLoi(duongDan, "#schema=1\nMOV|1\n", "ABC");
    kiemTraLoi(duongDan, "#schema=1\nMOV|" + std::to_string(LLONG_MAX) + "\n", "MOV");

    ghiFile(duongDan, "#schema=1\nMOV|" + std::to_string(LLONG_MAX - 1) + "\n");
    thanhCong = IdGenerator::taoMaTiepTheo("MOV", duongDan, ma, noiDungMoi, loi);
    assert(thanhCong && ma == "MOV" + std::to_string(LLONG_MAX - 1));
    assert(noiDungMoi == "#schema=1\nMOV|" + std::to_string(LLONG_MAX) + "\n");

    ghiFile(duongDan, "#schema=1\nMOV|000001\n");
    thanhCong = IdGenerator::taoMaTiepTheo("MOV", duongDan, ma, noiDungMoi, loi);
    assert(thanhCong && ma == "MOV000001");
    std::cout << "All id generator tests passed\n";
}
