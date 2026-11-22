#include "InvoiceRepository.h"
#include "ShowtimeRepository.h"
#include "UserRepository.h"
#include "MemberRepository.h"

#include "../models/Showtime.h"
#include "../models/User.h"
#include "../models/Member.h"

#include "../utils/Validator.h"
#include "../utils/DateTime.h"

#include <cmath>
#include <stdexcept>
#include <vector>

using namespace std;

InvoiceRepository::InvoiceRepository(const string& filePath,
                                     ShowtimeRepository& showtimeRepo,
                                     UserRepository& userRepo,
                                     MemberRepository& memberRepo)
    : repo(filePath, getInvoiceId, stringToInvoice, invoiceToString),
      showtimeRepo(showtimeRepo),
      userRepo(userRepo),
      memberRepo(memberRepo)
{
}

InvoiceRepository::~InvoiceRepository()
{
    unlinkAll();
}

void InvoiceRepository::unlinkAll()
{
    for (int i = 0; i < repo.count(); i++)
    {
        Invoice* invoice = repo.getAt(i);
        if (invoice != nullptr)
        {
            invoice->unlinkReferences();
        }
    }
}

string InvoiceRepository::getInvoiceId(Invoice* invoice)
{
    if (invoice == nullptr)
    {
        return "";
    }
    return invoice->getInvoiceId();
}

bool InvoiceRepository::isValidInvoice(const Invoice* invoice)
{
    if (invoice == nullptr)
    {
        return false;
    }
    if (!Validator::isValidId(invoice->getInvoiceId(), "INV") ||
        !Validator::isValidId(invoice->getShowtimeId(), "SHW") ||
        !Validator::isValidId(invoice->getStaffId(), "USR"))
    {
        return false;
    }
    if (!invoice->getMemberId().empty() &&
        !Validator::isValidId(invoice->getMemberId(), "MEM"))
    {
        return false;
    }
    if (!Validator::isValidDateTime(invoice->getCreatedAt()))
    {
        return false;
    }
    double totalAmount = invoice->getTotalAmount();
    if (!isfinite(totalAmount) || totalAmount < 0)
    {
        return false;
    }
    return true;
}

Invoice* InvoiceRepository::stringToInvoice(const string& line)
{
    vector<string> fields;
    size_t start = 0;
    while (true)
    {
        size_t pos = line.find('|', start);
        if (pos == string::npos)
        {
            fields.push_back(line.substr(start));
            break;
        }
        fields.push_back(line.substr(start, pos - start));
        start = pos + 1;
    }

    if (fields.size() != 6)
    {
        throw runtime_error("Sai so luong truong Invoice");
    }
    double totalAmount = 0;
    try
    {
        if (!Validator::isValidNumber(fields[5]))
        {
            throw runtime_error("Sai dinh dang tong tien");
        }
        size_t pos = 0;
        totalAmount = stod(fields[5], &pos);
        if (pos != fields[5].size())
        {
            throw runtime_error("Sai dinh dang tong tien");
        }
    }
    catch (const exception&)
    {
        throw runtime_error("Tong tien Invoice khong hop le");
    }

    Invoice* invoice = new Invoice(fields[0],
                                   fields[1],
                                   fields[2],
                                   fields[3],
                                   fields[4],
                                   totalAmount);

    if (!isValidInvoice(invoice))
    {
        delete invoice;
        throw runtime_error("Du lieu Invoice khong hop le");
    }
    return invoice;
}

string InvoiceRepository::invoiceToString(Invoice* invoice)
{
    if (!isValidInvoice(invoice))
    {
        throw runtime_error("Invoice khong hop le khi ghi file");
    }

    return invoice->getInvoiceId() + "|" +
           invoice->getShowtimeId() + "|" +
           invoice->getStaffId() + "|" +
           invoice->getMemberId() + "|" +
           invoice->getCreatedAt() + "|" +
           to_string(invoice->getTotalAmount());
}

bool InvoiceRepository::isValidReference(const Invoice* invoice)
{
    if (!isValidInvoice(invoice))
    {
        return false;
    }
    if (!showtimeRepo.isLoaded() ||
        !userRepo.isLoaded() ||
        !memberRepo.isLoaded())
    {
        return false;
    }
    Showtime* showtime = showtimeRepo.findById(invoice->getShowtimeId());
    User* seller = userRepo.findById(invoice->getStaffId());
    if (showtime == nullptr || seller == nullptr)
    {
        return false;
    }
    if (seller->getRole() != UserRole::STAFF &&
        seller->getRole() != UserRole::ADMIN)
    {
        return false;
    }
    if (!DateTime::isBefore(invoice->getCreatedAt(),
                            showtime->getStartAt()))
    {
        return false;
    }
    if (!invoice->getMemberId().empty())
    {
        Member* member = memberRepo.findById(invoice->getMemberId());
        if (member == nullptr)
        {
            return false;
        }
    }

    return true;
}

