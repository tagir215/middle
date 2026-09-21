#include "animation_actions.h"

namespace animationActions {

	void LinearTranslation::start(middle::GameState* gameState) {
		startPos = middle::getGlobalPosition(gameState, id);
		gameState->animations.push_back(shared_from_this());
	}

	void LinearTranslation::update(middle::GameState* gameState) {


	}
}
