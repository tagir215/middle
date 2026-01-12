#pragma once
#include "game_state.h"
#include "registrars.h"

class /*systemName*/ : public middle::MiddleGameplaySystem {
	void update(middle::GameState* gameState) override {

	}
};

static middle::SystemRegistrar</*systemName*/> reg("/*systemName*/");
