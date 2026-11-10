#ifndef PBL2_MOVIE_DALC_H
#define PBL2_MOVIE_DALC_H

#include "../models/Movie.h"
#include "../../data_structures/DoublyLinkedList.h"
#include <string>

class MovieDALC {
private:
    std::string filePath;
    DoublyLinkedList<Movie*> movies;
    bool loaded; //đánh dấu xem phim đã nạp dữ liệu chưa

    Movie* StringToMovie(const std::string& line);
    std::string MovieToString(Movie* movie);
    void clearMovies();
public:
    MovieDALC(const std::string& filePath);
    ~MovieDALC();
    // Doc / ghi du lieu
    void load();
    void save();

    // CRUD du lieu
    bool add(Movie* movie);
    bool update(const Movie& newValues);
    bool removeById(const std::string& movieId);
    // Tra cuu
    Movie* findById(const std::string& movieId);
    Movie* getAt(int index);
    int count();
};
#endif
  