#include <chrono>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <type_traits>
#include "../src/business/PricingService.h"
#include "../src/data_access/UserRepository.h"

using namespace std;
namespace fs = std::filesystem;

int passed = 0;
int failed = 0;

void check(bool condition, const string& name)
{
    cout << (condition ? "[PASS] " : "[FAIL] ") << name << endl;
    (condition ? passed : failed)++;
}

template <typename F>
bool throwsError(F action)
{
    try { action(); }
    catch (const exception&) { return true; }
    return false;
}

void writeFile(const fs::path& path, const string& data)
{
    ofstream out(path, ios::binary | ios::trunc);
    if (!out || !(out << data)) throw runtime_error("Khong ghi duoc TXT test");
}

struct TestFiles
{
    fs::path dir;
    TestFiles()
    {
        dir = fs::temp_directory_path() /
              ("pbl2_pricing_service_" +
               to_string(chrono::steady_clock::now().time_since_epoch().count()));
        fs::create_directories(dir);
    }
    ~TestFiles()
    {
        error_code error;
        fs::remove_all(dir, error);
    }
    fs::path path(const string& name) const { return dir / name; }
};

const string PRICES =
    "STANDARD|70000\nVIP|100000\nCOUPLE|160000\n"
    "STUDENT_DISCOUNT|10\nMEMBER_DISCOUNT|5\n";
const string USERS =
    "USR0001|admin|secret|Admin|ADMIN|1\n"
    "USR0002|staff|secret|Staff|STAFF|1\n"
    "USR0003|off|secret|Inactive|STAFF|0\n";
const string RIGHTS =
    "ADMIN|PRICING_VIEW\nADMIN|PRICING_UPDATE\n"
    "STAFF|PRICING_VIEW\nSTAFF|PRICING_UPDATE\n";

struct Fixture
{
    TestFiles files;
    PricingRepository pricing;
    UserRepository concreteUsers;
    IRepository<User>& users;
    RolePermissionRepository permissions;
    AuthService auth;
    PricingService service;

    Fixture(const string& prices = PRICES, const string& rights = RIGHTS,
            bool createPricing = true)
        : pricing(files.path("pricing.txt").string()),
          concreteUsers(files.path("users.txt").string()), users(concreteUsers),
          permissions(files.path("rights.txt").string()),
          auth(users, permissions), service(pricing, auth)
    {
        if (createPricing) writeFile(files.path("pricing.txt"), prices);
        writeFile(files.path("users.txt"), USERS);
        writeFile(files.path("rights.txt"), rights);
        if (!permissions.load()) throw runtime_error("Khong nap duoc quyen test");
    }
    void login(const string& name = "admin")
    {
        if (!users.isLoaded() && !users.load())
            throw runtime_error("Khong nap duoc user test");
        if (!auth.login(name, "secret")) throw runtime_error("Login test that bai");
    }
};

void testLoadAndPermissions()
{
    static_assert(is_constructible<PricingService, PricingRepository&, AuthService&>::value,
                  "PricingService phai nhan hai dependency qua constructor");
    Fixture f;
    check(f.service.load(), "Load bang gia 5 khoa hop le");
    check(throwsError([&] { f.service.getPricing(); }),
          "Chua dang nhap: PRICING_VIEW bi tu choi");
    check(throwsError([&] { f.service.updateSeatPrice(SeatType::VIP, 1); }),
          "Chua dang nhap: PRICING_UPDATE bi tu choi");
    f.login("staff");
    check(f.service.getPricing() != nullptr, "STAFF co PRICING_VIEW");
    check(!f.auth.login("off", "secret"), "Tai khoan inactive khong dang nhap duoc");

    Fixture missing(PRICES, RIGHTS, false);
    missing.login();
    check(!missing.service.load() && missing.service.getPricing() == nullptr,
          "Thieu file: load false, getPricing nullptr");
    check(!missing.service.updateSeatPrice(SeatType::STANDARD, 80000) &&
          !missing.service.updateDiscountRate(DiscountType::MEMBER, 8),
          "Chua load: hai ham update false");
    check(throwsError([&] { missing.service.getBasePrice(SeatType::STANDARD); }) &&
          throwsError([&] { missing.service.calculateDiscount(100, TicketType::REGULAR, false); }),
          "Chua load: gia goc va discount nem exception");
    check(missing.service.calculatePaidAmount(100, 0) == 100 &&
          missing.service.calculatePoints(10000, true) == 1,
          "Paid amount va points khong can loaded");

    Fixture invalid("STANDARD|abc\nVIP|100000\nCOUPLE|160000\n"
                    "STUDENT_DISCOUNT|10\nMEMBER_DISCOUNT|5\n");
    check(throwsError([&] { invalid.service.load(); }) &&
          invalid.pricing.getPricing() == nullptr,
          "TXT sai dinh dang bi tu choi");
    writeFile(invalid.files.path("pricing.txt"), PRICES);
    check(invalid.service.load(), "Sua TXT va load lai thanh cong");
    writeFile(invalid.files.path("pricing.txt"),
              "STANDARD|70000\nSTANDARD|100000\nCOUPLE|160000\n"
              "STUDENT_DISCOUNT|10\nMEMBER_DISCOUNT|5\n");
    check(throwsError([&] { invalid.service.load(); }) &&
          invalid.pricing.getPricing() == nullptr,
          "Reload trung khoa vo hieu hoa cau hinh");

    Fixture noView(PRICES, "ADMIN|PRICING_UPDATE\nSTAFF|PRICING_UPDATE\n");
    noView.login();
    check(noView.service.load(), "ADMIN thieu VIEW van load duoc");
    check(throwsError([&] { noView.service.getPricing(); }) &&
          throwsError([&] { noView.service.calculatePaidAmount(100, 0); }) &&
          throwsError([&] { noView.service.calculatePoints(100, true); }),
          "ADMIN khong mac nhien co PRICING_VIEW");
    check(noView.service.updateSeatPrice(SeatType::VIP, 110000),
          "PRICING_UPDATE doc lap voi VIEW");

    Fixture noUpdate(PRICES, "STAFF|PRICING_VIEW\nADMIN|TICKET_SELL\n");
    noUpdate.login("staff");
    check(noUpdate.service.load() && noUpdate.service.getPricing() != nullptr,
          "STAFF chi co PRICING_VIEW");
    check(throwsError([&] { noUpdate.service.updateSeatPrice(SeatType::VIP, 1); }) &&
          throwsError([&] { noUpdate.service.updateDiscountRate(DiscountType::MEMBER, 1); }),
          "Thieu PRICING_UPDATE: hai ham update bi tu choi");
    noUpdate.auth.logout();
    noUpdate.login("admin");
    check(throwsError([&] { noUpdate.service.calculateDiscount(100, TicketType::REGULAR, false); }),
          "Chi TICKET_SELL khong du de tinh gia");
}

