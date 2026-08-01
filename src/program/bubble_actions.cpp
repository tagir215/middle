#include "PhysicsData.h"
#include "OutputVariable.h"
#include "bubble_actions.h"
#include <string>
#include <stack>
#include "component_utils.h"
#include "BubbleRef.h"
#include "Circle.h"
#include "BubbleEqualsComponent.h"
#include "InventoryItem.h"
#include "DeleteComponent.h"
#include "BubbleVariable.h"
#include "bubble_utils.h"
#include "MouseSelectable.h"
#include "TopDogBubbleTag.h"
#include "ProcedureContainer.h"
#include "bubble_constants.h"
#include "EditThisTag.h"
#include "InsertableBubble.h"
#include "equlab_actions.h"
#include "GlobalTransform.h"
#include "BubblePowerComponent.h"

namespace bubbleActions {


	bool validateAdditionInitialState(middle::GameState* gameState, ExecuteAddition* addition) {
		middle::Id& parentAId = middle::getParent(gameState, addition->shapeToAddId);
		middle::Id& parentBId = middle::getParent(gameState, addition->shapeToAddIntoId);
		if (parentAId != parentBId) {
			return false;
		}
		if (bubble::isMultiplication(gameState, parentAId) || bubble::isEqualsOrInequals(gameState, parentAId)) {
			return false;
		}

		return true;
	}

	middle::Id createNegatedReplacementShape(middle::GameState* gameState, middle::Id id) {
		middle::Id copyId = middle::deepCopyShapeGlobalCoordinates(gameState, id);

		auto& copyShape = middle::getShape(gameState, copyId.index);
		auto unit = middle::getComponent<components::BubbleUnit>(copyShape);
		if (unit) {
			unit->value = -unit->value;
			return copyId;
		}

		std::vector<middle::Id>children;
		middle::getChildren(gameState, copyId, children);
		for (middle::Id id : children) {
			middle::Id targetId = id;
			auto childShape = middle::getShape(gameState, id.index);
			// link -1 to bubbles;
			auto mul = middle::getComponent<components::BubbleMultiplyComponent>(childShape);
			if (mul) {
				targetId = middle::getFirstChildWithComponent(gameState, id, middle::getTypeId<components::BubbleComponent>());
			}
			auto& targetShape = middle::getShape(gameState, targetId.index);
			auto unit = middle::getComponent<components::BubbleUnit>(targetShape);
			auto var = middle::getComponent<components::BubbleVariable>(targetShape);
			auto bubble = middle::getComponent<components::BubbleComponent>(targetShape);
			if (unit || var) {
				bubble::negate(gameState, targetId);
			}
			else if (bubble) {
				auto mulOneAction = MulOne(targetId);
				mulOneAction.execute(gameState);
				middle::Id one = mulOneAction.resultShapeId;
				bubble::negate(gameState, one);
			}
		}

		if (children.size() == 0) {
			auto& copyShape = middle::getShape(gameState, copyId.index);
			auto var = middle::getComponent<components::BubbleVariable>(copyShape);
			if (var) {
				var->isNegative = !var->isNegative;
			}
		}

		return copyId;
	}

	middle::Id createInverseReplacementShape(middle::GameState* gameState, middle::Id id) {
		auto& shape = middle::getShape(gameState, id.index);
		assert(middle::getComponent<components::BubbleComponent>(shape));
		middle::Id copyId = middle::deepCopyShapeGlobalCoordinates(gameState, id);
		Vector3 targetPos = middle::getGlobalPosition(gameState, id.index) + Vector3{1,0,0};
		// create exponent with value -1
		middle::Shape exponentProto = bubble::newUnit(gameState, targetPos, true);
		middle::Shape& exponentShape = middle::registerShape(gameState, exponentProto);
		middle::Id powerId = bubble::newPower(gameState, copyId, exponentShape.id, targetPos);
		return powerId;
	}

	middle::Id createAdditionReplacementShape(middle::GameState* gameState, middle::Id idA, middle::Id idB)
	{
		auto& shapeA = middle::getShape(gameState, idA.index);
		auto& shapeB = middle::getShape(gameState, idB.index);

		auto unitA = middle::getComponent<components::BubbleUnit>(shapeA);
		auto unitB = middle::getComponent<components::BubbleUnit>(shapeB);
		auto bubbleA = middle::getComponent<components::BubbleComponent>(shapeA);
		auto bubbleB = middle::getComponent<components::BubbleComponent>(shapeB);

		middle::Id replacementId;

		// NEW CONTAINING BUBBLE CASE
		if ((unitA && unitB) || (bubbleA && bubbleB)) {
			Vector3 targetPos = (middle::getGlobalPosition(gameState, idA.index) + middle::getGlobalPosition(gameState, idB.index)) * 0.5f;
			auto regAction = middle::EditorActionRegisterShape(bubble::newBubble(gameState, targetPos));
			regAction.execute(gameState);
			replacementId = regAction.newShapeId;

			middle::Shape& newBubbleShape = middle::getShape(gameState, replacementId.index);
			middle::moveShape(gameState, idA.index, { 5,0,0 });
			middle::moveShape(gameState, idB.index, { -5,0,0 });
			auto reparentActionA = middle::EditorActionReparent(newBubbleShape.id.index, idA.index);
			auto reparentActionB = middle::EditorActionReparent(newBubbleShape.id.index, idB.index);
			reparentActionA.execute(gameState);
			reparentActionB.execute(gameState);
		}
		// BUBBLE & UNIT CASE
		else if (unitA && bubbleB || unitB && bubbleA) {
			auto& unitShape = unitA != nullptr ? shapeA : shapeB;
			auto& bubbleShape = bubbleA != nullptr ? shapeA : shapeB;
			auto reparentAction = middle::EditorActionReparent(bubbleShape.id.index, unitShape.id.index);
			reparentAction.execute(gameState);
			replacementId = bubbleShape.id;
		}

		return replacementId;
	}

	middle::Id createMultiplicationIntoPowerReplacementShape(middle::GameState* gameState, middle::Id shapeToReplaceId, middle::Id exponentId)
	{
			// invert exponent
			middle::Id copyShapeToCopyId = middle::deepCopyShapeGlobalCoordinates(gameState, shapeToReplaceId);
			middle::Id invertedExponentId = createInverseReplacementShape(gameState, exponentId);
			auto connectPower = std::make_unique<equlab::ConnectPower>(copyShapeToCopyId, invertedExponentId);
			connectPower->execute(gameState);
			middle::Id newPowerId = connectPower->resultId;
			return newPowerId;
	}

	middle::Id createMultiplicationReplacementShape(middle::GameState* gameState, middle::Id shapeToReplaceId, middle::Id replacingShapeId)
	{
		auto& shapeToReplace = middle::getShape(gameState, shapeToReplaceId.index);
		auto& replacingShape = middle::getShape(gameState, replacingShapeId.index);

		Vector3 targetPos = middle::getGlobalPosition(gameState, shapeToReplace.id.index);

		auto bubbleComp = middle::getComponent<components::BubbleComponent>(shapeToReplace);
		auto mulComp = middle::getComponent<components::BubbleMultiplyComponent>(shapeToReplace);
		auto unit = middle::getComponent<components::BubbleUnit>(shapeToReplace);
		auto variable = middle::getComponent<components::BubbleVariable>(shapeToReplace);

		// if shape to replace is a unit
		if (unit)
		{
			middle::Id shapeToCopyId = replacingShape.id;
			if (unit->value == 0) {
				shapeToCopyId = shapeToReplace.id;
			}

			middle::Id copyId = middle::deepCopyShapeGlobalCoordinates(gameState, shapeToCopyId);
			// 
			unit = middle::getComponent<components::BubbleUnit>(shapeToReplace);

			if (unit->value == -1) {
				middle::Id negativeCopyId = createNegatedReplacementShape(gameState, copyId);
				Replace(copyId, negativeCopyId).execute(gameState);
				copyId = negativeCopyId;
			}

			// compute displacmenet from replacing shape to shapeToReplace position
			Vector3 replacingShapePos = middle::getGlobalPosition(gameState, copyId.index);
			Vector3 displacement = targetPos - replacingShapePos;
			middle::moveShape(gameState, copyId.index, displacement);
			return copyId;
		}
		else if (bubbleComp || mulComp || variable) {
			middle::Id replacingCopyId = middle::deepCopyShapeGlobalCoordinates(gameState, replacingShapeId);
			middle::Id toReplaceCopyId = middle::deepCopyShapeGlobalCoordinates(gameState, shapeToReplaceId);

			middle::Id recieverId;
			auto& shapeToReplaceCopy = middle::getShape(gameState, toReplaceCopyId.index);
			// if mul comp we link to one of the children, since thats what linkMultiplication expects
			auto mulComp = middle::getComponent<components::BubbleMultiplyComponent>(shapeToReplaceCopy);
			if (mulComp) {
				std::vector<middle::Id>children;
				middle::getChildren(gameState, shapeToReplaceCopy.id, children);
				recieverId = children[0];
			}
			else {
				recieverId = toReplaceCopyId;;
			}
			auto linkAction = LinkMultiplicationTerm(recieverId, replacingCopyId);
			linkAction.execute(gameState);
			return linkAction.resultShapeId;
		}

		return middle::Id();
	}

