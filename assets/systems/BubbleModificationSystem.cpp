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
public:
	components::CompCache* deletionCache;
	components::CompCache* intersectableCache;

	void init(middle::GameState* gameState) {
		deletionCache = middle::newCompCache(gameState);
		deletionCache->addType<components::DeleteComponent>();
		deletionCache->addType<components::IdRef>();

		intersectableCache = middle::newCompCache(gameState);
		intersectableCache->addType<components::MouseIntersectable>();

	}

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
			auto multiply = std::make_shared<bubbleActions::ExecuteMultiplication>(refShape.id, intersectedShape.id);
			middle::queueAction(gameState, multiply);
			gameState->bubbleAlgebraState.bubbleActions.push_back(multiply);
			return;
		}
		// else is addition connection
		else {
			auto add = std::make_shared<bubbleActions::ExecuteAddition>(refShape.id, intersectedShape.id);
			middle::queueAction(gameState, add);
			gameState->bubbleAlgebraState.bubbleActions.push_back(add);
			return;
		}
	}

	void inventoryAction(middle::GameState* gameState, int actionType, middle::Shape& intersectedShape) {
		std::shared_ptr<middle::EditorActionContainer>action;

		// pop as long as not multiplication
		if (actionType == bubbleInventoryitemType::POP) {
			middle::Id& parentId = middle::getParent(gameState, intersectedShape.id);
			if (parentId.index == middle::UNASSIGNED) {
				return;
			}
			auto& parentShape = middle::getShape(gameState, parentId.index);
			if (!isMultiplicationConnection(gameState, parentShape)) {
				action = std::make_shared<bubbleActions::Pop>(intersectedShape.id);
			}
		}
		else if (actionType == bubbleInventoryitemType::TIMES_ONE) {
			action = std::make_shared<bubbleActions::MulOne>(intersectedShape.id);
		}
		else if (actionType == bubbleInventoryitemType::COMPRESS) {
			action = std::make_shared<bubbleActions::Compress>(intersectedShape.id);
		}
		else if (actionType == bubbleInventoryitemType::BREAK_2) {
			action = std::make_shared<bubbleActions::Break>(intersectedShape.id, 2);
		}
		else if (actionType == bubbleInventoryitemType::BREAK_3) {
			action = std::make_shared<bubbleActions::Break>(intersectedShape.id, 3);
		}
		else if (actionType == bubbleInventoryitemType::BREAK_4) {
			action = std::make_shared<bubbleActions::Break>(intersectedShape.id, 4);
		}
		else if (actionType == bubbleInventoryitemType::BREAK_5) {
			action = std::make_shared<bubbleActions::Break>(intersectedShape.id, 5);
		}
		else if (actionType == bubbleInventoryitemType::BREAK_6) {
			action = std::make_shared<bubbleActions::Break>(intersectedShape.id, 6);
		}
		else if (actionType == bubbleInventoryitemType::BREAK_7) {
			action = std::make_shared<bubbleActions::Break>(intersectedShape.id, 7);
		}
		else if (actionType == bubbleInventoryitemType::BREAK_8) {
			action = std::make_shared<bubbleActions::Break>(intersectedShape.id, 8);
		}
		else if (actionType == bubbleInventoryitemType::BREAK_9) {
			action = std::make_shared<bubbleActions::Break>(intersectedShape.id, 9);
		}
		else if (actionType == bubbleInventoryitemType::BREAK_10) {
			action = std::make_shared<bubbleActions::Break>(intersectedShape.id, 10);
		}
		if (action) {
			middle::queueAction(gameState, action);
			gameState->bubbleAlgebraState.bubbleActions.push_back(action);
		}
	}

	void update(middle::GameState* gameState) override {

		auto deletionIt = deletionCache->begin<components::DeleteComponent>();
		auto idRefIt = deletionCache->begin<components::IdRef>();

		middle::Id shapeIdForDeletion;
		if(deletionCache->getSize() > 0){
			shapeIdForDeletion = deletionCache->relevantIdVector[0];
		}

		if (shapeIdForDeletion.index == middle::UNASSIGNED) {
			return;
		}

		auto& shapeForDeletion = middle::getShape(gameState, shapeIdForDeletion.index);
		auto ref = middle::getComponent<components::IdRef>(shapeForDeletion);
		if (!middle::isShapeAlive(gameState, ref->idRef.index)) {
			return;
		}
		auto& refShape = middle::getShape(gameState, ref->idRef.index);
		middle::Id refParentId = middle::getParent(gameState, refShape.id);
		assert(refShape.id.index != middle::UNASSIGNED);

		auto inventoryItem = middle::getComponent<components::InventoryItem>(refShape);

		auto intersectableIt = intersectableCache->begin<components::MouseIntersectable>();
		for (int i = 0; i < intersectableCache->getSize(); ++i) {
			auto intersectable = *intersectableIt;
			auto& shape = middle::getShape(gameState, intersectableCache->relevantIdVector[i].index);

			if (!bubbleActions::isIntersecting(gameState, shape)) {
				continue;
			}

			// skip shapefordeletion (the copy being dragged) and ref shape (shape its copy is pointing to)
			if (shapeForDeletion.id == shape.id || shape.id == refShape.id) {
				continue;
			}

			middle::Id parentId = middle::getParent(gameState, shape.id);
			if (parentId.index != middle::UNASSIGNED && parentId == refParentId) {
				auto& refParent = middle::getShape(gameState, refParentId.index);
				combine(gameState, refParent, refShape, shape);
				continue;
			}

			if (inventoryItem) {
				inventoryAction(gameState, inventoryItem->itemType, shape);
			}
		}

	}
};

static middle::SystemRegistrar<BubbleModificationSystem> reg("BubbleModificationSystem");
