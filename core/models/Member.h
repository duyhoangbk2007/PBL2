#ifndef PBL2_CORE_MODELS_MEMBER_H
#define PBL2_CORE_MODELS_MEMBER_H

#include <string>

class Member {
private:
	std::string memberId;
	std::string fullName;
	std::string phone;
	int points;

public:
	Member(const std::string& memberId,
		   const std::string& fullName,
		   const std::string& phone,
		   int points);

	const std::string& getMemberId() const;
	const std::string& getFullName() const;
	const std::string& getPhone() const;
	int getPoints() const;

	void setFullName(const std::string& fullName);
	void setPhone(const std::string& phone);
	void setPoints(const int& points);
};

#endif
