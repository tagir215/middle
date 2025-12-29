#pragma once
#include <unordered_map>
#include "game_state.h"
#include "init_methods.h"

namespace middle {
	void loadSceneNames(GameState* gameState);
	void saveScene(GameState* gameState, const std::string& sceneName);
	void loadScene(GameState* gameState, const std::string& name, bool import, const Vector3& pos ={0,0,0}, int referenceIndex = 0);
	void saveEditorState(GameState* gameState);
	void loadEditorState(GameState* gameState);
	void newScript(GameState* gameState, const std::string& filename, const std::string& sceneName, int index);
}