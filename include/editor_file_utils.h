#pragma once
#include <unordered_map>
#include "game_state.h"
#include <any>
#include <typeinfo>
#include <string>

namespace middle {
	void loadSceneAndShapeNames(GameState* gameState);
	void loadSystemNames(GameState* gameState);
	void loadComponentNames(GameState* gameState);
	void saveScene(GameState* gameState, const std::string& sceneName);
	void saveShape(GameState* gameState, Id& idToSave, const std::string& folder, const std::string& shapeName);
	middle::Id loadScene(GameState* gameState, const std::string& folder, const std::string& sceneName, bool import, const Vector3& pos = { 0,0,0 }, int referenceIndex = 0);
	std::vector<std::string> loadFileNamesInFolder(const std::string& folder);
	middle::Id loadShape(GameState* gameState, const std::string& folder, const std::string& sceneName, bool import, const Vector3& pos = {0,0,0});
	void saveEditorState(GameState* gameState);
	void loadEditorState(GameState* gameState);
	void newSystemFile(GameState* gameState, const std::string& scriptName);
	void newComponentFile(GameState* gameState, const std::string& componentName);
	std::string fieldToString(const std::any& field);
	FieldType fieldToType(const std::any& field);
	void fillField(void* field, const std::string& fieldString, int indexOffset = 0);
	void saveTempShape(GameState* gameState, Id& idToSave);
	middle::Id loadTempShape(GameState* gameState, Id& idToLoad);
	void resetGenerations(GameState* gameState);
	void incrementGenerations(GameState* gameState);
	void resetScene(GameState* gameState);
	void queueSound(GameState* gameState, const std::string& soundName);
}