	middle::Id createExpandedBaseReplacementShape(middle::GameState* gameState, middle::Id baseId, middle::Id exponentId) {
		// create replacement
		middle::Id replacementShapeId;
		std::vector<middle::Id>exponentChildren;
		middle::getChildren(gameState, exponentId, exponentChildren);

		if (exponentChildren.size() != 0) {

			Vector3 basePos = middle::getGlobalPosition(gameState, baseId.index);

			middle::Id prevId;
			for (middle::Id& exponentChildId : exponentChildren) {
				auto& childShape = middle::getShape(gameState, exponentChildId.index);

				middle::Id baseCopyId = middle::deepCopyShapeGlobalCoordinates(gameState, baseId);
				middle::moveShape(gameState, baseCopyId.index, middle::getGlobalPosition(gameState, exponentChildId.index) - basePos);

				middle::Id linkingId;
				auto unit = middle::getComponent<components::BubbleUnit>(childShape);
				// unit case
				if (unit) {
					if (unit->value == -1) {
						linkingId = bubbleActions::createInverseReplacementShape(gameState, baseCopyId);
						middle::deleteShapeRecursive(gameState, baseCopyId.index);
					}
					else {
						linkingId = baseCopyId;
					}
				}
				else {
					auto mul = middle::getComponent<components::BubbleMultiplyComponent>(childShape);
					// if multiplication containerize it first in a bubble, else assumed to be a bubble
					exponentChildId = middle::deepCopyShapeGlobalCoordinates(gameState, exponentChildId);
					if (mul) {
						exponentChildId = bubble::containerize(gameState, exponentChildId);
					}
					// create a new power for the term
					middle::Id newPowerId = bubble::newPower(gameState, baseCopyId, exponentChildId, middle::getGlobalPosition(gameState, exponentChildId.index));
					linkingId = newPowerId;
				}

				// link as multiplication
				if (prevId.index != middle::UNASSIGNED) {
					LinkMultiplicationTerm(prevId, linkingId).execute(gameState);
				}
				prevId = linkingId;

			}

			if (exponentChildren.size() == 1) {
				replacementShapeId = prevId;
			}
			else {
				replacementShapeId = middle::getParent(gameState, prevId);
			}
		}
		else if (bubble::isBubbleWithValueOne(gameState, exponentId)) {
			return middle::deepCopyShapeGlobalCoordinates(gameState, baseId);
		}
		// replacement shape is bubble with value 0
		else if (bubble::isBubbleZero(gameState, exponentId)) {
			Vector3 targetPosition = middle::getGlobalPosition(gameState, exponentId.index);
			middle::Shape newUnitProto = bubble::newUnit(gameState, targetPosition);
			middle::Shape& newUnit = middle::registerShape(gameState, newUnitProto);
			replacementShapeId = newUnit.id;
		}
		else {
			return middle::Id();
		}

		return bubble::containerize(gameState, replacementShapeId);
	}

	middle::Id createExpandedExponentReplacementShape(middle::GameState* gameState, middle::Id powerShapeId) {
		middle::Id baseId, exponentId;
		bubble::getPowerBaseAndExponent(gameState, powerShapeId, baseId, exponentId);

		bool isPower = bubble::isPowerBubble(gameState, baseId);
		bool isMul = bubble::isMultiplication(gameState, baseId);
		std::vector<middle::Id>children;
		middle::getChildren(gameState, baseId, children);
		bool isChildCount1 = children.size() == 1;
		if (!isPower && !isMul && !isChildCount1) {
			return middle::Id();
		}

		middle::Id copyPowerShapeId = middle::deepCopyShapeGlobalCoordinates(gameState, powerShapeId);
		middle::Id copyBaseId, copyExponentId;
		bubble::getPowerBaseAndExponent(gameState, copyPowerShapeId, copyBaseId, copyExponentId);

		if (isPower) {
			middle::Id childBaseId, childExponentId;
			bubble::getPowerBaseAndExponent(gameState, copyBaseId, childBaseId, childExponentId);
			middle::Id mulReplacement = createMultiplicationReplacementShape(gameState, childExponentId, copyExponentId);
			Replace(childExponentId, mulReplacement).execute(gameState);
			middle::EditorActionRemoveFromLoop(copyBaseId.index).execute(gameState);
			middle::deleteShapeRecursive(gameState, copyExponentId.index);
			middle::deleteShapeRecursive(gameState, copyPowerShapeId.index);
			return copyBaseId;
		}
		else if (isMul || isChildCount1) {
			std::vector<middle::Id>children;
			middle::getChildren(gameState, copyBaseId, children);
			for (middle::Id childId : children) {
				middle::Id copyExponentId2 = middle::deepCopyShapeGlobalCoordinates(gameState, copyExponentId);
				auto connectPower = equlab::ConnectPower(childId, copyExponentId2);
				connectPower.execute(gameState);
			}
			middle::EditorActionRemoveFromLoop(copyBaseId.index).execute(gameState);
			middle::deleteShapeRecursive(gameState, copyExponentId.index);
			middle::deleteShapeRecursive(gameState, copyPowerShapeId.index);
			return copyBaseId;
		}
		return middle::Id();
	}

	middle::Id createCommonBaseReplacementShape(middle::GameState* gameState, middle::Id commonBaseId) {
		middle::Id parentId = middle::getParent(gameState, commonBaseId);
		if (parentId.index == middle::UNASSIGNED) {
			return middle::Id();
		}
		middle::Id parentParentId = middle::getParent(gameState, parentId);
		if (parentParentId.index == middle::UNASSIGNED) {
			return middle::Id();
		}
		auto mulShape = middle::getShape(gameState, parentParentId.index);
		if (!middle::getComponent<components::BubbleMultiplyComponent>(mulShape)) {
			return middle::Id();
		}

		std::vector<middle::Id>mulChildren;
		middle::getChildren(gameState, mulShape.id, mulChildren);

		std::vector<middle::Id>exponentIds;
		for (middle::Id id : mulChildren) {
			if (!bubble::isPowerBubble(gameState, id)) {
				return middle::Id();
			}
			middle::Id baseId, exponentId;
			bubble::getPowerBaseAndExponent(gameState, id, baseId, exponentId);
			if (!bubble::matchingBubbles(gameState, baseId, commonBaseId)) {
				return middle::Id();
			}
			exponentIds.push_back(exponentId);
		}

		middle::Id copyBaseId = middle::deepCopyShapeGlobalCoordinates(gameState, commonBaseId);

		middle::Shape bubbleProto = bubble::newBubble(gameState, middle::getGlobalPosition(gameState, commonBaseId.index));
		middle::Shape& newExponent = middle::registerShape(gameState, bubbleProto);
		for (middle::Id exponentId : exponentIds) {
			middle::Id copyExponentId = middle::deepCopyShapeGlobalCoordinates(gameState, exponentId);
			middle::EditorActionReparent(newExponent.id.index, copyExponentId.index).execute(gameState);
		}

		auto connectPower = equlab::ConnectPower(copyBaseId, newExponent.id);
		connectPower.execute(gameState);
		return connectPower.resultId;
	}

	middle::Id createCompressedMultiplicationPowerShape(middle::GameState* gameState, middle::Id commonFactorId) {
		middle::Id parentId = middle::getParent(gameState, commonFactorId);
		std::vector<middle::Id>children;
		middle::getChildren(gameState, parentId, children);
		for (middle::Id id : children) {
			if (!bubble::matchingBubbles(gameState, id, commonFactorId)) {
				return middle::Id();
			}
		}
		middle::Id newExponentId = bubble::newBubbleWithIntValue(gameState, children.size(), middle::getGlobalPosition(gameState, commonFactorId.index));
		middle::Id commonCopyId = middle::deepCopyShapeGlobalCoordinates(gameState, commonFactorId);
		auto connectPower = equlab::ConnectPower(commonCopyId, newExponentId);
		connectPower.execute(gameState);
		return connectPower.resultId;
	}

	// I gave up naming this
	middle::Id createPowPowReplacmentShape(middle::GameState* gameState, middle::Id commonFactorId) {
		middle::Id parentId = middle::getParent(gameState, commonFactorId);
		if (!bubble::isPowerBubble(gameState, parentId)) {
			return middle::Id();
		}
		middle::Id parentParentId = middle::getParent(gameState, parentId);
		if (!bubble::isPowerBubble(gameState, parentParentId)) {
			return middle::Id();
		}
		middle::Id innerBaseId, innerExponentId;
		bubble::getPowerBaseAndExponent(gameState, parentId, innerBaseId, innerExponentId);
		middle::Id outerBaseId, outerExponentId;
		bubble::getPowerBaseAndExponent(gameState, parentParentId, outerBaseId, outerExponentId);

		middle::Id copyInnerExponent = middle::deepCopyShapeGlobalCoordinates(gameState, innerExponentId);
		middle::Id copyOuterExponent = middle::deepCopyShapeGlobalCoordinates(gameState, outerExponentId);

		middle::Shape newExponentProto = bubble::newBubble(gameState, middle::getGlobalPosition(gameState, outerExponentId.index));
		auto& newExponent = middle::registerShape(gameState, newExponentProto);
		auto link = LinkMultiplicationTerm(copyInnerExponent, copyOuterExponent);
		link.execute(gameState);
		middle::Id newMulId = link.resultShapeId;
		EditorActionReparent(newExponent.id.index, newMulId.index).execute(gameState);

		middle::Id copyInnerBaseId = middle::deepCopyShapeGlobalCoordinates(gameState, innerBaseId);

		auto connectPower = equlab::ConnectPower(copyInnerBaseId, newExponent.id);
		connectPower.execute(gameState);

		return connectPower.resultId;
	}

