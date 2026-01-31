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
#include "MultiplicationTag.h"
#include "Sphere.h"
#include "Text.h"

namespace bubbleActions{

	middle::Id createReplacementBubble(middle::GameState* gameState, middle::Shape& bubbleToReplace, middle::Shape& replacingBubble) {
		auto pos = middle::getComponent<components::Position>(bubbleToReplace);
		auto loop = middle::getComponent < components::LoopSociety>(bubbleToReplace);
		Vector3 targetPos = { pos->posX, pos->posY, pos->posZ };
		int parentIndex = loop->parentLoopId.index;

		// compute displacmenet from replacing bubble to bubbleToReplace position
		Vector3 replacingBubblePos = middle::getShapePosition(gameState, replacingBubble.id.index);
		Vector3 displacement = targetPos - replacingBubblePos;
		// deep copy shape and translate it 
		middle::Id copyId = middle::deepCopyShape(gameState, replacingBubble.id.index, parentIndex);
		middle::Shape& copyShape = middle::getShape(gameState, copyId.index);
		auto copyPos = middle::getComponent<components::Position>(copyShape);
		middle::moveShape(gameState, copyId.index, displacement);

		return copyId;
	}


	void deleteBubble(middle::GameState* gameState, middle::Id& id) {
		middle::Shape& shape = middle::getShape(gameState, id.index);

		// delete outline
		auto bubble = middle::getComponent<components::BubbleComponent>(shape);
		if (bubble) {
			for (middle::Id& outlineId : bubble->outline) {
				middle::deleteShape(gameState, outlineId.index);
			}
		}

		middle::deleteShapeRecursive(gameState, id.index);
	}

