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
#include <stack>

namespace middle {

	std::vector<std::string> split(const std::string& s, char delim) {
		std::vector<std::string> parts;
		std::size_t start = 0;
		std::size_t pos = 0;

		if (s == "")
			return parts;

		while ((pos = s.find(delim, start)) != std::string::npos) {
			if (pos == start) {
				break;
			}
			parts.push_back(s.substr(start, pos - start));
			start = pos + 1;
		}

		std::string lastSegment = s.substr(start);
		if(start != pos && lastSegment != "")
			parts.push_back(lastSegment);  // last segment
		return parts;
	}

	FieldType fieldToType(const std::any& field) {

		if (field.type() == typeid(std::string)) {
			return FieldType::String;
		}
		else if (field.type() == typeid(int)) {
			return FieldType::Int;
		}
		else if (field.type() == typeid(float)) {
			return FieldType::Float;
		}
		else if (field.type() == typeid(double)) {
			return FieldType::Double;
		}
		else if (field.type() == typeid(bool)) {
			return FieldType::Bool;
		}
		else if (field.type() == typeid(Vector3)) {
			return FieldType::Vector3;
		}
		else if (field.type() == typeid(Vector2)) {
			return FieldType::Vector2;
		}
		else if (field.type() == typeid(Quaternion)) {
			return FieldType::Quaternion;
		}
		else if (field.type() == typeid(Color)) {
			return FieldType::Color;
		}
		else if (field.type() == typeid(Id)) {
			return FieldType::Id;
		}
		else if (field.type() == typeid(std::vector<Id>)) {
			return FieldType::IdVector;
		}

		assert(false, "no we are not supporting this");
	}

	std::string Vector3ToString(const Vector3& v) {
		std::string result = "\n";
		result += std::to_string(v.x) + "\n";
		result += std::to_string(v.y) + "\n";
		result += std::to_string(v.z);
		return result;
	}

	std::string Vector2ToString(const Vector2& v) {
		std::string result = "\n";
		result += std::to_string(v.x) + "\n";
		result += std::to_string(v.y);
		return result;
	}

	std::string QuaternionToString(const Quaternion& q) {
		std::string result = "\n";
		result += std::to_string(q.x) + "\n";
		result += std::to_string(q.y) + "\n";
		result += std::to_string(q.z) + "\n";
		result += std::to_string(q.w) + "\n";
		return result;
	}

	std::string ColorToString(const Color& c) {
		std::string result = "\n";
		result += std::to_string(c.r) + "\n";
		result += std::to_string(c.g) + "\n";
		result += std::to_string(c.b) + "\n";
		result += std::to_string(c.a) + "\n";
		return result;
	}

	// DEPRECATED
	std::string coordToLines(const Vector3& position) {
		auto x = "f  " + std::to_string(position.x) + "\n";
		auto y = "f  " + std::to_string(position.y) + "\n";
		auto z = "f  " + std::to_string(position.z) + "\n";
		return x + y + z;
	}
	

	std::string fieldToString(const std::any& field) {
		FieldType type = fieldToType(field);
		std::string result = std::string(1, static_cast<char>(type)) + "  ";

		switch (type) {
		case FieldType::String:
			return result + std::any_cast<const std::string&>(field) + '\n';
		case FieldType::Int:
			return result + std::to_string(std::any_cast<int>(field)) + '\n';
		case FieldType::Float:
			return result + std::to_string(std::any_cast<float>(field)) + '\n';
		case FieldType::Double:
			return result + std::to_string(std::any_cast<double>(field)) + '\n';
		case FieldType::Bool:
			return result + std::to_string(std::any_cast<bool>(field)) + '\n';
		case FieldType::Vector3:
			return result + Vector3ToString(std::any_cast<Vector3>(field)) + '\n';
		case FieldType::Vector2:
			return result + Vector2ToString(std::any_cast<Vector2>(field)) + '\n';
		case FieldType::Quaternion:
			return result + QuaternionToString(std::any_cast<Quaternion>(field)) + '\n';
		case FieldType::Color:
			return result + ColorToString(std::any_cast<Color>(field)) + '\n';
		case FieldType::Id: {
			middle::Id id = std::any_cast<Id>(field);
			return result + std::to_string(id.index) + '_' + std::to_string(id.generation) + '\n';
		}
		case FieldType::IdVector: {
			auto v = std::any_cast<std::vector<Id>>(field);
			result += '\n';
			for (int i = 0; i < v.size(); ++i) {
				middle::Id id = v[i];
					result += fieldToString(id);
			}
			return result;
		}
		}

		assert(false, "no we are not supporting this");
	}