	middle::Id createCommonExponentReplacementShape(middle::GameState* gameState, middle::Id commonExponent) {
		middle::Id parentId = middle::getParent(gameState, commonExponent);
		assert(bubble::isPowerBubble(gameState, parentId));
		std::vector<middle::Id>baseIds;
		middle::Id parentParentId = middle::getParent(gameState, parentId);
		if (!bubble::isMultiplication(gameState, parentParentId)) {
			return middle::Id();
		}
		// check that all children are power bubbles, and check that exponents match the common exponent
		std::vector<middle::Id>children;
		middle::getChildren(gameState, parentParentId, children);
		for (middle::Id childId : children) {
			if (!bubble::isPowerBubble(gameState, childId)) {
				return middle::Id();
			}
			middle::Id baseId, exponentId;
			bubble::getPowerBaseAndExponent(gameState, childId, baseId, exponentId);
			if (!bubble::matchingBubbles(gameState, exponentId, commonExponent)) {
				return middle::Id();
			}
			baseIds.push_back(baseId);
		}
		// connect common exponent copy to base ids
		middle::Shape newMulProto = bubble::newMultiplication(gameState, middle::getGlobalPosition(gameState, parentParentId.index));
		middle::Shape& newMul = middle::registerShape(gameState, newMulProto);
		for (middle::Id baseId : baseIds) {
			middle::Id copyId = middle::deepCopyShapeGlobalCoordinates(gameState, baseId);
			EditorActionReparent(newMul.id.index, copyId.index).execute(gameState);
		}
		middle::Id exponentCopyId = middle::deepCopyShapeGlobalCoordinates(gameState, commonExponent);
		auto connectPower = equlab::ConnectPower(newMul.id, exponentCopyId);
		connectPower.execute(gameState);
		return connectPower.resultId;
	}



	ExecuteMultiplication::ExecuteMultiplication(middle::Id shapeToCopyId, middle::Id shapeToCopyIntoId) {
		this->shapeToCopyId = shapeToCopyId;
		this->shapeToCopyIntoId = shapeToCopyIntoId;
	}

	void ExecuteMultiplication::execute(middle::GameState* gameState) {

		// cancel if trying to expand into variable
		auto& shapeToAddInto = middle::getShape(gameState, shapeToCopyIntoId.index);
		auto varComp = middle::getComponent<components::BubbleVariable>(shapeToAddInto);
		if (varComp) {
			cancelled = true;
			return;
		}
		bool isPow = bubble::isPowerBubble(gameState, shapeToCopyIntoId);
		if (isPow) {
			cancelled = true;
			return;
		}

		middle::Id mulId = middle::getParent(gameState, shapeToAddInto.id);


		// create replacement shape, register and replace
		auto createAndReplace = [gameState, this](middle::Id toReplaceId, middle::Id replacementId) {
			middle::Id copyId = createMultiplicationReplacementShape(gameState, toReplaceId, replacementId);
			middle::executeAction<EditorActionRegisterId>(gameState, this, copyId);
			middle::executeAction<Replace>(gameState, this, toReplaceId, copyId);
			};

		// in unit case just replace the shape to copy into
		auto& shapeToCopyInto = middle::getShape(gameState, shapeToCopyIntoId.index);
		auto unitComp = middle::getComponent<components::BubbleUnit>(shapeToCopyInto);
		auto mulComp = middle::getComponent<components::BubbleMultiplyComponent>(shapeToCopyInto);
		if (unitComp || mulComp) {
			createAndReplace(shapeToCopyIntoId, shapeToCopyId);
		}
		// in mul case  replace all children
		else {
			// create replacements to the positions of the old children
			std::vector<middle::Id>children;
			middle::getChildren(gameState, shapeToCopyIntoId, children);
			for (int i = 0; i < children.size(); ++i) {
				middle::Id& childId = children[i];
				createAndReplace(childId, shapeToCopyId);
			}
		}

		middle::executeAction<middle::EditorActionDeleteSingle>(gameState, this, shapeToCopyId);

		middle::executeAction<UpdateBubblesMultiplicationIdentity>(gameState, this, mulId);

		queueSound(gameState, bubbleSounds::EXPAND_MULTIPLICATION_SOUND);
	}

	void ExecuteMultiplication::undo(middle::GameState* gameState) {
		while (actions.size() > 0) {
			actions.back()->undo(gameState);
			actions.pop_back();
		}
	}


	ExecuteAddition::ExecuteAddition(middle::Id shapeToAddId, middle::Id shapeToAddIntoId) {
		this->shapeToAddId = shapeToAddId;
		this->shapeToAddIntoId = shapeToAddIntoId;
	}


	void ExecuteAddition::execute(middle::GameState* gameState) {
		auto& shapeToAdd = middle::getShape(gameState, shapeToAddId.index);
		auto& shapeToAddInto = middle::getShape(gameState, shapeToAddIntoId.index);

		if (!validateAdditionInitialState(gameState, this)) {
			cancelled = true;
			return;
		}

		auto copyA = std::make_unique<middle::EditorActionCopySingle>(shapeToAdd.id);
		copyA->execute(gameState);
		middle::Id idA = copyA->resultId;
		actions.push_back(std::move(copyA));
		auto copyB = std::make_unique<middle::EditorActionCopySingle>(shapeToAddInto.id);
		copyB->execute(gameState);
		middle::Id idB = copyB->resultId;
		actions.push_back(std::move(copyB));

		middle::Shape& copyShapeA = middle::getShape(gameState, idA.index);
		middle::Shape& copyShapeB = middle::getShape(gameState, idB.index);

		// NORMAL AVERAGE BASIC CASE
		resultShapeId = createAdditionReplacementShape(gameState, idA, idB);
		auto registerAction = std::make_unique<middle::EditorActionRegisterId>(resultShapeId);
		registerAction->execute(gameState);
		actions.push_back(std::move(registerAction));

		auto deleteAction = std::make_unique<middle::EditorActionDeleteSingle>(shapeToAddId);
		deleteAction->execute(gameState);
		actions.push_back(std::move(deleteAction));

		auto replace = std::make_unique<Replace>(shapeToAddIntoId, resultShapeId);
		replace->execute(gameState);
		actions.push_back(std::move(replace));

		queueSound(gameState, bubbleSounds::COMBINE_SOUND);
	}

	void ExecuteAddition::undo(middle::GameState* gameState) {
		while (actions.size() > 0) {
			actions.back()->undo(gameState);
			actions.pop_back();
		}
	}

	void ExecutePower::execute(middle::GameState* gameState)
	{
		middle::Id powerId = middle::getParent(gameState, idA);
		middle::Id exponentId;
		middle::Id baseId;
		bubble::getPowerBaseAndExponent(gameState, powerId, baseId, exponentId);

		middle::Id replacementShapeId;
		if (idA == baseId) {
			// don't expand power if its variable or inverse, because I'm confused about inverse exponents
			auto& exponentShape = middle::getShape(gameState, exponentId.index);
			if (middle::getComponent<components::BubbleVariable>(exponentShape)) {
				cancelled = true;
				return;
			}
			replacementShapeId = createExpandedBaseReplacementShape(gameState, baseId, exponentId);
		}
		else if (idA == exponentId) {
			replacementShapeId = createExpandedExponentReplacementShape(gameState, powerId);
		}
		if (replacementShapeId.index == middle::UNASSIGNED) {
			cancelled = true;
			return;
		}
		middle::executeAction<middle::EditorActionRegisterId>(gameState, this, replacementShapeId);
		middle::executeAction<Replace>(gameState, this, powerId, replacementShapeId);;
	}

	void ExecutePower::undo(middle::GameState* gameState)
	{
		while (actions.size() > 0) {
			actions.back()->undo(gameState);
			actions.pop_back();
		}
	}

	Pop::Pop(middle::Id id) {
		this->id = id;
	}

	middle::Id popReplacement(middle::GameState* gameState, middle::Id containerId, middle::Id toPopId) {
		auto& toPopShape = middle::getShape(gameState, toPopId.index);
		auto var = middle::getComponent<components::BubbleVariable>(toPopShape);
		auto unit = middle::getComponent<components::BubbleUnit>(toPopShape);
		auto power = middle::getComponent<components::BubblePowerComponent>(toPopShape);
		auto mul = middle::getComponent<components::BubbleMultiplyComponent>(toPopShape);
		middle::Id copyId;
		if (var || unit || power || mul) {
			copyId = middle::deepCopyShapeGlobalCoordinates(gameState, toPopId);
		}
		else {
			copyId = middle::shallowCopyShapeGlobalCoordinates(gameState, containerId);
		}
		auto& newContainerShape = middle::getShape(gameState, copyId.index);
		auto loop = middle::getComponent<components::LoopSociety>(newContainerShape);
		loop->parentLoopId = middle::Id();
		// ?
		if (!power && !mul) {
			loop->loopMemberIds.clear();
		}
		return newContainerShape.id;
	}