	void setBubbleHidden(middle::GameState* gameState, middle::Id& id, bool hidden) {
		middle::Shape& shape = middle::getShape(gameState, id.index);
		auto bubbleComponent = middle::getComponent<components::BubbleComponent>(shape);
		auto bubbleUnit = middle::getComponent<components::BubbleUnit>(shape);
		if (bubbleUnit) {
			bubbleUnit->hidden = hidden;
			return;
		}

		if(bubbleComponent)
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

	class Multiply : public middle::GameplayAction {
	public:
		middle::Id multiplyShapeId;
		middle::Id shapeToCopyId;
		middle::Id shapeToCopyIntoId;
		std::vector<middle::Id>bubblesToDelete;
		std::vector<middle::Id>newReplacingBubbles;
		std::vector<middle::Id>newMultiplicationShapes;

		Multiply(middle::Id multiplyShapeId, middle::Id shapeToCopyId, middle::Id shapeToCopyIntoId) {
			this->multiplyShapeId = multiplyShapeId;
			this->shapeToCopyId = shapeToCopyId;
			this->shapeToCopyIntoId = shapeToCopyIntoId;
		}

		void execute(middle::GameState* gameState) {
			auto& shapeToCopyInto = middle::getShape(gameState, shapeToCopyIntoId.index);
			auto& shapeToCopy = middle::getShape(gameState, shapeToCopyId.index);

			auto loop = middle::getComponent<components::LoopSociety>(shapeToCopyInto);
			auto copyLoop = middle::getComponent<components::LoopSociety>(shapeToCopy);

			assert(loop->parentLoopId == copyLoop->parentLoopId);

			int size = loop->loopMemberIds.size();

			// create replacements to the positions of the old units
			for (int index = 0; index < size; ++index) {
				// get new pointer each loop
				loop = middle::getComponent<components::LoopSociety>(shapeToCopyInto);
				middle::Id& childId = loop->loopMemberIds[index];
				middle::Shape& childShape = middle::getShape(gameState, childId.index);
				middle::Id copy = createReplacementBubble(gameState, childShape, shapeToCopy);
				auto& copyShape = middle::getShape(gameState, copy.index);
				// delete multiplication tag from the copy shape
				middle::deleteComponent<components::MultiplicationTag>(copyShape);

				auto childBubble = middle::getComponent < components::BubbleComponent>(childShape);

				// if its a bubble with multiplication tag we need to copy multiplication shape only to one of the linked multiplications
				// so we add only to idA and skip idBs
				auto mulTag = middle::getComponent<components::MultiplicationTag>(childShape);
				if (mulTag) {
					auto& mulShape = middle::getShape(gameState, mulTag->multiplicationShapeId.index);
					auto mulComp = middle::getComponent<components::BubbleMultiplyComponent>(mulShape);
					if (mulComp->idA != childShape.id)
						continue;
				}

				// if is a bubble we create new multiplication link
				if (childBubble) {
					newReplacingBubbles.push_back(copy);
					middle::Shape& mulShape = middle::addShape(gameState, middle::findFreeIndex(gameState));
					auto mulComponent = middle::addComponent<components::BubbleMultiplyComponent>(mulShape);
					middle::addComponent<components::Position>(mulShape);
					auto sphere = middle::addComponent<components::Sphere>(mulShape);
					auto text = middle::addComponent<components::Text>(mulShape);
					sphere->radius = 2;
					text->text = "x";
					mulComponent->idA = copyShape.id;
					mulComponent->idB = childShape.id;
					newMultiplicationShapes.push_back(mulShape.id);
				}
				// if is unit then we replace the unit with the copy bubble
				else {
					newReplacingBubbles.push_back(copy);
					bubblesToDelete.push_back(childId);
				}

			}

			int deleteSize = bubblesToDelete.size();
			for (int index = 0; index < deleteSize; ++index) {
				setBubbleHidden(gameState, bubblesToDelete[index], true);
			}

			// add copies to parent
			loop = middle::getComponent<components::LoopSociety>(shapeToCopyInto);
			for (middle::Id& id : newReplacingBubbles) {
				loop->loopMemberIds.push_back(id);
			}

		}

		void undo(middle::GameState* gameState) {
			for (middle::Id& id : newReplacingBubbles) {
				deleteBubble(gameState, id);
			}
			for (middle::Id& id : bubblesToDelete) {
				setBubbleHidden(gameState, id, false);
			}
			setBubbleHidden(gameState, shapeToCopyId, false);
		}

		void finalize(middle::GameState* gameState) {
			for (middle::Id& id : bubblesToDelete) {
				deleteBubble(gameState, id);
			}

			auto& shapeToCopyInto = middle::getShape(gameState, shapeToCopyIntoId.index);
			middle::deleteComponent<components::MultiplicationTag>(shapeToCopyInto);

			middle::deleteShape(gameState, multiplyShapeId.index);
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
			for (middle::Id outlineId : bubble->outline) {
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
	BubbleMultipliplicationSystem() {
		//systemModeType = middle::SystemModeType::ENGINE;
	}

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

		for (middle::Id& mulId : multiplications) {
			auto& mulShape = middle::getShape(gameState, mulId.index);
			auto multiplication = middle::getComponent<components::BubbleMultiplyComponent>(mulShape);
			auto& shapeA = middle::getShape(gameState, multiplication->idA.index);
			auto& shapeB = middle::getShape(gameState, multiplication->idB.index);
			auto bubbleA = middle::getComponent<components::BubbleComponent>(shapeA);
			auto bubbleB = middle::getComponent<components::BubbleComponent>(shapeB);
			auto grabbableA = middle::getComponent<components::MouseGrabbable>(shapeA);
			auto grabbableB = middle::getComponent<components::MouseGrabbable>(shapeB);
			// here we add the multags at runtime, so they don't need to be added at manually
			if (!middle::getComponent<components::MultiplicationTag>(shapeA)) {
				auto tag = middle::addComponent<components::MultiplicationTag>(shapeA);
				tag->multiplicationShapeId = mulShape.id;
			}
			if (!middle::getComponent<components::MultiplicationTag>(shapeB)) {
				auto tag = middle::addComponent<components::MultiplicationTag>(shapeB);
				tag->multiplicationShapeId = mulShape.id;
			}

			// set render item
			Vector3 posA = middle::getShapePosition(gameState, shapeA.id.index);
			Vector3 posB = middle::getShapePosition(gameState, shapeB.id.index);
			Vector3 connectingLinePosA = closestPointOnOutlineToPoint(gameState, posB, bubbleA->outline);
			Vector3 connectingLinePosB = closestPointOnOutlineToPoint(gameState, posA, bubbleB->outline);
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


			if (!grabbableA->grabbing && !grabbableB->grabbing) {
				continue;
			}

			auto& shapeToCopy = grabbableA->grabbing ? shapeA : shapeB;
			auto& shapeToCopyInto = grabbableA->grabbing ? shapeB : shapeA;
			auto bubbleToCopy = grabbableA->grabbing ? bubbleA : bubbleB;
			auto bubbleToCopyInto = grabbableA->grabbing ? bubbleB : bubbleA;


			// bubble multiplication
			if (
				gameState->bubbleAlgebraState.mulAction == nullptr
				&& gameState->bubbleAlgebraState.grabbedId.index != middle::UNASSIGNED
				&& bubbleToCopyInto->intersectingTop
				) {

				// turn on infinite mass 
				bubbleToCopyInto->infiniteMass = true;

				// multiply action
				auto& mulAction = gameState->bubbleAlgebraState.mulAction;
				mulAction = std::make_unique<bubbleActions::Multiply>(mulShape.id, shapeToCopy.id, shapeToCopyInto.id);
				mulAction->execute(gameState);

				bubbleActions::setBubbleHidden(gameState, shapeToCopy.id, true);

				auto time = middle::addComponent<components::TimerComponent>(shapeToCopyInto);
				time->timeLeft = 1;
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

			middle::Shape& grabbedShape = middle::getShape(gameState, mulAction->shapeToCopyId.index);
			middle::deleteShapeRecursive(gameState, grabbedShape.id.index);

			// turn off infinite mass mode
			auto& container = middle::getShape(gameState, mulAction->shapeToCopyIntoId.index);
			auto containerBubble = middle::getComponent<components::BubbleComponent>(container);
			containerBubble->infiniteMass = false;

			gameState->bubbleAlgebraState.mulAction.release();
			gameState->bubbleAlgebraState.grabbedId = middle::Id();
		}



		// ADDITIONS

		// get grabbable if somethign is grabbed
		components::MouseGrabbable* grabbable = nullptr;
		components::LoopSociety* grabbableLoop = nullptr;
		if (gameState->bubbleAlgebraState.grabbedId.index != middle::UNASSIGNED) {
			auto& grabbedShape = middle::getShape(gameState, gameState->bubbleAlgebraState.grabbedId.index);
			auto mulTag = middle::getComponent<components::MultiplicationTag>(grabbedShape);
			if (!mulTag) {
				grabbable = middle::getComponent<components::MouseGrabbable>(grabbedShape);
				grabbableLoop = middle::getComponent<components::LoopSociety>(grabbedShape);
			}
		}

		middle::loopInstances(gameState, [gameState, grabbable, grabbableLoop](int i, middle::Shape& shape) {

			auto multiplicationTag = middle::getComponent<components::MultiplicationTag>(shape);
			if (multiplicationTag) {
				return;
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
			auto loop = middle::getComponent<components::LoopSociety>(shape);
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
				time->timeLeft = 1;
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
