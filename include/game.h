#pragma once
#include "middle_constants.h"
#include "game_state.h"
#include "middle_shape_utils.h"

using namespace middle;

float slowSystemThreshold = 0.5f;
float slowActionThreshold = 0.4f;
std::vector<std::string>slowSystems;
std::vector<std::string>slowActions;

extern "C"
{
	__declspec(dllexport) void UpdateGame(GameState* gameState);
}

void closeGame(GameState* gameState);
