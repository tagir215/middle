#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "middle_shape_utils.h"
#include "BubbleComponent.h"
#include "MouseGrabbable.h"
#include "GlobalTransform.h"
#include "PhysicsData.h"
#include "LoopSociety.h"
#include "MouseIntersectable.h"
#include "BubbleUnit.h"
#include "InventoryItem.h"
#include "MouseSelectable.h"
#include "DeleteComponent.h"
#include "IdRef.h"
#include "bubble_actions.h"
#include "component_utils.h"
#include "bubble_utils.h"
#include "BubbleEqualsComponent.h"
#include "BubbleVariable.h"
#include "BubbleEqualsVariable.h"
#include "ExponentComponent.h"

class BubbleManipulationSystem : public middle::MiddleGameplaySystem {

public:
	components::CompCache* bubbleCache;
	components::CompCache* unitCache;


	void init(middle::GameState* gameState) {
		bubbleCache = middle::newCompCache(gameState, systemName);
		bubbleCache->addType<components::MouseGrabbable>();
		bubbleCache->addType<components::BubbleComponent>();
		bubbleCache->addType<components::InventoryItem>(components::NOTINTERESTED);
	}

	void move(middle::GameState* gameState, middle::Shape& shape) {
		Vector3 pos;
		auto transform = middle::getComponent<components::GlobalTransform>(shape);
		if (transform) {
			pos = transform->pos;
		}

		Vector3 cameraPos = gameState->editorState.camera.position;
		float objYDistance = std::abs(pos.y - cameraPos.y);
		float yDistance = std::abs(cameraPos.y);
		if (yDistance == 0)
			yDistance = 0.001f;
		Vector3 xzVel = Vector3Scale(gameState->input.mouseXZ_PlaneVelocity, objYDistance / yDistance);
		moveShape(gameState, shape.id.index, Vector3Scale(xzVel, gameState->frameTime));
	}

	// attach bubble equals variable component to indicate this grabbed bubble equals to a variable
	void handleEqualsToVariableCase(middle::GameState* gameState, middle::Id parentId, middle::Id grabbedCopyId) {

		// check if the parent is equals comp
		auto& parentShape = middle::getShape(gameState, parentId.index);
		if (!middle::getComponent<components::BubbleEqualsComponent>(parentShape)) {
			return;
		}

		std::vector<middle::Id> siblings;
		middle::getChildren(gameState, parentShape.id, siblings);

		auto& grabbedShape = middle::getShape(gameState, grabbedCopyId.index);
		auto ref = middle::getComponent<components::IdRef>(grabbedShape);
		middle::Id grabbedRefId = ref->idRef;

		// GRABBED VARIABLE CASE
		auto grabbedVar = middle::getComponent<components::BubbleVariable>(grabbedShape);
		if (grabbedVar) {
			auto bubbleEqualsVar = middle::attachComponent<components::BubbleEqualsVariable>(gameState, grabbedCopyId);
			for (middle::Id& siblingId : siblings) {
				if (siblingId != grabbedRefId) {
					bubbleEqualsVar->matchingIdRef = siblingId;
				}
			}
			bubbleEqualsVar->wantsToReplaceBubble = true;
			return;
		}

		// GRABBED BUBBLE CASE
		for (middle::Id& id : siblings) {
			if (id == grabbedRefId) {
				continue;
			}
			// other children should be variable, for this to work
			auto& childOfOther = middle::getShape(gameState, id.index);
			auto expComp = middle::getComponent<components::ExponentComponent>(childOfOther);
			// can't be in exponent for this to work
			if (expComp) {
				return;
			}

			auto varComp = middle::getComponent<components::BubbleVariable>(childOfOther);
			if (!varComp) {
				return;
			}

			auto bubbleEqualsVar = middle::attachComponent<components::BubbleEqualsVariable>(gameState, grabbedCopyId);
			bubbleEqualsVar->variableLabel = varComp->label;
			bubbleEqualsVar->wantsToReplaceVariable = true;
		}
	}

	void attachComponents(middle::GameState* gameState, middle::Shape& shape, components::MouseGrabbable* grabbable) {

		bool intersecting = bubble::isIntersecting(gameState, shape);
		if (gameState->input.mouseClicked && intersecting && gameState->bubbleAlgebraState.grabbedId.index == middle::UNASSIGNED) {

			// copy as grabbed
			middle::Id copyId = middle::deepCopyShape(gameState, shape.id.index);
			auto& copyShape = middle::getShape(gameState, copyId.index);
			auto copyGrabbable = middle::getComponent<components::MouseGrabbable>(copyShape);
			copyGrabbable->grabbing = true;
			gameState->bubbleAlgebraState.grabbedId = copyId;
			// set og as reference
			auto ref = middle::attachComponent<components::IdRef>(gameState, copyShape.id);
			ref->idRef = shape.id;
			assert(ref->idRef.index != middle::UNASSIGNED);

			middle::Id& parentId = middle::getParent(gameState, shape.id);
			if (parentId.index != middle::UNASSIGNED) {
				handleEqualsToVariableCase(gameState, parentId, copyId);
			}
		}


		if (gameState->bubbleAlgebraState.grabbedId.index != middle::UNASSIGNED && grabbable->grabbing && !gameState->input.mouseHeld) {
			// set grabbable for deletion
			grabbable->grabbing = false;
			gameState->bubbleAlgebraState.grabbedId = middle::Id();
			auto deleteComp = middle::attachComponent<components::DeleteComponent>(gameState, shape.id);
			deleteComp->framesUntilDelete = 0;
		}
	}

	void update(middle::GameState* gameState) override {

		// todo figure input stuff at some point
		if (gameState->gameInput.one ||
			gameState->gameInput.two ||
			gameState->gameInput.three ||
			gameState->gameInput.four ||
			gameState->gameInput.five ||
			gameState->gameInput.six ||
			gameState->gameInput.seven ||
			gameState->gameInput.eight ||
			gameState->gameInput.nine ||
			gameState->gameInput.zero
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
