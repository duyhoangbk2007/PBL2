#ifndef PBL2_CORE_MODELS_SHOWTIME_H
#define PBL2_CORE_MODELS_SHOWTIME_H

#include <string>

enum class ShowtimeStatus {
	SCHEDULED,
	CANCELLED
};

class Showtime {
private:
	std::string showtimeId;
	std::string movieId;
	std::string roomId;
	std::string startAt;
	std::string endAt;
	ShowtimeStatus status;

public:
	Showtime();
	Showtime(const std::string& showtimeId,
			 const std::string& movieId,
			 const std::string& roomId,
			 const std::string& startAt,
			 const std::string& endAt,
			 ShowtimeStatus status);

	const std::string& getShowtimeId() const;
	const std::string& getMovieId() const;
	const std::string& getRoomId() const;
	const std::string& getStartAt() const;
	const std::string& getEndAt() const;
	ShowtimeStatus getStatus() const;
	bool isEndAfterStart() const;

	void setStartAt(const std::string& startAt);
	void setEndAt(const std::string& endAt);
	void setStatus(ShowtimeStatus status);
};

#endif
