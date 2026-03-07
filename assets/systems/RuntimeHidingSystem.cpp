#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "middle_shape_utils.h"
#include "RuntimeHiddenTag.h"

class RuntimeHidingSystem : public middle::MiddleGameplaySystem {
public:
	void init(middle::GameState* gameState) {

	}
	void update(middle::GameState* gameState) override {
		middle::loopInstances(gameState, [gameState](int i, middle::Shape& shape) {
			return false;
			});
	}
};

static middle::SystemRegistrar<RuntimeHidingSystem> reg("RuntimeHidingSystem");
