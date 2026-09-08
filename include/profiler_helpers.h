#pragma once
#include "game_state.h"

namespace middleProfiling {

	inline float slowSystemThreshold = 0.2f;
	inline float slowActionThreshold = 0.2f;

	void reviewSystemTime(middle::GameState* gameState, const middle::MiddleGameplaySystem* system) {
		float timeMs = system->updateTime.count();
		if (timeMs > slowSystemThreshold) {
			gameState->slowSystems.push_back(system->systemName + ": " + std::to_string(timeMs));
		}
	}


}
