#ifndef MOVIE_SERVICE_H
#define MOVIE_SERVICE_H

#include <string>
#include "../data_access/MovieRepository.h"
#include "AuthService.h"

using namespace std;

class MovieService
{
private:
    MovieRepository& movieRepo;
    AuthService& authService;
public:
    MovieService(MovieRepository& movieRepo, AuthService& authService);

    // Quan ly du lieu
    bool load();
    bool save();

    // Them phim
    bool addMovie(Movie* movie);
    bool updateMovie(const Movie& newValues);
    bool removeMovie(const string& movieId);
    // Tra cuu phim
    const Movie* findMovieById(const string& movieId);
    const Movie* getMovieAt(int index);
    int getMovieCount();
};

#endif
