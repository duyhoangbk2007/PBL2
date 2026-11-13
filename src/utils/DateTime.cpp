#include "DateTime.h"
#include <ctime>
#include <stdexcept>

using namespace std;

string DateTime::getCurrentDateTime() {
    time_t currentTime = time(nullptr);
    tm* local = localtime(&currentTime);

    if (local == nullptr) {
        throw runtime_error("Khong lay duoc thoi gian");
    }

    char buffer[20];

    if (strftime(buffer, sizeof(buffer),"%Y-%m-%dT%H:%M:%S", local) == 0) {
        throw runtime_error("Khong dinh dang duoc thoi gian");
    }

    return string(buffer);
}