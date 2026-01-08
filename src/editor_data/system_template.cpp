#pragma once
#include "game_state.h"

static std::string scriptName = "/*scriptname*/";

class /*scriptname*/ : public middle::MiddleGameplaySystem {
	void update(middle::GameState* gameState) override {

	}
};

static middle::SystemRegistrar</*scriptname*/> reg(scriptName);
