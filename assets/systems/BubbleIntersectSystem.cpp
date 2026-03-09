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

class BubbleIntersectSystem : public middle::MiddleGameplaySystem {
public:

	components::CompCache* bubbleCache;
	components::CompCache* unitCache;

	void init(middle::GameState* gameState) {
		bubbleCache = middle::newCompCache(gameState);
		bubbleCache->addType<components::BubbleComponent>();
		bubbleCache->addType<components::MouseIntersectable>();

		unitCache = middle::newCompCache(gameState);
		unitCache->addType<components::BubbleUnit>();
		unitCache->addType<components::MouseIntersectable>();
		unitCache->addType<components::Sphere>();
	}

	void update(middle::GameState* gameState) override {

		auto unitIt = unitCache->begin<components::BubbleUnit>();
		auto unitIntersectableIt = unitCache->begin<components::MouseIntersectable>();
		auto sphereIt = unitCache->begin<components::Sphere>();
		for (int i = 0; i < unitCache->getSize(); ++i) {
			auto& shape = middle::getShape(gameState, unitCache->relevantIdVector[i].index);
			auto unit = *unitIt;
			auto intersectable = *unitIntersectableIt;
			auto sphere = *sphereIt;
			intersectable->intersecting = false;
			intersectable->intersectingTop = false;

			if (gameState->bubbleAlgebraState.intersectingUI) {
				continue;
			}

			Vector3 pos = middle::getShapePosition(gameState, shape.id.index);
			Vector3 intersectPos;
			bool intersecting = middle::RayCastLineSphere(pos, sphere->radius, gameState->activeCamera.position,
				gameState->activeCamera.position + gameState->input.mouseDir, intersectPos);

			intersectable->intersecting = intersecting;
			intersectable->intersectingTop = intersecting;
		}


		auto bubbleIt = bubbleCache->begin<components::BubbleComponent>();
		auto bubbleIntersectableIt = bubbleCache->begin<components::MouseIntersectable>();
		for (int i = 0; i < bubbleCache->getSize(); ++i) {
			auto& shape = middle::getShape(gameState, bubbleCache->relevantIdVector[i].index);
			auto bubble = *bubbleIt;
			auto intersectable = *bubbleIntersectableIt;
			intersectable->intersecting = false;
			intersectable->intersectingTop = false;

			auto grabbable = middle::getComponent<components::MouseGrabbable>(shape);
			if (grabbable && grabbable->grabbing) {
				continue;
			}

			if (gameState->bubbleAlgebraState.intersectingUI) {
				continue;
			}

			// check that children are not already intersecting or grabbing 
			bool alreadyIntersecting = false;
			std::vector<middle::Id>children;
			middle::getAllChildren(gameState, shape.id, children);
			for (auto& childId : children) {
				auto& childShape = middle::getShape(gameState, childId.index);
				auto childIntersectable = middle::getComponent<components::MouseIntersectable>(childShape);
				if (childIntersectable && childIntersectable->intersecting) {
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

			Vector3 pos = middle::getShapePosition(gameState, shape.id.index);
			Vector3 mousePos = middle::RayCastLinePlane(pos, { 0,1,0 }, gameState->activeCamera.position, gameState->input.mouseDir);

			bool intersecting = bubble::pointIntersectBubble(gameState, shape, mousePos);

			intersectable->intersecting = intersecting;
			if (!alreadyIntersecting) {
				intersectable->intersectingTop = intersecting;
			}
		}

	}
};

static middle::SystemRegistrar<BubbleIntersectSystem> reg("BubbleIntersectSystem");
