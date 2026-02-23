#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "middle_shape_utils.h"
#include "DeleteComponent.h"
#include "MouseIntersectable.h"
#include "IdRef.h"
#include "BubbleMultiplyComponent.h"
#include "bubble_actions.h"

class BubbleModificationSystem : public middle::MiddleGameplaySystem {

	bool isMultiplicationConnection(middle::GameState* gameState, middle::Shape& parentShape) {
		auto mulComp = middle::getComponent<components::BubbleMultiplyComponent>(parentShape);
		if (mulComp) {
			return true;
		}
		return false;
	}

	void update(middle::GameState* gameState) override {

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
		auto& refParent = middle::getShape(gameState, refParentId.index);

		middle::loopInstances(gameState, [gameState, this, &refShape, &refParent, &shapeForDeletion](int i, middle::Shape& shape) {
			auto intersectable = middle::getComponent<components::MouseIntersectable>(shape);
			if (!intersectable || !intersectable->intersectingTop) {
				return true;
			}

			if (shapeForDeletion.id == shape.id) {
				return true;
			}
			if (shape.id == refShape.id) {
				return true;
			}

			middle::Id parentId = middle::getParent(gameState, shape.id);
			if (parentId == refParent.id) {
				// is multiplication connection
				if (isMultiplicationConnection(gameState, refParent)) {
					auto multiply = bubbleActions::Multiply(parentId, refShape.id, shape.id);
					multiply.execute(gameState);
					multiply.finalize(gameState);
					return false;
				}
				// else is addition connection
				else {
					auto add = bubbleActions::Combine(refShape.id, shape.id);
					add.execute(gameState);
					add.finalize(gameState);
					return false;
				}
			}

			return true;
			});
	}
};

static middle::SystemRegistrar<BubbleModificationSystem> reg("BubbleModificationSystem");
