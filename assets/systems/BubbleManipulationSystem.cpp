#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "middle_shape_utils.h"
#include "BubbleComponent.h"
#include "MouseGrabbable.h"
#include "Position.h"
#include "PhysicsData.h"
#include "LoopSociety.h"
#include "MouseIntersectable.h"


class BubbleManipulationSystem : public middle::MiddleGameplaySystem {

	void update(middle::GameState* gameState) override {


		// mouse movement
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
