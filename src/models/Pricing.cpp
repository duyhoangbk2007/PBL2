#include "Pricing.h"

Pricing::Pricing()
    : standardPrice(70000.0),
      vipPrice(100000.0),
      couplePrice(160000.0),
      studentDiscount(10.0),
      memberDiscount(5.0)
{
}

Pricing::Pricing(
    double standardPrice,
    double vipPrice,
    double couplePrice,
    double studentDiscount,
    double memberDiscount
)
    : standardPrice(standardPrice),
      vipPrice(vipPrice),
      couplePrice(couplePrice),
      studentDiscount(studentDiscount),
      memberDiscount(memberDiscount)
{
}

double Pricing::getStandardPrice() const
{
    return standardPrice;
}

double Pricing::getVipPrice() const
{
    return vipPrice;
}

double Pricing::getCouplePrice() const
{
    return couplePrice;
}

double Pricing::getStudentDiscount() const
{
    return studentDiscount;
}

double Pricing::getMemberDiscount() const
{
    return memberDiscount;
}

#include "Pricing.h"
#include <cmath>

bool Pricing::isValid() const
{
    if (!std::isfinite(standardPrice) ||
        !std::isfinite(vipPrice) ||
        !std::isfinite(couplePrice) ||
        !std::isfinite(studentDiscount) ||
        !std::isfinite(memberDiscount))
    {
        return false;
    }
    if (standardPrice <= 0 || vipPrice <= 0 || couplePrice <= 0)
    {
        return false;
    }
    if (studentDiscount < 0 || studentDiscount > 100)
    {
        return false;
    }
    if (memberDiscount < 0 || memberDiscount > 100)
    {
        return false;
    }

    return true;
}
