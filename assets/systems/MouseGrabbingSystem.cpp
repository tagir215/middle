#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "MouseIntersectable.h"
#include "middle_shape_utils.h"
#include "MouseGrabbable.h"
#include "Position.h"
#include "PlacementComponent.h"
#include "LoopSociety.h"

namespace MouseGrabbingSystem {

	class MouseGrabbingSystem : public middle::MiddleGameplaySystem {
	public:
		MouseGrabbingSystem() {
			systemUpdateType = middle::SystemUpdateType::PREFRAME;
			systemModeType = middle::SystemModeType::EDITOR;
		}
		void update(middle::GameState* gameState) override {
			for (int i = 0; i < gameState->shapes.size(); ++i) {
				middle::Shape& shape = gameState->shapes[i];
				auto grabbable = middle::getComponent<components::MouseGrabbable>(shape);
				auto placable = middle::getComponent<components::PlacementComponent>(shape);
				if (!grabbable && !placable)
					continue;

				if (middle::isShapeSelected(gameState, i) && gameState->input.grabDown) {
					grabbable->grabbing = true;
				}
				else if (!gameState->input.grabDown){
					grabbable->grabbing = false;
				}

				// remove placement component after clicking
				if (placable && gameState->input.mouseClicked) {
					middle::deleteComponent<components::PlacementComponent>(shape);
					std::vector<middle::Id>members;
					middle::getChildren(gameState, shape.id, members);
					for (middle::Id& childId : members) {
						middle::Shape& child = middle::getShape(gameState, childId.index);
						middle::deleteComponent<components::PlacementComponent>(child);
					}
				}

				if (grabbable->grabbing || (placable && placable->grabbing)) {
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

				if (gameState->input.grabReleased && gameState->editorState.selectCount == 1) {
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
