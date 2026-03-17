#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "Rectangle.h"
#include "middle_shape_utils.h"
#include "MouseIntersectable.h"
#include "PlacementComponent.h"
#include "LoopSociety.h"
#include "Circle.h"
#include "middle_math.h"

class RectangleIntersectionSystem : public middle::MiddleGameplaySystem {
public:

	void init(middle::GameState* gameState) {

	}


	void update(middle::GameState* gameState) override {



	}
};

static middle::SystemRegistrar<RectangleIntersectionSystem> reg("RectangleIntersectionSystem");
