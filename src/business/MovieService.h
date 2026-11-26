#ifndef MOVIE_SERVICE_H
#define MOVIE_SERVICE_H

#include <string>
#include <vector>
#include "../data_access/IRepository.h"
#include "../models/Movie.h"
#include "AuthService.h"

class MovieService
{
private:
    IRepository<Movie>& movieRepo;
    AuthService& authService;

public:
    MovieService(IRepository<Movie>& movieRepo,
                 AuthService& authService);

    // Quan ly du lieu
    bool load();
    bool save();

    // Quan ly phim
    bool addMovie(Movie* movie);
    bool updateMovie(const Movie& newValues);
    bool removeMovie(const std::string& movieId);
    bool setMovieActive(const std::string& movieId, bool active);

    // Tra cuu phim
    const Movie* findMovieById(const std::string& movieId);
    const Movie* getMovieAt(int index);
    int getMovieCount();
    std::vector<const Movie*> getNowShowingMovies();
    std::vector<const Movie*> getComingSoonMovies();
    std::vector<const Movie*> searchMoviesByTitle(const std::string& keyword);
};

#endif