	void fillField(void* field, const std::string& fieldString, int indexOffset) {
		char c = fieldString[0];
		std::string valueStr = fieldString.substr(3);

		switch (c) {
		case static_cast<char>(FieldType::String): {
			std::string* strptr = static_cast<std::string*>(field);
			*strptr = valueStr;
			return;
		}
		case static_cast<char>(FieldType::Int): {
			int* intptr = static_cast<int*>(field);
			*intptr = std::stoi(valueStr);
			return;
		}
		case static_cast<char>(FieldType::Float): {
			float* floatptr = static_cast<float*>(field);
			*floatptr = std::stof(valueStr);
			return;
		}
		case static_cast<char>(FieldType::Double): {
			double* doubleptr = static_cast<double*>(field);
			*doubleptr = std::stod(valueStr);
			return;
		}
		case static_cast<char>(FieldType::Bool): {
			bool* boolptr = static_cast<bool*>(field);
			*boolptr = std::stoi(valueStr);
			return;
		}
		case static_cast<char>(FieldType::Vector3): {
			std::vector<std::string> values = split(valueStr, '\n');
			Vector3* vptr = static_cast<Vector3*>(field);
			vptr->x = std::stof(values[0]);
			vptr->y = std::stof(values[1]);
			vptr->z = std::stof(values[2]);
			return;
		}
		case static_cast<char>(FieldType::Vector2): {
			std::vector<std::string> values = split(valueStr, '\n');
			Vector2* vptr = static_cast<Vector2*>(field);
			vptr->x = std::stof(values[0]);
			vptr->y = std::stof(values[1]);
			return;
		}
		case static_cast<char>(FieldType::Quaternion): {
			std::vector<std::string> values = split(valueStr, '\n');
			Quaternion* vptr = static_cast<Quaternion*>(field);
			vptr->x = std::stof(values[0]);
			vptr->y = std::stof(values[1]);
			vptr->z = std::stof(values[2]);
			vptr->w = std::stof(values[3]);
			return;
		}
		case static_cast<char>(FieldType::Color): {
			std::vector<std::string> values = split(valueStr, '\n');
			Color* vptr = static_cast<Color*>(field);
			vptr->r = std::stof(values[0]);
			vptr->g = std::stof(values[1]);
			vptr->b = std::stof(values[2]);
			vptr->a = std::stof(values[3]);
			return;
		}
		case static_cast<char>(FieldType::Id): {
			Id* id = static_cast<Id*>(field);
			// Offset by indexOffset. This is used when importing scenes into other scenes, offsetting imported scenes indexes to ghost area
			std::vector<std::string>idAndGen = split(valueStr, '_');
			id->index = std::stoi(idAndGen[0]); 
			if (id->index + indexOffset ==  1179) {
				int a = 0;
			}
			if (id->index != UNASSIGNED) {
				id->index += indexOffset;
			}
			if (idAndGen.size() == 2) {
				id->generation = std::stoi(idAndGen[1]);
			}
			return;
		}
		case static_cast<char>(FieldType::IdVector): {
			std::vector<std::string> values = split(valueStr, '\n');
			std::vector<Id>* vectorptr = static_cast<std::vector<Id>*>(field);
			vectorptr->resize(values.size());
			for (int i = 0; i < values.size(); ++i) {
				// Offset by indexOffsetGlobal. This is used when importing scenes into other scenes, offsetting imported scenes indexes to ghost area
				fillField(&(*vectorptr)[i], values[i], indexOffset);
			}
			return;
		}
		}

		assert("not supported");
	}

