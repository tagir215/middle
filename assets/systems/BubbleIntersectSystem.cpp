#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "middle_shape_utils.h"
#include "LoopSociety.h"
#include "BubbleComponent.h"
#include "Position.h"
#include "bubble_utils.h"
#include "MouseGrabbable.h"
#include "MouseIntersectable.h"

class BubbleIntersectSystem : public middle::MiddleGameplaySystem {


	void update(middle::GameState* gameState) override {

		middle::loopInstances(gameState, [gameState, this](int i, middle::Shape& shape) {

			auto bubble = middle::getComponent<components::BubbleComponent>(shape);
			if (!bubble)
				return;

			auto intersectable = middle::getComponent<components::MouseIntersectable>(shape);
			assert(intersectable);

			// check that children are not already intersecting or grabbing 
			bool alreadyIntersecting = false;
			std::vector<middle::Id>children;
			middle::getChildren(gameState, shape.id, children);
			for (auto& childId : children) {
				auto& childShape = middle::getShape(gameState, childId.index);
				auto childIntersectable = middle::getComponent<components::MouseIntersectable>(childShape);
				auto childBubble = middle::getComponent<components::BubbleComponent>(childShape);
				if (childBubble && childIntersectable->intersectingTop) {
					intersectable->intersectingTop = false;
					alreadyIntersecting = true;
					break;
				}
				auto grabbable = middle::getComponent<components::MouseGrabbable>(childShape);
				if (grabbable && grabbable->grabbing) {
					intersectable->intersectingTop = false;
					alreadyIntersecting = true;
				}
			}

			// update intersecting status
			Vector3 mousePos = gameState->input.mouseXZ_PlanePos;
			bool intersecting = bubble::pointIntersectBubble(gameState, shape, mousePos);
			intersectable->intersecting = intersecting;
			if (!alreadyIntersecting) {
				intersectable->intersectingTop = intersecting;
			}

			});
	}
};

static middle::SystemRegistrar<BubbleIntersectSystem> reg("BubbleIntersectSystem");
