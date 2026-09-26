#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "raylib.h"
#include "middle_math_maping_helper.h"

class InputSystem {
public:
	static void update(middle::MiddleInputState* middleState, const middle::MiddleOutputState* const outputState)  {
		if (outputState == nullptr) {
			return;
		}


		// INPUTS

		middleState->editorInput = middle::EditorInput();
		auto& ip = middleState->editorInput;

		middleState->editorInput.mouseX = GetMouseX();
		middleState->editorInput.mouseY = GetMouseY();

		if (outputState->inputBlockers.find(middle::InputBlockers::MOUSE_BLOCK) == outputState->inputBlockers.end()) {
			middleState->editorInput.mouseHeld = IsMouseButtonDown(MOUSE_BUTTON_LEFT);
			middleState->editorInput.mouseClicked = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
			middleState->editorInput.mouseReleased = IsMouseButtonReleased(MOUSE_BUTTON_LEFT);
		}

		if (outputState->inputBlockers.find(middle::InputBlockers::KEYBOARD_BLOCK) == outputState->inputBlockers.end()) {
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
			ip.newThing = middleState->editorInput.mouseClicked;
			ip.reparentClick = IsKeyPressed(KEY_E);
			ip.seaprateFromParentClick = IsKeyPressed(KEY_R);
			ip.nextScene = IsKeyPressed(KEY_TAB);
		}

		middleState->gameInput = middle::GameInput();
		auto& gi = middleState->gameInput;

		if (outputState->inputBlockers.find(middle::InputBlockers::KEYBOARD_BLOCK) == outputState->inputBlockers.end()) {
			gi.copy = IsKeyPressed(KEY_F);
			gi.insertTerm = IsKeyPressed(KEY_T);
			gi.pop = IsKeyPressed(KEY_B);
			gi.zoomIn = IsKeyDown(KEY_E);
			gi.zoomOut = IsKeyDown(KEY_Q);
			gi.panUp = IsKeyDown(KEY_W);
			gi.panDown = IsKeyDown(KEY_S);
			gi.panLeft = IsKeyDown(KEY_A);
			gi.panRight = IsKeyDown(KEY_D);
			gi.pop = IsKeyPressed(KEY_Z);
			gi.can = IsKeyPressed(KEY_X);
			gi.comp = IsKeyPressed(KEY_C);
			gi.mulOne = IsKeyPressed(KEY_V);
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

		middleState->gameInput.mouseWheelMove = GetMouseWheelMove();


		if (outputState->inputBlockers.find(middle::InputBlockers::KEYBOARD_BLOCK) == outputState->inputBlockers.end()) {
			middleState->equlabInput = middle::EqulabInput();
			auto& ei = middleState->equlabInput;
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
			ei.f1Clicked = IsKeyPressed(KEY_F1);
			ei.f2Clicked = IsKeyPressed(KEY_F2);
			ei.f3Clicked = IsKeyPressed(KEY_F3);
			ei.f4Clicked = IsKeyPressed(KEY_F4);
			ei.f5Clicked = IsKeyPressed(KEY_F5);
			ei.f6Clicked = IsKeyPressed(KEY_F6);
			ei.f7Clicked = IsKeyPressed(KEY_F7);
			ei.f8Clicked = IsKeyPressed(KEY_F8);
			ei.f9Clicked = IsKeyPressed(KEY_F9);
			ei.f10Clicked = IsKeyPressed(KEY_F10);
			ei.f11Clicked = IsKeyPressed(KEY_F11);
			ei.f12Clicked = IsKeyPressed(KEY_F12);
			ei.leftHeld = IsKeyDown(KEY_LEFT);
			ei.rightHeld = IsKeyDown(KEY_RIGHT);
			ei.upHeld = IsKeyDown(KEY_UP);
			ei.downHeld = IsKeyDown(KEY_DOWN);
			ei.shiftHeld = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);
			ei.ctrlHeld = IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL);
			ei.altHeld = IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT);
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


};

