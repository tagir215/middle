#include "bubble_actions.h"

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

	void deleteBubble(middle::GameState* gameState, middle::Id& id) {
		middle::Shape& shape = middle::getShape(gameState, id.index);

		// delete outline
		auto bubble = middle::getComponent<components::BubbleComponent>(shape);
		if (bubble) {
			for (middle::Id& outlineId : bubble->outlineNodes) {
				middle::deleteShape(gameState, outlineId.index);
			}
			for (middle::Id& constraintId : bubble->outlineConstraints) {
				middle::deleteShape(gameState, constraintId.index);
			}
		}

		auto loop = middle::getComponent<components::LoopSociety>(shape);
		if (loop) {
			int size = loop->loopMemberIds.size();
			for (int i = size - 1; i >= 0; --i) {
				middle::Id childId = loop->loopMemberIds[i];
				if (!middle::isShapeAlive(gameState, childId.index)) {
					assert(false);
				}
				deleteBubble(gameState, childId);
			}
		}

		middle::deleteShape(gameState, id.index);
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
			auto& newMulShape = middle::addShape(gameState, middle::findFreeIndex(gameState));
			middle::Id newMulShapeId = newMulShape.id;
			auto position = middle::addComponent<components::Position>(newMulShape);
			middle::addComponent<components::BubbleMultiplyComponent>(newMulShape);
			auto sphere = middle::addComponent<components::Sphere>(newMulShape);
			sphere->radius = 2;
			auto text = middle::addComponent<components::Text>(newMulShape);
			text->text = "x";
			middle::addComponent<components::MouseIntersectable>(newMulShape);
			middle::addComponent<components::MouseGrabbable>(newMulShape);
			// deep copy to replace and replacing
			middle::Id copyId = middle::deepCopyShape(gameState, replacingShape.id.index, newMulShapeId.index);
			middle::Id toReplaceCopyId = middle::deepCopyShape(gameState, shapeToReplaceId.index, newMulShapeId.index);

			auto newMulLoop = middle::addComponent<components::LoopSociety>(newMulShape);
			newMulLoop->loopMemberIds.push_back(copyId);
			newMulLoop->loopMemberIds.push_back(toReplaceCopyId);
			// compute displacmenet from replacing shape to shapeToReplace position
			Vector3 replacingShapePos = middle::getShapePosition(gameState, newMulShapeId.index);
			Vector3 displacement = targetPos - replacingShapePos;
			position = middle::getComponent<components::Position>(newMulShape);
			position->posX = targetPos.x;
			position->posY = targetPos.y;
			position->posZ = targetPos.z;
			resultShapeId = newMulShapeId;
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
				deleteBubble(gameState, id);
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
	}


	middle::Shape& newBubble(middle::GameState* gameState, const Vector3& targetPos) {
		auto& newBubbleShape = middle::addShape(gameState, middle::findFreeIndex(gameState));
		middle::addComponent<components::BubbleComponent>(newBubbleShape);
		middle::addComponent<components::MouseGrabbable>(newBubbleShape);
		middle::addComponent<components::MouseIntersectable>(newBubbleShape);
		middle::addComponent<components::LoopTag>(newBubbleShape);
		middle::addComponent<components::Position>(newBubbleShape);
		middle::addComponent<components::LoopSociety>(newBubbleShape);
		Vector3 pos = middle::getShapePosition(gameState, newBubbleShape.id.index);
		middle::moveShape(gameState, newBubbleShape.id.index, targetPos - pos);
		return newBubbleShape;
	}

	middle::Shape& newUnit(middle::GameState* gameState, const Vector3& targetPos)
	{
		auto& newUnitShape = middle::addShape(gameState, middle::findFreeIndex(gameState));
		middle::addComponent<components::BubbleUnit>(newUnitShape);
		middle::addComponent<components::MouseGrabbable>(newUnitShape);
		middle::addComponent<components::MouseIntersectable>(newUnitShape);
		middle::addComponent<components::Position>(newUnitShape);
		middle::addComponent<components::LoopSociety>(newUnitShape);
		auto sphere = middle::addComponent<components::Sphere>(newUnitShape);
		sphere->radius = 2;
		Vector3 pos = middle::getShapePosition(gameState, newUnitShape.id.index);
		middle::moveShape(gameState, newUnitShape.id.index, targetPos - pos);
		return newUnitShape;
	}

	middle::Shape& newFraction(middle::GameState* gameState, const Vector3& targetPos, int dividend)
	{
		auto& newFractionShape = middle::addShape(gameState, middle::findFreeIndex(gameState));
		middle::addComponent<components::FractionalComponent>(newFractionShape);
		middle::addComponent<components::LoopSociety>(newFractionShape);
		middle::addComponent<components::Position>(newFractionShape);
		middle::addComponent<components::LoopTag>(newFractionShape);
		middle::addComponent<components::MouseGrabbable>(newFractionShape);
		middle::addComponent<components::MouseIntersectable>(newFractionShape);
		Vector3 pos = middle::getShapePosition(gameState, newFractionShape.id.index);
		middle::moveShape(gameState, newFractionShape.id.index, targetPos - pos);
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
			middle::Shape& newBubbleShape = newBubble(gameState, targetPos);
			middle::moveShape(gameState, idA.index, { 5,0,0 });
			middle::moveShape(gameState, idB.index, { -5,0,0 });
			auto newLoop = middle::addComponent<components::LoopSociety>(newBubbleShape);
			auto reparentActionA = middle::EditorActionReparent(newBubbleShape.id.index, idA.index);
			auto reparentActionB = middle::EditorActionReparent(newBubbleShape.id.index, idB.index);
			reparentActionA.execute(gameState);
			reparentActionB.execute(gameState);
			newLoop = middle::getComponent<components::LoopSociety>(newBubbleShape);
			replacementId = newBubbleShape.id;
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
			deleteBubble(gameState, idA);
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
	}

	void updateVariable(middle::GameState* gameState, middle::Id& newUnitRef, const std::string& label) {
		middle::loopInstances(gameState, [gameState, &label, &newUnitRef](int i, middle::Shape& shape) {
			auto inputVariable = middle::getComponent<components::InputVariable>(shape);
			if (inputVariable && inputVariable->label == label) {
				inputVariable->unitRef = newUnitRef;
			}
			return true;
			});
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
			if (!bubble) {
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


	ExecuteMultiplication::ExecuteMultiplication(middle::Id multiplyShapeId, middle::Id shapeToCopyId, middle::Id shapeToCopyIntoId) {
		this->multiplyShapeId = multiplyShapeId;
		this->shapeToCopyId = shapeToCopyId;
		this->shapeToCopyIntoId = shapeToCopyIntoId;
	}

	void ExecuteMultiplication::execute(middle::GameState* gameState) {

		// copy multiplication container
		auto& mulShape = middle::getShape(gameState, multiplyShapeId.index);
		auto mulLoop = middle::getComponent<components::LoopSociety>(mulShape);
		resultShapeId = middle::deepCopyShape(gameState, mulShape.id.index, mulLoop->parentLoopId.index);
		mulLoop = middle::getComponent<components::LoopSociety>(mulShape);
		auto& operationContainer = middle::getShape(gameState, resultShapeId.index);
		auto ogPos = middle::getComponent<components::Position>(mulShape);
		auto copyContainerPos = middle::getComponent<components::Position>(operationContainer);
		Vector3 targetPosition = { ogPos->posX, ogPos->posY, ogPos->posZ };
		Vector3 copyPosition = { copyContainerPos->posX, copyContainerPos->posY, copyContainerPos->posZ };
		Vector3 displacement = targetPosition - copyPosition;
		middle::moveShape(gameState, operationContainer.id.index, displacement);

		// find loop indexes because we don't know have direct ids to the relevant shapes
		int shapeToCopyIntoLoopIndex, shapeToCopyLoopIndex;
		int containerLoopSize = mulLoop->loopMemberIds.size();
		int loopIndex = 0;
		for (middle::Id id : mulLoop->loopMemberIds) {
			if (id == shapeToCopyId) {
				shapeToCopyLoopIndex = loopIndex;
			}
			if (id == shapeToCopyIntoId) {
				shapeToCopyIntoLoopIndex = loopIndex;
			}
			++loopIndex;
		}

		// get shapes to copy into from the operation copy
		auto operationContainerLoop = middle::getComponent<components::LoopSociety>(operationContainer);
		middle::Id copyShapeToCopyIntoId = operationContainerLoop->loopMemberIds[shapeToCopyIntoLoopIndex];
		middle::Id copyShapeToCopyId = operationContainerLoop->loopMemberIds[shapeToCopyLoopIndex];
		auto& copyShapeToCopyInto = middle::getShape(gameState, copyShapeToCopyIntoId.index);
		auto& copyShapeToCopy = middle::getShape(gameState, copyShapeToCopyId.index);
		auto copyIntoLoop = middle::getComponent<components::LoopSociety>(copyShapeToCopyInto);
		auto copyCopyLoop = middle::getComponent<components::LoopSociety>(copyShapeToCopy);

		int intoSize = copyIntoLoop->loopMemberIds.size();

		std::vector<middle::Id>bubblesToDelete;

		// create replacements to the positions of the old units
		for (int index = 0; index < intoSize; ++index) {
			// get new pointer each loop
			copyIntoLoop = middle::getComponent<components::LoopSociety>(copyShapeToCopyInto);
			middle::Id& childId = copyIntoLoop->loopMemberIds[index];
			bubblesToDelete.push_back(childId);

			// replace unit with new copy bubble
			auto replacingAction = bubbleActions::CreateMulitiplicationReplacementShape(childId, copyShapeToCopyId);
			replacingAction.execute(gameState);
			middle::Id copyId = replacingAction.resultShapeId;
			auto& copyShpae = middle::getShape(gameState, copyId.index);
			auto copycopycopyLoop = middle::getComponent<components::LoopSociety>(copyShpae);
			auto reparentAction = middle::EditorActionReparent(copyShapeToCopyIntoId.index, copyId.index);
			reparentAction.execute(gameState);
		}

		// delete old ones
		int deleteSize = bubblesToDelete.size();
		for (int index = 0; index < deleteSize; ++index) {
			deleteBubble(gameState, bubblesToDelete[index]);
		}
		deleteBubble(gameState, copyShapeToCopyId);

		// if there's no multiplications left remove the multiplication shape
		auto containerLoop = middle::getComponent<components::LoopSociety>(operationContainer);
		if (containerLoop->loopMemberIds.size() < 2) {
			middle::deleteShape(gameState, resultShapeId.index);
			resultShapeId = copyShapeToCopyIntoId;
		}

		setBubbleHidden(gameState, multiplyShapeId, true);
	}

	void ExecuteMultiplication::undo(middle::GameState* gameState) {
		setBubbleHidden(gameState, multiplyShapeId, false);
		deleteBubble(gameState, resultShapeId);
	}

	void ExecuteMultiplication::finalize(middle::GameState* gameState) {
		auto replace = bubbleActions::Replace(multiplyShapeId, resultShapeId);
		replace.execute(gameState);
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

			auto additionReplacement = bubbleActions::CreateAdditionReplacementShape(childIdA, childIdB);
			additionReplacement.execute(gameState);
			middle::Id replacementId = additionReplacement.resultId;

			// reparent
			auto reparentAction = middle::EditorActionReparent(copyShapeA.id.index, replacementId.index);
			reparentAction.execute(gameState);
			resultShapeId = copyShapeA.id;


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
					deleteBubble(gameState, copyShapeA.id);
					auto& unitShape = middle::getShape(gameState, unitCopyId.index);
					auto unitLoop = middle::getComponent<components::LoopSociety>(unitShape);
					unitLoop->parentLoopId = middle::Id();
					resultShapeId = unitCopyId;
				}
			}

			deleteBubble(gameState, copyShapeB.id);


		}
		// NORMAL AVERAGE BASIC CASE
		else {
			auto additionAction = bubbleActions::CreateAdditionReplacementShape(idA, idB);
			additionAction.execute(gameState);
			resultShapeId = additionAction.resultId;
		}

		setBubbleHidden(gameState, shapeToAdd.id, true);
		setBubbleHidden(gameState, shapeToAddInto.id, true);

	}

	void ExecuteAddition::undo(middle::GameState* gameState) {
		setBubbleHidden(gameState, shapeToAddId, false);
		setBubbleHidden(gameState, shapeToAddIntoId, false);
		deleteBubble(gameState, resultShapeId);
	}

	void ExecuteAddition::finalize(middle::GameState* gameState) {
		deleteBubble(gameState, shapeToAddId);
		auto replace = bubbleActions::Replace(shapeToAddIntoId, resultShapeId);
		replace.execute(gameState);
	}

	Pop::Pop(middle::Id id) {
		this->id = id;
	}

	void Pop::execute(middle::GameState* gameState) {
		middle::Shape& shape = middle::getShape(gameState, id.index);
		// check that there is a parent
		auto loop = middle::getComponent<components::LoopSociety>(shape);
		if (loop->parentLoopId.index == middle::UNASSIGNED) {
			return;
		}

		// delete outline
		auto bubble = middle::getComponent<components::BubbleComponent>(shape);
		int size = bubble->outlineNodes.size();
		for (int i = size - 1; i >= 0; --i) {
			middle::Id& id = bubble->outlineNodes[i];
			middle::deleteShape(gameState, id.index);
		}

		middle::Shape& parentShape = middle::getShape(gameState, loop->parentLoopId.index);

		std::vector<middle::Id>children = loop->loopMemberIds;
		middle::deleteShape(gameState, shape.id.index);

		for (middle::Id& childId : loop->loopMemberIds) {
			auto& childShape = middle::getShape(gameState, childId.index);
			auto childLoop = middle::getComponent<components::LoopSociety>(childShape);
			childLoop->parentLoopId = parentShape.id;
			auto parentLoop = middle::getComponent<components::LoopSociety>(parentShape);
			parentLoop->loopMemberIds.push_back(childShape.id);
		}

	}

	void Pop::undo(middle::GameState* gameState) {

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
			auto reparent = middle::EditorActionReparent(containerParent.index, replacingShapeId.index);
			reparent.execute(gameState);
			// set the correct index
			auto newIndex = middle::EditorActionChangeLoopMemberIndex(containerParent.index, replacingShapeId.index, oldIndex);
			newIndex.execute(gameState);
		}
		Vector3 targetPos = middle::getShapePosition(gameState, shapeToReplaceId.index);
		Vector3 currentPos = middle::getShapePosition(gameState, replacingShapeId.index);
		middle::moveShape(gameState, shapeToReplaceId.index, targetPos - currentPos);
		deleteBubble(gameState, shapeToReplaceId);
	}

	void Replace::undo(middle::GameState* gameState)
	{
	}

	LinkMultiplicationTerm::LinkMultiplicationTerm(middle::Id recieverShape, middle::Id linkingShape)
	{
		this->recieverShapeId = recieverShape;
		this->linkingShapeId = linkingShape;
	}

	void LinkMultiplicationTerm::execute(middle::GameState* gameState)
	{
		auto createAction = bubbleActions::CreateMulitiplicationReplacementShape(recieverShapeId, linkingShapeId);
		createAction.execute(gameState);
		auto replaceAction = bubbleActions::Replace(recieverShapeId, createAction.resultShapeId);
		replaceAction.execute(gameState);
		bubbleActions::deleteBubble(gameState, linkingShapeId);
		resultShapeId = createAction.resultShapeId;
	}

	void LinkMultiplicationTerm::undo(middle::GameState* gameState)
	{

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
		// works for only units for now
		if (!unit)
			return;

		Vector3 containerPos = middle::getShapePosition(gameState, containerShapeId.index);

		auto& newBubble = bubbleActions::newBubble(gameState, containerPos);
		// replace unit with fractions
		Vector3 referencePos = containerPos;
		for (int i = 0; i < dividend; ++i) {
			auto& newFraction = bubbleActions::newFraction(gameState, referencePos, dividend);
			auto reparent = middle::EditorActionReparent(newBubble.id.index, newFraction.id.index);
			reparent.execute(gameState);
			referencePos.x += 5;
		}
		auto replace = bubbleActions::Replace(containerShapeId, newBubble.id);
		replace.execute(gameState);
	}
	void Break::undo(middle::GameState* gameState)
	{
	}
}
