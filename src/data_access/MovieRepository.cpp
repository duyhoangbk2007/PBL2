#include "MovieRepository.h"
#include "../utils/Validator.h"
#include <stdexcept>
#include <vector>

using namespace std;

static bool isValidMovieId(const string& id)
{
    //kiểm tra độ dài và 3 kí tự đầu có phải là MOV k
    if (id.length() != 7 || id.substr(0, 3) != "MOV")
    {
        return false;
    }
    //kí tự còn lại là chữ số
    for (int i = 3; i < 7; i++)
    {
        if (id[i] < '0' || id[i] > '9')
        {
            return false;
        }
    }
    //có ít nhất 1 chữ khác 0
    return id.substr(3) != "0000";
}

// Kiem tra toan bo du lieu Movie

bool MovieRepository::isValidMovie(const Movie* movie)
{
    if (movie == nullptr)
    {
        return false;
    }
    if (!isValidMovieId(movie->getMovieId()))
    {
        return false;
    }
    if (!Validator::isValidText(movie->getTitle()) || !Validator::isValidText(movie->getGenre()))
    {
        return false;
    }
    if (movie->getDurationMinutes() <= 0)
    {
        return false;
    }
    if (movie->getMinAge() < 0)
    {
        return false;
    }
    if (!Validator::isValidDate(movie->getReleaseDate()) || !Validator::isValidDate(movie->getEndDate()))
    {
        return false;
    }
    if (movie->getReleaseDate() > movie->getEndDate())
    {
        return false;
    }
    return true;
}


MovieRepository::MovieRepository(const string& filePath)
    : repo(filePath, getMovieId, parseMovie, serializeMovie)
{}
string MovieRepository::getMovieId(Movie* movie)
{
    if (movie == nullptr)
    {
        return "";
    }
    return movie->getMovieId();
}


Movie* MovieRepository::parseMovie(const string& line)
{
    vector<string> fields; //lưu dữ liệu sau khi tách chuỗi
    size_t start = 0;
    // Tach cac truong du lieu theo dau |
    while (true)
    {
        size_t pos = line.find('|', start);
        if (pos == string::npos) // nếu tới chuỗi ở cuối line sẽ k tìm thấy | sẽ trả về npos (-1)
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
    // Chuyen du lieu so
    try
    {
        size_t pos1 = 0;
        size_t pos2 = 0;
        durationMinutes = stoi(fields[3], &pos1);
        minAge = stoi(fields[4], &pos2);
        if (pos1 != fields[3].length() || pos2 != fields[4].length())
        {
            throw runtime_error("Sai dinh dang so");
        }
    }
    catch (const exception&)
    {
        throw runtime_error("Thoi luong hoac do tuoi khong hop le");
    }

    // Kiem tra trang thai (active)
    if (fields[7] != "0" && fields[7] != "1")
    {
        throw runtime_error("Trang thai Movie khong hop le");
    }
    bool active = (fields[7] == "1");

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

// Chuyen Movie thanh mot dong TXT
string MovieRepository::serializeMovie(Movie* movie)
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

// Doc du lieu phim tu file
bool MovieRepository::load()
{
    return repo.load();
}

// Luu du lieu phim xuong file
bool MovieRepository::save()
{
    return repo.save();
}

// Them phim moi
bool MovieRepository::add(Movie* movie)
{
    if (!repo.isLoaded() || !isValidMovie(movie))
    {
        return false;
    }
    // Khong them lai con tro da duoc quan ly
    for (int i = 0; i < repo.count(); i++)
    {
        if (repo.getAt(i) == movie)
        {
            return false;
        }
    }
    return repo.add(movie);
}

// Cap nhat thong tin phim
bool MovieRepository::update(const Movie& newValues)
{
    if (!repo.isLoaded() || !isValidMovie(&newValues))
    {
        return false;
    }
    Movie* movie = repo.findById(newValues.getMovieId());

    if (movie == nullptr)
    {
        return false;
    }
    movie->setTitle(newValues.getTitle());
    movie->setGenre(newValues.getGenre());
    movie->setDurationMinutes(newValues.getDurationMinutes());
    movie->setMinAge(newValues.getMinAge());
    movie->setReleaseDate(newValues.getReleaseDate());
    movie->setEndDate(newValues.getEndDate());
    movie->setActive(newValues.isActive());
    return true;
}

// Xoa phim theo ID
bool MovieRepository::removeById(const string& movieId)
{
    if (!repo.isLoaded())
    {
        return false;
    }
    Movie* movie = repo.findById(movieId);
    if (movie == nullptr)
    {
        return false;
    }
    // Khong xoa neu phim con lien ket Showtime
    if (movie->getShowtimeCount() > 0)
    {
        return false;
    }
    return repo.removeById(movieId);
}

// Tim phim theo ID
Movie* MovieRepository::findById(const string& movieId)
{
    return repo.findById(movieId);
}

// Lay phim theo vi tri
Movie* MovieRepository::getAt(int index)
{
    return repo.getAt(index);
}

// Dem so luong phim
int MovieRepository::count()
{
    return repo.count();
}

// Kiem tra trang thai load
bool MovieRepository::isLoaded()
{
    return repo.isLoaded();
}
