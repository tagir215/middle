#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "editor_actions.h"
#include "middle_shape_utils.h"

class EditorHotKeySystem : public middle::MiddleGameplaySystem {
public:
	EditorHotKeySystem() {
		systemUpdateType = middle::SystemUpdateType::RENDERING;
		systemModeType = middle::SystemModeType::EDITOR;
	}
	void update(middle::GameState* gameState) override {

		if (gameState->input.selectModeClick) {
			gameState->editorState.creationMode = middle::CreationMode::SELECT_MODE;
		}
		if (gameState->input.sphereModeClick) {
			gameState->editorState.creationMode = middle::CreationMode::SPHERE_MODE;
		}
		if (gameState->input.constraintModeClick) {
			gameState->editorState.creationMode = middle::CreationMode::CONSTRAINT_MODE;
		}
		if (gameState->input.cameraModeClick) {
			gameState->editorState.creationMode = middle::CreationMode::CAMERA_MODE;
		}
		if (gameState->input.loopModeClick) {
			gameState->editorState.creationMode = middle::CreationMode::LOOP_MODE;
		}

		if (gameState->editorState.creationMode != middle::CreationMode::SELECT_MODE) {

			if (gameState->editorState.creationMode == middle::CreationMode::SPHERE_MODE && gameState->input.newThing) {
				gameState->editorState.editorActions.push_back(
					std::make_unique<middle::EditorActionNewSphere>(gameState->input.mouseXZ_PlanePos)
				);
			}
			if (gameState->editorState.creationMode == middle::CreationMode::CONSTRAINT_MODE) {
				std::vector<int> selectedIndexes = middle::getSelectedShapes(gameState);
				if (selectedIndexes.size() == 2) {
					gameState->editorState.editorActions.push_back(
						std::make_unique<middle::EditorActionNewConstraint>(selectedIndexes[0], selectedIndexes[1])
					);
				}

			}
			if (gameState->editorState.creationMode == middle::CreationMode::CAMERA_MODE) {
				if (gameState->input.newThing) {
					Camera camera = gameState->editorState.camera;
					gameState->editorState.editorActions.push_back(
						std::make_unique<middle::EditorActionNewCamera>(camera.position, camera.target, camera.up, camera.fovy, camera.projection)
					);
				}
				if (gameState->input.focus) {
					std::vector<int>selectedIndexes = middle::getSelectedShapes(gameState);
					gameState->editorState.editorActions.push_back(
						std::make_unique<middle::EditorActionSelectCamera>(selectedIndexes[0])
					);
				}
			}

			if (gameState->editorState.creationMode == middle::CreationMode::LOOP_MODE) {
				// reparenting
				if (gameState->input.reparentClick) {
					std::vector<int>selectedShapes = middle::getSelectedShapes(gameState);
					int intersectedShape = middle::getMouseIntersectedShape(gameState);
					if (selectedShapes.size() == 1 
						&& intersectedShape != middle::UNASSIGNED
						&& selectedShapes[0] != intersectedShape
						) {
						int parentIndex = selectedShapes[0];
						int childIndex = intersectedShape;
						gameState->editorState.editorActions.push_back(
							std::make_unique<middle::EditorActionReparent>(parentIndex, childIndex)
						);
					}
				}

				// remove from loop
				if (gameState->input.seaprateFromParentClick) {
					int intersectedShape = middle::getMouseIntersectedShape(gameState);
					if (intersectedShape != middle::UNASSIGNED) {
						gameState->editorState.editorActions.push_back(
							std::make_unique<middle::EditorActionRemoveFromLoop>(intersectedShape)
						);
					}
				}
			}
		}

		if (gameState->editorState.creationMode == middle::CreationMode::SELECT_MODE) {
			if (gameState->input.copyClick) {
				gameState->editorState.editorActions.push_back(
					std::make_unique<middle::EditorActionCopy>(middle::getSelectedShapes(gameState))
				);
			}

			if (gameState->input.deleteClick) {
				gameState->editorState.editorActions.push_back(
					std::make_unique<middle::EditorActionDelete>(middle::getSelectedShapes(gameState))
				);
			}
		}

		if (gameState->input.saveClick) {
			gameState->editorState.editorActions.push_back(
				std::make_unique<middle::EditorActionSaveScene>(gameState->sceneNames[gameState->activeScene])
			);
		}

		if (gameState->input.loopClick) {
			gameState->editorState.editorActions.push_back(
				std::make_unique<middle::EditorActionCreateLoop>(middle::getSelectedShapes(gameState))
			);
		}

		if (gameState->input.hideClick) {
			std::vector<int>selectedShapes = middle::getSelectedShapes(gameState);
			if (selectedShapes.size() > 0) {
				gameState->editorState.editorActions.push_back(
					std::make_unique<middle::EditorActionHide>(selectedShapes)
				);
			}
			else {
				gameState->editorState.editorActions.push_back(
					std::make_unique<middle::EditorActionUnhide>()
				);
			}
		}
	}
};

static middle::SystemRegistrar<EditorHotKeySystem> reg("EditorHotKeySystem");
