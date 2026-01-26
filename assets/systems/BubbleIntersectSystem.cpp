#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "middle_shape_utils.h"
#include "LoopSociety.h"
#include "BubbleComponent.h"
#include "Position.h"
#include "bubble_utils.h"

class BubbleIntersectSystem : public middle::MiddleGameplaySystem {


	void update(middle::GameState* gameState) override {


		middle::loopInstances(gameState, [gameState, this](int i, middle::Shape& shape) {

			auto bubble = middle::getComponent<components::BubbleComponent>(shape);
			if (!bubble)
				return;

			// check that children are not already intersecting
			std::vector<middle::Id>children;
			middle::getChildren(gameState, shape.id, children);
			for (auto& childId : children) {
				auto& childShape = middle::getShape(gameState, childId.index);
				auto childBubble = middle::getComponent<components::BubbleComponent>(childShape);
				if (childBubble && childBubble->intersecting) {
					bubble->intersecting = false;
					return;
				}
			}

			// update intersecting status
			Vector3 mousePos = gameState->input.mouseXZ_PlanePos;
			bool intersecting = bubble::pointIntersectBubble(gameState, shape, mousePos);
			bubble->intersecting = intersecting;

			});
	}
};

static middle::SystemRegistrar<BubbleIntersectSystem> reg("BubbleIntersectSystem");
