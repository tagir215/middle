#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "middle_shape_utils.h"

class VariableLinkingSystem : public middle::MiddleGameplaySystem {
public:


	void init(middle::GameState* gameState) {
	}


	void update(middle::GameState* gameState) override {
	}
};

static middle::SystemRegistrar<VariableLinkingSystem> reg("VariableLinkingSystem");
