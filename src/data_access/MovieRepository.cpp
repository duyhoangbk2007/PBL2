#include "MovieRepository.h"
#include "../utils/FileIO.h"

#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;

static bool isValidMovieId(const string& id)
{
    if (id.length() != 7 || id.substr(0, 3) != "MOV")
        return false;

    for (int i = 3; i < 7; i++)
    {
        if (id[i] < '0' || id[i] > '9')
            return false;
    }

    return id.substr(3) != "0000";
}
// Khong cho phep chuoi rong hoac ky tu sai dinh dang TXT
static bool isValidText(const string& text)
{
    if (text.empty())
        return false;

    for (unsigned char c : text)
    {
        if (c == '|' || c < 32 || c == 127)
            return false;
    }

    return true;
}

// Kiem tra ngay YYYY-MM-DD, bao gom nam nhuan
static bool isValidDate(const string& date)
{
    if (date.length() != 10 ||
        date[4] != '-' || date[7] != '-')
    {
        return false;
    }

    for (int i = 0; i < 10; i++)
    {
        if (i == 4 || i == 7)
            continue;

        if (date[i] < '0' || date[i] > '9')
            return false;
    }

    int year = stoi(date.substr(0, 4));
    int month = stoi(date.substr(5, 2));
    int day = stoi(date.substr(8, 2));

    if (year < 1 || month < 1 || month > 12)
        return false;

    int daysInMonth[12] = {
        31, 28, 31, 30, 31, 30,
        31, 31, 30, 31, 30, 31
    };

    bool leapYear = (year % 400 == 0) ||
                    (year % 4 == 0 && year % 100 != 0);

    if (leapYear)
        daysInMonth[1] = 29;

    return day >= 1 && day <= daysInMonth[month - 1];
}

// Kiem tra du lieu co ban cua Movie
static bool isValidMovie(const Movie* movie)
{
    if (movie == nullptr)
        return false;

    if (!isValidMovieId(movie->getMovieId()))
        return false;

    if (!isValidText(movie->getTitle()) ||
        !isValidText(movie->getGenre()))
    {
        return false;
    }

    if (movie->getDurationMinutes() <= 0 ||
        movie->getMinAge() < 0)
    {
        return false;
    }

    if (!isValidDate(movie->getReleaseDate()) ||
        !isValidDate(movie->getEndDate()))
    {
        return false;
    }

    if (movie->getReleaseDate() > movie->getEndDate())
        return false;

    return true;
}

MovieRepository::MovieRepository(const string& filePath)
    : filePath(filePath), loaded(false)
{
}

MovieRepository::~MovieRepository()
{
    clearMovies();
}

// Xoa cac Movie va giai phong bo nho
void MovieRepository::clearMovies()
{
    while (movies.nodeCount() > 0)
    {
        ListNode<Movie*>* node = movies.getAt(0);
        delete node->data;
        movies.popFront();
    }
}

Movie* MovieRepository::stringToMovie(const string& line)
{
    vector<string> fields;
    size_t start = 0;
    // Tach cac truong theo dau |
    while (true)
    {
        size_t pos = line.find('|', start);

        if (pos == string::npos)
        {
            fields.push_back(line.substr(start));
            break;
        }

        fields.push_back(line.substr(start, pos - start));
        start = pos + 1;
    }

    // Movie phai co dung 8 truong
    if (fields.size() != 8)
    {
        throw runtime_error("Sai so luong truong Movie");
    }

    int durationMinutes;
    int minAge;

    try
    {
        size_t pos1 = 0;
        size_t pos2 = 0;

        durationMinutes = stoi(fields[3], &pos1);
        minAge = stoi(fields[4], &pos2);

        if (pos1 != fields[3].length() ||
            pos2 != fields[4].length())
        {
            throw runtime_error("Sai gia tri so");
        }
    }
    catch (const exception&)
    {
        throw runtime_error("Thoi luong hoac do tuoi khong hop le");
    }

    if (fields[7] != "0" && fields[7] != "1")
    {
        throw runtime_error("Trang thai Movie khong hop le");
    }

    bool active = (fields[7] == "1");

    // unique_ptr giup giai phong neu du lieu khong hop le
    unique_ptr<Movie> movie(new Movie(
        fields[0],
        fields[1],
        fields[2],
        durationMinutes,
        minAge,
        fields[5],
        fields[6],
        active
    ));

    if (!isValidMovie(movie.get()))
    {
        throw runtime_error("Du lieu Movie khong hop le");
    }
    return movie.release();
}

