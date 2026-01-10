#pragma once
#include "game_state.h"
#include "registrars.h"
#include "MouseIntersectable.h"
#include "middle_shape_utils.h"
#include "MouseGrabbable.h"
#include "Position.h"

namespace MouseGrabbingSystem {

	class MouseGrabbingSystem : public middle::MiddleGameplaySystem {
		void update(middle::GameState* gameState) override {
			for (int i = 0; i < gameState->shapes.size(); ++i) {
				middle::Shape& shape = gameState->shapes[i];
				auto grabbable = middle::getComponent<components::MouseGrabbable>(shape);
				if (!grabbable)
					continue;

				if (middle::isShapeSelected(gameState, i) && gameState->input.grabDown) {
					grabbable->grabbing = true;
				}
				else {
					grabbable->grabbing = false;
				}

				if (grabbable->grabbing) {
					Vector3 pos = middle::getShapePosition(gameState, i);
					Vector3 cameraPos = gameState->editorState.camera.position;
					float objYDistance = std::abs(pos.y - cameraPos.y);
					float yDistance = std::abs(cameraPos.y);
					if (yDistance == 0)
						yDistance = 0.001f;
					Vector3 xzVel = Vector3Scale(gameState->input.mouseXZ_PlaneVelocity, objYDistance / yDistance);
					dragShape(gameState, i, xzVel);
				}

				if (gameState->input.grabReleased && gameState->selectCount == 1) {
					unselect(gameState);
				}

				bool paused = gameState->paused && !gameState->editorState.doOneStep;
				gameState->editorState.doOneStep = false;

				if (paused || gameState->editorState.stepDir == -1)
				{
					gameState->editorState.doOneStep = false;
					return;
				}
			}
		}
	};

	static middle::SystemRegistrar<MouseGrabbingSystem> reg("MouseGrabbingSystem");

}
