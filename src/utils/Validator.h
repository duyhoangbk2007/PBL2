#ifndef VALIDATOR_H
#define VALIDATOR_H

#include <string>


class Validator
{
public:
    static bool isValidText(const std::string& text);
    static bool isValidDate(const std::string& date);
    static bool isValidId(const std::string& id, const std::string& prefix);
    static bool isValidDateTime(const std::string& dateTime);
    static bool isValidNumber(const std::string& value);
};

#endif
