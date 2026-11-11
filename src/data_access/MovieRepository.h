
#ifndef PBL2_MOVIE_REPOSITORY_H
#define PBL2_MOVIE_REPOSITORY_H

#include <string>

#include "../models/Movie.h"
#include "../data_structures/DoublyLinkedList.h"

class MovieRepository {
private:
    std::string filePath;
    DoublyLinkedList<Movie*> movies;
    bool loaded;
    Movie* stringToMovie(const std::string& line);
    std::string movieToString(const Movie* movie) const;
    void clearMovies();
public:
    explicit MovieRepository(const std::string& filePath);
    ~MovieRepository();
    void load();
    void save();

    bool add(Movie* movie);
    bool update(const Movie& newValues);
    bool removeById(const std::string& movieId);

    const Movie* findById(const std::string& movieId) const;
    const Movie* getAt(int index) const;
    int count() const;
};

#endif
