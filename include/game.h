#pragma once
#include "middle_constants.h"
#include "game_state.h"
#include "middle_shape_utils.h"

using namespace middle;

extern "C" 
{
	__declspec(dllexport) void UpdateGame(GameState* gameState);
}

void closeGame(GameState* gameState);
