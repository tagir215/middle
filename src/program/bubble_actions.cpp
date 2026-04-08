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

	void validateFraction(middle::GameState* gameState, middle::Id& id) {
		middle::Shape& shape = middle::getShape(gameState, id.index);
		auto loop = middle::getComponent<components::LoopSociety>(shape);
		auto fraction = middle::getComponent<components::FractionalComponent>(shape);
		assert(fraction);
		int nonZeroCount = 0;
		for (middle::Id& childId : loop->loopMemberIds) {
			auto& childShape = middle::getShape(gameState, childId.index);
			auto childFraction = middle::getComponent<components::FractionalComponent>(childShape);
			auto mulComp = middle::getComponent<components::BubbleMultiplyComponent>(childShape);
			assert(!childFraction);
			assert(!mulComp);
			auto unit = middle::getComponent<components::BubbleUnit>(childShape);
			if (!unit || unit->value != 0) {
				++nonZeroCount;
			}
		}
		assert(nonZeroCount == 1);
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
		auto fraction = middle::getComponent<components::FractionalComponent>(shapeToReplace);
		auto unit = middle::getComponent<components::BubbleUnit>(shapeToReplace);
		auto variable = middle::getComponent<components::BubbleVariable>(shapeToReplace);
		auto root = middle::getComponent<components::ExponentComponent>(shapeToReplace);

		// contain shape to replace in a new bubble and link to the new container
		if (variable || root) {
			middle::Shape newContainerBubbleProto = bubble::newBubble(gameState, targetPos);
			middle::Shape& newContainerBubble = middle::registerShape(gameState, newContainerBubbleProto);
			middle::Id copyId = middle::deepCopyShape(gameState, shapeToReplaceId.index);
			middle::EditorActionReparent(newContainerBubble.id.index, copyId.index).execute(gameState);
			middle::Id replacingCopyId = middle::deepCopyShape(gameState, replacingShapeId.index);

			auto newMul = NewMultiplication(replacingCopyId, newContainerBubble.id);
			newMul.execute(gameState);

			resultShapeId = newMul.resultShapeId;
			return;
		}
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

		if (fraction) {
			middle::Id& copyFractionId = middle::deepCopyShape(gameState, shapeToReplace.id.index);
			auto& copyFractionShape = middle::getShape(gameState, copyFractionId.index);

			// compute displacmenet from replacing shape to shapeToReplace position
			Vector3 replacingShapePos = middle::getShapePosition(gameState, copyFractionShape.id.index);
			Vector3 displacement = targetPos - replacingShapePos;
			middle::moveShape(gameState, copyFractionShape.id.index, displacement);

			middle::Id quotientId = bubble::fractionQuotient(gameState, copyFractionId);

			auto createReplacement = CreateMulitiplicationReplacementShape(quotientId, replacingShapeId);
			createReplacement.execute(gameState);

			middle::Id resultId = createReplacement.resultShapeId;
			auto& resultShape = middle::getShape(gameState, resultId.index);

			// if result is not a bubble we need to contain it into a bubble
			auto bubble = middle::getComponent<components::BubbleComponent>(resultShape);
			if (!bubble) {
				middle::Shape newContainerProto = bubble::newBubble(gameState, targetPos);
				middle::Shape& newContainer = middle::registerShape(gameState, newContainerProto);
				middle::EditorActionReparent(newContainer.id.index, resultId.index).execute(gameState);
				resultId = newContainer.id;
			}

			auto replace = Replace(quotientId, resultId);
			replace.execute(gameState);

			resultShapeId = copyFractionId;
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

			if (unit->value == -1) {
				bubble::negate(gameState, copyId);
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
		auto fractionA = middle::getComponent<components::FractionalComponent>(shapeA);
		auto fractionB = middle::getComponent<components::FractionalComponent>(shapeB);
		auto rootA = middle::getComponent<components::ExponentComponent>(shapeA);
		auto rootB = middle::getComponent<components::ExponentComponent>(shapeB);

		middle::Id replacementId;

		// note same scale fractions are handled separatedly

		// NEW CONTAINING BUBBLE CASE
		if ((unitA && unitB) || (fractionA && fractionB) || (unitA && fractionB) || (unitB && fractionA) || (rootA || rootB) || (bubbleA && bubbleB)) {
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
		// BUBBLE & FRACTION CASE
		else if (fractionA && bubbleB || fractionB && bubbleA) {
			auto& fractionShape = fractionA != nullptr ? shapeA : shapeB;
			auto& bubbleShape = bubbleA != nullptr ? shapeA : shapeB;
			auto reparentAction = middle::EditorActionReparent(bubbleShape.id.index, fractionShape.id.index);
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

	void ExecuteMultiplication::execute(middle::GameState* gameState) {
		auto copyA = std::make_unique<middle::EditorActionCopySingle>(shapeToCopyId);
		copyA->execute(gameState);
		middle::Id idA = copyA->resultId;
		actions.push_back(std::move(copyA));
		auto copyB = std::make_unique<middle::EditorActionCopySingle>(shapeToCopyIntoId);
		copyB->execute(gameState);
		middle::Id idB = copyB->resultId;
		actions.push_back(std::move(copyB));


		std::vector<middle::Id>children;
		middle::getChildren(gameState, idB, children);

		// create replacements to the positions of the old units
		for (int i = 0; i < children.size(); ++i) {
			middle::Id& childId = children[i];

			// replace unit with new copy bubble
			auto createMulAction = std::make_unique<bubbleActions::CreateMulitiplicationReplacementShape>(childId, idA);
			createMulAction->execute(gameState);
			middle::Id copyId = createMulAction->resultShapeId;
			actions.push_back(std::move(createMulAction));

			auto replaceAction = std::make_unique<bubbleActions::Replace>(childId, copyId);
			replaceAction->execute(gameState);
			actions.push_back(std::move(replaceAction));
		}

		auto deleteAction1 = std::make_unique<middle::EditorActionDeleteSingle>(idA);
		deleteAction1->execute(gameState);
		actions.push_back(std::move(deleteAction1));
		auto deleteAction2 = std::make_unique<middle::EditorActionDeleteSingle>(shapeToCopyId);
		deleteAction2->execute(gameState);
		actions.push_back(std::move(deleteAction2));

		resultShapeId = idB;
		auto replace = std::make_unique<bubbleActions::Replace>(shapeToCopyIntoId, resultShapeId);
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
				mulShapeId = parentId;
				auto deleteAction = std::make_unique<middle::EditorActionDeleteSingle>(parentId);
				deleteAction->execute(gameState);
				actions.push_back(std::move(deleteAction));
			}
		}

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
		auto addLoop = middle::getComponent<components::LoopSociety>(shapeToAdd);

		if (!validateAdditionInitialState(gameState, this)) {
			return;
		}

		if (additiveInverses(gameState, shapeToAddId, shapeToAddIntoId)) {
			auto deleteA = std::make_unique<middle::EditorActionDeleteSingle>(shapeToAddId);
			deleteA->execute(gameState);
			actions.push_back(std::move(deleteA));

			auto deleteB = std::make_unique<middle::EditorActionDeleteSingle>(shapeToAddIntoId);
			deleteB->execute(gameState);
			actions.push_back(std::move(deleteB));
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
		auto copyAddLoop = middle::getComponent<components::LoopSociety>(copyShapeA);
		auto fractionA = middle::getComponent<components::FractionalComponent>(copyShapeA);
		auto fractionB = middle::getComponent<components::FractionalComponent>(copyShapeB);
		auto loopA = middle::getComponent<components::LoopSociety>(copyShapeA);
		auto loopB = middle::getComponent<components::LoopSociety>(copyShapeB);
		// can't add fractions of different fractions 
		int sizeA = loopA->loopMemberIds.size();
		int sizeB = loopB->loopMemberIds.size();

		// FRACTION CASE
		if (fractionA && fractionB && sizeA == sizeB) {
			// find non zero indexes
			int indexA, indexB;
			for (int i = 0; i < sizeA; ++i) {
				auto& childShapeA = middle::getShape(gameState, loopA->loopMemberIds[i].index);
				auto& childShapeB = middle::getShape(gameState, loopB->loopMemberIds[i].index);
				auto unitA = middle::getComponent<components::BubbleUnit>(childShapeA);
				auto unitB = middle::getComponent<components::BubbleUnit>(childShapeB);
				auto bubbleA = middle::getComponent<components::BubbleComponent>(childShapeA);
				auto bubbleB = middle::getComponent<components::BubbleComponent>(childShapeB);
				if (unitA && unitA->value != 0 || bubbleA) {
					indexA = i;
				}
				if (unitB && unitB->value != 0 || bubbleB) {
					indexB = i;
				}
			}
			middle::Id childIdA = loopA->loopMemberIds[indexA];
			middle::Id childIdB = loopB->loopMemberIds[indexB];

			auto additionReplacement = std::make_unique<CreateAdditionReplacementShape>(childIdA, childIdB);
			additionReplacement->execute(gameState);
			middle::Id replacementId = additionReplacement->resultId;
			actions.push_back(std::move(additionReplacement));

			// reparent
			auto reparentAction = std::make_unique<middle::EditorActionReparent>(copyShapeA.id.index, replacementId.index);
			reparentAction->execute(gameState);
			resultShapeId = copyShapeA.id;
			actions.push_back(std::move(reparentAction));

			auto deleteAction = std::make_unique<middle::EditorActionDeleteSingle>(copyShapeB.id);
			deleteAction->execute(gameState);
			actions.push_back(std::move(deleteAction));

		}
		// NORMAL AVERAGE BASIC CASE
		else {
			auto additionAction = std::make_unique<CreateAdditionReplacementShape>(idA, idB);
			additionAction->execute(gameState);
			resultShapeId = additionAction->resultId;
			actions.push_back(std::move(additionAction));
		}

		auto deleteAction = std::make_unique<middle::EditorActionDeleteSingle>(shapeToAddId);
		deleteAction->execute(gameState);
		actions.push_back(std::move(deleteAction));

		auto replace = std::make_unique<Replace>(shapeToAddIntoId, resultShapeId);
		replace->execute(gameState);
		actions.push_back(std::move(replace));
	}

	void ExecuteAddition::undo(middle::GameState* gameState) {
		while (actions.size() > 0) {
			actions.back()->undo(gameState);
			actions.pop_back();
		}
	}

	void ExecutePower::execute(middle::GameState* gameState)
	{
		auto shapeToPower = middle::getShape(gameState, shapeToPowerId.index);
		auto exponent = middle::getComponent<components::ExponentComponent>(shapeToPower);
		// todo: how to do inverse?
		if (exponent->isInverse) {
			return;
		}
		int power = exponent->power;
		bool isNegative = exponent->power < 0;
		if (isNegative) {
			power = -power;
		}
		bool isInverse = exponent->isInverse;
		Vector3 targetPos = middle::getShapePosition(gameState, shapeToPower.id.index);

		middle::Id replacementShapeId;

		// create replacement shape
		if (power >= 0 && !isInverse) {
			auto bubbleProto = bubble::newBubble(gameState, targetPos);
			auto unitProto = bubble::newUnit(gameState, targetPos);
			middle::Shape& initialBubble = middle::registerShape(gameState, bubbleProto);
			middle::Shape& initialUnit = middle::registerShape(gameState, unitProto);
			middle::EditorActionReparent(initialBubble.id.index, initialUnit.id.index).execute(gameState);
			auto bubbleCircle = middle::getComponent<components::Circle>(shapeToPower);

			if (power != 0) {
				auto delComp = middle::attachComponent<components::DeleteComponent>(gameState, initialBubble.id);
				delComp->framesUntilDelete = 60;
			}

			float bubGap = bubbleCircle->radius;
			int sign = 1;
			float totalTranslation = 0;

			for (int i = 0; i < power; ++i) {
				middle::Id copyId;
				if (!isNegative) {
					copyId = middle::deepCopyShape(gameState, shapeToPowerId.index);
					middle::queueComponentDeletion<components::ExponentComponent>(gameState, copyId);
				}
				else {
					copyId = bubble::inverseBubble(gameState, shapeToPowerId);
					auto& containerBubble = middle::getShape(gameState, copyId.index);
					auto loop = middle::getComponent<components::LoopSociety>(containerBubble);
					assert(loop->loopMemberIds.size() == 1);
					middle::Id fractionId = loop->loopMemberIds[0];
					middle::Id quotientId = bubble::fractionQuotient(gameState, fractionId);
					middle::Id rootId = middle::getFirstChildWithComponent(gameState, quotientId, middle::getTypeId<components::ExponentComponent>());
					middle::queueComponentDeletion<components::ExponentComponent>(gameState, rootId);
					middle::queueAction(gameState, std::make_shared<Pop>(rootId));
				}
				// move so its not overlapping perfectly
				sign *= -1;
				const float varianceZ = 0.2f;
				middle::moveShape(gameState, copyId.index, { bubGap * i, 0, varianceZ * sign });
				totalTranslation += bubGap;
				LinkMultiplicationTerm(initialBubble.id, copyId).execute(gameState);
			}
			middle::Id toContainId;
			if (power > 0) {
				// parent should be multiplication at this point
				toContainId = middle::getParent(gameState, initialBubble.id);
				middle::moveShape(gameState, toContainId.index, { -totalTranslation * 0.5f, 0, 0 });
			}
			else {
				// else power by 0 = 1
				toContainId = initialBubble.id;
			}

			// containerize
			auto containerProto = bubble::newBubble(gameState, targetPos);
			middle::Shape& container = middle::registerShape(gameState, containerProto);
			auto reparent = middle::EditorActionReparent(container.id.index, toContainId.index);
			reparent.execute(gameState);
			replacementShapeId = container.id;
		}

		auto registerReplacementShapeAction = std::make_unique<middle::EditorActionRegisterId>(replacementShapeId);
		registerReplacementShapeAction->execute(gameState);
		actions.push_back(std::move(registerReplacementShapeAction));

		auto replaceAction = std::make_unique<Replace>(shapeToPower.id, replacementShapeId);
		replaceAction->execute(gameState);
		actions.push_back(std::move(replaceAction));

		resultShapeId = replacementShapeId;
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


	void Pop::execute(middle::GameState* gameState) {
		middle::Shape& shape = middle::getShape(gameState, id.index);
		// check that there is a parent
		auto bubble = middle::getComponent<components::BubbleComponent>(shape);
		auto fraction = middle::getComponent<components::FractionalComponent>(shape);
		auto root = middle::getComponent<components::ExponentComponent>(shape);
		middle::Id parentId = middle::getParent(gameState, shape.id);
		if (parentId.index == middle::UNASSIGNED) {
			cancelled = true;
			return;
		}
		if (root) {
			cancelled = true;
			return;
		}
		if (!bubble && !fraction) {
			cancelled = true;
			return;
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

		// if it's fraction, the children are converted to fractions when container bubbe
		// is popped
		if (fraction) {
			// calc (initial) fraction dividend, 
			std::vector<middle::Id>fractionChildren;
			middle::getChildren(gameState, id, fractionChildren);
			int dividend = fractionChildren.size();

			Vector3 referencePos = middle::getShapePosition(gameState, fractionChildren[0].index);
			for (int i = 0; i < dividend; ++i) {
				middle::Id fractionChildId = fractionChildren[i];
				auto& fractionChildShape = middle::getShape(gameState, fractionChildId.index);
				auto bubble = middle::getComponent<components::BubbleComponent>(fractionChildShape);
				if (!bubble)
					continue;

				std::vector<middle::Id>bubbleChildren;
				middle::getChildren(gameState, fractionChildShape.id, bubbleChildren);
				for (middle::Id& bubbleChild : bubbleChildren) {
					referencePos.x += 8;
					middle::Id replacementFractionId = bubble::shapeToFraction(gameState, bubbleChild, referencePos, dividend);
					validateFraction(gameState, replacementFractionId);

					auto regAction = std::make_unique<middle::EditorActionRegisterId>(replacementFractionId);
					regAction->execute(gameState);
					actions.push_back(std::move(regAction));

					auto replaceAction = std::make_unique<bubbleActions::Replace>(bubbleChild, replacementFractionId);
					replaceAction->execute(gameState);
					actions.push_back(std::move(replaceAction));

					auto reparentAction = std::make_unique<middle::EditorActionReparent>(parentId.index, replacementFractionId.index);
					reparentAction->execute(gameState);
					actions.push_back(std::move(reparentAction));

					// reposition fraction parts for nices visual
					std::vector<middle::Id>replacementFractionChildren;
					middle::getChildren(gameState, replacementFractionId, replacementFractionChildren);
					for (int index = 0; index < dividend; ++index) {
						Vector3 correspondingPos = middle::getShapePosition(gameState, fractionChildren[index].index);
						middle::Id fraction = replacementFractionChildren[index];
						Vector3 currentPos = middle::getShapePosition(gameState, fraction.index);
						middle::moveShape(gameState, fraction.index, correspondingPos - currentPos);
					}

					validateFraction(gameState, replacementFractionId);
				}
			}
		}
		else if (bubble) {
			std::vector<middle::Id>children;
			middle::getChildren(gameState, id, children);
			for (middle::Id& id : children) {
				auto reparentAction = std::make_unique<middle::EditorActionReparent>(parentId.index, id.index);
				reparentAction->execute(gameState);
				actions.push_back(std::move(reparentAction));
			}
		}

		auto deleteAction = std::make_unique<middle::EditorActionDeleteSingle>(shape.id);
		deleteAction->execute(gameState);
		actions.push_back(std::move(deleteAction));
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
		this->containerShapeId = containerShape;
		this->dividend = dividend;
	}
	void Break::execute(middle::GameState* gameState)
	{
		auto& containerShape = middle::getShape(gameState, containerShapeId.index);
		auto unit = middle::getComponent<components::BubbleUnit>(containerShape);
		if (!unit) {
			return;
		}
		Vector3 containerPos = middle::getShapePosition(gameState, containerShapeId.index);
		Vector3 referencePos = containerPos;
		auto newBubbleProto = bubble::newBubble(gameState, containerPos);
		auto registerAction = std::make_unique<middle::EditorActionRegisterShape>(newBubbleProto);
		registerAction->execute(gameState);
		middle::Id newBubbleId = registerAction->newShapeId;
		actions.push_back(std::move(registerAction));

		// replace unit with fractions
		for (int i = 0; i < dividend; ++i) {
			middle::Id& newFractionId = bubble::newFraction(gameState, referencePos, dividend);
			auto registerFraction = std::make_unique<middle::EditorActionRegisterId>(newFractionId);
			registerFraction->execute(gameState);
			actions.push_back(std::move(registerFraction));

			// replace default 1 unit with the actual unit value, because new fraction returns a generic fraction
			middle::Id& quotientId = bubble::fractionQuotient(gameState, newFractionId);
			auto copyAction = std::make_unique<middle::EditorActionCopySingle>(containerShapeId);
			copyAction->execute(gameState);
			auto replace = std::make_unique<Replace>(quotientId, copyAction->resultId);
			replace->execute(gameState);
			actions.push_back(std::move(copyAction));
			actions.push_back(std::move(replace));

			auto reparent = std::make_unique<middle::EditorActionReparent>(newBubbleId.index, newFractionId.index);
			reparent->execute(gameState);
			actions.push_back(std::move(reparent));
			referencePos.x += 5;
		}

		auto replace = std::make_unique<bubbleActions::Replace>(containerShapeId, newBubbleId);
		replace->execute(gameState);
		actions.push_back(std::move(replace));

		resultShapeId = newBubbleId;
	}

	void Break::undo(middle::GameState* gameState)
	{
		while (actions.size() > 0) {
			actions.back()->undo(gameState);
			actions.pop_back();
		}
	}

	Compress::Compress(middle::Id containerShape)
	{
		this->containerShapeId = containerShape;
	}

	middle::Id compressToPower(middle::GameState* gameState, middle::Shape& multiplicationShape) {
		assert(middle::getComponent<components::BubbleMultiplyComponent>(multiplicationShape));
		auto loop = middle::getComponent<components::LoopSociety>(multiplicationShape);
		assert(loop);
		// default to first child, but see if there's non ones
		middle::Id compressableShapeId = loop->loopMemberIds[0];
		bool compressableIsOne = true;
		for (middle::Id& id : loop->loopMemberIds) {
			if (!bubble::isBubbleWithValueOne(gameState, id)) {
				compressableIsOne = false;
				compressableShapeId = id;
				break;
			}
		}

		int compressableCount = 0;

		// check that all equal
		for (int i = 0; i < loop->loopMemberIds.size(); ++i) {
			middle::Id memberId = loop->loopMemberIds[i];
			// ones are ignored, don't affect multiplication value
			if (bubble::isBubbleWithValueOne(gameState, memberId)) {
				if (compressableIsOne) {
					++compressableCount;
				}
				continue;
			}
			if (!bubble::matchingBubbles(gameState, memberId, compressableShapeId)) {
				// return -1 if can't compress
				return middle::Id();
			}
			++compressableCount;
		}

		middle::Id replacementShapeId = middle::deepCopyShape(gameState, compressableShapeId.index);
		auto exponent = middle::attachComponent<components::ExponentComponent>(gameState, replacementShapeId);
		exponent->power = compressableCount;
		return replacementShapeId;
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

	CommonVariableResult findCommonVariable(middle::GameState* gameState, middle::Id bubbleId) {
		std::vector<RepresentativeGroup>groups;
		std::vector<middle::Id>children;
		middle::getChildren(gameState, bubbleId, children);
		for (middle::Id& childId : children) {
			auto& childShape = middle::getShape(gameState, childId.index);
			auto bubbleComp = middle::getComponent<components::BubbleComponent>(childShape);
			if (bubbleComp) {
				RepresentativeGroup group;
				group.containerId = childId;
				group.representatives.push_back(childId);
				groups.push_back(group);
				continue;
			}

			auto fractionComp = middle::getComponent<components::FractionalComponent>(childShape);
			if (fractionComp) {
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

		if (groups.size() < 2) {
			return CommonVariableResult();
		}

		RepresentativeGroup& firstGroup = groups[0];
		// check that all the other groups contain a similar representative to one of representatives of the first group
		for (int i = 0; i < firstGroup.representatives.size(); ++i) {
			middle::Id& baseRepresentativeId = firstGroup.representatives[i];

			bool foundCommonInAllGroups = true;
			for (int j = 1; j < groups.size(); ++j) {
				if (!groupContains(gameState, groups[j], baseRepresentativeId)) {
					foundCommonInAllGroups = false;
					break;
				}
			}
			if (foundCommonInAllGroups) {
				CommonVariableResult result;
				result.commonVariableFound = true;
				result.commonVariableId = baseRepresentativeId;
				firstGroup.commonIndex = i;
				result.groups = groups;
				return result;
			}
		}

		return CommonVariableResult();
	}

	middle::Id compressToMultiplication(middle::GameState* gameState, middle::Id compressTargetId) {

		CommonVariableResult commonVariableResult = findCommonVariable(gameState, compressTargetId);
		if (!commonVariableResult.commonVariableFound) {
			return middle::Id();
		}

		Vector3 targetPos = middle::getShapePosition(gameState, compressTargetId.index);
		middle::Shape bubbleProto = bubble::newBubble(gameState, targetPos);
		middle::Shape& resultBubble = middle::registerShape(gameState, bubbleProto);

		middle::Id commonCopyId = middle::deepCopyShape(gameState, commonVariableResult.commonVariableId.index);
		middle::EditorActionReparent(resultBubble.id.index, commonCopyId.index).execute(gameState);

		middle::Shape compressedProto = bubble::newBubble(gameState, targetPos);
		middle::Shape& compressedBubble = middle::registerShape(gameState, compressedProto);
		LinkMultiplicationTerm(commonCopyId, compressedBubble.id).execute(gameState);

		for (RepresentativeGroup& group : commonVariableResult.groups) {
			// assume singular thing, it will be replaced with bubble containing unit one
			if (group.representatives.size() == 1) {
				targetPos.x += 1;
				middle::Shape unitProto = bubble::newUnit(gameState, targetPos);
				auto& newUnit = middle::registerShape(gameState, unitProto);
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

		return resultBubble.id;
	}


	void Compress::execute(middle::GameState* gameState)
	{
		middle::Id compressTarget = containerShapeId;
		auto& shape = middle::getShape(gameState, compressTarget.index);
		middle::Id replacementShapeId;

		// if its a fraction try compress to one
		auto fraction = middle::getComponent<components::FractionalComponent>(shape);
		if (fraction) {
			compressTarget = bubble::fractionQuotient(gameState, compressTarget);
		}

		std::vector<middle::Id>candidateChildren;
		middle::getChildren(gameState, compressTarget, candidateChildren);
		int childCount = candidateChildren.size();

		if (childCount == 0) {
			cancelled = true;
			return;
		}

		middle::Id referenceId = candidateChildren[0];

		// if child count = 1 and its a multiplication try compress the multiplication members
		if (childCount == 1) {
			auto& firstChild = middle::getShape(gameState, referenceId.index);
			auto mul = middle::getComponent<components::BubbleMultiplyComponent>(firstChild);
			if (mul) {
				replacementShapeId = compressToPower(gameState, firstChild);
			}
		}
		else {
			replacementShapeId = compressToMultiplication(gameState, compressTarget);
		}

		if (replacementShapeId.index == middle::UNASSIGNED) {
			cancelled = true;
			return;
		}

		auto registerAction = std::make_unique<middle::EditorActionRegisterId>(replacementShapeId);
		registerAction->execute(gameState);
		actions.push_back(std::move(registerAction));

		auto replace = std::make_unique<Replace>(compressTarget, replacementShapeId);
		replace->execute(gameState);
		actions.push_back(std::move(replace));

		resultShapeId = replacementShapeId;
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
		middle::Id parentId = middle::getParent(gameState, recieverShapeId);
		if (parentId.index == middle::UNASSIGNED) {
			cancelled = true;
			return;
		}
		Vector3 targetPos = middle::getShapePosition(gameState, recieverShapeId.index);
		middle::Shape newBubbleProto = bubble::newBubble(gameState, targetPos + Vector3{1,0,0});
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
	}
	void MulOne::undo(middle::GameState* gameState)
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
			if (inputVariable && inputVariable->label == label) {
				oldUnitRef = inputVariable->unitRef;
				inputVariable->unitRef = newUnitRef;
			}
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
			middle::Id& toAddId = shapesToAddIds[i];
			auto reparentAction = std::make_unique<middle::EditorActionReparent>(intoId.index, toAddId.index);
			reparentAction->execute(gameState);
			Vector3 currentPos = middle::getShapePosition(gameState, toAddId.index);
			middle::moveShape(gameState, toAddId.index, targetPosition - currentPos);
			actions.push_back(std::move(reparentAction));
		}
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
	}

	void Bubblify::undo(middle::GameState* gameState)
	{
		while (actions.size() > 0) {
			actions.back()->undo(gameState);
			actions.pop_back();
		}
	}

	middle::Id simplifyToOne(middle::GameState* gameState, middle::Id bubbleId) {
		std::vector<middle::Id>children;
		middle::getChildren(gameState, bubbleId, children);
		if (children.size() == 1) {
			middle::Id& childId = children[0];
			auto& childShape = middle::getShape(gameState, childId.index);
			auto mul = middle::getComponent<components::BubbleMultiplyComponent>(childShape);
			std::vector<middle::Id> mulChildren;
			middle::getChildren(gameState, childShape.id, mulChildren);
			if (mulChildren.size() == 2) {
				if (multiplicativeInverses(gameState, mulChildren[0], mulChildren[1])) {
					Vector3 targetPos = middle::getShapePosition(gameState, bubbleId.index);
					middle::Shape unitProto = bubble::newUnit(gameState, targetPos);
					middle::Shape bubbleProto = bubble::newBubble(gameState, targetPos);
					middle::Shape& unitShape = middle::registerShape(gameState, unitProto);
					middle::Shape& bubbleShape = middle::registerShape(gameState, bubbleProto);
					middle::EditorActionReparent(bubbleShape.id.index, unitShape.id.index).execute(gameState);
					return bubbleShape.id;
				}
			}
		}
		return middle::Id();
	}

	middle::Id simplifyToOneOld(middle::GameState* gameState, middle::Id fractionId) {
		middle::Shape& fractionShape = middle::getShape(gameState, fractionId.index);
		assert(middle::getComponent<components::FractionalComponent>(fractionShape));

		std::vector < middle::Id>fractionChildren;
		middle::getChildren(gameState, fractionId, fractionChildren);
		middle::Id quotientId = bubble::fractionQuotient(gameState, fractionId);
		std::vector < middle::Id>quotientChildren;
		middle::getChildren(gameState, quotientId, quotientChildren);

		if (quotientChildren.size() == 0) {
			return middle::Id();
		}

		middle::Id firstChild = quotientChildren[0];
		auto& firstChildShape = middle::getShape(gameState, firstChild.index);

		int fractionDividend = fractionChildren.size();
		if (fractionDividend != quotientChildren.size()) {
			return middle::Id();
		}

		for (int i = 1; i < quotientChildren.size(); ++i) {
			if (!bubble::matchingBubbles(gameState, quotientChildren[i], firstChild)) {
				return middle::Id();
			}
		}

		middle::Id unitCopyId = middle::deepCopyShape(gameState, firstChild.index);
		return unitCopyId;
	}

	void Simplify::execute(middle::GameState* gameState)
	{
		middle::Shape& shape = middle::getShape(gameState, id.index);
		auto expComp = middle::getComponent<components::ExponentComponent>(shape);
		bool isTopDog = middle::getComponent<components::TopDogBubbleTag>(shape) != nullptr;

		if (expComp) {
			std::vector<middle::Id>children;
			middle::getChildren(gameState, shape.id, children);

			if (children.size() != 1) {
				cancelled = true;
				return;
			}

			auto& childShape = middle::getShape(gameState, children[0].index);
			auto childExp = middle::getComponent<components::ExponentComponent>(childShape);

			float powerA = expComp->isInverse ? 1.0f / expComp->power : expComp->power;
			float powerB = childExp->isInverse ? 1.0f / childExp->power : childExp->power;

			// check that same
			const float tolerance = 1e-8f;
			if (std::abs(powerA * powerB - 1) > tolerance) {
				cancelled = true;
				return;
			}


			auto copyAction = std::make_unique<middle::EditorActionCopySingle>(childShape.id);
			copyAction->execute(gameState);
			middle::Id copyId = copyAction->resultId;
			actions.push_back(std::move(copyAction));
			resultId = copyId;

			middle::Shape& copyShape = middle::getShape(gameState, copyId.index);
			middle::queueComponentDeletion<components::ExponentComponent>(gameState, copyShape.id);

			auto replaceAction = std::make_unique<Replace>(shape.id, copyId);
			replaceAction->execute(gameState);
			if (isTopDog) {
				middle::attachComponent<components::TopDogBubbleTag>(gameState, copyId);
				middle::attachComponent<components::BubbleAlgebraProblem>(gameState, copyId);
			}
			actions.push_back(std::move(replaceAction));
			return;
		}

		auto bubble = middle::getComponent<components::BubbleComponent>(shape);
		if (bubble) {
			middle::Id replacementShapeId = simplifyToOne(gameState, shape.id);
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
			if (isTopDog) {
				middle::attachComponent<components::TopDogBubbleTag>(gameState, replacementShapeId);
				middle::attachComponent<components::BubbleAlgebraProblem>(gameState, replacementShapeId);
			}
			return;
		}

		auto fraction = middle::getComponent<components::FractionalComponent>(shape);
		if (fraction) {
			middle::Id replacementShapeId = simplifyToOneOld(gameState, shape.id);
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
			return;
		}

		cancelled = true;
	}

	void Simplify::undo(middle::GameState* gameState)
	{
		while (actions.size() > 0) {
			actions.back()->undo(gameState);
			actions.pop_back();
		}
	}

}
