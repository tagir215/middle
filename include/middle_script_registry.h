#pragma once
#include "game_state.h"
#include <string>
#include <vector>

namespace middle {
	void runScript(GameState* gameState, int index);
	void registerScript(std::string name, GameScript script);
	bool scriptExists(const std::string name);
}