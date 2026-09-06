#include "alg_file_utils.h"
#include <filesystem>
#include <iostream>
#include "bubble_paths.h"
#include <fstream>
#include <unordered_map>
#include <sstream>
#include <cassert>
#include "sha256.h"
#include "bubequ_mapping.h"

namespace bubequ {

	const std::string version = "#ver 1";

	inline std::string stripBrackets(const std::string& str) {
		if (
			(str[0] != '(' || str[str.size() -1] != ')')
			&& (str[0] != '[' || str[str.size() -1] != ']')
			) {
			throw std::runtime_error("bracket something wrong (file error)");
		}
		return str.substr(1, str.size() - 2);
	}

	inline std::string getNums(const std::string& str) {
		std::string result;
		for (int i = 0; i < str.size(); ++i) {
			if (std::isdigit(str[i])) {
				result += str[i];
			}
		}
		return result;
	}


	inline std::string getLetters(const std::string& str) {
		std::string result;
		for (int i = 0; i < str.size(); ++i) {
			if (std::isalpha(str[i])) {
				result += str[i];
			}
		}
		return result;
	}

	inline std::shared_ptr<Unit> parseUnit(const std::string& valueStr) {
		auto unit = std::make_shared<Unit>();
		if (valueStr == "") {
			unit->value = 0;
			unit->type = UnitType::ZERO;
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

	inline std::shared_ptr<Link> parseLink(const std::string& linkStr) {
		auto link = std::make_shared<Link>();
		char operatorChar = linkStr[0];
		int substringStart = 1;
		if (operatorChar == '*') {
			link->type = LinkType::MULTIPLICATION;
		}
		else if (operatorChar == '^') {
			link->type = LinkType::POWER;
		}
		else if (operatorChar == '>') {
			if (linkStr[1] == '=') {
				link->type = LinkType::GREATER_OR_EQUAL;
				++substringStart;
			}
			else {
				link->type = LinkType::GREATER;
			}
		}
		else if (operatorChar == '=') {
			link->type = LinkType::EQUALS;
		}
		else if (operatorChar == '$') {
			link->type = LinkType::SUMMATION;
		}
		else if (std::isalpha(operatorChar)) {
			link->type = LinkType::FUNCTION;
			link->label = operatorChar;
		}
		else {
			throw std::runtime_error("file formal error: Not known linktype");
		}
		std::string subStr = linkStr.substr(substringStart);
		std::vector<std::string>scopes = splitChildren(subStr);
		for (const std::string& scopeStr : scopes) {
			link->children.push_back(parseScope(scopeStr));
		}
		return link;
	}

	inline std::shared_ptr<Scope> parseScope(const std::string& line) {

		std::string scopeStr = stripBrackets(line);

		if (line[0] == '[') {
			auto scope = std::make_shared<Scope>();
			scope->hash = scopeStr;
			return scope;
		}

		if (scopeStr == "") {
			return parseUnit(scopeStr);
		}
		char operatorChar = scopeStr[0];
		char nextChar = 0;
		if (scopeStr.size() > 0) {
			nextChar = scopeStr[1];
		}

		if (operatorChar == '*' 
			|| operatorChar == '^' 
			|| operatorChar == '>'
			|| operatorChar == '='
			|| operatorChar == '$'
			)
		{
			return parseLink(scopeStr);
		}
		else if (std::isalpha(operatorChar) && nextChar == '(') {
			return parseLink(scopeStr);
		}
		else if (operatorChar == '(' || operatorChar == '[') {
			auto scope = std::make_shared<bubequ::Scope>();
			int bracketLevel = 0;
			std::string currentScopeStr = "";
			for (int i = 0; i < scopeStr.size(); ++i) {
				char c = scopeStr[i];
				if (c == '(' || c == '[') {
					++bracketLevel;
				}
				currentScopeStr += c;
				if (c == ')' || c == ']') {
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

	bool checkVersion(const std::string& line, const std::string ver) {
		return line == "#" + ver;
	}

	std::vector<std::string> splitChildren(const std::string& s) {
		std::vector<std::string>parts;
		std::string currentPart = "";
		int bracketLevel = 0;
		for (int i = 0; i < s.size(); ++i) {
			char c = s[i];
			if (c == '(' || c == '[') {
				++bracketLevel;
			}

			currentPart += c;

			if (c == ')' || c == ']') {
				--bracketLevel;
			}

			if (bracketLevel == 0) {
				parts.push_back(currentPart);
				currentPart = "";
			}
		}
		return parts;
	}

	std::shared_ptr<Scope> loadBubequ(const std::string& path) {

		std::ifstream inputFile(path);
		if (!inputFile.is_open()) {
			throw std::runtime_error("Failed to open file to open");
		}
		std::string line;
		while (std::getline(inputFile, line)) {
			if (line.find("#ver") != std::string::npos) {
				if (!checkVersion(line, "ver 1")) {
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

	void saveBubequHead(const std::string& headName, const std::string& headHash, const std::unordered_map<std::string, std::string>& map)
	{
		// write head ref
		std::string path = bubblePaths::EQUATION_FOLDER + "/" + headName + ".bubequ";
		std::ofstream outFile(path);
		if (!outFile.is_open()) {
			std::cerr << "failed to open to write\n";
		}
		// todo move
		const std::string version = "ver 2";
		outFile << "#" + version << "\n";
		outFile << headHash;
		outFile.flush();
		outFile.close();

		// write all the bubs
		for (auto& pair : map) {
			const std::string hash = pair.first;
			const std::string content = pair.second;
			std::string bubPath = bubblePaths::BUBBLE_TREE_FOLDER + "/" + hash;
			std::ofstream bubOutFile(bubPath);
			if (!bubOutFile.is_open()) {
				std::cerr << "failed to open to write\n";
			}
			bubOutFile << content;
			bubOutFile.flush();
			bubOutFile.close();
		}
	}

	std::shared_ptr<bubequ::Scope> loadBub(const std::string& bubHash, const BubTraversePath& traversePath, int loadDepth, int pathStepIndex, int depthIndex) {
		const std::string path = bubblePaths::BUBBLE_TREE_FOLDER + "/" + bubHash;
		std::ifstream inputFile(path);
		if (!inputFile.is_open()) {
			throw std::runtime_error("Failed to open file to open");
		}
		std::string line;
		if (std::getline(inputFile, line)) {
			auto scope = parseScope(line);

			// traverse path, don't load the whole tree until at destination
			if (pathStepIndex < traversePath.size()) {
				int pathDirection = traversePath[pathStepIndex];
				auto& toLoadBub = scope->children[pathDirection];
				return loadBub(toLoadBub->hash, traversePath, loadDepth, ++pathStepIndex, depthIndex);
			}


			// at end return
			if (depthIndex >= loadDepth || dynamic_cast<Unit*>(scope.get())) {
				return scope;
			}

			// load the whole tree until depth reached
			for (int i = 0; i < scope->children.size(); ++i) {
				auto& child = scope->children[i];
				if (child->hash != "") {
					auto newChild = loadBub(child->hash, traversePath, loadDepth, pathStepIndex, ++depthIndex);
					child = newChild;
				}
			}
			return scope;
		}
		throw std::runtime_error("Something wrong with the data");
	}

	std::shared_ptr<Scope> loadBubequHead(const std::string& headName, const BubTraversePath& traversePath, int loadDepth) {
		const std::string path = bubblePaths::EQUATION_FOLDER + "/" + headName;

		std::ifstream inputFile(path);
		if (!inputFile.is_open()) {
			throw std::runtime_error("Failed to open file to open");
		}
		std::string line;
		while (std::getline(inputFile, line)) {
			if (line.find("#ver") != std::string::npos) {
				if (!checkVersion(line, "ver 2")) {
					throw std::runtime_error("bubequ file version not matching");
				}
				continue;
			}

			// line after ver assumed to be hash
			std::string hash = line;
			return loadBub(hash, traversePath, loadDepth, 0, 0);
		}

		throw std::runtime_error("Something wrong with the data");

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
