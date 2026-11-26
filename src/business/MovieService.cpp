#include "MovieService.h"
#include "../models/Showtime.h"
#include "../utils/DateTime.h"
#include "../utils/IdGenerator.h"
#include <cctype>
using namespace std;

MovieService::MovieService(IRepository<Movie>& movieRepo,
                           AuthService& authService)
    : movieRepo(movieRepo), authService(authService)
{
}

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
    authService.requirePermission("MOVIE_ADD");

    if (!movieRepo.isLoaded() || movie == nullptr)
    {
        return false;
    }

    vector<string> existingIds;
    for (int i = 0; i < movieRepo.count(); i++)
    {
        const Movie* current = movieRepo.getAt(i);
        if (current == nullptr)
        {
            return false;
        }
        existingIds.push_back(current->getMovieId());
    }

    string movieId = IdGenerator::generateNextId("MOV", existingIds);
    Movie* newMovie = new Movie(movieId,
                               movie->getTitle(),
                               movie->getGenre(),
                               movie->getDurationMinutes(),
                               movie->getMinAge(),
                               movie->getReleaseDate(),
                               movie->getEndDate(),
                               movie->isActive());
    try
    {
        if (!movieRepo.add(newMovie))
        {
            delete newMovie;
            return false;
        }
    }
    catch (...)
    {
        delete newMovie;
        throw;
    }
    return true;
}

bool MovieService::updateMovie(const Movie& newValues)
{
    authService.requirePermission("MOVIE_UPDATE");

    if (!movieRepo.isLoaded())
    {
        return false;
    }
    const Movie* movie = movieRepo.findById(newValues.getMovieId());
    if (movie == nullptr)
    {
        return false;
    }
    if (movie->isActive() != newValues.isActive())
    {
        return false;
    }

    bool dateChanged =
        movie->getReleaseDate() != newValues.getReleaseDate() ||
        movie->getEndDate() != newValues.getEndDate();
    bool durationChanged =
        movie->getDurationMinutes() != newValues.getDurationMinutes();

    if (movie->getShowtimeCount() > 0 &&
        (dateChanged || durationChanged))
    {
        return false;
    }
    return movieRepo.update(newValues);
}

bool MovieService::removeMovie(const string& movieId)
{
    authService.requirePermission("MOVIE_DELETE");
    return movieRepo.removeById(movieId);
}

bool MovieService::setMovieActive(const string& movieId, bool active)
{
    authService.requirePermission("MOVIE_UPDATE");

    if (!movieRepo.isLoaded())
    {
        return false;
    }
    Movie* movie = movieRepo.findById(movieId);
    if (movie == nullptr)
    {
        return false;
    }
    if (movie->isActive() == active)
    {
        return true;
    }
    if (!active)
    {
        string now = DateTime::getCurrentDateTime();
        for (int i = 0; i < movie->getShowtimeCount(); i++)
        {
            const Showtime* showtime = movie->getShowtimeAt(i);
            if (showtime == nullptr)
            {
                return false;
            }
            if (showtime->getStatus() == ShowtimeStatus::SCHEDULED &&
                DateTime::isBefore(now, showtime->getStartAt()))
            {
                return false;
            }
        }
    }

    Movie updated(movie->getMovieId(),
                  movie->getTitle(),
                  movie->getGenre(),
                  movie->getDurationMinutes(),
                  movie->getMinAge(),
                  movie->getReleaseDate(),
                  movie->getEndDate(),
                  active);
    return movieRepo.update(updated);
}

const Movie* MovieService::findMovieById(const string& movieId)
{
    authService.requirePermission("MOVIE_VIEW");
    return movieRepo.findById(movieId);
}

const Movie* MovieService::getMovieAt(int index)
{
    authService.requirePermission("MOVIE_VIEW");
    return movieRepo.getAt(index);
}

int MovieService::getMovieCount()
{
    authService.requirePermission("MOVIE_VIEW");
    return movieRepo.count();
}

vector<const Movie*> MovieService::getNowShowingMovies()
{
    authService.requirePermission("MOVIE_VIEW");

    vector<const Movie*> result;
    if (!movieRepo.isLoaded())
    {
        return result;
    }
    string today = DateTime::getCurrentDateTime().substr(0, 10);
    for (int i = 0; i < movieRepo.count(); i++)
    {
        const Movie* movie = movieRepo.getAt(i);
        if (movie == nullptr)
        {
            continue;
        }
        if (movie->isActive() &&
            movie->getReleaseDate() <= today &&
            today <= movie->getEndDate())
        {
            result.push_back(movie);
        }
    }
    return result;
}

vector<const Movie*> MovieService::getComingSoonMovies()
{
    authService.requirePermission("MOVIE_VIEW");

    vector<const Movie*> result;
    if (!movieRepo.isLoaded())
    {
        return result;
    }
    string today = DateTime::getCurrentDateTime().substr(0, 10);
    for (int i = 0; i < movieRepo.count(); i++)
    {
        const Movie* movie = movieRepo.getAt(i);
        if (movie == nullptr)
        {
            continue;
        }
        if (movie->isActive() &&
            today < movie->getReleaseDate())
        {
            result.push_back(movie);
        }
    }
    return result;
}

vector<const Movie*> MovieService::searchMoviesByTitle(const string& keyword)
{
    authService.requirePermission("MOVIE_VIEW");

    vector<const Movie*> result;
    if (!movieRepo.isLoaded())
    {
        return result;
    }
    string searchKey = keyword;
    for (char& c : searchKey)
    {
        c = static_cast<char>(tolower(static_cast<unsigned char>(c)));
    }
    for (int i = 0; i < movieRepo.count(); i++)
    {
        const Movie* movie = movieRepo.getAt(i);
        if (movie == nullptr)
        {
            continue;
        }
        string title = movie->getTitle();
        for (char& c : title)
        {
            c = static_cast<char>(tolower(static_cast<unsigned char>(c)));
        }
        if (title.find(searchKey) != string::npos)
        {
            result.push_back(movie);
        }
    }
    return result;
}
