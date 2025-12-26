#pragma once

#include "game_state.h"
namespace middle {
	extern std::vector<PhysicsBody*> physicsBodies;

	void reset(GameState* gameState);

	void updateInstances(GameState* gameState);
}
