#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "Rectangle.h"
#include "middle_shape_utils.h"
#include "MouseIntersectable.h"
#include "PlacementComponent.h"
#include "LoopSociety.h"

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
		middle::loopInstances(gameState, [gameState, this](int i, middle::Shape& shape) {
			auto rectangle = middle::getComponent<components::Rectangle>(shape);
			auto intersectable = middle::getComponent<components::MouseIntersectable>(shape);
			if (!rectangle || !intersectable) {
				return;
			}

			Vector3 mouseXZ = gameState->input.mouseXZ_PlanePos;
			Vector3 position = middle::getShapePosition(gameState, shape.id.index);

			float axisX = rectangle->width * 0.5f;
			float axisZ = rectangle->height* 0.5f;
			intersectable->intersecting = 
				mouseXZ.x > position.x - axisX && mouseXZ.x < position.x + axisX &&
				mouseXZ.z > position.z - axisZ && mouseXZ.z < position.z + axisZ;

			intersectable->intersectingTop = false;

			if (isPlacedRecursive(gameState, shape.id)) {
				intersectable->intersecting = false;
				return;
			}

			// check that not intersecting children as well
			if (intersectable->intersecting) {
				intersectable->intersectingTop = true;
				std::vector<middle::Id>children;
				middle::getChildren(gameState, shape.id, children);
				for (middle::Id childId : children) {
					auto& child = middle::getShape(gameState, childId.index);
					auto intersectableChild = middle::getComponent<components::MouseIntersectable>(child);
					if (intersectableChild->intersecting) {
						intersectable->intersectingTop = false;
						break;
					}
				}
			}
			});
	}
};

static middle::SystemRegistrar<RectangleIntersectionSystem> reg("RectangleIntersectionSystem");
