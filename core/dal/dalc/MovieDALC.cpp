#include "MovieDALC.h"
#include "../../utils/FileHandler.h"
#include "../../utils/TransactionManager.h"

#include <stdexcept>
#include <vector>

using namespace std;

// Kiem tra ma phim MOV0001 -> MOV9999
static bool isValidMovieId(const string& id)
{
    if (id.length() != 7)
        return false;

    if (id.substr(0, 3) != "MOV")
        return false;

    for (int i = 3; i < 7; i++)
    {
        if (id[i] < '0' || id[i] > '9')
            return false;
    }

    if (id.substr(3) == "0000")
        return false;

    return true;
}
// Kiem tra chuoi van ban
static bool isValidText(const string& text)
{
    if (text.empty())
        return false;

    for (char c : text)
    {
        unsigned char ch = static_cast<unsigned char>(c);

        if (c == '|' || ch < 32 || ch == 127)
            return false;
    }

    return true;
}
// Kiem tra ngay YYYY-MM-DD
static bool isValidDate(const string& date)
{
    if (date.length() != 10)
        return false;

    if (date[4] != '-' || date[7] != '-')
        return false;

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

    int days[] = {
        31, 28, 31, 30, 31, 30,
        31, 31, 30, 31, 30, 31
    };

    bool leapYear =
        (year % 400 == 0) ||
        (year % 4 == 0 && year % 100 != 0);

    if (leapYear)
        days[1] = 29;

    return day >= 1 && day <= days[month - 1];
}
// Kiem tra toan bo thong tin Movie
static bool isValidMovie(const Movie* movie)
{
    if (movie == nullptr)
        return false;

    if (!isValidMovieId(movie->getMovieId()))
        return false;

    if (!isValidText(movie->getTitle()))
        return false;

    if (!isValidText(movie->getGenre()))
        return false;

    if (movie->getDurationMinutes() <= 0)
        return false;

    if (movie->getMinAge() < 0)
        return false;

    if (!isValidDate(movie->getReleaseDate()))
        return false;

    if (!isValidDate(movie->getEndDate()))
        return false;

    if (movie->getReleaseDate() > movie->getEndDate())
        return false;
    return true;
}

MovieDALC::MovieDALC(const string& filePath)
    : filePath(filePath), loaded(false)
{}
MovieDALC::~MovieDALC()
{
    clearMovies();
}

void MovieDALC::clearMovies()
{
    while (movies.nodeCount() > 0)
    {
        ListNode<Movie*>* node = movies.getAt(0);
        delete node->data;
        movies.popFront();
    }
}

int MovieDALC::count()
{
    return movies.nodeCount();
}
Movie* MovieDALC::getAt(int index)
{
    ListNode<Movie*>* node = movies.getAt(index);
    if (node == nullptr)
        return nullptr;

    return node->data;
}
Movie* MovieDALC::findById(const string& movieId)
{
    int n = movies.nodeCount();
    for (int i = 0; i < n; i++)
    {
        Movie* movie = getAt(i);
        if (movie->getMovieId() == movieId)
            return movie;
    }
    return nullptr;
}

Movie* MovieDALC::StringToMovie(const string& line)
{
    string fields[8]; //mỗi phần tử lưu 1 chuỗi thuộc tính của phim
    size_t start = 0;
    // Tach chuoi thanh 8 truong
    for (int i = 0; i < 7; i++)
    {
        size_t pos = line.find('|', start); //tìm dấu | kể từ start
        if (pos == string::npos)
        {
            throw runtime_error("Movie thieu truong");
        }
        fields[i] = line.substr(start, pos - start);
        start = pos + 1;
    }

    // Lay truong cuoi cung
    fields[7] = line.substr(start);

    // Kiem tra du truong
    if (fields[7].find('|') != string::npos)
    {
        throw runtime_error("Movie du truong");
    }
    int durationMinutes;
    int minAge;
    try
    {
        size_t pos1 = 0;
        size_t pos2 = 0;

        //pos ở đây là dùng để xử lý trường hợp có kí tự chữ như 180abc thì nó chỉ đếm 180 là 3 kí tự, sau đó đem so với length()
        durationMinutes = stoi(fields[3], &pos1); //stoi(180, 3)
        minAge = stoi(fields[4], &pos2);

        if (pos1 != fields[3].length() || pos2 != fields[4].length())
        {
            throw runtime_error("Gia tri so khong hop le");
        }
    }
    catch (const exception&)
    {
        throw runtime_error("Thoi luong hoac do tuoi sai");
    }

    // Kiem tra trang thai
    if (fields[7] != "0" && fields[7] != "1")
    {
        throw runtime_error("Trang thai Movie sai");
    }
    bool active = (fields[7] == "1"); // nếu là 1 thì active = true

    // Tao doi tuong Movie
    Movie* movie = new Movie(
        fields[0],
        fields[1],
        fields[2],
        durationMinutes,
        minAge,
        fields[5],
        fields[6],
        active
    );
    // Kiem tra du lieu Movie
    if (!isValidMovie(movie))
    {
        delete movie;
        throw runtime_error("Du lieu Movie khong hop le");
    }
    return movie;
}
string MovieDALC::MovieToString(Movie* movie)
{
    if (!isValidMovie(movie))
    {
        throw runtime_error("Movie khong hop le");
    }

    string line = "";

    line += movie->getMovieId() + "|";
    line += movie->getTitle() + "|";
    line += movie->getGenre() + "|";
    line += to_string(movie->getDurationMinutes()) + "|";
    line += to_string(movie->getMinAge()) + "|";
    line += movie->getReleaseDate() + "|";
    line += movie->getEndDate() + "|";
    line += (movie->isActive() ? "1" : "0");

    return line;
}