	void Pop::execute(middle::GameState* gameState) {
		middle::Shape& shapeToPop = middle::getShape(gameState, id.index);
		// check that there is a parent
		auto bubble = middle::getComponent<components::BubbleComponent>(shapeToPop);
		auto unit = middle::getComponent<components::BubbleUnit>(shapeToPop);
		auto variable = middle::getComponent<components::BubbleVariable>(shapeToPop);
		auto power = middle::getComponent<components::BubblePowerComponent>(shapeToPop);
		auto mul = middle::getComponent<components::BubbleMultiplyComponent>(shapeToPop);
		middle::Id parentId = middle::getParent(gameState, shapeToPop.id);
		if (parentId.index == middle::UNASSIGNED) {
			cancelled = true;
			return;
		}
		if (!bubble) {
			cancelled = true;
			return;
		}
		if (bubble::isEqualsOrInequals(gameState, parentId)) {
			cancelled = true;
			return;
		}
		if (bubble::isPowerBubble(gameState, parentId)) {
			cancelled = true;
			return;
		}
		if (bubble::isMultiplication(gameState, parentId)) {
			cancelled = true;
			return;
		}

		std::vector<middle::Id>children;
		middle::getChildren(gameState, parentId, children);
		int siblingCount = children.size() - 1;

		// variables and units can be popped if they are the only child
		if (siblingCount != 0) {
			if (variable || unit || mul) {
				cancelled = true;
				return;
			}
			if (bubble::isPowerBubble(gameState, shapeToPop.id)) {
				cancelled = true;
				return;
			}
		}

		auto& parentShape = middle::getShape(gameState, parentId.index);
		auto parentBubble = middle::getComponent<components::BubbleComponent>(parentShape);
		if (!parentBubble) {
			cancelled = true;
			return;
		}

		// if popping variable or unit (as only child), we need to replace the parent instead. to retain var or unit identity
		if (variable || unit || power || mul) {
			Vector3 targetPos = middle::getGlobalPosition(gameState, parentId.index);

			middle::Id varPopReplacementId = popReplacement(gameState, parentId, shapeToPop.id);
			auto registerAction = std::make_unique <middle::EditorActionRegisterId>(varPopReplacementId);
			registerAction->execute(gameState);
			actions.push_back(std::move(registerAction));

			auto replace = std::make_unique<Replace>(parentId, varPopReplacementId);
			replace->execute(gameState);
			actions.push_back(std::move(replace));
		}
		else if (bubble) {
			std::vector<middle::Id>children;
			middle::getChildren(gameState, id, children);
			for (middle::Id& id : children) {
				auto reparentAction = std::make_unique<middle::EditorActionReparent>(parentId.index, id.index);
				reparentAction->execute(gameState);
				actions.push_back(std::move(reparentAction));
			}
			auto deleteAction = std::make_unique<middle::EditorActionDeleteSingle>(shapeToPop.id);
			deleteAction->execute(gameState);
			actions.push_back(std::move(deleteAction));
		}

		queueSound(gameState, bubbleSounds::POP_SOUND);
	}

	void Pop::undo(middle::GameState* gameState) {
		while (actions.size() > 0) {
			actions.back()->undo(gameState);
			actions.pop_back();
		}
	}

	Replace::Replace(middle::Id shapeToReplace, middle::Id replacingShape)
	{
		this->shapeToReplaceId = shapeToReplace;
		this->replacingShapeId = replacingShape;
	}

	void Replace::execute(middle::GameState* gameState)
	{
		middle::Id containerParent = middle::getParent(gameState, shapeToReplaceId);
		if (containerParent.index != middle::UNASSIGNED) {
			std::vector<middle::Id>children;
			middle::getChildren(gameState, containerParent, children);
			// find old index to preserve the same index after replacing
			int oldIndex = middle::getLoopIndex(gameState, shapeToReplaceId);
			// reparent to replaced shapes parent
			auto reparent = std::make_unique<middle::EditorActionReparent>(containerParent.index, replacingShapeId.index);
			reparent->execute(gameState);
			actions.push_back(std::move(reparent));
			// set the correct index
			auto newIndex = std::make_unique<middle::EditorActionChangeLoopMemberIndex>(containerParent.index, replacingShapeId.index, oldIndex);
			newIndex->execute(gameState);
			actions.push_back(std::move(newIndex));
		}

		auto delAction = std::make_unique<middle::EditorActionDeleteSingle>(shapeToReplaceId);
		delAction->execute(gameState);
		actions.push_back(std::move(delAction));
	}

	void Replace::undo(middle::GameState* gameState)
	{
		while (actions.size() > 0) {
			actions.back()->undo(gameState);
			actions.pop_back();
		}
	}


	LinkMultiplicationTerm::LinkMultiplicationTerm(middle::Id recieverShape, middle::Id linkingShape)
	{
		this->recieverShapeId = recieverShape;
		this->linkingShapeId = linkingShape;
	}

	void LinkMultiplicationTerm::execute(middle::GameState* gameState)
	{
		// if already multiplication, link as next in line
		middle::Id parentId = middle::getParent(gameState, recieverShapeId);
		if (parentId.index != middle::UNASSIGNED) {
			auto& parent = middle::getShape(gameState, parentId.index);
			auto mulComp = middle::getComponent<components::BubbleMultiplyComponent>(parent);
			auto powComp = middle::getComponent<components::BubblePowerComponent>(parent);
			if (mulComp) {
				middle::executeAction<EditorActionReparent>(gameState, this, parent.id.index, linkingShapeId.index);;
				resultShapeId = parent.id;
				return;
			}
		}

		// else create new mul
		Vector3 targetPos = (middle::getGlobalPosition(gameState, recieverShapeId.index)
			+ middle::getGlobalPosition(gameState, linkingShapeId.index)) * 0.5f;

		middle::Shape mulProto = bubble::newMultiplication(gameState, targetPos);
		middle::Shape& mulShape = middle::registerShape(gameState, mulProto);
		middle::executeAction<EditorActionRegisterId>(gameState, this, mulShape.id);

		middle::Id recieverCopyId = middle::deepCopyShapeGlobalCoordinates(gameState, recieverShapeId);
		middle::executeAction<EditorActionRegisterId>(gameState, this, recieverCopyId);
		middle::executeAction<EditorActionReparent>(gameState, this, mulShape.id.index, recieverCopyId.index);
		middle::executeAction<EditorActionReparent>(gameState, this, mulShape.id.index, linkingShapeId.index);

		middle::executeAction<Replace>(gameState, this, recieverShapeId, mulShape.id);

		resultShapeId = mulShape.id;
	}

	void LinkMultiplicationTerm::undo(middle::GameState* gameState)
	{
		while (actions.size() > 0) {
			actions.back()->undo(gameState);
			actions.pop_back();
		}
	}


	middle::Id createUnlinkedReplacementShape(middle::GameState* gameState, middle::Id unlinkingId) {
		middle::Id mulParentId = middle::getParent(gameState, unlinkingId);
		int loopIndex = middle::getLoopIndex(gameState, unlinkingId);
		middle::Id copyMulShapeId = middle::deepCopyShapeGlobalCoordinates(gameState, mulParentId);
		std::vector<middle::Id>children;
		middle::getChildren(gameState, copyMulShapeId, children);
		middle::Id unlinkingCopyId = children[loopIndex];
		middle::deleteShapeRecursive(gameState, unlinkingCopyId.index);
		std::vector<middle::Id>children2;
		middle::getChildren(gameState, copyMulShapeId, children2);
		if (children2.size() == 1) {
			middle::Id resultId = children2[0];
			EditorActionRemoveFromLoop(resultId.index).execute(gameState);
			middle::deleteShapeRecursive(gameState, copyMulShapeId.index);
			return resultId;
		}
		return copyMulShapeId;
	}

	void UnlinkMultiplicationTerm::execute(middle::GameState* gameState)
	{
		middle::Id replacementId = createUnlinkedReplacementShape(gameState, unlinkingShapeId);
		middle::executeAction<EditorActionRegisterId>(gameState, this, replacementId);
		resultUnlinkedMulId = replacementId;
		middle::Id parentId = getParent(gameState, unlinkingShapeId);
		middle::Id copyUnlinkingId = middle::deepCopyShapeGlobalCoordinates(gameState, unlinkingShapeId);
		middle::executeAction<EditorActionRegisterId>(gameState, this, copyUnlinkingId);
		resultUnlinkedId = copyUnlinkingId;
		middle::executeAction<Replace>(gameState, this, parentId, replacementId);
	}

	void UnlinkMultiplicationTerm::undo(middle::GameState* gameState)
	{
		while (actions.size() > 0) {
			actions.back()->undo(gameState);
			actions.pop_back();
		}
	}



