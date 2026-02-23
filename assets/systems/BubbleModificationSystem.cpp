#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "middle_shape_utils.h"
#include "DeleteComponent.h"
#include "MouseIntersectable.h"
#include "IdRef.h"
#include "BubbleMultiplyComponent.h"
#include "bubble_actions.h"
#include "InventoryItem.h"

class BubbleModificationSystem : public middle::MiddleGameplaySystem {

	bool isMultiplicationConnection(middle::GameState* gameState, middle::Shape& parentShape) {
		auto mulComp = middle::getComponent<components::BubbleMultiplyComponent>(parentShape);
		if (mulComp) {
			return true;
		}
		return false;
	}

	void combine(middle::GameState* gameState, middle::Shape& refParent, middle::Shape& refShape, middle::Shape& intersectedShape) {
		// is multiplication connection
		if (isMultiplicationConnection(gameState, refParent)) {
			auto multiply = bubbleActions::ExecuteMultiplication(refParent.id, refShape.id, intersectedShape.id);
			multiply.execute(gameState);
			multiply.finalize(gameState);
			return;
		}
		// else is addition connection
		else {
			auto add = bubbleActions::ExecuteAddition(refShape.id, intersectedShape.id);
			add.execute(gameState);
			add.finalize(gameState);
			return;
		}
	}

	void inventoryAction(middle::GameState* gameState, int actionType, middle::Shape& intersectedShape) {
		// pop as long as not multiplication
		if (actionType == bubbleInventoryitemType::POP) {
			middle::Id& parentId = middle::getParent(gameState, intersectedShape.id);
			auto& parentShape = middle::getShape(gameState, parentId.index);
			if (!isMultiplicationConnection(gameState, parentShape)) {
				auto pop = bubbleActions::Pop(intersectedShape.id);
				pop.execute(gameState);
			}
		}
		else if (actionType == bubbleInventoryitemType::TIMES_ONE) {
			Vector3 targetPos = middle::getShapePosition(gameState, intersectedShape.id.index);
			middle::Shape& newBubble = bubbleActions::newBubble(gameState, targetPos);
			middle::Shape& newUnit = bubbleActions::newUnit(gameState, targetPos);
		}
	}

	void update(middle::GameState* gameState) override {

		// search for shapes for deletion
		middle::Id shapeIdForDeletion;
		middle::loopInstances(gameState, [gameState, &shapeIdForDeletion](int i, middle::Shape& shape) {
			auto deletion = middle::getComponent<components::DeleteComponent>(shape);
			auto idRef = middle::getComponent<components::IdRef>(shape);
			if (!deletion || !idRef) {
				return true;
			}
			shapeIdForDeletion = shape.id;
			return false;
			});
		if (shapeIdForDeletion.index == middle::UNASSIGNED)
			return;

		auto& shapeForDeletion = middle::getShape(gameState, shapeIdForDeletion.index);
		auto ref = middle::getComponent<components::IdRef>(shapeForDeletion);
		auto& refShape = middle::getShape(gameState, ref->idRef.index);
		middle::Id refParentId = middle::getParent(gameState, refShape.id);
		assert(refShape.id.index != middle::UNASSIGNED);

		auto inventoryItem = middle::getComponent<components::InventoryItem>(refShape);

		// search for intersecting shapes, and see if operation can be done
		middle::loopInstances(gameState, [gameState, this, &refShape, &refParentId, &shapeForDeletion, inventoryItem](int i, middle::Shape& shape) {
			auto intersectable = middle::getComponent<components::MouseIntersectable>(shape);
			if (!intersectable || !intersectable->intersectingTop) {
				return true;
			}
			// skip shapefordeletion (the copy being dragged) and ref shape (shape its copy is pointing to)
			if (shapeForDeletion.id == shape.id || shape.id == refShape.id) {
				return true;
			}
			middle::Id parentId = middle::getParent(gameState, shape.id);
			if (parentId == refParentId) {
				auto& refParent = middle::getShape(gameState, refParentId.index);
				combine(gameState, refParent, refShape, shape);
				return false;
			}

			if (inventoryItem) {
				inventoryAction(gameState, inventoryItem->itemType, shape);
				return false;
			}

			return true;
			});



		// TOP BUBBLE STUFF
		// is inventory item intersecting with top Bubble?
		inventoryItem = middle::getComponent<components::InventoryItem>(refShape);
		if (inventoryItem) {
			middle::Id topBubble = bubbleActions::topLevelBubble(gameState);
			auto& topBubbleShape = middle::getShape(gameState, topBubble.index);
			auto intersectable = middle::getComponent<components::MouseIntersectable>(topBubbleShape);
			if (intersectable->intersecting) {
				if (inventoryItem->itemType == bubbleInventoryitemType::ADD) {
					middle::deleteComponent<components::DeleteComponent>(refShape);
					auto newAddition = bubbleActions::CreateAdditionReplacementShape(topBubbleShape.id, refShape.id);
					newAddition.execute(gameState);
					middle::addComponent<components::DeleteComponent>(refShape);
					middle::addComponent<components::DeleteComponent>(topBubbleShape);
				}
				else if (inventoryItem->itemType == bubbleInventoryitemType::MULTIPLICATION) {
					middle::deleteComponent<components::DeleteComponent>(refShape);
					auto newMultiplication = bubbleActions::CreateMulitiplicationReplacementShape(topBubbleShape.id, refShape.id);
					newMultiplication.execute(gameState);
					middle::addComponent<components::DeleteComponent>(refShape);
					middle::addComponent<components::DeleteComponent>(topBubbleShape);
				}
			}
		}
	}
};

static middle::SystemRegistrar<BubbleModificationSystem> reg("BubbleModificationSystem");
