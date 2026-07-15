#include "alg_file_utils.h"
#include <filesystem>
#include <iostream>
#include <fstream>
namespace bubequ {
	bool checkVersion(const std::string& line) {
		const std::string expectedLine = "ver 1";
		return line == expectedLine;
	}

	std::vector<std::string> split(const std::string& s) {
		std::vector<std::string>parts;
		std::string currentPart = "";
		int bracketLevel = 0;
		for (int i = 0; i < s.size(); ++i) {
			char c = s[i];
			if (c == '(') {
				++bracketLevel;
			}

			currentPart += c;

			if (c == ')') {
				--bracketLevel;
			}

			if (bracketLevel == 0) {
				parts.push_back(currentPart);
				currentPart = "";
			}
		}
		return parts;
	}

	std::string stripBrackets(const std::string& str) {
		if (str[0] != '(' || str[str.size() -1] != ')') {
			throw std::runtime_error("bracket something wrong (file error)");
		}
		return str.substr(1, str.size() - 2);
	}

	std::string getNums(const std::string& str) {
		std::string result;
		for (int i = 0; i < str.size(); ++i) {
			if (std::isdigit(str[i])) {
				result += str[i];
			}
		}
		return result;
	}
	std::string getLetters(const std::string& str) {
		std::string result;
		for (int i = 0; i < str.size(); ++i) {
			if (std::isalpha(str[i])) {
				result += str[i];
			}
		}
		return result;
	}

	Unit parseUnit(const std::string& valueStr) {
		Unit unit;
		if (valueStr == "") {
			unit.value = 0;
			return unit;
		}
		unit.value = 1;
		if (valueStr[0] == '-') {
			unit.value = -1;
		}
		std::string numSubstr = getNums(valueStr);
		if (numSubstr == "") {
			unit.value = 1;
		}
		else {
			unit.value = std::stoi(numSubstr);
		}
		std::string varSubstr = getLetters(valueStr);
		unit.label = varSubstr;
		if (varSubstr != "") {
			unit.type = UnitType::VARIABLE;
		}
		else {
			unit.type = UnitType::CONSTANT;
		}
		return unit;
	}

	Link parseLink(const std::string& linkStr) {
		Link link;
		char operatorChar = linkStr[0];
		if (operatorChar == '+') {
			link.type = LinkType::ADDITION;
		}
		else if (operatorChar == '*') {
			link.type = LinkType::MULTIPLICATION;
		}
		else if (operatorChar == '^') {
			link.type = LinkType::POWER;
		}
		else if (operatorChar == '=') {
			link.type == LinkType::EQUALS;
		}
		else {
			throw std::runtime_error("file formal error: Not known linktype");
		}
		std::string subStr = linkStr.substr(1);
		std::vector<std::string>scopes = split(subStr);
		for (const std::string& scopeStr : scopes) {
			link.children.push_back(parseScope(scopeStr));
		}
		return link;
	}

	Scope parseScope(const std::string& line) {
		Scope scope;
		std::string scopeStr = stripBrackets(line);

		if (scopeStr == "") {
			scope.children.push_back(parseUnit(scopeStr));
			return scope;
		}
		char operatorChar = scopeStr[0];
		if (operatorChar == '+' || operatorChar == '*' || operatorChar == '^' || operatorChar == '=') {
			scope.children.push_back(parseLink(scopeStr));
		}
		// this case is just if theres brackets within brackets as only child for some reason... 
		else if (operatorChar == '(') {
			scope.children.push_back(parseScope(scopeStr));
		}
		else {
			scope.children.push_back(parseUnit(scopeStr));
		}
		return scope;
	}

	Scope parseBubequ(const std::string& path) {

		std::ifstream inputFile(path);
		if (!inputFile.is_open()) {
			throw std::runtime_error("Failed to open file to open");
		}
		std::string line;
		while (std::getline(inputFile, line)) {
			if (line.find("ver") != std::string::npos) {
				if (!checkVersion(line)) {
					throw std::runtime_error("bubequ file version not matching");
				}
				continue;
			}
			return parseScope(line);
		}

		throw std::runtime_error("Something wrong with the data");
	}
}
