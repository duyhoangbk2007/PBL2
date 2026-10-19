#include "Invoice.h"
using namespace std;

Invoice::Invoice()
    : invoiceId(""),
      showtimeId(""),
      staffId(""),
      memberId(""),
      createdAt(""),
      totalAmount(0.0)
{}

Invoice::Invoice(const string& invoiceId,
                 const string& showtimeId,
                 const string& staffId,
                 const string& memberId,
                 const string& createdAt,
                 double totalAmount)
    : invoiceId(invoiceId),
      showtimeId(showtimeId),
      staffId(staffId),
      memberId(memberId),
      createdAt(createdAt),
      totalAmount(totalAmount)
{}

const string& Invoice::getInvoiceId() const {
    return this->invoiceId;
}

const string& Invoice::getShowtimeId() const {
    return this->showtimeId;
}

const string& Invoice::getStaffId() const {
    return this->staffId;
}

const string& Invoice::getMemberId() const {
    return this->memberId;
}

const string& Invoice::getCreatedAt() const {
    return this->createdAt;
}

double Invoice::getTotalAmount() const {
    return this->totalAmount;
}
