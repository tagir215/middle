#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "middle_shape_utils.h"
#include "middle_component_table.h"
#include "MidComp/Position.h"
#include "MidComp/LoopSociety.h"
#include "MidComp/BubbleComponent.h"
#include "MidComp/BubbleUnit.h"
#include "MidComp/PhysicsData.h"
#include "MidComp/MouseGrabbable.h"
#include "MidComp/BubbleMultiplyComponent.h"

class BubbleCollisionSystem : public middle::MiddleGameplaySystem {

public:
	void init(middle::GameState* gameState) {
	}

	void update(middle::GameState* gameState) override {

	}
};

static middle::SystemRegistrar<BubbleCollisionSystem> reg("BubbleCollisionSystem");
