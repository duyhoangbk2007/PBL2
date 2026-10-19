#ifndef PBL2_CORE_MODELS_MOVIE_H
#define PBL2_CORE_MODELS_MOVIE_H

#include <string>

class Movie {
private:
    std::string movieId;
    std::string title;
    std::string genre;
    int durationMinutes;
    int minAge;
    std::string releaseDate;
    std::string endDate;
    bool active;
public:
    Movie();
    Movie(const std::string& movieId,
          const std::string& title,
          const std::string& genre,
          int durationMinutes,
          int minAge,
          const std::string& releaseDate,
          const std::string& endDate,
          bool active);
    //getter
    const std::string& getMovieId() const;
    const std::string& getTitle() const;
    const std::string& getGenre() const;
    int getDurationMinutes() const;
    int getMinAge() const;
    const std::string& getReleaseDate() const;
    const std::string& getEndDate() const;
    bool isActive() const;
    //setter
    void setTitle(const std::string& title);
    void setGenre(const std::string& genre);
    void setDurationMinutes(int durationMinutes);
    void setMinAge(int minAge);
    void setReleaseDate(const std::string& releaseDate);
    void setEndDate(const std::string& endDate);
    void setActive(bool active);
};

#endif
