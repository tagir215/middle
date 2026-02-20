#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "BubbleComponent.h"
#include "BubbleMultiplyComponent.h"
#include "MouseGrabbable.h"
#include "middle_shape_utils.h"
#include "middle_component_table.h"
#include "Position.h"
#include "LoopSociety.h"
#include "BubbleUnit.h"
#include "TimerComponent.h"
#include "Sphere.h"
#include "Text.h"
#include "editor_actions.h"
#include "FractionalComponent.h"
#include "MouseIntersectable.h"
#include "LoopTag.h"
#include "InventoryItem.h"

namespace bubbleActions{

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


	class CreateMulitiplicationReplacementShape : public middle::GameplayAction {
	public:
		middle::Id shapeToReplaceId;
		middle::Id replacingShapeId;
		middle::Id resultShapeId;
		CreateMulitiplicationReplacementShape(middle::Id shapeToReplace, middle::Id replacingShape) {
			shapeToReplaceId = shapeToReplace;
			replacingShapeId = replacingShape;
		}

		void execute(middle::GameState* gameState) override {
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

		void undo(middle::GameState* gameState) override {

		}
	};

	middle::Shape& newBubble(middle::GameState* gameState, const Vector3& targetPos) {
		auto& newBubbleShape = middle::addShape(gameState, middle::findFreeIndex(gameState));
		middle::addComponent<components::BubbleComponent>(newBubbleShape);
		middle::addComponent<components::MouseGrabbable>(newBubbleShape);
		middle::addComponent<components::MouseIntersectable>(newBubbleShape);
		middle::addComponent<components::LoopTag>(newBubbleShape);
		middle::addComponent<components::Position>(newBubbleShape);
		Vector3 pos = middle::getShapePosition(gameState, newBubbleShape.id.index);
		middle::moveShape(gameState, newBubbleShape.id.index, targetPos - pos);
		return newBubbleShape;
	}

	class CreateAdditionReplacementShape : public middle::GameplayAction {
	public:
		middle::Id idA;
		middle::Id idB;
		middle::Id resultId;

		CreateAdditionReplacementShape(middle::Id idA, middle::Id idB) {
			this->idA = idA;
			this->idB = idB;
		}

		void execute(middle::GameState* gameState) override { 
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

		void undo(middle::GameState* gameState) override {
		}

	};


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
		middle::getChildren(gameState, shape.id, children);
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


	struct MultiplyPair {
		middle::Id parentId;
		middle::Id idA;
		middle::Id idB;
	};

	class Multiply : public middle::GameplayAction {
	public:
		middle::Id multiplyShapeId;
		middle::Id shapeToCopyId;
		middle::Id shapeToCopyIntoId;
		middle::Id operationContainerId;

		Multiply(middle::Id multiplyShapeId, middle::Id shapeToCopyId, middle::Id shapeToCopyIntoId) {
			this->multiplyShapeId = multiplyShapeId;
			this->shapeToCopyId = shapeToCopyId;
			this->shapeToCopyIntoId = shapeToCopyIntoId;
		}

		void execute(middle::GameState* gameState) {

			// copy multiplication container
			auto& mulShape = middle::getShape(gameState, multiplyShapeId.index);
			auto mulLoop = middle::getComponent<components::LoopSociety>(mulShape);
			operationContainerId = middle::deepCopyShape(gameState, mulShape.id.index, mulLoop->parentLoopId.index);
			mulLoop = middle::getComponent<components::LoopSociety>(mulShape);
			auto& operationContainer = middle::getShape(gameState, operationContainerId.index);
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
				middle::deleteShape(gameState, operationContainerId.index);
				operationContainerId = copyShapeToCopyIntoId;
			}

			setBubbleHidden(gameState, multiplyShapeId, true);
		}

		void undo(middle::GameState* gameState) override {
			setBubbleHidden(gameState, multiplyShapeId, false);
			deleteBubble(gameState, operationContainerId);
		}

		void finalize(middle::GameState* gameState) {

			auto& multiplyShape = middle::getShape(gameState, multiplyShapeId.index);
			auto loop = middle::getComponent<components::LoopSociety>(multiplyShape);
			if (loop->parentLoopId.index != middle::UNASSIGNED) {
				auto reparentAction = middle::EditorActionReparent(loop->parentLoopId.index, operationContainerId.index);
				reparentAction.execute(gameState);
			}
			else {
				loop->parentLoopId = middle::Id();
			}

			deleteBubble(gameState, multiplyShapeId);
		}

	};


