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
#include "PlacementComponent.h"
#include "CodeBlock.h"
#include "CodeFunction.h"
#include "UiComponent.h"
#include "Button.h"
#include "ActiveCheckBoxTag.h"
#include "MouseClickComponent.h"
#include "InventorySlot.h"
#include "SnapRef.h"
#include "Layer.h"


class BubbleInventorySystem : public middle::MiddleGameplaySystem {
public:

	components::CompCache* inventoryCache;
	components::CompCache* inventoryItemCache;
	components::CompCache* grabbableCache;
	components::CompCache* uiComponentlessBubbleInventoryItemCache;
	components::CompCache* snapReflessBubbleInventoryItemCache;
	components::CompCache* uiButtonsCache;
	components::CompCache* activeCheckBoxesCache;
	components::CompCache* inventorySlotCache;

	void init(middle::GameState* gameState) {
		inventoryCache = middle::newCompCache(gameState);
		inventoryCache->addType<components::Inventory>();
		inventoryCache->addType<components::LoopSociety>();
		inventoryItemCache = middle::newCompCache(gameState);
		inventoryItemCache->addType<components::InventoryItem>();
		grabbableCache = middle::newCompCache(gameState);
		grabbableCache->addType<components::InventoryItem>();
		grabbableCache->addType<components::MouseGrabbable>();
		uiComponentlessBubbleInventoryItemCache = middle::newCompCache(gameState);
		uiComponentlessBubbleInventoryItemCache->addType<components::InventoryItem>();
		uiComponentlessBubbleInventoryItemCache->addType<components::BubbleComponent>();
		uiComponentlessBubbleInventoryItemCache->addType<components::UiComponent>(components::NOTINTERESTED);
		snapReflessBubbleInventoryItemCache = middle::newCompCache(gameState);
		snapReflessBubbleInventoryItemCache->addType<components::InventoryItem>();
		snapReflessBubbleInventoryItemCache->addType<components::BubbleComponent>();
		snapReflessBubbleInventoryItemCache->addType<components::SnapRef>(components::NOTINTERESTED);
		snapReflessBubbleInventoryItemCache->addType<components::PlacementComponent>(components::NOTINTERESTED);
		uiButtonsCache = middle::newCompCache(gameState);
		uiButtonsCache->addType<components::Button>();
		uiButtonsCache->addType<components::UiComponent>();
		uiButtonsCache->addType<components::MouseClickComponent>();
		activeCheckBoxesCache = middle::newCompCache(gameState);
		activeCheckBoxesCache->addType<components::ActiveCheckBoxTag>();
		inventorySlotCache = middle::newCompCache(gameState);
		inventorySlotCache->addType<components::InventorySlot>();
	}

	void deactivateCheckboxes(middle::GameState* gameState) {
		auto activeIt = activeCheckBoxesCache->begin<components::ActiveCheckBoxTag>();
		for (int i = 0; i < activeCheckBoxesCache->getSize(); ++i) {
			middle::queueComponentDeletion<components::ActiveCheckBoxTag>(gameState, activeCheckBoxesCache->relevantIdVector[i]);
		}
	}

	void changeTermAdditionTypes(middle::GameState* gameState, int newType) {
		auto termIt = inventoryItemCache->begin<components::InventoryItem>();
		for (int i = 0; i < inventoryItemCache->getSize(); ++i) {
			auto* term = *termIt;
			if (term->itemType == bubbleInventoryItemType::NEW_ADDITION_TERM) {
				term->itemType = newType;
			}
			if (term->itemType == bubbleInventoryItemType::NEW_MULTIPLICATION_TERM) {
				term->itemType = newType;
			}
			if (term->itemType == bubbleInventoryItemType::INSERT_X_OVER_X) {
				term->itemType = newType;
			}
			if (term->itemType == bubbleInventoryItemType::INSERT_X_MINUS_X) {
				term->itemType = newType;
			}
		}
	}


