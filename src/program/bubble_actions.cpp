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
#include "ExponentComponent.h"
#include "TopDogBubbleTag.h"
#include "BubbleAlgebraProblem.h"
#include "ProcedureContainer.h"
#include "bubble_constants.h"
#include "HelperBubbleEquation.h"
#include "EditThisTag.h"

namespace bubbleActions {

	bool additiveInverses(middle::GameState* gameState, middle::Id idA, middle::Id idB) {
		auto& shapeA = middle::getShape(gameState, idA.index);
		auto& shapeB = middle::getShape(gameState, idB.index);
		auto unitA = middle::getComponent<components::BubbleUnit>(shapeA);
		auto unitB = middle::getComponent<components::BubbleUnit>(shapeB);
		if (!unitA || !unitB) {
			return false;
		}
		auto variableA = middle::getComponent<components::BubbleVariable>(shapeA);
		auto variableB = middle::getComponent<components::BubbleVariable>(shapeB);
		if (variableA && variableB && variableA->label == variableB->label) {
			return unitA->value + unitB->value == 0;
		}
		else if (!variableA && !variableB) {
			return unitA->value + unitB->value == 0;
		}
		return false;
	}

	bool multiplicativeInverses(middle::GameState* gameState, middle::Id idA, middle::Id idB)
	{
		auto& shapeA = middle::getShape(gameState, idA.index);
		auto& shapeB = middle::getShape(gameState, idB.index);
		auto bubbleA = middle::getComponent<components::BubbleComponent>(shapeA);
		auto bubbleB = middle::getComponent<components::BubbleComponent>(shapeB);
		assert(bubbleA && bubbleB);
		if (bubbleA->inverse != bubbleB->inverse) {
			bool oldInverse = bubbleA->inverse;
			bubbleA->inverse = !oldInverse;
			bool areMatching = bubble::matchingBubbles(gameState, idA, idB);
			bubbleA->inverse = oldInverse;
			return areMatching;
		}
		return false;
	}

	bool validateAdditionInitialState(middle::GameState* gameState, ExecuteAddition* addition) {

		middle::Id& parentAId = middle::getParent(gameState, addition->shapeToAddId);
		middle::Id& parentBId = middle::getParent(gameState, addition->shapeToAddIntoId);
		if (parentAId != parentBId) {
			return false;
		}
		auto& parentShape = middle::getShape(gameState, parentAId.index);
		auto parentMul = middle::getComponent<components::BubbleMultiplyComponent>(parentShape);
		auto parentEquals = middle::getComponent<components::BubbleEqualsComponent>(parentShape);
		if (parentMul || parentEquals) {
			return false;
		}

		return true;
	}

