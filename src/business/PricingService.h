#ifndef PRICING_SERVICE_H
#define PRICING_SERVICE_H

#include "../data_access/PricingRepository.h"
#include "../models/Pricing.h"
#include "../models/Seat.h"
#include "../models/Ticket.h"
#include "AuthService.h"

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
    PricingService(PricingRepository& pricingRepo,
                   AuthService& authService);

    // Quan ly du lieu
    bool load();
    const Pricing* getPricing() const;

    // Cap nhat bang gia
    bool updateSeatPrice(SeatType seatType, double newPrice);
    bool updateDiscountRate(DiscountType discountType, double newRate);

    // Tinh gia ve va diem
    double getBasePrice(SeatType seatType) const;
    double calculateDiscount(double basePrice,
                             TicketType ticketType,
                             bool isMember) const;
    double calculatePaidAmount(double basePrice, double discountAmount) const;
    int calculatePoints(double paidAmount, bool isMember) const;
};

#endif
