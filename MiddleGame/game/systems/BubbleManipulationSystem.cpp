#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "middle_shape_utils.h"
#include "MidComp/BubbleComponent.h"
#include "MidComp/MouseGrabbable.h"
#include "MidComp/GlobalTransform.h"
#include "MidComp/PhysicsData.h"
#include "MidComp/LoopSociety.h"
#include "MidComp/MouseIntersectable.h"
#include "MidComp/BubbleUnit.h"
#include "MidComp/InventoryItem.h"
#include "MidComp/MouseSelectable.h"
#include "MidComp/DeleteComponent.h"
#include "MidComp/IdRef.h"
#include "bubble_actions.h"
#include "component_utils.h"
#include "bubble_utils.h"
#include "MidComp/BubbleEqualsComponent.h"
#include "MidComp/BubbleVariable.h"
#include "MidComp/BubbleEqualsVariable.h"
#include "MidComp/BubbleManipulatable.h"
#include "MidComp/NonPhysicalBubbleTag.h"

class BubbleManipulationSystem : public middle::MiddleGameplaySystem {

public:
	components::CompCache* bubbleCache;
	components::CompCache* unitCache;


	void init(middle::GameState* gameState) {
		bubbleCache = middle::newCompCache(gameState, systemName);
		bubbleCache->addType<components::MouseGrabbable>();
		bubbleCache->addType<components::BubbleComponent>();
		bubbleCache->addType<components::BubbleManipulatable>();
		bubbleCache->addType<components::InventoryItem>(components::NOTINTERESTED);
	}

	void move(middle::GameState* gameState, middle::Shape& shape) {
		midMath::Vector3 pos;
		auto transform = middle::getComponent<components::GlobalTransform>(shape);
		if (transform) {
			pos = transform->pos;
		}

		midMath::Vector3 cameraPos = gameState->editorState.camera.position;
		float objYDistance = std::abs(pos.y - cameraPos.y);
		float yDistance = std::abs(cameraPos.y);
		if (yDistance == 0)
			yDistance = 0.001f;
		midMath::Vector3 xzVel = Vector3Scale(gameState->mouseState.mouseXZ_PlaneVelocity, objYDistance / yDistance);
		moveShape(gameState, shape.id.index, Vector3Scale(xzVel, gameState->middleInputState.frameTime));
	}

	void attachNonPhysical(middle::GameState* gameState, middle::Id id) {
		middle::attachComponent<components::NonPhysicalBubbleTag>(gameState, id);
		std::vector<middle::Id>children;
		middle::getChildren(gameState, id, children);
		for (middle::Id childId : children) {
			attachNonPhysical(gameState, childId);
		}
	}

	void attachComponents(middle::GameState* gameState, middle::Shape& shape, components::MouseGrabbable* grabbable) {

		bool intersecting = bubble::isIntersecting(gameState, shape);
		if (gameState->middleInputState.editorInput.mouseClicked && intersecting && gameState->bubbleAlgebraState.grabbedId.index == middle::UNASSIGNED) {

			// copy as grabbed
			middle::Id oldParentId = middle::getParent(gameState, shape.id);
			middle::Id copyId = middle::deepCopyShape(gameState, shape.id.index);
			middle::updateLocalCoordinateToProjectedGlobalCoordinate(gameState, copyId, oldParentId);
			auto& copyShape = middle::getShape(gameState, copyId.index);
			auto copyGrabbable = middle::getComponent<components::MouseGrabbable>(copyShape);
			copyGrabbable->grabbing = true;
			gameState->bubbleAlgebraState.grabbedId = copyId;
			attachNonPhysical(gameState, copyId);
			// set og as reference
			auto ref = middle::attachComponent<components::IdRef>(gameState, copyShape.id);
			ref->idRef = shape.id;
			assert(ref->idRef.index != middle::UNASSIGNED);
		}


		if (gameState->bubbleAlgebraState.grabbedId.index != middle::UNASSIGNED && grabbable->grabbing && !gameState->middleInputState.editorInput.mouseHeld) {
			// set grabbable for deletion
			grabbable->grabbing = false;
			gameState->bubbleAlgebraState.grabbedId = middle::Id();
			auto deleteComp = middle::attachComponent<components::DeleteComponent>(gameState, shape.id);
			deleteComp->framesUntilDelete = 1;
		}
	}

	void update(middle::GameState* gameState) override {

		// todo figure input stuff at some point
		if (gameState->middleInputState.gameInput.one ||
			gameState->middleInputState.gameInput.two ||
			gameState->middleInputState.gameInput.three ||
			gameState->middleInputState.gameInput.four ||
			gameState->middleInputState.gameInput.five ||
			gameState->middleInputState.gameInput.six ||
			gameState->middleInputState.gameInput.seven ||
			gameState->middleInputState.gameInput.eight ||
			gameState->middleInputState.gameInput.nine ||
			gameState->middleInputState.gameInput.zero
			) {
			return;
		}

		auto bubbleIt = bubbleCache->begin<components::BubbleComponent>();
		auto bubbleGrabbableIt = bubbleCache->begin<components::MouseGrabbable>();
		for (int i = 0; i < bubbleCache->getSize(); ++i) {
			auto& shape = middle::getShape(gameState, bubbleCache->relevantIdVector[i].index);
			auto bubble = *bubbleIt;
			auto grabbable = *bubbleGrabbableIt;
			if (grabbable->grabbing) {
				move(gameState, shape);
			}
			attachComponents(gameState, shape, grabbable);
		}

	}

};

static middle::SystemRegistrar<BubbleManipulationSystem> reg("BubbleManipulationSystem");
