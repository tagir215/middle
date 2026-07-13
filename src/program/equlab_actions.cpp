#include "equlab_actions.h"
#include "editor_actions.h"
#include "bubble_actions.h"
#include "bubble_utils.h"
#include "MouseSelectable.h"
#include "BubbleEqualsComponent.h"
#include "BubbleVariable.h"
#include "component_utils.h"

namespace equlab {

	void AddBubble::execute(middle::GameState* gameState) {
		middle::Shape newBubble = bubble::newBubble(gameState, targetPosition);
		auto registerAction = std::make_unique<middle::EditorActionRegisterShape>(newBubble);
		registerAction->execute(gameState);
		resultId = registerAction->newShapeId;
		actions.push_back(std::move(registerAction));
		if (parentId.index != middle::UNASSIGNED) {
			auto reparent = std::make_unique<middle::EditorActionReparent>(parentId.index, resultId.index);
			reparent->execute(gameState);
			actions.push_back(std::move(reparent));
		}
	}
	void AddBubble::undo(middle::GameState* gameState) {
		while (actions.size() > 0) {
			actions.back()->undo(gameState);
			actions.pop_back();
		}
	}

	void AddUnit::execute(middle::GameState* gameState) {
		middle::Shape newUnit = bubble::newUnit(gameState, targetPosition);
		auto registerAction = std::make_unique<middle::EditorActionRegisterShape>(newUnit);
		registerAction->execute(gameState);
		resultId = registerAction->newShapeId;
		actions.push_back(std::move(registerAction));
		if (parentId.index != middle::UNASSIGNED) {
			auto reparent = std::make_unique<middle::EditorActionReparent>(parentId.index, resultId.index);
			reparent->execute(gameState);
			actions.push_back(std::move(reparent));
		}
	}

	void AddUnit::undo(middle::GameState* gameState) {
		while (actions.size() > 0) {
			actions.back()->undo(gameState);
			actions.pop_back();
		}
	}

	void Negate::execute(middle::GameState* gameState) {
		middle::Id replacementShapeId = bubbleActions::createNegatedReplacementShape(gameState, id);
		auto registerAction = std::make_unique<middle::EditorActionRegisterId>(replacementShapeId);
		registerAction->execute(gameState);
		actions.push_back(std::move(registerAction));

		auto replace = std::make_unique<bubbleActions::Replace>(id, replacementShapeId);
		replace->execute(gameState);
		actions.push_back(std::move(replace));
	}
	void Negate::undo(middle::GameState* gameState) {
		while (actions.size() > 0) {
			actions.back()->undo(gameState);
			actions.pop_back();
		}
	}

	void Invert::execute(middle::GameState* gameState) {
		middle::Id replacementShapeId = bubble::inverseBubble(gameState, id);
		auto registerAction = std::make_unique<middle::EditorActionRegisterId>(replacementShapeId);
		registerAction->execute(gameState);
		actions.push_back(std::move(registerAction));

		auto replace = std::make_unique<bubbleActions::Replace>(id, replacementShapeId);
		replace->execute(gameState);
		actions.push_back(std::move(replace));
	}

	void Invert::undo(middle::GameState* gameState) {
		while (actions.size() > 0) {
			actions.back()->undo(gameState);
			actions.pop_back();
		}
	}

	void AddLabelCharacterToVariable::execute(middle::GameState* gameState) {
		auto& shape = middle::getShape(gameState, id.index);

		auto comp = middle::getComponent<components::BubbleVariable>(shape);


		// if no comp create new bubble variable
		if (!comp) {
			std::vector<middle::Id>children;
			middle::getChildren(gameState, id, children);
			if (children.size() > 0) {
				cancelled = true;
				return;
			}
			else {
				middle::Id targetId = id;
				std::string targetLabel = this->label;
				auto customAction = std::make_unique<CustomActionWithUndo>(
					[targetId, targetLabel](middle::GameState* gameState) {
						auto newComp = middle::attachComponent<components::BubbleVariable>(gameState, targetId);
						newComp->label = targetLabel;
					},
					[targetId](middle::GameState* gameState) {
						middle::queueComponentDeletion<components::BubbleVariable>(gameState, targetId);
					});
				customAction->execute(gameState);
				actions.push_back(std::move(customAction));
			}
		}

		// if is already comp add character to end of current lable name
		else {
			middle::Id targetId = id;
			std::string targetLabel = this->label;
			auto customAction = std::make_unique<CustomActionWithUndo>(
				[targetId, targetLabel](middle::GameState* gameState) {
					auto& targetShape = middle::getShape(gameState, targetId.index);
					auto comp = middle::getComponent<components::BubbleVariable>(targetShape);
					comp->label += targetLabel;
				},
				[targetId, targetLabel](middle::GameState* gameState) {
					auto& targetShape = middle::getShape(gameState, targetId.index);
					auto comp = middle::getComponent<components::BubbleVariable>(targetShape);
					int targetSize = targetLabel.size();
					int labelSize = comp->label.size();
					assert(targetSize < labelSize);
					comp->label.erase(labelSize - targetSize, targetSize);
				});
			customAction->execute(gameState);
			actions.push_back(std::move(customAction));
		}
	}
	void AddLabelCharacterToVariable::undo(middle::GameState* gameState) {
		while (actions.size() > 0) {
			actions.back()->undo(gameState);
			actions.pop_back();
		}
	}





