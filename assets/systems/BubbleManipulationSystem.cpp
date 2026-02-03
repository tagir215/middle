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
#include "BubbleUnit.h"
#include "FractionalComponent.h"

class BubbleManipulationSystem : public middle::MiddleGameplaySystem {

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

	void update(middle::GameState* gameState) override {


		// mouse movement
		middle::loopInstances(gameState, [gameState, this](int i, middle::Shape& shape) {

			auto grabbable = middle::getComponent<components::MouseGrabbable>(shape);
			if (!grabbable) {
				return;
			}

			bool intersecting = isIntersecting(gameState, shape);

			if (intersecting && gameState->bubbleAlgebraState.grabbedId.index == middle::UNASSIGNED && gameState->input.mouseHeld) {
				grabbable->grabbing = true;
				gameState->bubbleAlgebraState.grabbedId = shape.id;
			}

			if (gameState->bubbleAlgebraState.grabbedId.index != middle::UNASSIGNED && grabbable->grabbing && !gameState->input.mouseHeld) {
				grabbable->grabbing = false;
				gameState->bubbleAlgebraState.grabbedId = middle::Id();
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

};

static middle::SystemRegistrar<BubbleManipulationSystem> reg("BubbleManipulationSystem");
