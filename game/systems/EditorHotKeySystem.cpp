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
	void init(middle::GameState* gameState) {

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
				middle::queueEditorAction(gameState, std::make_shared<middle::EditorActionNewSphere>(gameState->input.mouseXZ_PlanePos));
			}
			if (gameState->editorState.creationMode == middle::CreationMode::CONSTRAINT_MODE) {
				std::vector<int> selectedIndexes = middle::getSelectedShapes(gameState);
				if (selectedIndexes.size() == 2) {
					middle::queueEditorAction(gameState, std::make_shared<middle::EditorActionNewConstraint>(selectedIndexes[0], selectedIndexes[1]));
				}

			}
			if (gameState->editorState.creationMode == middle::CreationMode::CAMERA_MODE) {
				if (gameState->input.newThing) {
					Camera camera = gameState->editorState.camera;
					middle::queueEditorAction(gameState, std::make_shared<middle::EditorActionNewCamera>(camera.position, camera.target, camera.up, camera.fovy, camera.projection));
				}
				if (gameState->input.focus) {
					std::vector<int>selectedIndexes = middle::getSelectedShapes(gameState);
					middle::queueEditorAction(gameState, std::make_shared<middle::EditorActionSelectCamera>(selectedIndexes[0]));
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
						middle::queueEditorAction(gameState, std::make_shared<middle::EditorActionReparent>(parentIndex, childIndex));
					}
				}

				// remove from loop
				if (gameState->input.seaprateFromParentClick) {
					int intersectedShape = middle::getMouseIntersectedShape(gameState);
					if (intersectedShape != middle::UNASSIGNED) {
						middle::queueEditorAction(gameState, std::make_shared<middle::EditorActionRemoveFromLoop>(intersectedShape));
					}
				}
			}
		}

		if (gameState->editorState.creationMode == middle::CreationMode::SELECT_MODE) {
			if (gameState->input.copyClick) {
				middle::queueEditorAction(gameState, std::make_shared<middle::EditorActionCopy>(middle::getSelectedShapes(gameState)));
			}

			if (gameState->input.deleteClick) {
				middle::queueEditorAction(gameState, std::make_shared<middle::EditorActionDelete>(middle::getSelectedShapes(gameState)));
			}
		}

		if (gameState->input.saveClick) {
			middle::queueAction(gameState, std::make_shared<middle::EditorActionSaveScene>(gameState->activeSceneName));
		}

		if (gameState->input.loopClick) {
			middle::queueEditorAction(gameState, std::make_shared<middle::EditorActionCreateLoop>(middle::getSelectedShapes(gameState)));
		}

		if (gameState->input.hideClick) {
			std::vector<int>selectedShapes = middle::getSelectedShapes(gameState);
			if (selectedShapes.size() > 0) {
				middle::queueEditorAction(gameState, std::make_shared<middle::EditorActionHide>(selectedShapes));
			}
			else {
				middle::queueEditorAction(gameState, std::make_shared<middle::EditorActionUnhide>());
			}
		}

	}
};

static middle::SystemRegistrar<EditorHotKeySystem> reg("EditorHotKeySystem");