	void saveTempShape(GameState* gameState, Id& idToSave)
	{
		std::string folder = "../src/editor_data/temp/";
		std::string name = "s" + std::to_string(idToSave.index) + "_" + std::to_string(idToSave.generation);
		saveShape(gameState, idToSave, folder, name);
	}

	middle::Id loadTempShape(GameState* gameState, Id& idToLoad)
	{
		std::string folder = "../src/editor_data/temp/";
		std::string name = "s" + std::to_string(idToLoad.index) + "_" + std::to_string(idToLoad.generation);
		return loadShape(gameState, folder, name, false);
	}

	void resetGenerations(GameState* gameState)
	{
		for (int i = 0; i < gameState->ids.size(); ++i) {
			if (!isValidId(gameState, gameState->ids[i])) {
				continue;
			}
			gameState->ids[i].generation = 0;
			gameState->shapes[i].id.generation = 0;
		}
	}

	void incrementGenerations(GameState* gameState)
	{
		for (int i = 0; i < gameState->ids.size(); ++i) {
			if (!isValidId(gameState, gameState->ids[i])) {
				continue;
			}
			static int incrementAmount = 1;
			gameState->ids[i].generation += incrementAmount;
			gameState->shapes[i].id.generation += incrementAmount;
			++incrementAmount;
		}
	}

	void resetScene(GameState* gameState)
	{
		for (int i = 0; i < gameState->shapes.size(); ++i) {
			if (isValidId(gameState, gameState->ids[i])) {
				deleteShape(gameState, i);
			}
		}
		gameState->reset = true;
		while (gameState->undoQueue.size() > 0) {
			gameState->undoQueue.pop();
		}
		if (gameState->bubbleAlgebraState.bubbleActions.size() > 0) {
			gameState->bubbleAlgebraState.bubbleActions.clear();
		}
	}



	bool isEmptyOrWhitespace(const std::string& s) {
		return s.empty() ||
			std::all_of(s.begin(), s.end(),
				[](unsigned char c) { return std::isspace(c); });
	}

