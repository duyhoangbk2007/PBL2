#ifndef PRICING_SERVICE_H
#define PRICING_SERVICE_H

#include "../data_access/PricingRepository.h"
#include "AuthService.h"
#include "../models/Seat.h"
#include "../models/Ticket.h"

enum class DiscountType
{
    STUDENT,
    MEMBER
};

class PricingService
{
private:
    PricingRepository& pricingRepo;
    AuthService& authService;

public:
    PricingService(PricingRepository& pricingRepo, AuthService& authService);

    bool load();

    const Pricing* getPricing() const;

    bool updateSeatPrice(SeatType seatType, double newPrice);
    bool updateDiscountRate(DiscountType discountType, double newRate);

    double getBasePrice(SeatType seatType) const;
    double calculateDiscount(double basePrice, TicketType ticketType, bool isMember) const;
    double calculatePaidAmount(double basePrice, double discountAmount) const;
    int calculatePoints(double paidAmount, bool isMember) const;
};

#endif
