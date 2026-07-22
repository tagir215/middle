#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "middle_math.h"

class InputSystem : public middle::MiddleGameplaySystem {
public:
	void init(middle::GameState* gameState) {

	}
	void update(middle::GameState* gameState) override {


		Matrix scalorM = MatrixScale(1, -1, 1);
		Matrix translatorM = MatrixTranslate(0, GetScreenHeight(), 0);
		gameState->screenOrientorM = MatrixMultiply(scalorM, translatorM);

		// INPUTS
		Vector3 mousePos = { GetMouseX(), GetMouseY(), 0 };
		Vector3 invertedMouse = Vector3Transform(mousePos, gameState->screenOrientorM);
		gameState->input.mousePos.x = invertedMouse.x;
		gameState->input.mousePos.y = invertedMouse.y;

		gameState->input.mouseHeld = false;
		gameState->input.mouseClicked = false;
		gameState->input.mouseReleased = false;

		if (gameState->inputBlockers.find(middle::InputBlockers::MOUSE_BLOCK) == gameState->inputBlockers.end()) {
			gameState->input.mouseHeld = IsMouseButtonDown(MOUSE_BUTTON_LEFT);
			gameState->input.mouseClicked = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
			gameState->input.mouseReleased = IsMouseButtonReleased(MOUSE_BUTTON_LEFT);
			gameState->input.zoomIn = GetMouseWheelMoveV().y > 0;
			gameState->input.zoomOut = GetMouseWheelMoveV().y < 0;
		}

		if (gameState->applicationMode == middle::ApplicationMode::EDITOR_MODE) {
			//gameState->input = middle::EditorInput();
			auto& ip = gameState->input;

			ip.w = false;
			ip.a = false;
			ip.s = false;
			ip.d = false;
			ip.q = false;
			ip.e = false;
			ip.altDown = false;
			ip.grabDown = false;
			ip.grabReleased = false;
			ip.rotatePressed = false;
			ip.rotateReleased = false;
			ip.scaleDown = false;
			ip.scaleReleased = false;
			ip.infoClick = false;
			ip.loopClick = false;
			ip.sphereModeClick = false;
			ip.hideClick = false;
			ip.constraintModeClick = false;
			ip.cameraModeClick = false;
			ip.loopModeClick = false;
			ip.selectModeClick = false;
			ip.deleteClick = false;
			ip.copyClick = false;
			ip.saveClick = false;
			ip.navigateToFileClick = false;
			ip.focus = false;
			ip.newThing = false;
			ip.reparentClick = false;
			ip.seaprateFromParentClick = false;

			if (gameState->inputBlockers.find(middle::InputBlockers::KEYBOARD_BLOCK) == gameState->inputBlockers.end()) {
				ip.w = IsKeyDown(KEY_W);
				ip.s = IsKeyDown(KEY_S);
				ip.a = IsKeyDown(KEY_A);
				ip.d = IsKeyDown(KEY_D);
				ip.q = IsKeyDown(KEY_Q);
				ip.e = IsKeyDown(KEY_E);
				ip.altDown = IsKeyDown(KEY_LEFT_ALT);
				ip.grabDown = IsKeyDown(KEY_G);
				ip.grabReleased = IsKeyReleased(KEY_G);
				ip.grabReleased = IsKeyReleased(KEY_G);
				ip.rotatePressed = IsKeyPressed(KEY_X);
				ip.rotateReleased = IsKeyReleased(KEY_X);
				ip.scaleDown = IsKeyDown(KEY_Z);
				ip.scaleReleased = IsKeyReleased(KEY_Z);
				ip.infoClick = IsKeyPressed(KEY_I);
				ip.loopClick = IsKeyPressed(KEY_L);
				ip.selectModeClick = IsKeyPressed(KEY_ONE);
				ip.sphereModeClick = IsKeyPressed(KEY_TWO);
				ip.hideClick = IsKeyPressed(KEY_H);
				ip.constraintModeClick = IsKeyPressed(KEY_THREE);
				ip.cameraModeClick = IsKeyPressed(KEY_FOUR);
				ip.loopModeClick = IsKeyPressed(KEY_FIVE);
				ip.deleteClick = IsKeyPressed(KEY_R);
				ip.copyClick = IsKeyPressed(KEY_C);
				ip.saveClick = IsKeyPressed(KEY_P);
				ip.navigateToFileClick = IsKeyPressed(KEY_SPACE);
				ip.focus = IsKeyPressed(KEY_F);
				ip.newThing = gameState->input.mouseClicked;
				ip.reparentClick = IsKeyPressed(KEY_E);
				ip.seaprateFromParentClick = IsKeyPressed(KEY_R);
			}
		}

		if (gameState->applicationMode == middle::ApplicationMode::GAME_MODE) {
			gameState->gameInput = middle::GameInput();
			auto& gi = gameState->gameInput;

			if (gameState->inputBlockers.find(middle::InputBlockers::KEYBOARD_BLOCK) == gameState->inputBlockers.end()) {
				gi.pop = IsKeyPressed(KEY_B);
				gi.zoomIn = IsKeyDown(KEY_W);
				gi.zoomOut = IsKeyDown(KEY_S);
				gi.panUp = IsKeyDown(KEY_W);
				gi.panDown = IsKeyDown(KEY_S);
				gi.panLeft = IsKeyDown(KEY_A);
				gi.panRight = IsKeyDown(KEY_D);
				gi.pop = IsKeyPressed(KEY_Z);
				gi.can = IsKeyPressed(KEY_X);
				gi.comp = IsKeyPressed(KEY_C);
				gi.mulOne = IsKeyPressed(KEY_V);
				gi.proc = IsKeyPressed(KEY_E);
				gi.undo = IsKeyPressed(KEY_SPACE);
				gi.one = IsKeyDown(KEY_ONE);
				gi.two = IsKeyDown(KEY_TWO);
				gi.three = IsKeyDown(KEY_THREE);
				gi.four = IsKeyDown(KEY_FOUR);
				gi.five = IsKeyDown(KEY_FIVE);
				gi.six = IsKeyDown(KEY_SIX);
				gi.seven = IsKeyDown(KEY_SEVEN);
				gi.eight = IsKeyDown(KEY_EIGHT);
				gi.nine = IsKeyDown(KEY_NINE);
				gi.zero = IsKeyDown(KEY_ZERO);
				gi.shiftHeld = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);
			}

			gameState->gameInput.mouseWheelMove = GetMouseWheelMove();


			if (gameState->inputBlockers.find(middle::InputBlockers::KEYBOARD_BLOCK) == gameState->inputBlockers.end()) {
				gameState->equlabInput = middle::EqulabInput();
				auto& ei = gameState->equlabInput;
				ei.oneHeld = IsKeyDown(KEY_ONE);
				ei.twoHeld = IsKeyDown(KEY_TWO);
				ei.threeHeld = IsKeyDown(KEY_THREE);
				ei.fourHeld = IsKeyDown(KEY_FOUR);
				ei.fiveHeld = IsKeyDown(KEY_FIVE);
				ei.sixHeld = IsKeyDown(KEY_SIX);
				ei.sevenHeld = IsKeyDown(KEY_SEVEN);
				ei.eightHeld = IsKeyDown(KEY_EIGHT);
				ei.nineHeld = IsKeyDown(KEY_NINE);
				ei.zeroHeld = IsKeyDown(KEY_ZERO);
				ei.shiftHeld = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);
				ei.ctrlHeld = IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL);
				ei.oneClicked = IsKeyPressed(KEY_ONE);
				ei.twoClicked = IsKeyPressed(KEY_TWO);
				ei.threeClicked = IsKeyPressed(KEY_THREE);
				ei.fourClicked = IsKeyPressed(KEY_FOUR);
				ei.fiveClicked = IsKeyPressed(KEY_FIVE);
				ei.sixClicked = IsKeyPressed(KEY_SIX);
				ei.sevenClicked = IsKeyPressed(KEY_SEVEN);
				ei.eightClicked = IsKeyPressed(KEY_EIGHT);
				ei.nineClicked = IsKeyPressed(KEY_NINE);
				ei.zeroClicked = IsKeyPressed(KEY_ZERO);
				ei.aClicked = IsKeyPressed(KEY_A);
				ei.bClicked = IsKeyPressed(KEY_B);
				ei.cClicked = IsKeyPressed(KEY_C);
				ei.dClicked = IsKeyPressed(KEY_D);
				ei.eClicked = IsKeyPressed(KEY_E);
				ei.fClicked = IsKeyPressed(KEY_F);
				ei.gClicked = IsKeyPressed(KEY_G);
				ei.hClicked = IsKeyPressed(KEY_H);
				ei.iClicked = IsKeyPressed(KEY_I);
				ei.jClicked = IsKeyPressed(KEY_J);
				ei.kClicked = IsKeyPressed(KEY_K);
				ei.lClicked = IsKeyPressed(KEY_L);
				ei.mClicked = IsKeyPressed(KEY_M);
				ei.nClicked = IsKeyPressed(KEY_N);
				ei.oClicked = IsKeyPressed(KEY_O);
				ei.pClicked = IsKeyPressed(KEY_P);
				ei.qClicked = IsKeyPressed(KEY_Q);
				ei.rClicked = IsKeyPressed(KEY_R);
				ei.sClicked = IsKeyPressed(KEY_S);
				ei.tClicked = IsKeyPressed(KEY_T);
				ei.uClicked = IsKeyPressed(KEY_U);
				ei.vClicked = IsKeyPressed(KEY_V);
				ei.wClicked = IsKeyPressed(KEY_W);
				ei.xClicked = IsKeyPressed(KEY_X);
				ei.yClicked = IsKeyPressed(KEY_Y);
				ei.zClicked = IsKeyPressed(KEY_Z);
			}
		}









		// TODO MOVE THESE
		// CAMERA POSITION UPDATE
		int cameraPosX = gameState->screenWidth / 2;
		int cameraPosY = gameState->screenHeight / 2;

		// MOUSE POSITION UPDATE
		int relativeX = gameState->input.mousePos.x - cameraPosX;
		int relativeY = gameState->input.mousePos.y - cameraPosY;
		gameState->input.mouseNormalizedPos.x = (float)relativeX / (float)cameraPosX;
		gameState->input.mouseNormalizedPos.y = (float)relativeY / (float)cameraPosX;
		gameState->aspectRatio = gameState->screenWidth / gameState->screenHeight;
		float angle = gameState->activeCamera.fovy * DEG2RAD * 0.5f;
		float nearAxisY = tan(angle) * gameState->nearPlaneDistance;
		float nearAxisX = nearAxisY * gameState->aspectRatio;
		float nearPlanePos2dX = nearAxisX * gameState->input.mouseNormalizedPos.x;
		float nearPlanePos2dY = nearAxisX * gameState->input.mouseNormalizedPos.y;

		Vector3 cameraDir = Vector3Normalize(gameState->activeCamera.target - gameState->activeCamera.position);
		Vector3 cameraRight = Vector3Normalize(Vector3CrossProduct(cameraDir, gameState->activeCamera.up));
		Vector3 cameraUp = Vector3CrossProduct(cameraRight, cameraDir);
		gameState->input.mouseNearPlanePos = gameState->activeCamera.position
			+ cameraDir * gameState->nearPlaneDistance
			+ cameraRight * nearPlanePos2dX
			+ cameraUp * nearPlanePos2dY;

		gameState->input.mouseDir = Vector3Normalize(gameState->input.mouseNearPlanePos - gameState->activeCamera.position);

		Vector3 xzPlanePos = { 0,0,0 };
		Vector3 xzPlaneNormal = { 0,-1,0 };
		Vector3 previousXZ_PlanePos = gameState->input.mouseXZ_PlanePos;
		Vector3 nextXZ_PlanePos = middle::RayCastLinePlane(xzPlanePos, xzPlaneNormal, gameState->input.mouseNearPlanePos, gameState->input.mouseDir);
		gameState->input.mouseXZ_PlanePos = nextXZ_PlanePos;
		gameState->input.mouseXZ_PlaneVelocity = (nextXZ_PlanePos - previousXZ_PlanePos) / gameState->frameTime;
	}
};

static middle::SystemRegistrar<InputSystem> reg("InputSystem");
