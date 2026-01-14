#pragma once
#include "game_state.h"
#include "registrars.h"
#include "middle_shape_utils.h"
#include "LoopSociety.h"
#include "LoopTag.h"
#include "BubbleComponent.h"

class BubbleOutlineSystem : public middle::MiddleGameplaySystem {
	void update(middle::GameState* gameState) override {
		middle::loopInstances(gameState, [](int i, middle::Shape& shape) {
			auto bubble = middle::getComponent<components::BubbleComponent>(shape);
			if (!bubble)
				return;

			auto loopTag = middle::getComponent<components::LoopTag>(shape);
			auto loopSociety = middle::getComponent<components::LoopSociety>(shape);
			assert(loopTag);
			assert(loopSociety);

			});
	}
};

static middle::SystemRegistrar<BubbleOutlineSystem> reg("BubbleOutlineSystem");