	class Combine : public middle::GameplayAction {
	public:
		middle::Id shapeToAddId;
		middle::Id shapeToAddIntoId;
		middle::Id operationContainerId;

		Combine(middle::Id shapeToAddId, middle::Id shapeToAddIntoId) {
			this->shapeToAddId = shapeToAddId;
			this->shapeToAddIntoId = shapeToAddIntoId;
		}

		void execute(middle::GameState* gameState) override {
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
				operationContainerId = copyShapeA.id;


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
						operationContainerId = unitCopyId;
					}
				}

				deleteBubble(gameState, copyShapeB.id);


			}
			// NORMAL AVERAGE BASIC CASE
			else {
				auto additionAction = bubbleActions::CreateAdditionReplacementShape(idA, idB);
				additionAction.execute(gameState);
				operationContainerId = additionAction.resultId;
			}

			setBubbleHidden(gameState, shapeToAdd.id, true);
			setBubbleHidden(gameState, shapeToAddInto.id, true);

		}

		void undo(middle::GameState* gameState) override {
			setBubbleHidden(gameState, shapeToAddId, false);
			setBubbleHidden(gameState, shapeToAddIntoId, false);
			deleteBubble(gameState, operationContainerId);
		}

		void finalize(middle::GameState* gameState) {
			auto& shapeToAddInto = middle::getShape(gameState, shapeToAddIntoId.index);
			auto loop = middle::getComponent<components::LoopSociety>(shapeToAddInto);
			auto reparentAction = middle::EditorActionReparent(loop->parentLoopId.index, operationContainerId.index);
			reparentAction.execute(gameState);
			deleteBubble(gameState, shapeToAddId);
			deleteBubble(gameState, shapeToAddIntoId);
		}
	};

	class Pop : public middle::GameplayAction {
	public:
		middle::Id id;

		Pop(middle::Id id) {
			this->id = id;
		}

		void execute(middle::GameState* gameState) override {
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

		void undo(middle::GameState* gameState) override {

		}
	};
}

class BubbleMultipliplicationSystem : public middle::MiddleGameplaySystem {
public:

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

