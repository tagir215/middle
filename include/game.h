#pragma once
#include "middle_constants.h"
#include "middle_shape_utils.h"
#include "editor_file_utils.h"
#include "middle_math.h"
#include "editor_actions.h"
using namespace middle;

std::vector<PhysicsBody*>physicsBodies;

extern "C" 
{
	__declspec(dllexport) void UpdateGame(GameState* renderData);
}

