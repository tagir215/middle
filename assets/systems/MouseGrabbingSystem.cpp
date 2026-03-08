#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "MouseIntersectable.h"
#include "middle_shape_utils.h"
#include "MouseGrabbable.h"
#include "Position.h"
#include "PlacementComponent.h"
#include "LoopSociety.h"
#include "editor_actions.h"
#include "GridElement.h"
#include "component_utils.h"

namespace MouseGrabbingSystem {

	class MouseGrabbingSystem : public middle::MiddleGameplaySystem {
	public:
		MouseGrabbingSystem() {
			systemUpdateType = middle::SystemUpdateType::PREFRAME;
			systemModeType = middle::SystemModeType::EDITOR;
		}
		void init(middle::GameState* gameState) {

		}
		void update(middle::GameState* gameState) override {

			// setup editor action for movement
			if (gameState->editorState.selectCount > 0 && gameState->input.grabDown && !gameState->editorState.grabbing) {
				middle::queueEditorAction(gameState, std::make_shared<middle::EditorActionMove>(middle::getSelectedShapes(gameState)));
				gameState->editorState.grabbing = true;
				return;
			}

			for (int i = 0; i < gameState->shapes.size(); ++i) {
				middle::Shape& shape = gameState->shapes[i];
				auto grabbable = middle::getComponent<components::MouseGrabbable>(shape);
				auto placable = middle::getComponent<components::PlacementComponent>(shape);
				auto position = middle::getComponent<components::Position>(shape);
				if (!position)
					continue;
				if (!grabbable && !placable)
					continue;


				if (grabbable && middle::isShapeSelected(gameState, i) && gameState->input.grabDown) {
					grabbable->grabbing = true;
				}
				else if (grabbable && !gameState->input.grabDown) {
					grabbable->grabbing = false;
				}

				// remove placement component after clicking
				if (placable && gameState->input.mouseClicked) {
					middle::deleteComponent<components::PlacementComponent>(shape);
					std::vector<middle::Id>members;
					middle::getAllChildren(gameState, shape.id, members);
					for (middle::Id& childId : members) {
						middle::Shape& child = middle::getShape(gameState, childId.index);
						middle::queueComponentDeletion<components::PlacementComponent>(gameState, childId);
					}
				}

				if ((grabbable && grabbable->grabbing) || (placable && placable->grabbing)) {
					Vector3 pos = middle::getShapePosition(gameState, i);
					auto grid = middle::getComponent<components::GridElement>(shape);

					if (!grid) {
						Vector3 cameraPos = gameState->editorState.camera.position;
						float objYDistance = std::abs(pos.y - cameraPos.y);
						float yDistance = std::abs(cameraPos.y);
						if (yDistance == 0)
							yDistance = 0.001f;
						Vector3 xzVel = Vector3Scale(gameState->input.mouseXZ_PlaneVelocity, objYDistance / yDistance);
						dragShape(gameState, i, xzVel);
					}
					else {
						Vector3 targetPos = gameState->input.mouseXZ_PlanePos;
						middle::moveShape(gameState, i, targetPos - pos);
					}
				}

				if (gameState->input.grabReleased && gameState->editorState.grabbing) {
					// update move action with new positions for redo to work
					middle::EditorActionContainer* lastAction = gameState->editorState.actionHistory.back().get();
					auto lastMoveAction = static_cast<middle::EditorActionMove*>(lastAction);
					assert(lastMoveAction);
					lastMoveAction->newPositions.resize(lastMoveAction->selectedShapes.size());
					for (int j = 0; j < lastMoveAction->selectedShapes.size(); ++j) {
						auto& selectedShape = middle::getShape(gameState, lastMoveAction->selectedShapes[j]);
						auto position = middle::getComponent<components::Position>(selectedShape);
						lastMoveAction->newPositions[j] = { position->posX, position->posY, position->posZ };
					}
					gameState->editorState.grabbing = false;
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
