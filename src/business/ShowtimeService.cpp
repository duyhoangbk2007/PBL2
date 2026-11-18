
#include "ShowtimeService.h"
#include "../utils/DateTime.h"
#include "../utils/Validator.h"

using namespace std;

ShowtimeService::ShowtimeService(ShowtimeRepository& showtimeRepo, MovieRepository& movieRepo, RoomRepository& roomRepo, AuthService& authService)
    : showtimeRepo(showtimeRepo),
      movieRepo(movieRepo),
      roomRepo(roomRepo),
      authService(authService)
{}

bool ShowtimeService::isValidSchedule(const Showtime& showtime)
{
    Movie* movie = movieRepo.findById(showtime.getMovieId());
    if (movie == nullptr)
    {
        return false;
    }
    const string& startAt = showtime.getStartAt();
    if (!Validator::isValidDateTime(startAt))
    {
        return false;
    }
    // Suat chieu phai bat dau trong tuong lai
    if (!DateTime::isBefore(DateTime::getCurrentDateTime(), startAt))
    {
        return false;
    }
    // Ngay chieu nam trong khoang phat hanh phim
    string showDate = startAt.substr(0, 10);
    if (showDate < movie->getReleaseDate() ||
        showDate > movie->getEndDate())
    {
        return false;
    }
    // Thoi gian ket thuc phai khop thoi luong phim
    string expectedEnd = DateTime::addMinutes(startAt, movie->getDurationMinutes());
    return showtime.getEndAt() == expectedEnd;
}

bool ShowtimeService::load()
{
    return showtimeRepo.load();
}

bool ShowtimeService::save()
{
    return showtimeRepo.save();
}

bool ShowtimeService::addShowtime(Showtime* showtime)
{
    authService.requirePermission("SHOWTIME_ADD");
    if (showtime == nullptr)
    {
        return false;
    }
    // Chi tao suat o trang thai SCHEDULED
    if (showtime->getStatus() != ShowtimeStatus::SCHEDULED)
    {
        return false;
    }
    if (!isValidSchedule(*showtime))
    {
        return false;
    }
    return showtimeRepo.add(showtime);
}

bool ShowtimeService::updateShowtime(const Showtime& newValues)
{
    authService.requirePermission("SHOWTIME_UPDATE");
    Showtime* current = showtimeRepo.findById(newValues.getShowtimeId());
    if (current == nullptr)
    {
        return false;
    }
    // Khong sua suat da huy
    if (current->getStatus() != ShowtimeStatus::SCHEDULED || newValues.getStatus() != ShowtimeStatus::SCHEDULED)
    {
        return false;
    }
    // Khong sua suat da bat dau
    if (!DateTime::isBefore(DateTime::getCurrentDateTime(), current->getStartAt()))
    {
        return false;
    }
    if (!isValidSchedule(newValues))
    {
        return false;
    }
    return showtimeRepo.update(newValues);
}

bool ShowtimeService::cancelShowtime(const string& showtimeId)
{
    authService.requirePermission("SHOWTIME_CANCEL");

    if (!showtimeRepo.isLoaded())
    {
        return false;
    }
    Showtime* current = showtimeRepo.findById(showtimeId);
    if (current == nullptr)
    {
        return false;
    }
    // Khong huy lai suat da huy
    if (current->getStatus() != ShowtimeStatus::SCHEDULED)
    {
        return false;
    }
    // Chi huy suat chua bat dau
    if (!DateTime::isBefore(DateTime::getCurrentDateTime(), current->getStartAt()))
    {
        return false;
    }

    // TODO: Kiem tra khong con ve SOLD khi co TicketRepository.
    // Hien tai chua du thong tin de huy suat an toan.
    return false;
}

bool ShowtimeService::removeShowtime(const string& showtimeId)
{
    authService.requirePermission("SHOWTIME_DELETE");

    if (!showtimeRepo.isLoaded())
    {
        return false;
    }

    Showtime* current = showtimeRepo.findById(showtimeId);

    if (current == nullptr)
    {
        return false;
    }

    // TODO: Chi duoc xoa khi suat chua tung co ve.
    // Chua co TicketRepository de xac minh dieu kien nay.
    return false;
}

const Showtime* ShowtimeService::findShowtimeById(const string& showtimeId)
{
    authService.requirePermission("SHOWTIME_VIEW");
    return showtimeRepo.findById(showtimeId);
}

const Showtime* ShowtimeService::getShowtimeAt(int index)
{
    authService.requirePermission("SHOWTIME_VIEW");
    return showtimeRepo.getAt(index);
}

int ShowtimeService::getShowtimeCount()
{
    authService.requirePermission("SHOWTIME_VIEW");
    return showtimeRepo.count();
}
