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
#include "component_utils.h"

class BubbleInventorySystem : public middle::MiddleGameplaySystem {
public:

	components::CompCache* inventoryCache;
	components::CompCache* grabbableCache;

	void init(middle::GameState* gameState) {
		inventoryCache = middle::newCompCache(gameState);
		inventoryCache->addType<components::Inventory>();
		inventoryCache->addType<components::LoopSociety>();

		grabbableCache = middle::newCompCache(gameState);
		grabbableCache->addType<components::InventoryItem>();
		grabbableCache->addType<components::MouseGrabbable>();
	}
	void update(middle::GameState* gameState) override {

		auto inventoryIt = inventoryCache->begin<components::Inventory>();
		auto loopIt = inventoryCache->begin<components::LoopSociety>();
		for (int i = 0; i < inventoryCache->getSize(); ++i) {
			auto inventory = *inventoryIt;
			auto loop = *loopIt;

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
					auto ref = middle::attachComponent<components::IdRef>(gameState, copyShape.id);
					ref->idRef = copyId;
					middle::queueComponentDeletion<components::MouseIntersectable>(gameState, copyShape.id);
				}
			}
		}

		if (gameState->bubbleAlgebraState.grabbedId.index != middle::UNASSIGNED && !gameState->input.mouseHeld) {
			auto grabbableIt = grabbableCache->begin<components::MouseGrabbable>();
			auto inventoryItemIt = grabbableCache->begin<components::InventoryItem>();
			for (int i = 0; i < grabbableCache->getSize(); ++i) {
				auto grabbable = *grabbableIt;
				auto inventoryItem = *inventoryItemIt;
				auto& shape = middle::getShape(gameState, grabbableCache->relevantIdVector[i].index);
				if (grabbable->grabbing) {
					auto delComp = middle::attachComponent<components::DeleteComponent>(gameState, shape.id);
					delComp->framesUntilDelete = 0;
					gameState->bubbleAlgebraState.grabbedId = middle::Id();
				}

				// item moving
				if (grabbable->grabbing) {
					moveShape(gameState, i, gameState->input.mouseXZ_PlanePos - middle::getShapePosition(gameState, shape.id.index));
				}
			}
		}
	}


};

static middle::SystemRegistrar<BubbleInventorySystem> reg("BubbleInventorySystem");
