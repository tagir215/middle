#include "bubble_animations.h"
#include "middle_shape_utils.h"

namespace bubbleAnimations {


	void Animation::addBeginState(middle::GameState* gameState, const AnimationTransforms& state)
	{
		this->beginState = state;
	}

	void Animation::addEndState(middle::GameState* gameState, const AnimationTransforms& state)
	{
		this->endState = state;
	}

	void AdditionAnimation::update(middle::GameState* gameState, AnimationTransforms& state)
	{
		Vector3& posElementA = state[ELEMENT_A].position;
		Vector3& scaleElementA = state[ELEMENT_A].scale;
		Vector3& posElementB = state[ELEMENT_B].position;
		Vector3& scaleElementB = state[ELEMENT_B].scale;
		Vector3& posNewContainer = state[NEW_CONTAINER].position;
		Vector3& scaleNewContainer = state[NEW_CONTAINER].scale;

		const float progressRatio = progress / duration;

		const float phase1 = 0.2f;
		if (progressRatio < phase1) {
			const float ySeparation = 50;
			const Vector3& cameraPos = gameState->activeCamera.position;
			const Vector3 targetPosContainer = { cameraPos.x, -ySeparation, cameraPos.y };
			const Vector3 targetPosA = targetPosContainer + Vector3{ -20,0,0 };
			const Vector3 targetPosB = targetPosContainer + Vector3{ 20,0,0 };

			posNewContainer = targetPosContainer;
			posElementA = targetPosA;
			posElementB = targetPosB;
		}

		if (progressRatio > phase1) {
			posElementA = endState[ELEMENT_A].position;
			posElementB = endState[ELEMENT_B].position;
			posNewContainer = endState[NEW_CONTAINER].position;
		}
	}


}
