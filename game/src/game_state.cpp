#include "game_state.h"

namespace middle {
	BubbleAlgebraState::BubbleAlgebraState() : 
		copyNegated(false),
		copyInverted(false),
		postUndoFrames(0),
		worldScale(1),
		loadDepth(200),
		cameraVelocity({0,0,0}),
		worldScalarRate(1)
	{
	}

	void Animation::setDuration(float duration) {
		this->duration = duration;
	}
	void Animation::progressAnimation(middle::GameState* gameState) {
		if (progress < duration) {
			progress += gameState->frameTime;
			update(gameState);
		}
	}
}

