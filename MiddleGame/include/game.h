#pragma once
#include "middle_constants.h"
#include "game_state.h"
#include "middle_shape_utils.h"

extern "C"
{
	__declspec(dllexport) void UpdateGame(middle::GameState* gameState);
}

void closeGame(middle::GameState* gameState);
