#pragma once
#include "game_state.h"
#include "bubble_actions.h"

namespace bubbleAnimations {

	struct AnimationTransform {
		midMath::Vector3 position = { 0,0,0 };
		midMath::Vector3 scale = { 1,1,1 };
		midMath::Quaternion rotation{ 0,0,0,0 };
	};

	typedef std::vector<AnimationTransform> AnimationTransforms;

	using ActionPtr = std::shared_ptr<bubbleActions::BubbleAction>;
	

	struct BubbleAnimation : public middle::Animation {
		ActionPtr action;
		AnimationTransforms prevFrame;
		std::vector<AnimationTransforms>animationKeyFrames;
		std::vector<middle::Id> actorIds;
		std::vector<std::shared_ptr<Animation>>embeddedAnimations;
		virtual AnimationTransforms captureBefore(middle::GameState* gameState) = 0;
		virtual void assignActors(middle::GameState* gameState) = 0;

		template<class T, class... Args>
		void addChildAnimation(middle::GameState* gameState, BubbleAnimation* bubbleAnimation, Args&&... args) {
			auto animation = std::make_shared<T>(std::forward<Args>(args)...);
			animation->start(gameState);
			bubbleAnimation->embeddedAnimations.push_back(animation);
		}

		void playChildAnimations(middle::GameState* gameState) {
			for (auto& animation : embeddedAnimations) {
				animation->progressAnimation(gameState);
			}
		}
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
			animation->assignActors(gameState);
			animation->start(gameState);
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
		void assignActors(middle::GameState* gameState) override;
		void start(middle::GameState* gameState) override;

		AdditionAnimation(ActionPtr action) {
			this->action = action;
		}

		void update(middle::GameState* gameState) override;
	};

}
