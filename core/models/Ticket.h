#ifndef PBL2_CORE_MODELS_TICKET_H
#define PBL2_CORE_MODELS_TICKET_H

#include <string>

enum class TicketType {
    REGULAR,
    STUDENT
};

enum class TicketStatus {
    SOLD,
    CANCELLED
};

class Ticket {
private:
    std::string ticketId;             
    std::string invoiceId;            
    std::string showtimeId;           
    std::string seatId;               
    TicketType ticketType;           
    double basePrice;                 
    double discountAmount;           
    double paidAmount;   
    int pointsEarned;       
    TicketStatus status;          
    std::string cancelledAt;    
    std::string cancelledBy;  
    std::string cancelReason; 
    double refundAmount;              

public:
    Ticket();
    Ticket(const std::string& ticketId,
           const std::string& invoiceId,
           const std::string& showtimeId,
           const std::string& seatId,
           TicketType ticketType,
           double basePrice,
           double discountAmount,
           double paidAmount,
           int pointsEarned,
           TicketStatus status,
           const std::string& cancelledAt,
           const std::string& cancelledBy,
           const std::string& cancelReason,
           double refundAmount);

    const std::string& getTicketId() const;
    const std::string& getInvoiceId() const;
    const std::string& getShowtimeId() const;
    const std::string& getSeatId() const;
    TicketType getTicketType() const;
    double getBasePrice() const;
    double getDiscountAmount() const;
    double getPaidAmount() const;
    int getPointsEarned() const;
    TicketStatus getStatus() const;
    const std::string& getCancelledAt() const;
    const std::string& getCancelledBy() const;
    const std::string& getCancelReason() const;
    double getRefundAmount() const;

    void markCancelled(const std::string& cancelledAt,
                       const std::string& cancelledBy,
                       const std::string& cancelReason);
};

#endif
