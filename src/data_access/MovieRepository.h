
#ifndef MOVIE_REPOSITORY_H
#define MOVIE_REPOSITORY_H

#include <string>
#include "Repository.h"
#include "../models/Movie.h"

using namespace std;

class MovieRepository
{
private:
    Repository<Movie> repo;

    // Cac ham cung cap cho Repository<Movie>
    static string getMovieId(Movie* movie);
    static Movie* stringToMovie(const string& line);
    static string movieToString(Movie* movie);

    // Kiem tra du lieu Movie
    static bool isValidMovie(const Movie* movie);

public:
    MovieRepository(const string& filePath);

    // Doc va ghi du lieu
    bool load();
    bool save();

    // Quan ly phim
    bool add(Movie* movie);
    bool update(const Movie& newValues);
    bool removeById(const string& movieId);
    
    // Truy van phim
    Movie* findById(const string& movieId);
    Movie* getAt(int index);
    int count();
    bool isLoaded();
};

#endif
