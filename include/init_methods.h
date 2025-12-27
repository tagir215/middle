#pragma once
#include "game_state.h"


namespace middle {
	void sphere(GameState* gameState, int index, Vector3 position, int offset = 0);
	void constraint(GameState* gameState, int index, int indexA, int indexB, float targetDistance, int offset = 0);
	void loop(GameState* gameState, int index, const std::vector<int>& loopIndexes, int offset = 0);
	void reference(GameState* gameState, int index, const std::vector<int>& loopIndexes, const std::string& sceneName, int offset = 0);
}