string MovieRepository::movieToString(const Movie* movie) const
{
    if (!isValidMovie(movie))
    {
        throw runtime_error("Movie khong hop le khi ghi file");
    }

    return movie->getMovieId() + "|" +
           movie->getTitle() + "|" +
           movie->getGenre() + "|" +
           to_string(movie->getDurationMinutes()) + "|" +
           to_string(movie->getMinAge()) + "|" +
           movie->getReleaseDate() + "|" +
           movie->getEndDate() + "|" +
           (movie->isActive() ? "1" : "0");
}

void MovieRepository::load()
{
    if (loaded || movies.nodeCount() > 0)
    {
        throw runtime_error("MovieRepository da co du lieu");
    }
    vector<string> lines;
    if (!FileIO::readLines(filePath, lines))
    {
        throw runtime_error("Khong doc duoc file: " + filePath);
    }
    try
    {
        for (size_t i = 0; i < lines.size(); i++)
        {
            string line = lines[i];
            try
            {
                unique_ptr<Movie> movie(stringToMovie(line));

                if (findById(movie->getMovieId()) != nullptr)
                {
                    throw runtime_error("Trung ma Movie");
                }
                movies.pushBack(movie.get());
                movie.release();
            }
            catch (const exception& e)
            {
                throw runtime_error(
                    filePath + " - dong " +
                    to_string(i + 1) + ": " + e.what()
                );
            }
        }
    }
    catch (...)
    {
        clearMovies();
        loaded = false;
        throw;
    }

    loaded = true;
}

void MovieRepository::save()
{
    if (!loaded)
    {
        throw runtime_error("MovieRepository chua load du lieu");
    }

    vector<string> lines;

    // Chuyen toan bo Movie trong RAM thanh dong TXT
    for (int i = 0; i < count(); i++)
    {
        lines.push_back(movieToString(getAt(i)));
    }

    if (!FileIO::writeLines(filePath, lines))
    {
        throw runtime_error("Khong ghi duoc file: " + filePath);
    }
}

bool MovieRepository::add(Movie* movie)
{
    if (!loaded || !isValidMovie(movie))
        return false;

    for (int i = 0; i < count(); i++)
    {
        if (getAt(i) == movie)
            return false;
    }

    if (findById(movie->getMovieId()) != nullptr)
        return false;

    movies.pushBack(movie);
    return true;
}

bool MovieRepository::update(const Movie& newValues)
{
    if (!loaded || !isValidMovie(&newValues))
        return false;

    for (int i = 0; i < movies.nodeCount(); i++)
    {
        Movie* movie = movies.getAt(i)->data;

        if (movie->getMovieId() == newValues.getMovieId())
        {
            movie->setTitle(newValues.getTitle());
            movie->setGenre(newValues.getGenre());
            movie->setDurationMinutes(newValues.getDurationMinutes());
            movie->setMinAge(newValues.getMinAge());
            movie->setReleaseDate(newValues.getReleaseDate());
            movie->setEndDate(newValues.getEndDate());
            movie->setActive(newValues.isActive());
            return true;
        }
    }

    return false;
}

bool MovieRepository::removeById(const string& movieId)
{
    if (!loaded)
        return false;

    for (int i = 0; i < movies.nodeCount(); i++)
    {
        Movie* movie = movies.getAt(i)->data;

        if (movie->getMovieId() == movieId)
        {
            // Chan xoa neu da co lien ket Showtime trong RAM
            if (movie->getShowtimeCount() > 0)
                return false;

            // Service van phai xac minh tham chieu du lieu
            delete movie;
            movies.erase(i);

            return true;
        }
    }

    return false;
}

const Movie* MovieRepository::findById(const string& movieId) const
{
    for (int i = 0; i < count(); i++)
    {
        const Movie* movie = getAt(i);

        if (movie->getMovieId() == movieId)
            return movie;
    }

    return nullptr;
}

const Movie* MovieRepository::getAt(int index) const
{
    // DoublyLinkedList::getAt chua khai bao const.
    // Chi doc node, khong thay doi danh sach.
    auto& list =
        const_cast<DoublyLinkedList<Movie*>&>(movies);

    ListNode<Movie*>* node = list.getAt(index);

    if (node == nullptr)
        return nullptr;

    return node->data;
}

int MovieRepository::count() const
{
    return movies.nodeCount();
}