	middle::Id createNegatedReplacementShape(middle::GameState* gameState, middle::Id id) {
		middle::Id copyId = middle::deepCopyShape(gameState, id.index);

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
			auto bubble = middle::getComponent<components::BubbleComponent>(targetShape);
			if (bubble) {
				auto mulOneAction = MulOne(targetId);
				mulOneAction.execute(gameState);
				middle::Id one = mulOneAction.resultShapeId;
				bubble::negate(gameState, one);
			}
			// directly negate units
			else {
				bubble::negate(gameState, targetId);
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



	CreateMulitiplicationReplacementShape::CreateMulitiplicationReplacementShape(middle::Id shapeToReplace, middle::Id replacingShape)
	{
		this->shapeToReplaceId = shapeToReplace;
		this->replacingShapeId = replacingShape;
	}

	void CreateMulitiplicationReplacementShape::execute(middle::GameState* gameState) {
		auto& shapeToReplace = middle::getShape(gameState, shapeToReplaceId.index);
		auto& replacingShape = middle::getShape(gameState, replacingShapeId.index);

		auto pos = middle::getComponent<components::Position>(shapeToReplace);
		Vector3 targetPos = { pos->posX, pos->posY, pos->posZ };

		auto bubbleComp = middle::getComponent<components::BubbleComponent>(shapeToReplace);
		auto mulComp = middle::getComponent<components::BubbleMultiplyComponent>(shapeToReplace);
		auto unit = middle::getComponent<components::BubbleUnit>(shapeToReplace);
		auto variable = middle::getComponent<components::BubbleVariable>(shapeToReplace);
		// containe copies in a new multiplication
		if (bubbleComp) {
			// deep copy to replace and replacing
			middle::Id copyId = middle::deepCopyShape(gameState, replacingShape.id.index);
			middle::Id toReplaceCopyId = middle::deepCopyShape(gameState, shapeToReplaceId.index);

			auto newMulAction = bubbleActions::NewMultiplication(copyId, toReplaceCopyId);
			newMulAction.execute(gameState);

			auto& newMulShape = middle::getShape(gameState, newMulAction.resultShapeId.index);
			auto position = middle::getComponent<components::Position>(newMulShape);

			// compute displacmenet from replacing shape to shapeToReplace position
			Vector3 replacingShapePos = middle::getShapePosition(gameState, newMulShape.id.index);
			Vector3 displacement = targetPos - replacingShapePos;
			position = middle::getComponent<components::Position>(newMulShape);
			position->posX = targetPos.x;
			position->posY = targetPos.y;
			position->posZ = targetPos.z;
			resultShapeId = newMulShape.id;
			return;
		}

		// if shape to replace is already a multiplicaiton
		if (mulComp) {
			middle::Id& copyMulShapeId = middle::deepCopyShape(gameState, shapeToReplace.id.index);
			auto& copyMulShape = middle::getShape(gameState, copyMulShapeId.index);
			middle::Id copyId = middle::deepCopyShape(gameState, replacingShape.id.index, copyMulShape.id.index);
			auto copyMulLoop = middle::getComponent<components::LoopSociety>(copyMulShape);
			copyMulLoop->loopMemberIds.push_back(copyId);
			// compute displacmenet from replacing shape to shapeToReplace position
			Vector3 replacingShapePos = middle::getShapePosition(gameState, copyMulShape.id.index);
			Vector3 displacement = targetPos - replacingShapePos;
			middle::moveShape(gameState, copyMulShape.id.index, displacement);
			resultShapeId = copyMulShape.id;
			return;
		}

		// if shape to replace is a unit
		else if (unit)
		{
			middle::Id shapeToCopyId = replacingShape.id;
			if (unit->value == 0) {
				shapeToCopyId = shapeToReplace.id;
			}

			middle::Id copyId = middle::deepCopyShape(gameState, shapeToCopyId.index);
			// 
			unit = middle::getComponent<components::BubbleUnit>(shapeToReplace);

			if (unit->value == -1) {
				middle::Id negativeCopyId = createNegatedReplacementShape(gameState, copyId);
				Replace(copyId, negativeCopyId).execute(gameState);
				copyId = negativeCopyId;
			}

			// compute displacmenet from replacing shape to shapeToReplace position
			Vector3 replacingShapePos = middle::getShapePosition(gameState, copyId.index);
			Vector3 displacement = targetPos - replacingShapePos;
			middle::moveShape(gameState, copyId.index, displacement);
			resultShapeId = copyId;
			return;
		}


	}

	void CreateMulitiplicationReplacementShape::undo(middle::GameState* gameState)
	{
		deleteShapeRecursive(gameState, resultShapeId.index, true);
	}




	CreateAdditionReplacementShape::CreateAdditionReplacementShape(middle::Id idA, middle::Id idB) {
		this->idA = idA;
		this->idB = idB;
	}

	void CreateAdditionReplacementShape::execute(middle::GameState* gameState) {
		auto& shapeA = middle::getShape(gameState, idA.index);
		auto& shapeB = middle::getShape(gameState, idB.index);

		auto unitA = middle::getComponent<components::BubbleUnit>(shapeA);
		auto unitB = middle::getComponent<components::BubbleUnit>(shapeB);
		auto bubbleA = middle::getComponent<components::BubbleComponent>(shapeA);
		auto bubbleB = middle::getComponent<components::BubbleComponent>(shapeB);
		auto rootA = middle::getComponent<components::ExponentComponent>(shapeA);
		auto rootB = middle::getComponent<components::ExponentComponent>(shapeB);

		middle::Id replacementId;

		// note same scale fractions are handled separatedly

		// NEW CONTAINING BUBBLE CASE
		if ((unitA && unitB) || (rootA || rootB) || (bubbleA && bubbleB)) {
			Vector3 targetPos = middle::getShapePosition(gameState, idA.index);
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

		resultId = replacementId;
	}

	void CreateAdditionReplacementShape::undo(middle::GameState* gameState) {
		deleteShapeRecursive(gameState, resultId.index, true);
	}



	ExecuteMultiplication::ExecuteMultiplication(middle::Id shapeToCopyId, middle::Id shapeToCopyIntoId) {
		this->shapeToCopyId = shapeToCopyId;
		this->shapeToCopyIntoId = shapeToCopyIntoId;
	}

	bool isPowerBubble(middle::GameState* gameState, middle::Id id){
		std::vector<middle::Id>children;
		middle::getChildren(gameState, id, children);
		if (children.size() != 1) {
			return false;
		}
		auto& shape = middle::getShape(gameState, children[0].index);
		auto mulComp = middle::getComponent<components::BubbleMultiplyComponent>(shape);
		if (!mulComp) {
			return false;
		}
		return mulComp->operationType == static_cast<int>(components::OperationType::POWER);
	}

	void ExecuteMultiplication::execute(middle::GameState* gameState) {

		// cancel if trying to expand into variable or expanding into power bubble
		auto& shapeToAddInto = middle::getShape(gameState, shapeToCopyIntoId.index);
		auto varComp = middle::getComponent<components::BubbleVariable>(shapeToAddInto);
		if (varComp || isPowerBubble(gameState, shapeToCopyIntoId)) {
			cancelled = true;
			return;
		}

		middle::Id idA = middle::deepCopyShape(gameState, shapeToCopyId.index);
		middle::Id idB = middle::deepCopyShape(gameState, shapeToCopyIntoId.index);

		auto registerResult = std::make_unique<middle::EditorActionRegisterId>(idB);
		registerResult->execute(gameState);
		actions.push_back(std::move(registerResult));

		// if shape to add into is inverse, invert the input 
		auto bub = middle::getComponent<components::BubbleComponent>(shapeToAddInto);
		if (bub->inverse) {
			bubble::invert(gameState, idA);
		}
		// if shape to add into is exponent the inputs exponent inverts and has same sign
		auto exp = middle::getComponent<components::ExponentComponent>(shapeToAddInto);
		if (exp) {
			int power = exp->power;
			bool isInverse = exp->isInverse;
			bool isNegative = exp->isNegative;
			idA = bubble::containerize(gameState, idA);
			auto newExp = middle::attachComponent<components::ExponentComponent>(gameState, idA);
			newExp->power = power;
			newExp->isInverse = !isInverse;
			newExp->isNegative = isNegative;
		}

		std::vector<middle::Id>children;
		middle::getChildren(gameState, idB, children);

		// create replacements to the positions of the old children
		for (int i = 0; i < children.size(); ++i) {
			middle::Id& childId = children[i];

			// replace unit with new copy bubble
			auto createMulAction = std::make_unique<bubbleActions::CreateMulitiplicationReplacementShape>(childId, idA);
			createMulAction->execute(gameState);
			middle::Id copyId = createMulAction->resultShapeId;
			actions.push_back(std::move(createMulAction));

			auto replaceAction = std::make_unique<bubbleActions::ReplaceBubbleAndTransferTags>(childId, copyId);
			replaceAction->execute(gameState);
			actions.push_back(std::move(replaceAction));
		}

		middle::deleteShapeRecursive(gameState, idA.index);

		auto deleteAction2 = std::make_unique<middle::EditorActionDeleteSingle>(shapeToCopyId);
		deleteAction2->execute(gameState);
		actions.push_back(std::move(deleteAction2));

		resultShapeId = idB;

		auto replace = std::make_unique<bubbleActions::ReplaceBubbleAndTransferTags>(shapeToCopyIntoId, resultShapeId);
		replace->execute(gameState);
		actions.push_back(std::move(replace));


		// delete multiplication shape if children size < 2
		middle::Id parentId = middle::getParent(gameState, resultShapeId);
		if (parentId.index != middle::UNASSIGNED) {
			auto& parentShape = middle::getShape(gameState, parentId.index);
			std::vector<middle::Id>children;
			middle::getChildren(gameState, parentId, children);
			middle::Id parentsParentId = middle::getParent(gameState, parentId);
			// reparent to parents parent (if it exists)
			if (children.size() == 1 && parentsParentId.index != middle::UNASSIGNED) {
				auto reparent = std::make_unique<middle::EditorActionReparent>(parentsParentId.index, children[0].index);
				reparent->execute(gameState);
				actions.push_back(std::move(reparent));
			}
			// (if it doesn't exist) remove from the current parent to prevent it being deleted when parent is deleted
			else if (children.size() == 1) {
				auto removeFromLoop = std::make_unique<middle::EditorActionRemoveFromLoop>(children[0].index);
				removeFromLoop->execute(gameState);
				actions.push_back(std::move(removeFromLoop));
			}
			// delete parent, which is a multiplication with less than 2 values
			if (children.size() < 2) {
				auto deleteAction = std::make_unique<middle::EditorActionDeleteSingle>(parentId);
				deleteAction->execute(gameState);
				actions.push_back(std::move(deleteAction));
			}
		}

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
		auto additionAction = std::make_unique<CreateAdditionReplacementShape>(idA, idB);
		additionAction->execute(gameState);
		resultShapeId = additionAction->resultId;
		actions.push_back(std::move(additionAction));

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


	void ExecutePowerNew::execute(middle::GameState* gameState)
	{
		auto shape = middle::getShape(gameState, powerShapeId.index);
		auto operationComp = middle::getComponent<components::BubbleMultiplyComponent>(shape);
		assert(operationComp);
		assert(operationComp->operationType == static_cast<int>(components::OperationType::POWER));
		std::vector<middle::Id>children;
		middle::getChildren(gameState, shape.id, children);
		assert(children.size() == 2);

		// create replacement shape
		// create container
		std::vector<middle::Id>exponentChildren;
		middle::Id exponentId = children[1];
		middle::Id baseId = children[0];
		middle::getChildren(gameState, exponentId, exponentChildren);
		Vector3 basePos = middle::getShapePosition(gameState, baseId.index);

		middle::Id prevId;
		for (middle::Id& id : exponentChildren) {
			if (bubble::getStructureType(gameState, id) == components::AlgebraNodeType::UNIT) {
				middle::Id copyId = middle::deepCopyShape(gameState, baseId.index);
				middle::moveShape(gameState, copyId.index, middle::getShapePosition(gameState, id.index) - basePos);
				if (prevId.index != middle::UNASSIGNED) {
					LinkMultiplicationTerm(prevId, copyId).execute(gameState);
				}
				prevId = copyId;
			}
		}
		middle::Id operationId = middle::getParent(gameState, prevId);

		auto registerId = std::make_unique<middle::EditorActionRegisterId>(operationId);
		registerId->execute(gameState);
		actions.push_back(std::move(registerId));

		auto replace = std::make_unique<ReplaceBubbleAndTransferTags>(powerShapeId, operationId);
		replace->execute(gameState);
		actions.push_back(std::move(replace));
	}

	void ExecutePowerNew::undo(middle::GameState* gameState)
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
		middle::Id containerCopy = middle::copyShape(gameState, containerId.index);
		auto& newContainerShape = middle::getShape(gameState, containerCopy.index);
		auto var = middle::getComponent<components::BubbleVariable>(toPopShape);
		if (var) {
			auto newVar = middle::attachComponent<components::BubbleVariable>(gameState, newContainerShape.id);
			newVar->isNegative = var->isNegative;
			newVar->label = var->label;
		}
		auto loop = middle::getComponent<components::LoopSociety>(newContainerShape);
		loop->parentLoopId = middle::Id();
		loop->loopMemberIds.clear();

		return newContainerShape.id;
	}

	void Pop::execute(middle::GameState* gameState) {
		middle::Shape& shapeToPop = middle::getShape(gameState, id.index);
		// check that there is a parent
		auto bubble = middle::getComponent<components::BubbleComponent>(shapeToPop);
		auto exp = middle::getComponent<components::ExponentComponent>(shapeToPop);
		auto variable = middle::getComponent<components::BubbleVariable>(shapeToPop);
		middle::Id parentId = middle::getParent(gameState, shapeToPop.id);
		if (parentId.index == middle::UNASSIGNED) {
			cancelled = true;
			return;
		}
		if (!bubble) {
			cancelled = true;
			return;
		}
		if (exp) {
			cancelled = true;
			return;
		}
		if (isPowerBubble(gameState, shapeToPop.id)) {
			cancelled = true;
			return;
		}

		std::vector<middle::Id>children;
		middle::getChildren(gameState, parentId, children);
		int siblingCount = children.size() - 1;

		if (siblingCount != 0) {
			if (variable) {
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
		if (bubble->inverse) {
			cancelled = true;
			return;
		}

		if (variable) {
			Vector3 targetPos = middle::getShapePosition(gameState, parentId.index);

			middle::Id varPopReplacementId = popReplacement(gameState, parentId, shapeToPop.id);
			auto registerAction = std::make_unique <middle::EditorActionRegisterId>(varPopReplacementId);
			registerAction->execute(gameState);
			actions.push_back(std::move(registerAction));

			auto replace = std::make_unique<ReplaceBubbleAndTransferTags>(parentId, varPopReplacementId);
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
			int oldIndex = middle::UNASSIGNED;
			for (int i = 0; i < children.size(); ++i) {
				if (children[i] == shapeToReplaceId) {
					oldIndex = i;
				}
			}
			assert(oldIndex != middle::UNASSIGNED);
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


	void ReplaceBubbleAndTransferTags::execute(middle::GameState* gameState)
	{
		auto& shapeToReplace = middle::getShape(gameState, shapeToReplaceId.index);
		bool isTopDog = middle::getComponent<components::TopDogBubbleTag>(shapeToReplace) != nullptr;
		auto algProb = middle::getComponent<components::BubbleAlgebraProblem>(shapeToReplace);
		auto helper = middle::getComponent<components::HelperBubbleEquation>(shapeToReplace);
		auto editThisTag = middle::getComponent<components::EditThisTag>(shapeToReplace);
		bool isEditable = false;
		if (algProb) {
			isEditable = algProb->editable;
		}

		auto replaceAction = std::make_unique<Replace>(shapeToReplace.id, replacingShapeId);
		replaceAction->execute(gameState);
		actions.push_back(std::move(replaceAction));

		if (isTopDog) {
			middle::attachComponent<components::TopDogBubbleTag>(gameState, replacingShapeId);
		}
		if (algProb) {
			auto newComp = middle::attachComponent<components::BubbleAlgebraProblem>(gameState, replacingShapeId);
			newComp->editable = isEditable;
		}
		if (helper) {
			middle::attachComponent<components::HelperBubbleEquation>(gameState, replacingShapeId);
		}
		if (editThisTag) {
			middle::attachComponent<components::EditThisTag>(gameState, replacingShapeId);
		}
	}

	void ReplaceBubbleAndTransferTags::undo(middle::GameState* gameState)
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
		middle::Id& parentId = middle::getParent(gameState, recieverShapeId);
		if (parentId.index != middle::UNASSIGNED) {
			auto& parent = middle::getShape(gameState, parentId.index);
			auto mulComp = middle::getComponent<components::BubbleMultiplyComponent>(parent);
			if (mulComp) {
				auto reparent = std::make_unique<middle::EditorActionReparent>(parent.id.index, linkingShapeId.index);
				reparent->execute(gameState);
				actions.push_back(std::move(reparent));

				resultShapeId = parent.id;
				return;
			}
		}


		// else create new multiplication
		auto newMul = std::make_unique<NewMultiplication>(recieverShapeId, linkingShapeId);
		newMul->execute(gameState);
		middle::Id newMulId = newMul->resultShapeId;
		actions.push_back(std::move(newMul));

		if (parentId.index != middle::UNASSIGNED) {
			auto reparent = std::make_unique<middle::EditorActionReparent>(parentId.index, newMulId.index);
			reparent->execute(gameState);
			actions.push_back(std::move(reparent));

		}
		resultShapeId = newMulId;
	}

	void LinkMultiplicationTerm::undo(middle::GameState* gameState)
	{
		while (actions.size() > 0) {
			actions.back()->undo(gameState);
			actions.pop_back();
		}
	}

	void UnlinkMultiplicationTerm::execute(middle::GameState* gameState)
	{
		middle::Id mulParentId = middle::getParent(gameState, multiplicationId);

		auto removeAction = std::make_unique<middle::EditorActionReparent>(mulParentId.index, unlinkingShapeId.index);
		removeAction->execute(gameState);
		actions.push_back(std::move(removeAction));

		std::vector<middle::Id>children;
		middle::getChildren(gameState, multiplicationId, children);
		resultShapeId = multiplicationId;
		if (children.size() == 1) {
			auto reparent = std::make_unique<middle::EditorActionReparent>(mulParentId.index, children[0].index);
			reparent->execute(gameState);
			actions.push_back(std::move(reparent));
			auto deleteMul = std::make_unique<middle::EditorActionDeleteSingle>(multiplicationId);
			deleteMul->execute(gameState);
			actions.push_back(std::move(deleteMul));
			resultShapeId = children[0];
		}
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
		Vector3 targetPos = middle::getShapePosition(gameState, unitShape.id.index);
		middle::Shape containerBubbleProto = bubble::newBubble(gameState, targetPos);
		auto registerAction = std::make_unique<middle::EditorActionRegisterShape>(containerBubbleProto);
		registerAction->execute(gameState);
		middle::Id newContainerId = registerAction->newShapeId;
		actions.push_back(std::move(registerAction));

		for (int i = 0; i < dividend; ++i) {

			// create and register inverse bubble
			Vector3 targetPos = middle::getShapePosition(gameState, unitShape.id.index);
			middle::Shape containerBubbleProto = bubble::newBubble(gameState, targetPos + Vector3{ 1.0f * i, 0,0 });
			middle::Shape& containerBubble = middle::registerShape(gameState, containerBubbleProto);
			auto bubble = middle::getComponent<components::BubbleComponent>(containerBubble);
			bubble->inverse = true;

			// create units 
			for (int j = 0; j < dividend; ++j) {
				middle::Id& copyId = middle::deepCopyShape(gameState, unitShape.id.index);
				middle::EditorActionReparent(containerBubble.id.index, copyId.index).execute(gameState);
			}

			middle::EditorActionReparent(newContainerId.index, containerBubble.id.index).execute(gameState);
		}

		auto replace = std::make_unique<ReplaceBubbleAndTransferTags>(unitShapeId, newContainerId);
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

	Compress::Compress(middle::Id containerShape, bool compressToExponent)
	{
		this->commonFactorId = containerShape;
		this->compressToExponent = compressToExponent;
	}

	middle::Id compressToPower(middle::GameState* gameState, middle::Id compressTargetId, middle::Id commonFactorId) {
		auto& multiplicationShape = middle::getShape(gameState, compressTargetId.index);
		assert(middle::getComponent<components::BubbleMultiplyComponent>(multiplicationShape));
		std::vector<middle::Id>children;
		middle::getChildren(gameState, multiplicationShape.id, children);

		int compressableCount = 0;
		bool compressToNegative = false;
		for (int i = 0; i < children.size(); ++i) {
			middle::Id memberId = children[i];
		}

		// check that all equal
		for (int i = 0; i < children.size(); ++i) {
			middle::Id memberId = children[i];
			// ones are ignored, don't affect multiplication value
			if (!bubble::matchingBubbles(gameState, memberId, commonFactorId)) {
				// return -1 if can't compress
				return middle::Id();
			}
			++compressableCount;
		}

		middle::Id baseId = middle::deepCopyShape(gameState, commonFactorId.index);
		Vector3 targetPos = middle::getShapePosition(gameState, baseId.index);

		middle::Shape newBubbleProto = bubble::newBubble(gameState, targetPos);
		middle::Shape& containerBubble = middle::registerShape(gameState, newBubbleProto);

		middle::Shape exponentProto = bubble::newBubble(gameState, targetPos);
		middle::Shape& exponentShape = middle::registerShape(gameState, exponentProto);

		for (middle::Id& memberId : children) {
			Vector3 unitPos = middle::getShapePosition(gameState, memberId.index);
			middle::Shape unitProto = bubble::newUnit(gameState, unitPos);
			middle::Shape& expUnit = middle::registerShape(gameState, unitProto);
			middle::EditorActionReparent(exponentShape.id.index, expUnit.id.index).execute(gameState);
		}

		LinkMultiplicationTerm linkAction(baseId, exponentShape.id);
		linkAction.execute(gameState);
		middle::Id operationId = linkAction.resultShapeId;
		auto operationShape = middle::getShape(gameState, operationId.index);
		auto operationComp = middle::getComponent<components::BubbleMultiplyComponent>(operationShape);
		operationComp->operationType = static_cast<int>(components::OperationType::POWER);

		middle::EditorActionReparent(containerBubble.id.index, operationId.index).execute(gameState);

		return containerBubble.id;
	}

	struct RepresentativeGroup {
		middle::Id containerId;
		std::vector<middle::Id>representatives;
		int commonIndex = -1;
	};

	bool groupContains(middle::GameState* gameState, RepresentativeGroup& group, middle::Id representativeId) {
		for (int i = 0; i < group.representatives.size(); ++i) {
			middle::Id& id = group.representatives[i];
			if (bubble::matchingBubbles(gameState, id, representativeId)) {
				group.commonIndex = i;
				return true;
			}
		}
		return false;
	}

	struct CommonVariableResult {
		bool commonVariableFound = false;
		middle::Id commonVariableId;
		// indexes where the common variable can be found in multiplications
		std::vector<RepresentativeGroup>groups;
	};

	CommonVariableResult getCommonVariableGroups(middle::GameState* gameState, middle::Id bubbleId, middle::Id commonId) {
		std::vector<RepresentativeGroup>groups;
		std::vector<middle::Id>children;
		middle::getChildren(gameState, bubbleId, children);
		for (middle::Id& childId : children) {
			auto& childShape = middle::getShape(gameState, childId.index);
			auto bubbleComp = middle::getComponent<components::BubbleComponent>(childShape);
			auto unitComp = middle::getComponent<components::BubbleUnit>(childShape);

			if (unitComp) {
				RepresentativeGroup group;
				group.containerId = childId;
				group.representatives.push_back(childId);
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
		}

		if (groups.size() < 1) {
			return CommonVariableResult();
		}

		// check that all the other groups contain a similar representative to one of representatives of the first group
		bool foundCommonInAllGroups = true;
		for (int i = 0; i < groups.size(); ++i) {
			if (!groupContains(gameState, groups[i], commonId)) {
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

	middle::Id compressToMultiplication(middle::GameState* gameState, middle::Id compressTargetId, middle::Id commonFactorId) {

		CommonVariableResult commonVariableResult = getCommonVariableGroups(gameState, compressTargetId, commonFactorId);
		if (!commonVariableResult.commonVariableFound) {
			return middle::Id();
		}

		auto& shapeToCompress = middle::getShape(gameState, compressTargetId.index);
		auto bubble1 = middle::getComponent<components::BubbleComponent>(shapeToCompress);
		bool isInverse = bubble1->inverse;
		auto exp = middle::getComponent<components::ExponentComponent>(shapeToCompress);
		bool isExp = exp != nullptr;

		Vector3 targetPos = middle::getShapePosition(gameState, compressTargetId.index);

		middle::Id commonCopyId = middle::deepCopyShape(gameState, commonFactorId.index);
		auto& commonShape = middle::getShape(gameState, commonCopyId.index);
		if (middle::getComponent<components::BubbleUnit>(commonShape)) {
			middle::Shape bubbleProto = bubble::newBubble(gameState, targetPos);
			middle::Shape& newBubbleShape = middle::registerShape(gameState, bubbleProto);
			middle::EditorActionReparent(newBubbleShape.id.index, commonCopyId.index).execute(gameState);
			commonCopyId = newBubbleShape.id;
		}

		middle::Id shapeToCompressShallowCopy = middle::copyShape(gameState, shapeToCompress.id.index);
		middle::Shape& compressedBubble = getShape(gameState, shapeToCompressShallowCopy.index);
		// clear loop
		auto loop = middle::getComponent<components::LoopSociety>(compressedBubble);
		loop->loopMemberIds.clear();
		loop->parentLoopId = middle::Id();

		LinkMultiplicationTerm(commonCopyId, compressedBubble.id).execute(gameState);

		for (RepresentativeGroup& group : commonVariableResult.groups) {
			// assume singular thing, it will be replaced with bubble containing unit one
			if (group.representatives.size() == 1) {
				targetPos.x += 1;
				middle::Shape unitProto = bubble::newUnit(gameState, targetPos);
				auto& newUnit = middle::registerShape(gameState, unitProto);
				auto unit = middle::getComponent<components::BubbleUnit>(newUnit);
				middle::EditorActionReparent(compressedBubble.id.index, newUnit.id.index).execute(gameState);
			}
			//assume is multiplication, it will be replaced with unlinked version of itself
			else {
				middle::Id copyMulId = middle::deepCopyShape(gameState, group.containerId.index);
				std::vector<middle::Id>copyChildren;
				middle::getChildren(gameState, copyMulId, copyChildren);
				middle::Id unlinkingId = copyChildren[group.commonIndex];
				auto unlink = UnlinkMultiplicationTerm(copyMulId, unlinkingId);
				unlink.execute(gameState);
				middle::EditorActionReparent(compressedBubble.id.index, unlink.resultShapeId.index).execute(gameState);
				middle::deleteShapeRecursive(gameState, unlinkingId.index);
			}
		}

		middle::Id parentMul = middle::getParent(gameState, commonCopyId);

		// if inverse, invert the things
		if (isInverse) {
			bubble::invert(gameState, commonCopyId);
		}
		if (isExp) {
			middle::Id newContainerId = bubble::containerize(gameState, commonShape.id);
			middle::EditorActionReparent(parentMul.index, newContainerId.index).execute(gameState);
			auto newExp = middle::attachComponent<components::ExponentComponent>(gameState, newContainerId);
			exp = middle::getComponent<components::ExponentComponent>(shapeToCompress);
			newExp->power = exp->power;
			newExp->isInverse = exp->isInverse;
		}

		return parentMul;
	}

	void Compress::execute(middle::GameState* gameState)
	{
		auto& shape = middle::getShape(gameState, commonFactorId.index);
		middle::Id replacementShapeId;

		middle::Id compressTargetId = middle::getParent(gameState, commonFactorId);
		if (compressTargetId.index == middle::UNASSIGNED || !middle::getComponent<components::BubbleComponent>(shape)) {
			cancelled = true;
			return;
		}

		std::vector<middle::Id>candidateChildren;
		middle::getChildren(gameState, compressTargetId, candidateChildren);
		int childCount = candidateChildren.size();

		if (childCount == 0) {
			cancelled = true;
			return;
		}

		middle::Id referenceId = candidateChildren[0];

		// link to outer multiplication if multiplication is there is
		// otherwise just replace
		// but also if parent is top dog we need to containerize...

		auto& parentShape = middle::getShape(gameState, compressTargetId.index);
		auto mul = middle::getComponent<components::BubbleMultiplyComponent>(parentShape);

		if (compressToExponent && mul) {
			replacementShapeId = compressToPower(gameState, compressTargetId, commonFactorId);
		}
		else {
			if (mul) {
				compressTargetId = getParent(gameState, compressTargetId);
			}
			if (compressTargetId.index == middle::UNASSIGNED) {
				cancelled = true;
				return;
			}
			replacementShapeId = compressToMultiplication(gameState, compressTargetId, commonFactorId);
		}

		if (replacementShapeId.index == middle::UNASSIGNED) {
			cancelled = true;
			return;
		}

		bool isMul = false;
		middle::Id containerOfContainersId = middle::getParent(gameState, compressTargetId);
		if (containerOfContainersId.index != middle::UNASSIGNED) {
			auto& containerOfCotnainersShape = middle::getShape(gameState, containerOfContainersId.index);
			isMul = middle::getComponent<components::BubbleMultiplyComponent>(containerOfCotnainersShape) != nullptr;
		}
		// if is mul link to the mul..
		if (isMul) {
			std::vector<middle::Id>replacementChildren;
			middle::getChildren(gameState, replacementShapeId, replacementChildren);
			auto repShape = middle::getShape(gameState, replacementShapeId.index);
			assert(middle::getComponent<components::BubbleMultiplyComponent>(repShape));

			for (int i = 0; i < replacementChildren.size(); ++i) {
				auto& childId = replacementChildren[i];
				auto registerAction = std::make_unique<middle::EditorActionRegisterId>(childId);
				registerAction->execute(gameState);
				actions.push_back(std::move(registerAction));
				auto childShape = middle::getShape(gameState, childId.index);
				assert(middle::getComponent<components::BubbleComponent>(childShape));
				assert(!middle::getComponent<components::BubbleMultiplyComponent>(childShape));

				if (i == 0) {
					auto replaceAction = std::make_unique<ReplaceBubbleAndTransferTags>(compressTargetId, replacementChildren[0]);
					replaceAction->execute(gameState);
					actions.push_back(std::move(replaceAction));
					// first one replaced the thign
					continue;
				}
				auto reparent = std::make_unique<middle::EditorActionReparent>(containerOfContainersId.index, childId.index);
				reparent->execute(gameState);
				actions.push_back(std::move(reparent));
			}

			auto delAction = std::make_unique<middle::EditorActionDeleteSingle>(replacementShapeId);
			delAction->execute(gameState);
			actions.push_back(std::move(delAction));
		}
		else {
			auto& shape = middle::getShape(gameState, compressTargetId.index);
			bool isTopDog = middle::getComponent<components::TopDogBubbleTag>(shape);
			if (isTopDog) {
				replacementShapeId = bubble::containerize(gameState, replacementShapeId);
			}

			auto registerAction = std::make_unique<middle::EditorActionRegisterId>(replacementShapeId);
			registerAction->execute(gameState);
			actions.push_back(std::move(registerAction));

			auto replace = std::make_unique<ReplaceBubbleAndTransferTags>(compressTargetId, replacementShapeId);
			replace->execute(gameState);
			actions.push_back(std::move(replace));

			resultShapeId = replacementShapeId;
		}

		queueSound(gameState, bubbleSounds::COMPRESS_SOUND);
	}

	void Compress::undo(middle::GameState* gameState)
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
		if (middle::getComponent<components::ExponentComponent>(shape)) {
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
		Vector3 targetPos = middle::getShapePosition(gameState, recieverShapeId.index);
		middle::Shape newBubbleProto = bubble::newBubble(gameState, targetPos + Vector3{ 1,0,0 });
		middle::Shape newUnitProto = bubble::newUnit(gameState, targetPos);
		auto register1 = std::make_unique<middle::EditorActionRegisterShape>(newBubbleProto);
		register1->execute(gameState);
		middle::Id newBubbleId = register1->newShapeId;
		actions.push_back(std::move(register1));
		auto register2 = std::make_unique<middle::EditorActionRegisterShape>(newUnitProto);
		register2->execute(gameState);
		middle::Id newUnitId = register2->newShapeId;
		actions.push_back(std::move(register2));

		auto reparent = std::make_unique<middle::EditorActionReparent>(newBubbleId.index, newUnitId.index);
		reparent->execute(gameState);
		actions.push_back(std::move(reparent));
		auto link = std::make_unique<LinkMultiplicationTerm>(recieverShapeId, newBubbleId);
		link->execute(gameState);
		actions.push_back(std::move(link));

		resultShapeId = newBubbleId;

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
		auto update = UpdateVariable(label, [oldRef]() {return oldRef;});
		update.execute(gameState);
	}

	void NewMultiplication::execute(middle::GameState* gameState)
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
		resultShapeId = newMulShape.id;
	}

	void NewMultiplication::undo(middle::GameState* gameState)
	{
		while (actions.size() > 0) {
			actions.back()->undo(gameState);
			actions.pop_back();
		}
	}

	bool validateNewTermState(middle::GameState* gameState, middle::Id& shapeToAddIntoId, middle::Id& termId) {
		return true;
	}



	void NewAdditionTerm::execute(middle::GameState* gameState)
	{
		middle::Id topDog = bubble::findCompFromParents<components::TopDogBubbleTag>(gameState, shapeToAddIntoId);
		shapeToAddIntoId = topDog;

		middle::Id parentId = middle::getParent(gameState, shapeToAddIntoId);
		std::vector<middle::Id>shapesToAddIntoIds;

		std::vector<middle::Id>shapesToAddIds;
		// if no parent, add just the og shapeToAddIntoId
		if (parentId.index == middle::UNASSIGNED) {
			shapesToAddIntoIds.push_back(shapeToAddIntoId);
			shapesToAddIds.push_back(newTermId);
		}
		// else we check that the parent has equals component, then we add all the children of it as shapes to add into
		else {
			auto& parentShape = middle::getShape(gameState, parentId.index);
			auto equalsComp = middle::getComponent<components::BubbleEqualsComponent>(parentShape);
			if (!equalsComp) {
				cancelled = true;
				return;
			}
			shapesToAddIds.push_back(newTermId);
			middle::getChildren(gameState, parentShape.id, shapesToAddIntoIds);
			// copy the new terms to be added to the other containers
			for (int i = 1; i < shapesToAddIntoIds.size(); ++i) {
				auto copyAction = std::make_unique<middle::EditorActionCopySingle>(newTermId);
				copyAction->execute(gameState);
				shapesToAddIds.push_back(copyAction->resultId);
				actions.push_back(std::move(copyAction));
			}
		}


		assert(shapesToAddIntoIds.size() == shapesToAddIds.size());
		for (int i = 0; i < shapesToAddIntoIds.size(); ++i) {
			middle::Id& intoId = shapesToAddIntoIds[i];

			// if its a single variable, or exponent we need to containerize first
			auto& shapeToAddInto = middle::getShape(gameState, intoId.index);
			auto var = middle::getComponent<components::BubbleVariable>(shapeToAddInto);
			auto exp = middle::getComponent<components::ExponentComponent>(shapeToAddInto);
			if (var || exp) {
				middle::Id copyId = middle::deepCopyShape(gameState, shapeToAddIntoId.index);
				middle::Id replacingId = bubble::containerize(gameState, copyId);

				auto registerAction = std::make_unique<middle::EditorActionRegisterId>(replacingId);
				registerAction->execute(gameState);
				actions.push_back(std::move(registerAction));

				auto replaceAction = std::make_unique<ReplaceBubbleAndTransferTags>(intoId, replacingId);
				replaceAction->execute(gameState);
				actions.push_back(std::move(replaceAction));

				intoId = replacingId;
			}

			middle::Id& toAddId = shapesToAddIds[i];
			auto reparentAction = std::make_unique<middle::EditorActionReparent>(intoId.index, toAddId.index);
			reparentAction->execute(gameState);
			Vector3 currentPos = middle::getShapePosition(gameState, toAddId.index);
			middle::moveShape(gameState, toAddId.index, targetPosition - currentPos);
			actions.push_back(std::move(reparentAction));
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
		middle::Id topDog = bubble::findCompFromParents<components::TopDogBubbleTag>(gameState, shapeToAddIntoId);
		shapeToAddIntoId = topDog;

		middle::Id parentId = middle::getParent(gameState, shapeToAddIntoId);
		std::vector<middle::Id>shapesToAddIntoIds;
		std::vector<middle::Id>shapesToAddIds;
		// if no parent, add just the og shapeToAddIntoId
		if (parentId.index == middle::UNASSIGNED) {
			shapesToAddIntoIds.push_back(shapeToAddIntoId);
			shapesToAddIds.push_back(newTermId);
		}
		// else we check that the parent has equals component, then we add all the children of it as shapes to add into
		else {
			auto& parentShape = middle::getShape(gameState, parentId.index);
			auto equalsComp = middle::getComponent<components::BubbleEqualsComponent>(parentShape);
			if (!equalsComp) {
				cancelled = true;
				return;
			}
			shapesToAddIds.push_back(newTermId);
			middle::getChildren(gameState, parentShape.id, shapesToAddIntoIds);
			// copy the new terms to be added to the other containers
			for (int i = 1; i < shapesToAddIntoIds.size(); ++i) {
				auto copyAction = std::make_unique<middle::EditorActionCopySingle>(newTermId);
				copyAction->execute(gameState);
				shapesToAddIds.push_back(copyAction->resultId);
				actions.push_back(std::move(copyAction));
			}
		}

		for (int i = 0; i < shapesToAddIntoIds.size(); ++i) {
			middle::Id containerId = shapesToAddIntoIds[i];
			middle::Id toAddId = shapesToAddIds[i];

			// if its a single variable, or exponent we need to containerize first
			auto& shapeToAddInto = middle::getShape(gameState, containerId.index);
			auto var = middle::getComponent<components::BubbleVariable>(shapeToAddInto);
			auto exp = middle::getComponent<components::ExponentComponent>(shapeToAddInto);
			if (var || exp) {
				middle::Id copyId = middle::deepCopyShape(gameState, shapeToAddIntoId.index);
				middle::Id replacingId = bubble::containerize(gameState, copyId);

				auto registerAction = std::make_unique<middle::EditorActionRegisterId>(replacingId);
				registerAction->execute(gameState);
				actions.push_back(std::move(registerAction));

				auto replaceAction = std::make_unique<ReplaceBubbleAndTransferTags>(containerId, replacingId);
				replaceAction->execute(gameState);
				actions.push_back(std::move(replaceAction));

				containerId = replacingId;
			}


			std::vector<middle::Id>children;
			middle::getChildren(gameState, containerId, children);
			if (children.size() == 0) {
				return;
			}
			middle::Id toLinkIntoId;

			if (children.size() == 1) {
				std::vector<middle::Id>multiplicationMembers;
				auto& child = middle::getShape(gameState, children[0].index);
				auto mul = middle::getComponent<components::BubbleMultiplyComponent>(child);
				// if mul link to one of multiplications members
				if (mul) {
					middle::getChildren(gameState, children[0], multiplicationMembers);
					assert(multiplicationMembers.size() >= 2);
					toLinkIntoId = multiplicationMembers[0];
				}
				else {
					toLinkIntoId = child.id;
				}
			}
			// create new container bubble, because the top dog bubble should be alone
			else if (children.size() > 1) {
				middle::Shape newContainer = bubble::newBubble(gameState, middle::getShapePosition(gameState, containerId.index));
				auto registerAction = std::make_unique<middle::EditorActionRegisterShape>(newContainer);
				registerAction->execute(gameState);
				toLinkIntoId = registerAction->newShapeId;
				actions.push_back(std::move(registerAction));
				for (middle::Id childId : children) {
					auto reparentAction = std::make_unique<middle::EditorActionReparent>(toLinkIntoId.index, childId.index);
					reparentAction->execute(gameState);
					actions.push_back(std::move(reparentAction));
				}

				auto reparentNewBubble = std::make_unique<middle::EditorActionReparent>(containerId.index, toLinkIntoId.index);
				reparentNewBubble->execute(gameState);
				actions.push_back(std::move(reparentNewBubble));
			}

			auto linkAction = std::make_unique<bubbleActions::LinkMultiplicationTerm>(toLinkIntoId, toAddId);
			linkAction->execute(gameState);
			Vector3 currentPos = middle::getShapePosition(gameState, toAddId.index);
			middle::moveShape(gameState, toAddId.index, targetPosition - currentPos);
			actions.push_back(std::move(linkAction));

			queueSound(gameState, bubbleSounds::ADD_TERM_SOUND);
		}
	}

	void NewMultiplicationTerm::undo(middle::GameState* gameState)
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
		Vector3 targetPos = middle::getShapePosition(gameState, id.index);
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

		auto reparentToBubble = std::make_unique<middle::EditorActionReparent>(resultId.index, targetId.index);
		reparentToBubble->execute(gameState);
		actions.push_back(std::move(reparentToBubble));

		auto reparentBubble = std::make_unique<middle::EditorActionReparent>(parentId.index, resultId.index);
		reparentBubble->execute(gameState);
		actions.push_back(std::move(reparentBubble));

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
			Vector3 targetPos = middle::getShapePosition(gameState, bubbleId.index);
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
			Vector3 targetPos = middle::getShapePosition(gameState, bubbleId.index);
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
		auto expComp = middle::getComponent<components::ExponentComponent>(shape);

		// simplify exp
		if (expComp) {
			std::vector<middle::Id>children;
			middle::getChildren(gameState, shape.id, children);
			if (children.size() != 1) {
				return middle::Id();
			}

			auto& childShape = middle::getShape(gameState, children[0].index);
			auto childExp = middle::getComponent<components::ExponentComponent>(childShape);
			float powerA = expComp->isInverse ? 1.0f / expComp->power : expComp->power;
			float powerB = childExp->isInverse ? 1.0f / childExp->power : childExp->power;

			// check that same
			const float tolerance = 1e-8f;
			if (std::abs(powerA * powerB - 1) > tolerance) {
				return middle::Id();
			}
			middle::Id copyId = middle::deepCopyShape(gameState, childShape.id.index);
			middle::Shape& copyShape = middle::getShape(gameState, copyId.index);
			middle::queueComponentDeletion<components::ExponentComponent>(gameState, copyShape.id);
			return copyShape.id;
		}

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
				auto unlink = std::make_unique<UnlinkMultiplicationTerm>(parentId, id);
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

			auto replaceAction = std::make_unique<ReplaceBubbleAndTransferTags>(shape.id, replacementShapeId);
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

		// if has exp comp return;
		if (middle::getComponent<components::ExponentComponent>(shape)) {
			cancelled = true;
			return;
		}

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

		auto replaceAction = std::make_unique<ReplaceBubbleAndTransferTags>(shape.id, replacementShapeId);
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

	void Insert::execute(middle::GameState* gameState)
	{
		auto copy = std::make_unique <middle::EditorActionCopySingle>(shapeToInsertId);
		copy->execute(gameState);
		middle::Id copyId = copy->resultId;
		actions.push_back(std::move(copy));

		auto& varShape = middle::getShape(gameState, shapeToReplaceId.index);
		auto bubVar = middle::getComponent<components::BubbleVariable>(varShape);
		if (bubVar && bubVar->isNegative) {
			bubble::negate(gameState, copyId);
		}

		Vector3 targetPos = middle::getShapePosition(gameState, shapeToReplaceId.index);
		Vector3 currentPos = middle::getShapePosition(gameState, copyId.index);
		middle::moveShape(gameState, copyId.index, targetPos - currentPos);

		auto replace = std::make_unique<bubbleActions::ReplaceBubbleAndTransferTags>(shapeToReplaceId, copyId);
		replace->replacingShapeId = copyId;
		replace->execute(gameState);
		actions.push_back(std::move(replace));

		queueSound(gameState, bubbleSounds::ADD_TERM_SOUND);
	}

	void Insert::undo(middle::GameState* gameState)
	{
		while (actions.size() > 0) {
			actions.back()->undo(gameState);
			actions.pop_back();
		}
	}


	void InsertAsXOverX::execute(middle::GameState* gameState)
	{

		Vector3 currPos = middle::getShapePosition(gameState, newTermId.index);
		middle::moveShape(gameState, newTermId.index, targetPos - currPos);

		middle::Id inverseId = bubble::inverseBubble(gameState, newTermId);
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
			auto newContainerProto = bubble::newBubble(gameState, middle::getShapePosition(gameState, shapeToAddInto.id.index));
			auto& newContainerShape = middle::registerShape(gameState, newContainerProto);
			middle::Id addIntoCopy = middle::deepCopyShape(gameState, shapeToAddInto.id.index);
			middle::EditorActionReparent(newContainerShape.id.index, addIntoCopy.index).execute(gameState);

			auto registerContainer = std::make_unique<middle::EditorActionRegisterId>(newContainerShape.id);
			registerContainer->execute(gameState);
			actions.push_back(std::move(registerContainer));

			auto replace = std::make_unique<ReplaceBubbleAndTransferTags>(targetLinkReciever, newContainerShape.id);
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

		Vector3 currppos = middle::getShapePosition(gameState, newTermId.index);
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
		auto exp = middle::getComponent<components::ExponentComponent>(shapeToAddInto);
		if (var || exp) {
			middle::Id copyId = middle::deepCopyShape(gameState, shapeToAddIntoId.index);
			middle::Id replacingId = bubble::containerize(gameState, copyId);

			auto registerAction = std::make_unique<middle::EditorActionRegisterId>(replacingId);
			registerAction->execute(gameState);
			actions.push_back(std::move(registerAction));

			auto replaceAction = std::make_unique<ReplaceBubbleAndTransferTags>(containerId, replacingId);
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


}
