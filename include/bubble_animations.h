#pragma once
#include "game_state.h"
#include "bubble_actions.h"

namespace bubbleAnimations {

	struct AnimationTransform {
		Vector3 position;
		Vector3 scale;
		Quaternion rotation;
	};

	typedef std::vector<AnimationTransform> AnimationTransforms;
	
	class Animation {
	public:
		float progress = 0;
		float duration = 0;

		AnimationTransforms beginState;
		AnimationTransforms endState;
		void addBeginState(middle::GameState* gameState, const AnimationTransforms& state);
		void addEndState(middle::GameState* gameState, const AnimationTransforms& state);
		virtual void update(middle::GameState* gameState, AnimationTransforms& state) = 0;

		void setDuration(float duration) {
			this->duration = duration;
		}
		void progressAnimation(middle::GameState* gameState, std::vector<AnimationTransform>& state) {
			if (progress < duration) {
				progress += gameState->frameTime;
				update(gameState, state);
			}
		}
		virtual ~Animation() = default;
	};

	struct BubbleAnimationMap {
		std::vector<middle::Id>ids;
		AnimationTransforms animationState;
	};


	template<typename ActionType>
	class BubbleAnimationWrapper : public middle::EditorActionContainer {
	public:
		using ActionPtr = std::shared_ptr<ActionType>;
		using MappingFunc = std::function<void(middle::GameState*, ActionPtr, AnimationTransforms&)>;

		MappingFunc createBeginFrame;
		MappingFunc createEndFrame;
		ActionPtr action;

		AnimationTransforms beginState;
		AnimationTransforms currentState;
		AnimationTransforms endState;

		BubbleAnimationWrapper(ActionPtr action, MappingFunc createBeginFrame, MappingFunc createEndFrame) {
			this->action = action;
			this->createBeginFrame = createBeginFrame;
			this->createEndFrame = createEndFrame;
		}

		void execute(middle::GameState* gameState) override {
			createBeginFrame(gameState, action, beginState);
			action->execute(gameState);
			createEndFrame(gameState, action, endState);
		}
		void undo(middle::GameState* gameState) override {
			createBeginFrame(gameState, action, beginState);
			action->undo(gameState);
			createEndFrame(gameState, action, endState);
		}
	};

	class AdditionAnimation : public Animation {
	public:

		enum roles {
			ELEMENT_A,
			ELEMENT_B,
			NEW_CONTAINER
		};

		AdditionAnimation() {}
		void update(middle::GameState* gameState, AnimationTransforms& state) override;
	};

}
