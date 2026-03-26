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
#include "Circle.h"
#include "Rectangle.h"
#include "PlacementComponent.h"
#include "InventoryItem.h"

class BubbleIntersectSystem : public middle::MiddleGameplaySystem {
public:

	components::CompCache* bubbleCache;
	components::CompCache* unitCache;
	components::CompCache* rectangleCache;
	components::CompCache* nonBubbleCircleCache;

	void init(middle::GameState* gameState) {
		bubbleCache = middle::newCompCache(gameState);
		bubbleCache->addType<components::BubbleComponent>();
		bubbleCache->addType<components::MouseIntersectable>();
		bubbleCache->addType<components::Circle>();
		bubbleCache->addType<components::Position>();

		unitCache = middle::newCompCache(gameState);
		unitCache->addType<components::BubbleUnit>();
		unitCache->addType<components::MouseIntersectable>();
		unitCache->addType<components::Circle>();
		unitCache->addType<components::Position>();

		rectangleCache = middle::newCompCache(gameState);
		rectangleCache->addType<components::MouseIntersectable>();
		rectangleCache->addType<components::Rectangle>();

		nonBubbleCircleCache = middle::newCompCache(gameState);
		nonBubbleCircleCache->addType<components::Circle>();
		nonBubbleCircleCache->addType<components::Position>();
		nonBubbleCircleCache->addType<components::MouseIntersectable>();
		nonBubbleCircleCache->addType<components::BubbleComponent>(components::NOTINTERESTED);
		nonBubbleCircleCache->addType<components::BubbleUnit>(components::NOTINTERESTED);
	}

	bool isPlacedRecursive(middle::GameState* gameState, middle::Id& id) {
		auto& shape = middle::getShape(gameState, id.index);
		auto placement = middle::getComponent<components::PlacementComponent>(shape);
		if (placement) {
			return true;
		}
		auto loop = middle::getComponent<components::LoopSociety>(shape);
		if (loop->parentLoopId.index != middle::UNASSIGNED) {
			return isPlacedRecursive(gameState, loop->parentLoopId);
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

		bool uiIntersected = false;
		auto rectangleIt = rectangleCache->begin<components::Rectangle>();
		auto intersectableIt = rectangleCache->begin<components::MouseIntersectable>();
		for (int i = 0; i < rectangleCache->getSize(); ++i) {
			auto& shape = middle::getShape(gameState, rectangleCache->relevantIdVector[i].index);
			auto rectangle = *rectangleIt;
			auto intersectable = *intersectableIt;
			Vector3 position = middle::getShapePosition(gameState, shape.id.index);
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
			Vector3 position = middle::getShapePosition(gameState, shape.id.index);
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



		auto unitIt = unitCache->begin<components::BubbleUnit>();
		auto unitIntersectableIt = unitCache->begin<components::MouseIntersectable>();
		auto unitCircleIt = unitCache->begin<components::Circle>();
		auto unitPositionIt = unitCache->begin<components::Position>();
		for (int i = 0; i < unitCache->getSize(); ++i) {
			auto& shape = middle::getShape(gameState, unitCache->relevantIdVector[i].index);
			auto unit = *unitIt;
			auto intersectable = *unitIntersectableIt;
			auto circle = *unitCircleIt;
			auto position = *unitPositionIt;
			intersectable->intersecting = false;
			intersectable->intersectingTop = false;

			if (uiIntersected) {
				continue;
			}

			Vector3 pos = { position->posX, position->posY, position->posZ };
			Vector3 intersectPos;
			bool intersecting = middle::RayCastLineSphere(pos, circle->radius, gameState->activeCamera.position,
				gameState->activeCamera.position + gameState->input.mouseDir, intersectPos);

			intersectable->intersecting = intersecting;
			intersectable->intersectingTop = intersecting;
		}


		auto bubbleIntersectableIt = bubbleCache->begin<components::MouseIntersectable>();
		auto bubblePositionIt = bubbleCache->begin<components::Position>();
		auto bubbleCircleIt = bubbleCache->begin<components::Circle>();
		for (int i = 0; i < bubbleCache->getSize(); ++i) {
			auto& shape = middle::getShape(gameState, bubbleCache->relevantIdVector[i].index);
			auto intersectable = *bubbleIntersectableIt;
			auto position = *bubblePositionIt;
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

			Vector3 pos = { position->posX, position->posY, position->posZ };
			Vector3 mousePos = middle::RayCastLinePlane(pos, { 0,1,0 }, gameState->activeCamera.position, gameState->input.mouseDir);

			bool intersecting = Vector3DistanceSqr(pos, mousePos) < circle->radius * circle->radius;
			intersectable->intersecting = intersecting;

			if (!alreadyIntersecting) {
				intersectable->intersectingTop = intersecting;
			}
		}


	}
};

static middle::SystemRegistrar<BubbleIntersectSystem> reg("BubbleIntersectSystem");
