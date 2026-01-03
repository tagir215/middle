#pragma once
#include "game_state.h"

static std::string scriptName = "/*scriptname*/";

class /*scriptname*/ : public middle::MiddleGameplayScript {
	void onCreate(middle::GameState* gameState) override {

	}

	void onUpdate(middle::GameState* gameState) override {

	}

	void onDestroy(middle::GameState* gameState) override {

	}
};

static middle::Registrar</*scriptname*/> reg(scriptName);
