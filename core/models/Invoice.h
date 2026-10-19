#ifndef PBL2_CORE_MODELS_INVOICE_H
#define PBL2_CORE_MODELS_INVOICE_H

#include <string>

class Invoice {
private:
    std::string invoiceId;
    std::string showtimeId;
    std::string staffId;
    std::string memberId;
    std::string createdAt;
    double totalAmount;

public:
    Invoice();
    Invoice(const std::string& invoiceId,
            const std::string& showtimeId,
            const std::string& staffId,
            const std::string& memberId,
            const std::string& createdAt,
            double totalAmount);

    const std::string& getInvoiceId() const;
    const std::string& getShowtimeId() const;
    const std::string& getStaffId() const;
    const std::string& getMemberId() const;
    const std::string& getCreatedAt() const;
    double getTotalAmount() const;
};

#endif
