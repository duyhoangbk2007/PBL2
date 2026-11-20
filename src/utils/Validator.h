#ifndef VALIDATOR_H
#define VALIDATOR_H

#include <string>

using namespace std;

class Validator
{
public:
    static bool isValidText(const string& text);
    static bool isValidDate(const string& date);
    static bool isValidId(const string& id, const string& prefix);
    static bool isValidDateTime(const string& dateTime);
    static bool isValidNumber(const string& value);
};

#endif
