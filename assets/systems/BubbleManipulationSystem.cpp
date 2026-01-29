#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "middle_shape_utils.h"
#include "BubbleComponent.h"
#include "MouseGrabbable.h"
#include "Position.h"
#include "PhysicsData.h"
#include "BubbleUnit.h"
#include "LoopSociety.h"
#include "MouseIntersectable.h"
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
		middle::Id shapeToCopyId;
		middle::Id shapeToCopyIntoId;
		std::vector<middle::Id>bubblesToDelete;
		std::vector<middle::Id>newReplacingBubbles;

		Multiply(middle::Id shapeToCopyId, middle::Id shapeToCopyIntoId) {
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
		}

	};

}

class BubbleManipulationSystem : public middle::MiddleGameplaySystem {

	void update(middle::GameState* gameState) override {
		middle::loopInstances(gameState, [gameState, this](int i, middle::Shape& shape) {

			auto bubble = middle::getComponent<components::BubbleComponent>(shape);
			if (!bubble)
				return;

			auto grabbable = middle::getComponent<components::MouseGrabbable>(shape);
			assert(grabbable);


			if (gameState->bubbleAlgebraState.bubblesGrabbed == 0 && bubble->intersectingTop && gameState->input.mouseHeld) {
				grabbable->grabbing = true;
				++gameState->bubbleAlgebraState.bubblesGrabbed;
			}

			if (gameState->bubbleAlgebraState.bubblesGrabbed == 1 && grabbable->grabbing && !gameState->input.mouseHeld) {
				grabbable->grabbing = false;
				--gameState->bubbleAlgebraState.bubblesGrabbed;
			}


			// bubble moving
			if (grabbable->grabbing) {
				Vector3 pos;
				auto posComponent = middle::getComponent<components::Position>(shape);
				if (posComponent) {
					pos = { posComponent->posX, posComponent->posY, posComponent->posZ };
				}

				Vector3 cameraPos = gameState->editorState.camera.position;
				float objYDistance = std::abs(pos.y - cameraPos.y);
				float yDistance = std::abs(cameraPos.y);
				if (yDistance == 0)
					yDistance = 0.001f;
				Vector3 xzVel = Vector3Scale(gameState->input.mouseXZ_PlaneVelocity, objYDistance / yDistance);
				dragShape(gameState, i, xzVel);
			}

			// bubble multiplication
			if (gameState->bubbleAlgebraState.mulAction == nullptr
				&& !grabbable->grabbing
				&& gameState->bubbleAlgebraState.bubblesGrabbed == 1
				&& bubble->intersectingTop) {
				middle::Id grabbedId = findGrabbedBubble(gameState);

				// turn on infinite mass 
				bubble->infiniteMass = true;

				// multiply action
				auto& mulAction = gameState->bubbleAlgebraState.mulAction;
				mulAction = std::make_unique<bubbleActions::Multiply>(grabbedId, shape.id);
				mulAction->execute(gameState);

				bubbleActions::setBubbleHidden(gameState, grabbedId, true);

				auto time = middle::addComponent<components::TimerComponent>(shape);
				time->timeLeft = 1;
			}

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

			// mouse release after multiplication
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

			});

	}

	middle::Id findGrabbedBubble(middle::GameState* gameState) {
		for (int i = 0; i < gameState->shapes.size(); ++i) {
			if (!middle::isShapeAlive(gameState, i))
				continue;
			auto& shape = middle::getShape(gameState, i);
			auto grabbable = middle::getComponent<components::MouseGrabbable>(shape);
			if (grabbable && grabbable->grabbing) {
				return shape.id;
			}
		}
		return middle::Id();
	}

};

static middle::SystemRegistrar<BubbleManipulationSystem> reg("BubbleManipulationSystem");
