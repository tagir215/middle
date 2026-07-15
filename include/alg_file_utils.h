#pragma once
#include "bubequ.h"
#include <vector>

namespace bubequ {
	Scope parseScope(const std::string& line);
	bool checkVersion(const std::string& line);
	std::vector<std::string> split(const std::string& s);
	std::string stripBrackets(const std::string& str);
	std::string getNums(const std::string& str);
	std::string getLetters(const std::string& str);
	Unit parseUnit(const std::string& valueStr);
	Link parseLink(const std::string& linkStr);
	Scope parseScope(const std::string& line);
	Scope parseBubequ(const std::string& path);
}
