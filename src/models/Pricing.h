#ifndef PRICING_H
#define PRICING_H

class Pricing
{
private:
    double standardPrice;
    double vipPrice;
    double couplePrice;
    double studentDiscount;
    double memberDiscount;

public:
    Pricing();
    Pricing(double standardPrice, double vipPrice, double couplePrice, double studentDiscount, double memberDiscount);

    // Getters
    double getStandardPrice() const;
    double getVipPrice() const;
    double getCouplePrice() const;
    double getStudentDiscount() const;
    double getMemberDiscount() const;

    // Validation
    bool isValid() const;
};

#endif
