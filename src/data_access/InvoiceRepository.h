#ifndef INVOICE_REPOSITORY_H
#define INVOICE_REPOSITORY_H

#include <string>
#include <vector>

#include "Repository.h"
#include "../models/Invoice.h"

class ShowtimeRepository;
class UserRepository;
class MemberRepository;

class InvoiceRepository
{
private:
    Repository<Invoice> repo;
    std::vector<std::string> pendingIds;
    ShowtimeRepository& showtimeRepo;
    UserRepository& userRepo;
    MemberRepository& memberRepo;

    static std::string getInvoiceId(Invoice* invoice);
    static Invoice* stringToInvoice(const std::string& line);
    static std::string invoiceToString(Invoice* invoice);

    static bool isValidInvoice(const Invoice* invoice);
    bool isValidReference(const Invoice* invoice);

    void unlinkAll();

public:
    InvoiceRepository(const std::string& filePath,
                      ShowtimeRepository& showtimeRepo,
                      UserRepository& userRepo,
                      MemberRepository& memberRepo);

    ~InvoiceRepository();

    bool load();
    bool save();

    bool add(Invoice* invoice);
    bool rollbackPending(const std::string& invoiceId);

    Invoice* findById(const std::string& invoiceId);
    Invoice* getAt(int index);
    int count();
    bool isLoaded();

    std::vector<Invoice*> getByShowtime(const std::string& showtimeId);
    std::vector<Invoice*> getByStaff(const std::string& staffId);
    std::vector<Invoice*> getByMember(const std::string& memberId);
};

#endif
