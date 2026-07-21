#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "middle_shape_utils.h"
#include "LoopSociety.h"
#include "BubbleComponent.h"
#include "bubble_utils.h"
#include "MouseGrabbable.h"
#include "MouseIntersectable.h"
#include "BubbleUnit.h"
#include "FractionalComponent.h"
#include "middle_math.h"
#include "Sphere.h"
#include "Circle.h"
#include "Rectangle.h"
#include "PlacementComponent.h"
#include "InventoryItem.h"
#include "UiComponent.h"
#include "UnIntersectableWindowComponent.h"
#include "component_utils.h"
#include "GlobalTransform.h"

class BubbleIntersectSystem : public middle::MiddleGameplaySystem {
public:

	components::CompCache* bubbleCache;
	components::CompCache* rectangleCache;
	components::CompCache* nonBubbleCircleCache;
	components::CompCache* unIntersectableBubbleCache;

	void init(middle::GameState* gameState) {
		bubbleCache = middle::newCompCache(gameState, systemName);
		bubbleCache->addType<components::BubbleComponent>();
		bubbleCache->addType<components::MouseIntersectable>();
		bubbleCache->addType<components::Circle>();
		bubbleCache->addType<components::GlobalTransform>();
		bubbleCache->addType<components::UnIntersectableWindowComponent>(components::NOTINTERESTED);

		rectangleCache = middle::newCompCache(gameState, systemName);
		rectangleCache->addType<components::MouseIntersectable>();
		rectangleCache->addType<components::Rectangle>();

		nonBubbleCircleCache = middle::newCompCache(gameState, systemName);
		nonBubbleCircleCache->addType<components::Circle>();
		nonBubbleCircleCache->addType<components::GlobalTransform>();
		nonBubbleCircleCache->addType<components::MouseIntersectable>();
		nonBubbleCircleCache->addType<components::BubbleComponent>(components::NOTINTERESTED);

		unIntersectableBubbleCache = middle::newCompCache(gameState, systemName);
		unIntersectableBubbleCache->addType<components::BubbleComponent>();
		unIntersectableBubbleCache->addType<components::UnIntersectableWindowComponent>();
	}

	bool isPlacedRecursive(middle::GameState* gameState, middle::Id& id) {
		auto& shape = middle::getShape(gameState, id.index);
		auto placement = middle::getComponent<components::PlacementComponent>(shape);
		if (placement) {
			return true;
		}
		middle::Id parentId = middle::getParent(gameState, shape.id);
		if (parentId.index != middle::UNASSIGNED) {
			return isPlacedRecursive(gameState, parentId);
		}
		return false;
	}

	void childrenIntersecting(middle::GameState* gameState, middle::Shape& shape, components::MouseIntersectable* intersectable) {
		// check that not intersecting children as well
		if (intersectable->intersecting) {
			intersectable->intersectingTop = true;
			std::vector<middle::Id>children;
			middle::getAllChildren(gameState, shape.id, children);
			for (middle::Id childId : children) {
				auto& child = middle::getShape(gameState, childId.index);
				auto intersectableChild = middle::getComponent<components::MouseIntersectable>(child);
				if (intersectableChild && intersectableChild->intersecting) {
					intersectable->intersectingTop = false;
					break;
				}
			}
		}
	}

