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
#include "LoopSociety.h"
#include "ReferenceEntity.h"

namespace middle {

	std::vector<std::string> split(const std::string& s, char delim) {
		std::vector<std::string> parts;
		std::size_t start = 0;
		std::size_t pos;

		if (s == "")
			return parts;

		while ((pos = s.find(delim, start)) != std::string::npos) {
			parts.push_back(s.substr(start, pos - start));
			start = pos + 1;
		}

		std::string lastSegment = s.substr(start);
		if(lastSegment != "")
			parts.push_back(lastSegment);  // last segment
		return parts;
	}

	std::string fieldToString(const std::any& field) {
		if (field.type() == typeid(std::string)) {
			const std::string& v = std::any_cast<const std::string&>(field);
			return "s  " + v + "\n";
		}
		else if (field.type() == typeid(int)) {
			int v = std::any_cast<int>(field);
			return "i  " + std::to_string(v) + "\n";
		}
		else if (field.type() == typeid(float)) {
			float v = std::any_cast<float>(field);
			return "f  " + std::to_string(v) + "\n";
		}
		else if (field.type() == typeid(double)) {
			double v = std::any_cast<double>(field);
			return "d  " + std::to_string(v) + "\n";
		}
		else if (field.type() == typeid(bool)) {
			bool v = std::any_cast<bool>(field);
			return "b  " + std::to_string(v) + "\n";
		}
		else if (field.type() == typeid(Id)) {
			Id id = std::any_cast<Id>(field);
			return "q  " + std::to_string(id.index) + "\n";
		}
		else if (field.type() == typeid(std::vector<Id>)) {
			auto v = std::any_cast<std::vector<Id>>(field);
			std::string result = "vq\n";
			for (int i = 0; i < v.size(); ++i) {
				// Don't serialize ghost objects... TODO maybe refactor
				if (v[i].index >= GHOST_INDEX_OFFSET)
					continue;
				result += std::to_string(v[i].index) + "\n";
			}
			return result;
		}

		assert(true, "nope not supporting");
	}

	void fillField(void* field, const std::string& fieldString, int indexOffset) {
		char c = fieldString[0];
		std::string valueStr = fieldString.substr(3);
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
		else if (c == 'q') {
			Id* id = static_cast<Id*>(field);
			// Offset by indexOffset. This is used when importing scenes into other scenes, offsetting imported scenes indexes to ghost area
			id->index = std::stoi(valueStr);
			if (id->index != UNASSIGNED) {
				id->index += indexOffset;
			}
			id->generation = 0;
		}
		else if (c == 'v') {
			char vectorType = fieldString[1];
			std::vector<std::string> values = split(valueStr, '\n');
			if (vectorType == 'q') {
				std::vector<Id>* vectorptr = static_cast<std::vector<Id>*>(field);
				vectorptr->resize(values.size());
				for (int i = 0; i < values.size(); ++i) {
					// Offset by indexOffsetGlobal. This is used when importing scenes into other scenes, offsetting imported scenes indexes to ghost area
					(*vectorptr)[i].index = std::stoi(values[i]) + indexOffset;
					(*vectorptr)[i].generation = 0;
				}
			}
		}

		assert("nope not supported");
	}

