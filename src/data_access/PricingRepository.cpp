#include "PricingRepository.h"
#include "../utils/FileIO.h"
#include "../utils/Validator.h"

#include <exception>
#include <stdexcept>
#include <vector>

using namespace std;

bool PricingRepository::writePricing(const Pricing& pricing)
{
    if (!pricing.isValid())
    {
        return false;
    }

    vector<string> lines =
    {
        "STANDARD|" + to_string(pricing.getStandardPrice()),
        "VIP|" + to_string(pricing.getVipPrice()),
        "COUPLE|" + to_string(pricing.getCouplePrice()),
        "STUDENT_DISCOUNT|" + to_string(pricing.getStudentDiscount()),
        "MEMBER_DISCOUNT|" + to_string(pricing.getMemberDiscount())
    };

    return FileIO::writeLines(filePath, lines);
}

PricingRepository::PricingRepository(const string& filePath)
    : filePath(filePath), loaded(false)
{
}

bool PricingRepository::load()
{
    // Khong su dung bang gia cu neu nap lai that bai
    loaded = false;

    vector<string> lines;

    if (!FileIO::readLines(filePath, lines))
    {
        return false;
    }

    if (lines.size() != 5)
    {
        throw runtime_error(filePath + ": Phai co dung 5 dong");
    }

    const string keys[5] =
    {
        "STANDARD",
        "VIP",
        "COUPLE",
        "STUDENT_DISCOUNT",
        "MEMBER_DISCOUNT"
    };

    double values[5] = {};
    bool found[5] = {};

    for (size_t i = 0; i < lines.size(); i++)
    {
        const string& line = lines[i];
        string error = filePath + ", dong " + to_string(i + 1) + ": ";

        size_t pos = line.find('|');

        if (pos == string::npos)
        {
            throw runtime_error(error + "Sai dinh dang");
        }

        string key = line.substr(0, pos);
        string value = line.substr(pos + 1);

        int index = -1;

        for (int j = 0; j < 5; j++)
        {
            if (key == keys[j])
            {
                index = j;
                break;
            }
        }

        if (index == -1 || found[index])
        {
            throw runtime_error(error + "Khoa sai hoac bi trung");
        }

        if (!Validator::isValidNumber(value))
        {
            throw runtime_error(error + "Gia tri khong hop le");
        }

        try
        {
            values[index] = stod(value);
        }
        catch (const exception&)
        {
            throw runtime_error(error + "Loi chuyen doi gia tri");
        }

        found[index] = true;
    }

    Pricing temp(values[0],
                 values[1],
                 values[2],
                 values[3],
                 values[4]);

    if (!temp.isValid())
    {
        throw runtime_error(filePath + ": Bang gia khong hop le");
    }

    // Chi cap nhat khi toan bo bang gia hop le
    pricing = temp;
    loaded = true;

    return true;
}

bool PricingRepository::save()
{
    if (!loaded)
    {
        return false;
    }

    return writePricing(pricing);
}

bool PricingRepository::update(const Pricing& newPricing)
{
    if (!loaded || !writePricing(newPricing))
    {
        return false;
    }

    // Chi cap nhat bo nho khi ghi file thanh cong
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
