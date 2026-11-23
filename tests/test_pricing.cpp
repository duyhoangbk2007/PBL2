
#include <iostream>
#include <fstream>
#include <cmath>
#include <stdexcept>
#include <cstdio>
#include <limits>

#include "../src/models/Pricing.h"
#include "../src/data_access/PricingRepository.h"
#include "../src/data_access/UserRepository.h"
#include "../src/data_access/RolePermissionRepository.h"
#include "../src/business/AuthService.h"
#include "../src/business/PricingService.h"

using namespace std;

int passed = 0;
int failed = 0;

// Kiem tra ket qua test
void check(bool condition, const string& name)
{
    if (condition)
    {
        passed++;
        cout << "[PASS] " << name << endl;
    }
    else
    {
        failed++;
        cout << "[FAIL] " << name << endl;
    }
}

// So sanh hai so thuc
bool equalDouble(double a, double b)
{
    return fabs(a - b) < 0.000001;
}

// Tao file du lieu test
void writeFile(const string& path, const string& content)
{
    ofstream out(path);

    if (!out.is_open())
    {
        throw runtime_error("Khong tao duoc file: " + path);
    }

    out << content;

    if (!out.good())
    {
        throw runtime_error("Khong ghi duoc file: " + path);
    }
}

// Kiem tra ham co nem runtime_error hay khong
bool throwsError(PricingService& service, bool update)
{
    try
    {
        if (update)
        {
            service.updateSeatPrice(SeatType::VIP, 120000);
        }
        else
        {
            service.getPricing();
        }
    }
    catch (const runtime_error&)
    {
        return true;
    }

    return false;
}

// ========================================
// TEST 1: PRICING MODEL
// ========================================
void testModel()
{
    cout << "\n===== TEST PRICING MODEL =====\n";

    Pricing pricing;

    check(equalDouble(pricing.getStandardPrice(), 70000), "Default STANDARD");
    check(equalDouble(pricing.getVipPrice(), 100000), "Default VIP");
    check(equalDouble(pricing.getCouplePrice(), 160000), "Default COUPLE");
    check(equalDouble(pricing.getStudentDiscount(), 10), "Default STUDENT");
    check(equalDouble(pricing.getMemberDiscount(), 5), "Default MEMBER");

    check(pricing.isValid(), "Default Pricing valid");

    check(!Pricing(0, 100000, 160000, 10, 5).isValid(), "Reject zero price");
    check(!Pricing(-1, 100000, 160000, 10, 5).isValid(), "Reject negative price");
    check(!Pricing(70000, 100000, 160000, -1, 5).isValid(), "Reject negative discount");
    check(!Pricing(70000, 100000, 160000, 10, 101).isValid(), "Reject discount over 100");
    check(Pricing(70000, 100000, 160000, 0, 100).isValid(), "Accept discount boundaries");
    const double nanValue = numeric_limits<double>::quiet_NaN();
    const double infinity = numeric_limits<double>::infinity();
    check(!Pricing(nanValue, 100000, 160000, 10, 5).isValid(), "Reject NaN standard price");
    check(!Pricing(70000, infinity, 160000, 10, 5).isValid(), "Reject Infinity VIP price");
    check(!Pricing(70000, 100000, -infinity, 10, 5).isValid(), "Reject negative Infinity COUPLE price");
    check(!Pricing(70000, 100000, 160000, nanValue, 5).isValid(), "Reject NaN student discount");
    check(!Pricing(70000, 100000, 160000, 10, infinity).isValid(), "Reject Infinity member discount");
}

