#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "middle_shape_utils.h"
#include "Inventory.h"
#include "BubbleComponent.h"
#include "MouseIntersectable.h"
#include "MouseGrabbable.h"
#include "editor_actions.h"
#include "LoopSociety.h"
#include "InventoryItem.h"
#include "Position.h"
#include "DeleteComponent.h"
#include "IdRef.h"

class BubbleInventorySystem : public middle::MiddleGameplaySystem {
public:
	void init(middle::GameState* gameState) {

	}
	void update(middle::GameState* gameState) override {

		middle::loopInstances(gameState, [gameState](int i, middle::Shape& shape) {

			// copy from inventory
			auto inventory = middle::getComponent<components::Inventory>(shape);
			auto inventoryItem = middle::getComponent<components::InventoryItem>(shape);
			if (!inventory && !inventoryItem) {
				return true;
			}

			if (inventory) {
				auto loop = middle::getComponent<components::LoopSociety>(shape);
				std::vector < middle::Id>children = loop->loopMemberIds;

				for (middle::Id childId : children) {
					auto& child = middle::getShape(gameState, childId.index);
					auto intersectable = middle::getComponent<components::MouseIntersectable>(child);
					if (intersectable->intersectingTop && gameState->input.mouseClicked) {
						middle::Id copyId = middle::deepCopyShape(gameState, childId.index, middle::UNASSIGNED);
						auto& copyShape = middle::getShape(gameState, copyId.index);
						auto grabbable = middle::getComponent<components::MouseGrabbable>(copyShape);
						grabbable->grabbing = true;
						auto removeLoop = middle::EditorActionRemoveFromLoop(copyId.index);
						removeLoop.execute(gameState);
						gameState->bubbleAlgebraState.grabbedId = copyId;
						auto ref = middle::addComponent<components::IdRef>(copyShape);
						ref->idRef = copyId;
						middle::deleteComponent<components::MouseIntersectable>(copyShape);
					}
				}
			}

			auto grabbable = middle::getComponent < components::MouseGrabbable>(shape);

			if (gameState->bubbleAlgebraState.grabbedId.index != middle::UNASSIGNED && grabbable->grabbing && !gameState->input.mouseHeld) {
				grabbable->grabbing = false;
				auto inventoryItem = middle::getComponent<components::InventoryItem>(shape);
				if (inventoryItem) {
					auto delComp = middle::addComponent<components::DeleteComponent>(shape);
					delComp->framesUntilDelete = 0;
				}
				gameState->bubbleAlgebraState.grabbedId = middle::Id();
			}

			// item moving
			if (grabbable->grabbing) {
				moveShape(gameState, i, gameState->input.mouseXZ_PlanePos - middle::getShapePosition(gameState, shape.id.index));
			}
			return true;
			});


	}

};

static middle::SystemRegistrar<BubbleInventorySystem> reg("BubbleInventorySystem");
