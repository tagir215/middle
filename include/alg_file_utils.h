#pragma once
#include "bubequ.h"
#include <vector>
 
namespace bubequ {
	bool checkVersion(const std::string& line);
	std::vector<std::string> split(const std::string& s);
	std::string stripBrackets(const std::string& str);
	std::string getNums(const std::string& str);
	std::string getLetters(const std::string& str);
	std::shared_ptr<Unit> parseUnit(const std::string& valueStr);
	std::shared_ptr<Link>parseLink(const std::string& linkStr);
	std::shared_ptr<Scope>parseScope(const std::string& line);
	std::shared_ptr<Scope>parseBubequ(const std::string& path);
	void saveBubequ(const std::string& bubequ);
}
