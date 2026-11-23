
#include "PricingRepository.h"
#include "../utils/FileIO.h"
#include "../utils/Validator.h"

#include <vector>
#include <cmath>

using namespace std;

PricingRepository::PricingRepository(const string& filePath)
    : filePath(filePath), loaded(false)
{
}

bool PricingRepository::load()
{
    vector<string> lines;
    if (!FileIO::readLines(filePath, lines))
    {
        return false;
    }
    if (lines.size() != 5)
    {
        return false;
    }
    double values[5] = {};
    bool found[5] = {};
    const string keys[5] = {
        "STANDARD",
        "VIP",
        "COUPLE",
        "STUDENT_DISCOUNT",
        "MEMBER_DISCOUNT"
    };
    for (const string& line : lines)
    {
        size_t pos = line.find('|');
        // Kiem tra dinh dang KEY|VALUE
        if (pos == string::npos ||
            pos == 0 ||
            pos == line.size() - 1 ||
            line.find('|', pos + 1) != string::npos)
        {
            return false;
        }
        string key = line.substr(0, pos);
        string value = line.substr(pos + 1);
        int index = -1;
        for (int i = 0; i < 5; i++)
        {
            if (key == keys[i])
            {
                index = i;
                break;
            }
        }
        // Kiem tra khoa khong ton tai hoac bi trung
        if (index == -1 || found[index])
        {
            return false;
        }
        if (!Validator::isValidNumber(value))
        {
            return false;
        }
        try
        {
            values[index] = stod(value); //string to double
        }
        catch (...)
        {
            return false;
        }

        if (!std::isfinite(values[index])) //kiểm tra có hữu hạn hay k
        {
            return false;
        }

        found[index] = true;
    }

    // Kiem tra du 5 khoa
    for (int i = 0; i < 5; i++)
    {
        if (!found[i])
        {
            return false;
        }
    }

    Pricing temp(
        values[0],
        values[1],
        values[2],
        values[3],
        values[4]
    );

    if (!temp.isValid())
    {
        return false;
    }
    // Chi cap nhat khi toan bo du lieu hop le
    pricing = temp;
    loaded = true;
    return true;
}

bool PricingRepository::save()
{
    if (!loaded || !pricing.isValid())
    {
        return false;
    }
    vector<string> lines = {
        "STANDARD|" + to_string(pricing.getStandardPrice()),
        "VIP|" + to_string(pricing.getVipPrice()),
        "COUPLE|" + to_string(pricing.getCouplePrice()),
        "STUDENT_DISCOUNT|" + to_string(pricing.getStudentDiscount()),
        "MEMBER_DISCOUNT|" + to_string(pricing.getMemberDiscount())
    };
    return FileIO::writeLines(filePath, lines);
}

bool PricingRepository::update(const Pricing& newPricing)
{
    if (!loaded || !newPricing.isValid())
    {
        return false;
    }
    pricing = newPricing;
    return true;
}

const Pricing* PricingRepository::getPricing() const
{
    if (!loaded)
    {
        return nullptr;
    }
    return &pricing;
}

bool PricingRepository::isLoaded() const
{
    return loaded;
}