	Break::Break(middle::Id containerShape, int dividend)
	{
		this->unitShapeId = containerShape;
		this->dividend = dividend;
	}
	void Break::execute(middle::GameState* gameState)
	{
		auto& unitShape = middle::getShape(gameState, unitShapeId.index);
		auto unit = middle::getComponent<components::BubbleUnit>(unitShape);
		auto variable = middle::getComponent<components::BubbleVariable>(unitShape);
		if (!unit || variable) {
			cancelled = true;
			return;
		}

		// create and register inverse bubble
		Vector3 targetPos = middle::getGlobalPosition(gameState, unitShape.id.index);
		middle::Shape containerBubbleProto = bubble::newBubble(gameState, targetPos);
		auto registerAction = std::make_unique<middle::EditorActionRegisterShape>(containerBubbleProto);
		registerAction->execute(gameState);
		middle::Id newContainerId = registerAction->newShapeId;
		actions.push_back(std::move(registerAction));

		for (int i = 0; i < dividend; ++i) {

			// create and register inverse bubble
			Vector3 targetPos = middle::getGlobalPosition(gameState, unitShape.id.index);
			middle::Shape containerBubbleProto = bubble::newBubble(gameState, targetPos + Vector3{ 1.0f * i, 0,0 });
			middle::Shape& containerBubble = middle::registerShape(gameState, containerBubbleProto);
			auto bubble = middle::getComponent<components::BubbleComponent>(containerBubble);

			// create units 
			for (int j = 0; j < dividend; ++j) {
				middle::Id& copyId = middle::deepCopyShapeGlobalCoordinates(gameState, unitShape.id);
				middle::EditorActionReparent(containerBubble.id.index, copyId.index).execute(gameState);
			}

			middle::EditorActionReparent(newContainerId.index, containerBubble.id.index).execute(gameState);
		}

		auto replace = std::make_unique<Replace>(unitShapeId, newContainerId);
		replace->execute(gameState);
		actions.push_back(std::move(replace));

		resultShapeId = newContainerId;
	}

	void Break::undo(middle::GameState* gameState)
	{
		while (actions.size() > 0) {
			actions.back()->undo(gameState);
			actions.pop_back();
		}
	}

	CompressCommonFactor::CompressCommonFactor(middle::Id containerShape)
	{
		this->commonFactorId = containerShape;
	}

	struct RepresentativeGroup {
		middle::Id containerId;
		std::vector<middle::Id>representatives;
		int commonIndex = -1;
	};

	void updateGroupWithCommonVariable(middle::GameState* gameState, RepresentativeGroup& group, middle::Id commonId) {
		for (int i = 0; i < group.representatives.size(); ++i) {
			middle::Id& id = group.representatives[i];
			if (bubble::matchingBubbles(gameState, id, commonId)) {
				group.commonIndex = i;
				return;
			}
		}
		if (bubble::matchingBubbles(gameState, group.containerId, commonId)) {
			group.representatives.clear();
			group.representatives.push_back(group.containerId);
			group.containerId = middle::getParent(gameState, group.containerId);
			group.commonIndex = 0;
		}
	}

	struct CommonVariableResult {
		bool commonVariableFound = false;
		middle::Id commonVariableId;
		// indexes where the common variable can be found in multiplications
		std::vector<RepresentativeGroup>groups;
	};

	CommonVariableResult getCommonVariableGroups(middle::GameState* gameState, middle::Id bubbleId, middle::Id commonId) {

		std::vector<RepresentativeGroup>groups;

		// if is power bubble we do this...
		if (bubble::isPowerBubble(gameState, bubbleId)) {
			middle::Id baseId, exponentId;
			bubble::getPowerBaseAndExponent(gameState, bubbleId, baseId, exponentId);
			if (commonId == exponentId) {
				return CommonVariableResult();
			}
			RepresentativeGroup group;
			middle::Id containerId = middle::getParent(gameState, commonId);
			// if baseId is common factor, that means common factor is a bubble and can be only other child besides exponent
			if (commonId == baseId) {
				group.representatives = { baseId };
			}
			else {
				std::vector<middle::Id>children;
				middle::getChildren(gameState, containerId, children);
				group.representatives = children;
			}
			CommonVariableResult result;
			group.containerId = containerId;
			updateGroupWithCommonVariable(gameState, group, commonId);
			result.groups.push_back(group);
			result.commonVariableFound = true;
			result.commonVariableId = commonId;
			return result;
		}


		std::vector<middle::Id>children;
		middle::getChildren(gameState, bubbleId, children);
		for (middle::Id& childId : children) {
			auto& childShape = middle::getShape(gameState, childId.index);
			auto bubbleComp = middle::getComponent<components::BubbleComponent>(childShape);

			auto multiplicationComp = middle::getComponent<components::BubbleMultiplyComponent>(childShape);
			if (multiplicationComp) {
				std::vector<middle::Id>mulChildren;
				middle::getChildren(gameState, childId, mulChildren);
				RepresentativeGroup group;
				group.containerId = childId;
				group.representatives = mulChildren;
				groups.push_back(group);
				continue;
			}

			if (bubbleComp) {
				RepresentativeGroup group;
				group.containerId = childId;
				group.representatives.push_back(childId);
				groups.push_back(group);
				continue;
			}

		}

		if (groups.size() < 1) {
			return CommonVariableResult();
		}

		// check that all the other groups contain a similar representative to one of representatives of the first group
		bool foundCommonInAllGroups = true;
		for (int i = 0; i < groups.size(); ++i) {
			updateGroupWithCommonVariable(gameState, groups[i], commonId);
			if (groups[i].commonIndex == -1) {
				foundCommonInAllGroups = false;
				break;
			}
		}
		if (foundCommonInAllGroups) {
			CommonVariableResult result;
			result.commonVariableFound = true;
			result.commonVariableId = commonId;
			result.groups = groups;
			return result;
		}

		return CommonVariableResult();
	}

	bool isCommonVariable(middle::GameState* gameState, middle::Id targetId, middle::Id commonId) {
		std::vector<middle::Id>children;
		middle::getChildren(gameState, targetId, children);
		for (middle::Id& id : children) {
			if (id == commonId) {
				continue;
			}
			if (!bubble::matchingBubbles(gameState, id, commonId)) {
				return false;
			}
		}
		return true;
	}

	middle::Id createMultiplicationCompressedShape(middle::GameState* gameState, middle::Id compressTargetId, middle::Id commonFactorId) {
		// find commonFactor indexes from each scope within compress target
		CommonVariableResult commonVariableResult = getCommonVariableGroups(gameState, compressTargetId, commonFactorId);
		if (!commonVariableResult.commonVariableFound) {
			return middle::Id();
		}

		Vector3 targetPos = middle::getGlobalPosition(gameState, compressTargetId.index);
		middle::Shape containerProto = bubble::newBubble(gameState, targetPos);
		middle::Shape& container = middle::registerShape(gameState, containerProto);

		for (RepresentativeGroup& group : commonVariableResult.groups) {
			// assume singular thing, it will be replaced with bubble containing unit one
			if (group.representatives.size() == 1) {
				targetPos.x += 1;
				middle::Shape unitProto = bubble::newUnit(gameState, targetPos);
				auto& newUnit = middle::registerShape(gameState, unitProto);
				auto unit = middle::getComponent<components::BubbleUnit>(newUnit);
				middle::EditorActionReparent(container.id.index, newUnit.id.index).execute(gameState);
			}
			//assume is multiplication, it will be replaced with unlinked version of itself
			else {
				middle::Id copyMulId = middle::deepCopyShapeGlobalCoordinates(gameState, group.containerId);
				std::vector<middle::Id>copyChildren;
				middle::getChildren(gameState, copyMulId, copyChildren);
				middle::Id unlinkingId = copyChildren[group.commonIndex];
				auto unlink = UnlinkMultiplicationTerm(unlinkingId);
				unlink.execute(gameState);
				middle::EditorActionReparent(container.id.index, unlink.resultUnlinkedMulId.index).execute(gameState);
				middle::deleteShapeRecursive(gameState, unlink.resultUnlinkedId.index);
			}
		}

		// if is power bubble, we also need to include the exponent to the compressed shape
		if (bubble::isPowerBubble(gameState, compressTargetId)) {
			middle::Id baseId, exponentId;
			bubble::getPowerBaseAndExponent(gameState, compressTargetId, baseId, exponentId);
			middle::Id exponentCopyId = middle::deepCopyShapeGlobalCoordinates(gameState, exponentId);
			middle::EditorActionReparent(container.id.index, exponentCopyId.index).execute(gameState);
			middle::attachComponent<components::BubblePowerComponent>(gameState, container.id);
		}

		return container.id;
	}

	void CompressCommonFactor::execute(middle::GameState* gameState)
	{
		// find common factor container bubble
		middle::Id commonParentId = middle::getParent(gameState, commonFactorId);
		if (commonParentId.index == middle::UNASSIGNED) {
			cancelled = true;
			return;
		}
		bool commonParentIsMultiplication = middle::getComp<components::BubbleMultiplyComponent>(gameState, commonParentId);
		middle::Id containerBubbleId = commonParentIsMultiplication ?
			middle::getParent(gameState, commonParentId) : commonParentId;

		if (containerBubbleId.index == middle::UNASSIGNED) {
			cancelled = true;
			return;
		}
		middle::Id compressedBubbleId = createMultiplicationCompressedShape(gameState, containerBubbleId, commonFactorId);
		if (compressedBubbleId.index == middle::UNASSIGNED) {
			cancelled = true;
			return;
		}
		middle::executeAction<middle::EditorActionRegisterId>(gameState, this, compressedBubbleId);

		// common factor that links
		middle::Id linkingFactorId;

		// if containded by power we need to power it with inverse exponent of the power
		if (bubble::isPowerBubble(gameState, containerBubbleId)) {
			middle::Id baseId, exponentId;
			bubble::getPowerBaseAndExponent(gameState, containerBubbleId, baseId, exponentId);
			linkingFactorId = createMultiplicationIntoPowerReplacementShape(gameState, commonFactorId, exponentId);
		}
		else {
			linkingFactorId = middle::deepCopyShapeGlobalCoordinates(gameState, commonFactorId);
		}

		middle::executeAction<middle::EditorActionRegisterId>(gameState, this, linkingFactorId);
		middle::executeAction<Replace>(gameState, this, containerBubbleId, compressedBubbleId);
		middle::executeAction<LinkMultiplicationTerm>(gameState, this, compressedBubbleId, linkingFactorId);

		queueSound(gameState, bubbleSounds::COMPRESS_SOUND);
	}

