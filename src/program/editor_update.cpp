#include "editor_update.h"
#include "Position.h"
#include "Constraint.h"
#include "Sphere.h"

namespace middle {
	void updateEditor(GameState* gameState)
	{
		processEditorActions(gameState);

		if (gameState->startGame) {
			if (gameState->applicationMode == ApplicationMode::EDITOR_MODE) {
				loadEditorState(gameState);
			}
			gameState->startGame = false;
		}

		// update
		if (gameState->reload) {
			reset(gameState);
			loadSceneNames(gameState);
			loadScriptNames(gameState);
			loadComponentNames(gameState);
			if (gameState->sceneNames.size() > 0) {
				loadScene(gameState, gameState->sceneNames[gameState->activeScene], false);
			}
			gameState->reload = false;
		}
		updateInstances(gameState);


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

		// open script
		//if (gameState->input.openScript && gameState->intersectCount > 0) {
		//	loopInstances(gameState, [&](int i, ShapeInstance& instance) {
		//		if (instance.mouseIntersects) {
		//			//if(instance.shape.type == ShapeType::SYSTEM)
		//			//	gameState->editorState.nextEditorAction = EditorAction::OPEN_SYSTEM;
		//			//if(instance.shape.type == ShapeType::COMPONENT)
		//			//	gameState->editorState.nextEditorAction = EditorAction::OPEN_COMPONENT;
		//			//gameState->editorState.nextEditorActionParams = { "", i };
		//		}
		//		});
		//}

	}
}