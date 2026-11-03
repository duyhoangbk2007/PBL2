#include "Movie.h"
#include "Showtime.h"
using namespace std;

Movie::Movie(const string& movieId,
             const string& title,
             const string& genre,
             int durationMinutes,
             int minAge,
             const string& releaseDate,
             const string& endDate,
             bool active)
    : movieId(movieId),
      title(title),
      genre(genre),
      durationMinutes(durationMinutes),
      minAge(minAge),
      releaseDate(releaseDate),
      endDate(endDate),
      active(active) 
{}


const string& Movie::getMovieId() const {
    return this->movieId;
}

const string& Movie::getTitle() const {
    return this->title;
}

const string& Movie::getGenre() const {
    return this->genre;
}

int Movie::getDurationMinutes() const {
    return this->durationMinutes;
}

int Movie::getMinAge() const {
    return this->minAge;
}

const string& Movie::getReleaseDate() const {
    return this->releaseDate;
}

const string& Movie::getEndDate() const {
    return this->endDate;
}

bool Movie::isActive() const {
    return this->active;
}


void Movie::setTitle(const string& title) {
    this->title = title;
}

void Movie::setGenre(const string& genre) {
    this->genre = genre;
}

void Movie::setDurationMinutes(int durationMinutes) {
    this->durationMinutes = durationMinutes;
}

void Movie::setMinAge(int minAge) {
    this->minAge = minAge;
}

void Movie::setReleaseDate(const string& releaseDate) {
    this->releaseDate = releaseDate;
}

void Movie::setEndDate(const string& endDate) {
    this->endDate = endDate;
}

void Movie::setActive(bool active) {
    this->active = active;
}

bool Movie::linkShowtime(Showtime* showtime) {
    if (showtime == nullptr) 
        return false;
    if (showtime->getMovieId() != this->movieId) 
        return false;
    if (showtime->getMovie() != nullptr && showtime->getMovie() != this)
        return false;
    int n = this->showtimes.nodeCount();
    for (int i = 0; i < n; i++)
    {
        Showtime* current = this->showtimes.getAt(i)->data;
        if (current == showtime)
            return true;
        if (current->getShowtimeId() == showtime->getShowtimeId())
            return false;
    }
    if (showtime->getMovie() == this)
        return false;
    this->showtimes.pushBack(showtime);
    showtime->movie = this;
    return true;
}

Showtime* Movie::findShowtime(const std::string& showtimeId) {
    int n = this->showtimes.nodeCount();
    for (int i = 0; i < n; i++)
    {
        Showtime* current = this->showtimes.getAt(i)->data;
        if (current->getShowtimeId() == showtimeId)
            return current;
    }
    return nullptr;
}
int Movie::getShowtimeCount() const {
    return this->showtimes.nodeCount();
}