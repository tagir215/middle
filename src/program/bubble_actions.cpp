#include "PhysicsData.h"
#include "OutputVariable.h"
#include "bubble_actions.h"
#include <string>

namespace bubbleActions{

	bool isIntersecting(middle::GameState* gameState, middle::Shape& shape) {
		auto fraction = middle::getComponent<components::FractionalComponent>(shape);
		auto intersectable = middle::getComponent<components::MouseIntersectable>(shape);

		if (fraction) {
			auto loop = middle::getComponent<components::LoopSociety>(shape);
			for (middle::Id id : loop->loopMemberIds) {
				middle::Shape& shape = middle::getShape(gameState, id.index);
				if (isIntersecting(gameState, shape)) {
					return true;
				}
			}
			return false;
		}
		else if (!intersectable) {
			return false;
		}

		return intersectable->intersectingTop;
	}

	bool equals(middle::GameState* gameState, middle::Id& idA, middle::Id& idB)
	{
		float valueA = unitValue(gameState, idA);
		float valueB = unitValue(gameState, idB);
		const float epsilon = 1e-4f;
		return std::abs(valueA - valueB) < epsilon;
	}

	float unitValue(middle::GameState* gameState, middle::Id& containerId)
	{
		middle::Shape& containerShape = middle::getShape(gameState, containerId.index);
		std::vector<middle::Id>children;
		middle::getChildren(gameState, containerShape.id, children);
		int childCount = children.size();
		auto fraction = middle::getComponent<components::FractionalComponent>(containerShape);
		float value = 0;
		for (middle::Id childId : children) {
			middle::Shape& childShape = middle::getShape(gameState, childId.index);
			auto unit = middle::getComponent<components::BubbleUnit>(childShape);
			if (unit) {
				float v = unit->value;
				if (fraction) {
					assert(childCount != 0);
					v / childCount;
				}
				value += v;
			}
			else {
				value += unitValue(gameState, childId);
			}
		}
		return value;
	}

