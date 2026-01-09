#include "editor_file_utils.h"
#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <cctype>
#include <filesystem>
#include <middle_shape_utils.h>
#include "middle_component_table.h"
#include <set>

namespace middle {

	std::string fieldToString(const std::any& field) {
		if (field.type() == typeid(std::string)) {
			const std::string& v = std::any_cast<const std::string&>(field);
			return "s " + v + "\n";
		}
		else if (field.type() == typeid(int)) {
			int v = std::any_cast<int>(field);
			return "i " + std::to_string(v) + "\n";
		}
		else if (field.type() == typeid(float)) {
			int v = std::any_cast<float>(field);
			return "f " + std::to_string(v) + "\n";
		}
		else if (field.type() == typeid(double)) {
			int v = std::any_cast<double>(field);
			return "d " + std::to_string(v) + "\n";
		}
		else if (field.type() == typeid(double)) {
			int v = std::any_cast<double>(field);
			return "d " + std::to_string(v) + "\n";
		}
		else if (field.type() == typeid(bool)) {
			bool v = std::any_cast<bool>(field);
			return "b " + std::to_string(v) + "\n";
		}

		assert(true, "nope not supporting");
	}

	void fillField(void* field, const std::string& fieldString) {
		char c = fieldString[0];
		std::string valueStr = fieldString.substr(2);
		if (c == 's') {
			std::string* strptr = static_cast<std::string*>(field);
			*strptr = valueStr;
		}
		else if (c == 'i') {
			int* intptr = static_cast<int*>(field);
			*intptr = std::stoi(valueStr);
		}
		else if (c == 'f') {
			float* floatptr = static_cast<float*>(field);
			*floatptr = std::stof(valueStr);
		}
		else if (c == 'd') {
			double* doubleptr = static_cast<double*>(field);
			*doubleptr = std::stod(valueStr);
		}
		else if (c == 'b') {
			bool* boolptr = static_cast<bool*>(field);
			*boolptr = std::stoi(valueStr);
		}

		assert("nope not supported");
	}

	std::string coordToLines(const Vector3& position) {
		auto x = "f " + std::to_string(position.x) + "\n";
		auto y = "f " + std::to_string(position.y) + "\n";
		auto z = "f " + std::to_string(position.z) + "\n";
		return x + y + z;
	}

	bool isEmptyOrWhitespace(const std::string& s) {
		return s.empty() ||
			std::all_of(s.begin(), s.end(),
				[](unsigned char c) { return std::isspace(c); });
	}

	void loadSceneNames(GameState* gameState)
	{
		namespace fs = std::filesystem;
		std::vector<std::string>& names = gameState->sceneNames;
		names.clear();

		std::string folder = "../assets/scenes/";

		for (const auto& entry : fs::directory_iterator(folder)) {
			if (entry.path().extension() == ".midsc") {
				names.push_back(entry.path().stem().string());
			}
		}
	}

	void loadScriptNames(GameState* gameState)
	{
		for (auto& pair : scriptMap) {
			gameState->scriptNames.push_back(pair.first);
		}
	}

	void loadComponentNames(GameState* gameState)
	{
		for (auto& pair : componentTypeMap) {
			gameState->componentNames.push_back(pair.first);
		}
	}

	void saveScene(GameState* gameState, const std::string& sceneName) {
		std::string line;

		int maxIndex = findHighestUsedIndex(gameState);

		std::string filename = "../assets/scenes/" + sceneName + ".midsc";
		std::ofstream outFile(filename);
		if (!outFile.is_open()) {
			std::cerr << "failed to open to write\n";
		}

		outFile << "#scene\n";
		outFile << fieldToString(sceneName);
		outFile << "#activeCamera\n";
		outFile << fieldToString(gameState->activeCameraIndex);

		int saveSpam = GHOST_INDEX_OFFSET;
		for (int i = 0; i < saveSpam; ++i) {
			// skip empty parts if over max used index
			if (isSlotFree(gameState, i))
				continue;

			auto& shape = gameState->shapes[i];

			outFile << "__" << std::to_string((int)shape.id.index) << "__" << std::endl;
			for (auto& pair : shape.componentMap) {
				outFile << fieldToString(pair.first);
				Serializable* serializable = getSerializableComponent(shape, pair.second.typeId);
				serializable->serialize(outFile);
			}
		}

		outFile.flush();
		outFile.close();
	}


