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
#include "NonPhysicalBubbleTag.h"
#include "NeedsUpdateTag.h"

class BubbleSwapSystem : public middle::MiddleGameplaySystem {
	components::CompCache* cache;
	components::CompCache* needsUpdateCache;

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

			bubble::swapBubbleSwap(gameState, parentId);
		}

	}
};

static middle::SystemRegistrar<BubbleSwapSystem> reg("BubbleSwapSystem");
