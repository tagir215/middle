#pragma once
#include "game_state.h"


namespace middle {
	extern GameState* gameStateRef;
	void sphere(int index, Vector3 position, int offset = 0);
	void constraint(int index, int indexA, int indexB, float targetDistance, int offset = 0);
	void loop(int index, const std::vector<int>& loopIndexes, int offset = 0);
	void reference(int index, const std::vector<int>& loopIndexes, const std::string& sceneName, int offset = 0);
}