	void loadSceneAndShapeNames(GameState* gameState)
	{
		namespace fs = std::filesystem;
		std::vector<std::string>& sceneNames = gameState->sceneNames;
		sceneNames.clear();

		std::string folder = "../assets/scenes/";
		for (const auto& entry : fs::directory_iterator(folder)) {
			if (entry.path().extension() == ".midsc") {
				sceneNames.push_back(entry.path().stem().string());
			}
		}
		std::vector<std::string>& shapeNames = gameState->shapeNames;
		std::string folder2 = "../assets/shapes/";
		for (const auto& entry : fs::directory_iterator(folder2)) {
			if (entry.path().extension() == ".midsc") {
				shapeNames.push_back(entry.path().stem().string());
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

	void saveComponent(middle::Shape& shape, std::ofstream& outFile) {

		// references are special. for references skip children to save storage 
		bool skipChildren = middle::getComponent<components::Reference>(shape);

		for (auto& pair : shape.componentMap) {
			std::string componentName = componentNameMap[pair.first];
			outFile << componentName << "\n";
			int componentTypeId = pair.first;
			Component component = pair.second;
			Serializable* serializable = componentListMap[componentTypeId]->getSerializable(component.componentOffset);

			// skip children for reference types to save storage memory
			if (skipChildren) {
				bool isLoopComp = middle::getTypeId<components::LoopSociety>() == pair.first;
				if (isLoopComp) continue;
			}

			serializable->serialize(outFile);
		}
	}

	void saveScene(GameState* gameState, const std::string& sceneName) {
		std::string filename = "../assets/scenes/" + sceneName + ".midsc";
		std::ofstream outFile(filename);
		if (!outFile.is_open()) {
			std::cerr << "failed to open to write\n";
		}

		outFile << "#scene\n";
		outFile << fieldToString(sceneName);

		int saveSpam = GHOST_INDEX_OFFSET;
		for (int i = 0; i < saveSpam; ++i) {
			// skip empty parts if over max used index
			if (!isValidId(gameState, gameState->ids[i]))
				continue;

			auto& shape = gameState->shapes[i];
			// skip empty shapes
			if (shape.componentMap.size() == 0) {
				continue;
			}
			std::string idString = fieldToString(shape.id);
			outFile << "__" << idString;

			saveComponent(shape, outFile);
		}

		outFile.flush();
		outFile.close();
	}

	void saveShape(GameState* gameState, Id& idToSave, const std::string& folder, const std::string& shapeName)
	{
		auto& shapeToSave = getShape(gameState, idToSave.index);
		std::string path = folder + shapeName + ".midsc";
		std::ofstream outFile(path);
		if (!outFile.is_open()) {
			std::cerr << "failed to open to write\n";
		}

		std::stack<middle::Id> idStack;
		idStack.push(idToSave);
		while (idStack.size() > 0) {
			Id currentId = idStack.top();
			idStack.pop();

			auto& shape = getShape(gameState, currentId.index);
			std::string idString = fieldToString(shape.id);
			outFile << "__" << idString;
			saveComponent(shape, outFile);

			std::vector<middle::Id> children;
			middle::getChildren(gameState, shape.id, children);
			for (Id& id : children) {
				idStack.push(id);
			}
		}

		outFile.flush();
		outFile.close();

	}

	middle::Id loadShape(GameState* gameState, const std::string& folder, const std::string& sceneName, bool import, const Vector3& pos) {
		int freeIndex = findFreeIndex(gameState);
		return loadScene(gameState, folder, sceneName, import, pos, freeIndex);
	}


	void saveEditorState(GameState* gameState)
	{
		std::string filename = "../src/editor_data/editor_state.midsc";
		std::ofstream outFile(filename);
		if (!outFile.is_open()) {
			std::cerr << "failed to open to write\n";
		}

		outFile << "#activeScene" << "\n";
		outFile << fieldToString(gameState->activeSceneName);
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
			loadScene(gameState, referenceComponent->folder, referenceComponent->sceneName, true, pos, index);
		}
	}


	void flushBuffer(GameState* gameState, std::vector<std::string>& buffer, const std::string& componentName, int index, int indexOffset = 0) {
		int typeId = componentTypeMap[componentName];
		auto& componentList = componentListMap[typeId];
		int componentOffset = componentList->grow();
		Serializable* serializable = componentListMap[typeId]->getSerializable(componentOffset);
		serializable->deserialize(buffer, indexOffset);
		auto& shape = gameState->shapes[index];
		shape.componentMap[typeId].componentOffset = componentOffset;
		gameState->componentTypeIdSetWithStructuralChanges.insert(typeId);
		buffer.clear();

	}

	void flushFieldBuffer(GameState* gameState, std::vector<std::string>& buffer, const std::string& field) {
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
			fillField(&gameState->activeSceneName, buffer[0]);
			buffer.clear();
		}
		assert("something wrong about data");
	}

	bool isVectorType(char typeC) {
		return typeC == static_cast<char>(FieldType::IdVector)
			|| typeC == static_cast<char>(FieldType::Vector3)
			|| typeC == static_cast<char>(FieldType::Vector2)
			|| typeC == static_cast<char>(FieldType::Quaternion)
			|| typeC == static_cast<char>(FieldType::Color);
	}

	middle::Id loadScene(GameState* gameState, const std::string& folder, const std::string& sceneName, bool import, const Vector3& pos, int sceneReferenceIndex) {

		std::string path = folder + sceneName + ".midsc";

		int indexOffset = 0;

		std::ifstream inputFile(path);
		if (!inputFile.is_open()) {
			throw std::runtime_error("Failed to open file to open");
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

		std::vector<middle::Id>newShapeIds;

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
				int end = l;
				std::string idString = line.substr(start, end);
				middle::Id id;
				fillField(&id, idString, indexOffset);
				auto& shape = insertShape(gameState, id);
				activeShapeIndex = id.index;
				newShapeIds.push_back(shape.id);

				parseMode = noParse;
			}

			// component name found from component type map
			if (componentTypeMap.find(line) != componentTypeMap.end()) {
				if (parseMode != noParse) {
					flushBuffer(gameState, buffer, activeComponentName, activeShapeIndex, indexOffset);
				}
				activeComponentName = line;
				parseMode = componentMode;
				continue;
			}

			// type is first character
			char typeC = line[0];
			if (isVectorType(typeC)) {
				parseMode = vectorMode;
				buffer.push_back(line);
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
		for (middle::Id& id : newShapeIds) {
			auto& shape = gameState->shapes[id.index];
			if (getComponent<components::Reference>(shape)) {
				loadReferences(gameState, id.index);
			}
		}

		// if we import we contain all the content in a reference loop
		if (import) {
			std::set<int>highestLevelContainers;
			for (middle::Id& newId : newShapeIds) {
				auto& shape = getShape(gameState, newId.index);
				if (getComponent<components::LoopSociety>(shape) != nullptr) {
					highestLevelContainers.insert(findHighestLevelContainer(gameState, newId.index));
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
			if (!isValidId(gameState, gameState->ids[sceneReferenceIndex])) {
				entities::initReference(gameState, sceneReferenceIndex, members, folder, sceneName);
			}
			// if reference already exists, when deserializing, just update the container loop, since its refence objects are not stored to the file
			else {
				auto loop = getComponent<components::LoopSociety>(gameState->shapes[sceneReferenceIndex]);
				loop->loopMemberIds = members;
			}

			// move imported scene where it wants to be
			moveShape(gameState, sceneReferenceIndex, pos);

			return gameState->ids[sceneReferenceIndex];
		}

		return middle::Id();
	}

	std::vector<std::string> loadFileNamesInFolder(const std::string& folderPath)
	{
		std::vector<std::string>filenames;
		try {
			if (std::filesystem::exists(folderPath) && std::filesystem::is_directory(folderPath)) {
				for (const auto& entry : std::filesystem::directory_iterator(folderPath)) {
					filenames.push_back(entry.path().stem().string());
				}
			}
		}
		catch (const std::filesystem::filesystem_error& err) {
			std::cerr << "hmm" << err.what();
		}
		return filenames;
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


	void generateFileFromTemplate(const std::string& destinationPath, const std::string& templateFilePath, const std::string& objectName, const std::string& placeholder) {
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

		std::string OBJECTNAME = objectName;
		std::transform(OBJECTNAME.begin(), OBJECTNAME.end(), OBJECTNAME.begin(), ::toupper);
		std::string PLACEHOLDER = placeholder;
		std::transform(PLACEHOLDER.begin(), PLACEHOLDER.end(), PLACEHOLDER.begin(), ::toupper);
		// replace upper cased placeholders 
		for (int i = 0; i < templateLines.size(); ++i) {
			auto& line = templateLines[i];

			size_t pos = 0;
			while (pos != std::string::npos) {
				pos = line.find(PLACEHOLDER, pos);
				if (pos != std::string::npos) {
					line.replace(pos, PLACEHOLDER.length(), "MIDDLE" + OBJECTNAME);
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

		generateFileFromTemplate(filename, templateFilename, systemName, placeholder);
	}

	void newComponentFile(GameState* gameState, const std::string& componentName)
	{
		const std::string templateFilenameHeader = "../src/editor_data/component_template.h";
		const std::string templateFilenameSource = "../src/editor_data/component_template.cpp";
		const std::string filenameHeader = "../assets/components/" + componentName + ".h";
		const std::string filenameSource = "../assets/components/" + componentName + ".cpp";
		const std::string placeholder = "/*componentName*/";

		generateFileFromTemplate(filenameHeader, templateFilenameHeader, componentName, placeholder);
		generateFileFromTemplate(filenameSource, templateFilenameSource, componentName, placeholder);
	}


	void queueSound(GameState* gameState, const std::string& soundName) {
		gameState->soundQueue.push(gameState->soundMap[soundName]);
	}
}

