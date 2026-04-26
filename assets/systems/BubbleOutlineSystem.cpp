#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "middle_shape_utils.h"
#include "LoopSociety.h"
#include "LoopTag.h"
#include "BubbleComponent.h"
#include "Sphere.h"
#include "Position.h"
#include "Constraint.h"
#include "PhysicsData.h"
#include "PlacementComponent.h"
#include <random>
#include "editor_actions.h"
#include "BubbleRef.h" 
#include "DependencyComponent.h"
#include "bubble_utils.h"
#include "component_utils.h"

class BubbleOutlineSystem : public middle::MiddleGameplaySystem {
public:
	void init(middle::GameState* gameState) {

	}

	void update(middle::GameState* gameState) {

	}
};

static middle::SystemRegistrar<BubbleOutlineSystem> reg("BubbleOutlineSystem");
