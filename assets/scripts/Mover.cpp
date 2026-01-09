#pragma once
#include "game_state.h"
#include "middle_shape_utils.h"
#include "middle_component_table.h"
#include <iostream>
#include "registrars.h"

namespace MoverSystem {

	static std::string scriptName = "MoverSystem";
	static int typeId = 0;

	class Mover : public middle::MiddleGameplaySystem {
		void update(middle::GameState* gameState) override {
		}
	};

	static middle::SystemRegistrar<Mover> reg(scriptName);

}
