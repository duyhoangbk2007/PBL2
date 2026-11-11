#include "Member.h"
#include "Invoice.h"
using namespace std;

Member::Member(const string& memberId,
			   const string& fullName,
			   const string& phone,
			   int points)
	: memberId(memberId),
	  fullName(fullName),
	  phone(phone),
	  points(points)
{}

const string& Member::getMemberId() const {
	return this->memberId;
}
const string& Member::getFullName() const {
	return this->fullName;
}
const string& Member::getPhone() const {
	return this->phone;
}
int Member::getPoints() const {
	return this->points;
}
int Member::getInvoiceCount() const {
	return this->invoices.nodeCount();
}
Invoice* Member::getInvoiceAt(int index) {
	ListNode<Invoice*>* node = this->invoices.getAt(index);
	return node == nullptr ? nullptr : node->data;
}

bool Member::linkInvoice(Invoice* invoice) {
	if (invoice == nullptr || invoice->getMemberId().empty() ||
		invoice->getMemberId() != this->memberId) {
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

void Member::setFullName(const string& fullName) {
	this->fullName = fullName;
}
void Member::setPhone(const string& phone) {
	this->phone = phone;
}
void Member::setPoints(const int& points) {
	this->points = points;
}
