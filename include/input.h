#pragma once
#include "raylib.h"

namespace middle {

	enum class InputBlockers {
		KEYBOARD_BLOCK,
		MOUSE_BLOCK,
		MOUSE_RELEASE_BLOCK,
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
		bool altDown = false;
		bool grabDown = false;
		bool grabReleased = false;
		bool rotatePressed = false;
		bool rotateReleased = false;
		bool scaleDown = false;
		bool scaleReleased = false;
		bool infoClick = false;
		bool loopClick = false;
		bool sphereModeClick = false;
		bool hideClick = false;
		bool constraintModeClick = false;
		bool cameraModeClick = false;
		bool loopModeClick = false;
		bool selectModeClick = false;
		bool deleteClick = false;
		bool copyClick = false;
		bool saveClick = false;
		bool navigateToFileClick = false;
		bool focus = false;
		bool newThing = false;
		bool reparentClick = false;
		bool seaprateFromParentClick = false;
	};

	struct GameInput {
		bool pop = false;
		bool mulOne = false;
		bool comp = false;
		bool can = false;
		bool proc = false;
		bool zoomIn = false;
		bool zoomOut = false;
		bool panDown = false;
		bool panUp = false;
		bool panLeft = false;
		bool panRight = false;
		bool undo = false;
		bool one = false;
		bool two = false;
		bool three = false;
		bool four = false;
		bool five = false;
		bool six = false;
		bool seven = false;
		bool eight = false;
		bool nine = false;
		bool zero = false;
		bool shiftHeld = false;
		float mouseWheelMove = false;
	};

	struct EqulabInput {
		bool oneHeld = false;
		bool twoHeld = false;
		bool threeHeld = false;
		bool fourHeld = false;
		bool fiveHeld = false;
		bool sixHeld = false;
		bool sevenHeld = false;
		bool eightHeld = false;
		bool nineHeld = false;
		bool zeroHeld = false;
		bool shiftHeld = false;
		bool ctrlHeld = false;
		bool leftHeld = false;
		bool rightHeld = false;
		bool upHeld = false;
		bool downHeld = false;

		bool oneClicked = false;
		bool twoClicked = false;
		bool threeClicked = false;
		bool fourClicked = false;
		bool fiveClicked = false;
		bool sixClicked = false;
		bool sevenClicked = false;
		bool eightClicked = false;
		bool nineClicked = false;
		bool zeroClicked = false;
		bool aClicked = false;
		bool bClicked = false;
		bool cClicked = false;
		bool dClicked = false;
		bool eClicked = false;
		bool fClicked = false;
		bool gClicked = false;
		bool hClicked = false;
		bool iClicked = false;
		bool jClicked = false;
		bool kClicked = false;
		bool lClicked = false;
		bool mClicked = false;
		bool nClicked = false;
		bool oClicked = false;
		bool pClicked = false;
		bool qClicked = false;
		bool rClicked = false;
		bool sClicked = false;
		bool tClicked = false;
		bool uClicked = false;
		bool vClicked = false;
		bool wClicked = false;
		bool xClicked = false;
		bool yClicked = false;
		bool zClicked = false;
	};

}
