#pragma once
#include "game_state.h"
#include "bubble_actions.h"

namespace bubbleAnimations {

	struct AnimationTransform {
		Vector3 position = { 0,0,0 };
		Vector3 scale = { 1,1,1 };
		Quaternion rotation{ 0,0,0,0 };
	};

	typedef std::vector<AnimationTransform> AnimationTransforms;

	using ActionPtr = std::shared_ptr<bubbleActions::BubbleAction>;
	

	struct BubbleAnimation : public middle::Animation {
		ActionPtr action;
		AnimationTransforms prevFrame;
		std::vector<AnimationTransforms>animationKeyFrames;
		virtual AnimationTransforms captureBefore(middle::GameState* gameState) = 0;
		virtual AnimationTransforms captureCurrent(middle::GameState* gameState) = 0;
		virtual void start(middle::GameState* gamestate) = 0;
	};

	using AnimationPtr = std::shared_ptr<BubbleAnimation>;

	class BubbleAnimationWrapper : public bubbleActions::BubbleAction {
	public:

		AnimationPtr animation;
		BubbleAnimationWrapper(AnimationPtr animationPtr) {
			this->animation = animationPtr;
		}

		void execute(middle::GameState* gameState) override {
			animation->animationKeyFrames.push_back(animation->captureBefore(gameState));
			animation->action->execute(gameState);
			animation->start(gameState);
			gameState->animations.push_back(animation);
		}
		void undo(middle::GameState* gameState) override {
			animation->action->undo(gameState);
		}
	};

	class AdditionAnimation : public BubbleAnimation {
	public:
		enum Roles {
			ELEMENT_A,
			ELEMENT_B,
			NEW_CONTAINER
		};

		AnimationTransforms captureBefore(middle::GameState* gameState) override;
		AnimationTransforms captureCurrent(middle::GameState* gameState) override;
		void start(middle::GameState* gameState) override;

		AdditionAnimation(ActionPtr action) {
			this->action = action;
			duration = 0.5f;
		}

		void update(middle::GameState* gameState) override;
	};

}
