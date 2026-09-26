#pragma once
#include "middle_constants.h"
#include "game_state.h"
#include "middle_shape_utils.h"

extern "C"
{
	__declspec(dllexport) void UpdateGame(const middle::MiddleInputState& inputState, middle::MiddleOutputState** outputState);
}

void closeGame(middle::GameState* gameState);
