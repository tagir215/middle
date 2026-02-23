#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "middle_shape_utils.h"
#include "BubbleComponent.h"
#include "MouseGrabbable.h"
#include "Position.h"
#include "PhysicsData.h"
#include "LoopSociety.h"
#include "MouseIntersectable.h"
#include "BubbleUnit.h"
#include "FractionalComponent.h"
#include "InventoryItem.h"
#include "MouseSelectable.h"
#include "DeleteComponent.h"
#include "IdRef.h"

class BubbleManipulationSystem : public middle::MiddleGameplaySystem {

	bool isIntersecting(middle::GameState* gameState, middle::Shape& shape) {
		auto fraction = middle::getComponent<components::FractionalComponent>(shape);
		auto intersectable = middle::getComponent<components::MouseIntersectable>(shape);

		if (fraction) {
			auto loop = middle::getComponent<components::LoopSociety>(shape);
			for (middle::Id id : loop->loopMemberIds) {
				middle::Shape& shape = middle::getShape(gameState, id.index);
				if (isIntersecting(gameState, shape)) {
					return true;
				}
			}
			return false;
		}

		return intersectable->intersectingTop;
	}

	void update(middle::GameState* gameState) override {

		// mouse movement
		middle::loopInstances(gameState, [gameState, this](int i, middle::Shape& shape) {

			auto grabbable = middle::getComponent<components::MouseGrabbable>(shape);
			if (!grabbable) {
				return true;
			}

			auto bubble = middle::getComponent<components::BubbleComponent>(shape);
			auto unit = middle::getComponent<components::BubbleUnit>(shape);
			auto inventoryItem = middle::getComponent<components::InventoryItem>(shape);

			if (inventoryItem) {
				return true;
			}

			if (!bubble && !unit) {
				return true;
			}

			if (unit) {
				auto loop = middle::getComponent<components::LoopSociety>(shape);
				if (loop->parentLoopId.index != middle::UNASSIGNED) {
					auto& parentShape = middle::getShape(gameState, loop->parentLoopId.index);
					auto parentFraction = middle::getComponent<components::FractionalComponent>(parentShape);
					if (parentFraction) {
						return true;
					}
				}
			}

			bool intersecting = isIntersecting(gameState, shape);

			auto fraction = middle::getComponent<components::FractionalComponent>(shape);

			if (intersecting && gameState->bubbleAlgebraState.grabbedId.index == middle::UNASSIGNED && gameState->input.mouseHeld) {
				middle::Id& parentId = middle::getParent(gameState, shape.id);
				if (parentId.index != middle::UNASSIGNED) {
					// copy as grabbed
					middle::Id copyId = middle::deepCopyShape(gameState, shape.id.index, middle::UNASSIGNED);
					auto& copyShape = middle::getShape(gameState, copyId.index);
					auto copyGrabbable = middle::getComponent<components::MouseGrabbable>(copyShape);
					copyGrabbable->grabbing = true;
					gameState->bubbleAlgebraState.grabbedId = copyId;
					// set og as reference
					auto ref = middle::addComponent<components::IdRef>(copyShape);
					ref->idRef = shape.id;
					assert(ref->idRef.index != middle::UNASSIGNED);
				}
			}

			if (gameState->bubbleAlgebraState.grabbedId.index != middle::UNASSIGNED && grabbable->grabbing && !gameState->input.mouseHeld) {
				// set grabbable for deletion
				grabbable->grabbing = false;
				gameState->bubbleAlgebraState.grabbedId = middle::Id();
				auto deleteComp = middle::addComponent<components::DeleteComponent>(shape);
				deleteComp->framesUntilDelete = 1;
			}

			// bubble moving
			if ((bubble || fraction || unit) && grabbable->grabbing) {
				Vector3 pos;
				auto posComponent = middle::getComponent<components::Position>(shape);
				if (posComponent) {
					pos = { posComponent->posX, posComponent->posY, posComponent->posZ };
				}

				Vector3 cameraPos = gameState->editorState.camera.position;
				float objYDistance = std::abs(pos.y - cameraPos.y);
				float yDistance = std::abs(cameraPos.y);
				if (yDistance == 0)
					yDistance = 0.001f;
				Vector3 xzVel = Vector3Scale(gameState->input.mouseXZ_PlaneVelocity, objYDistance / yDistance);
				moveShape(gameState, i, Vector3Scale(xzVel, gameState->frameTime));
			}

			return true;
			});

	}

};

static middle::SystemRegistrar<BubbleManipulationSystem> reg("BubbleManipulationSystem");
