#pragma once
#include "game_state.h"


namespace middle {
	void initJoint(GameState* gameState, int index, Vector3 position, int offset = 0);
	void initConstraint(GameState* gameState, int index, int indexA, int indexB, float targetDistance, int offset = 0);
	void initLoop(GameState* gameState, int index, const std::vector<int>& loopIndexes, int offset = 0);
	void initReference(GameState* gameState, int index, const std::vector<int>& loopIndexes, const std::string& sceneName, int offset = 0);
	void initCamera(GameState* gameState, int index, const Vector3& position);
	void initScript(GameState* gameState, int index, const std::string& scriptName, const Vector3& position);
}