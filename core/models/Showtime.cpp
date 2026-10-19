#include "Showtime.h"
using namespace std;

Showtime::Showtime()
	: showtimeId(""),
	  movieId(""),
	  roomId(""),
	  startAt(""),
	  endAt(""),
	  status(ShowtimeStatus::SCHEDULED)
{}

Showtime::Showtime(const string& showtimeId,
				   const string& movieId,
				   const string& roomId,
				   const string& startAt,
				   const string& endAt,
				   ShowtimeStatus status)
	: showtimeId(showtimeId),
	  movieId(movieId),
	  roomId(roomId),
	  startAt(startAt),
	  endAt(endAt),
	  status(status)
{}

const string& Showtime::getShowtimeId() const {
	return this->showtimeId;
}
const string& Showtime::getMovieId() const {
	return this->movieId;
}
const string& Showtime::getRoomId() const {
	return this->roomId;
}
const string& Showtime::getStartAt() const {
	return this->startAt;
}
const string& Showtime::getEndAt() const {
	return this->endAt;
}
ShowtimeStatus Showtime::getStatus() const {
	return this->status;
}

bool Showtime::isEndAfterStart() const {
	return this->endAt > this->startAt;
}

void Showtime::setStartAt(const string& startAt) {
	this->startAt = startAt;
}
void Showtime::setEndAt(const string& endAt) {
	this->endAt = endAt;
}
void Showtime::setStatus(ShowtimeStatus status) {
	this->status = status;
}
