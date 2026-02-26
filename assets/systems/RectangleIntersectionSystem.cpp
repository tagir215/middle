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

	void update(middle::GameState* gameState) override {
		bool oneIntersect = false;
		middle::loopInstances(gameState, [gameState, this, &oneIntersect](int i, middle::Shape& shape) {
			auto rectangle = middle::getComponent<components::Rectangle>(shape);
			auto circle = middle::getComponent<components::Circle>(shape);
			auto intersectable = middle::getComponent<components::MouseIntersectable>(shape);
			if ((!rectangle && !circle) || !intersectable) {
				return true;
			}

			Vector3 position = middle::getShapePosition(gameState, shape.id.index);
			Vector3 scale = middle::getTotalScale(gameState, shape.id);

			Vector3 uiPlaneIntersectPoint = middle::RayCastLinePlane(position, { 0,-1,0 }, 
				gameState->input.mouseNearPlanePos, gameState->input.mouseDir);
			Vector3 mouseXZ = uiPlaneIntersectPoint;


			if (rectangle) {
				float axisX = rectangle->width * 0.5f * scale.x;
				float axisZ = rectangle->height * 0.5f * scale.z;
				intersectable->intersecting =
					mouseXZ.x > position.x - axisX && mouseXZ.x < position.x + axisX &&
					mouseXZ.z > position.z - axisZ && mouseXZ.z < position.z + axisZ;
			}
			if (circle) {
				intersectable->intersecting = Vector3DistanceSqr(mouseXZ, position) < circle->radius * circle->radius;
			}

			if (intersectable->intersecting) {
				oneIntersect = true;
			}

			intersectable->intersectingTop = false;

			if (isPlacedRecursive(gameState, shape.id)) {
				intersectable->intersecting = false;
				return true;
			}

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
			return true;
			});

		gameState->bubbleAlgebraState.intersectingUI = oneIntersect;
	}
};

static middle::SystemRegistrar<RectangleIntersectionSystem> reg("RectangleIntersectionSystem");