	void CompressCommonFactor::undo(middle::GameState* gameState)
	{
		while (actions.size() > 0) {
			actions.back()->undo(gameState);
			actions.pop_back();
		}
	}

	CompressPowers::CompressPowers(middle::Id commonFactorId) {
		this->commonFactorId = commonFactorId;
	}

	void CompressPowers::execute(middle::GameState* gameState)
	{
		middle::Id parentId = middle::getParent(gameState, commonFactorId);
		if (parentId.index == middle::UNASSIGNED) {
			cancelled = true;
			return;
		}

		middle::Id baseId, exponentId, replacementShapeId, targetId;
		if (bubble::isPowerBubble(gameState, parentId)) {
			bubble::getPowerBaseAndExponent(gameState, parentId, baseId, exponentId);
			if (commonFactorId == baseId) {
				replacementShapeId = createCommonBaseReplacementShape(gameState, commonFactorId);
			}
			else if (commonFactorId == exponentId) {
				replacementShapeId = createPowPowReplacmentShape(gameState, commonFactorId);
				if (replacementShapeId.index == middle::UNASSIGNED) {
					replacementShapeId = createCommonExponentReplacementShape(gameState, commonFactorId);
				}
			}
			targetId = middle::getParent(gameState, parentId);
		}
		else if (bubble::isMultiplication(gameState, parentId)) {
			replacementShapeId = createCompressedMultiplicationPowerShape(gameState, commonFactorId);
			targetId = parentId;
		}

		if (replacementShapeId.index != middle::UNASSIGNED) {
			middle::executeAction<middle::EditorActionRegisterId>(gameState, this, replacementShapeId);
			middle::executeAction<Replace>(gameState, this, targetId, replacementShapeId);
		}
		else {
			cancelled = true;
		}
	}

	void CompressPowers::undo(middle::GameState* gameState)
	{
		while (actions.size() > 0) {
			actions.back()->undo(gameState);
			actions.pop_back();
		}
	}


	MulOne::MulOne(middle::Id recieverShapeId)

	{
		this->recieverShapeId = recieverShapeId;
	}
	void MulOne::execute(middle::GameState* gameState)
	{
		middle::Shape& shape = middle::getShape(gameState, recieverShapeId.index);
		auto bubble = middle::getComponent<components::BubbleComponent>(shape);
		if (!bubble) {
			cancelled = true;
			return;
		}
		if (middle::getComponent<components::TopDogBubbleTag>(shape)) {
			cancelled = true;
			return;
		}
		middle::Id parentId = middle::getParent(gameState, recieverShapeId);
		if (parentId.index == middle::UNASSIGNED) {
			cancelled = true;
			return;
		}
		Vector3 targetPos = middle::getGlobalPosition(gameState, recieverShapeId.index);
		middle::Shape newUnitProto = bubble::newUnit(gameState, targetPos);
		middle::Shape& newUnit = middle::registerShape(gameState, newUnitProto);
		middle::Id containerId = bubble::containerize(gameState, newUnit.id);

		auto registerAction = std::make_unique<middle::EditorActionRegisterId>(containerId);
		registerAction->execute(gameState);
		actions.push_back(std::move(registerAction));

		auto link = std::make_unique<LinkMultiplicationTerm>(recieverShapeId, containerId);
		link->execute(gameState);
		actions.push_back(std::move(link));

		resultShapeId = containerId;

		queueSound(gameState, bubbleSounds::MUL_ONE_SOUND);
	}

	void MulOne::undo(middle::GameState* gameState)
	{
		while (actions.size() > 0) {
			actions.back()->undo(gameState);
			actions.pop_back();
		}
	}


	void MulNegativeOne::execute(middle::GameState* gameState)
	{
		auto mulOne = std::make_unique<MulOne>(recieverShapeId);
		mulOne->execute(gameState);
		if (mulOne->cancelled) {
			cancelled = true;
			return;
		}
		middle::Id one = mulOne->resultShapeId;
		bubble::negate(gameState, one);
		actions.push_back(std::move(mulOne));

		auto mulOne2 = std::make_unique<MulOne>(recieverShapeId);
		mulOne2->execute(gameState);
		if (mulOne2->cancelled) {
			cancelled = true;
			return;
		}
		middle::Id one2 = mulOne2->resultShapeId;
		bubble::negate(gameState, one2);
		actions.push_back(std::move(mulOne2));
	}

	void MulNegativeOne::undo(middle::GameState* gameState)
	{
		while (actions.size() > 0) {
			actions.back()->undo(gameState);
			actions.pop_back();
		}
	}



	UpdateVariable::UpdateVariable(std::string label, std::function<middle::Id()> newUnitRefProvider)
	{
		this->label = label;
		this->newUnitRefProvider = newUnitRefProvider;
	}
	void UpdateVariable::execute(middle::GameState* gameState)
	{
		std::string& label = this->label;
		middle::Id& newUnitRef = newUnitRefProvider();
		middle::Id& oldUnitRef = this->oldUnitRef;

		middle::loopInstances(gameState, [gameState, &label, &newUnitRef, &oldUnitRef](int i, middle::Shape& shape) {
			auto inputVariable = middle::getComponent<components::InputVariable>(shape);
			auto outputVariable = middle::getComponent<components::OutputVariable>(shape);
			if (outputVariable && outputVariable->label == label) {
				oldUnitRef = outputVariable->unitRef;
				outputVariable->unitRef = newUnitRef;
			}
			return true;
			});
	}
	void UpdateVariable::undo(middle::GameState* gameState)
	{
		middle::Id& oldRef = oldUnitRef;
		auto update = UpdateVariable(label, [oldRef]() {return oldRef; });
		update.execute(gameState);
	}


	bool validateNewTermState(middle::GameState* gameState, middle::Id& shapeToAddIntoId, middle::Id& termId) {
		return true;
	}



	void NewAdditionTerm::execute(middle::GameState* gameState)
	{
		middle::Id topDog = bubble::findIdWithCompFromShapeOrItsParents<components::TopDogBubbleTag>(gameState, shapeToAddIntoId);
		shapeToAddIntoId = topDog;

		std::vector<middle::Id>shapesToAddIntoIds;

		bool isEquOrInequ = bubble::isEqualsOrInequals(gameState, topDog);
		if (isEquOrInequ) {
			std::vector<middle::Id>equChildren;
			middle::getChildren(gameState, topDog, equChildren);
			for (middle::Id childId : equChildren) {
				shapesToAddIntoIds.push_back(childId);
			}
		}
		else {
			shapesToAddIntoIds.push_back(shapeToAddIntoId);
		}

		for (middle::Id shapeToAddInto : shapesToAddIntoIds) {
			middle::Id copyId = middle::deepCopyShapeGlobalCoordinates(gameState, newTermId);
			middle::executeAction<middle::EditorActionRegisterId>(gameState, this, copyId);
			middle::executeAction<middle::EditorActionReparent>(gameState, this, shapeToAddInto.index, copyId.index);
		}

		queueSound(gameState, bubbleSounds::ADD_TERM_SOUND);
	}

	void NewAdditionTerm::undo(middle::GameState* gameState)
	{
		while (actions.size() > 0) {
			actions.back()->undo(gameState);
			actions.pop_back();
		}
	}

	void NewMultiplicationTerm::execute(middle::GameState* gameState)
	{
		middle::Id topDog = bubble::findIdWithCompFromShapeOrItsParents<components::TopDogBubbleTag>(gameState, shapeToAddIntoId);
		shapeToAddIntoId = topDog;

		std::vector<middle::Id>shapesToAddIntoIds;

		bool isEquOrInequ = bubble::isEqualsOrInequals(gameState, topDog);
		if (isEquOrInequ) {
			std::vector<middle::Id>equChildren;
			middle::getChildren(gameState, topDog, equChildren);
			for (middle::Id childId : equChildren) {
				shapesToAddIntoIds.push_back(childId);
			}
		}
		else {
			shapesToAddIntoIds.push_back(shapeToAddIntoId);
		}

		for (middle::Id shapeToAddInto : shapesToAddIntoIds) {
			middle::Id replacementShapeId = createMultiplicationReplacementShape(gameState, shapeToAddInto, newTermId);
			middle::executeAction<middle::EditorActionRegisterId>(gameState, this, replacementShapeId);
			middle::executeAction<Replace>(gameState, this, shapeToAddInto, replacementShapeId);
		}

		queueSound(gameState, bubbleSounds::ADD_TERM_SOUND);
	}

	void NewMultiplicationTerm::undo(middle::GameState* gameState)
	{
		while (actions.size() > 0) {
			actions.back()->undo(gameState);
			actions.pop_back();
		}
	}

