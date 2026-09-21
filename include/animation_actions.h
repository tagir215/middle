#pragma once
#include "bubble_animations.h"

namespace animationActions {

	class LinearTranslation : public middle::Animation {
	public:
		middle::Id id;
		Vector3 startPos;
		Vector3 targetPos;
		float duration;
		LinearTranslation(middle::Id id, const Vector3& targetPos, float duration) {
			this->id = id;
			this->targetPos = targetPos;
			this->duration = duration;
		}
		void start(middle::GameState* gameState);
		void update(middle::GameState* gameState);
	};
}
