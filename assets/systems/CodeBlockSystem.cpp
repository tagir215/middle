#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "middle_shape_utils.h"

class CodeBlockSystem : public middle::MiddleGameplaySystem {
public:
	CodeBlockSystem() {
		systemModeType = middle::SystemModeType::GAMEPLAY;
	}


	void init(middle::GameState* gameState) {
	}

	void update(middle::GameState* gameState) override {

	}
};

static middle::SystemRegistrar<CodeBlockSystem> reg("CodeBlockSystem");
