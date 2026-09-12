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
#include "LoopSociety.h"
#include "bubble_utils.h"

class BubbleSwapSystem : public middle::MiddleGameplaySystem {
	components::CompCache* cache;
	components::CompCache* intersectingCache;

	void init(middle::GameState* gameState) override {
		cache = middle::newCompCache(gameState, systemName);
		cache->addType<components::BubbleComponent>();
		cache->addType<components::MouseClickComponent>();
		cache->addType<components::LoopSociety>();
	}
	void update(middle::GameState* gameState) override {
		auto loopIt = cache->begin<components::LoopSociety>();
		for (middle::Id id : cache->relevantIdVector) {
			auto loop = *loopIt;
			middle::Id parentId = loop->parentLoopId;
			if (!middle::isValidId(gameState, parentId)) {
				continue;
			}
			if (!bubble::isSwapBubble(gameState, parentId)) {
				continue;
			}
			auto swapComp = middle::getComp<components::BubbleSwapComponent>(gameState, parentId);
			int currentIndex = swapComp->activeIndex;
			swapComp->activeIndex = currentIndex == 0 ? 1 : 0;
			int inActiveIndex = currentIndex == 0 ? 0 : 1;

			std::vector<middle::Id>children;
			middle::getChildren(gameState, parentId, children);

			middle::Id activeChildId = children[swapComp->activeIndex];
			middle::Id inActiveChildId = children[inActiveIndex];
			bubble::recursiveUnHideBubble(gameState, activeChildId);
			middle::queueComponentDeletion<components::Button>(gameState, activeChildId);
			bubble::recursiveHideBubble(gameState, inActiveChildId);
			middle::attachComponent<components::Button>(gameState, inActiveChildId);
		}
	}
};

static middle::SystemRegistrar<BubbleSwapSystem> reg("BubbleSwapSystem");
