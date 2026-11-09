
#include <iostream>
#include "../../core/utils/TransactionManager.h"

using namespace std;

int main() {
    try {
        TransactionManager::recover();
        cout << "PASS: Recover da duyet giao dich\n";
    }
    catch (const exception& e) {
        cout << "FAIL: " << e.what() << '\n';
        return 1;
    }

    return 0;
}