	std::string coordToLines(const Vector3& position) {
		auto x = "f  " + std::to_string(position.x) + "\n";
		auto y = "f  " + std::to_string(position.y) + "\n";
		auto z = "f  " + std::to_string(position.z) + "\n";
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

	void loadSystemNames(GameState* gameState)
	{
		for (auto& pair : getSystemMap()) {
			gameState->systemNames.push_back(pair.first);
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
			if (!isShapeAlive(gameState, i))
				continue;

			auto& shape = gameState->shapes[i];

			outFile << "__" << std::to_string(i) << "__" << std::endl;
			for (auto& pair : shape.componentMap) {
				std::string componentName = componentNameMap[pair.first];
				outFile << componentName << "\n";
				int componentTypeId = pair.first;
				Serializable* serializable = getSerializableComponent(shape, componentTypeId);
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
		outFile << coordToLines(gameState->editorState.camera.position) << std::endl;

		outFile.flush();
		outFile.close();
	}

	void loadReferences(GameState* gameState, int index) {
		auto& shape = getShape(gameState, index);

		// load scene if added a reference to a scene above
		auto referenceComponent = getComponent<components::Reference>(shape);
		assert(referenceComponent);
		if (referenceComponent) {
			auto posComponent = getComponent<components::Position>(shape);
			Vector3 pos = { posComponent->posX, posComponent->posY, posComponent->posZ };
			// reset to zero, because load scene will again set the position, while also moving its children
			posComponent->posX = 0;
			posComponent->posY = 0;
			posComponent->posZ = 0;
			loadScene(gameState, referenceComponent->sceneName, true, pos, index);
		}
	}


	void flushBuffer(GameState* gameState, std::vector<std::string>& buffer, const std::string& componentName, int index, int indexOffset = 0) {
		int typeId = componentTypeMap[componentName];
		int componentOffset = componentListMap[typeId]->grow();
		componentSerializableRefMap[typeId][componentOffset]->deserialize(buffer, indexOffset);
		auto& shape = gameState->shapes[index + indexOffset];
		shape.componentMap[typeId].componentOffset = componentOffset;
		buffer.clear();

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
			moveCameraXZ(gameState->editorState.camera, pos);
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

		int indexOffset = 0;

		std::ifstream inputFile(filename);
		if (!inputFile.is_open()) {
			std::cerr << "Failed to open file to write";
			return;
		}

		std::string line;

		// all import indexes are shifted by half of total allowed shape count
		// if highest used index is above half of total allowed shape count, use the next one after highest used as offset
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
		std::string component = "";


		// reset input file to start:w
		buffer.clear();
		inputFile.clear();
		inputFile.seekg(0, std::ios::beg);

		std::string activeComponentName = "";
		const int noParse = 0;
		const int vectorMode = 1;
		const int componentMode = 2;
		int activeShapeIndex = -1;
		int parseMode = noParse;

		// read objects
		while (std::getline(inputFile, line)) {

			// stumbled into an entity. flush the previous entity
			if (line.find("__") != std::string::npos) {
				if (parseMode != noParse) {
					flushBuffer(gameState, buffer, activeComponentName, activeShapeIndex, indexOffset);
				}
				// parse entity index and initialize it
				int l = line.size();
				int start = 2;
				int end = l - 2;
				std::string digits = line.substr(start, end);
				activeShapeIndex = std::stoi(digits);
				addShape(gameState, activeShapeIndex + indexOffset);
				parseMode = noParse;
			}

			// component name found from component type map
			if(componentTypeMap.find(line) != componentTypeMap.end()){
				if (parseMode != noParse) {
					flushBuffer(gameState, buffer, activeComponentName, activeShapeIndex, indexOffset);
				}
				activeComponentName = line;
				parseMode = componentMode;
				continue;
			}

			if (line[0] == 'v') {
				parseMode = vectorMode;
				buffer.push_back(line + '\n');
				continue;
			}


			if (parseMode == componentMode) {
				buffer.push_back(line);
			}
			// in vector mode push new lines to the last buffer element
			if (parseMode == vectorMode) {
				buffer[buffer.size() - 1] += line + '\n';
			}
		}

		if (parseMode != noParse) {
			flushBuffer(gameState, buffer, activeComponentName, activeShapeIndex, indexOffset);
		}

		inputFile.close();



		int highestUsedIndex = findHighestUsedIndex(gameState);

		// loop added indexes and load all the references 
		for(int i=indexOffset; i<highestUsedIndex + 1; ++i){
			auto& shape = gameState->shapes[i];
			if (getComponent<components::Reference>(shape)) {
				loadReferences(gameState, i);
			}
		}

		

		// if we import we contain all the content in a reference loop
		if (import) {
			std::set<int>highestLevelContainers;
			for (int i = indexOffset; i < highestUsedIndex + 1; ++i) {
				// skip nons and skip constraints since they don't have parents
				if (!isShapeAlive(gameState, i))
					continue;
				auto& shape = getShape(gameState, i);
				if (getComponent<components::LoopSociety>(shape) != nullptr) {
					highestLevelContainers.insert(findHighestLevelContainer(gameState, i));
				}
			}

			// make reference
			std::vector<Id>members;
			for (int v : highestLevelContainers) {
				auto& shape = getShape(gameState, v);
				members.push_back(shape.id);
			}

			// if it's ghost scene, basically a scene imported by a scene, find next highest index to use, otherwise the reference index should be the one passed in
			if (isGhostShape(sceneReferenceIndex)) {
				sceneReferenceIndex = highestUsedIndex + 1;
			}

			// if reference doesn't exist yet, when importing from editor, create new reference
			if(!isShapeAlive(gameState, sceneReferenceIndex)){
				entities::initReference(gameState, sceneReferenceIndex, members, sceneName);
			}
			// if reference already exists, when deserializing, just update the container loop, since its refence objects are not stored to the file
			else {
				auto loop = getComponent<components::LoopSociety>(gameState->shapes[sceneReferenceIndex]);
				loop->loopMemberIds = members;
			}

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


	void generateFileFromTemplate(const std::string& destinationPath, const std::string& templateFilePath, const std::string& placeholder, const std::string& objectName) {
		std::ifstream inputFile(templateFilePath);
		if (!inputFile.is_open()) {
			std::cerr << "Failed to open file to write";
			return;
		}

		// read template to string array
		std::string templateLine;
		std::vector<std::string> templateLines;
		while (std::getline(inputFile, templateLine)) {
			templateLines.push_back(templateLine);
		}
		inputFile.close();

		// replace placeholders with object names
		for (int i = 0; i < templateLines.size(); ++i) {
			auto& line = templateLines[i];

			size_t pos = 0;
			while (pos != std::string::npos) {
				pos = line.find(placeholder, pos);
				if (pos != std::string::npos) {
					line.replace(pos, placeholder.length(), objectName);
				}
			}
		}

		// write generated code
		std::ofstream outFile(destinationPath);
		if (!outFile.is_open()) {
			std::cerr << "failed to open to write\n";
		}

		for (auto& line : templateLines) {
			outFile << line << std::endl;
		}

		outFile.flush();
		outFile.close();
	}


	void newSystemFile(GameState* gameState, const std::string& systemName)
	{
		const std::string templateFilename = "../src/editor_data/system_template.cpp";
		const std::string filename = "../assets/systems/" + systemName + ".cpp";
		const std::string placeholder = "/*systemName*/";

		generateFileFromTemplate(filename, templateFilename, placeholder, systemName);
	}

	void newComponentFile(GameState* gameState, const std::string& componentName)
	{
		const std::string templateFilenameHeader = "../src/editor_data/component_template.h";
		const std::string templateFilenameSource = "../src/editor_data/component_template.cpp";
		const std::string filenameHeader = "../assets/components/" + componentName + ".h";
		const std::string filenameSource = "../assets/components/" + componentName + ".cpp";
		const std::string placeholder = "/*componentName*/";

		generateFileFromTemplate(filenameHeader, templateFilenameHeader, placeholder, componentName);
		generateFileFromTemplate(filenameSource, templateFilenameSource, placeholder, componentName);
	}
}

