#pragma once
#include "game_state.h"
#include "registrars.h"

class BubbleRenderSetup : public middle::MiddleGameplaySystem {
	void update(middle::GameState* gameState) override {

	}
};

static middle::SystemRegistrar<BubbleRenderSetup> reg("BubbleRenderSetup");
