#include "game_state.h"

namespace middle {
	BubbleAlgebraState::BubbleAlgebraState() : 
		copyNegated(false),
		copyInverted(false),
		postUndoFrames(0),
		worldScale(1),
		loadDepth(20),
		cameraVelocity({0,0,0}),
		worldScalarRate(1)
	{
	}
}