void MovieDALC::load()
{
    if (loaded || movies.nodeCount() > 0)
    {
        throw runtime_error("Danh sach Movie da co du lieu");
    }
    vector<string> lines;
    // Doc du lieu tu file
    if (!FileHandler::readlines(filePath, lines))
    {
        throw runtime_error("Khong doc duoc file Movie");
    }
    try
    {
        // Bo qua dong schema, bat dau tu dong thu hai
        for (int i = 1; i < (int)lines.size(); i++)
        {
            Movie* movie = StringToMovie(lines[i]);

            // Kiem tra trung ma
            if (findById(movie->getMovieId()) != nullptr)
            {
                delete movie;
                throw runtime_error("Movie ID bi trung");
            }
            movies.pushBack(movie);
        }
    }
    catch (...)
    {
        clearMovies();
        throw;
    }
    loaded = true;
}

void MovieDALC::save()
{
    if (!loaded)
    {
        throw runtime_error("MovieDALC chua load du lieu");
    }
    vector<string> lines;
    // Chuyen tung Movie thanh chuoi
    for (int i = 0; i < movies.nodeCount(); i++)
    {
        Movie* movie = movies.getAt(i)->data;
        string line = MovieToString(movie);
        lines.push_back(line);
    }

    // Tao du lieu ghi file
    TransactionFile file;
    file.path = filePath;
    file.newLines = lines;

    vector<TransactionFile> files;
    files.push_back(file);
    // Ghi file thong qua TransactionManager
    TransactionManager::execute(files);
}

bool MovieDALC::add(Movie* movie)
{
    if (!loaded)
    {
        return false;
    }
    if (!isValidMovie(movie))
    {
        return false;
    }
    // Kiem tra trung ma phim
    if (findById(movie->getMovieId()) != nullptr)
    {
        return false;
    }
    // Them phim vao danh sach
    movies.pushBack(movie);

    return true;
}

bool MovieDALC::update(const Movie& newValues)
{
    if (!loaded)
    {
        return false;
    }
    // Tim phim can cap nhat theo ID
    Movie* movie = findById(newValues.getMovieId());
    if (movie == nullptr)
    {
        return false;
    }
    // Kiem tra du lieu moi
    if (!isValidMovie(&newValues))
    {
        return false;
    }
    // Cap nhat thong tin phim
    movie->setTitle(newValues.getTitle());
    movie->setGenre(newValues.getGenre());
    movie->setDurationMinutes(newValues.getDurationMinutes());
    movie->setMinAge(newValues.getMinAge());
    movie->setReleaseDate(newValues.getReleaseDate());
    movie->setEndDate(newValues.getEndDate());
    movie->setActive(newValues.isActive());

    return true;
}

bool MovieDALC::removeById(const string& movieId)
{
    if (!loaded)
    {
        return false;
    }
    int n = movies.nodeCount();
    for (int i = 0; i < n; i++)
    {
        Movie* movie = movies.getAt(i)->data;

        // Tim phim co ma can xoa
        if (movie->getMovieId() == movieId)
        {
            // Khong xoa phim co suat chieu
            if (movie->getShowtimeCount() > 0)
            {
                return false;
            }
            delete movie;
            movies.erase(i);
            return true;
        }
    }
    return false;
}

