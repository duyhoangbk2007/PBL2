#include "Member.h"
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

void Member::setFullName(const string& fullName) {
	this->fullName = fullName;
}
void Member::setPhone(const string& phone) {
	this->phone = phone;
}
void Member::setPoints(const int& points) {
	this->points = points;
}
