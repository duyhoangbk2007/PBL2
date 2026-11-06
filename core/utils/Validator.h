#ifndef VALIDATOR_H
#define VALIDATOR_H

#include <string>

class Validator
{
public:
    static bool isInteger(const std::string& value);
    static bool isBoolean(const std::string& value);
    static bool isDate(const std::string& value);
};


#endif