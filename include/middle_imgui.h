#pragma once
#include "game_state.h"
#include "rlImGui.h"
#include "imgui.h"
#include <cstdlib>
#include <thread>
#include "rlgl.h"
#include "middle_shape_utils.h"

namespace middle {
	void setupUI(GameState* gameState);
}