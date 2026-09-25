#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "middle_shape_utils.h"
#include "PhysicsData.h"
#include "BubbleComponent.h"
#include "component_utils.h"
#include "Circle.h"
#include "LoopSociety.h"
#include "Rectangle.h"
#include "TopDogBubbleTag.h"
#include "DeleteComponent.h" 
#include "IdRef.h"
#include "GlobalTransform.h"
#include "LocalPosition.h"
#include "GlobalRadius.h"

class BubblePhysics : public middle::MiddleGameplaySystem {
public:

	void init(middle::GameState* gameState) {

	}

	void update(middle::GameState* gameState) override {

	}
};

static middle::SystemRegistrar<BubblePhysics> reg("BubblePhysics");
