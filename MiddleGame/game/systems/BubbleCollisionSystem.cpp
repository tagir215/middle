#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "middle_shape_utils.h"
#include "middle_component_table.h"
#include "Position.h"
#include "LoopSociety.h"
#include "BubbleComponent.h"
#include "BubbleUnit.h"
#include "PhysicsData.h"
#include "MouseGrabbable.h"
#include "BubbleMultiplyComponent.h"

class BubbleCollisionSystem : public middle::MiddleGameplaySystem {

public:
	void init(middle::GameState* gameState) {
	}

	void update(middle::GameState* gameState) override {

	}
};

static middle::SystemRegistrar<BubbleCollisionSystem> reg("BubbleCollisionSystem");
