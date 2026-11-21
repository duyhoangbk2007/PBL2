#include "MovieService.h"
#include "../models/Showtime.h"
#include "../utils/DateTime.h"
#include <algorithm>
#include <cctype>
#include <vector>
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
    authService.requirePermission("MOVIE_ADD");
    return movieRepo.add(movie);
}

bool MovieService::updateMovie(const Movie& newValues)
{
    authService.requirePermission("MOVIE_UPDATE");

    if (!movieRepo.isLoaded())
    {
        return false;
    }
    Movie* movie = movieRepo.findById(newValues.getMovieId());
    if (movie == nullptr)
    {
        return false;
    }
    // Khong cho thay doi trang thai qua updateMovie
    if (movie->isActive() != newValues.isActive())
    {
        return false;
    }
    // Kiem tra thay doi ngay phat hanh
    bool dateChanged =
        movie->getReleaseDate() != newValues.getReleaseDate() ||
        movie->getEndDate() != newValues.getEndDate();

    // Kiem tra thay doi thoi luong phim
    bool durationChanged = movie->getDurationMinutes() != newValues.getDurationMinutes();

    // Khong cho doi ngay hoac thoi luong khi da co suat chieu
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
    // Trang thai da dung voi yeu cau
    if (movie->isActive() == active)
    {
        return true;
    }
    // Kiem tra cac suat chieu khi ngung hoat dong phim
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
            // suất chiếu đã lên lịch và chưa bắt đầu thì không cho hủy phim
            if (showtime->getStatus() == ShowtimeStatus::SCHEDULED &&
                DateTime::isBefore(now, showtime->getStartAt()))
            {
                return false;
            }
        }
    }
    // Tao doi tuong tam voi trang thai moi
    Movie updated(
        movie->getMovieId(),
        movie->getTitle(),
        movie->getGenre(),
        movie->getDurationMinutes(),
        movie->getMinAge(),
        movie->getReleaseDate(),
        movie->getEndDate(),
        active
    );

    return movieRepo.update(updated);
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
    // Chuyen tu khoa thanh chu thuong
    string searchKey = keyword;
    for (char& c : searchKey)
    {
        c = static_cast<char>(
            tolower(static_cast<unsigned char>(c))
        );
    }
    // Duyet danh sach phim
    for (int i = 0; i < movieRepo.count(); i++)
    {
        const Movie* movie = movieRepo.getAt(i);
        if (movie == nullptr)
        {
            continue;
        }
        string title = movie->getTitle();
        // Chuyen ten phim thanh chu thuong
        for (char& c : title)
        {
            c = static_cast<char>(
                tolower(static_cast<unsigned char>(c))
            );
        }
        // Tim tu khoa trong ten phim
        if (title.find(searchKey) != string::npos)
        {
            result.push_back(movie);
        }
    }
    return result;
}
