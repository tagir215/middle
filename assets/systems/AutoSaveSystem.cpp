#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "component_utils.h"
#include "QueuedForSaveTag.h"
#include "alg_file_utils.h"

class AutoSaveSystem : public middle::MiddleGameplaySystem {
	components::CompCache* cache;

	void init(middle::GameState* gameState) override {
		systemUpdateType = middle::SystemUpdateType::INITFRAME;
		updatePriority = 0;

		cache = middle::newCompCache(gameState, systemName);
		cache->addType<components::QueuedForSaveTag>();
	}
	void update(middle::GameState* gameState) override {

		for (middle::Id id : cache->relevantIdVector) {
			middle::queueComponentDeletion<components::QueuedForSaveTag>(gameState, id);
			bubequ::saveBubble(gameState, id, gameState->bubbleAlgebraState.activeBubbleName);
		}
	}
};

static middle::SystemRegistrar<AutoSaveSystem> reg("AutoSaveSystem");
