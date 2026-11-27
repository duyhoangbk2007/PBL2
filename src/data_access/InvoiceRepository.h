#ifndef INVOICE_REPOSITORY_H
#define INVOICE_REPOSITORY_H

#include <string>
#include <vector>
#include "IRepository.h"
#include "Repository.h"
#include "../models/Invoice.h"

class Showtime;
class User;
class Member;

class InvoiceRepository : public IRepository<Invoice>
{
private:
    Repository<Invoice> repo;
    IRepository<Showtime>& showtimeRepo;
    IRepository<User>& userRepo;
    IRepository<Member>& memberRepo;

    // Chuyen doi du lieu
    static std::string getInvoiceId(Invoice* invoice);
    static Invoice* stringToInvoice(const std::string& line);
    static std::string invoiceToString(Invoice* invoice);

    // Kiem tra du lieu va tham chieu
    static bool isValidInvoice(const Invoice* invoice);
    bool isValidReference(const Invoice* invoice);

    // Quan ly lien ket
    void unlinkAll();

public:
    InvoiceRepository(const std::string& filePath,
                      IRepository<Showtime>& showtimeRepo,
                      IRepository<User>& userRepo,
                      IRepository<Member>& memberRepo);
    ~InvoiceRepository() override;

    // Doc va ghi du lieu
    bool load() override;
    bool save() override;

    // Quan ly du lieu
    bool add(Invoice* invoice) override;
    bool update(const Invoice& newValues) override;
    bool removeById(const std::string& invoiceId) override;

    // Truy van du lieu
    Invoice* findById(const std::string& invoiceId) override;
    Invoice* getAt(int index) override;
    int count() override;
    bool isLoaded() override;

};

#endif
