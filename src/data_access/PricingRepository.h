#ifndef PRICING_REPOSITORY_H
#define PRICING_REPOSITORY_H

#include <string>
#include "../models/Pricing.h"

using namespace std;

class PricingRepository
{
private:
    string filePath;
    Pricing pricing;
    bool loaded;

public:
    PricingRepository(const string& filePath);
    bool load();
    bool save();
    bool update(const Pricing& newPricing);
    const Pricing* getPricing() const;
    bool isLoaded() const;
};

#endif