void testUpdatesAndPrices()
{
    Fixture f;
    f.login();
    check(f.service.load(), "Load truoc cap nhat");
    const Pricing* address = f.service.getPricing();
    check(f.service.updateSeatPrice(SeatType::STANDARD, 80000), "Update STANDARD");
    check(f.service.updateSeatPrice(SeatType::VIP, 120000), "Update VIP");
    check(f.service.updateSeatPrice(SeatType::COUPLE, 180000), "Update COUPLE");
    check(f.service.updateDiscountRate(DiscountType::STUDENT, 20), "Update STUDENT");
    check(f.service.updateDiscountRate(DiscountType::MEMBER, 15), "Update MEMBER");
    const Pricing* current = f.service.getPricing();
    check(current == address && current->getStandardPrice() == 80000 &&
          current->getVipPrice() == 120000 && current->getCouplePrice() == 180000 &&
          current->getStudentDiscount() == 20 && current->getMemberDiscount() == 15,
          "Update giu dia chi va cac thuoc tinh con lai");
    PricingRepository reloaded(f.files.path("pricing.txt").string());
    check(reloaded.load() && reloaded.getPricing()->getVipPrice() == 120000 &&
          reloaded.getPricing()->getMemberDiscount() == 15,
          "Update thanh cong da ghi TXT, Repository moi load lai duoc");
    check(f.service.getBasePrice(SeatType::STANDARD) == 80000 &&
          f.service.getBasePrice(SeatType::VIP) == 120000 &&
          f.service.getBasePrice(SeatType::COUPLE) == 180000,
          "getBasePrice du ba loai ghe");

    const double nan = numeric_limits<double>::quiet_NaN();
    const double inf = numeric_limits<double>::infinity();
    for (double value : {0.0, -1.0, nan, inf})
        check(!f.service.updateSeatPrice(SeatType::STANDARD, value),
              "Tu choi gia ghe 0/am/NaN/infinity");
    for (double value : {-1.0, 101.0, nan, inf})
        check(!f.service.updateDiscountRate(DiscountType::STUDENT, value),
              "Tu choi ty le ngoai mien/NaN/infinity");
    check(!f.service.updateSeatPrice(static_cast<SeatType>(99), 90000) &&
          !f.service.updateDiscountRate(static_cast<DiscountType>(99), 50),
          "Tu choi enum khong hop le khi update");
    check(current->getStandardPrice() == 80000 &&
          current->getStudentDiscount() == 20 &&
          reloaded.getPricing()->getStandardPrice() == 80000,
          "Update that bai khong doi cau hinh RAM");
    check(throwsError([&] { f.service.getBasePrice(static_cast<SeatType>(99)); }),
          "getBasePrice tu choi enum sai");
    check(f.service.updateDiscountRate(DiscountType::STUDENT, 0) &&
          f.service.updateDiscountRate(DiscountType::MEMBER, 100),
          "Ty le bien 0 va 100 hop le");
    const fs::path pricingPath = f.files.path("pricing.txt");
    const fs::path backupPath = f.files.path("pricing_backup.txt");
    fs::rename(pricingPath, backupPath);
    fs::create_directory(pricingPath);
    check(!f.service.updateSeatPrice(SeatType::STANDARD, 90000) &&
          f.service.getPricing()->getStandardPrice() == 80000,
          "Loi ghi TXT: update false, cau hinh RAM khong doi");
    fs::remove(pricingPath);
    fs::rename(backupPath, pricingPath);
}