// ========================================
// TEST 2: PRICING REPOSITORY
// ========================================
void testRepository(const string& path)
{
    cout << "\n===== TEST PRICING REPOSITORY =====\n";

    // Tao du lieu hop le truoc khi load
    writeFile(path,
        "STANDARD|70000\n"
        "VIP|100000\n"
        "COUPLE|160000\n"
        "STUDENT_DISCOUNT|10\n"
        "MEMBER_DISCOUNT|5\n"
    );

    PricingRepository repo(path);

    check(repo.getPricing() == nullptr, "Not loaded initially");
    check(!repo.isLoaded(), "Initial loaded false");

    check(repo.load(), "Load valid pricing");
    check(repo.isLoaded(), "Loaded true");

    const Pricing* pricing = repo.getPricing();

    check(pricing != nullptr, "Get loaded pricing");

    if (pricing != nullptr)
    {
        check(equalDouble(pricing->getStandardPrice(), 70000), "Read STANDARD price");
        check(equalDouble(pricing->getVipPrice(), 100000), "Read VIP price");
        check(equalDouble(pricing->getCouplePrice(), 160000), "Read COUPLE price");
        check(equalDouble(pricing->getStudentDiscount(), 10), "Read STUDENT discount");
        check(equalDouble(pricing->getMemberDiscount(), 5), "Read MEMBER discount");
    }

    // Cap nhat bang gia
    Pricing newPricing(70000, 120000, 160000, 15, 5);

    check(repo.update(newPricing), "Update pricing");
    PricingRepository beforeSave(path);
    check(beforeSave.load() &&
          equalDouble(repo.getPricing()->getVipPrice(), 120000) &&
          equalDouble(beforeSave.getPricing()->getVipPrice(), 100000) &&
          equalDouble(beforeSave.getPricing()->getStudentDiscount(), 10),
          "Update changes RAM without writing TXT");
    check(repo.save(), "Save pricing");

    // Kiem tra du lieu sau khi luu
    PricingRepository reloaded(path);

    check(reloaded.load(), "Reload saved pricing");

    const Pricing* saved = reloaded.getPricing();

    check(saved != nullptr && equalDouble(saved->getVipPrice(), 120000), "Persist VIP price");
    check(saved != nullptr && equalDouble(saved->getStudentDiscount(), 15), "Persist student discount");

    // Khong cho cap nhat du lieu sai
    check(!reloaded.update(Pricing(-1, 120000, 160000, 15, 5)), "Reject invalid update");
    const double nanValue = numeric_limits<double>::quiet_NaN();
    const double infinity = numeric_limits<double>::infinity();
    check(!reloaded.update(Pricing(nanValue, 120000, 160000, 15, 5)), "Reject NaN update");
    check(!reloaded.update(Pricing(70000, 120000, 160000, infinity, 5)), "Reject Infinity update");

    // Thieu truong du lieu
    writeFile(path,
        "STANDARD|70000\n"
        "VIP|100000\n"
    );

    check(!reloaded.load(), "Reject missing fields");

    // Gia tri khong phai so
    writeFile(path,
        "STANDARD|70000\n"
        "VIP|abc\n"
        "COUPLE|160000\n"
        "STUDENT_DISCOUNT|10\n"
        "MEMBER_DISCOUNT|5\n"
    );

    check(!reloaded.load(), "Reject invalid number");

    writeFile(path,
        "STANDARD|NaN\n"
        "VIP|100000\n"
        "COUPLE|160000\n"
        "STUDENT_DISCOUNT|10\n"
        "MEMBER_DISCOUNT|5\n"
    );
    check(!reloaded.load(), "Reject NaN in file");

    writeFile(path,
        "STANDARD|70000\n"
        "VIP|Infinity\n"
        "COUPLE|160000\n"
        "STUDENT_DISCOUNT|10\n"
        "MEMBER_DISCOUNT|5\n"
    );
    check(!reloaded.load(), "Reject Infinity in file");

    // Trung key
    writeFile(path,
        "STANDARD|70000\n"
        "VIP|100000\n"
        "COUPLE|160000\n"
        "STUDENT_DISCOUNT|10\n"
        "STUDENT_DISCOUNT|5\n"
    );
    check(!reloaded.load(), "Reject duplicate key");

    // ========================================
// TEST RELOAD THAT BAI
// ========================================

// Tao lai file hop le
writeFile(path,
    "STANDARD|70000\n"
    "VIP|120000\n"
    "COUPLE|160000\n"
    "STUDENT_DISCOUNT|15\n"
    "MEMBER_DISCOUNT|5\n"
);

// Load thanh cong
check(reloaded.load(), "Reload valid pricing before failure");

// Luu gia cu de so sanh
const Pricing* oldPricing = reloaded.getPricing();

check(oldPricing != nullptr, "Old pricing exists");

double oldVipPrice = oldPricing != nullptr
    ? oldPricing->getVipPrice()
    : 0;

// Ghi du lieu sai vao file
writeFile(path,
    "STANDARD|abc\n"
    "VIP|100000\n"
    "COUPLE|160000\n"
    "STUDENT_DISCOUNT|10\n"
    "MEMBER_DISCOUNT|5\n"
);

// Reload that bai
check(!reloaded.load(), "Reload invalid pricing fails");

// Trang thai cu van duoc giu
check(reloaded.isLoaded(), "Keep loaded state after failed reload");

// Bang gia cu van ton tai
const Pricing* currentPricing = reloaded.getPricing();

check(currentPricing != nullptr, "Keep old pricing after failed reload");

// Gia VIP khong thay doi
check(
    currentPricing != nullptr &&
    equalDouble(currentPricing->getVipPrice(), oldVipPrice),
    "Keep old VIP price after failed reload"
);

}

