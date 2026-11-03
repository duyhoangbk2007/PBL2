#ifndef PBL2_CORE_MODELS_MEMBER_H
#define PBL2_CORE_MODELS_MEMBER_H

#include <string>
#include "../../data_structures/DoublyLinkedList.h"

class Invoice;

class Member {
private:
	std::string memberId;
	std::string fullName;
	std::string phone;
	int points;
	DoublyLinkedList<Invoice*> invoices;

	bool linkInvoice(Invoice* invoice);

	friend class Invoice;

public:
	Member(const std::string& memberId,
		   const std::string& fullName,
		   const std::string& phone,
		   int points);

	Member(const Member&) = delete;
	Member& operator=(const Member&) = delete;
	Member(Member&&) = delete;
	Member& operator=(Member&&) = delete;

	const std::string& getMemberId() const;
	const std::string& getFullName() const;
	const std::string& getPhone() const;
	int getPoints() const;
	int getInvoiceCount() const;
	Invoice* getInvoiceAt(int index);

	void setFullName(const std::string& fullName);
	void setPhone(const std::string& phone);
	void setPoints(const int& points);
};

#endif
