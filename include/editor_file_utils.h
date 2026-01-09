#pragma once
#include <unordered_map>
#include "game_state.h"
#include "init_methods.h"
#include <any>
#include <typeinfo>
#include <string>

namespace middle {
	struct Serializable {
		virtual void serialize(std::ostream& istream) = 0;
		virtual void deserialize(const std::vector<std::string>& buffer) = 0;
	};

	void loadSceneNames(GameState* gameState);
	void loadScriptNames(GameState* gameState);
	void loadComponentNames(GameState* gameState);
	void saveScene(GameState* gameState, const std::string& sceneName);
	void loadScene(GameState* gameState, const std::string& name, bool import, const Vector3& pos ={0,0,0}, int referenceIndex = 0);
	void saveEditorState(GameState* gameState);
	void loadEditorState(GameState* gameState);
	void newSystemFile(GameState* gameState, const std::string& scriptName);
	void newComponentFile(GameState* gameState, const std::string& componentName);
	std::string fieldToString(const std::any& field);
	void fillField(void* field, const std::string& fieldString);
}