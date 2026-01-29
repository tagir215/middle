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
				newReplacingBubbles.push_back(copy);
				bubblesToDelete.push_back(childId);
			}

			for (int index = 0; index < size; ++index) {
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

			middle::deleteShape(gameState, multiplyShapeId.index);
		}

	};

}

class BubbleMultipliplicationSystem : public middle::MiddleGameplaySystem {
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
				&& gameState->bubbleAlgebraState.bubblesGrabbed == 1
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
			gameState->bubbleAlgebraState.bubblesGrabbed = 0;
		}


	}
};

static middle::SystemRegistrar<BubbleMultipliplicationSystem> reg("BubbleMultipliplicationSystem");
