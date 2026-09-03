#include "equlab_actions.h"
#include "editor_actions.h"
#include "bubble_actions.h"
#include "bubble_utils.h"
#include "MouseSelectable.h"
#include "BubbleEqualsComponent.h"
#include "BubbleVariable.h"
#include "component_utils.h"
#include "alg_file_utils.h"
#include <queue>
#include "UnIntersectableWindowComponent.h"
#include "TopDogBubbleTag.h"
#include "BubblePowerComponent.h"
#include "BubbleInequaltyComponent.h"
#include "BubbleFunctionComponent.h"
#include "BubbleSummationComponent.h"
#include "LocalScale.h"
#include "bubequ_mapping.h"

namespace equlab {

	const float freshnessTime = 0.8f;

	void AddBubble::execute(middle::GameState* gameState) {
		middle::Shape newBubbleProto = bubble::newBubble(gameState, targetPosition);
		auto registerAction = middle::executeAction<middle::EditorActionRegisterShape>
			(gameState, this, newBubbleProto);
		resultId = registerAction->newShapeId;
		if (parentId.index != middle::UNASSIGNED) {
			middle::executeAction<middle::EditorActionReparent>(gameState, this, parentId.index, resultId.index);
		}
		auto scaleComp = middle::getComp<components::LocalScale>(gameState, resultId);
		float scale = gameState->bubbleAlgebraState.worldScale;

		scaleComp->scale.x = scale;
		scaleComp->scale.y = scale;
		scaleComp->scale.z = scale;
		auto timer = middle::attachComponent<components::UnIntersectableWindowComponent>(gameState, resultId);
		timer->timeLeft = freshnessTime;
	}
	void AddBubble::undo(middle::GameState* gameState) {
		while (actions.size() > 0) {
			actions.back()->undo(gameState);
			actions.pop_back();
		}
	}

	void AddUnit::execute(middle::GameState* gameState) {
		if (parentId.index == middle::UNASSIGNED) {
			cancelled = true;
			return;
		}

		middle::Shape newUnitProto = bubble::newUnit(gameState, targetPosition);
		auto registerAction = std::make_unique<middle::EditorActionRegisterShape>(newUnitProto);
		registerAction->execute(gameState);
		resultId = registerAction->newShapeId;
		actions.push_back(std::move(registerAction));
		if (parentId.index != middle::UNASSIGNED) {
			auto reparent = std::make_unique<middle::EditorActionReparent>(parentId.index, resultId.index);
			reparent->execute(gameState);
			actions.push_back(std::move(reparent));
		}
		auto timer = middle::attachComponent<components::UnIntersectableWindowComponent>(gameState, resultId);
		timer->timeLeft = freshnessTime;
	}

	void AddUnit::undo(middle::GameState* gameState) {
		while (actions.size() > 0) {
			actions.back()->undo(gameState);
			actions.pop_back();
		}
	}

