#pragma once
#include "game_state.h"
#include "registrars.h"

class TestSystem : public middle::MiddleGameplaySystem {
	void update(middle::GameState* gameState) override {

	}
};

static middle::SystemRegistrar<TestSystem> reg("TestSystem");
