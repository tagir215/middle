#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "middle_shape_utils.h"
#include "MidComp/BubbleComponent.h"
#include "MidComp/Position.h"
#include "MidComp/PhysicsData.h"
#include "MidComp/LoopSociety.h"
#include "bubble_utils.h"
#include "MidComp/BubbleRef.h"

class BubbleOutlinePhysics : public middle::MiddleGameplaySystem {
public:
	void init(middle::GameState* gameState) {
	}

	void update(middle::GameState* gameState) override {

	}
};

static middle::SystemRegistrar<BubbleOutlinePhysics> reg("BubbleOutlinePhysics");
