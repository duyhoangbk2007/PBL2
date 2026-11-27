#ifndef PRICING_REPOSITORY_H
#define PRICING_REPOSITORY_H

#include <string>
#include "../models/Pricing.h"

class PricingRepository
{
private:
    std::string filePath;
    Pricing pricing;
    bool loaded;

    bool writePricing(const Pricing& pricing);

public:
    PricingRepository(const std::string& filePath);

    bool load();
    bool save();

    bool update(const Pricing& newPricing);

    const Pricing* getPricing() const;
    bool isLoaded() const;
};

#endif