	void saveEditorState(GameState* gameState)
	{
		std::string filename = "../src/editor_data/editor_state.midsc";
		std::ofstream outFile(filename);
		if (!outFile.is_open()) {
			std::cerr << "failed to open to write\n";
		}

		outFile << "#activeScene" << "\n";
		outFile << fieldToString(gameState->activeScene);
		outFile << "#editorCameraPos" << "\n";
		outFile << coordToLines(gameState->editorState.initCamera.position) << std::endl;

		outFile.flush();
		outFile.close();
	}


	void flushBuffer(GameState* gameState, std::vector<std::string>& buffer, int type, int index, int offset = 0) {

	}

	void flushFieldBuffer(GameState* gameState, std::vector<std::string>& buffer, const std::string& field) {
		if (field == "#activeCamera") {
			assert(buffer.size() == 1);
			fillField(&gameState->activeCameraIndex, buffer[0]);
			buffer.clear();
		}
		if (field == "#editorCameraPos") {
			assert(buffer.size() == 3);
			Vector3 pos;
			fillField(&pos.x, buffer[0]);
			fillField(&pos.y, buffer[1]);
			fillField(&pos.z, buffer[2]);
			moveCameraXZ(gameState->editorState.initCamera, pos);
			buffer.clear();
		}
		if (field == "#activeScene") {
			assert(buffer.size() == 1);
			fillField(&gameState->activeScene, buffer[0]);
			buffer.clear();
		}
		assert("something wrong about data");
	}

	void loadScene(GameState* gameState, const std::string& sceneName, bool import, const Vector3& pos, int sceneReferenceIndex) {
		std::string filename = "../assets/scenes/" + sceneName + ".midsc";

		std::ifstream inputFile(filename);
		if (!inputFile.is_open()) {
			std::cerr << "Failed to open file to write";
			return;
		}

		std::string line;
		int currentType = -1;
		int currentIndex = 0;

		// all import indexes are shifted by half of total allowed shape count
		// if highest used index is above half of total allowed shape count, use the next one after highest used as offset
		int indexOffset = 0;
		if (import) {
			indexOffset = findHighestUsedIndex(gameState) + 1;
			int minImportOffset = GHOST_INDEX_OFFSET;
			indexOffset = indexOffset > minImportOffset ? indexOffset : minImportOffset;
		}

		// if not importing make sure loop index is 0, 
		if (!import) {
			gameState->loopIndex = 0;
		}

		std::vector<std::string>buffer;

		std::string field = "";

		// read scene info
		if (!import) {
			while (std::getline(inputFile, line)) {
				if (line.find("#") != std::string::npos) {
					if (buffer.size() > 0)
						flushFieldBuffer(gameState, buffer, field);
					field = line;
					buffer.clear();
					continue;
				}

				if (line.find("__") != std::string::npos)
					break;

				if (!isEmptyOrWhitespace(line))
					buffer.push_back(line);
			}
			flushFieldBuffer(gameState, buffer, field);
		}


		// reset input file to start:w
		buffer.clear();
		inputFile.clear();
		inputFile.seekg(0, std::ios::beg);

		// read objects
		while (std::getline(inputFile, line)) {

			// read shapes
			if (line.find("__") != std::string::npos) {
				if (currentType >= 0) {
					flushBuffer(gameState, buffer, currentType, currentIndex, indexOffset);
					++currentIndex;
				}
				int l = line.size();
				int start = 2;
				int end = l - 2;
				std::string digits = line.substr(start, end);
				currentType = std::stoi(digits);
				continue;
			}
			// don't append until first type is found, (where data section starts, so skip metadata)
			if (currentType >= 0 && !isEmptyOrWhitespace(line))
				buffer.push_back(line);
		}

		flushBuffer(gameState, buffer, currentType, currentIndex, indexOffset);

		inputFile.close();


		// if we import we contain all the content in a reference loop
		if (import) {
			currentIndex++;
			int highestUsedIndex = findHighestUsedIndex(gameState);
			int shapesAddedCount = highestUsedIndex - indexOffset;
			std::set<int>highestLevelContainers;
			for (int i = indexOffset; i < highestUsedIndex; ++i) {
				// skip nons and skip constraints since they don't have parents
				//if (!isSlotFree(gameState, i) && gameState->shapes[i].type != ShapeType::CONSTRAINT)
					//highestLevelContainers.insert(findHighestLevelContainer(gameState, i));
			}

			// make reference
			std::vector<int>members;
			for (int v : highestLevelContainers) {
				members.push_back(v);
			}

			// if it's ghost scene, basically a scene imported by a scene, find next highest index to use, otherwise the reference index should be the one passed in
			if (isGhostShape(sceneReferenceIndex)) {
				sceneReferenceIndex = highestUsedIndex + 1;
			}

			initReference(gameState, sceneReferenceIndex, members, sceneName);

			// move imported scene where it wants to be
			moveShape(gameState, sceneReferenceIndex, pos);
		}
	}