		return intersectable->intersectingTop;
	}

	const float timerTime = 0.2f;
	// MULTIPLICATIONS

	Vector3 closestPointOnOutlineToPoint(middle::GameState* gameState,
		const Vector3& point, const std::vector<middle::Id>& outline) {
		float minDistSq = std::numeric_limits<float>::max();
		Vector3 outlinePos;
		for (const middle::Id& outlineId : outline) {
			Vector3 nodePos = middle::getShapePosition(gameState, outlineId.index);
			float distSq = Vector3DistanceSqr(point, nodePos);
			if (distSq < minDistSq) {
				minDistSq = distSq;
				outlinePos = nodePos;
			}
		}
		return outlinePos;
	}


	void update(middle::GameState* gameState) override {

		// multiplications
		std::vector<middle::Id> multiplications;
		middle::loopInstances(gameState, [gameState, this, &multiplications](int i, middle::Shape& shape) {
			auto multiplication = middle::getComponent<components::BubbleMultiplyComponent>(shape);
			if (multiplication) {
				multiplications.push_back(shape.id);
			}
			});

		// init and render TODO separate
		for (middle::Id& mulId : multiplications) {
			auto& mulShape = middle::getShape(gameState, mulId.index);
			auto multiplication = middle::getComponent<components::BubbleMultiplyComponent>(mulShape);
			auto loop = middle::getComponent<components::LoopSociety>(mulShape);

			// create mul pairs
			std::vector<bubbleActions::MultiplyPair>mulPairs;
			int mulCount = loop->loopMemberIds.size();
			for (int x = 0; x < mulCount; ++x) {
				for (int y = x + 1; y < mulCount; ++y) {
					bubbleActions::MultiplyPair pair;
					pair.parentId = mulId;
					pair.idA = loop->loopMemberIds[x];
					pair.idB = loop->loopMemberIds[y];
					mulPairs.push_back(pair);
				}
			}

			// calc multiplication for each mulpair
			for (auto& mulPair : mulPairs) {
				auto shapeA = middle::getShape(gameState, mulPair.idA.index);
				auto shapeB = middle::getShape(gameState, mulPair.idB.index);
				auto bubbleA = middle::getComponent < components::BubbleComponent>(shapeA);
				auto bubbleB = middle::getComponent < components::BubbleComponent>(shapeB);
				// set render item
				Vector3 posA = middle::getShapePosition(gameState, shapeA.id.index);
				Vector3 posB = middle::getShapePosition(gameState, shapeB.id.index);
				Vector3 connectingLinePosA = closestPointOnOutlineToPoint(gameState, posB, bubbleA->outlineNodes);
				Vector3 connectingLinePosB = closestPointOnOutlineToPoint(gameState, posA, bubbleB->outlineNodes);
				Vector3 connectionCenter = Vector3Scale(connectingLinePosA + connectingLinePosB, 0.5f);
				middle::RenderItem connectingLine;
				connectingLine.type = middle::RenderItemType::LINE;
				connectingLine.linePointA = connectingLinePosA;
				connectingLine.linePointB = connectingLinePosB;
				gameState->renderData.push_back(connectingLine);
				auto mulpos = middle::getComponent<components::Position>(mulShape);
				mulpos->posX = connectionCenter.x;
				mulpos->posY = connectionCenter.y;
				mulpos->posZ = connectionCenter.z;


				// bubble multiplication
				if (gameState->bubbleAlgebraState.mulAction != nullptr)
					continue;

				auto grabbableA = middle::getComponent<components::MouseGrabbable>(shapeA);
				auto grabbableB = middle::getComponent<components::MouseGrabbable>(shapeB);


				if (!grabbableA->grabbing && !grabbableB->grabbing) {
					continue;
				}

				auto& shapeToCopyId = grabbableA->grabbing ? shapeA.id : shapeB.id;
				auto& shapeToCopyIntoId = grabbableA->grabbing ? shapeB.id : shapeA.id;
				auto bubbleToCopy = grabbableA->grabbing ? bubbleA : bubbleB;
				auto bubbleToCopyInto = grabbableA->grabbing ? bubbleB : bubbleA;

				auto& shapeToCopyInto = middle::getShape(gameState, shapeToCopyIntoId.index);
				auto intersectable = middle::getComponent<components::MouseIntersectable>(shapeToCopyInto);

				if (
					gameState->bubbleAlgebraState.grabbedId.index != middle::UNASSIGNED
					&& intersectable->intersectingTop
					) {

					// turn on infinite mass 
					bubbleToCopyInto->infiniteMass = true;

					// multiply action
					auto& mulAction = gameState->bubbleAlgebraState.mulAction;
					mulAction = std::make_unique<bubbleActions::Multiply>(mulShape.id, shapeToCopyId, shapeToCopyIntoId);
					mulAction->execute(gameState);

					bubbleActions::setBubbleHidden(gameState, shapeToCopyId, true);

					auto& shapeToCopyInto = middle::getShape(gameState, shapeToCopyIntoId.index);
					auto time = middle::addComponent<components::TimerComponent>(shapeToCopyInto);
					time->timeLeft = timerTime;
				}

			}
		}

		// inventory additions
		if (gameState->bubbleAlgebraState.grabbedId.index != middle::UNASSIGNED) {
			auto& grabbedShape = middle::getShape(gameState, gameState->bubbleAlgebraState.grabbedId.index);
			auto inventoryItem = middle::getComponent<components::InventoryItem>(grabbedShape);
			bool intersecting = false;
			// find top bubble
			middle::Id topBubbleId;
			const int addType = 1;
			const int mulType = 2;
			if (inventoryItem && (inventoryItem->itemType == mulType || inventoryItem->itemType == addType)) {
				middle::loopInstances(gameState, [gameState, &topBubbleId, &intersecting](int i, middle::Shape& shape) {
					if (shape.id == gameState->bubbleAlgebraState.grabbedId) {
						return;
					}
					auto bubble = middle::getComponent<components::BubbleComponent>(shape);
					auto mul = middle::getComponent<components::BubbleMultiplyComponent>(shape);
					if (!bubble && !mul) {
						return;
					}
					auto loop = middle::getComponent<components::LoopSociety>(shape);
					assert(loop);
					if (loop->parentLoopId.index == middle::UNASSIGNED) {
						topBubbleId = shape.id;
					}
					});

				bool intersecting = false;
				auto& topBubbleShape = middle::getShape(gameState, topBubbleId.index);
				// if is multiplication check interesct from the children
				auto mul = middle::getComponent<components::BubbleMultiplyComponent>(topBubbleShape);
				if (mul) {
					auto loop = middle::getComponent<components::LoopSociety>(topBubbleShape);
					assert(loop);
					for (middle::Id& childId : loop->loopMemberIds) {
						bool childIntersecting = middle::isMouseIntersectingShape(gameState, childId.index);
						if (childIntersecting) {
							intersecting = true;
						}
					}
				}
				else {
					auto intersectable = middle::getComponent<components::MouseIntersectable>(topBubbleShape);
					intersecting = intersectable->intersectingTop;
				}
				if (intersecting) {
					middle::deleteComponent<components::InventoryItem>(grabbedShape);
					if (inventoryItem->itemType == mulType) {
						auto newMul = bubbleActions::CreateMulitiplicationReplacementShape(topBubbleId, grabbedShape.id);
						newMul.execute(gameState);
						middle::deleteShapeRecursive(gameState, grabbedShape.id.index);
						middle::deleteShapeRecursive(gameState, topBubbleId.index);
					}
					else if (inventoryItem->itemType == addType) {
						auto reparent = middle::EditorActionReparent(topBubbleId.index, grabbedShape.id.index);
						reparent.execute(gameState);
					}
					gameState->bubbleAlgebraState.grabbedId = middle::Id();

				}
			}
		}


		// undo multiplication if moving the bubble out of intersection
		if (gameState->bubbleAlgebraState.mulAction != nullptr) {
			auto mulAction = static_cast<bubbleActions::Multiply*>(gameState->bubbleAlgebraState.mulAction.get());
			auto& containerShape = middle::getShape(gameState, mulAction->shapeToCopyIntoId.index);
			auto intersectable = middle::getComponent<components::MouseIntersectable>(containerShape);
			auto containerBubble = middle::getComponent<components::BubbleComponent>(containerShape);
			auto timer = middle::getComponent<components::TimerComponent>(containerShape);
			if (!intersectable->intersecting && !timer) {
				mulAction->undo(gameState);
				gameState->bubbleAlgebraState.mulAction.release();
			}
		}

		// mouse release after multiplication, this will finallize the multiplication
		if (gameState->bubbleAlgebraState.mulAction != nullptr
			&& gameState->input.mouseReleased) {
			auto mulAction = static_cast<bubbleActions::Multiply*>(gameState->bubbleAlgebraState.mulAction.get());
			mulAction->finalize(gameState);
			gameState->bubbleAlgebraState.mulAction.release();
			gameState->bubbleAlgebraState.grabbedId = middle::Id();
		}



		// ADDITIONS

		// get grabbable if somethign is grabbed
		components::MouseGrabbable* grabbable = nullptr;
		middle::Id grabbedParentId;

		if (gameState->bubbleAlgebraState.grabbedId.index != middle::UNASSIGNED) {
			auto& grabbedShape = middle::getShape(gameState, gameState->bubbleAlgebraState.grabbedId.index);
			auto grabbableLoop = middle::getComponent<components::LoopSociety>(grabbedShape);
			if (grabbableLoop->parentLoopId.index == middle::UNASSIGNED) {
				return;
			}

			auto& parentShape = middle::getShape(gameState, grabbableLoop->parentLoopId.index);
			auto mulComp = middle::getComponent<components::BubbleMultiplyComponent>(parentShape);

			// if not multiplciation component we can probably add them together
			if (!mulComp) {
				grabbable = middle::getComponent<components::MouseGrabbable>(grabbedShape);
			}
			else {
				return;
			}
			// set parent.. if fraction we use parents parent, since that is then the same level as the units we would be intersecting
			auto fraction = middle::getComponent<components::FractionalComponent>(parentShape);
			if (fraction) {
				auto parentLoop = middle::getComponent<components::LoopSociety>(parentShape);
				grabbedParentId = parentLoop->parentLoopId;
			}
			else {
				grabbedParentId = parentShape.id;
			}
		}

		middle::loopInstances(gameState, [gameState, grabbable, this, &grabbedParentId](int i, middle::Shape& shape) {

			auto loop = middle::getComponent<components::LoopSociety>(shape);
			if (!loop) {
				return;
			}

			auto bubble = middle::getComponent<components::BubbleComponent>(shape);
			auto unit = middle::getComponent<components::BubbleUnit>(shape);
			auto fraction = middle::getComponent<components::FractionalComponent>(shape);
			if (!bubble && !unit && !fraction) {
				return;
			}

			if (loop->parentLoopId.index != middle::UNASSIGNED) {
				auto& parentShape = middle::getShape(gameState, loop->parentLoopId.index);
				// if grabbing units parent is same as grabbed one we can skip
				if (parentShape.id == gameState->bubbleAlgebraState.grabbedId) {
					return;
				}
				auto mulComp = middle::getComponent<components::BubbleMultiplyComponent>(parentShape);
				if (mulComp) {
					return;
				}
			}

			auto intersectable = middle::getComponent<components::MouseIntersectable>(shape);

			// pop action
			if (bubble && intersectable->intersectingTop && gameState->gameInput.pop) {
				auto popAction = std::make_unique<bubbleActions::Pop>(shape.id);
				popAction->execute(gameState);
			}

			// if not grabbing anything can continue
			if (!grabbable) {
				return;
			}
			// if already adding can continue
			if (gameState->bubbleAlgebraState.addAction != nullptr) {
				return;
			}
			// if not grabbing something from same parents can continue
			if (loop->parentLoopId != grabbedParentId) {
				return;
			}
			// if the grabbed one is the same one as in this iteration we can continue
			if (shape.id == gameState->bubbleAlgebraState.grabbedId) {
				return;
			}

			intersectable = middle::getComponent<components::MouseIntersectable>(shape);

			bool isIntersecting = this->isIntersecting(gameState, shape);

			// if intersecting while grabbing do addition
			if (isIntersecting) {
				auto& addAction = gameState->bubbleAlgebraState.addAction;
				addAction = std::make_unique<bubbleActions::Combine>(gameState->bubbleAlgebraState.grabbedId, shape.id);
				addAction->execute(gameState);

				// infinite mass
				auto bubbleToAddInto = middle::getComponent<components::BubbleComponent>(shape);
				if (bubbleToAddInto) {
					bubbleToAddInto->infiniteMass = true;
				}

				auto time = middle::addComponent<components::TimerComponent>(shape);
				time->timeLeft = timerTime;
			}

			});


		// undo if moved to add shape out
		if (gameState->bubbleAlgebraState.addAction != nullptr) {
			auto addAction = static_cast<bubbleActions::Combine*>(gameState->bubbleAlgebraState.addAction.get());
			auto& shapeToAddInto = middle::getShape(gameState, addAction->shapeToAddIntoId.index);
			auto intersectable = middle::getComponent<components::MouseIntersectable>(shapeToAddInto);
			auto bubbleToAddInto = middle::getComponent<components::BubbleComponent>(shapeToAddInto);
			auto timer = middle::getComponent<components::TimerComponent>(shapeToAddInto);
			if (!intersectable->intersecting && !timer) {
				addAction->undo(gameState);
				gameState->bubbleAlgebraState.addAction.release();
			}
		}

		// finialize addition if releasing mouse
		if (gameState->bubbleAlgebraState.addAction != nullptr) {
			if (gameState->input.mouseReleased) {
				auto addAction = static_cast<bubbleActions::Combine*>(gameState->bubbleAlgebraState.addAction.get());
				auto& shapeToAddInto = middle::getShape(gameState, addAction->shapeToAddIntoId.index);
				auto bubbleToAddInto = middle::getComponent<components::BubbleComponent>(shapeToAddInto);
				addAction->finalize(gameState);
				gameState->bubbleAlgebraState.addAction.release();
			}
		}

	}
};

static middle::SystemRegistrar<BubbleMultipliplicationSystem> reg("BubbleMultipliplicationSystem");
