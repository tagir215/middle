#pragma once
#include "bubequ.h"
#include <vector>
#include <unordered_map>
 
namespace bubequ {
	std::string stripBrackets(const std::string & str);
	std::string getNums(const std::string& str);
	std::string getLetters(const std::string& str);
	std::shared_ptr<Scope> parseScope(const std::string& line);
	std::shared_ptr<Unit> parseUnit(const std::string& valueStr);
	std::shared_ptr<Link> parseLink(const std::string& linkStr);
	bool checkVersion(const std::string& line, const std::string ver);
	std::vector<std::string> split(const std::string& s);
	std::shared_ptr<Scope>loadBubequ(const std::string& path);
	void saveBubequ(const std::string& equname, const std::string& bubequ);
	void saveTextFile(const std::string& title, const std::string& text);
	std::string loadText(const std::string& path);
	std::vector<std::string>getFilenames(const std::string directoryPath);
	void saveBubequHead(const std::string& head, const std::string& headHash, const std::unordered_map<std::string, std::string>& map);
	std::shared_ptr<Scope> loadBubequHead(const std::string& headName, const BubTraversePath& traversePath, int loadDepth);
}
