#ifndef VALIDATOR_H
#define VALIDATOR_H

#include <string>

using namespace std;

class Validator
{
public:
    static bool isValidText(const string& text);
    static bool isValidDate(const string& date);
};

#endif