	void update(middle::GameState* gameState) override {

		auto unintersectableIt = unIntersectableBubbleCache->begin<components::UnIntersectableWindowComponent>();
		for (middle::Id& id : unIntersectableBubbleCache->relevantIdVector) {
			auto unIntersectable = *unintersectableIt;
			unIntersectable->timeLeft -= gameState->frameTime;
			if (unIntersectable->timeLeft <= 0) {
				middle::queueComponentDeletion<components::UnIntersectableWindowComponent>(gameState, id);
			}
		}

		bool uiIntersected = false;
		auto rectangleIt = rectangleCache->begin<components::Rectangle>();
		auto intersectableIt = rectangleCache->begin<components::MouseIntersectable>();
		for (int i = 0; i < rectangleCache->getSize(); ++i) {
			auto& shape = middle::getShape(gameState, rectangleCache->relevantIdVector[i].index);
			auto rectangle = *rectangleIt;
			auto intersectable = *intersectableIt;

			auto uiComp = middle::getComponent<components::UiComponent>(shape);
			if (uiComp && uiComp->type == UiElementTypes::UI_BACKGROUND) {
				continue;
			}

			Vector3 position = middle::getGlobalPosition(gameState, shape.id.index);
			Vector3 scale = middle::getTotalScale(gameState, shape.id);
			Vector3 uiPlaneIntersectPoint = middle::RayCastLinePlane(position, { 0,-1,0 },
				gameState->input.mouseNearPlanePos, gameState->input.mouseDir);
			Vector3 mouseXZ = uiPlaneIntersectPoint;

			float axisX = rectangle->width * 0.5f * scale.x;
			float axisZ = rectangle->height * 0.5f * scale.z;
			intersectable->intersecting =
				mouseXZ.x > position.x - axisX && mouseXZ.x < position.x + axisX &&
				mouseXZ.z > position.z - axisZ && mouseXZ.z < position.z + axisZ;


			if (intersectable->intersecting) {
				uiIntersected = true;
			}

			intersectable->intersectingTop = false;
			if (isPlacedRecursive(gameState, shape.id)) {
				intersectable->intersecting = false;
				continue;
			}
			childrenIntersecting(gameState, shape, intersectable);
		}

		auto circleIt = nonBubbleCircleCache->begin<components::Circle>();
		auto circleIntersectableIt = nonBubbleCircleCache->begin<components::MouseIntersectable>();
		for (int i = 0; i < nonBubbleCircleCache->getSize(); ++i) {
			auto& shape = middle::getShape(gameState, nonBubbleCircleCache->relevantIdVector[i].index);
			auto circle = *circleIt;
			auto intersectable = *circleIntersectableIt;
			Vector3 position = middle::getGlobalPosition(gameState, shape.id.index);
			Vector3 scale = middle::getTotalScale(gameState, shape.id);
			Vector3 uiPlaneIntersectPoint = middle::RayCastLinePlane(position, { 0,-1,0 },
				gameState->input.mouseNearPlanePos, gameState->input.mouseDir);
			Vector3 mouseXZ = uiPlaneIntersectPoint;

			if (circle) {
				intersectable->intersecting = Vector3DistanceSqr(mouseXZ, position) < circle->radius * circle->radius;
			}

			if (intersectable->intersecting) {
				uiIntersected = true;
			}

			intersectable->intersectingTop = false;
			if (isPlacedRecursive(gameState, shape.id)) {
				intersectable->intersecting = false;
				continue;
			}
			childrenIntersecting(gameState, shape, intersectable);
		}


		auto bubbleIntersectableIt = bubbleCache->begin<components::MouseIntersectable>();
		auto bubbleTransformIt = bubbleCache->begin<components::GlobalTransform>();
		auto bubbleCircleIt = bubbleCache->begin<components::Circle>();
		for (int i = 0; i < bubbleCache->getSize(); ++i) {
			auto& shape = middle::getShape(gameState, bubbleCache->relevantIdVector[i].index);
			auto intersectable = *bubbleIntersectableIt;
			auto transform = *bubbleTransformIt;
			auto circle = *bubbleCircleIt;
			intersectable->intersecting = false;
			intersectable->intersectingTop = false;

			auto grabbable = middle::getComponent<components::MouseGrabbable>(shape);
			if (grabbable && grabbable->grabbing) {
				continue;
			}

			bool isInventoryItem = middle::getComponent<components::InventoryItem>(shape) != nullptr;
			if (uiIntersected && !isInventoryItem) {
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

			Vector3 pos = transform->pos;
			float globalR = circle->radius * transform->scale.x;
			Vector3 mousePos = middle::RayCastLinePlane(pos, { 0,1,0 }, gameState->activeCamera.position, gameState->input.mouseDir);

			bool intersecting = Vector3DistanceSqr(pos, mousePos) < globalR * globalR;
			intersectable->intersecting = intersecting;

			if (!alreadyIntersecting) {
				intersectable->intersectingTop = intersecting;
			}
		}


	}
};

static middle::SystemRegistrar<BubbleIntersectSystem> reg("BubbleIntersectSystem");