	void Negate::execute(middle::GameState* gameState) {
		if (id.index == middle::UNASSIGNED) {
			cancelled = true;
			return;
		}
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
		if (id.index == middle::UNASSIGNED) {
			cancelled = true;
			return;
		}
		middle::Id replacementShapeId = bubbleActions::createInverseReplacementShape(gameState, id);
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
				auto action = middle::executeAction<middle::AttachComponentAction<components::BubbleVariable>>(gameState, this, targetId);
				action->resultComp->label = targetLabel;
			}
		}
		else {
			cancelled = true;
		}
	}
	void AddLabelCharacterToVariable::undo(middle::GameState* gameState) {
		while (actions.size() > 0) {
			actions.back()->undo(gameState);
			actions.pop_back();
		}
	}

	void AddLabelToFunction::execute(middle::GameState* gameState) {
		auto& shape = middle::getShape(gameState, id.index);
		auto comp = middle::getComponent<components::BubbleVariable>(shape);

		// if no comp create new function
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
				auto attach = middle::executeAction
					<middle::AttachComponentAction<components::BubbleFunctionComponent>>
					(gameState, this, targetId);
				attach->resultComp->label = targetLabel;
			}
		}
		else {
			cancelled = true;
		}
	}
	void AddLabelToFunction::undo(middle::GameState* gameState) {
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
				auto unlink = std::make_unique<bubbleActions::UnlinkMultiplicationTerm>(id);
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



	void AddEquals::execute(middle::GameState* gameState) {
		middle::Shape bubAProto = bubble::newBubble(gameState, targetPos + Vector3{-1,0,0});
		middle::Shape bubBProto = bubble::newBubble(gameState, targetPos + Vector3{1,0,0});
		middle::Shape equalsProto = bubble::newEquals(gameState, targetPos);
		middle::Shape& bubA = middle::registerShape(gameState, bubAProto);
		middle::Shape& bubB = middle::registerShape(gameState, bubBProto);
		middle::Shape& equals = middle::registerShape(gameState, equalsProto);

		middle::EditorActionReparent(equals.id.index, bubA.id.index).execute(gameState);
		middle::EditorActionReparent(equals.id.index, bubB.id.index).execute(gameState);

		middle::executeAction<middle::EditorActionRegisterId>(gameState, this, equals.id);
		middle::executeAction<middle::EditorActionReparent>(gameState, this, parentId.index, equals.id.index);
	}

	void AddEquals::undo(middle::GameState* gameState) {
		while (actions.size() > 0) {
			actions.back()->undo(gameState);
			actions.pop_back();
		}
	}

	void AddInequals::execute(middle::GameState* gameState) {
		middle::Shape bubAProto = bubble::newBubble(gameState, targetPos + Vector3{-1,0,0});
		middle::Shape bubBProto = bubble::newBubble(gameState, targetPos + Vector3{1,0,0});
		middle::Shape inequalsProto = bubble::newInequals(gameState, targetPos, equalOr);
		middle::Shape& bubA = middle::registerShape(gameState, bubAProto);
		middle::Shape& bubB = middle::registerShape(gameState, bubBProto);
		middle::Shape& inequalsShape = middle::registerShape(gameState, inequalsProto);

		middle::EditorActionReparent(inequalsShape.id.index, bubA.id.index).execute(gameState);
		middle::EditorActionReparent(inequalsShape.id.index, bubB.id.index).execute(gameState);

		middle::executeAction<middle::EditorActionRegisterId>(gameState, this, inequalsShape.id);
		middle::executeAction<middle::EditorActionReparent>(gameState, this, parentId.index, inequalsShape.id.index);
	}

	void AddInequals::undo(middle::GameState* gameState) {
		while (actions.size() > 0) {
			actions.back()->undo(gameState);
			actions.pop_back();
		}
	}

	void AddSummation::execute(middle::GameState* gameState) {
		middle::Id newSum = bubble::newSummationWithChildren(gameState, targetPos);
		middle::executeAction<middle::EditorActionRegisterId>(gameState, this, newSum);
		middle::executeAction<middle::EditorActionReparent>(gameState, this, parentId.index, newSum.index);
	}

	void AddSummation::undo(middle::GameState* gameState) {
		while (actions.size() > 0) {
			actions.back()->undo(gameState);
			actions.pop_back();
		}
	}

	void ToggleEditable::execute(middle::GameState* gameState) {
	}

	void ToggleEditable::undo(middle::GameState* gameState) {
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
		if (parentIdA != parentIdB) {
			cancelled = true;
			return;
		}

		// create replacement
		int oldIndexA = middle::getLoopIndex(gameState, bubbleIdA);
		int oldIndexB = middle::getLoopIndex(gameState, bubbleIdB);
		middle::Id mulCopyId = middle::deepCopyShapeGlobalCoordinates(gameState, parentIdA);
		std::vector<middle::Id>children;
		middle::getChildren(gameState, mulCopyId, children);
		middle::Id copyIdA = children[oldIndexA];
		middle::Id copyIdB = children[oldIndexB];
		int childrenSize = children.size();
		middle::EditorActionRemoveFromLoop(copyIdA.index).execute(gameState);
		middle::EditorActionRemoveFromLoop(copyIdB.index).execute(gameState);
		if (childrenSize < 4) {
			middle::queueComponentDeletion<components::BubbleMultiplyComponent>(gameState, mulCopyId);
		}
		auto connectAction = bubbleActions::LinkMultiplicationTerm(copyIdA, copyIdB);
		connectAction.execute(gameState);
		middle::EditorActionReparent(mulCopyId.index, connectAction.resultShapeId.index).execute(gameState);
		resultId = mulCopyId;
		middle::executeAction<middle::EditorActionRegisterId>(gameState, this, mulCopyId);

		// replace
		middle::executeAction<bubbleActions::Replace>(gameState, this, parentIdA, mulCopyId);
	}
	void ConnectMultiplicationLink::undo(middle::GameState* gameState) {
		while (actions.size() > 0) {
			actions.back()->undo(gameState);
			actions.pop_back();
		}
	}

	void ConnectPower::execute(middle::GameState* gameState) {
		middle::Id oldParentId = middle::getParent(gameState, baseId);

		Vector3 targetPos = (middle::getGlobalPosition(gameState, baseId.index)
			+ middle::getGlobalPosition(gameState, exponentId.index)) * 0.5f;

		middle::Shape newPowerProto = bubble::newPower(gameState, targetPos);
		middle::Shape& newPower = middle::registerShape(gameState, newPowerProto);
		auto registerAction = std::make_unique<middle::EditorActionRegisterId>(newPower.id);
		registerAction->execute(gameState);
		actions.push_back(std::move(registerAction));

		auto reparentA = std::make_unique<middle::EditorActionReparent>(newPower.id.index, baseId.index);
		reparentA->execute(gameState);
		actions.push_back(std::move(reparentA));
		auto reparentB = std::make_unique<middle::EditorActionReparent>(newPower.id.index, exponentId.index);
		reparentB->execute(gameState);
		actions.push_back(std::move(reparentB));

		auto reparentC = std::make_unique<middle::EditorActionReparent>(oldParentId.index, newPower.id.index);
		reparentC->execute(gameState);
		actions.push_back(std::move(reparentC));

		resultId = newPower.id;
	}

	void ConnectPower::undo(middle::GameState* gameState) {
		while (actions.size() > 0) {
			actions.back()->undo(gameState);
			actions.pop_back();
		}
	}


	void FreeParent::execute(middle::GameState* gameState)
	{
		middle::Id parentId = middle::getParent(gameState, id);
		middle::executeAction<middle::EditorActionRemoveFromLoop>(gameState, this, id.index);
		middle::Id topId = bubble::findIdWithCompFromShapeOrItsParents<components::TopDogBubbleTag>(gameState, parentId);
		middle::executeAction<middle::EditorActionDeleteSingle>(gameState, this, topId);
	}

	void FreeParent::undo(middle::GameState* gameState)
	{
		while (actions.size() > 0) {
			actions.back()->undo(gameState);
			actions.pop_back();
		}
	}


	void LoadParent::execute(middle::GameState* gameState)
	{
		std::vector<int> traversePath = gameState->bubbleAlgebraState.traversePath;
		//traversePath.pop_back();

		auto scope = bubequ::loadBubequHead(gameState->bubbleAlgebraState.activeBubbleName, 
			traversePath, traversePath.size() + 1);
		middle::Id loadedId = bubequ::bubequToBubble(gameState, Vector3{0,0,0}, scope);
		bubble::recursiveBubbleLayoutUpdate(gameState, loadedId);

		middle::Id pathStepId = loadedId;
		for (int childIndex : traversePath) {
			std::vector<middle::Id>children;
			middle::getChildren(gameState, pathStepId, children);
			pathStepId = children[childIndex];
		}

		middle::Id pathStepParentId = middle::getParent(gameState, pathStepId);
		bubble::matchBubbleTransforms(gameState, id, pathStepParentId);


		//middle::deleteShapeRecursive(gameState, pathStepId.index);

		//middle::executeAction<middle::EditorActionReparent>(gameState, this, pathStepParentId.index, id.index);

		gameState->bubbleAlgebraState.traversePath.pop_back();
	}

	void LoadParent::undo(middle::GameState* gameState)
	{
		while (actions.size() > 0) {
			actions.back()->undo(gameState);
			actions.pop_back();
		}
	}

}