	void Delete::execute(middle::GameState* gameState) {
		middle::Id parentId = middle::getParent(gameState, id);

		if (parentId.index != middle::UNASSIGNED) {
			auto& parentShape = middle::getShape(gameState, parentId.index);
			auto opComp = middle::getComponent<components::BubbleMultiplyComponent>(parentShape);
			// if op comp unlink instead of delete
			if (opComp) {
				auto unlink = std::make_unique<bubbleActions::UnlinkMultiplicationTerm>(parentId, id);
				unlink->execute(gameState);
				actions.push_back(std::move(unlink));
				return;
			}
		}

		auto del = std::make_unique < middle::EditorActionDeleteSingle>(id);
		del->execute(gameState);
		actions.push_back(std::move(del));
	}

	void Delete::undo(middle::GameState* gameState) {
		while (actions.size() > 0) {
			actions.back()->undo(gameState);
			actions.pop_back();
		}
	}

	bool canConnect(middle::GameState* gameState, middle::Id idA, middle::Id idB) {
		auto& shapeA = middle::getShape(gameState, idA.index);
		auto& shapeB = middle::getShape(gameState, idB.index);
		if (!middle::getComponent<components::BubbleComponent>(shapeA) || !middle::getComponent<components::BubbleComponent>(shapeB)) {
			return false;
		}
		middle::Id parentAId = middle::getParent(gameState, idA);
		middle::Id parentBId = middle::getParent(gameState, idB);
		return parentAId == parentBId;
	}

	// TODO RENAME MULTIPLY COMPONENT
	void ConnectOperationLink::execute(middle::GameState* gameState)
	{

		middle::Shape newMulShapeProto;
		auto position = middle::addComponent<components::Position>(newMulShapeProto);
		middle::addComponent<components::BubbleMultiplyComponent>(newMulShapeProto);
		middle::addComponent<components::MouseIntersectable>(newMulShapeProto);
		middle::addComponent<components::MouseGrabbable>(newMulShapeProto);
		middle::addComponent<components::MouseSelectable>(newMulShapeProto);
		middle::addComponent<components::LoopTag>(newMulShapeProto);
		middle::addComponent<components::LoopSociety>(newMulShapeProto);
		auto& newMulShape = middle::registerShape(gameState, newMulShapeProto);

		auto registerAction = std::make_unique<middle::EditorActionRegisterId>(newMulShape.id);
		registerAction->execute(gameState);
		actions.push_back(std::move(registerAction));

		auto reparentA = std::make_unique<middle::EditorActionReparent>(newMulShape.id.index, idA.index);
		reparentA->execute(gameState);
		actions.push_back(std::move(reparentA));
		auto reparentB = std::make_unique<middle::EditorActionReparent>(newMulShape.id.index, idB.index);
		reparentB->execute(gameState);
		actions.push_back(std::move(reparentB));

		Vector3 center = middle::getShapePosition(gameState, idA.index) + middle::getShapePosition(gameState, idB.index);
		center *= 0.5f;
		position->posX = center.x;
		position->posY = center.y;
		position->posZ = center.z;
		resultId = newMulShape.id;
	}


	void ConnectOperationLink::undo(middle::GameState* gameState)
	{
		while (actions.size() > 0) {
			actions.back()->undo(gameState);
			actions.pop_back();
		}
	}