	void loadEditorState(GameState* gameState) {
		std::string filename = "../src/editor_data/editor_state.midsc";

		std::ifstream inputFile(filename);
		if (!inputFile.is_open()) {
			std::cerr << "Failed to open file to write";
			return;
		}

		std::string line;
		std::vector<std::string>buffer;
		std::string field = "";

		while (std::getline(inputFile, line)) {
			if (line.find("#") != std::string::npos) {
				if (field != "")
					flushFieldBuffer(gameState, buffer, field);
				field = line;
				continue;
			}
			if (!isEmptyOrWhitespace(line))
				buffer.push_back(line);
		}
		flushFieldBuffer(gameState, buffer, field);
	}

	void newSystemFile(GameState* gameState, const std::string& scriptName)
	{
		std::string templateFilename = "../src/editor_data/system_template.cpp";
		std::ifstream inputFile(templateFilename);
		if (!inputFile.is_open()) {
			std::cerr << "Failed to open file to write";
			return;
		}

		std::string filename = "../assets/scripts/" + scriptName + ".cpp";

		// read template to string array
		std::string templateLine;
		std::vector<std::string> templateLines;
		while (std::getline(inputFile, templateLine)) {
			templateLines.push_back(templateLine);
		}
		inputFile.close();

		// replace lines with script names
		std::string placeholder = "/*scriptname*/";
		for (int i = 0; i < templateLines.size(); ++i) {
			auto& line = templateLines[i];
			size_t pos = line.find(placeholder);
			if (pos != std::string::npos) {
				line.replace(pos, placeholder.length(), scriptName);
			}
		}

		// write generated code
		std::ofstream outFile(filename);
		if (!outFile.is_open()) {
			std::cerr << "failed to open to write\n";
		}

		for (auto& line : templateLines) {
			outFile << line << std::endl;
		}

		outFile.flush();
		outFile.close();
	}

	void newComponentFile(GameState* gameState, const std::string& componentName)
	{
		std::string templateFilename = "../src/editor_data/component_template.h";
		std::ifstream inputFile(templateFilename);
		if (!inputFile.is_open()) {
			std::cerr << "Failed to open file to write";
			return;
		}

		std::string filename = "../assets/components/" + componentName + ".h";

		// read template to string array
		std::string templateLine;
		std::vector<std::string> templateLines;
		while (std::getline(inputFile, templateLine)) {
			templateLines.push_back(templateLine);
		}
		inputFile.close();

		// replace lines with script names
		std::string placeholder = "/*componentName*/";
		for (int i = 0; i < templateLines.size(); ++i) {
			auto& line = templateLines[i];
			size_t pos = line.find(placeholder);
			if (pos != std::string::npos) {
				line.replace(pos, placeholder.length(), componentName);
			}
		}

		// write generated code
		std::ofstream outFile(filename);
		if (!outFile.is_open()) {
			std::cerr << "failed to open to write\n";
		}

		for (auto& line : templateLines) {
			outFile << line << std::endl;
		}

		outFile.flush();
		outFile.close();
	}
}

