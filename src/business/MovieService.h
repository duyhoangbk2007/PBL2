#ifndef MOVIE_SERVICE_H
#define MOVIE_SERVICE_H

#include <string>
#include "../data_access/MovieRepository.h"

using namespace std;

class MovieService
{
private:
    MovieRepository& movieRepo;

public:
    MovieService(MovieRepository& movieRepo);

    // Quan ly du lieu
    bool load();
    bool save();

    // Them phim
    bool addMovie(Movie* movie);
    bool updateMovie(const Movie& newValues);
    // Tra cuu phim
    Movie* findMovieById(const string& movieId);
    Movie* getMovieAt(int index);
    int getMovieCount();
};

#endif
