#include "alg_file_utils.h"
#include <filesystem>
#include <iostream>
#include "bubble_paths.h"
#include <fstream>
#include <unordered_map>
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

	void saveTextFile(const std::string& path, const std::string& text)
	{
		std::ofstream outFile(path);
		if (!outFile.is_open()) {
			std::cerr << "failed to open to write\n";
		}
		outFile << text;
		outFile.flush();
		outFile.close();
	}

	void cleanString(std::string& text) {
		text.erase(
			std::remove_if(text.begin(), text.end(), [](char c) {
				return c == ',' || c == '.' || c == ';' ||
					c == ':' || c == '?';
				}),
			text.end()
		);
	}

	std::string loadText(const std::string& path) {
		std::ifstream inputFile(path);
		if (!inputFile.is_open()) {
			throw std::runtime_error("Failed to open file to open");
		}
		std::stringstream buffer;
		buffer << inputFile.rdbuf();
		return buffer.str();
	}

	WordProblem loadWordProblem(const std::string& path) {
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

			// parse
			std::unordered_map<std::string, std::string>varMap;
			std::vector<std::string>words;
			std::vector<std::string>labels;
			std::string currentWord = "";
			std::string currentLabel = "";
			SentenceUnit currentUnit;

			//cleanString(line);

			// collect words and labels
			bool bracketOpen = false;
			bool parsingVarLabel = false;
			for (int i = 0; i < line.size(); ++i) {
				char c = line[i];
				if (c == ' ') {
					if (parsingVarLabel) {
						labels.push_back(currentLabel);
						parsingVarLabel = false;
						currentLabel = "";
						continue;
					}
					else if (!bracketOpen) {
						words.push_back(currentWord);
						currentWord = "";
						continue;
					}
				}
				if (c == '[') {
					bracketOpen = true;
					parsingVarLabel = true;
					continue;
				}
				else if (c == ']') {
					bracketOpen = false;
					varMap[currentWord] = labels.back();
					continue;
				}
				if (parsingVarLabel) {
					currentLabel += c;
				}
				else {
					currentWord += c;
				}
			}
			if (currentWord != "") {
				words.push_back(currentWord);
			}

			// labels...
			for (std::string& word : words) {
				SentenceUnit unit;
				unit.text = word;
				if (varMap.find(word) != varMap.end()) {
					unit.varLabel = varMap[word];
				}
				result.sentenceUnits.push_back(unit);
			}

		}

		return result;
	}

	WordProblemMobjs loadWordProblemMobjs(const std::string& path)
	{
		WordProblemMobjs mobjs;
		std::ifstream inputFile(path);
		if (!inputFile.is_open()) {
			throw std::runtime_error("Failed to open file to open");
		}
		std::string line;
		while (std::getline(inputFile, line)) {
			if (line.find("#ver") != std::string::npos) {
				checkVersion(line);
			}
			else if (line.find("text:") != std::string::npos) {
				std::string problemFile = line.substr(5);
				mobjs.problem = std::make_shared<WordProblem>(loadWordProblem(bubblePaths::WORD_PROBLEMS_FOLDER + "/" + problemFile));
			}
			else if (line.find("mobj:") != std::string::npos) {
				std::string solutionfile = line.substr(5);
				auto scope = loadBubequ(bubblePaths::EQUATION_FOLDER + "/" + solutionfile);
				mobjs.solutionMobj = scope;
			}
			else if (line != ""){
				throw std::runtime_error("file should have: ver: text: or mobj: at each line");
			}
		}
		return mobjs;
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