	void ConnectEqualsLink::execute(middle::GameState* gameState) {
		if (!canConnect(gameState, bubbleIdA, bubbleIdB)) {
			cancelled = true;
			return;
		}

		auto& shapeA = middle::getShape(gameState, bubbleIdA.index);
		auto& shapeB = middle::getShape(gameState, bubbleIdB.index);
		if (!middle::getComponent<components::BubbleComponent>(shapeA) || !middle::getComponent<components::BubbleComponent>(shapeB)) {
			cancelled = true;
			return;
		}

		middle::Shape equalsProto;
		middle::addComponent<components::BubbleEqualsComponent>(equalsProto);
		auto position = middle::addComponent<components::Position>(equalsProto);
		middle::addComponent<components::MouseIntersectable>(equalsProto);
		middle::addComponent<components::MouseGrabbable>(equalsProto);
		middle::addComponent<components::MouseSelectable>(equalsProto);
		middle::addComponent<components::LoopTag>(equalsProto);
		middle::addComponent<components::LoopSociety>(equalsProto);
		middle::Shape& equalsShape = middle::registerShape(gameState, equalsProto);

		Vector3 center = middle::getShapePosition(gameState, bubbleIdA.index) + middle::getShapePosition(gameState, bubbleIdB.index);
		center *= 0.5f;
		position->posX = center.x;
		position->posY = center.y;
		position->posZ = center.z;

		auto registerAction = std::make_unique<middle::EditorActionRegisterId>(equalsShape.id);
		registerAction->execute(gameState);
		actions.push_back(std::move(registerAction));

		auto reparentA = std::make_unique<middle::EditorActionReparent>(equalsShape.id.index, bubbleIdA.index);
		reparentA->execute(gameState);
		actions.push_back(std::move(reparentA));
		auto reparentB = std::make_unique<middle::EditorActionReparent>(equalsShape.id.index, bubbleIdB.index);
		reparentB->execute(gameState);
		actions.push_back(std::move(reparentB));
	}
	void ConnectEqualsLink::undo(middle::GameState* gameState) {
		while (actions.size() > 0) {
			actions.back()->undo(gameState);
			actions.pop_back();
		}
	}

	void ConnectMultiplicationLink::execute(middle::GameState* gameState) {

		auto& shapeA = middle::getShape(gameState, bubbleIdA.index);
		auto& shapeB = middle::getShape(gameState, bubbleIdB.index);
		if (!middle::getComponent<components::BubbleComponent>(shapeA) || !middle::getComponent<components::BubbleComponent>(shapeB)) {
			cancelled = true;
			return;
		}
		middle::Id parentIdA = middle::getParent(gameState, bubbleIdA);
		middle::Id parentIdB = middle::getParent(gameState, bubbleIdB);

		// check if b is in a multiplication
		// if they are remove first from the operation and connect each child seperatedly
		// we don't have to check for a because a can work as a reciever even if its in multiplciation
		bool parentBIsMul = false;
		if (parentIdB.index != middle::UNASSIGNED) {
			auto& parentShapeB = middle::getShape(gameState, parentIdB.index);
			parentBIsMul = middle::getComponent<components::BubbleMultiplyComponent>(parentShapeB);
		}
		if (parentBIsMul) {
			auto unlink = std::make_unique<bubbleActions::UnlinkMultiplicationTerm>(parentIdB, bubbleIdB);
			unlink->execute(gameState);
			actions.push_back(std::move(unlink));
		}

		// connect idB or if they were in operation then all the op children
		auto connectAction = std::make_unique<bubbleActions::LinkMultiplicationTerm>(bubbleIdA, bubbleIdB);
		connectAction->execute(gameState);
		resultId = connectAction->resultShapeId;
		auto& resultShape = middle::getShape(gameState, resultId.index);
		actions.push_back(std::move(connectAction));
	}
	void ConnectMultiplicationLink::undo(middle::GameState* gameState) {
		while (actions.size() > 0) {
			actions.back()->undo(gameState);
			actions.pop_back();
		}
	}

	void ConnectPowerLink::execute(middle::GameState* gameState) {
		if (!canConnect(gameState, bubbleIdA, bubbleIdB)) {
			cancelled = true;
			return;
		}

		middle::Id parentId = middle::getParent(gameState, bubbleIdA);

		auto connectAction = std::make_unique<ConnectOperationLink>(bubbleIdA, bubbleIdB);
		connectAction->execute(gameState);
		resultId = connectAction->resultId;
		auto& resultShape = middle::getShape(gameState, resultId.index);
		auto opComp = middle::getComponent<components::BubbleMultiplyComponent>(resultShape);
		opComp->operationType = components::OperationType::POWER;
		actions.push_back(std::move(connectAction));

		auto reparent = std::make_unique<middle::EditorActionReparent>(parentId.index, resultId.index);
		reparent->execute(gameState);
		actions.push_back(std::move(reparent));
	}
	void ConnectPowerLink::undo(middle::GameState* gameState) {
		while (actions.size() > 0) {
			actions.back()->undo(gameState);
			actions.pop_back();
		}
	}

}