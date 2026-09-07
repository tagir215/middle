#pragma once
#include "game_state.h"

namespace middle{
	void drawImGuiInt(middle::GameState* gameState, const char* label, int i);
	void drawImGuiFloat(middle::GameState* gameState, const char* label, float f);
	void drawImGuiIntVector(middle::GameState* gameState, const char* label, const std::vector<int>& vector);
}
