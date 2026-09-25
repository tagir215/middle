#pragma once
#include "bubble_animations.h"

namespace animationActions {

	class LinearTranslation : public middle::Animation {
	public:
		middle::Id id;
		midMath::Vector3 startPos;
		midMath::Vector3 targetPos;
		LinearTranslation(middle::Id id, const midMath::Vector3& targetPos, float duration) {
			this->id = id;
			this->targetPos = targetPos;
			this->duration = duration;
		}
		void start(middle::GameState* gameState);
		void update(middle::GameState* gameState);
	};

	class Teleport : public middle::Animation {
	public:
		middle::Id id;
		midMath::Vector3 targetPos;
		midMath::Vector3 startPos;
		Teleport(middle::Id id, const midMath::Vector3& targetPos) {
			this->id = id;
			this->targetPos = targetPos;
		}
		void start(middle::GameState* gameState);
		void update(middle::GameState* gameState);
	};
}
