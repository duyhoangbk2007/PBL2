#include "MovieService.h"

using namespace std;

MovieService::MovieService(MovieRepository& movieRepo, AuthService& authService)
    : movieRepo(movieRepo), authService(authService)
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
    authService.requireAdmin();
    return movieRepo.add(movie);
}
bool MovieService::updateMovie(const Movie& newValues)
{
    authService.requireAdmin();

    if (!movieRepo.isLoaded())
    {
        return false;
    }
    // Tim phim hien tai
    Movie* movie = movieRepo.findById(newValues.getMovieId());
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
bool MovieService::removeMovie(const string& movieId)
{
    authService.requireAdmin();
    return movieRepo.removeById(movieId);
}

const Movie* MovieService::findMovieById(const string& movieId)
{
    authService.requireLogin();

    return movieRepo.findById(movieId);
}
const Movie* MovieService::getMovieAt(int index)
{
    authService.requireLogin();
    return movieRepo.getAt(index);
}
int MovieService::getMovieCount()
{
    authService.requireLogin();
    return movieRepo.count();
}
