#pragma once
#include "game_state.h"


namespace middle {
	extern GameState* gameStateRef;
	void sphere(int index, Vector3 position);
	void constraint(int index, int indexA, int indexB, float targetDistance);
	void loop(int index, std::vector<int> loopIndexes);
}