	void NewPowerTerm::execute(middle::GameState* gameState)
	{
		middle::Id topDog = bubble::findIdWithCompFromShapeOrItsParents<components::TopDogBubbleTag>(gameState, shapeToAddIntoId);
		shapeToAddIntoId = topDog;

		std::vector<middle::Id>shapesToAddIntoIds;

		bool isEquOrInequ = bubble::isEqualsOrInequals(gameState, topDog);
		if (isEquOrInequ) {
			std::vector<middle::Id>equChildren;
			middle::getChildren(gameState, topDog, equChildren);
			for (middle::Id childId : equChildren) {
				shapesToAddIntoIds.push_back(childId);
			}
		}
		else {
			shapesToAddIntoIds.push_back(shapeToAddIntoId);
		}
		for (middle::Id shapeToAddInto : shapesToAddIntoIds) {
			middle::Id copyId = middle::deepCopyShapeGlobalCoordinates(gameState, newTermId);
			middle::executeAction<middle::EditorActionRegisterId>(gameState, this, copyId);
			middle::executeAction<equlab::ConnectPower>(gameState, this, shapeToAddInto, copyId);
		}
		queueSound(gameState, bubbleSounds::ADD_TERM_SOUND);
	}

	void NewPowerTerm::undo(middle::GameState* gameState)
	{
		while (actions.size() > 0) {
			actions.back()->undo(gameState);
			actions.pop_back();
		}
	}

	bool parentIsBubble(middle::GameState* gameState, middle::Id id) {
		middle::Id parentId = middle::getParent(gameState, id);
		if (parentId.index == middle::UNASSIGNED) {
			return false;
		}
		auto& parentShape = middle::getShape(gameState, parentId.index);
		auto bubble = middle::getComponent<components::BubbleComponent>(parentShape);
		return bubble != nullptr;
	}

	void Bubblify::execute(middle::GameState* gameState)
	{
		Vector3 targetPos = middle::getGlobalPosition(gameState, id.index);
		middle::Id parentId;
		middle::Id targetId;
		if (parentIsBubble(gameState, id)) {
			targetId = id;
			parentId = middle::getParent(gameState, id);
		}
		else {
			targetId = middle::getParent(gameState, id);
			if (targetId.index == middle::UNASSIGNED) {
				cancelled = true;
				return;
			}
			parentId = middle::getParent(gameState, targetId);
			if (parentId.index == middle::UNASSIGNED) {
				cancelled = true;
				return;
			}
		}


		middle::Shape bubbleProto = bubble::newBubble(gameState, targetPos);
		auto registerBubble = std::make_unique<middle::EditorActionRegisterShape>(bubbleProto);
		registerBubble->execute(gameState);
		resultId = registerBubble->newShapeId;
		actions.push_back(std::move(registerBubble));

		middle::Id copyTargetId = middle::deepCopyShapeGlobalCoordinates(gameState, targetId);
		auto registerId = std::make_unique<middle::EditorActionRegisterId>(copyTargetId);
		registerId->execute(gameState);
		actions.push_back(std::move(registerId));

		auto reparentToBubble = std::make_unique<middle::EditorActionReparent>(resultId.index, copyTargetId.index);
		reparentToBubble->execute(gameState);
		actions.push_back(std::move(reparentToBubble));

		auto replace = std::make_unique<Replace>(targetId, resultId);
		replace->execute(gameState);
		actions.push_back(std::move(replace));

		queueSound(gameState, bubbleSounds::BUBBLIFY_SOUND);
	}

	void Bubblify::undo(middle::GameState* gameState)
	{
		while (actions.size() > 0) {
			actions.back()->undo(gameState);
			actions.pop_back();
		}
	}

	middle::Id simplifyToZero(middle::GameState* gameState, middle::Id bubbleId) {
		std::unordered_map<std::string, int>randomVars;
		int counter = 2;
		bubble::generateRandomVariablesValues(gameState, bubbleId, randomVars, counter);
		bubble::BubbleValue value = bubble::calculateBubbleValue(gameState, bubbleId, randomVars);

		const float epsilon = 1e-4f;
		if (std::abs(value.scale) < epsilon) {
			Vector3 targetPos = middle::getGlobalPosition(gameState, bubbleId.index);
			auto bubbleProto = bubble::newBubble(gameState, targetPos);
			auto& result = middle::registerShape(gameState, bubbleProto);
			return result.id;
		}

		return middle::Id();
	}

	middle::Id simplifyToOneOrNegativeOne(middle::GameState* gameState, middle::Id bubbleId) {
		std::unordered_map<std::string, int>randomVars;
		int counter = 2;
		bubble::generateRandomVariablesValues(gameState, bubbleId, randomVars, counter);
		bubble::BubbleValue value = bubble::calculateBubbleValue(gameState, bubbleId, randomVars);

		const float epsilon = 1e-4f;
		float absValue = std::abs(value.scale);
		bool valueIsOne = false;
		if (std::abs(absValue - 1) < epsilon) {
			valueIsOne = true;
		}

		if (valueIsOne) {
			Vector3 targetPos = middle::getGlobalPosition(gameState, bubbleId.index);
			middle::Shape unitProto = bubble::newUnit(gameState, targetPos);
			middle::Shape bubbleProto = bubble::newBubble(gameState, targetPos);
			middle::Shape& unitShape = middle::registerShape(gameState, unitProto);
			middle::Shape& bubbleShape = middle::registerShape(gameState, bubbleProto);
			auto unitComp = middle::getComponent<components::BubbleUnit>(unitShape);
			if (value.scale > 0) {
				unitComp->value = 1;
			}
			else {
				unitComp->value = -1;
			}
			middle::EditorActionReparent(bubbleShape.id.index, unitShape.id.index).execute(gameState);
			return bubbleShape.id;
		}
		return middle::Id();
	}

	middle::Id simplifyToSame(middle::GameState* gameState, middle::Id bubbleId) {
		auto& shape = middle::getShape(gameState, bubbleId.index);

		//
		return middle::Id();
	}


	void Simplify::execute(middle::GameState* gameState)
	{
		auto& shape = middle::getShape(gameState, id.index);

		// if its a variable bubble,  don't simplify
		auto bubbleVariable = middle::getComponent<components::BubbleVariable>(shape);
		if (bubbleVariable) {
			cancelled = true;
			return;
		}

		if (bubble::isBubbleWithValueOne(gameState, id)) {
			middle::Id parentId = middle::getParent(gameState, id);
			auto& parentShape = middle::getShape(gameState, parentId.index);
			if (middle::getComponent<components::BubbleMultiplyComponent>(parentShape)) {
				auto unlink = std::make_unique<UnlinkMultiplicationTerm>(id);
				unlink->execute(gameState);
				actions.push_back(std::move(unlink));

				auto deleteAction = std::make_unique<middle::EditorActionDeleteSingle>(id);
				deleteAction->execute(gameState);
				actions.push_back(std::move(deleteAction));
			}
			else {
				cancelled = true;
				return;
			}
		}
		else {
			middle::Id replacementShapeId = simplifyToSame(gameState, shape.id);
			if (replacementShapeId.index == middle::UNASSIGNED) {
				cancelled = true;
				return;
			}

			auto registerAction = std::make_unique<middle::EditorActionRegisterId>(replacementShapeId);
			registerAction->execute(gameState);
			actions.push_back(std::move(registerAction));

			auto replaceAction = std::make_unique<Replace>(shape.id, replacementShapeId);
			replaceAction->execute(gameState);
			actions.push_back(std::move(replaceAction));
		}

		queueSound(gameState, bubbleSounds::SIMPLIFY_SOUND);
	}

	void Simplify::undo(middle::GameState* gameState)
	{
		while (actions.size() > 0) {
			actions.back()->undo(gameState);
			actions.pop_back();
		}
	}

	void Cancel::execute(middle::GameState* gameState)
	{
		auto& shape = middle::getShape(gameState, id.index);

		auto bubble = middle::getComponent<components::BubbleComponent>(shape);
		middle::Id replacementShapeId;

		middle::Id parentId = middle::getParent(gameState, shape.id);

		if (bubble) {
			replacementShapeId = simplifyToOneOrNegativeOne(gameState, shape.id);

			if (replacementShapeId.index == middle::UNASSIGNED) {
				replacementShapeId = simplifyToZero(gameState, shape.id);
			}
		}

		if (replacementShapeId.index == middle::UNASSIGNED) {
			cancelled = true;
			return;
		}

		auto registerAction = std::make_unique<middle::EditorActionRegisterId>(replacementShapeId);
		registerAction->execute(gameState);
		actions.push_back(std::move(registerAction));

		auto replaceAction = std::make_unique<Replace>(shape.id, replacementShapeId);
		replaceAction->execute(gameState);
		actions.push_back(std::move(replaceAction));

		queueSound(gameState, bubbleSounds::CANCEL_SOUND);

		if (parentId.index != middle::UNASSIGNED) {
			auto pop = std::make_unique<Pop>(replacementShapeId);
			pop->execute(gameState);
			actions.push_back(std::move(pop));
		}
	}

	void Cancel::undo(middle::GameState* gameState)
	{
		while (actions.size() > 0) {
			actions.back()->undo(gameState);
			actions.pop_back();
		}
	}



