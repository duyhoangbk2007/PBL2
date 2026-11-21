#ifndef TICKET_REPOSITORY_H
#define TICKET_REPOSITORY_H

#include <string>
#include <vector>

#include "Repository.h"
#include "../models/Ticket.h"

class ShowtimeRepository;
class SeatRepository;
class InvoiceRepository;

class TicketRepository
{
private:
    Repository<Ticket> repo;
    ShowtimeRepository& showtimeRepo;
    SeatRepository& seatRepo;
    InvoiceRepository& invoiceRepo;

    static std::string getTicketId(Ticket* ticket);
    static Ticket* stringToTicket(const std::string& line);
    static std::string ticketToString(Ticket* ticket);
    static bool isValidTicket(const Ticket* ticket);

    bool isValidReference(const Ticket* ticket);
    void unlinkAll();

public:
    TicketRepository(const std::string& filePath, ShowtimeRepository& showtimeRepo, SeatRepository& seatRepo, InvoiceRepository& invoiceRepo);
    ~TicketRepository();
    
    bool load();
    bool save();

    bool add(Ticket* ticket);
    Ticket* findById(const std::string& ticketId);
    Ticket* getAt(int index);
    int count();
    bool isLoaded();

    std::vector<Ticket*> getByShowtime(const std::string& showtimeId);
    std::vector<Ticket*> getByInvoice(const std::string& invoiceId);
    bool isSeatSold(const std::string& showtimeId, const std::string& seatId);
};

#endif
