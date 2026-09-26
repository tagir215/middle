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
#include "MidComp/LoopSociety.h"
#include "ReferenceEntity.h"
#include <stack>
#include "midconfig.h"
#include "middle_paths.h"

namespace middle {



	// DEPRECATED
	std::string coordToLines(const midMath::Vector3& position) {
		auto x = "f  " + std::to_string(position.x) + "\n";
		auto y = "f  " + std::to_string(position.y) + "\n";
		auto z = "f  " + std::to_string(position.z) + "\n";
		return x + y + z;
	}


	void saveTempShape(GameState* gameState, Id& idToSave)
	{
		std::string folder = std::string(middlePaths::TEMP_FOLDER) + "/";
		std::string name = "s" + std::to_string(idToSave.index) + "_" + std::to_string(idToSave.generation);
		saveShape(gameState, idToSave, folder, name);
	}

	middle::Id loadTempShape(GameState* gameState, Id& idToLoad)
	{
		std::string folder = std::string(middlePaths::TEMP_FOLDER) + "/";
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
		while (gameState->undoQueue.size() > 0) {
			gameState->undoQueue.pop();
		}
		if (gameState->bubbleAlgebraState.bubbleActions.size() > 0) {
			gameState->bubbleAlgebraState.bubbleActions.clear();
		}
		gameState->reset = false;
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

		std::string folder = std::string(middlePaths::SCENES_FOLDER) + "/";
		for (const auto& entry : fs::directory_iterator(folder)) {
			if (entry.path().extension() == std::string(middlePaths::MIDSC_FILE_EXTENSION)) {
				sceneNames.push_back(entry.path().stem().string());
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

		for (int typeId : shape.componentTypes) {
			int offset = getCompOffset(shape, typeId);
			std::string componentName = componentNameMap[typeId];

			const std::string initializedThing = "InitializedTag";
			if (componentName == initializedThing) {
				continue;
			}

			outFile << componentName << "\n";
			Serializable* serializable = componentListMap[typeId]->getSerializable(offset);

			// skip children for reference types to save storage memory
			if (skipChildren) {
				bool isLoopComp = middle::getTypeId<components::LoopSociety>() == typeId;
				if (isLoopComp) continue;
			}

			serializable->serialize(outFile);
		}
	}

	void saveScene(GameState* gameState, const std::string& sceneName) {
		std::string filename = std::string(middlePaths::SCENES_FOLDER) + "/" + sceneName + middlePaths::MIDSC_FILE_EXTENSION;
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
			if (shape.componentTypes.size() == 0) {
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
		std::string path = folder + shapeName + middlePaths::MIDSC_FILE_EXTENSION;
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

	middle::Id loadShape(GameState* gameState, const std::string& folder, const std::string& sceneName, bool import, const midMath::Vector3& pos) {
		int freeIndex = findFreeIndex(gameState);
		return loadScene(gameState, folder, sceneName, import, pos, freeIndex);
	}


	void saveEditorState(GameState* gameState)
	{
		std::string filename = middlePaths::EDITOR_STATE;
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

			auto posComponent = getComponent<components::LocalPosition>(shape);
			midMath::Vector3 pos = { 0,0,0 };
			if (posComponent) {
				pos = posComponent->pos;
			// reset to zero, because load scene will again set the position, while also moving its children
				posComponent->pos = { 0,0,0 };
			}
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
		middle::setCompOffset(shape, typeId, componentOffset);
		middle::notifyStructuralChanges(gameState, shape.id, typeId);
		buffer.clear();

	}

	void flushFieldBuffer(GameState* gameState, std::vector<std::string>& buffer, const std::string& field) {
		if (field == "#editorCameraPos") {
			assert(buffer.size() == 3);
			midMath::Vector3 pos;
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

	middle::Id loadScene(GameState* gameState, const std::string& folder, const std::string& sceneName, bool import, const midMath::Vector3& pos, int sceneReferenceIndex) {

		std::string path = folder + "/" + sceneName + ".midsc";


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
			midMath::Vector3 displacement = pos;
			std::vector<middle::Id>children;
			middle::Id sceneReferenceId = gameState->ids[sceneReferenceIndex];
			middle::getChildren(gameState, sceneReferenceId, children);
			moveShape(gameState, sceneReferenceIndex, displacement);
			for (middle::Id id : children) {
				moveShape(gameState, id.index, displacement);
			}

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
		std::string filename = middlePaths::EDITOR_STATE;

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
		const std::string filename = std::string(middlePaths::SYSTEMS_FOLDER) + "/" + systemName + ".cpp";
		const std::string placeholder = "/*systemName*/";

		generateFileFromTemplate(filename, std::string(middlePaths::SYSTEM_TEMPLATE), systemName, placeholder);
	}

	void newComponentFile(GameState* gameState, const std::string& componentName)
	{
		const std::string filenameHeader = std::string(middlePaths::COMPONENT_FOLDER) + "/" + componentName + ".h";
		const std::string filenameSource = std::string(middlePaths::COMPONENT_FOLDER) + "/" + componentName + ".cpp";
		const std::string placeholder = "/*componentName*/";

		generateFileFromTemplate(filenameHeader, std::string(middlePaths::COMPONENT_TEMPLATE_HEADER), componentName, placeholder);
		generateFileFromTemplate(filenameSource, std::string(middlePaths::COMPONENT_TEMPLATE_SOURCE), componentName, placeholder);
	}

	void queueSound(GameState* gameState, const std::string& soundName) {
		midPrimitive::Sound sound;
		sound.name = soundName;
		gameState->sounds.push_back(sound);
	}
}

