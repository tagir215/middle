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

namespace bubbleActions{

	middle::Id createMultiplicationReplacementShape(middle::GameState* gameState, middle::Id shapeToReplaceId, middle::Id replacingShapeId) {
		auto& shapeToReplace = middle::getShape(gameState, shapeToReplaceId.index);
		auto& replacingShape = middle::getShape(gameState, replacingShapeId.index);

		auto pos = middle::getComponent<components::Position>(shapeToReplace);
		Vector3 targetPos = { pos->posX, pos->posY, pos->posZ };

		auto bubbleComp = middle::getComponent<components::BubbleComponent>(shapeToReplace);
		auto mulComp = middle::getComponent<components::BubbleMultiplyComponent>(shapeToReplace);

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
			return newMulShapeId;
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
			return copyMulShape.id;
		}

		// if shape to replace is a unit
		{
			middle::Id copyId = middle::deepCopyShape(gameState, replacingShape.id.index, middle::UNASSIGNED);
			// compute displacmenet from replacing shape to shapeToReplace position
			Vector3 replacingShapePos = middle::getShapePosition(gameState, copyId.index);
			Vector3 displacement = targetPos - replacingShapePos;
			middle::moveShape(gameState, copyId.index, displacement);
			return copyId;
		}


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
				middle::Id copyId = createMultiplicationReplacementShape(gameState, childId, copyShapeToCopyId);
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
			if(loop->parentLoopId.index != middle::UNASSIGNED) {
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
		std::vector<middle::Id>copyBubbles;

		Combine(middle::Id shapeToAddId, middle::Id shapeToAddIntoId) {
			this->shapeToAddId = shapeToAddId;
			this->shapeToAddIntoId = shapeToAddIntoId;
		}

		void execute(middle::GameState* gameState) override {
			auto& shapeToAdd = middle::getShape(gameState, shapeToAddId.index);
			auto& shapeToAddInto = middle::getShape(gameState, shapeToAddIntoId.index);
			auto addLoop = middle::getComponent<components::LoopSociety>(shapeToAdd);

			// add members from frombubble to intobubble
			for (middle::Id& id : addLoop->loopMemberIds) {
				auto copyId = middle::deepCopyShape(gameState, id.index, shapeToAddInto.id.index);
				copyBubbles.push_back(copyId);
				auto intoLoop = middle::getComponent<components::LoopSociety>(shapeToAddInto);
				intoLoop->loopMemberIds.push_back(copyId);
			}

			setBubbleHidden(gameState, shapeToAdd.id, true);
		}

		void undo(middle::GameState* gameState) override {
			for (middle::Id id : copyBubbles) {
				deleteBubble(gameState, id);
			}
			setBubbleHidden(gameState, shapeToAddId, false);
		}

		void finalize(middle::GameState* gameState) {
			deleteBubble(gameState, shapeToAddId);
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
			for (middle::Id outlineId : bubble->outlineNodes) {
				middle::deleteShape(gameState, outlineId.index);
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

				if (
					gameState->bubbleAlgebraState.grabbedId.index != middle::UNASSIGNED
					&& bubbleToCopyInto->intersectingTop
					) {

					// turn on infinite mass 
					bubbleToCopyInto->infiniteMass = true;

					// multiply action
					auto& mulAction = gameState->bubbleAlgebraState.mulAction;
					mulAction = std::make_unique<bubbleActions::Multiply>(mulShape.id, shapeToCopyId, shapeToCopyIntoId);
					mulAction->execute(gameState);

					bubbleActions::setBubbleHidden(gameState, shapeToCopyId, true);

					// get new reference to shapetopcopyinto because it mulaction might change vectors causing dangling pointers
					auto& shapeToCopyInto = middle::getShape(gameState, shapeToCopyIntoId.index);
					auto time = middle::addComponent<components::TimerComponent>(shapeToCopyInto);
					time->timeLeft = timerTime;
				}

			}
		}

		// undo multiplication if moving the bubble out of intersection
		if (gameState->bubbleAlgebraState.mulAction != nullptr) {
			auto mulAction = static_cast<bubbleActions::Multiply*>(gameState->bubbleAlgebraState.mulAction.get());
			auto& containerShape = middle::getShape(gameState, mulAction->shapeToCopyIntoId.index);
			auto containerBubble = middle::getComponent<components::BubbleComponent>(containerShape);
			auto timer = middle::getComponent<components::TimerComponent>(containerShape);
			if (!containerBubble->intersectingBelow && !timer) {
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
		components::LoopSociety* grabbableLoop = nullptr;
		if (gameState->bubbleAlgebraState.grabbedId.index != middle::UNASSIGNED) {
			auto& grabbedShape = middle::getShape(gameState, gameState->bubbleAlgebraState.grabbedId.index);
			grabbableLoop = middle::getComponent<components::LoopSociety>(grabbedShape);
			if (grabbableLoop->parentLoopId.index == middle::UNASSIGNED) {
				return;
			}

			auto& parentShape = middle::getShape(gameState, grabbableLoop->parentLoopId.index);
			auto mulComp = middle::getComponent<components::BubbleMultiplyComponent>(parentShape);
			if (!mulComp) {
				grabbable = middle::getComponent<components::MouseGrabbable>(grabbedShape);
			}
			else {
				return;
			}
		}

		middle::loopInstances(gameState, [gameState, grabbable, grabbableLoop, this](int i, middle::Shape& shape) {

			auto loop = middle::getComponent<components::LoopSociety>(shape);
			if (!loop)
				return;
			if (loop->parentLoopId.index != middle::UNASSIGNED) {
				auto& parentShape = middle::getShape(gameState, loop->parentLoopId.index);
				auto mulComp = middle::getComponent<components::BubbleMultiplyComponent>(parentShape);
				if (mulComp) {
					return;
				}
			}

			auto bubble = middle::getComponent<components::BubbleComponent>(shape);
			if (!bubble) {
				return;
			}

			// pop action
			if (bubble->intersectingTop && gameState->gameInput.pop) {
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
			if (loop->parentLoopId != grabbableLoop->parentLoopId) {
				return;
			}
			// if the grabbed one is the same one as in this iteration we can continue
			if (shape.id == gameState->bubbleAlgebraState.grabbedId) {
				return;
			}


			// if intersecting while grabbing do addition
			if (bubble->intersectingTop) {
				auto& addAction = gameState->bubbleAlgebraState.addAction;
				addAction = std::make_unique<bubbleActions::Combine>(gameState->bubbleAlgebraState.grabbedId, shape.id);
				addAction->execute(gameState);

				// infinite mass
				auto bubbleToAddInto = middle::getComponent<components::BubbleComponent>(shape);
				bubbleToAddInto->infiniteMass = true;

				auto time = middle::addComponent<components::TimerComponent>(shape);
				time->timeLeft = timerTime;
			}

			});


		// undo if moved to add shape out
		if (gameState->bubbleAlgebraState.addAction != nullptr) {
			auto addAction = static_cast<bubbleActions::Combine*>(gameState->bubbleAlgebraState.addAction.get());
			auto& shapeToAddInto = middle::getShape(gameState, addAction->shapeToAddIntoId.index);
			auto bubbleToAddInto = middle::getComponent<components::BubbleComponent>(shapeToAddInto);
			auto timer = middle::getComponent<components::TimerComponent>(shapeToAddInto);
			if (!bubbleToAddInto->intersectingBelow && !timer) {
				addAction->undo(gameState);
				gameState->bubbleAlgebraState.addAction.release();
				bubbleToAddInto->infiniteMass = false;
			}
		}

		// finialize addition if releasing mouse
		if (gameState->bubbleAlgebraState.addAction != nullptr) {
			if (gameState->input.mouseReleased) {
				auto addAction = static_cast<bubbleActions::Combine*>(gameState->bubbleAlgebraState.addAction.get());
				auto& shapeToAddInto = middle::getShape(gameState, addAction->shapeToAddIntoId.index);
				auto bubbleToAddInto = middle::getComponent<components::BubbleComponent>(shapeToAddInto);
				bubbleToAddInto->infiniteMass = false;
				addAction->finalize(gameState);
				gameState->bubbleAlgebraState.addAction.release();
			}
		}

	}
};

static middle::SystemRegistrar<BubbleMultipliplicationSystem> reg("BubbleMultipliplicationSystem");
