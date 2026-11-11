#include "IdGenerator.h"
#include <fstream>
#include <stdexcept>
#include <string>

using namespace std;

bool IdGenerator::isValidEntity(const string& entity)
{
    return entity == "USR" ||
           entity == "MOV" ||
           entity == "ROM" ||
           entity == "SHW" ||
           entity == "TKT" ||
           entity == "INV" ||
           entity == "MEM";
}
string IdGenerator::formatId(const string& entity, int number)
{
    if (number < 1 || number > 9999)
    {
        throw runtime_error("So thu tu ID khong hop le");
    }
    string numberText = to_string(number);
    while (numberText.length() < 4)
    {
        numberText = "0" + numberText;
    }

    return entity + numberText;
}

void IdGenerator::generateNextId(const string& entity, const string& sequencesPath, string& id, string& updatedSequences)
{
    id = "";
    updatedSequences = "";
    string tempId = "";
    string tempSequences = "";
    if (!isValidEntity(entity))
    {
        throw runtime_error("Loai ma khong hop le: " + entity);
    }
    // Mở file sequences.txt
    ifstream input(sequencesPath);

    if (!input.is_open())
    {
        throw runtime_error("Khong mo duoc file: " + sequencesPath);
    }
    string line;
    bool found = false;
    int lineNumber = 0;
    string usedEntities[7];
    int usedCount = 0;

    while (getline(input, line))
    {
        lineNumber++;
        // Xu ly ky tu xuong dong Windows neu con
        if (!line.empty() && line.back() == '\r')
        {
            line.pop_back();
        }
        if (line.empty())
        {
            continue;
        }
        // Tim dau phan cach |
        size_t pos = line.find('|');
        if (pos == string::npos)
        {
            throw runtime_error("Sai dinh dang tai dong " + to_string(lineNumber));
        }
        // Tach loai ma va so thu tu
        string key = line.substr(0, pos);
        string numberText = line.substr(pos + 1);

        // Kiem tra loai ma
        if (!isValidEntity(key))
        {
            throw runtime_error("Loai ma khong hop le tai dong " + to_string(lineNumber));
        }
        // Kiem tra chuoi so
        if (numberText.empty())
        {
            throw runtime_error("Bo dem rong tai dong " + to_string(lineNumber));
        }
        // kiểm tra có phải là số hay không
        for (int i = 0; i < (int)numberText.length(); i++)
        {
            if (numberText[i] < '0' || numberText[i] > '9')
            {
                throw runtime_error("Bo dem khong hop le tai dong " + to_string(lineNumber));
            }
        }
        // Chuyen chuoi thanh so
        int number;
        try
        {
            number = stoi(numberText);
        }
        catch (const exception&)
        {
            throw runtime_error("Bo dem vuot gioi han tai dong " + to_string(lineNumber));
        }
        if (number < 1 || number > 10000)
        {
            throw runtime_error("Bo dem phai tu 1 den 10000");
        }
        //kiểm tra trùng
        for (int i = 0; i < usedCount; i++)
        {
            if (usedEntities[i] == key)
            {
                throw runtime_error("Loai ma bi trung: " + key);
            }
        }

        if (usedCount >= 7)
        {
            throw runtime_error("Qua nhieu loai ma trong file");
        }
        usedEntities[usedCount] = key;
        usedCount++;
        // Tạo id
        // Neu dung loai ma can tao
        if (key == entity)
        {
            if (number == 10000)
            {
                throw runtime_error("Da het ID cho loai ma: " + entity);
            }
            tempId = formatId(entity, number);
            number++;
            found = true;
        }
        // Them dong vao noi dung file moi
        tempSequences += key + "|" + to_string(number) + "\n";
    }
    if (input.bad())
    {
        throw runtime_error("Loi khi doc file sequences");
    }
    // Kiem tra da tim thay bo dem hay chua
    if (!found)
    {
        throw runtime_error("Khong tim thay bo dem: " + entity);
    }
    id = tempId;
    updatedSequences = tempSequences;

}
