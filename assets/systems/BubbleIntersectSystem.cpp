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
#include "BubbleUnit.h"
#include "FractionalComponent.h"
#include "middle_math.h"
#include "Sphere.h"
#include "middle_math.h"

class BubbleIntersectSystem : public middle::MiddleGameplaySystem {


	void update(middle::GameState* gameState) override {

		middle::loopInstances(gameState, [gameState, this](int i, middle::Shape& shape) {

			auto bubble = middle::getComponent<components::BubbleComponent>(shape);
			auto unit = middle::getComponent<components::BubbleUnit>(shape);
			if (!bubble && !unit)
				return;

			auto intersectable = middle::getComponent<components::MouseIntersectable>(shape);
			assert(intersectable);

			// check that children are not already intersecting or grabbing 
			bool alreadyIntersecting = false;
			std::vector<middle::Id>children;
			middle::getAllChildren(gameState, shape.id, children);
			for (auto& childId : children) {
				auto& childShape = middle::getShape(gameState, childId.index);
				auto childIntersectable = middle::getComponent<components::MouseIntersectable>(childShape);
				if (childIntersectable && childIntersectable->intersectingTop) {
					intersectable->intersectingTop = false;
					alreadyIntersecting = true;
					break;
				}
				auto grabbable = middle::getComponent<components::MouseGrabbable>(childShape);
				if (childIntersectable && grabbable->grabbing) {
					intersectable->intersectingTop = false;
					alreadyIntersecting = true;
				}
			}

			// update intersecting status

			auto position = middle::getComponent<components::Position>(shape);
			Vector3 pos = middle::getShapePosition(gameState, shape.id.index);
			Vector3 mousePos = middle::RayCastLinePlane(pos, { 0,1,0 }, gameState->activeCamera.position, gameState->input.mouseDir);

			bool intersecting = false;
			if (bubble) {
				intersecting = bubble::pointIntersectBubble(gameState, shape, mousePos);
			}
			if (unit) {
				auto sphere = middle::getComponent<components::Sphere>(shape);
				assert(position && sphere);
				Vector3 intersectPos;
				intersecting = middle::RayCastLineSphere(pos, sphere->radius, gameState->activeCamera.position,
					gameState->activeCamera.position + gameState->input.mouseDir, intersectPos);
			}
			intersectable->intersecting = intersecting;
			if (!alreadyIntersecting) {
				intersectable->intersectingTop = intersecting;
			}

			});
	}
};

static middle::SystemRegistrar<BubbleIntersectSystem> reg("BubbleIntersectSystem");
