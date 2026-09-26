#include "bubble_animations.h"
#include "middle_shape_utils.h"
#include "MidComp/PauseLayoutTag.h"
#include "MidComp/Rectangle.h"
#include "animation_actions.h"
#include "bubble_utils.h"

namespace bubbleAnimations {


	AnimationTransforms AdditionAnimation::captureBefore(middle::GameState* gameState)
	{
		AnimationTransforms state;
		state.resize(3);
		using e = bubbleActions::ExecuteAddition;
		middle::Id idA = action->inputs[e::ID_TO_ADD];
		middle::Id idB = action->inputs[e::ID_TO_ADD_INTO];
		state[ELEMENT_A].position = middle::getGlobalPosition(gameState, idA);
		state[ELEMENT_B].position = middle::getGlobalPosition(gameState, idB);
		state[ELEMENT_B].scale = middle::getLocalScale(gameState, idA);
		return state;
	}

	void AdditionAnimation::assignActors(middle::GameState* gameState)
	{
		actorIds.resize(3);
		middle::Id container = action->outputs.back();
		std::vector<middle::Id>children;
		middle::getChildren(gameState, container, children);
		using e = bubbleActions::ExecuteAddition;
		actorIds[ELEMENT_A] = children[0];
		actorIds[ELEMENT_B] = children[1];
		actorIds[NEW_CONTAINER] = container;
	}

	void AdditionAnimation::start(middle::GameState* gameState)
	{
		duration = 0.4f;
		middle::Id outputParent = middle::getParent(gameState, action->outputs.back());
		auto pause1 = middle::attachComponent<components::PauseLayoutTag>(gameState, outputParent);
		pause1->timeLeft = duration;
		auto pause2 = middle::attachComponent<components::PauseLayoutTag>(gameState, action->outputs.back());
		pause2->timeLeft = duration;
		gameState->animations.push_back(shared_from_this());
	}

	void AdditionAnimation::update(middle::GameState* gameState)
	{
		const float prevT = prevProgress / duration;
		const float t = progress / duration;

		const float phase1 = 0.2f;
		const float phase2 = 0.3f;

		auto& prevKeyFrame = animationKeyFrames.back();

			
		prevFrame = animationKeyFrames.back();
		// frame 1
		if (prevT == 0) {
			midMath::Vector3 initPosA = prevKeyFrame[ELEMENT_A].position;
			midMath::Vector3 initPosB = prevKeyFrame[ELEMENT_B].position;
			midMath::Vector3 initScaleB = prevKeyFrame[ELEMENT_B].scale;
			const midMath::Vector3 targetPosContainer = initPosB;
			const midMath::Vector3 targetScaleContainer = initScaleB;
			const midMath::Vector3 ySeparation = { 0,-5,0 };

			float time = duration * phase1;
			middle::setGlobalPosition(gameState, actorIds[NEW_CONTAINER], targetPosContainer + ySeparation);
			middle::setLocalScale(gameState, actorIds[NEW_CONTAINER], targetScaleContainer);
			bubble::recursiveBubbleLayoutScaleUpdate(gameState, actorIds[NEW_CONTAINER]);
			bubble::recursiveBubbleLayoutUpdate(gameState, actorIds[NEW_CONTAINER]);
		}

		playChildAnimations(gameState);
	}

}
