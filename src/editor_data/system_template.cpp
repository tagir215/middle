#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"

class /*systemName*/ : public middle::MiddleGameplaySystem {
	void init(middle::GameState* gameState) override {

	}
	void update(middle::GameState* gameState) override {

	}
};

static middle::SystemRegistrar</*systemName*/> reg("/*systemName*/");
