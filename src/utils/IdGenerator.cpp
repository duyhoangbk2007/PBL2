
#include "IdGenerator.h"
#include <stdexcept>

using namespace std;

string IdGenerator::generateNextId(const string& entity,
                                   const vector<string>& existingIds)
{
    // Kiem tra loai ID
    if (entity != "USR" &&
        entity != "MOV" &&
        entity != "ROM" &&
        entity != "SHW" &&
        entity != "TKT" &&
        entity != "INV" &&
        entity != "MEM")
    {
        throw runtime_error("Loai ID khong hop le");
    }

    int maxNumber = 0;

    // Tim so thu tu lon nhat
    for (const string& id : existingIds)
    {
        if (id.length() != 7 || id.substr(0, 3) != entity)
        {
            throw runtime_error("ID khong hop le: " + id);
        }

        string numberText = id.substr(3);

        for (char c : numberText)
        {
            if (c < '0' || c > '9')
            {
                throw runtime_error("ID khong hop le: " + id);
            }
        }

        int number = stoi(numberText);

        if (number < 1)
        {
            throw runtime_error("ID khong hop le: " + id);
        }

        if (number > maxNumber)
        {
            maxNumber = number;
        }
    }

    // Kiem tra gioi han
    if (maxNumber >= 9999)
    {
        throw runtime_error("Da het ID cho loai: " + entity);
    }

    // Tao ID moi
    int nextNumber = maxNumber + 1;
    string numberText = to_string(nextNumber);

    while (numberText.length() < 4)
    {
        numberText = "0" + numberText;
    }

    return entity + numberText;
}
