#pragma once
#include "game_state.h"
#include "middle_shape_utils.h"
#include "middle_component_table.h"
#include "Superman.h"
#include <iostream>
#include "registrars.h"

namespace MoverSystem {

	static std::string scriptName = "MoverSystem";
	static int typeId = 0;

	class Mover : public middle::MiddleGameplaySystem {
		void update(middle::GameState* gameState) override {
			auto array = middle::getComponentArray<components::Superman>();
			for (auto& man : array) {
				std::cout << man.power << std::endl;
			}
		}
	};

	static middle::SystemRegistrar<Mover> reg(scriptName);

}
