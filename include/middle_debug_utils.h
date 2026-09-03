#pragma once
#include "game_state.h"

namespace middle{
	void drawImGuiFloat(middle::GameState* gameState, const char* label, float f);
	void drawImGuiIntVector(middle::GameState* gameState, const char* label, const std::vector<int>& vector);
}