// ========================================
// TEST 3: PRICING SERVICE
// ========================================
void testService(const string& pricingPath, const string& usersPath, const string& permissionsPath)
{
    cout << "\n===== TEST PRICING SERVICE =====\n";

    // Tao bang gia
    writeFile(pricingPath,
        "STANDARD|70000\n"
        "VIP|100000\n"
        "COUPLE|160000\n"
        "STUDENT_DISCOUNT|10\n"
        "MEMBER_DISCOUNT|5\n"
    );

    // Tao tai khoan Admin va Staff
    // User ID phai co dang USRxxxx
    writeFile(usersPath,
        "USR0001|admin|123456|Admin Test|ADMIN|1\n"
        "USR0002|staff|123456|Staff Test|STAFF|1\n"
    );

    // Phan quyen
    writeFile(permissionsPath,
        "ADMIN|PRICING_VIEW\n"
        "ADMIN|PRICING_UPDATE\n"
        "STAFF|PRICING_VIEW\n"
    );

    PricingRepository pricingRepo(pricingPath);
    UserRepository userRepo(usersPath);
    RolePermissionRepository permissionRepo(permissionsPath);

    check(userRepo.load(), "Load users");
    check(permissionRepo.load(), "Load permissions");

    AuthService auth(userRepo, permissionRepo);
    PricingService service(pricingRepo, auth);

    check(service.load(), "Service load");

    // ====================================
    // CHUA DANG NHAP
    // ====================================

    check(throwsError(service, false), "Guest cannot view pricing");
    check(throwsError(service, true), "Guest cannot update pricing");

    // ====================================
    // STAFF
    // ====================================

    check(auth.login("staff", "123456"), "Staff login");

    check(auth.hasPermission("PRICING_VIEW"), "Staff has PRICING_VIEW");
    check(!auth.hasPermission("PRICING_UPDATE"), "Staff lacks PRICING_UPDATE");

    check(service.getPricing() != nullptr, "Staff view pricing");
    check(throwsError(service, true), "Staff update denied");

    // ====================================
    // ADMIN
    // ====================================

    auth.logout();

    check(auth.login("admin", "123456"), "Admin login");

    check(auth.hasPermission("PRICING_VIEW"), "Admin has PRICING_VIEW");
    check(auth.hasPermission("PRICING_UPDATE"), "Admin has PRICING_UPDATE");

    // Cap nhat gia ghe
    check(service.updateSeatPrice(SeatType::VIP, 120000), "Admin update VIP");

    check(equalDouble(service.getBasePrice(SeatType::VIP), 120000), "Read updated VIP");
    check(equalDouble(service.getBasePrice(SeatType::STANDARD), 70000), "STANDARD unchanged");

    check(!service.updateSeatPrice(SeatType::VIP, -1), "Reject negative price");
    check(!service.updateSeatPrice(SeatType::VIP, 0), "Reject zero price");

    // Cap nhat giam gia
    check(service.updateDiscountRate(DiscountType::STUDENT, 15), "Update student discount");
    check(service.updateDiscountRate(DiscountType::MEMBER, 8), "Update member discount");

    check(!service.updateDiscountRate(DiscountType::MEMBER, 101), "Reject discount over 100");
    check(!service.updateDiscountRate(DiscountType::STUDENT, -1), "Reject negative discount");

    // ====================================
    // TINH GIAM GIA
    // ====================================

    check(
        equalDouble(service.calculateDiscount(100000, TicketType::REGULAR, false), 0),
        "Regular no discount"
    );

    check(
        equalDouble(service.calculateDiscount(100000, TicketType::REGULAR, true), 8000),
        "Member discount"
    );

    check(
        equalDouble(service.calculateDiscount(100000, TicketType::STUDENT, false), 15000),
        "Student discount"
    );

    check(
        equalDouble(service.calculateDiscount(100000, TicketType::STUDENT, true), 15000),
        "No stacked discounts"
    );

    // ====================================
    // TINH TIEN THANH TOAN
    // ====================================

    check(
        equalDouble(service.calculatePaidAmount(100000, 15000), 85000),
        "Calculate paid amount"
    );

    check(
        equalDouble(service.calculatePaidAmount(100000, 0), 100000),
        "No discount paid amount"
    );

    // ====================================
    // TINH DIEM
    // ====================================

    check(service.calculatePoints(85000, true) == 8, "Member earns 8 points");
    check(service.calculatePoints(85000, false) == 0, "Non-member earns no points");
    check(service.calculatePoints(9999, true) == 0, "Points rounded down");
    check(service.calculatePoints(10000, true) == 1, "Exactly 10000 earns 1 point");
    check(service.calculatePoints(0, true) == 0, "Zero payment earns zero points");

    auth.logout();
}

// ========================================
// MAIN
// ========================================
int main()
{
    const string pricingPath = "test_pricing_temp.txt";
    const string usersPath = "test_pricing_users_temp.txt";
    const string permissionsPath = "test_pricing_permissions_temp.txt";

    try
    {
        testModel();
        testRepository(pricingPath);
        testService(pricingPath, usersPath, permissionsPath);
    }
    catch (const exception& e)
    {
        failed++;
        cout << "[ERROR] " << e.what() << endl;
    }

    // Xoa cac file tam
    remove(pricingPath.c_str());
    remove(usersPath.c_str());
    remove(permissionsPath.c_str());

    cout << "\n===== SUMMARY =====\n";
    cout << "Passed: " << passed << endl;
    cout << "Failed: " << failed << endl;
    cout << "Total: " << passed + failed << endl;

    return failed == 0 ? 0 : 1;
}
