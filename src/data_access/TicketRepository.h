#ifndef TICKET_REPOSITORY_H
#define TICKET_REPOSITORY_H

#include <string>
#include "IRepository.h"
#include "Repository.h"
#include "../models/Ticket.h"

class Showtime;
class Seat;
class Invoice;

class TicketRepository : public IRepository<Ticket>
{
private:
    Repository<Ticket> repo;
    IRepository<Showtime>& showtimeRepo;
    IRepository<Seat>& seatRepo;
    IRepository<Invoice>& invoiceRepo;

    static bool isValidTicket(const Ticket* ticket);
    static std::string getTicketId(Ticket* ticket);
    static Ticket* stringToTicket(const std::string& line);
    static std::string ticketToString(Ticket* ticket);

    bool isValidReference(const Ticket* ticket);

    bool isSeatSold(const std::string& showtimeId,
                    const std::string& seatId);
    void unlinkAll();

public:
    TicketRepository(const std::string& filePath,
                     IRepository<Showtime>& showtimeRepo,
                     IRepository<Seat>& seatRepo,
                     IRepository<Invoice>& invoiceRepo);
    ~TicketRepository() override;

    // Doc va ghi du lieu
    bool load() override;
    bool save() override;

    // Quan ly du lieu
    bool add(Ticket* ticket) override;
    bool update(const Ticket& newValues) override;
    bool removeById(const std::string& ticketId) override;

    // Truy van du lieu
    Ticket* findById(const std::string& ticketId) override;
    Ticket* getAt(int index) override;
    int count() override;
    bool isLoaded() override;
};

#endif
