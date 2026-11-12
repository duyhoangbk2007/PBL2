#include "MovieService.h"

using namespace std;

MovieService::MovieService(MovieRepository& movieRepo)
    : movieRepo(movieRepo)
{}
bool MovieService::load()
{
    return movieRepo.load();
}
bool MovieService::save()
{
    return movieRepo.save();
}
bool MovieService::addMovie(Movie* movie)
{
    return movieRepo.add(movie);
}

bool MovieService::updateMovie(const Movie& newValues)
{
    if (!movieRepo.isLoaded())
    {
        return false;
    }
    // Tim phim hien tai
    Movie* movie = findMovieById(newValues.getMovieId());
    if (movie == nullptr)
    {
        return false;
    }
    // Khong cho doi trang thai qua ham update
    if (movie->isActive() != newValues.isActive())
    {
        return false;
    }
    // Kiem tra thay doi khoang ngay
    bool dateChanged = movie->getReleaseDate() != newValues.getReleaseDate() || movie->getEndDate() != newValues.getEndDate();

    // Tam thoi khong cho doi ngay neu da co Showtime
    if (dateChanged && movie->getShowtimeCount() > 0)
    {
        return false;
    }
    // Repository kiem tra dinh dang va cap nhat
    return movieRepo.update(newValues);
}


Movie* MovieService::findMovieById(const string& movieId)
{
    return movieRepo.findById(movieId);
}
Movie* MovieService::getMovieAt(int index)
{
    return movieRepo.getAt(index);
}
int MovieService::getMovieCount()
{
    return movieRepo.count();
}
