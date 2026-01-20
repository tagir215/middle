#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "middle_math.h"

class InputSystem : public middle::MiddleGameplaySystem {
	void update(middle::GameState* gameState) override {

		Matrix scalorM = MatrixScale(1, -1, 1);
		Matrix translatorM = MatrixTranslate(0, GetScreenHeight(), 0);
		gameState->screenOrientorM = MatrixMultiply(scalorM, translatorM);

		// INPUTS
		Vector3 mousePos = { GetMouseX(), GetMouseY(), 0 };
		Vector3 invertedMouse = Vector3Transform(mousePos, gameState->screenOrientorM);
		gameState->input.mousePos.x = invertedMouse.x;
		gameState->input.mousePos.y = invertedMouse.y;
		gameState->input.zoomIn = GetMouseWheelMoveV().y > 0;
		gameState->input.zoomOut = GetMouseWheelMoveV().y < 0;

		gameState->input.mouseHeld = false;
		gameState->input.mouseClicked = false;
		gameState->input.mouseReleased = false;
		gameState->input.w = false;
		gameState->input.s = false;
		gameState->input.a = false;
		gameState->input.d = false;
		gameState->input.q = false;
		gameState->input.e = false;
		gameState->input.grabDown = false;
		gameState->input.grabReleased = false;
		gameState->input.grabReleased = false;
		gameState->input.infoClick = false;
		gameState->input.loopClick = false;
		gameState->input.selectModeClick = false;
		gameState->input.sphereModeClick = false;
		gameState->input.constraintModeClick = false;
		gameState->input.cameraModeClick = false;
		gameState->input.loopModeClick = false;
		gameState->input.deleteClick = false;
		gameState->input.copyClick = false;
		gameState->input.saveClick = false;
		gameState->input.navigateToFileClick = false;
		gameState->input.focus = false;
		gameState->input.newThing = false;
		gameState->input.reparentClick = false;
		gameState->input.seaprateFromParentClick = false;

		if (gameState->inputBlockers.find(middle::InputBlockers::MOUSE_BLOCK) == gameState->inputBlockers.end()) {
			gameState->input.mouseHeld = IsMouseButtonDown(MOUSE_BUTTON_LEFT);
			gameState->input.mouseClicked = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
			gameState->input.mouseReleased = IsMouseButtonReleased(MOUSE_BUTTON_LEFT);
		}

		if (gameState->inputBlockers.find(middle::InputBlockers::KEYBOARD_BLOCK) == gameState->inputBlockers.end()) {
			gameState->input.w = IsKeyDown(KEY_W);
			gameState->input.s = IsKeyDown(KEY_S);
			gameState->input.a = IsKeyDown(KEY_A);
			gameState->input.d = IsKeyDown(KEY_D);
			gameState->input.q = IsKeyDown(KEY_Q);
			gameState->input.e = IsKeyDown(KEY_E);
			gameState->input.altDown = IsKeyDown(KEY_LEFT_ALT);
			gameState->input.grabDown = IsKeyDown(KEY_G);
			gameState->input.grabReleased = IsKeyReleased(KEY_G);
			gameState->input.grabReleased = IsKeyReleased(KEY_G);
			gameState->input.infoClick = IsKeyPressed(KEY_I);
			gameState->input.loopClick = IsKeyPressed(KEY_L);
			gameState->input.selectModeClick = IsKeyPressed(KEY_ONE);
			gameState->input.sphereModeClick = IsKeyPressed(KEY_TWO);
			gameState->input.constraintModeClick = IsKeyPressed(KEY_THREE);
			gameState->input.cameraModeClick = IsKeyPressed(KEY_FOUR);
			gameState->input.loopModeClick = IsKeyPressed(KEY_FIVE);
			gameState->input.deleteClick = IsKeyPressed(KEY_R);
			gameState->input.copyClick = IsKeyPressed(KEY_C);
			gameState->input.saveClick = IsKeyPressed(KEY_P);
			gameState->input.navigateToFileClick = IsKeyPressed(KEY_SPACE);
			gameState->input.focus = IsKeyPressed(KEY_F);
			gameState->input.newThing = gameState->input.mouseClicked;
			gameState->input.reparentClick = IsKeyPressed(KEY_E);
			gameState->input.seaprateFromParentClick = IsKeyPressed(KEY_R);
		}

		// CAMERA POSITION UPDATE
		int cameraPosX = gameState->screenWidth / 2;
		int cameraPosY = gameState->screenHeight / 2;

		// MOUSE POSITION UPDATE
		int relativeX = gameState->input.mousePos.x - cameraPosX;
		int relativeY = gameState->input.mousePos.y - cameraPosY;
		gameState->input.mouseNormalizedPos.x = (float)relativeX / (float)cameraPosX;
		gameState->input.mouseNormalizedPos.y = (float)relativeY / (float)cameraPosX;
		gameState->aspectRatio = gameState->screenWidth / gameState->screenHeight;
		float angle = gameState->editorState.camera.fovy * DEG2RAD * 0.5f;
		float nearAxisY = tan(angle) * gameState->nearPlaneDistance;
		float nearAxisX = nearAxisY * gameState->aspectRatio;
		float nearPlanePos2dX = nearAxisX * gameState->input.mouseNormalizedPos.x;
		float nearPlanePos2dY = nearAxisX * gameState->input.mouseNormalizedPos.y;

		Vector3 cameraDir = Vector3Normalize(gameState->editorState.camera.target - gameState->editorState.camera.position);
		Vector3 cameraRight = Vector3Normalize(Vector3CrossProduct(cameraDir, gameState->editorState.camera.up));
		Vector3 cameraUp = Vector3CrossProduct(cameraRight, cameraDir);
		gameState->input.mouseNearPlanePos = gameState->editorState.camera.position
			+ cameraDir * gameState->nearPlaneDistance
			+ cameraRight * nearPlanePos2dX
			+ cameraUp * nearPlanePos2dY;

		gameState->input.mouseDir = Vector3Normalize(gameState->input.mouseNearPlanePos - gameState->editorState.camera.position);

		Vector3 xzPlanePos = { 0,0,0 };
		Vector3 xzPlaneNormal = { 0,-1,0 };
		Vector3 previousXZ_PlanePos = gameState->input.mouseXZ_PlanePos;
		Vector3 nextXZ_PlanePos = middle::RayCastLinePlane(xzPlanePos, xzPlaneNormal, gameState->input.mouseNearPlanePos, gameState->input.mouseDir);
		gameState->input.mouseXZ_PlanePos = nextXZ_PlanePos;
		gameState->input.mouseXZ_PlaneVelocity = (nextXZ_PlanePos - previousXZ_PlanePos) / gameState->frameTime;
	}
};

static middle::SystemRegistrar<InputSystem> reg("InputSystem");
