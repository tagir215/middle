#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "UiComponent.h"
#include "middle_component_table.h"
#include "middle_shape_utils.h"
#include "Constraint.h"
#include "LoopSociety.h"
#include "Position.h"
#include "bubble_algebra_buttons.h"

class BubbleUiSystem : public middle::MiddleGameplaySystem {
public:


	void init(middle::GameState* gameState) {
	}
	void update(middle::GameState* gameState) override {

	}
};

static middle::SystemRegistrar<BubbleUiSystem> reg("BubbleUiSystem");
