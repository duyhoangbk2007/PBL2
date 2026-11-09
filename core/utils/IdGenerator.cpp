#include "IdGenerator.h"

#include <fstream> // Đọc file bằng ifstream.

// Giới hạn bộ đếm bằng giá trị lớn nhất của long long 64 bit.
// Đuôi LL cho biết hằng số có kiểu long long.
static const long long GIOI_HAN_BO_DEM = 9223372036854775807LL;

// Các loại mã được xếp theo thứ tự chữ cái để nội dung file luôn có cùng thứ tự.
static const int SO_LOAI_MA = 7;
static const std::string CAC_LOAI_MA[SO_LOAI_MA] = {
    "INV", "MEM", "MOV", "ROM", "SHW", "TKT", "USR"
};

int IdGenerator::timViTriLoaiMa(const std::string& loaiMa) {
    for (int i = 0; i < SO_LOAI_MA; i++) {
        if (CAC_LOAI_MA[i] == loaiMa) {
            return i;
        }
    }
    return -1; // Không tìm thấy loại mã.
}

bool IdGenerator::docSoDuong(const std::string& chuoiSo, long long& ketQua) {
    if (chuoiSo.empty()) {
        return false;
    }

    long long so = 0;
    for (std::size_t i = 0; i < chuoiSo.size(); i++) {
        if (chuoiSo[i] < '0' || chuoiSo[i] > '9') {
            return false;
        }

        int chuSo = chuoiSo[i] - '0'; // Ví dụ: '5' - '0' = 5.
        // Kiểm tra trước khi nhân và cộng để không vượt giới hạn long long.
        if (so > (GIOI_HAN_BO_DEM - chuSo) / 10) {
            return false;
        }
        so = so * 10 + chuSo; // Ví dụ: đang có 12, đọc thêm 3 thì được 123.
    }

    if (so == 0) {
        return false;
    }
    ketQua = so;
    return true;
}

bool IdGenerator::taoMaTiepTheo(const std::string& loaiMa,
                              const std::string& duongDanBoDem,
                              std::string& maMoi,
                              std::string& noiDungBoDemMoi,
                              std::string& loi) {
    maMoi.clear();
    noiDungBoDemMoi.clear();
    loi.clear();

    // Bước 1: Kiểm tra loại mã và mở file bộ đếm.
    int viTriCanTao = timViTriLoaiMa(loaiMa);
    if (viTriCanTao == -1) {
        loi = "Loại mã không hợp lệ: " + loaiMa;
        return false;
    }

    std::ifstream tep(duongDanBoDem);
    if (!tep.is_open()) {
        loi = "Không mở được file sequences: " + duongDanBoDem;
        return false;
    }

    std::string dong;
    if (!std::getline(tep, dong) || (dong != "#schema=1" && dong != "#schema=1\r")) {
        loi = "File sequences thiếu dòng #schema=1.";
        return false;
    }

    // Bước 2: Đọc các dòng dạng MOV|12.
    // boDem[i] thuộc loại mã CAC_LOAI_MA[i]. Giá trị 0 nghĩa là chưa đọc loại đó.
    long long boDem[SO_LOAI_MA] = {};
    std::size_t soDong = 1;
    while (std::getline(tep, dong)) {
        soDong++;
        // Bỏ ký tự xuống dòng còn lại khi đọc file có kiểu xuống dòng Windows.
        if (!dong.empty() && dong.back() == '\r') {
            dong.pop_back();
        }
        if (dong.empty()) {
            continue;
        }

        std::size_t viTriDau = dong.find('|');
        if (viTriDau == std::string::npos) {
            loi = duongDanBoDem + ":" + std::to_string(soDong)
                + ": Dữ liệu không hợp lệ: " + dong;
            return false;
        }

        std::string loaiMaTrongDong = dong.substr(0, viTriDau);
        std::string chuoiSo = dong.substr(viTriDau + 1);
        int viTri = timViTriLoaiMa(loaiMaTrongDong);
        long long soTiepTheo = 0;

        if (viTri == -1 || !docSoDuong(chuoiSo, soTiepTheo)) {
            loi = duongDanBoDem + ":" + std::to_string(soDong)
                + ": Dữ liệu không hợp lệ: " + dong;
            return false;
        }
        if (boDem[viTri] != 0) {
            loi = duongDanBoDem + ":" + std::to_string(soDong)
                + ": Loại mã bị lặp: " + loaiMaTrongDong;
            return false;
        }
        boDem[viTri] = soTiepTheo;
    }

    if (!tep.eof()) {
        loi = "Lỗi khi đọc file sequences.";
        return false;
    }
    if (boDem[viTriCanTao] == 0) {
        loi = "Không tìm thấy bộ đếm: " + loaiMa;
        return false;
    }
    if (boDem[viTriCanTao] == GIOI_HAN_BO_DEM) {
        loi = "Bộ đếm đã đạt giới hạn: " + loaiMa;
        return false;
    }

    // Bước 3: Thêm số 0 ở đầu cho đủ ít nhất 6 chữ số. Ví dụ: 12 -> MOV000012.
    std::string phanSo = std::to_string(boDem[viTriCanTao]);
    while (phanSo.size() < 6) {
        phanSo = "0" + phanSo;
    }
    maMoi = loaiMa + phanSo;
    boDem[viTriCanTao]++;

    // Bước 4: Ghép nội dung bộ đếm mới, chỉ giữ các loại có trong file đã đọc.
    // Hàm chưa ghi file; TransactionManager sẽ lưu nội dung này cùng dữ liệu.
    noiDungBoDemMoi = "#schema=1\n";
    for (int i = 0; i < SO_LOAI_MA; i++) {
        if (boDem[i] != 0) {
            noiDungBoDemMoi += CAC_LOAI_MA[i] + "|" + std::to_string(boDem[i]) + "\n";
        }
    }
    return true;
}
