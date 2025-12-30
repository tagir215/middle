#pragma once
#include "middle_constants.h"
#include "editor_update.h"
using namespace middle;

extern "C" 
{
	__declspec(dllexport) void UpdateGame(GameState* renderData);
	void closeGame(GameState* gameState);
}

