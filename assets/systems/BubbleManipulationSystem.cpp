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
	components::CompCache* fractionCache;


	void init(middle::GameState* gameState) {
		bubbleCache = middle::newCompCache(gameState);
		bubbleCache->addType<components::MouseGrabbable>();
		bubbleCache->addType<components::BubbleComponent>();
		bubbleCache->addType<components::InventoryItem>(components::NOTINTERESTED);
		unitCache = middle::newCompCache(gameState);
		unitCache->addType<components::MouseGrabbable>();
		unitCache->addType<components::BubbleUnit>();
		unitCache->addType<components::LoopSociety>();
		fractionCache = middle::newCompCache(gameState);
		fractionCache->addType<components::MouseGrabbable>();
		fractionCache->addType<components::FractionalComponent>();
	}

	void move(middle::GameState* gameState, middle::Shape& shape) {
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
		std::vector<middle::Id> grabbedChildren;
		middle::getChildren(gameState, grabbedShape.id, grabbedChildren);
		if (grabbedChildren.size() == 1) {
			auto& childShape = middle::getShape(gameState, grabbedChildren[0].index);
			auto var = middle::getComponent<components::BubbleVariable>(childShape);
			if (var) {
				auto bubbleEqualsVar = middle::attachComponent<components::BubbleEqualsVariable>(gameState, grabbedCopyId);
				for (middle::Id& siblingId : siblings) {
					if (siblingId != grabbedRefId) {
						bubbleEqualsVar->matchingIdRef = siblingId;
					}
				}
				bubbleEqualsVar->wantsToReplaceBubble = true;
				return;
			}
		}

		// GRABBED BUBBLE CASE
		for (middle::Id& id : siblings) {
			if (id == grabbedRefId) {
				continue;
			}
			// other children should be 1 and a variable, for this to work
			std::vector<middle::Id>otherChildren;
			middle::getChildren(gameState, id, otherChildren);
			if (otherChildren.size() != 1) {
				return;
			}

			auto& childOfOther = middle::getShape(gameState, otherChildren[0].index);
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
		if (intersecting && gameState->bubbleAlgebraState.grabbedId.index == middle::UNASSIGNED && gameState->input.mouseHeld) {
			middle::Id& parentId = middle::getParent(gameState, shape.id);
			if (parentId.index != middle::UNASSIGNED) {
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

		auto unitIt = unitCache->begin<components::BubbleUnit>();
		auto unitGrabbableIt = unitCache->begin<components::MouseGrabbable>();
		for (int i = 0; i < unitCache->getSize(); ++i) {
			auto& shape = middle::getShape(gameState, unitCache->relevantIdVector[i].index);
			auto unit = *unitIt;
			auto grabbable = *unitGrabbableIt;
			middle::Id parentId = middle::getParent(gameState, shape.id);
			if (parentId.index != middle::UNASSIGNED) {
				auto& parentShape = middle::getShape(gameState, parentId.index);
				auto parentFraction = middle::getComponent<components::FractionalComponent>(parentShape);
				if (parentFraction) {
					continue;
				}
			}
			if (grabbable->grabbing) {
				move(gameState, shape);
			}
			attachComponents(gameState, shape, grabbable);

		}

		auto fractionIt = fractionCache->begin<components::FractionalComponent>();
		auto fractionGrabbableIt = fractionCache->begin<components::MouseGrabbable>();
		for (int i = 0; i < fractionCache->getSize(); ++i) {
			auto& shape = middle::getShape(gameState, fractionCache->relevantIdVector[i].index);
			auto fraction = *fractionIt;
			auto grabbable = *fractionGrabbableIt;
			attachComponents(gameState, shape, grabbable);
			if (grabbable->grabbing) {
				move(gameState, shape);
			}
		}

	}

};

static middle::SystemRegistrar<BubbleManipulationSystem> reg("BubbleManipulationSystem");
