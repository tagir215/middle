#pragma once
#include "raylib.h"

namespace middle {

	enum InputBlockers {
		KEYBOARD_BLOCK,
		ADD_OBJECT_CHECKBOX,
	};

	struct EditorInput {
		Vector2 mousePos;
		Vector2 mouseNormalizedPos;
		Vector3 mouseNearPlanePos;
		Vector3 mouseXZ_PlanePos;
		Vector3 mouseXZ_PlaneVelocity;
		Vector3 mouseDir;
		bool mouseClicked = false;
		bool mouseReleased = false;
		bool mouseHeld = false;
		bool zoomIn = false;
		bool zoomOut = false;
		bool w = false;
		bool a = false;
		bool s = false;
		bool d = false;
		bool q = false;
		bool e = false;
		bool grabDown = false;
		bool grabReleased = false;
		bool infoClick = false;
		bool loopClick = false;
		bool sphereModeClick = false;
		bool constraintModeClick = false;
		bool cameraModeClick = false;
		bool selectModeClick = false;
		bool deleteClick = false;
		bool copyClick = false;
		bool saveClick = false;
		bool openScript = false;
		bool focus = false;
		bool newThing = false;
	};


}
