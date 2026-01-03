#pragma once
#include "game_state.h"
#include <iostream>

static std::string scriptName = "girlman";

class girlman : public middle::MiddleGameplayScript {
	void onCreate(middle::GameState* gameState) override {

	}

	void onUpdate(middle::GameState* gameState) override {

	}

	void onDestroy(middle::GameState* gameState) override {

	}
};

static middle::Registrar<girlman> reg(scriptName);