	int fractionUnitCount(middle::GameState* gameState, middle::Id& fractionId)
	{
		auto& shape = middle::getShape(gameState, fractionId.index);
		auto loop = middle::getComponent<components::LoopSociety>(shape);
		return loop->loopMemberIds.size();
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

		// containe copies in a new multiplication
		if (bubbleComp) {
			// deep copy to replace and replacing
			middle::Id copyId = middle::deepCopyShape(gameState, replacingShape.id.index);
			middle::Id toReplaceCopyId = middle::deepCopyShape(gameState, shapeToReplaceId.index);
			auto& newMulShape = newMultiplication(gameState, copyId, toReplaceCopyId);
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
			auto& copyFraction = middle::getShape(gameState, copyFractionId.index);
			// compute displacmenet from replacing shape to shapeToReplace position
			Vector3 replacingShapePos = middle::getShapePosition(gameState, copyFraction.id.index);
			Vector3 displacement = targetPos - replacingShapePos;
			middle::moveShape(gameState, copyFraction.id.index, displacement);
			std::vector<middle::Id> shapesToDelete;
			auto loop = middle::getComponent<components::LoopSociety>(copyFraction);
			assert(loop);
			int size = loop->loopMemberIds.size();
			for (int i = 0; i < size; ++i) {
				// referesh loop pointer...
				loop = middle::getComponent<components::LoopSociety>(copyFraction);
				middle::Id& id = loop->loopMemberIds[i];
				shapesToDelete.push_back(id);
				auto action = CreateMulitiplicationReplacementShape(id, replacingShape.id);
				action.execute(gameState);
				middle::Id copyId = action.resultShapeId;
				auto reparentAction = middle::EditorActionReparent(copyFractionId.index, copyId.index);
				reparentAction.execute(gameState);
			}
			for (middle::Id& id : shapesToDelete) {
				deleteShapeRecursive(gameState, id.index);
			}
			loop = middle::getComponent<components::LoopSociety>(copyFraction);
			resultShapeId = copyFractionId;
			return;
		}

		// if shape to replace is a unit
		if (unit)
		{
			middle::Id shapeToCopyId;
			if (unit->value == 1) {
				shapeToCopyId = replacingShape.id;
			}
			if (unit->value == 0) {
				shapeToCopyId = shapeToReplace.id;
			}
			middle::Id copyId = middle::deepCopyShape(gameState, shapeToCopyId.index, middle::UNASSIGNED);
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
		deleteShapeRecursive(gameState, resultShapeId.index);
	}


	middle::Shape newBubble(middle::GameState* gameState, const Vector3& targetPos) {
		middle::Shape newBubbleShape;
		middle::addComponent<components::BubbleComponent>(newBubbleShape);
		middle::addComponent<components::MouseGrabbable>(newBubbleShape);
		middle::addComponent<components::MouseIntersectable>(newBubbleShape);
		middle::addComponent<components::LoopTag>(newBubbleShape);
		middle::addComponent<components::LoopSociety>(newBubbleShape);
		auto position = middle::addComponent<components::Position>(newBubbleShape);
		position->posX = targetPos.x;
		position->posY = targetPos.y;
		position->posZ = targetPos.z;
		return newBubbleShape;
	}

	middle::Shape newUnit(middle::GameState* gameState, const Vector3& targetPos)
	{
		middle::Shape newUnitShape;
		middle::addComponent<components::BubbleUnit>(newUnitShape);
		middle::addComponent<components::MouseGrabbable>(newUnitShape);
		middle::addComponent<components::MouseIntersectable>(newUnitShape);
		middle::addComponent<components::LoopSociety>(newUnitShape);
		middle::addComponent<components::PhysicsData>(newUnitShape);
		auto sphere = middle::addComponent<components::Sphere>(newUnitShape);
		sphere->radius = 2;
		auto position = middle::addComponent<components::Position>(newUnitShape);
		position->posX = targetPos.x;
		position->posY = targetPos.y;
		position->posZ = targetPos.z;
		return newUnitShape;
	}

	middle::Shape newFraction(middle::GameState* gameState, const Vector3& targetPos, int dividend)
	{
		middle::Shape newFractionShape;
		middle::addComponent<components::FractionalComponent>(newFractionShape);
		middle::addComponent<components::LoopSociety>(newFractionShape);
		middle::addComponent<components::LoopTag>(newFractionShape);
		middle::addComponent<components::MouseGrabbable>(newFractionShape);
		middle::addComponent<components::MouseIntersectable>(newFractionShape);
		auto position = middle::addComponent<components::Position>(newFractionShape);
		position->posX = targetPos.x;
		position->posY = targetPos.y;
		position->posZ = targetPos.z;
		const float fractionUnitSpacing = 10;
		float height = fractionUnitSpacing * dividend - dividend;
		Vector3 referencePos = targetPos;
		referencePos.z += height * 0.5f;
		for (int i = 0; i < dividend; ++i) {
			auto& unit = newUnit(gameState, referencePos);
			auto unitComp = middle::getComponent<components::BubbleUnit>(unit);
			// set everything other than bottom one as 0
			if (i < dividend - 1) {
				unitComp->value = 0;
			}
			else {
				unitComp->value = 1;
			}
			auto reparent = middle::EditorActionReparent(newFractionShape.id.index, unit.id.index);
			reparent.execute(gameState);
			referencePos.z -= fractionUnitSpacing;
		}
		return newFractionShape;
	}

	middle::Shape shapeToFraction(middle::GameState* gameState, middle::Id shapeId, const Vector3& targetPos, int dividend)
	{
		auto& shape = middle::getShape(gameState, shapeId.index);
		auto fraction = middle::getComponent<components::FractionalComponent>(shape);
		if (fraction) {
			int fractionSize = fractionUnitCount(gameState, shapeId);
			dividend *= fractionSize;
			auto& fractionShape = newFraction(gameState, targetPos, dividend);
			return fractionShape;
		}

		auto& fractionShape = newFraction(gameState, targetPos, dividend);
		std::vector<middle::Id> fractionChildren;
		middle::getChildren(gameState, fractionShape.id, fractionChildren);
		for (middle::Id& fractionUnitId : fractionChildren) {
			auto& unitShape = middle::getShape(gameState, fractionUnitId.index);
			auto unit = middle::getComponent<components::BubbleUnit>(unitShape);
			if (unit->value == 1) {
				Replace(fractionUnitId, shapeId).execute(gameState);
			}
		}
		return fractionShape;
	}


	middle::Shape newMultiplication(middle::GameState* gameState, middle::Id& idA, middle::Id& idB)
	{
		middle::Shape newMulShape;
		auto position = middle::addComponent<components::Position>(newMulShape);
		middle::addComponent<components::BubbleMultiplyComponent>(newMulShape);
		middle::addComponent<components::MouseIntersectable>(newMulShape);
		middle::addComponent<components::MouseGrabbable>(newMulShape);
		middle::addComponent<components::LoopSociety>(newMulShape);
		auto sphere = middle::addComponent<components::Sphere>(newMulShape);
		middle::Id newMulShapeId = newMulShape.id;
		sphere->radius = 2;
		auto text = middle::addComponent<components::Text>(newMulShape);
		text->text = "x";
		auto reparentA = middle::EditorActionReparent(newMulShape.id.index, idA.index);
		reparentA.execute(gameState);
		auto reparentB = middle::EditorActionReparent(newMulShape.id.index, idB.index);
		reparentB.execute(gameState);
		Vector3 center = middle::getShapePosition(gameState, idA.index) + middle::getShapePosition(gameState, idB.index);
		center *= 0.5f;
		position->posX = center.x;
		position->posY = center.y;
		position->posZ = center.z;
		return newMulShape;
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

		middle::Id replacementId;

		// note same scale fractions are handled separatedly

		// UNIT CASE, or different scale fractions
		if ((unitA && unitB) || (fractionA && fractionB) || (unitA && fractionB) || (unitB && fractionA)) {
			Vector3 targetPos = middle::getShapePosition(gameState, idA.index);
			auto regAction = middle::EditorActionRegisterShape(newBubble(gameState, targetPos));
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
		// BUBBLE CASE
		// add members from frombubble to intobubble
		else if (bubbleA && bubbleB) {
			auto loopA = middle::getComponent<components::LoopSociety>(shapeA);
			for (middle::Id& id : loopA->loopMemberIds) {
				auto copyId = middle::deepCopyShape(gameState, id.index, idB.index);
				auto loopB = middle::getComponent<components::LoopSociety>(shapeB);
				loopB->loopMemberIds.push_back(copyId);
			}
			deleteShapeRecursive(gameState, idA.index);
			auto deleteActio = std::make_unique<middle::EditorActionDeleteSingle>(idA);
			replacementId = idB;
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
		deleteShapeRecursive(gameState, resultId.index);
	}


	middle::Id inverseBubble(middle::GameState* gameState, middle::Id& id)
	{
		return middle::Id();
	}

	middle::Id topLevelBubble(middle::GameState* gameState)
	{
		middle::Id resultId;
		middle::loopInstances(gameState, [gameState, &resultId](int i, middle::Shape& shape) {
			auto bubble = middle::getComponent<components::BubbleComponent>(shape);
			auto mul = middle::getComponent<components::BubbleMultiplyComponent>(shape);
			if (!bubble && !mul) {
				return true;
			}
			middle::Id& parentId = middle::getParent(gameState, shape.id);
			if (parentId.index == middle::UNASSIGNED) {
				resultId = shape.id;
				return false;
			}
			return true;
			});
		return resultId;
	}


	void setBubbleHidden(middle::GameState* gameState, middle::Id& id, bool hidden) {
		middle::Shape& shape = middle::getShape(gameState, id.index);
		auto bubbleComponent = middle::getComponent<components::BubbleComponent>(shape);
		auto bubbleUnit = middle::getComponent<components::BubbleUnit>(shape);
		if (bubbleUnit) {
			bubbleUnit->hidden = hidden;
			return;
		}

		if (bubbleComponent)
			bubbleComponent->hidden = hidden;

		std::vector<middle::Id>children;
		middle::getAllChildren(gameState, shape.id, children);
		for (middle::Id& childId : children) {
			middle::Shape& childShape = middle::getShape(gameState, childId.index);
			auto childBubble = middle::getComponent<components::BubbleComponent>(childShape);
			if (childBubble) {
				childBubble->hidden = hidden;
			}
			auto childUnit = middle::getComponent<components::BubbleUnit>(childShape);
			if (childUnit) {
				childUnit->hidden = hidden;
			}
		}
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
			if (children.size() == 1 && parentsParentId.index != middle::UNASSIGNED) {
				auto reparent = std::make_unique<middle::EditorActionReparent>(parentsParentId.index, children[0].index);
				reparent->execute(gameState);
				actions.push_back(std::move(reparent));
			}
			if (children.size() < 2) {
				mulShapeId = parentId;
				auto deleteAction = std::make_unique<middle::EditorActionDeleteSingle>(parentId);
				deleteAction->execute(gameState);
				actions.push_back(std::move(deleteAction));
			}
		}

	}

	void ExecuteMultiplication::undo(middle::GameState* gameState) {
		while (actions.size() > 0){
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

		middle::Id idA = middle::deepCopyShape(gameState, shapeToAdd.id.index, addLoop->parentLoopId.index);
		middle::Id idB = middle::deepCopyShape(gameState, shapeToAddInto.id.index, addLoop->parentLoopId.index);
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
				if (unitA && unitA->value == 1 || bubbleA) {
					indexA = i;
				}
				if (unitB && unitB->value == 1 || bubbleB) {
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


			// if value is filled,  like 3/3  then turn into 1
			{
				int value = 0;
				auto& replacementShape = middle::getShape(gameState, replacementId.index);
				auto replacementLoop = middle::getComponent<components::LoopSociety>(replacementShape);
				for (middle::Id& childId : replacementLoop->loopMemberIds) {
					auto& newChild = middle::getShape(gameState, childId.index);
					auto unit = middle::getComponent<components::BubbleUnit>(newChild);
					if (unit) {
						value += unit->value;
					}
				}
				if (value == sizeA) {
					middle::Id someUnitId = replacementLoop->loopMemberIds[0];
					middle::Id unitCopyId = middle::deepCopyShape(gameState, someUnitId.index, middle::UNASSIGNED);
					auto deleteAction = std::make_unique<middle::EditorActionDeleteSingle>(copyShapeA.id);
					deleteAction->execute(gameState);
					actions.push_back(std::move(deleteAction));
					auto& unitShape = middle::getShape(gameState, unitCopyId.index);
					auto unitLoop = middle::getComponent<components::LoopSociety>(unitShape);
					unitLoop->parentLoopId = middle::Id();
					resultShapeId = unitCopyId;
				}
			}

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

	Pop::Pop(middle::Id id) {
		this->id = id;
	}

	void Pop::execute(middle::GameState* gameState) {
		middle::Shape& shape = middle::getShape(gameState, id.index);
		// check that there is a parent
		auto bubble = middle::getComponent<components::BubbleComponent>(shape);
		auto fraction = middle::getComponent<components::FractionalComponent>(shape);
		middle::Id parentId = middle::getParent(gameState, shape.id);
		if (parentId.index == middle::UNASSIGNED) {
			return;
		}
		if (!bubble && !fraction) {
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
					auto replacementFraction = shapeToFraction(gameState, bubbleChild, referencePos, dividend);
					auto regAction = std::make_unique<middle::EditorActionRegisterShape>(replacementFraction);
					actions.push_back(std::move(regAction));
					auto reparentAction = std::make_unique<middle::EditorActionReparent>(parentId.index, replacementFraction.id.index);
					reparentAction->execute(gameState);
					actions.push_back(std::move(reparentAction));
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
		bool alreadyMultiplication = false;
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
		auto& newMul = newMultiplication(gameState, recieverShapeId, linkingShapeId);
		if (parentId.index != middle::UNASSIGNED) {
			auto reparent = std::make_unique<middle::EditorActionReparent>(parentId.index, newMul.id.index);
			reparent->execute(gameState);
			actions.push_back(std::move(reparent));

		}
		resultShapeId = newMul.id;
	}

	void LinkMultiplicationTerm::undo(middle::GameState* gameState)
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
		auto bubble = middle::getComponent<components::BubbleComponent>(containerShape);
		auto fraction = middle::getComponent<components::FractionalComponent>(containerShape);
		Vector3 containerPos = middle::getShapePosition(gameState, containerShapeId.index);
		Vector3 referencePos = containerPos;
		auto newBubble = bubbleActions::newBubble(gameState, containerPos);
		auto registerAction = std::make_unique<middle::EditorActionRegisterShape>(newBubble);
		registerAction->execute(gameState);
		resultShapeId = registerAction->newShapeId;
		actions.push_back(std::move(registerAction));

		if (unit) {
			// replace unit with fractions
			for (int i = 0; i < dividend; ++i) {
				auto& newFraction = bubbleActions::newFraction(gameState, referencePos, dividend);
				auto reparent = std::make_unique<middle::EditorActionReparent>(newBubble.id.index, newFraction.id.index);
				reparent->execute(gameState);
				actions.push_back(std::move(registerAction));
				referencePos.x += 5;
			}
		}

		if (bubble) {
			// replace bubble with multiplications with container copies and fractions
			for (int i = 0; i < dividend; ++i) {
				auto newFraction = bubbleActions::newFraction(gameState, referencePos, dividend);
				auto newFractionBubble = bubbleActions::newBubble(gameState, referencePos);
				auto reparentAction1 = std::make_unique<middle::EditorActionReparent>(newFractionBubble.id.index, newFraction.id.index);
				actions.push_back(std::move(reparentAction1));

				auto copy = std::make_unique<middle::EditorActionCopySingle>(containerShapeId);
				copy->execute(gameState);
				middle::Id& containerCopyId = copy->resultId;
				actions.push_back(std::move(copy));

				Vector3 currentPos = middle::getShapePosition(gameState, containerCopyId.index);
				middle::moveShape(gameState, containerCopyId.index, referencePos - currentPos);

				auto newMul = newMultiplication(gameState, newFractionBubble.id, containerCopyId);
				auto reparentAction2 = std::make_unique<middle::EditorActionReparent>(newBubble.id.index, newMul.id.index);
				reparentAction2->execute(gameState);
				actions.push_back(std::move(reparentAction2));
				referencePos.x += 5;
			}
		}

		auto replace = std::make_unique<bubbleActions::Replace>(containerShapeId, newBubble.id);
		replace->execute(gameState);
		actions.push_back(std::move(replace));
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
	void Compress::execute(middle::GameState* gameState)
	{
		std::vector<middle::Id>children;
		middle::getChildren(gameState, containerShapeId, children);
		middle::Id referenceId = children[0];
		int childCount = children.size();
		for (int i = 1; i < childCount; ++i) {
			// if all children are not the same, can't compress
			if (!equals(gameState, referenceId, children[i])) {
				return;
			}
		}

		// create bubble with compress count
		Vector3 targetPos = middle::getShapePosition(gameState, containerShapeId.index);
		middle::Shape countBubble = newBubble(gameState, targetPos);

		auto registerAction1 = std::make_unique<middle::EditorActionRegisterShape>(countBubble);
		registerAction1->execute(gameState);
		resultCountBubbleId = registerAction1->newShapeId;
		actions.push_back(std::move(registerAction1));

		Vector3 refPos = targetPos;
		for (int i = 0; i < childCount; ++i) {
			middle::Shape unitShape = newUnit(gameState, refPos);
			auto registerAction2 = std::make_unique<middle::EditorActionRegisterShape>(countBubble);
			registerAction2->execute(gameState);
			actions.push_back(std::move(registerAction2));

			auto reparent = std::make_unique<middle::EditorActionReparent>(countBubble.id.index, unitShape.id.index);
			reparent->execute(gameState);
			actions.push_back(std::move(reparent));
			refPos.x += 5;
		}

		// create compressed bubble
		middle::Shape compressedBubble = newBubble(gameState, targetPos);
		auto registerAction = std::make_unique<middle::EditorActionRegisterShape>(compressedBubble);
		registerAction->execute(gameState);
		resultCompressedBubbleId = registerAction->newShapeId;
		actions.push_back(std::move(registerAction));

		middle::Id& copyContentId = middle::deepCopyShape(gameState, referenceId.index);
		auto reparent = std::make_unique<middle::EditorActionReparent>(compressedBubble.id.index, copyContentId.index);
		reparent->execute(gameState);
		actions.push_back(std::move(reparent));

		// replace container with compressed
		auto replace = std::make_unique<Replace>(containerShapeId, compressedBubble.id);
		replace->execute(gameState);
		actions.push_back(std::move(replace));

		// link compress count bubble to compressed bubble
		auto link = std::make_unique<LinkMultiplicationTerm>(compressedBubble.id, countBubble.id);
		link->execute(gameState);
		actions.push_back(std::move(link));
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
		Vector3 targetPos = middle::getShapePosition(gameState, recieverShapeId.index);
		middle::Shape newBubble = bubbleActions::newBubble(gameState, targetPos);
		middle::Shape newUnit = bubbleActions::newUnit(gameState, targetPos);
		auto register1 = std::make_unique<middle::EditorActionRegisterShape>(newBubble);
		register1->execute(gameState);
		resultShapeId = register1->newShapeId;
		actions.push_back(std::move(register1));
		auto register2 = std::make_unique<middle::EditorActionRegisterShape>(newUnit);
		register2->execute(gameState);
		actions.push_back(std::move(register2));

		auto reparent = std::make_unique<middle::EditorActionReparent>(newBubble.id.index, newUnit.id.index);
		reparent->execute(gameState);
		actions.push_back(std::move(reparent));
		auto link = std::make_unique<LinkMultiplicationTerm>(recieverShapeId, newBubble.id);
		link->execute(gameState);
		actions.push_back(std::move(link));
	}
	void MulOne::undo(middle::GameState* gameState)
	{
		while (actions.size() > 0) {
			actions.back()->undo(gameState);
			actions.pop_back();
		}
	}
	UpdateVariable::UpdateVariable(std::string& label, middle::Id& newUnitRef)
	{
		this->label = label;
		this->newUnitRef = newUnitRef;
	}
	void UpdateVariable::execute(middle::GameState* gameState)
	{
		std::string& label = this->label;
		middle::Id& newUnitRef = this->newUnitRef;
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
		auto update = UpdateVariable(label, oldUnitRef);
		update.execute(gameState);
	}
}
