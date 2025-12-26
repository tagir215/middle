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

		outFile << "# scene:" << std::endl;
		outFile << sceneName << std::endl;
		outFile << std::endl;

		int cumulatedOffset = 0;

		for (int i = 0; i < gameState->shapes.size(); ++i) {
			// skip empty parts if over max used index
			if (i > maxIndex && gameState->isSlotFree(i))
				continue;

			auto& shape = gameState->shapes[i];

			// do references in second pass, 
			if (shape.isReferenceShape) {
				++cumulatedOffset;
				continue;
			}

			outFile << "__" << std::to_string((int)shape.type) << "__" << std::endl;

			if (shape.type == ShapeType::SPHERE) {
				outFile << coordToLines(shape.position) << std::endl;
			}
			if (shape.type == ShapeType::CONSTRAINT) {
				outFile << shape.constraint.targetDistance << std::endl;
				// IMPORTANT: cumulated offset cumulates because of the scene references, so all the above saved indexes are shifted down for the save file
				outFile << shape.constraint.indexA - cumulatedOffset << std::endl;
				outFile << shape.constraint.indexB - cumulatedOffset << std::endl;
			}
			if (shape.type == ShapeType::LOOP) {
				for (int mIndex = shape.loopArrayOffset; mIndex < shape.loopArrayOffset + shape.loopSize; ++mIndex) {
					// IMPORTANT: cumulated offset here too
					outFile << gameState->loopMembers[mIndex] - cumulatedOffset << std::endl;
				}
			}
		}

		// do second pass to save references at the end of the file
		for (int i = 0; i < gameState->shapes.size(); ++i) {
			auto& shape = gameState->shapes[i];
			if (shape.type == ShapeType::REFERENCE) {
				outFile << "__" << std::to_string((int)shape.type) << "__" << std::endl;
				outFile << shape.name << std::endl;
				outFile << coordToLines(shape.position) << std::endl;
			}
		}

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
			sphere(index, pos, offset);
			buffer.clear();
			return;
		}
		if (type == (int)ShapeType::CONSTRAINT) {
			assert(buffer.size() == 3);
			float targetDistance = std::stof(buffer[0]);
			int indexA = std::stoi(buffer[1]);
			int indexB = std::stoi(buffer[2]);
			constraint(index, indexA, indexB, targetDistance, offset);
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
			loop(index, memberIndexes, offset);
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
			loadScene(gameState, sceneName, true, pos);
			return;
		}
		assert(true, "somethings wrong, about data");
	}

	void loadScene(GameState* gameState, const std::string& sceneName, bool import, const Vector3& pos) {
		gameStateRef = gameState;

		std::string filename = "../assets/scenes/" + sceneName + ".midsc";

		std::ifstream inputFile(filename);
		if (!inputFile.is_open()) {
			std::cerr << "Failed to open file to write";
			return;
		}

		std::string line;
		int currentType = -1;
		int currentIndex = 0;

		// index offset is offsetting all the indexes of scene by current scenes highest used index, if we are importing a scene
		int indexOffset = 0;
		if (import) {
			indexOffset = findHighestUsedIndex(gameState) + 1;
		}

		// if not importing make sure loop index is 0, 
		if (!import) {
			gameState->loopIndex = 0;
		}

		std::vector<std::string>buffer;

		while (std::getline(inputFile, line)) {
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
			if(currentType >= 0)
				buffer.push_back(line);
		}

		flushBuffer(gameState, buffer, currentType, currentIndex, indexOffset);

		inputFile.close();


		// if we import we contain all the content in a reference loop
		if (import) {
			currentIndex++;
			int shapesAddedCount = currentIndex;
			std::set<int>highestLevelContainers;
			for (int i = 0; i < shapesAddedCount; ++i) {
				// skip nons and skip constraints since they don't have parents
				if(!gameState->isSlotFree(indexOffset + i) && gameState->shapes[indexOffset + i].type != ShapeType::CONSTRAINT)
					highestLevelContainers.insert(findHighestLevelContainer(gameState, indexOffset + i));
			}

			// make reference
			std::vector<int>members;
			for (int v : highestLevelContainers) {
				members.push_back(v);
			}
			reference(indexOffset + currentIndex, members, sceneName);

			// set as reference shapes,  its good to know..
			for (int i = 0; i < shapesAddedCount; ++i) {
				Shape& shape = gameState->shapes[indexOffset + i];
				shape.isReferenceShape = true;
			}

			// move imported scene where it wants to be
			moveShape(gameState, indexOffset + currentIndex, pos);
		}
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

