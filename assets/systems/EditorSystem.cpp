#pragma once
#include "game_state.h"
#include "registrars.h"
#include "editor_actions.h"
#include "editor_file_utils.h"
#include "middle_shape_utils.h"
#include "middle_gameplay_script_map.h"
#include "SystemEntity.h"

class EditorSystem : public middle::MiddleGameplaySystem {

	void reset(middle::GameState* gameState) {
		for (int i = 0; i < gameState->shapes.size(); ++i) {
			middle::deleteShape(gameState, i);
		}
	}

	// for mental palace. Palace is superior filesystem! TRUST THE PALACE
	void importEngineSystemReferences(middle::GameState* gameState) {
		int highestUsedIndex = middle::findHighestUsedIndex(gameState);
		int index = highestUsedIndex + 1;
		index = highestUsedIndex > middle::GHOST_INDEX_OFFSET ? highestUsedIndex : middle::GHOST_INDEX_OFFSET;
		std::vector<std::string>systemNames;
		systemNames.insert(systemNames.end(), middle::engineSystemNamesFrameStart.begin(), middle::engineSystemNamesFrameStart.end());
		systemNames.insert(systemNames.end(), middle::engineSystemNamesFrameEnd.begin(), middle::engineSystemNamesFrameEnd.end());

		int systemCount = systemNames.size();
		float angleBetween = PI / systemCount;
		std::vector<Vector3> positions;
		const float r = 50;
		positions.resize(systemCount);
		for (int i = 0; i < systemCount; ++i) {
			float angle = angleBetween * i;
			float x = std::cosf(angle) * r;
			float z = std::sinf(angle) * r;
			Vector3 pos = { x,0,z };
			entities::initSystem(gameState, index + i, pos, systemNames[i]);
		}
	}


	void update(middle::GameState* gameState) override {
		processEditorActions(gameState);

		if (gameState->startGame) {
			if (gameState->applicationMode == middle::ApplicationMode::EDITOR_MODE) {
				middle::loadEditorState(gameState);
			}
			gameState->startGame = false;
		}

		// update
		if (gameState->reload) {
			reset(gameState);
			loadSceneNames(gameState);
			loadSystemNames(gameState);
			importEngineSystemReferences(gameState);
			loadComponentNames(gameState);
			if (gameState->sceneNames.size() > 0) {
				loadScene(gameState, gameState->sceneNames[gameState->activeScene], false);
			}
			gameState->reload = false;
		}


		// camera controls
		const float maxCameraSpeed = 60;
		float mouseCamRatio = gameState->input.mousePos.y / gameState->screenHeight;
		const float cameraSpeed = mouseCamRatio * mouseCamRatio * mouseCamRatio * maxCameraSpeed;
		Vector3 cameraMovementDir = { 0,0,0 };
		if (gameState->input.w)
			cameraMovementDir += Vector3Normalize(gameState->editorState.camera.target - gameState->editorState.camera.position);
		if (gameState->input.s)
			cameraMovementDir += Vector3Negate(Vector3Normalize(gameState->editorState.camera.target - gameState->editorState.camera.position));
		if (gameState->input.e)
			cameraMovementDir += { 0, 0, 1 };
		if (gameState->input.q)
			cameraMovementDir += { 0, 0, -1 };
		if (gameState->input.d)
			cameraMovementDir += Vector3Negate(Vector3Normalize(Vector3CrossProduct(gameState->editorState.camera.up, gameState->editorState.camera.target - gameState->editorState.camera.position)));
		if (gameState->input.a)
			cameraMovementDir += Vector3Normalize(Vector3CrossProduct(gameState->editorState.camera.up, gameState->editorState.camera.target - gameState->editorState.camera.position));

		gameState->editorState.camera.position += cameraMovementDir * cameraSpeed;
		gameState->editorState.camera.target += cameraMovementDir * cameraSpeed;

		// count update
		gameState->intersectCount = 0;
		gameState->selectCount = 0;
		for (int i = 0; i < gameState->shapes.size(); ++i) {
			if (isShapeSelected(gameState, i))
				++gameState->selectCount;
			if (isMouseIntersectingShape(gameState, i))
				++gameState->intersectCount;
		}

		// unselect
		if (gameState->input.mouseClicked && gameState->intersectCount == 0) {
			unselect(gameState);
		}
	}
};

static middle::SystemRegistrar<EditorSystem> reg("EditorSystem");
