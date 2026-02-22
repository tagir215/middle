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

class BubbleInventorySystem : public middle::MiddleGameplaySystem {
	void update(middle::GameState* gameState) override {

		middle::loopInstances(gameState, [gameState](int i, middle::Shape& shape) {

			// copy from inventory
			auto inventory = middle::getComponent<components::Inventory>(shape);
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
					}
				}
			}
			return true;
			});


	}

};

static middle::SystemRegistrar<BubbleInventorySystem> reg("BubbleInventorySystem");
