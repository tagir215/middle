#include "animation_actions.h"

namespace animationActions {

	void LinearTranslation::start(middle::GameState* gameState) {
		startPos = middle::getGlobalPosition(gameState, id);
		progress = 0;
	}

	void LinearTranslation::update(middle::GameState* gameState) {
		float t = progress / duration;
		midMath::Vector3 pos = startPos + (targetPos - startPos) * t;
		middle::setGlobalPosition(gameState, id, pos);
	}

	void Teleport::start(middle::GameState* gameState) {
		startPos = middle::getGlobalPosition(gameState, id);
		progress = 0;
	}

	void Teleport::update(middle::GameState* gameState) {
		middle::setGlobalPosition(gameState, id, targetPos);
	}

}
