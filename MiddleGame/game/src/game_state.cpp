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
		if (!reverseMode) {
			if (progress <= duration) {
				prevProgress = progress;
				progress += gameState->middleInputState.frameTime;
				update(gameState);
			}
		}
		else {
			if (progress >= 0) {
				prevProgress = progress;
				progress -= gameState->middleInputState.frameTime;
				update(gameState);
			}
		}
	}
}

