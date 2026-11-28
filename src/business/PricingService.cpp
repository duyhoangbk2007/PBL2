#include "PricingService.h"
#include <stdexcept>
#include <cmath>
#include <limits>
using namespace std;

PricingService::PricingService(PricingRepository& pricingRepo,
                               AuthService& authService)
    : pricingRepo(pricingRepo),
      authService(authService)
{
}

bool PricingService::load()
{
    return pricingRepo.load();
}

const Pricing* PricingService::getPricing() const
{
    authService.requirePermission("PRICING_VIEW");
    return pricingRepo.getPricing();
}

bool PricingService::updateSeatPrice(SeatType seatType, double newPrice)
{
    authService.requirePermission("PRICING_UPDATE");

    const Pricing* current = pricingRepo.getPricing();
    if (current == nullptr)
    {
        return false;
    }

    double standardPrice = current->getStandardPrice();
    double vipPrice = current->getVipPrice();
    double couplePrice = current->getCouplePrice();

    if (seatType == SeatType::STANDARD)
    {
        standardPrice = newPrice;
    }
    else if (seatType == SeatType::VIP)
    {
        vipPrice = newPrice;
    }
    else if (seatType == SeatType::COUPLE)
    {
        couplePrice = newPrice;
    }
    else
    {
        return false;
    }

    Pricing newPricing(standardPrice,
                       vipPrice,
                       couplePrice,
                       current->getStudentDiscount(),
                       current->getMemberDiscount());
    return pricingRepo.update(newPricing);
}

bool PricingService::updateDiscountRate(DiscountType discountType, double newRate)
{
    authService.requirePermission("PRICING_UPDATE");

    const Pricing* current = pricingRepo.getPricing();
    if (current == nullptr)
    {
        return false;
    }

    double studentDiscount = current->getStudentDiscount();
    double memberDiscount = current->getMemberDiscount();

    if (discountType == DiscountType::STUDENT)
    {
        studentDiscount = newRate;
    }
    else if (discountType == DiscountType::MEMBER)
    {
        memberDiscount = newRate;
    }
    else
    {
        return false;
    }

    Pricing newPricing(current->getStandardPrice(),
                       current->getVipPrice(),
                       current->getCouplePrice(),
                       studentDiscount,
                       memberDiscount);
    return pricingRepo.update(newPricing);
}

double PricingService::getBasePrice(SeatType seatType) const
{
    authService.requirePermission("PRICING_VIEW");

    const Pricing* current = pricingRepo.getPricing();
    if (current == nullptr)
    {
        throw runtime_error("Bang gia chua duoc nap");
    }
    if (seatType == SeatType::STANDARD)
    {
        return current->getStandardPrice();
    }
    if (seatType == SeatType::VIP)
    {
        return current->getVipPrice();
    }
    if (seatType == SeatType::COUPLE)
    {
        return current->getCouplePrice();
    }
    throw invalid_argument("Loai ghe khong hop le");
}

double PricingService::calculateDiscount(double basePrice,
                                         TicketType ticketType,
                                         bool isMember) const
{
    authService.requirePermission("PRICING_VIEW");

    const Pricing* current = pricingRepo.getPricing();
    if (current == nullptr)
    {
        throw runtime_error("Bang gia chua duoc nap");
    }
    if (!isfinite(basePrice) || basePrice <= 0)
    {
        throw invalid_argument("Gia ve khong hop le");
    }

    double discountRate = 0;
    if (ticketType == TicketType::STUDENT)
    {
        discountRate = current->getStudentDiscount();
    }
    else if (ticketType != TicketType::REGULAR)
    {
        throw invalid_argument("Loai ve khong hop le");
    }
    if (isMember && current->getMemberDiscount() > discountRate)
    {
        discountRate = current->getMemberDiscount();
    }
    return basePrice * (discountRate / 100.0);
}

double PricingService::calculatePaidAmount(double basePrice, double discountAmount) const
{
    authService.requirePermission("PRICING_VIEW");

    if (!isfinite(basePrice) || !isfinite(discountAmount) ||
        basePrice <= 0 || discountAmount < 0 ||
        discountAmount > basePrice)
    {
        throw invalid_argument("So tien khong hop le");
    }
    return basePrice - discountAmount;
}

int PricingService::calculatePoints(double paidAmount, bool isMember) const
{
    authService.requirePermission("PRICING_VIEW");

    if (!isfinite(paidAmount) || paidAmount < 0)
    {
        throw invalid_argument("So tien thanh toan khong hop le");
    }
    if (!isMember)
    {
        return 0;
    }

    double points = floor(paidAmount / 10000.0);
    if (points > numeric_limits<int>::max())
    {
        throw invalid_argument("So diem vuot gioi han");
    }
    return static_cast<int>(points);
}
