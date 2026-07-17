#include "alg_file_utils.h"
#include <filesystem>
#include <iostream>
#include "bubble_paths.h"
#include <fstream>
#include <sstream>

namespace bubequ {

	const std::string version = "#ver 1";

	bool checkVersion(const std::string& line) {
		return line == version;
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

	std::shared_ptr<Unit> parseUnit(const std::string& valueStr) {
		auto unit = std::make_shared<Unit>();
		if (valueStr == "") {
			unit->value = 0;
			return unit;
		}
		unit->value = 1;
		bool isNegative = valueStr[0] == '-';
		std::string numSubstr = getNums(valueStr);
		if (numSubstr == "") {
			unit->value = 1;
		}
		else {
			unit->value = std::stoi(numSubstr);
		}
		if (isNegative) {
			unit->value *= -1;
		}
		std::string varSubstr = getLetters(valueStr);
		unit->label = varSubstr;
		if (varSubstr != "") {
			unit->type = UnitType::VARIABLE;
		}
		else {
			unit->type = UnitType::CONSTANT;
		}
		return unit;
	}

	std::shared_ptr<Link> parseLink(const std::string& linkStr) {
		auto link = std::make_shared<Link>();
		char operatorChar = linkStr[0];
		if (operatorChar == '*') {
			link->type = LinkType::MULTIPLICATION;
		}
		else if (operatorChar == '^') {
			link->type = LinkType::POWER;
		}
		else if (operatorChar == '=') {
			link->type = LinkType::EQUALS;
		}
		else {
			throw std::runtime_error("file formal error: Not known linktype");
		}
		std::string subStr = linkStr.substr(1);
		std::vector<std::string>scopes = split(subStr);
		for (const std::string& scopeStr : scopes) {
			link->children.push_back(parseScope(scopeStr));
		}
		return link;
	}

	std::shared_ptr<Scope> parseScope(const std::string& line) {
		std::string scopeStr = stripBrackets(line);

		if (scopeStr == "") {
			return parseUnit(scopeStr);
		}
		char operatorChar = scopeStr[0];
		if (operatorChar == '*' || operatorChar == '^') {
			return parseLink(scopeStr);
		}
		else if (operatorChar == '=') {
			return parseLink(scopeStr);
		}
		else if (operatorChar == '(') {
			auto scope = std::make_shared<bubequ::Scope>();
			int bracketLevel = 0;
			std::string currentScopeStr;
			for (int i = 0; i < scopeStr.size(); ++i) {
				char c = scopeStr[i];
				if (c == '(') {
					++bracketLevel;
				}
				currentScopeStr += c;
				if (c == ')') {
					--bracketLevel;
				}
				if (bracketLevel == 0) {
					scope->children.push_back(parseScope(currentScopeStr));
					currentScopeStr = "";
				}
			}
			return scope;
		}
		else {
			return parseUnit(scopeStr);
		}
	}

	std::shared_ptr<Scope> loadBubequ(const std::string& path) {

		std::ifstream inputFile(path);
		if (!inputFile.is_open()) {
			throw std::runtime_error("Failed to open file to open");
		}
		std::string line;
		while (std::getline(inputFile, line)) {
			if (line.find("#ver") != std::string::npos) {
				if (!checkVersion(line)) {
					throw std::runtime_error("bubequ file version not matching");
				}
				continue;
			}
			return parseScope(line);
		}

		throw std::runtime_error("Something wrong with the data");
	}
	void saveBubequ(const std::string& equname, const std::string& bubequ)
	{	
		std::string path = bubblePaths::EQUATION_FOLDER + "/" + equname + ".bubequ";
		std::ofstream outFile(path);
		if (!outFile.is_open()) {
			std::cerr << "failed to open to write\n";
		}

		outFile << version << "\n";
		outFile << bubequ;
		outFile.flush();
		outFile.close();
	}

	void savePuzzleText(const std::string& title, const std::string& text)
	{
		const std::string path = bubblePaths::WORD_PROBLEMS_FOLDER + "/" + title + ".txt";
		std::ofstream outFile(path);
		if (!outFile.is_open()) {
			std::cerr << "failed to open to write\n";
		}
		outFile << text;
		outFile.flush();
		outFile.close();
	}

	WordProblem loadPuzzleText(const std::string& path) {
		std::ifstream inputFile(path);
		if (!inputFile.is_open()) {
			throw std::runtime_error("Failed to open file to open");
		}

		WordProblem result;
		std::string line;
		while (std::getline(inputFile, line)) {
			if (line.find("#ver") != std::string::npos) {
				if (!checkVersion(line)) {
					throw std::runtime_error("bubequ file version not matching");
				}
				continue;
			}

			result.rawText += line + '\n';

			if (line.find("equ:") != std::string::npos) {
				std::string equname = line.substr(4);
				std::string path = bubblePaths::EQUATION_FOLDER + "/" + equname;
				auto bubequ = loadBubequ(path);
				result.bubequs.push_back(bubequ);
				continue;
			}
			
			std::istringstream iss(line);
			std::string word;
			while(iss >> word) {
				SentenceUnit unit;
				if (word.size() > 1 && std::isdigit(word[0]) && word[1] == '(') {
					unit.text = word.substr(1, word.size() - 2);
					// reduce 1 cause in data 1 means index 0
					unit.bubequIndex = word[0] - '0' -1;
				}
				else {
					unit.text = word;
				}
				result.sentenceUnits.push_back(unit);
			}

		}

		return result;
	}

	std::vector<std::string> getFilenames(const std::string directoryPath)
	{
		std::vector<std::string>files;
		try {
			for (const auto& entry : std::filesystem::directory_iterator(directoryPath)) {
				if (std::filesystem::is_regular_file(entry.status())) {
					files.push_back(entry.path().filename().string());
				}
			}
		}
		catch (const std::filesystem::filesystem_error& e) {
			std::cerr << "Error: " << e.what() << '\n';
		}

		return files;
	}
}
