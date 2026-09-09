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

	void saveBubequHead(const std::string& headName, const std::string& headHash, const std::unordered_map<std::string, std::string>& map, const BubTraversePath& traversePath)
	{
		// write head ref
		std::string path = bubblePaths::EQUATION_FOLDER + "/" + headName + ".bubequ";

		bool fileExists = std::filesystem::exists(path);

		std::vector<std::string>lines;

		if (fileExists) {
			// read lines
			std::ifstream istream(path);
			if (!istream.is_open()) {
				std::cerr << "failed to open\n";
			}
			std::string line;
			while (std::getline(istream, line)) {
				lines.push_back(line);
			}
			istream.close();
			assert(checkVersion(lines[0], bubbleFileVersions::EQUATION_FILE_VERSION));
		}

		if (!fileExists) {
			lines.push_back("#" + bubbleFileVersions::EQUATION_FILE_VERSION);
		}

		// line 1 is hash
		lines.push_back(headHash);
		// line 2 is traverse path
		std::string pathLine;
		for (int i : traversePath) {
			pathLine += std::to_string(i) + " ";
		}
		lines.push_back(pathLine);
		// line 3 is date and time
		auto now = std::chrono::system_clock::now();
		std::time_t end_time = std::chrono::system_clock::to_time_t(now);
		std::string timeStr = std::ctime(&end_time);
		// pop \n 
		timeStr.pop_back();
		lines.push_back(timeStr);

		std::ofstream outFile(path);
		if (!outFile.is_open()) {
			std::cerr << "failed to open to write\n";
		}
		for (auto& line : lines) {
			outFile << line << "\n";
		}
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
				return loadBub(toLoadBub->hash, traversePath, loadDepth, pathStepIndex + 1, depthIndex);
			}

			// at end return
			if (depthIndex >= loadDepth || dynamic_cast<Unit*>(scope.get())) {
				return scope;
			}

			// load the whole tree until depth reached
			for (int i = 0; i < scope->children.size(); ++i) {
				auto& child = scope->children[i];
				if (child->hash != "") {
					auto newChild = loadBub(child->hash, traversePath, loadDepth, pathStepIndex, depthIndex + 1);
					child = newChild;
				}
			}
			return scope;
		}
		throw std::runtime_error("Something wrong with the data");
	}

	BubTraversePath stringToBubPath(const std::string& s) {
		std::vector<int>result;
		if (s == "" || s == " ") {
			return  result;
		}
		std::string buffer = "";
		for (char c : s) {
			if (c == ' ' || c == '\n') {
				result.push_back(std::stoi(buffer));
				continue;
			}
			buffer += c;
		}
		return result;
	}

	std::vector<std::string>loadBubequLines(const std::string& headName) {
		const std::string path = bubblePaths::EQUATION_FOLDER + "/" + headName + ".bubequ";
		std::ifstream inputFile(path);
		if (!inputFile.is_open()) {
			throw std::runtime_error("Failed to open file to open");
		}
		std::vector<std::string>lines;
		std::string line;
		while (std::getline(inputFile, line)) {
			lines.push_back(line);
		}
		assert(checkVersion(lines[0], bubbleFileVersions::EQUATION_FILE_VERSION));
		while (lines.back() == "" || lines.back() == " " || lines.back() == "\n") {
			lines.pop_back();
		}
		return lines;
	}

	// load with inputted traverse path
	std::shared_ptr<Scope> loadBubequHead(const std::string& headName, const BubTraversePath& traversePath, int loadDepth) {
		auto lines = loadBubequLines(headName);
		const int hashIndexOffset = -3;
		std::string hash = lines[lines.size() + hashIndexOffset];
		return loadBub(hash, traversePath, loadDepth, 0, 0);

		throw std::runtime_error("Something wrong with the data");

	}

	// load with traverse path found from the file and history offset
	std::shared_ptr<Scope> loadBubequHead(middle::GameState* gameState, const std::string& headName, int loadDepth, int historyOffset)
	{
		auto lines = loadBubequLines(headName);
		const int hashIndexOffset = -3;
		const int pathIndexOffset = -2;
		const int elementSize = 3;
		int elementOffset = elementSize * historyOffset;
		// if trying to load older histories than there exist just return the most recent one
		if (elementOffset > lines.size()) {
			elementOffset = 0;
		}
		int hashOffset = -elementOffset + hashIndexOffset;
		int pathOffset = -elementOffset + pathIndexOffset;
		std::string hash = lines[lines.size() + hashOffset];
		std::string pathString = lines[lines.size() + pathOffset];
		BubTraversePath loadedPath = stringToBubPath(pathString);
		gameState->bubbleAlgebraState.traversePath = loadedPath;
		return loadBub(hash, loadedPath, loadDepth, 0, 0);

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
					files.push_back(entry.path().filename().stem().string());
				}
			}
		}
		catch (const std::filesystem::filesystem_error& e) {
			std::cerr << "Error: " << e.what() << '\n';
		}

		return files;
	}


	void saveBubble(middle::GameState* gameState, middle::Id id, const std::string& name)
	{
		std::string path = bubblePaths::EQUATION_FOLDER + "/" + name + ".bubequ";
		bool fileExists = std::filesystem::exists(path);
		auto& traversePath = gameState->bubbleAlgebraState.traversePath;

		std::shared_ptr<bubequ::Scope> root;
		if (fileExists) {
			middle::Id backgroundId = gameState->bubbleAlgebraState.backgroundBubbleId;
			auto newBranch = bubequ::bubbleToBubequ(gameState, backgroundId);
			root = bubequ::loadBubequHead(name, {}, 400);
			// load root from disc
			// replace current visible branch on the loaded tree
			bubequ::replaceBranch(root, newBranch, traversePath);
		}
		else {
			root = bubequ::bubbleToBubequ(gameState, id);
		}
		// convert to hashes and save head reference
		std::unordered_map<std::string, std::string>hashMap;
		std::string head = bubequ::bubequToHashes(gameState, root, hashMap);
		bubequ::saveBubequHead(name, head, hashMap, traversePath);
	}

}
