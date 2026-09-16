#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"

class TestSystem : public middle::MiddleGameplaySystem {
public:
	void init(middle::GameState* gameState) {

	}
	void update(middle::GameState* gameState) override {

	}
};

static middle::SystemRegistrar<TestSystem> reg("TestSystem");
