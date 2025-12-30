#include "editor_file_utils.h"
#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <cctype>
#include <filesystem>
#include <middle_shape_utils.h>
#include <set>

namespace middle {


	std::string coordToLines(const Vector3& position) {
		auto x = std::to_string(position.x) + "\n";
		auto y = std::to_string(position.y) + "\n";
		auto z = std::to_string(position.z);
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

	void saveScene(GameState* gameState, const std::string& sceneName) {
		std::string line;

		int maxIndex = findHighestUsedIndex(gameState);

		std::string filename = "../assets/scenes/" + sceneName + ".midsc";
		std::ofstream outFile(filename);
		if (!outFile.is_open()) {
			std::cerr << "failed to open to write\n";
		}

		outFile << "#scene" << std::endl;
		outFile << sceneName << std::endl;
		outFile << "#activeCamera" << std::endl;
		outFile << gameState->activeCameraIndex << std::endl;

		int saveSpam = GHOST_INDEX_OFFSET;
		for (int i = 0; i < saveSpam; ++i) {
			// skip empty parts if over max used index
			if (isSlotFree(gameState, i))
				continue;

			auto& shape = gameState->shapes[i];

			outFile << "__" << std::to_string((int)shape.type) << "__" << std::endl;

			if (shape.type == ShapeType::SPHERE) {
				outFile << coordToLines(shape.position) << std::endl;
			}
			if (shape.type == ShapeType::CONSTRAINT) {
				outFile << shape.constraint.targetDistance << std::endl;
				outFile << shape.constraint.indexA << std::endl;
				outFile << shape.constraint.indexB << std::endl;
			}
			if (shape.type == ShapeType::LOOP) {
				for (int mIndex = shape.loopArrayOffset; mIndex < shape.loopArrayOffset + shape.loopSize; ++mIndex) {
					outFile << gameState->loopMembers[mIndex] << std::endl;
				}
			}
			if (shape.type == ShapeType::REFERENCE) {
				outFile << shape.name << std::endl;
				outFile << coordToLines(shape.position) << std::endl;
			}
			if (shape.type == ShapeType::CAMERA) {
				outFile << coordToLines(shape.position) << std::endl;
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

		outFile << "#activeScene" << std::endl;
		outFile << gameState->activeScene << std::endl;
		outFile << "#editorCameraPos" << std::endl;
		outFile << coordToLines(gameState->editorState.camera.position) << std::endl;

		outFile.flush();
		outFile.close();
	}


	void flushBuffer(GameState* gameState, std::vector<std::string>& buffer, int type, int index, int offset = 0) {
		if (type == (int)ShapeType::SPHERE) {
			assert(buffer.size() == 3);
			Vector3 pos;
			pos.x = std::stof(buffer[0]);
			pos.y = std::stof(buffer[1]);
			pos.z = std::stof(buffer[2]);
			sphere(gameState, index, pos, offset);
			buffer.clear();
			return;
		}
		if (type == (int)ShapeType::CONSTRAINT) {
			assert(buffer.size() == 3);
			float targetDistance = std::stof(buffer[0]);
			int indexA = std::stoi(buffer[1]);
			int indexB = std::stoi(buffer[2]);
			constraint(gameState, index, indexA, indexB, targetDistance, offset);
			buffer.clear();
			return;
		}
		if (type == (int)ShapeType::LOOP) {
			int loopSize = buffer.size();
			assert(loopSize > 0);
			std::vector<int>memberIndexes;
			for (int i = 0; i < buffer.size(); ++i) {
				memberIndexes.push_back(std::stoi(buffer[i]));
			}
			loop(gameState, index, memberIndexes, offset);
			buffer.clear();
			return;
		}
		if (type == (int)ShapeType::REFERENCE) {
			assert(buffer.size() == 4);
			// index 0 is scene name
			std::string sceneName = buffer[0];
			// move scene
			Vector3 pos;
			pos.x = std::stof(buffer[1]);
			pos.y = std::stof(buffer[2]);
			pos.z = std::stof(buffer[3]);
			// import scene
			loadScene(gameState, sceneName, true, pos, index + offset);
			buffer.clear();
			return;
		}
		if (type == (int)ShapeType::CAMERA) {
			assert(buffer.size() == 3);
			Vector3 pos;
			pos.x = std::stof(buffer[0]);
			pos.y = std::stof(buffer[1]);
			pos.z = std::stof(buffer[2]);
			camera(gameState, index, pos);
			buffer.clear();
			return;
		}
		assert(true, "somethings wrong, about data");
	}

	void flushFieldBuffer(GameState* gameState, std::vector<std::string>& buffer, const std::string& field) {
		if (field == "#activeCamera") {
			assert(buffer.size() == 1);
			gameState->activeCameraIndex = std::stoi(buffer[0]);
			buffer.clear();
		}
		if (field == "#editorCameraPos") {
			assert(buffer.size() == 3);
			Vector3 pos;
			pos.x = std::stof(buffer[0]);
			pos.y = std::stof(buffer[1]);
			pos.z = std::stof(buffer[2]);
			gameState->editorState.camera.position = pos;
			buffer.clear();
		}
		if (field == "#activeScene") {
			assert(buffer.size() == 1);
			gameState->activeScene = std::stoi(buffer[0]);
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
				if (!isSlotFree(gameState, i) && gameState->shapes[i].type != ShapeType::CONSTRAINT)
					highestLevelContainers.insert(findHighestLevelContainer(gameState, i));
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

			reference(gameState, sceneReferenceIndex, members, sceneName);

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

	void newScript(GameState* gameState, const std::string& filename, const std::string& sceneName, int index)
	{

		std::string line;

		// yes filename is index... open it from editor, deal with it. I don't think its a big deal
		std::ofstream outFile(filename);
		if (!outFile.is_open()) {
			std::cerr << "failed to open to write\n";
		}

		outFile << "#include \"middle_script_registry.h\"" << std::endl;
		outFile << "static std::string name = \"" << sceneName << index << "\";" << std::endl;
		outFile << std::endl;
		outFile << "static void print(middle::GameState* gameState) {" << std::endl;
		outFile << std::endl;
		outFile << "}" << std::endl;
		outFile << std::endl;
		outFile << "static bool registered = []() {" << std::endl;
		outFile << "\tmiddle::registerScript(name, &print);" << std::endl;
		outFile << "\treturn true;" << std::endl;
		outFile << "}();" << std::endl;

		outFile.flush();
		outFile.close();
	}
}

