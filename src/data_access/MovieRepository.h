#ifndef MOVIE_REPOSITORY_H
#define MOVIE_REPOSITORY_H

#include <string>
#include "IRepository.h"
#include "Repository.h"
#include "../models/Movie.h"


class MovieRepository : public IRepository<Movie>
{
private:
    Repository<Movie> repo;

    static std::string getMovieId(Movie* movie);
    static Movie* stringToMovie(const std::string& line);
    static std::string movieToString(Movie* movie);

    static bool isValidMovie(const Movie* movie);

public:
    MovieRepository(const std::string& filePath);

    // Doc va ghi du lieu
    bool load() override;
    bool save() override;

    // Quan ly du lieu
    bool add(Movie* movie) override;
    bool update(const Movie& newValues) override;
    bool removeById(const std::string& movieId) override;

    // Truy van du lieu
    Movie* findById(const std::string& movieId) override;
    Movie* getAt(int index) override;
    int count() override;
    bool isLoaded() override;
};

#endif