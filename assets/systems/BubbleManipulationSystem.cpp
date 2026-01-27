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

namespace bubbleActions{
	class BubbleAction {
		virtual void execute(middle::GameState* gameState) = 0;
		virtual void undo(middle::GameState* gameState) = 0;
	};

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

	class Multiply : public BubbleAction {
	public:
		middle::Id shapeToCopyId;
		middle::Id shapeToCopyIntoId;

		Multiply(middle::Id shapeToCopyId, middle::Id shapeToCopyIntoId) {
			this->shapeToCopyId = shapeToCopyId;
			this->shapeToCopyIntoId = shapeToCopyIntoId;
		}

		void execute(middle::GameState* gameState) {
			auto& shapeToCopyInto = middle::getShape(gameState, shapeToCopyIntoId.index);
			auto& shapeToCopy = middle::getShape(gameState, shapeToCopyId.index);

			auto loop = middle::getComponent<components::LoopSociety>(shapeToCopyInto);
			int size = loop->loopMemberIds.size();
			std::vector<middle::Id>duplicates;
			std::vector<middle::Id>bubblesToDelete;

			// create replacements to the positions of the old units
			for (int index = 0; index < size; ++index) {
				// get new pointer each loop
				loop = middle::getComponent<components::LoopSociety>(shapeToCopyInto);
				middle::Id& childId = loop->loopMemberIds[index];
				middle::Shape& childShape = middle::getShape(gameState, childId.index);
				middle::Id copy = createReplacementBubble(gameState, childShape, shapeToCopy);
				duplicates.push_back(copy);
				bubblesToDelete.push_back(childId);
			}

			for (int index = 0; index < size; ++index) {
				deleteBubble(gameState, bubblesToDelete[index]);
			}

			// add copies to parent
			loop = middle::getComponent<components::LoopSociety>(shapeToCopyInto);
			for (middle::Id& id : duplicates) {
				loop->loopMemberIds.push_back(id);
			}

		}
		void undo(middle::GameState* gameState) {

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


			if (gameState->bubbleAlgebraState.bubblesGrabbed == 0 && bubble->intersecting && gameState->input.mouseHeld) {
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
			if (gameState->bubbleAlgebraState.bubbleToDelete.index == middle::UNASSIGNED
				&& !grabbable->grabbing
				&& gameState->bubbleAlgebraState.bubblesGrabbed == 1
				&& bubble->intersecting) {
				middle::Id grabbedId = findGrabbedBubble(gameState);
				auto mulAction = bubbleActions::Multiply(grabbedId, shape.id);
				mulAction.execute(gameState);
				gameState->bubbleAlgebraState.bubbleToDelete = grabbedId;
			}

			if (gameState->bubbleAlgebraState.bubbleToDelete.index != middle::UNASSIGNED 
				&& gameState->input.mouseReleased) {
				middle::Shape& grabbedShape = middle::getShape(gameState, 
					gameState->bubbleAlgebraState.bubbleToDelete.index);
				middle::deleteShapeRecursive(gameState, grabbedShape.id.index);
				gameState->bubbleAlgebraState.bubbleToDelete = middle::Id();
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
