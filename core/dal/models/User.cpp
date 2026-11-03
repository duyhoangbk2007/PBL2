#include "User.h"
#include "Invoice.h"
using namespace std;

User::User(const string& userId,
           const string& username,
           const string& password,
           const string& fullName,
           UserRole role,
           bool active)
    : userId(userId),
      username(username),
      password(password),
      fullName(fullName),
      role(role),
      active(active)
{}

const string& User::getUserId() const {
    return this->userId;
}

const string& User::getUsername() const {
    return this->username;
}

const string& User::getPassword() const {
    return this->password;
}

const string& User::getFullName() const {
    return this->fullName;
}

UserRole User::getRole() const {
    return this->role;
}

bool User::isActive() const {
    return this->active;
}

int User::getInvoiceCount() const {
    return this->invoices.nodeCount();
}

Invoice* User::getInvoiceAt(int index) {
    ListNode<Invoice*>* node = this->invoices.getAt(index);
    return node == nullptr ? nullptr : node->data;
}

bool User::addInvoiceReference(Invoice* invoice) {
    if (invoice == nullptr || invoice->getStaffId() != this->userId) {
        return false;
    }

    for (int index = 0; index < this->invoices.nodeCount(); ++index) {
        Invoice* current = this->invoices.getAt(index)->data;
        if (current == invoice) {
            return true;
        }
        if (current->getInvoiceId() == invoice->getInvoiceId()) {
            return false;
        }
    }

    this->invoices.pushBack(invoice);
    return true;
}

void User::setUsername(const string& username) {
    this->username = username;
}

void User::setPassword(const string& password) {
    this->password = password;
}

void User::setFullName(const string& fullName) {
    this->fullName = fullName;
}

void User::setActive(bool active) {
    this->active = active;
}
