#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "middle_shape_utils.h"
#include "MidComp/PhysicsData.h"
#include "MidComp/BubbleComponent.h"
#include "component_utils.h"
#include "MidComp/Circle.h"
#include "MidComp/LoopSociety.h"
#include "MidComp/Rectangle.h"
#include "MidComp/TopDogBubbleTag.h"
#include "MidComp/DeleteComponent.h" 
#include "MidComp/IdRef.h"
#include "MidComp/GlobalTransform.h"
#include "MidComp/LocalPosition.h"
#include "MidComp/GlobalRadius.h"

class BubblePhysics : public middle::MiddleGameplaySystem {
public:

	void init(middle::GameState* gameState) {

	}

	void update(middle::GameState* gameState) override {

	}
};

static middle::SystemRegistrar<BubblePhysics> reg("BubblePhysics");
