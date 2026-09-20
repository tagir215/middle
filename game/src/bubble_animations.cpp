#include "bubble_animations.h"
#include "middle_shape_utils.h"
#include "PauseLayoutTag.h"
#include "Rectangle.h"

namespace bubbleAnimations {


	AnimationTransforms AdditionAnimation::captureBefore(middle::GameState* gameState)
	{
		AnimationTransforms state;
		state.resize(3);
		using e = bubbleActions::ExecuteAddition;
		middle::Id idA = action->inputs[e::ID_TO_ADD];
		middle::Id idB = action->inputs[e::ID_TO_ADD_INTO];
		state[ELEMENT_A].position = middle::getGlobalPosition(gameState, idA.index);
		state[ELEMENT_B].position = middle::getGlobalPosition(gameState, idB.index);
		state[NEW_CONTAINER].position = { 0,0,0 };
		return state;
	}

	AnimationTransforms AdditionAnimation::captureCurrent(middle::GameState* gameState)
	{
		AnimationTransforms state;
		state.resize(3);

		using e = bubbleActions::ExecuteAddition;
		middle::Id containerId = action->outputs.back();
		std::vector<middle::Id>children;
		middle::getChildren(gameState, containerId , children);
		middle::Id idA = children[e::ID_TO_ADD];
		middle::Id idB = children[e::ID_TO_ADD_INTO];

		state[ELEMENT_A].position = middle::getGlobalPosition(gameState, idA.index);
		state[ELEMENT_B].position = middle::getGlobalPosition(gameState, idB.index);
		state[NEW_CONTAINER].position = middle::getGlobalPosition(gameState, containerId.index);
		return state;
	}


	void AdditionAnimation::start(middle::GameState* gameState)
	{
		middle::Id outputParent = middle::getParent(gameState, action->outputs.back());
		auto pause1 = middle::attachComponent<components::PauseLayoutTag>(gameState, outputParent);
		pause1->timeLeft = duration;
		auto pause2 = middle::attachComponent<components::PauseLayoutTag>(gameState, action->outputs.back());
		pause2->timeLeft = duration;
	}

	void AdditionAnimation::update(middle::GameState* gameState)
	{
		const float t = progress / duration;

		const float phase1 = 0.5f;

		Vector3 newContainerPos;
		Vector3 newPosA;
		Vector3 newPosB;


		if (t >= phase1 && animationKeyFrames.size() == 1) {
			animationKeyFrames.push_back(prevFrame);
		}

		auto& prevKeyFrame = animationKeyFrames.back();

		if (t < phase1) {
			const float ySeparation = 50;
			const Vector3& cameraPos = gameState->activeCamera.position;
			const Vector3 targetPosContainer = { cameraPos.x, -ySeparation, cameraPos.z };
			newContainerPos = targetPosContainer;
			auto rect = middle::getComp<components::Rectangle>(gameState, action->outputs.back());
			Vector3 scale = middle::getGlobalScale(gameState, action->outputs.back());
			float width = rect->width * scale.x;

			Vector3 startPosA = prevKeyFrame[ELEMENT_A].position;
			Vector3 startPosB = prevKeyFrame[ELEMENT_B].position;
			const Vector3 targetPosA = targetPosContainer + Vector3{ -width,0,0 };
			const Vector3 targetPosB = targetPosContainer + Vector3{ width,0,0 };

			const float phase1Ratio = t / phase1;
			newPosA = startPosA + (targetPosA - startPosA) * phase1Ratio;
			newPosB = startPosB + (targetPosB - startPosB) * phase1Ratio;
		}
		else {
			const float ySeparation = 50;
			const Vector3& cameraPos = gameState->activeCamera.position;
			const Vector3 targetPosContainer = { cameraPos.x, -ySeparation, cameraPos.z };
			newContainerPos = targetPosContainer;
			auto rect = middle::getComp<components::Rectangle>(gameState, action->outputs.back());
			Vector3 scale = middle::getGlobalScale(gameState, action->outputs.back());
			float width = rect->width * scale.x;

			Vector3 startPosA = prevKeyFrame[ELEMENT_A].position;
			Vector3 startPosB = prevKeyFrame[ELEMENT_B].position;
			const Vector3 targetPosA = targetPosContainer + Vector3{ -width * 0.2f,0,0 };
			const Vector3 targetPosB = targetPosContainer + Vector3{ width * 0.2f,0,0 };

			const float phase2Ratio = (t - phase1) / (1-phase1);
			newPosA = startPosA + (targetPosA - startPosA) * phase2Ratio;
			newPosB = startPosB + (targetPosB - startPosB) * phase2Ratio;
		}



		// update values
		using e = bubbleActions::ExecuteAddition;
		middle::Id resultId = action->outputs[e::ID_RESULT_ADDITION];
		// find corresponding entities from new graph and update positions
		std::vector<middle::Id>children;
		middle::getChildren(gameState, resultId, children);
		middle::Id newIdA = children[e::ID_TO_ADD];
		middle::Id newIdB = children[e::ID_TO_ADD_INTO];
		middle::setGlobalPosition(gameState, resultId, newContainerPos);
		middle::setGlobalPosition(gameState, newIdA, newPosA);
		middle::setGlobalPosition(gameState, newIdB, newPosB);


		prevFrame = captureCurrent(gameState);
	}

}