void testCalculations()
{
    Fixture f;
    f.login();
    check(f.service.load(), "Load truoc tinh discount");
    check(f.service.calculateDiscount(100000, TicketType::REGULAR, false) == 0,
          "Ve thuong khong member: khong giam");
    check(f.service.calculateDiscount(100000, TicketType::STUDENT, false) == 10000,
          "Ve hoc sinh: giam theo STUDENT");
    check(f.service.calculateDiscount(100000, TicketType::REGULAR, true) == 5000,
          "Ve thuong co member: giam theo MEMBER");
    check(f.service.calculateDiscount(100000, TicketType::STUDENT, true) == 10000,
          "STUDENT cao hon MEMBER: khong cong don");
    check(f.service.updateDiscountRate(DiscountType::MEMBER, 20) &&
          f.service.calculateDiscount(100000, TicketType::STUDENT, true) == 20000,
          "MEMBER cao hon STUDENT: chon muc cao nhat");
    check(f.service.updateDiscountRate(DiscountType::STUDENT, 20) &&
          f.service.calculateDiscount(100000, TicketType::STUDENT, true) == 20000,
          "Hai ty le bang nhau: khong cong don");
    check(throwsError([&] { f.service.calculateDiscount(100, static_cast<TicketType>(99), false); }),
          "Discount tu choi TicketType sai");
    for (double value : {0.0, -1.0, numeric_limits<double>::quiet_NaN(),
                         numeric_limits<double>::infinity()})
        check(throwsError([&] { f.service.calculateDiscount(value, TicketType::REGULAR, false); }),
              "Discount tu choi basePrice khong hop le");
    const double huge = numeric_limits<double>::max();
    check(f.service.updateDiscountRate(DiscountType::STUDENT, 50) &&
          isfinite(f.service.calculateDiscount(huge, TicketType::STUDENT, false)) &&
          f.service.calculateDiscount(huge, TicketType::STUDENT, false) == huge * 0.5,
          "Gia rat lon: chia rate truoc khi nhan, khong tran trung gian");

    check(f.service.calculatePaidAmount(100, 0) == 100 &&
          f.service.calculatePaidAmount(100, 100) == 0,
          "Tien giam 0% va 100%");
    check(f.service.calculatePaidAmount(100.5, 0.25) == 100.25,
          "Tien thuc tra giu phan thap phan");
    for (double amount : {-1.0, 101.0, numeric_limits<double>::quiet_NaN(),
                          numeric_limits<double>::infinity()})
        check(throwsError([&] { f.service.calculatePaidAmount(100, amount); }),
              "Paid amount tu choi tien giam khong hop le");
    check(throwsError([&] { f.service.calculatePaidAmount(0, 0); }) &&
          throwsError([&] { f.service.calculatePaidAmount(huge, huge); }) == false,
          "Paid amount tu choi base 0, cho phep gia huu han lon");

    check(f.service.calculatePoints(9999, true) == 0 &&
          f.service.calculatePoints(10000, true) == 1 &&
          f.service.calculatePoints(19999, true) == 1,
          "Diem tinh tung ve va lam tron xuong");
    check(f.service.calculatePoints(100000, false) == 0 &&
          f.service.calculatePoints(0, true) == 0,
          "Khong member va thanh toan 0 cho 0 diem");
    const double maxPaid = static_cast<double>(numeric_limits<int>::max()) * 10000.0;
    check(f.service.calculatePoints(maxPaid, true) == numeric_limits<int>::max(),
          "Diem tai gioi han int");
    check(throwsError([&] { f.service.calculatePoints(maxPaid + 10000, true); }),
          "Diem vuot gioi han int bi tu choi");
    for (double amount : {-1.0, numeric_limits<double>::quiet_NaN(),
                          numeric_limits<double>::infinity()})
        check(throwsError([&] { f.service.calculatePoints(amount, false); }),
              "Points tu choi dau vao sai ke ca khong member");
}

int main()
{
    try
    {
        testLoadAndPermissions();
        testUpdatesAndPrices();
        testCalculations();
    }
    catch (const exception& error)
    {
        check(false, string("Ngoai le ngoai du kien: ") + error.what());
    }
    cout << "\nPassed: " << passed << ", Failed: " << failed
         << ", Total: " << passed + failed << endl;
    return failed == 0 ? 0 : 1;
}
