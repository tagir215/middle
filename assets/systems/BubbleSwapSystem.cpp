#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "MouseClickComponent.h"
#include "Button.h"
#include "BubbleComponent.h"
#include "component_utils.h"
#include "BubbleSwapComponent.h"
#include "RuntimeHiddenTag.h"
#include "IntersectingTag.h"

class BubbleSwapSystem : public middle::MiddleGameplaySystem {
	components::CompCache* cache;
	components::CompCache* intersectingCache;

	void init(middle::GameState* gameState) override {
		cache = middle::newCompCache(gameState, systemName);
		cache->addType<components::BubbleComponent>();
		cache->addType<components::MouseClickComponent>();
		cache->addType<components::BubbleSwapComponent>();
	}
	void update(middle::GameState* gameState) override {
		auto swapIt = cache->begin<components::BubbleSwapComponent>();
		for (middle::Id id : cache->relevantIdVector) {
			auto swap = *swapIt;
			int currentIndex = swap->activeIndex;
			swap->activeIndex = currentIndex == 0 ? 1 : 0;
			int inActiveIndex = currentIndex == 0 ? 0 : 1;

			std::vector<middle::Id>children;
			middle::getChildren(gameState, id, children);
			middle::Id activeChildId = children[swap->activeIndex];
			middle::Id inActiveChildId = children[inActiveIndex];
			middle::queueComponentDeletion<components::RuntimeHiddenTag>(gameState, activeChildId);
			middle::attachComponent<components::RuntimeHiddenTag>(gameState, inActiveChildId);
		}
	}
};

static middle::SystemRegistrar<BubbleSwapSystem> reg("BubbleSwapSystem");