	void update(middle::GameState* gameState) override {

		auto inventoryIt = inventoryCache->begin<components::Inventory>();
		auto loopIt = inventoryCache->begin<components::LoopSociety>();
		for (int i = 0; i < inventoryCache->getSize(); ++i) {
			auto inventory = *inventoryIt;
			auto loop = *loopIt;

			std::vector < middle::Id>children = loop->loopMemberIds;
			for (int i = 0; i < children.size(); ++i) {
				middle::Id childId = children[i];
				auto& child = middle::getShape(gameState, childId.index);
				auto intersectable = middle::getComponent<components::MouseIntersectable>(child);
				auto grabbable = middle::getComponent<components::MouseGrabbable>(child);
				if (!grabbable) {
					continue;
				}
				if (intersectable->intersectingTop && gameState->input.mouseClicked) {
					middle::Id copyId = middle::deepCopyShape(gameState, childId.index, middle::UNASSIGNED);
					auto& copyShape = middle::getShape(gameState, copyId.index);
					auto grabbable = middle::getComponent<components::MouseGrabbable>(copyShape);
					grabbable->grabbing = true;
					auto removeLoop = middle::EditorActionRemoveFromLoop(copyId.index);
					removeLoop.execute(gameState);
					gameState->bubbleAlgebraState.grabbedId = copyId;
					auto ref = middle::attachComponent<components::IdRef>(gameState, copyShape.id);
					auto placement = middle::attachComponent<components::PlacementComponent>(gameState, copyShape.id);
					placement->grabbing = true;
					ref->idRef = childId;
					middle::queueComponentDeletion<components::MouseIntersectable>(gameState, copyShape.id);
					middle::queueComponentDeletion<components::SnapRef>(gameState, copyShape.id);
				}
			}
		}


		// attach ui components to bubbles that are in the inventory
		auto uiComponentlessIt = uiComponentlessBubbleInventoryItemCache->begin<components::BubbleComponent>();
		for (int i = 0; i < uiComponentlessBubbleInventoryItemCache->getSize(); ++i) {
			middle::Id id = uiComponentlessBubbleInventoryItemCache->relevantIdVector[i];
			middle::queueComponentAttachment<components::UiComponent>(gameState, id);
			std::vector<middle::Id>children;
			middle::getAllChildren(gameState, id, children);
			for (middle::Id& child : children) {
				auto& childShape = middle::getShape(gameState, child.index);
				auto comp = middle::getComponent<components::UiComponent>(childShape);
				if (!comp) {
					middle::queueComponentAttachment<components::UiComponent>(gameState, child);
				}
			}
		}

		auto bubbleItemIt = snapReflessBubbleInventoryItemCache->begin<components::InventoryItem>();
		for (int i = 0; i < snapReflessBubbleInventoryItemCache->getSize(); ++i) {
			auto invItem = *bubbleItemIt;
			middle::Id parentInventoryId = middle::getParent(gameState, snapReflessBubbleInventoryItemCache->relevantIdVector[i]);
			if (parentInventoryId.index == middle::UNASSIGNED) {
				continue;
			}
			std::vector<middle::Id>children;
			middle::getChildren(gameState, parentInventoryId, children);
			bool attachedRefs = false;
			for (int index = 0; index < children.size(); ++index) {
				// TODO THIS IS ASSUMES SLOTS MATCH THIS BUBBLE INVENTORYS MEMBERS
				middle::Id slot = inventorySlotCache->relevantIdVector[index];
				auto snapRef = middle::attachComponent<components::SnapRef>(gameState, children[index]);
				snapRef->snapTargetId = slot;
				//invItem->idRef = inventorySlotCache->relevantIdVector[i];
				attachedRefs = true;
				// set layer here, because its seems like good time
				auto& shape = middle::getShape(gameState, children[index].index);
				auto layer = middle::getComponent<components::Layer>(shape);
				layer->layer = 2;
			}
			if (attachedRefs) {
				break;
			}
		}

		auto buttonIt = uiButtonsCache->begin<components::Button>();
		for (int i = 0; i < uiButtonsCache->getSize(); ++i) {
			middle::Id buttonId = uiButtonsCache->relevantIdVector[i];
			auto& shape = middle::getShape(gameState, buttonId.index);
			auto button = *buttonIt;
			if (button->function == bubbleButton::SELECT_PLUS) {
				if (!middle::getComponent<components::ActiveCheckBoxTag>(shape)) {
					deactivateCheckboxes(gameState);
					middle::queueComponentAttachment<components::ActiveCheckBoxTag>(gameState, buttonId);
					changeTermAdditionTypes(gameState, bubbleInventoryItemType::NEW_ADDITION_TERM);
				}
			}
			if (button->function == bubbleButton::SELECT_MULTIPLY) {
				if (!middle::getComponent<components::ActiveCheckBoxTag>(shape)) {
					deactivateCheckboxes(gameState);
					middle::queueComponentAttachment<components::ActiveCheckBoxTag>(gameState, buttonId);
					changeTermAdditionTypes(gameState, bubbleInventoryItemType::NEW_MULTIPLICATION_TERM);
				}
			}
			if (button->function == bubbleButton::SELECT_INSERT_X_OVER_X) {
				if (!middle::getComponent<components::ActiveCheckBoxTag>(shape)) {
					deactivateCheckboxes(gameState);
					middle::queueComponentAttachment<components::ActiveCheckBoxTag>(gameState, buttonId);
					changeTermAdditionTypes(gameState, bubbleInventoryItemType::INSERT_X_OVER_X);
				}
			}
			if (button->function == bubbleButton::SELECT_INSERT_X_MINUS_X) {
				if (!middle::getComponent<components::ActiveCheckBoxTag>(shape)) {
					deactivateCheckboxes(gameState);
					middle::queueComponentAttachment<components::ActiveCheckBoxTag>(gameState, buttonId);
					changeTermAdditionTypes(gameState, bubbleInventoryItemType::INSERT_X_MINUS_X);
				}
			}
		}


		if (gameState->bubbleAlgebraState.grabbedId.index != middle::UNASSIGNED) {
			auto grabbableIt = grabbableCache->begin<components::MouseGrabbable>();
			auto inventoryItemIt = grabbableCache->begin<components::InventoryItem>();
			for (int i = 0; i < grabbableCache->getSize(); ++i) {
				auto grabbable = *grabbableIt;
				auto inventoryItem = *inventoryItemIt;
				auto& shape = middle::getShape(gameState, grabbableCache->relevantIdVector[i].index);

				// item moving
				if (grabbable->grabbing) {
					moveShape(gameState, shape.id.index, gameState->input.mouseXZ_PlanePos - middle::getShapePosition(gameState, shape.id.index));
				}

				if (grabbable->grabbing && !gameState->input.mouseHeld) {
					auto delComp = middle::attachComponent<components::DeleteComponent>(gameState, shape.id);
					delComp->framesUntilDelete = 0;
					gameState->bubbleAlgebraState.grabbedId = middle::Id();
				}

			}
		}
	}


};

static middle::SystemRegistrar<BubbleInventorySystem> reg("BubbleInventorySystem");