	void StartProcedure::execute(middle::GameState* gameState)
	{
		auto& procContainerShape = middle::getShape(gameState, procContainer.index);
		auto procContainerComp = middle::getComponent<components::ProcedureContainer>(procContainerShape);

		std::vector<middle::Id> children;
		middle::getChildren(gameState, procContainerShape.id, children);
		middle::Id inputId = middle::getFirstChildWithComponent(gameState, procContainerShape.id, middle::getTypeId<components::InputVariable>());
		auto& inputShape = middle::getShape(gameState, inputId.index);
		auto inputComp = middle::getComponent<components::InputVariable>(inputShape);
		procContainerComp->variableOverrides = bubble::generateVariableOverrides(gameState, input, inputComp->rootNodeId);
		if (procContainerComp->variableOverrides.size() == 0) {
			cancelled = true;
			return;
		}
		inputComp->unitRef = input;
		procContainerComp->bubbleRef = input;
		procContainerComp->mode = procedureConstants::EXECUTING;


	}

	void StartProcedure::undo(middle::GameState* gameState)
	{
		while (actions.size() > 0) {
			actions.back()->undo(gameState);
			actions.pop_back();
		}
	}

	void Substitute::execute(middle::GameState* gameState)
	{
		middle::Id equalsParentId = middle::getParent(gameState, shapeToInsertId);
		if (equalsParentId.index == middle::UNASSIGNED || !bubble::isEqualsBubble(gameState, equalsParentId)) {
			cancelled = true;
			return;
		}
		std::vector<middle::Id>children;
		middle::getChildren(gameState, equalsParentId, children);
		// find other , equals should have 2 children
		assert(children.size() == 2);
		middle::Id otherId;
		for (middle::Id childId : children) {
			if (childId != shapeToInsertId) {
				otherId = childId;
				break;
			}
		}

		bool matching = bubble::matchingBubbles(gameState, shapeToReplaceId, otherId);
		if (matching) {
			middle::Id copyId = middle::deepCopyShapeGlobalCoordinates(gameState, shapeToInsertId);
			middle::executeAction<middle::EditorActionRegisterId>(gameState, this, copyId);
			middle::executeAction<Replace>(gameState, this, shapeToReplaceId, copyId);
		}
		else {
			cancelled = true;
			return;
		}

		queueSound(gameState, bubbleSounds::ADD_TERM_SOUND);
	}

	void Substitute::undo(middle::GameState* gameState)
	{
		while (actions.size() > 0) {
			actions.back()->undo(gameState);
			actions.pop_back();
		}
	}


	void InsertAsXOverX::execute(middle::GameState* gameState)
	{

		Vector3 currPos = middle::getGlobalPosition(gameState, newTermId.index);
		middle::moveShape(gameState, newTermId.index, targetPos - currPos);

		middle::Id inverseId = bubbleActions::createInverseReplacementShape(gameState, newTermId);
		middle::moveShape(gameState, inverseId.index, { 1,0,0 });

		auto linkAction = LinkMultiplicationTerm(newTermId, inverseId);
		linkAction.execute(gameState);
		middle::Id mulId = linkAction.resultShapeId;

		// container Bubble
		middle::Shape bubbleProto = bubble::newBubble(gameState, targetPos);
		middle::Shape& bubbleShape = middle::registerShape(gameState, bubbleProto);
		middle::EditorActionReparent(bubbleShape.id.index, mulId.index).execute(gameState);

		auto registerAction = std::make_unique<middle::EditorActionRegisterId>(bubbleShape.id);
		registerAction->execute(gameState);
		actions.push_back(std::move(registerAction));

		middle::Id targetLinkReciever = shapeToAddIntoId;

		// create another container if top dog, beacuse top dog shouldn't be in multiplication
		auto& shapeToAddInto = middle::getShape(gameState, shapeToAddIntoId.index);
		auto topDog = middle::getComponent<components::TopDogBubbleTag>(shapeToAddInto);
		if (topDog) {
			auto newContainerProto = bubble::newBubble(gameState, middle::getGlobalPosition(gameState, shapeToAddInto.id.index));
			auto& newContainerShape = middle::registerShape(gameState, newContainerProto);
			middle::Id addIntoCopy = middle::deepCopyShapeGlobalCoordinates(gameState, shapeToAddInto.id);
			middle::EditorActionReparent(newContainerShape.id.index, addIntoCopy.index).execute(gameState);

			auto registerContainer = std::make_unique<middle::EditorActionRegisterId>(newContainerShape.id);
			registerContainer->execute(gameState);
			actions.push_back(std::move(registerContainer));

			auto replace = std::make_unique<Replace>(targetLinkReciever, newContainerShape.id);
			replace->execute(gameState);
			actions.push_back(std::move(replace));

			targetLinkReciever = addIntoCopy;
		}

		auto linkToReciever = std::make_unique<LinkMultiplicationTerm>(targetLinkReciever, bubbleShape.id);
		linkToReciever->execute(gameState);
		actions.push_back(std::move(linkToReciever));

		queueSound(gameState, bubbleSounds::ADD_TERM_SOUND);
	}

	void InsertAsXOverX::undo(middle::GameState* gameState)
	{
		while (actions.size() > 0) {
			actions.back()->undo(gameState);
			actions.pop_back();
		}
	}

	void InsertAsXMinusX::execute(middle::GameState* gameState)
	{
		auto& shapeToAddInto = middle::getShape(gameState, shapeToAddIntoId.index);

		Vector3 currppos = middle::getGlobalPosition(gameState, newTermId.index);
		middle::moveShape(gameState, newTermId.index, targetPos - currppos);
		middle::Id inverseFriend = createNegatedReplacementShape(gameState, newTermId);
		middle::moveShape(gameState, inverseFriend.index, { 1,0,0 });

		// container Bubble
		middle::Shape bubbleProto = bubble::newBubble(gameState, targetPos);
		middle::Shape& bubbleShape = middle::registerShape(gameState, bubbleProto);
		middle::EditorActionReparent(bubbleShape.id.index, newTermId.index).execute(gameState);
		middle::EditorActionReparent(bubbleShape.id.index, inverseFriend.index).execute(gameState);

		auto registerAction = std::make_unique<middle::EditorActionRegisterId>(bubbleShape.id);
		registerAction->execute(gameState);
		actions.push_back(std::move(registerAction));


		middle::Id containerId = shapeToAddIntoId;

		// containerize these
		auto var = middle::getComponent<components::BubbleVariable>(shapeToAddInto);
		if (var) {
			middle::Id copyId = middle::deepCopyShapeGlobalCoordinates(gameState, shapeToAddIntoId);
			middle::Id replacingId = bubble::containerize(gameState, copyId);

			auto registerAction = std::make_unique<middle::EditorActionRegisterId>(replacingId);
			registerAction->execute(gameState);
			actions.push_back(std::move(registerAction));

			auto replaceAction = std::make_unique<Replace>(containerId, replacingId);
			replaceAction->execute(gameState);
			actions.push_back(std::move(replaceAction));

			containerId = replacingId;
		}

		auto reparent = std::make_unique<middle::EditorActionReparent>(containerId.index, bubbleShape.id.index);
		reparent->execute(gameState);
		actions.push_back(std::move(reparent));

		queueSound(gameState, bubbleSounds::ADD_TERM_SOUND);
	}

	void InsertAsXMinusX::undo(middle::GameState* gameState)
	{
		while (actions.size() > 0) {
			actions.back()->undo(gameState);
			actions.pop_back();
		}
	}


	void CopyAsHelper::execute(middle::GameState* gameState)
	{
		if (!gameState->bubbleAlgebraState.copyNegated) {
			copyShapeId = middle::deepCopyShapeGlobalCoordinates(gameState, shapeToCopyId);
		}
		else {
			copyShapeId = createNegatedReplacementShape(gameState, shapeToCopyId);
		}

		if (gameState->bubbleAlgebraState.copyInverted) {
			middle::Id invertedShapeId = createInverseReplacementShape(gameState, copyShapeId);
			middle::deleteShapeRecursive(gameState, copyShapeId.index);
			copyShapeId = invertedShapeId;
		}
		middle::moveShape(gameState, copyShapeId.index, targetPosition - middle::getGlobalPosition(gameState, copyShapeId.index));
		auto insertable = middle::attachComponent<components::InsertableBubble>(gameState, copyShapeId);
		auto& copyShape = middle::getShape(gameState, copyShapeId.index);
		auto loop = middle::getComponent<components::LoopSociety>(copyShape);
		loop->parentLoopId = middle::Id();
	}

	void CopyAsHelper::undo(middle::GameState* gameState)
	{
		middle::deleteShapeRecursive(gameState, copyShapeId.index);
	}


	void UpdateBubblesMultiplicationIdentity::execute(middle::GameState* gameState)
	{
		assert(bubble::isMultiplication(gameState, mulId));
		std::vector<middle::Id>children;
		middle::getChildren(gameState, mulId, children);
		if (children.size() <= 1) {
			middle::queueComponentDeletion<components::BubbleMultiplyComponent>(gameState, mulId);
			removedMulComp = true;
		}
	}
	void UpdateBubblesMultiplicationIdentity::undo(middle::GameState* gameState)
	{
		if (removedMulComp) {
			middle::attachComponent<components::BubbleMultiplyComponent>(gameState, mulId);
		}
	}

}
