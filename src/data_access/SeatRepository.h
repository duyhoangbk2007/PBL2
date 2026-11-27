#ifndef SEAT_REPOSITORY_H
#define SEAT_REPOSITORY_H

#include <string>
#include "IRepository.h"
#include "Repository.h"
#include "../models/Seat.h"

class Room;

class SeatRepository : public IRepository<Seat>
{
private:
    Repository<Seat> repo;
    IRepository<Room>& roomRepo;

    // Chuyen doi du lieu
    static std::string getSeatKey(Seat* seat);
    static Seat* stringToSeat(const std::string& line);
    static std::string seatToString(Seat* seat);

    // Kiem tra du lieu
    static bool isValidSeat(const Seat* seat);

    // Quan ly lien ket
    void unlinkReferences(Seat* seat);
    void unlinkAll();

public:
    SeatRepository(const std::string& filePath,
                   IRepository<Room>& roomRepo);
    ~SeatRepository() override;

    // Doc va ghi du lieu
    bool load() override;
    bool save() override;

    // Quan ly du lieu
    bool add(Seat* seat) override;
    bool update(const Seat& newValues) override;

    bool removeById(const std::string& key) override;
    bool removeById(const std::string& roomId,
                    const std::string& seatId);

    // Truy van du lieu
    Seat* findById(const std::string& key) override;
    Seat* findById(const std::string& roomId,
                   const std::string& seatId);

    Seat* getAt(int index) override;
    int count() override;
    bool isLoaded() override;
};

#endif