bool InvoiceRepository::load()
{
    if (repo.isLoaded() ||
        !showtimeRepo.isLoaded() ||
        !userRepo.isLoaded() ||
        !memberRepo.isLoaded())
    {
        return false;
    }
    if (!repo.load())
    {
        return false;
    }
    try
    {
        // Giai doan 1: Kiem tra tham chieu
        for (int i = 0; i < repo.count(); i++)
        {
            Invoice* invoice = repo.getAt(i);
            if (!isValidReference(invoice))
            {
                throw runtime_error("Invoice co tham chieu khong hop le");
            }
        }
        // Giai doan 2: Lien ket doi tuong
        for (int i = 0; i < repo.count(); i++)
        {
            Invoice* invoice = repo.getAt(i);
            Showtime* showtime = showtimeRepo.findById(invoice->getShowtimeId());
            User* seller = userRepo.findById(invoice->getStaffId());
            Member* member = nullptr;

            if (!invoice->getMemberId().empty())
            {
                member = memberRepo.findById(invoice->getMemberId());
            }
            if (!invoice->linkShowtime(showtime) ||
                !invoice->linkSeller(seller) ||
                !invoice->linkMember(member))
            {
                throw runtime_error("Khong the lien ket Invoice");
            }
        }
    }
    catch (...)
    {
        unlinkAll();
        repo.clear();
        throw;
    }
    return true;
}

bool InvoiceRepository::save()
{
    if (!repo.save())
    {
        return false;
    }
    pendingIds.clear();
    return true;
}

bool InvoiceRepository::add(Invoice* invoice)
{
    if (!repo.isLoaded() || !isValidReference(invoice))
    {
        return false;
    }

    if (repo.findById(invoice->getInvoiceId()) != nullptr)
    {
        return false;
    }
    // không thêm trùng
    for (int i = 0; i < repo.count(); i++)
    {
        if (repo.getAt(i) == invoice)
        {
            return false;
        }
    }
    // Khong them Invoice da duoc lien ket
    if (invoice->getShowtime() != nullptr ||
        invoice->getSeller() != nullptr ||
        invoice->getMember() != nullptr)
    {
        return false;
    }
    Showtime* showtime = showtimeRepo.findById(invoice->getShowtimeId());
    User* seller = userRepo.findById(invoice->getStaffId());
    Member* member = nullptr;

    if (!invoice->getMemberId().empty())
    {
        member = memberRepo.findById(invoice->getMemberId());
    }
    pendingIds.push_back(invoice->getInvoiceId());
    try
    {
        if (!invoice->linkShowtime(showtime) ||
            !invoice->linkSeller(seller) ||
            !invoice->linkMember(member))
        {
            invoice->unlinkReferences();
            pendingIds.pop_back();
            return false;
        }

        if (!repo.add(invoice))
        {
            invoice->unlinkReferences();
            pendingIds.pop_back();
            return false;
        }
    }
    catch (...)
    {
        invoice->unlinkReferences();
        pendingIds.pop_back();
        throw;
    }

    return true;
}

bool InvoiceRepository::rollbackPending(const string& invoiceId)
{
    if (pendingIds.empty() || pendingIds.back() != invoiceId)
    {
        return false;
    }
    Invoice* invoice = repo.findById(invoiceId);
    if (invoice == nullptr || invoice->getTicketCount() != 0)
    {
        return false;
    }
    invoice->unlinkReferences();
    if (!repo.removeById(invoiceId))
    {
        return false;
    }
    pendingIds.pop_back();
    return true;
}

// Tim hoa don theo ID
Invoice* InvoiceRepository::findById(const string& invoiceId)
{
    return repo.findById(invoiceId);
}

// Lay hoa don theo vi tri
Invoice* InvoiceRepository::getAt(int index)
{
    return repo.getAt(index);
}

// Dem so hoa don
int InvoiceRepository::count()
{
    return repo.count();
}

// Kiem tra trang thai load
bool InvoiceRepository::isLoaded()
{
    return repo.isLoaded();
}

// Loc hoa don theo suat chieu
vector<Invoice*> InvoiceRepository::getByShowtime(const string& showtimeId)
{
    vector<Invoice*> result;

    if (!repo.isLoaded())
    {
        return result;
    }

    for (int i = 0; i < repo.count(); i++)
    {
        Invoice* invoice = repo.getAt(i);

        if (invoice->getShowtimeId() == showtimeId)
        {
            result.push_back(invoice);
        }
    }

    return result;
}

// Loc hoa don theo nhan vien
vector<Invoice*> InvoiceRepository::getByStaff(const string& staffId)
{
    vector<Invoice*> result;

    if (!repo.isLoaded())
    {
        return result;
    }

    for (int i = 0; i < repo.count(); i++)
    {
        Invoice* invoice = repo.getAt(i);

        if (invoice->getStaffId() == staffId)
        {
            result.push_back(invoice);
        }
    }

    return result;
}

// Loc hoa don theo thanh vien
vector<Invoice*> InvoiceRepository::getByMember(const string& memberId)
{
    vector<Invoice*> result;

    if (!repo.isLoaded() || memberId.empty())
    {
        return result;
    }

    for (int i = 0; i < repo.count(); i++)
    {
        Invoice* invoice = repo.getAt(i);

        if (invoice->getMemberId() == memberId)
        {
            result.push_back(invoice);
        }
    }

    return result;
}
